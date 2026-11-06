#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>
#include <crow.h>

#include "BookingRoutes.h"
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
#include <cmath>
#include <stdexcept>
#include <utility>

using namespace std;

namespace {

    struct Dates {
        string start;
        string end;
        int days = 0;
    };

    struct VehicleInfo {
        int vehicleId = 0;
        string make;
        string model;
        string status;
        double rate = 0;
    };


    // =====================================
    // JSON RESPONSES
    // =====================================

    crow::response jsonReply(
        int code,
        crow::json::wvalue& data
    ) {
        crow::response response(code, data.dump());

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

    crow::response errorReply(
        int code,
        const string& message
    ) {
        crow::json::wvalue data;

        data["status"] = "error";
        data["message"] = message;

        return jsonReply(code, data);
    }


    // =====================================
    // MYSQL CONNECTION
    // =====================================

    unique_ptr<sql::Connection> openDatabase(
        const string& password
    ) {
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
    // DATE VALIDATION
    // =====================================

    bool isLeapYear(int year) {
        return year % 4 == 0 &&
            (year % 100 != 0 || year % 400 == 0);
    }

    // Convert a calendar date to a day number.
    // Avoid time zones and daylight-saving problems.

    long long serialDay(
        int year,
        unsigned month,
        unsigned day
    ) {
        year -= month <= 2;

        const int era =
            (year >= 0 ? year : year - 399) / 400;

        const unsigned yoe =
            static_cast<unsigned>(year - era * 400);

        const unsigned adjustedMonth =
            month > 2 ? month - 3 : month + 9;

        const unsigned doy =
            (153 * adjustedMonth + 2) / 5 + day - 1;

        const unsigned yoeDays =
            yoe * 365 + yoe / 4 - yoe / 100 + doy;

        return static_cast<long long>(era) * 146097
            + static_cast<long long>(yoeDays);
    }

    long long parseISODate(const string& text) {

        if (text.size() != 10 ||
            text[4] != '-' ||
            text[7] != '-') {

            throw invalid_argument(
                "Use YYYY-MM-DD for rental dates"
            );
        }

        for (size_t i = 0; i < text.size(); ++i) {

            if (i == 4 || i == 7) {
                continue;
            }

            if (text[i] < '0' || text[i] > '9') {
                throw invalid_argument(
                    "Invalid rental date"
                );
            }
        }

        const int year =
            stoi(text.substr(0, 4));

        const unsigned month =
            static_cast<unsigned>(
                stoi(text.substr(5, 2))
                );

        const unsigned day =
            static_cast<unsigned>(
                stoi(text.substr(8, 2))
                );

        if (year < 2020 ||
            year > 2100 ||
            month < 1 ||
            month > 12) {

            throw invalid_argument(
                "Date is outside supported range"
            );
        }

        const unsigned daysInMonth[] = {
            0, 31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        unsigned maxDay = daysInMonth[month];

        if (month == 2 && isLeapYear(year)) {
            maxDay = 29;
        }

        if (day < 1 || day > maxDay) {
            throw invalid_argument(
                "Invalid calendar date"
            );
        }

        return serialDay(year, month, day);
    }

    Dates checkedDates(
        const string& start,
        const string& end
    ) {
        const long long first =
            parseISODate(start);

        const long long last =
            parseISODate(end);

        // Both dates are counted, matching
        // your existing console project.

        const long long days =
            last - first + 1;

        if (days < 1 || days > 60) {
            throw invalid_argument(
                "Choose between 1 and 60 rental days"
            );
        }

        return {
            start,
            end,
            static_cast<int>(days)
        };
    }

    string databaseToday(sql::Connection& connection) {

        unique_ptr<sql::Statement> statement(
            connection.createStatement()
        );

        unique_ptr<sql::ResultSet> result(
            (*statement).executeQuery(
                "SELECT DATE_FORMAT("
                "CURDATE(), '%Y-%m-%d'"
                ") AS today"
            )
        );

        if (!(*result).next()) {
            throw runtime_error(
                "Database date unavailable"
            );
        }

        return string(
            (*result).getString("today").c_str()
        );
    }

    void checkNotPast(
        const Dates& dates,
        sql::Connection& connection
    ) {
        if (dates.start < databaseToday(connection)) {
            throw invalid_argument(
                "Pickup date cannot be in the past"
            );
        }
    }


    // =====================================
    // VEHICLE AND AVAILABILITY
    // =====================================

    bool readVehicle(
        sql::Connection& connection,
        int id,
        VehicleInfo& car,
        bool lockRow
    ) {
        string query =
            "SELECT vehicle_id, make, model, "
            "daily_rate, status "
            "FROM vehicles WHERE vehicle_id = ?";

        if (lockRow) {
            query += " FOR UPDATE";
        }

        unique_ptr<sql::PreparedStatement> statement(
            connection.prepareStatement(query)
        );

        (*statement).setInt(1, id);

        unique_ptr<sql::ResultSet> result(
            (*statement).executeQuery()
        );

        if (!(*result).next()) {
            return false;
        }

        car.vehicleId =
            (*result).getInt("vehicle_id");

        car.make =
            string((*result).getString("make").c_str());

        car.model =
            string((*result).getString("model").c_str());

        car.status =
            string((*result).getString("status").c_str());

        car.rate =
            (*result).getDouble("daily_rate");

        return true;
    }

    // The booking transaction locks the vehicle row.
    // Use a locking reservation read there as well,
    // so it sees the latest committed reservations.

    bool hasConflictingReservation(
        sql::Connection& connection,
        int vehicleId,
        const Dates& dates,
        bool lockingRead
    ) {
        string query =
            "SELECT reservation_id "
            "FROM reservations "
            "WHERE vehicle_id = ? "
            "AND status <> 'Cancelled' "
            "AND start_date <= ? "
            "AND end_date >= ? "
            "LIMIT 1";

        if (lockingRead) {
            query += " FOR UPDATE";
        }

        unique_ptr<sql::PreparedStatement> statement(
            connection.prepareStatement(query)
        );

        (*statement).setInt(1, vehicleId);
        (*statement).setString(2, dates.end);
        (*statement).setString(3, dates.start);

        unique_ptr<sql::ResultSet> result(
            (*statement).executeQuery()
        );

        return (*result).next();
    }

    double calculateTotal(
        double dailyRate,
        int days
    ) {
        return round(
            dailyRate * static_cast<double>(days) * 100.0
        ) / 100.0;
    }

    void ensureCustomerExists(
        sql::Connection& connection,
        int customerId
    ) {
        unique_ptr<sql::PreparedStatement> statement(
            connection.prepareStatement(
                "SELECT customer_id FROM customers "
                "WHERE customer_id = ?"
            )
        );

        (*statement).setInt(1, customerId);

        unique_ptr<sql::ResultSet> result(
            (*statement).executeQuery()
        );

        if (!(*result).next()) {
            throw runtime_error(
                "Customer account not found"
            );
        }
    }

} // namespace


// =====================================
// REGISTER BOOKING API
// =====================================

void registerBookingRoutes(
    crow::SimpleApp& app,
    const string& mysqlPassword
) {
    auto password =
        make_shared<const string>(mysqlPassword);


    // CUSTOMER WEBSITE FILES

    app.static_file(
        "/vehicle.html",
        "vehicle.html"
    );

    app.static_file(
        "/vehicle.js",
        "vehicle.js"
    );

    app.static_file(
        "/vehicle.css",
        "vehicle.css"
    );

    app.static_file(
        "/manager-reservations.js",
        "manager-reservations.js"
    );


    // =====================================
    // GET RENTAL QUOTE
    // GET /api/vehicles/1/quote
    // =====================================

    CROW_ROUTE(app, "/api/vehicles/<int>/quote")
        .methods(crow::HTTPMethod::Get)
        ([password](
            const crow::request& request,
            int vehicleId
            ) {
                if (vehicleId <= 0) {
                    return errorReply(
                        400, "Invalid vehicle ID"
                    );
                }

                const char* startRaw =
                    request.url_params.get("startDate");

                const char* endRaw =
                    request.url_params.get("endDate");

                if (!startRaw || !endRaw) {
                    return errorReply(
                        400, "Choose pickup and return dates"
                    );
                }

                Dates dates;

                try {
                    dates = checkedDates(
                        startRaw, endRaw
                    );
                }
                catch (const invalid_argument& error) {
                    return errorReply(
                        400, error.what()
                    );
                }

                try {
                    auto connection =
                        openDatabase(*password);

                    checkNotPast(dates, *connection);

                    VehicleInfo car;

                    if (!readVehicle(
                        *connection,
                        vehicleId,
                        car,
                        false
                    )) {
                        return errorReply(
                            404, "Vehicle not found"
                        );
                    }

                    bool available =
                        car.status == "Available" &&
                        !hasConflictingReservation(
                            *connection,
                            vehicleId,
                            dates,
                            false
                        );

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["vehicleId"] = vehicleId;
                    response["days"] = dates.days;
                    response["dailyRate"] = car.rate;

                    response["total"] =
                        calculateTotal(
                            car.rate, dates.days
                        );

                    response["available"] = available;

                    response["message"] =
                        available
                        ? "Dates available"
                        : "Vehicle unavailable for these dates";

                    return jsonReply(200, response);
                }
                catch (const invalid_argument& error) {
                    return errorReply(
                        400, error.what()
                    );
                }
                catch (const exception& error) {
                    cerr << "Quote error: "
                        << error.what() << endl;

                    return errorReply(
                        500,
                        "Unable to check availability"
                    );
                }
            });


    // =====================================
    // CREATE RESERVATION
    // POST /api/bookings
    // =====================================

    CROW_ROUTE(app, "/api/bookings")
        .methods(crow::HTTPMethod::Post)
        ([password](const crow::request& request) {

        if (!authorizeWriteOrigin(request)) {
            return errorReply(
                403, "Forbidden"
            );
        }

        int customerId = 0;

        if (!getLoggedInCustomerId(
            request, customerId
        )) {
            return errorReply(
                401,
                "Please log in before reserving"
            );
        }

        if (request.body.size() > 4096) {
            return errorReply(
                413, "Request too large"
            );
        }

        int vehicleId = 0;
        Dates dates;
        string paymentMethod;

        try {
            auto body =
                crow::json::load(request.body);

            if (!body ||
                body.t() != crow::json::type::Object ||
                !body.has("vehicleId") ||
                !body.has("startDate") ||
                !body.has("endDate") ||
                !body.has("paymentMethod") ||
                body["vehicleId"].t() !=
                crow::json::type::Number ||
                body["startDate"].t() !=
                crow::json::type::String ||
                body["endDate"].t() !=
                crow::json::type::String ||
                body["paymentMethod"].t() !=
                crow::json::type::String) {

                return errorReply(
                    400, "Invalid booking information"
                );
            }

            double numericId =
                body["vehicleId"].d();

            if (!isfinite(numericId) ||
                numericId < 1 ||
                numericId > 100000000 ||
                floor(numericId) != numericId) {

                return errorReply(
                    400, "Invalid vehicle ID"
                );
            }

            vehicleId =
                static_cast<int>(numericId);

            dates = checkedDates(
                string(body["startDate"].s()),
                string(body["endDate"].s())
            );

            paymentMethod =
                string(body["paymentMethod"].s());

            if (paymentMethod != "CashOnPickup" &&
                paymentMethod != "CardOnPickup") {

                return errorReply(
                    400,
                    "Choose cash or card at pickup"
                );
            }
        }
        catch (const exception& error) {
            return errorReply(
                400, error.what()
            );
        }

        unique_ptr<sql::Connection> connection;
        bool inTransaction = false;

        try {
            connection =
                openDatabase(*password);

            (*connection).setAutoCommit(false);
            inTransaction = true;

            ensureCustomerExists(
                *connection, customerId
            );

            checkNotPast(
                dates, *connection
            );

            // Lock the vehicle before checking
            // for conflicting reservations.

            VehicleInfo car;

            if (!readVehicle(
                *connection,
                vehicleId,
                car,
                true
            )) {
                (*connection).rollback();

                return errorReply(
                    404, "Vehicle not found"
                );
            }

            if (car.status != "Available" ||
                hasConflictingReservation(
                    *connection,
                    vehicleId,
                    dates,
                    true
                )) {

                (*connection).rollback();

                return errorReply(
                    409,
                    "Vehicle is no longer available"
                );
            }

            double total =
                calculateTotal(
                    car.rate, dates.days
                );

            // Insert reservation.

            unique_ptr<sql::PreparedStatement> create(
                (*connection).prepareStatement(
                    "INSERT INTO reservations "
                    "(customer_id, vehicle_id, "
                    "start_date, end_date, "
                    "total_cost, status) "
                    "VALUES (?, ?, ?, ?, ?, 'Confirmed')"
                )
            );

            (*create).setInt(1, customerId);
            (*create).setInt(2, vehicleId);
            (*create).setString(3, dates.start);
            (*create).setString(4, dates.end);
            (*create).setDouble(5, total);

            (*create).executeUpdate();

            // Obtain new reservation ID.

            unique_ptr<sql::Statement> idStatement(
                (*connection).createStatement()
            );

            unique_ptr<sql::ResultSet> idResult(
                (*idStatement).executeQuery(
                    "SELECT LAST_INSERT_ID() AS booking_id"
                )
            );

            if (!(*idResult).next()) {
                throw runtime_error(
                    "Unable to retrieve reservation ID"
                );
            }

            int bookingId =
                (*idResult).getInt("booking_id");

            // Save payment preference.

            unique_ptr<sql::PreparedStatement> payment(
                (*connection).prepareStatement(
                    "INSERT INTO "
                    "reservation_payment_preferences "
                    "(reservation_id, payment_method, "
                    "payment_status) "
                    "VALUES (?, ?, 'DueAtPickup')"
                )
            );

            (*payment).setInt(1, bookingId);
            (*payment).setString(2, paymentMethod);

            (*payment).executeUpdate();

            (*connection).commit();
            inTransaction = false;

            crow::json::wvalue response;

            response["status"] = "success";
            response["reservationId"] = bookingId;
            response["vehicleId"] = vehicleId;
            response["startDate"] = dates.start;
            response["endDate"] = dates.end;
            response["days"] = dates.days;
            response["dailyRate"] = car.rate;
            response["total"] = total;
            response["reservationStatus"] = "Confirmed";
            response["paymentMethod"] = paymentMethod;
            response["paymentStatus"] = "DueAtPickup";

            response["message"] =
                "Reservation confirmed. "
                "Payment is due at pickup.";

            return jsonReply(201, response);
        }
        catch (const exception& error) {

            if (inTransaction && connection) {
                try {
                    (*connection).rollback();
                }
                catch (...) {}
            }

            cerr << "Create booking error: "
                << error.what() << endl;

            return errorReply(
                500,
                "Unable to create reservation"
            );
        }
            });


    // =====================================
    // CUSTOMER'S OWN RESERVATIONS
    // GET /api/bookings/my
    // =====================================

    CROW_ROUTE(app, "/api/bookings/my")
        .methods(crow::HTTPMethod::Get)
        ([password](const crow::request& request) {

        int customerId = 0;

        if (!getLoggedInCustomerId(
            request, customerId
        )) {
            return errorReply(
                401,
                "Please log in to view reservations"
            );
        }

        try {
            auto connection =
                openDatabase(*password);

            unique_ptr<sql::PreparedStatement> statement(
                (*connection).prepareStatement(
                    "SELECT "
                    "r.reservation_id, r.vehicle_id, "
                    "r.start_date, r.end_date, "
                    "r.total_cost, r.status, "
                    "v.make, v.model, v.year, "
                    "COALESCE(p.payment_method, 'Legacy') "
                    "AS method, "
                    "COALESCE(p.payment_status, "
                    "'NotRecorded') AS payment_status "
                    "FROM reservations r "
                    "JOIN vehicles v "
                    "ON v.vehicle_id = r.vehicle_id "
                    "LEFT JOIN reservation_payment_preferences p "
                    "ON p.reservation_id = r.reservation_id "
                    "WHERE r.customer_id = ? "
                    "ORDER BY r.reservation_id DESC"
                )
            );

            (*statement).setInt(1, customerId);

            unique_ptr<sql::ResultSet> result(
                (*statement).executeQuery()
            );

            crow::json::wvalue::list bookings;

            while ((*result).next()) {

                crow::json::wvalue item;

                item["reservationId"] =
                    (*result).getInt("reservation_id");

                item["vehicleId"] =
                    (*result).getInt("vehicle_id");

                item["make"] =
                    string((*result).getString("make").c_str());

                item["model"] =
                    string((*result).getString("model").c_str());

                item["year"] =
                    (*result).getInt("year");

                item["startDate"] =
                    string(
                        (*result).getString("start_date").c_str()
                    ).substr(0, 10);

                item["endDate"] =
                    string(
                        (*result).getString("end_date").c_str()
                    ).substr(0, 10);

                item["total"] =
                    static_cast<double>(
                        (*result).getDouble("total_cost")
                        );

                item["reservationStatus"] =
                    string((*result).getString("status").c_str());

                item["paymentMethod"] =
                    string((*result).getString("method").c_str());

                item["paymentStatus"] =
                    string(
                        (*result).getString("payment_status").c_str()
                    );

                bookings.push_back(move(item));
            }

            crow::json::wvalue response;

            response["status"] = "success";
            response["bookings"] = move(bookings);

            return jsonReply(200, response);
        }
        catch (const exception& error) {

            cerr << "List bookings error: "
                << error.what() << endl;

            return errorReply(
                500,
                "Unable to retrieve reservations"
            );
        }
            });


    // =====================================
    // CUSTOMER CANCEL RESERVATION
    // POST /api/bookings/ID/cancel
    // =====================================

    CROW_ROUTE(app, "/api/bookings/<int>/cancel")
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int bookingId
            ) {
                if (!authorizeWriteOrigin(request)) {
                    return errorReply(403, "Forbidden");
                }

                int customerId = 0;

                if (!getLoggedInCustomerId(
                    request, customerId
                )) {
                    return errorReply(
                        401, "Please log in"
                    );
                }

                if (bookingId <= 0) {
                    return errorReply(
                        400, "Invalid reservation ID"
                    );
                }

                unique_ptr<sql::Connection> connection;
                bool inTransaction = false;

                try {
                    connection =
                        openDatabase(*password);

                    (*connection).setAutoCommit(false);
                    inTransaction = true;

                    unique_ptr<sql::PreparedStatement> statement(
                        (*connection).prepareStatement(
                            "SELECT r.status, "
                            "COALESCE(p.payment_status, "
                            "'NotRecorded') AS payment_status "
                            "FROM reservations r "
                            "LEFT JOIN reservation_payment_preferences p "
                            "ON p.reservation_id = r.reservation_id "
                            "WHERE r.reservation_id = ? "
                            "AND r.customer_id = ? FOR UPDATE"
                        )
                    );

                    (*statement).setInt(1, bookingId);
                    (*statement).setInt(2, customerId);

                    unique_ptr<sql::ResultSet> result(
                        (*statement).executeQuery()
                    );

                    if (!(*result).next()) {

                        (*connection).rollback();

                        return errorReply(
                            404, "Reservation not found"
                        );
                    }

                    string status =
                        string((*result).getString("status").c_str());

                    string payment =
                        string(
                            (*result).getString("payment_status").c_str()
                        );

                    if (status == "Cancelled") {

                        (*connection).rollback();

                        return errorReply(
                            409, "Already cancelled"
                        );
                    }

                    if (payment == "Paid") {

                        (*connection).rollback();

                        return errorReply(
                            409,
                            "Paid bookings require staff assistance"
                        );
                    }

                    unique_ptr<sql::PreparedStatement> update(
                        (*connection).prepareStatement(
                            "UPDATE reservations "
                            "SET status = 'Cancelled' "
                            "WHERE reservation_id = ? "
                            "AND customer_id = ?"
                        )
                    );

                    (*update).setInt(1, bookingId);
                    (*update).setInt(2, customerId);

                    (*update).executeUpdate();

                    (*connection).commit();
                    inTransaction = false;

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["reservationId"] = bookingId;

                    response["message"] =
                        "Reservation cancelled";

                    return jsonReply(200, response);
                }
                catch (const exception& error) {

                    if (inTransaction && connection) {
                        try {
                            (*connection).rollback();
                        }
                        catch (...) {}
                    }

                    cerr << "Cancel booking error: "
                        << error.what() << endl;

                    return errorReply(
                        500,
                        "Unable to cancel reservation"
                    );
                }
            });


    // =====================================
    // MANAGER: VIEW ALL RESERVATIONS
    // GET /api/manager/bookings
    // =====================================

    CROW_ROUTE(app, "/api/manager/bookings")
        .methods(crow::HTTPMethod::Get)
        ([password](const crow::request& request) {

        int managerId = 0;

        if (!authorizeManager(
            request, *password, managerId
        )) {
            return errorReply(
                403, "Manager access required"
            );
        }

        try {
            auto connection =
                openDatabase(*password);

            unique_ptr<sql::Statement> statement(
                (*connection).createStatement()
            );

            unique_ptr<sql::ResultSet> result(
                (*statement).executeQuery(
                    "SELECT "
                    "r.reservation_id, r.vehicle_id, "
                    "r.start_date, r.end_date, "
                    "r.total_cost, r.status, "
                    "c.name AS customer_name, "
                    "v.make, v.model, v.year, "
                    "COALESCE(p.payment_status, "
                    "'NotRecorded') AS payment_status "
                    "FROM reservations r "
                    "JOIN customers c "
                    "ON c.customer_id = r.customer_id "
                    "JOIN vehicles v "
                    "ON v.vehicle_id = r.vehicle_id "
                    "LEFT JOIN reservation_payment_preferences p "
                    "ON p.reservation_id = r.reservation_id "
                    "ORDER BY r.reservation_id DESC "
                    "LIMIT 500"
                )
            );

            crow::json::wvalue::list bookings;

            while ((*result).next()) {

                crow::json::wvalue item;

                item["reservationId"] =
                    (*result).getInt("reservation_id");

                item["vehicleId"] =
                    (*result).getInt("vehicle_id");

                item["customerName"] =
                    string(
                        (*result).getString("customer_name").c_str()
                    );

                item["make"] =
                    string((*result).getString("make").c_str());

                item["model"] =
                    string((*result).getString("model").c_str());

                item["year"] =
                    (*result).getInt("year");

                item["startDate"] =
                    string(
                        (*result).getString("start_date").c_str()
                    ).substr(0, 10);

                item["endDate"] =
                    string(
                        (*result).getString("end_date").c_str()
                    ).substr(0, 10);

                item["total"] =
                    static_cast<double>(
                        (*result).getDouble("total_cost")
                        );

                item["reservationStatus"] =
                    string((*result).getString("status").c_str());

                item["paymentStatus"] =
                    string(
                        (*result).getString("payment_status").c_str()
                    );

                bookings.push_back(move(item));
            }

            crow::json::wvalue response;

            response["status"] = "success";
            response["bookings"] = move(bookings);

            return jsonReply(200, response);
        }
        catch (const exception& error) {

            cerr << "Manager booking list error: "
                << error.what() << endl;

            return errorReply(
                500, "Unable to retrieve reservations"
            );
        }
            });


    // =====================================
    // MANAGER: RECORD PAYMENT AT PICKUP
    // POST /api/manager/bookings/ID/paid
    // =====================================

    CROW_ROUTE(app, "/api/manager/bookings/<int>/paid")
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int bookingId
            ) {
                if (!authorizeWriteOrigin(request)) {
                    return errorReply(403, "Forbidden");
                }

                int managerId = 0;

                if (!authorizeManager(
                    request, *password, managerId
                )) {
                    return errorReply(
                        403, "Manager access required"
                    );
                }

                if (bookingId <= 0) {
                    return errorReply(
                        400, "Invalid reservation ID"
                    );
                }

                unique_ptr<sql::Connection> connection;
                bool inTransaction = false;

                try {
                    connection =
                        openDatabase(*password);

                    (*connection).setAutoCommit(false);
                    inTransaction = true;

                    unique_ptr<sql::PreparedStatement> inspect(
                        (*connection).prepareStatement(
                            "SELECT r.status, p.payment_status "
                            "FROM reservations r "
                            "JOIN reservation_payment_preferences p "
                            "ON p.reservation_id = r.reservation_id "
                            "WHERE r.reservation_id = ? FOR UPDATE"
                        )
                    );

                    (*inspect).setInt(1, bookingId);

                    unique_ptr<sql::ResultSet> result(
                        (*inspect).executeQuery()
                    );

                    if (!(*result).next()) {

                        (*connection).rollback();

                        return errorReply(
                            404,
                            "Reservation or payment record not found"
                        );
                    }

                    string status =
                        string((*result).getString("status").c_str());

                    string paymentStatus =
                        string(
                            (*result).getString("payment_status").c_str()
                        );

                    if (status != "Confirmed" ||
                        paymentStatus != "DueAtPickup") {

                        (*connection).rollback();

                        return errorReply(
                            409,
                            "Only confirmed unpaid bookings can be marked paid"
                        );
                    }

                    unique_ptr<sql::PreparedStatement> update(
                        (*connection).prepareStatement(
                            "UPDATE reservation_payment_preferences "
                            "SET payment_status = 'Paid', "
                            "paid_at = NOW(), "
                            "paid_by_manager = ? "
                            "WHERE reservation_id = ? "
                            "AND payment_status = 'DueAtPickup'"
                        )
                    );

                    (*update).setInt(1, managerId);
                    (*update).setInt(2, bookingId);

                    if ((*update).executeUpdate() != 1) {
                        throw runtime_error(
                            "Unable to update payment record"
                        );
                    }

                    (*connection).commit();
                    inTransaction = false;

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["reservationId"] = bookingId;
                    response["message"] =
                        "Payment recorded as received at pickup";

                    return jsonReply(200, response);
                }
                catch (const exception& error) {

                    if (inTransaction && connection) {
                        try {
                            (*connection).rollback();
                        }
                        catch (...) {}
                    }

                    cerr << "Payment status error: "
                        << error.what() << endl;

                    return errorReply(
                        500,
                        "Unable to record payment"
                    );
                }
            });


    // =====================================
    // MANAGER: CANCEL RESERVATION
    // POST /api/manager/bookings/ID/cancel
    // =====================================

    CROW_ROUTE(app, "/api/manager/bookings/<int>/cancel")
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int bookingId
            ) {
                if (!authorizeWriteOrigin(request)) {
                    return errorReply(403, "Forbidden");
                }

                int managerId = 0;

                if (!authorizeManager(
                    request, *password, managerId
                )) {
                    return errorReply(
                        403, "Manager access required"
                    );
                }

                if (bookingId <= 0) {
                    return errorReply(
                        400, "Invalid reservation ID"
                    );
                }

                unique_ptr<sql::Connection> connection;
                bool inTransaction = false;

                try {
                    connection =
                        openDatabase(*password);

                    (*connection).setAutoCommit(false);
                    inTransaction = true;

                    unique_ptr<sql::PreparedStatement> inspect(
                        (*connection).prepareStatement(
                            "SELECT r.status, "
                            "COALESCE(p.payment_status, "
                            "'NotRecorded') AS payment_status "
                            "FROM reservations r "
                            "LEFT JOIN reservation_payment_preferences p "
                            "ON p.reservation_id = r.reservation_id "
                            "WHERE r.reservation_id = ? FOR UPDATE"
                        )
                    );

                    (*inspect).setInt(1, bookingId);

                    unique_ptr<sql::ResultSet> result(
                        (*inspect).executeQuery()
                    );

                    if (!(*result).next()) {

                        (*connection).rollback();

                        return errorReply(
                            404, "Reservation not found"
                        );
                    }

                    string status =
                        string((*result).getString("status").c_str());

                    string paymentStatus =
                        string(
                            (*result).getString("payment_status").c_str()
                        );

                    if (status == "Cancelled" ||
                        paymentStatus == "Paid") {

                        (*connection).rollback();

                        return errorReply(
                            409,
                            "Already cancelled or refund required first"
                        );
                    }

                    unique_ptr<sql::PreparedStatement> update(
                        (*connection).prepareStatement(
                            "UPDATE reservations "
                            "SET status = 'Cancelled' "
                            "WHERE reservation_id = ?"
                        )
                    );

                    (*update).setInt(1, bookingId);

                    (*update).executeUpdate();

                    (*connection).commit();
                    inTransaction = false;

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["reservationId"] = bookingId;
                    response["message"] = "Reservation cancelled";

                    return jsonReply(200, response);
                }
                catch (const exception& error) {

                    if (inTransaction && connection) {
                        try {
                            (*connection).rollback();
                        }
                        catch (...) {}
                    }

                    cerr << "Manager cancellation error: "
                        << error.what() << endl;

                    return errorReply(
                        500,
                        "Unable to cancel reservation"
                    );
                }
            });
}