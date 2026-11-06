
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>

#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <filesystem>
#include <algorithm>
#include <cctype>

#include <crow.h>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

#include "VehicleImageRepository.h"
#include "AuthRoutes.h"
#include "ManagerRoutes.h"
#include "BookingRoutes.h"

using namespace std;

namespace fs = std::filesystem;


// ==========================================
// DATABASE CONFIGURATION
// ==========================================

const string DATABASE_HOST =
"tcp://127.0.0.1:3306";

const string DATABASE_USER =
"vehicle_app";

const string DATABASE_NAME =
"vehicle_rental_db";


// ==========================================
// MYSQL PASSWORD
// ==========================================

string dbPassword;


// ==========================================
// MYSQL CONNECTION
// ==========================================

unique_ptr<sql::Connection> connectDatabase()
{
    sql::Driver* driver =
        sql::mysql::get_driver_instance();

    unique_ptr<sql::Connection> connection(
        (*driver).connect(
            DATABASE_HOST,
            DATABASE_USER,
            dbPassword
        )
    );

    (*connection).setSchema(
        DATABASE_NAME
    );

    return connection;
}


// ==========================================
// JSON RESPONSE
// ==========================================

crow::response makeJsonResponse(
    int statusCode,
    crow::json::wvalue& data
)
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


// ==========================================
// JSON ERROR RESPONSE
// ==========================================

crow::response makeErrorResponse(
    int statusCode,
    const string& message
)
{
    crow::json::wvalue response;

    response["status"] = "error";

    response["message"] = message;

    return makeJsonResponse(
        statusCode,
        response
    );
}


// ==========================================
// GET QUERY PARAMETER
// ==========================================

string getQueryValue(
    const crow::request& request,
    const string& key
)
{
    const char* value =
        request.url_params.get(key);

    if (value == nullptr)
    {
        return "";
    }

    return string(value);
}


// ==========================================
// GET QUERY PARAMETER WITH ALIASES
// ==========================================

string getQueryValueAny(
    const crow::request& request,
    const string& first,
    const string& second
)
{
    string value =
        getQueryValue(request, first);

    if (!value.empty())
    {
        return value;
    }

    return getQueryValue(
        request,
        second
    );
}


// ==========================================
// VALIDATE NUMBER
// ==========================================

double parseNumber(
    const string& text
)
{
    size_t position = 0;

    double value =
        stod(text, &position);

    if (position != text.size() ||
        !isfinite(value))
    {
        throw invalid_argument(
            "Invalid numeric search filter"
        );
    }

    return value;
}


// ==========================================
// NORMALIZE VEHICLE IMAGE URL
// ==========================================

string getImageUrl(
    string imagePath
)
{
    if (imagePath.empty())
    {
        return "";
    }

    replace(
        imagePath.begin(),
        imagePath.end(),
        '\\',
        '/'
    );

    // Paths stored as:
    // /images/vehicles/photo.jpg

    if (imagePath.rfind(
        "/images/vehicles/", 0
    ) == 0)
    {
        return imagePath;
    }

    // Paths stored as:
    // images/vehicles/photo.jpg

    if (imagePath.rfind(
        "images/vehicles/", 0
    ) == 0)
    {
        return "/" + imagePath;
    }

    // Paths stored as a filename or a
    // local file path.

    size_t slash =
        imagePath.find_last_of('/');

    string filename =
        slash == string::npos
        ? imagePath
        : imagePath.substr(slash + 1);

    if (filename.empty())
    {
        return "";
    }

    return "/images/vehicles/" +
        filename;
}


// ==========================================
// CHECK SAFE IMAGE FILENAME
// ==========================================

bool isSafeImageFilename(
    const string& filename
)
{
    if (filename.empty() ||
        filename.size() > 200)
    {
        return false;
    }

    if (filename.find("..") != string::npos)
    {
        return false;
    }

    for (unsigned char character : filename)
    {
        if (!isalnum(character) &&
            character != '_' &&
            character != '-' &&
            character != '.')
        {
            return false;
        }
    }

    size_t dot =
        filename.find_last_of('.');

    if (dot == string::npos)
    {
        return false;
    }

    string extension =
        filename.substr(dot);

    transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(
                tolower(character)
                );
        }
    );

    return (
        extension == ".jpg" ||
        extension == ".jpeg" ||
        extension == ".png"
        );
}


// ==========================================
// ENSURE CORRECT WEB DIRECTORY
// ==========================================

void configureWebDirectory()
{
    // If the current working directory already
    // contains the website, keep it unchanged.

    if (fs::exists("index.html") &&
        fs::exists("manager.html"))
    {
        return;
    }

    // Your project folder from Visual Studio.

    fs::path webFolder =
        R"(C:\Users\hiepn\source\repos\Vehicle-Rental-System\web)";

    if (!fs::exists(
        webFolder / "index.html"
    ))
    {
        throw runtime_error(
            "Cannot find the web directory. "
            "Check Visual Studio Debugging "
            "Working Directory."
        );
    }

    fs::current_path(webFolder);

    cout << "Web directory: "
        << fs::current_path().string()
        << endl;
}


// ==========================================
// MAIN FUNCTION
// ==========================================

int main()
{
    cout << "========================================"
        << endl;

    cout << "       QuickRoute Rentals Web Server"
        << endl;

    cout << "========================================"
        << endl;


    // ======================================
    // MYSQL PASSWORD
    // ======================================

    cout << "Enter vehicle_app MySQL password: ";

    getline(
        cin,
        dbPassword
    );

    if (dbPassword.empty())
    {
        cerr << "MySQL password is required."
            << endl;

        return 1;
    }


    // ======================================
    // TEST MYSQL CONNECTION
    // ======================================

    try
    {
        unique_ptr<sql::Connection> connection =
            connectDatabase();

        cout << endl;

        cout << "Connected to "
            << DATABASE_NAME
            << " successfully!"
            << endl;
    }
    catch (const sql::SQLException& error)
    {
        cerr << "MySQL connection error: "
            << error.what()
            << endl;

        return 1;
    }
    catch (const exception& error)
    {
        cerr << "Server error: "
            << error.what()
            << endl;

        return 1;
    }


    // ======================================
    // CONFIGURE WEBSITE DIRECTORY
    // ======================================

    try
    {
        configureWebDirectory();
    }
    catch (const exception& error)
    {
        cerr << error.what() << endl;

        return 1;
    }


    // ======================================
    // CREATE CROW APPLICATION
    // ======================================

    crow::SimpleApp app;


    // ======================================
    // CUSTOMER HOMEPAGE
    // ======================================

    app.static_file(
        "/",
        "index.html"
    );

    app.static_file(
        "/index.html",
        "index.html"
    );


    // ======================================
    // CUSTOMER HOMEPAGE FILES
    // ======================================

    app.static_file(
        "/style.css",
        "style.css"
    );

    app.static_file(
        "/script.js",
        "script.js"
    );

    app.static_file(
        "/booking-card-links.js",
        "booking-card-links.js"
    );


    // ======================================
    // CUSTOMER LOGIN FILES
    // ======================================

    app.static_file(
        "/login.html",
        "login.html"
    );

    app.static_file(
        "/login.js",
        "login.js"
    );


    // ======================================
    // HOMEPAGE BACKGROUND IMAGE
    // ======================================

    app.static_file(
        "/images/car-hero.jpg",
        "images/car-hero.jpg"
    );


    // ======================================
    // SERVE VEHICLE PHOTOGRAPHS
    //
    // Example:
    // /images/vehicles/car.jpg
    // ======================================

    CROW_ROUTE(
        app,
        "/images/vehicles/<string>"
    )
        ([](
            const crow::request& request,
            crow::response& response,
            string filename
            )
            {
                (void)request;

                if (!isSafeImageFilename(filename))
                {
                    response.code = 400;

                    response.write(
                        "Invalid image filename"
                    );

                    response.end();

                    return;
                }

                string filePath =
                    "images/vehicles/" + filename;

                if (!fs::exists(filePath) ||
                    !fs::is_regular_file(filePath))
                {
                    response.code = 404;

                    response.write(
                        "Image not found"
                    );

                    response.end();

                    return;
                }

                response.set_static_file_info(
                    filePath
                );

                response.end();
            });


    // ======================================
    // API STATUS
    //
    // GET /api/status
    // ======================================

    CROW_ROUTE(
        app,
        "/api/status"
    )
        ([]()
            {
                crow::json::wvalue response;

                response["status"] = "success";

                response["message"] =
                    "QuickRoute Rentals API is running";

                return response;
            });


    // ======================================
    // VEHICLE SEARCH API
    //
    // GET /api/vehicles
    //
    // SUPPORTED FILTERS:
    // search / q
    // make
    // category
    // year
    // status
    // minPrice
    // maxPrice
    // sort
    // ======================================

    CROW_ROUTE(
        app,
        "/api/vehicles"
    )
        ([](
            const crow::request& request
            )
            {
                try
                {
                    // ==============================
                    // READ SEARCH FILTERS
                    // ==============================

                    string keyword =
                        getQueryValueAny(
                            request,
                            "search",
                            "q"
                        );

                    if (keyword.empty())
                    {
                        keyword =
                            getQueryValue(
                                request,
                                "keyword"
                            );
                    }

                    string make =
                        getQueryValue(
                            request,
                            "make"
                        );

                    string category =
                        getQueryValue(
                            request,
                            "category"
                        );

                    string yearText =
                        getQueryValue(
                            request,
                            "year"
                        );

                    string status =
                        getQueryValue(
                            request,
                            "status"
                        );

                    string minPriceText =
                        getQueryValueAny(
                            request,
                            "minPrice",
                            "min_price"
                        );

                    string maxPriceText =
                        getQueryValueAny(
                            request,
                            "maxPrice",
                            "max_price"
                        );

                    string sort =
                        getQueryValue(
                            request,
                            "sort"
                        );


                    // ==============================
                    // VALIDATE FILTERS
                    // ==============================

                    if (keyword.size() > 80 ||
                        make.size() > 50 ||
                        category.size() > 50 ||
                        status.size() > 20)
                    {
                        return makeErrorResponse(
                            400,
                            "Search value is too long"
                        );
                    }

                    int year = 0;

                    if (!yearText.empty())
                    {
                        double number =
                            parseNumber(yearText);

                        if (number < 1886 ||
                            number > 2100 ||
                            floor(number) != number)
                        {
                            return makeErrorResponse(
                                400,
                                "Invalid model year"
                            );
                        }

                        year =
                            static_cast<int>(number);
                    }

                    double minPrice = -1;

                    double maxPrice = -1;

                    if (!minPriceText.empty())
                    {
                        minPrice =
                            parseNumber(minPriceText);

                        if (minPrice < 0)
                        {
                            return makeErrorResponse(
                                400,
                                "Minimum price must be positive"
                            );
                        }
                    }

                    if (!maxPriceText.empty())
                    {
                        maxPrice =
                            parseNumber(maxPriceText);

                        if (maxPrice < 0)
                        {
                            return makeErrorResponse(
                                400,
                                "Maximum price must be positive"
                            );
                        }
                    }

                    if (minPrice >= 0 &&
                        maxPrice >= 0 &&
                        minPrice > maxPrice)
                    {
                        return makeErrorResponse(
                            400,
                            "Minimum price exceeds maximum price"
                        );
                    }


                    // ==============================
                    // BUILD MYSQL QUERY
                    // ==============================

                    string query =
                        "SELECT "
                        "v.vehicle_id, "
                        "v.make, "
                        "v.model, "
                        "v.year, "
                        "v.category, "
                        "v.daily_rate, "
                        "v.status, "

                        "COALESCE(("
                        "SELECT vi.image_path "
                        "FROM vehicle_images vi "
                        "WHERE vi.vehicle_id = v.vehicle_id "
                        "ORDER BY "
                        "vi.is_primary DESC, "
                        "vi.image_order ASC, "
                        "vi.image_id ASC "
                        "LIMIT 1"
                        "), '') AS image_path "

                        "FROM vehicles v "
                        "WHERE 1 = 1 ";


                    // ==============================
                    // KEYWORD SEARCH
                    // ==============================

                    if (!keyword.empty())
                    {
                        query +=
                            "AND LOWER(CONCAT("
                            "v.make, ' ', "
                            "v.model, ' ', "
                            "v.category, ' ', "
                            "v.year"
                            ")) LIKE LOWER(?) ";
                    }


                    // ==============================
                    // MAKE FILTER
                    // ==============================

                    if (!make.empty())
                    {
                        query +=
                            "AND v.make = ? ";
                    }


                    // ==============================
                    // CATEGORY FILTER
                    // ==============================

                    if (!category.empty())
                    {
                        query +=
                            "AND v.category = ? ";
                    }


                    // ==============================
                    // YEAR FILTER
                    // ==============================

                    if (!yearText.empty())
                    {
                        query +=
                            "AND v.year = ? ";
                    }


                    // ==============================
                    // STATUS FILTER
                    // ==============================

                    if (!status.empty())
                    {
                        query +=
                            "AND v.status = ? ";
                    }


                    // ==============================
                    // MINIMUM PRICE
                    // ==============================

                    if (minPrice >= 0)
                    {
                        query +=
                            "AND v.daily_rate >= ? ";
                    }


                    // ==============================
                    // MAXIMUM PRICE
                    // ==============================

                    if (maxPrice >= 0)
                    {
                        query +=
                            "AND v.daily_rate <= ? ";
                    }


                    // ==============================
                    // SORT RESULTS
                    //
                    // SQL SORT COLUMNS ARE
                    // WHITELISTED, NOT USER INPUT.
                    // ==============================

                    if (sort == "price_asc" ||
                        sort == "daily_rate_asc")
                    {
                        query +=
                            "ORDER BY v.daily_rate ASC, "
                            "v.vehicle_id ASC";
                    }
                    else if (sort == "price_desc" ||
                        sort == "daily_rate_desc")
                    {
                        query +=
                            "ORDER BY v.daily_rate DESC, "
                            "v.vehicle_id ASC";
                    }
                    else if (sort == "year_desc")
                    {
                        query +=
                            "ORDER BY v.year DESC, "
                            "v.vehicle_id ASC";
                    }
                    else if (sort == "year_asc")
                    {
                        query +=
                            "ORDER BY v.year ASC, "
                            "v.vehicle_id ASC";
                    }
                    else if (sort == "make_asc")
                    {
                        query +=
                            "ORDER BY v.make ASC, "
                            "v.model ASC, "
                            "v.vehicle_id ASC";
                    }
                    else if (sort == "make_desc")
                    {
                        query +=
                            "ORDER BY v.make DESC, "
                            "v.model ASC, "
                            "v.vehicle_id ASC";
                    }
                    else
                    {
                        query +=
                            "ORDER BY v.vehicle_id ASC";
                    }


                    // ==============================
                    // CONNECT MYSQL
                    // ==============================

                    unique_ptr<sql::Connection> connection =
                        connectDatabase();


                    // ==============================
                    // PREPARE SQL QUERY
                    // ==============================

                    unique_ptr<sql::PreparedStatement> statement(
                        (*connection).prepareStatement(
                            query
                        )
                    );


                    // ==============================
                    // BIND SEARCH PARAMETERS
                    // ==============================

                    int parameter = 1;

                    if (!keyword.empty())
                    {
                        (*statement).setString(
                            parameter++,
                            "%" + keyword + "%"
                        );
                    }

                    if (!make.empty())
                    {
                        (*statement).setString(
                            parameter++,
                            make
                        );
                    }

                    if (!category.empty())
                    {
                        (*statement).setString(
                            parameter++,
                            category
                        );
                    }

                    if (!yearText.empty())
                    {
                        (*statement).setInt(
                            parameter++,
                            year
                        );
                    }

                    if (!status.empty())
                    {
                        (*statement).setString(
                            parameter++,
                            status
                        );
                    }

                    if (minPrice >= 0)
                    {
                        (*statement).setDouble(
                            parameter++,
                            minPrice
                        );
                    }

                    if (maxPrice >= 0)
                    {
                        (*statement).setDouble(
                            parameter++,
                            maxPrice
                        );
                    }


                    // ==============================
                    // EXECUTE QUERY
                    // ==============================

                    unique_ptr<sql::ResultSet> result(
                        (*statement).executeQuery()
                    );


                    // ==============================
                    // CONVERT VEHICLES TO JSON
                    // ==============================

                    crow::json::wvalue::list vehicles;

                    int count = 0;

                    while ((*result).next())
                    {
                        crow::json::wvalue vehicle;


                        // VEHICLE ID

                        vehicle["id"] =
                            (*result).getInt(
                                "vehicle_id"
                            );


                        // MAKE

                        vehicle["make"] =
                            string(
                                (*result).getString(
                                    "make"
                                ).c_str()
                            );


                        // MODEL

                        vehicle["model"] =
                            string(
                                (*result).getString(
                                    "model"
                                ).c_str()
                            );


                        // YEAR

                        vehicle["year"] =
                            (*result).getInt(
                                "year"
                            );


                        // CATEGORY

                        vehicle["category"] =
                            string(
                                (*result).getString(
                                    "category"
                                ).c_str()
                            );


                        // DAILY RATE

                        vehicle["dailyRate"] =
                            static_cast<double>(
                                (*result).getDouble(
                                    "daily_rate"
                                )
                                );


                        // VEHICLE STATUS

                        vehicle["status"] =
                            string(
                                (*result).getString(
                                    "status"
                                ).c_str()
                            );


                        // PRIMARY PHOTOGRAPH

                        string imagePath =
                            string(
                                (*result).getString(
                                    "image_path"
                                ).c_str()
                            );

                        vehicle["imageUrl"] =
                            getImageUrl(
                                imagePath
                            );


                        // ADD VEHICLE TO JSON LIST

                        vehicles.push_back(
                            move(vehicle)
                        );

                        count++;
                    }


                    // ==============================
                    // SUCCESS RESPONSE
                    // ==============================

                    crow::json::wvalue response;

                    response["status"] =
                        "success";

                    response["count"] =
                        count;

                    response["vehicles"] =
                        move(vehicles);


                    cout << "Vehicles loaded from MySQL: "
                        << count
                        << endl;


                    return makeJsonResponse(
                        200,
                        response
                    );

                }
                catch (const invalid_argument& error)
                {
                    return makeErrorResponse(
                        400,
                        error.what()
                    );
                }
                catch (const sql::SQLException& error)
                {
                    cerr << "MySQL vehicle search error: "
                        << error.what()
                        << endl;

                    return makeErrorResponse(
                        500,
                        "Unable to retrieve vehicle data"
                    );
                }
                catch (const exception& error)
                {
                    cerr << "Vehicle search error: "
                        << error.what()
                        << endl;

                    return makeErrorResponse(
                        500,
                        "An internal server error occurred"
                    );
                }
            });


    // ======================================
    // VEHICLE IMAGE GALLERY API
    //
    // GET /api/vehicles/1/images
    // ======================================

    CROW_ROUTE(
        app,
        "/api/vehicles/<int>/images"
    )
        ([](int vehicleId)
            {
                if (vehicleId <= 0)
                {
                    return makeErrorResponse(
                        400,
                        "Invalid vehicle ID"
                    );
                }

                try
                {
                    unique_ptr<sql::Connection> connection =
                        connectDatabase();


                    // ==============================
                    // VERIFY VEHICLE EXISTS
                    // ==============================

                    unique_ptr<sql::PreparedStatement> checkVehicle(
                        (*connection).prepareStatement(
                            "SELECT vehicle_id "
                            "FROM vehicles "
                            "WHERE vehicle_id = ?"
                        )
                    );

                    (*checkVehicle).setInt(
                        1,
                        vehicleId
                    );

                    unique_ptr<sql::ResultSet> vehicleResult(
                        (*checkVehicle).executeQuery()
                    );

                    if (!(*vehicleResult).next())
                    {
                        return makeErrorResponse(
                            404,
                            "Vehicle not found"
                        );
                    }


                    // ==============================
                    // GET VEHICLE IMAGES
                    // ==============================

                    unique_ptr<sql::PreparedStatement> statement(
                        (*connection).prepareStatement(
                            "SELECT "
                            "image_id, "
                            "image_path, "
                            "image_order, "
                            "is_primary "

                            "FROM vehicle_images "

                            "WHERE vehicle_id = ? "

                            "ORDER BY "
                            "is_primary DESC, "
                            "image_order ASC, "
                            "image_id ASC"
                        )
                    );

                    (*statement).setInt(
                        1,
                        vehicleId
                    );


                    unique_ptr<sql::ResultSet> result(
                        (*statement).executeQuery()
                    );


                    // ==============================
                    // BUILD IMAGE JSON LIST
                    // ==============================

                    crow::json::wvalue::list images;

                    int count = 0;

                    while ((*result).next())
                    {
                        crow::json::wvalue image;


                        image["imageId"] =
                            (*result).getInt(
                                "image_id"
                            );


                        image["imageUrl"] =
                            getImageUrl(
                                string(
                                    (*result).getString(
                                        "image_path"
                                    ).c_str()
                                )
                            );


                        image["imageOrder"] =
                            (*result).getInt(
                                "image_order"
                            );


                        image["isPrimary"] =
                            (*result).getInt(
                                "is_primary"
                            ) != 0;


                        images.push_back(
                            move(image)
                        );

                        count++;
                    }


                    // ==============================
                    // SUCCESS RESPONSE
                    // ==============================

                    crow::json::wvalue response;

                    response["status"] =
                        "success";

                    response["vehicleId"] =
                        vehicleId;

                    response["count"] =
                        count;

                    response["images"] =
                        move(images);


                    return makeJsonResponse(
                        200,
                        response
                    );

                }
                catch (const sql::SQLException& error)
                {
                    cerr << "MySQL image error: "
                        << error.what()
                        << endl;

                    return makeErrorResponse(
                        500,
                        "Unable to retrieve vehicle images"
                    );
                }
                catch (const exception& error)
                {
                    cerr << "Vehicle image error: "
                        << error.what()
                        << endl;

                    return makeErrorResponse(
                        500,
                        "An internal server error occurred"
                    );
                }
            });


    // ======================================
    // CUSTOMER LOGIN AND REGISTRATION
    // ======================================

    registerAuthRoutes(
        app,
        dbPassword
    );


    // ======================================
    // MANAGER VEHICLE MANAGEMENT
    //
    // ADD VEHICLES
    // EDIT VEHICLES
    // UPDATE PRICES
    // UPLOAD PHOTOS
    // CHANGE MAINTENANCE STATUS
    // ======================================

    registerManagerRoutes(
        app,
        dbPassword
    );


    // ======================================
    // CUSTOMER RESERVATION SYSTEM
    //
    // THIS IS THE MISSING REGISTRATION!
    //
    // VEHICLE DETAIL PAGE
    // RENTAL DATE AVAILABILITY
    // RENTAL PRICE CALCULATION
    // CUSTOMER BOOKINGS
    // CANCELLATIONS
    // MANAGER RESERVATIONS
    // PAYMENT AT PICKUP
    // ======================================

    registerBookingRoutes(
        app,
        dbPassword
    );


    // ======================================
    // START QUICKROUTE SERVER
    // ======================================

    cout << endl;

    cout << "========================================"
        << endl;

    cout << "   QuickRoute Rentals Web Server Ready"
        << endl;

    cout << "========================================"
        << endl;


    cout << "MySQL authentication successful!"
        << endl;


    cout << endl;

    cout << "Customer Homepage:"
        << endl;

    cout << "http://localhost:18080/"
        << endl;


    cout << endl;

    cout << "Vehicle Search API:"
        << endl;

    cout << "http://localhost:18080/api/vehicles"
        << endl;


    cout << endl;

    cout << "Customer Login:"
        << endl;

    cout << "http://localhost:18080/login.html"
        << endl;


    cout << endl;

    cout << "Manager Dashboard:"
        << endl;

    cout << "http://localhost:18080/manager.html"
        << endl;


    cout << endl;

    cout << "Vehicle Reservation Page:"
        << endl;

    cout << "http://localhost:18080/vehicle.html?id=1"
        << endl;


    cout << endl;

    cout << "Reservation Availability API:"
        << endl;

    cout << "http://localhost:18080/api/vehicles/1/quote"
        << "?startDate=2026-12-01"
        << "&endDate=2026-12-03"
        << endl;


    cout << endl;

    cout << "Starting C++ Crow HTTP server..."
        << endl;

    cout << endl;


    // Run only on this computer.

    app.bindaddr(
        "127.0.0.1"
    )
        .port(
            18080
        )
        .run();


    return 0;
}
