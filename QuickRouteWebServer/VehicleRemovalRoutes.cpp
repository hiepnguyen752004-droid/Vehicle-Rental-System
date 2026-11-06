
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>

#include <crow.h>

#include "VehicleRemovalRoutes.h"
#include "AuthRoutes.h"

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <utility>

using namespace std;

namespace
{
    // =====================================
    // JSON RESPONSES
    // =====================================

    crow::response jsonResponse(
        int statusCode,
        crow::json::wvalue& data)
    {
        crow::response response(
            statusCode,
            data.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        response.set_header(
            "Cache-Control",
            "no-store"
        );

        return response;
    }

    crow::response errorResponse(
        int statusCode,
        const string& message)
    {
        crow::json::wvalue data;

        data["status"] = "error";
        data["message"] = message;

        return jsonResponse(
            statusCode,
            data
        );
    }


    // =====================================
    // MYSQL CONNECTION
    // =====================================

    unique_ptr<sql::Connection> openDatabase(
        const string& password)
    {
        sql::Driver* driver =
            sql::mysql::get_driver_instance();

        unique_ptr<sql::Connection> connection(
            (*driver).connect(
                "tcp://127.0.0.1:3306",
                "vehicle_app",
                password
            )
        );

        (*connection).setSchema(
            "vehicle_rental_db"
        );

        return connection;
    }


    // =====================================
    // ARCHIVE, RESTORE, DELETE
    // =====================================

    crow::response changeVehicle(
        const crow::request& request,
        int vehicleId,
        const string& action,
        const string& password)
    {
        // Verify request origin.

        if (!authorizeWriteOrigin(request))
        {
            return errorResponse(
                403, "Forbidden"
            );
        }

        // Verify manager login.

        int managerId = 0;

        if (!authorizeManager(
            request,
            password,
            managerId))
        {
            return errorResponse(
                403,
                "Manager access required"
            );
        }

        if (vehicleId <= 0)
        {
            return errorResponse(
                400, "Invalid vehicle ID"
            );
        }

        unique_ptr<sql::Connection> connection;
        bool inTransaction = false;

        try
        {
            connection = openDatabase(password);

            (*connection).setAutoCommit(false);
            inTransaction = true;

            // Lock vehicle row so booking and
            // removal operations cannot change
            // the vehicle simultaneously.

            unique_ptr<sql::PreparedStatement> find(
                (*connection).prepareStatement(
                    "SELECT status FROM vehicles "
                    "WHERE vehicle_id = ? "
                    "FOR UPDATE"
                )
            );

            (*find).setInt(1, vehicleId);

            unique_ptr<sql::ResultSet> vehicle(
                (*find).executeQuery()
            );

            if (!(*vehicle).next())
            {
                (*connection).rollback();

                return errorResponse(
                    404,
                    "Vehicle not found"
                );
            }

            string currentStatus =
                string(
                    (*vehicle).getString("status").c_str()
                );

            // Check ALL reservations for deletion.

            unique_ptr<sql::PreparedStatement> countBookings(
                (*connection).prepareStatement(
                    "SELECT COUNT(*) AS total "
                    "FROM reservations "
                    "WHERE vehicle_id = ?"
                )
            );

            (*countBookings).setInt(1, vehicleId);

            unique_ptr<sql::ResultSet> bookingCount(
                (*countBookings).executeQuery()
            );

            (*bookingCount).next();

            int totalBookings =
                (*bookingCount).getInt("total");


            // =================================
            // DELETE VEHICLE
            // =================================

            if (action == "delete")
            {
                // Keep reservation history intact.

                if (totalBookings > 0)
                {
                    (*connection).rollback();

                    return errorResponse(
                        409,
                        "This vehicle has reservation "
                        "history. Archive it instead."
                    );
                }

                unique_ptr<sql::PreparedStatement> remove(
                    (*connection).prepareStatement(
                        "DELETE FROM vehicles "
                        "WHERE vehicle_id = ?"
                    )
                );

                (*remove).setInt(1, vehicleId);

                if ((*remove).executeUpdate() != 1)
                {
                    throw runtime_error(
                        "Vehicle deletion failed"
                    );
                }

                (*connection).commit();
                inTransaction = false;

                crow::json::wvalue result;

                result["status"] = "success";
                result["vehicleId"] = vehicleId;
                result["message"] =
                    "Vehicle permanently deleted";

                return jsonResponse(200, result);
            }


            // =================================
            // ARCHIVE VEHICLE
            // =================================

            if (action == "archive")
            {
                if (currentStatus == "Archived")
                {
                    (*connection).rollback();

                    return errorResponse(
                        409,
                        "Vehicle is already archived"
                    );
                }

                // Do not archive a vehicle while
                // it has confirmed current/future
                // reservations that must be honored.

                unique_ptr<sql::PreparedStatement> future(
                    (*connection).prepareStatement(
                        "SELECT COUNT(*) AS total "
                        "FROM reservations "
                        "WHERE vehicle_id = ? "
                        "AND status = 'Confirmed' "
                        "AND end_date >= CURDATE()"
                    )
                );

                (*future).setInt(1, vehicleId);

                unique_ptr<sql::ResultSet> futureResult(
                    (*future).executeQuery()
                );

                (*futureResult).next();

                if ((*futureResult).getInt("total") > 0)
                {
                    (*connection).rollback();

                    return errorResponse(
                        409,
                        "Vehicle has active or upcoming "
                        "reservations. Resolve them "
                        "before archiving."
                    );
                }
            }


            // =================================
            // RESTORE VEHICLE
            // =================================

            if (action == "restore")
            {
                if (currentStatus != "Archived")
                {
                    (*connection).rollback();

                    return errorResponse(
                        409,
                        "Vehicle is not archived"
                    );
                }
            }

            if (action != "archive" &&
                action != "restore")
            {
                (*connection).rollback();

                return errorResponse(
                    400, "Invalid action"
                );
            }

            string newStatus =
                action == "archive"
                ? "Archived"
                : "Available";

            unique_ptr<sql::PreparedStatement> update(
                (*connection).prepareStatement(
                    "UPDATE vehicles "
                    "SET status = ? "
                    "WHERE vehicle_id = ?"
                )
            );

            (*update).setString(1, newStatus);
            (*update).setInt(2, vehicleId);

            if ((*update).executeUpdate() != 1)
            {
                throw runtime_error(
                    "Vehicle status update failed"
                );
            }

            (*connection).commit();
            inTransaction = false;

            crow::json::wvalue result;

            result["status"] = "success";
            result["vehicleId"] = vehicleId;
            result["vehicleStatus"] = newStatus;

            result["message"] =
                action == "archive"
                ? "Vehicle archived successfully"
                : "Vehicle restored successfully";

            return jsonResponse(200, result);
        }
        catch (const exception& error)
        {
            if (inTransaction && connection)
            {
                try
                {
                    (*connection).rollback();
                }
                catch (...) {}
            }

            cerr << "Vehicle removal error: "
                << error.what() << endl;

            return errorResponse(
                500,
                "Unable to update vehicle inventory"
            );
        }
    }
}


// =========================================
// REGISTER VEHICLE REMOVAL ROUTES
// =========================================

void registerVehicleRemovalRoutes(
    crow::SimpleApp& app,
    const string& mysqlPassword)
{
    auto password =
        make_shared<const string>(mysqlPassword);


    // =====================================
    // MANAGER REMOVAL PAGE
    // =====================================

    app.static_file(
        "/manager-removal.html",
        "manager-removal.html"
    );


    // =====================================
    // LIST ALL VEHICLES
    // INCLUDING ARCHIVED VEHICLES
    // =====================================

    CROW_ROUTE(
        app,
        "/api/manager/vehicle-removal/list"
    )
        .methods(crow::HTTPMethod::Get)
        ([password](const crow::request& request)
            {
                int managerId = 0;

                if (!authorizeManager(
                    request,
                    *password,
                    managerId))
                {
                    return errorResponse(
                        403,
                        "Manager access required"
                    );
                }

                try
                {
                    auto connection =
                        openDatabase(*password);

                    unique_ptr<sql::Statement> statement(
                        (*connection).createStatement()
                    );

                    unique_ptr<sql::ResultSet> rows(
                        (*statement).executeQuery(
                            "SELECT "
                            "vehicle_id, make, model, "
                            "year, category, daily_rate, "
                            "status "
                            "FROM vehicles "
                            "ORDER BY vehicle_id ASC"
                        )
                    );

                    crow::json::wvalue::list vehicles;

                    while ((*rows).next())
                    {
                        crow::json::wvalue item;

                        item["id"] =
                            (*rows).getInt("vehicle_id");

                        item["make"] =
                            string(
                                (*rows).getString("make").c_str()
                            );

                        item["model"] =
                            string(
                                (*rows).getString("model").c_str()
                            );

                        item["year"] =
                            (*rows).getInt("year");

                        item["category"] =
                            string(
                                (*rows).getString("category").c_str()
                            );

                        item["dailyRate"] =
                            static_cast<double>(
                                (*rows).getDouble("daily_rate")
                                );

                        item["status"] =
                            string(
                                (*rows).getString("status").c_str()
                            );

                        vehicles.push_back(move(item));
                    }

                    crow::json::wvalue result;

                    result["status"] = "success";
                    result["vehicles"] = move(vehicles);

                    return jsonResponse(200, result);
                }
                catch (const exception& error)
                {
                    cerr << "Removal list error: "
                        << error.what() << endl;

                    return errorResponse(
                        500,
                        "Unable to load vehicle inventory"
                    );
                }
            });


    // =====================================
    // ARCHIVE
    // =====================================

    CROW_ROUTE(
        app,
        "/api/manager/vehicle-removal/<int>/archive"
    )
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int vehicleId)
            {
                return changeVehicle(
                    request,
                    vehicleId,
                    "archive",
                    *password
                );
            });

    // RESTORE


    CROW_ROUTE(
        app,
        "/api/manager/vehicle-removal/<int>/restore"
    )
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int vehicleId)
            {
                return changeVehicle(
                    request,
                    vehicleId,
                    "restore",
                    *password
                );
            });



    CROW_ROUTE(
        app,
        "/api/manager/vehicle-removal/<int>/delete"
    )
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int vehicleId)
            {
                return changeVehicle(
                    request,
                    vehicleId,
                    "delete",
                    *password
                );
            });
}
