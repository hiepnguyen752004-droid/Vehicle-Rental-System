
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

#include <crow.h>
#include <crow/multipart.h>

#include "ManagerRoutes.h"
#include "AuthRoutes.h"

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <filesystem>
#include <array>
#include <stdexcept>
#include <cmath>
#include <utility>
#include <system_error>

using namespace std;
namespace fs = std::filesystem;

namespace {

    const size_t MAX_IMAGE = 3 * 1024 * 1024;
    const size_t MAX_REQUEST = 16 * 1024 * 1024;
    const size_t MAX_PHOTOS = 5;

    const fs::path PHOTO_FOLDER =
        fs::path("images") / "vehicles";

    struct Photo {
        string bytes;
        string extension;
    };

    struct UploadForm {
        map<string, string> fields;
        vector<Photo> photos;
    };

    struct VehicleData {
        string make;
        string model;
        string category;
        string status;
        int year = 0;
        double dailyRate = 0;
    };


    // =====================================
    // JSON RESPONSES
    // =====================================

    crow::response jsonReply(
        int code,
        crow::json::wvalue& data) {

        crow::response result(code, data.dump());

        result.set_header(
            "Content-Type", "application/json"
        );

        result.set_header(
            "Cache-Control", "no-store"
        );

        return result;
    }

    crow::response fail(
        int code,
        const string& message) {

        crow::json::wvalue data;

        data["status"] = "error";
        data["message"] = message;

        return jsonReply(code, data);
    }


    // =====================================
    // MYSQL CONNECTION
    // =====================================

    unique_ptr<sql::Connection> connectDatabase(
        const string& password) {

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
    // TEXT AND FIELD HELPERS
    // =====================================

    string trim(const string& text) {

        size_t start =
            text.find_first_not_of(" \t\r\n");

        if (start == string::npos) {
            return "";
        }

        size_t end =
            text.find_last_not_of(" \t\r\n");

        return text.substr(
            start, end - start + 1
        );
    }

    string getField(
        const UploadForm& form,
        const string& key) {

        auto found = form.fields.find(key);

        if (found == form.fields.end()) {
            return "";
        }

        return found->second;
    }

    int parseInt(
        const string& text,
        int minimum,
        int maximum) {

        size_t consumed = 0;
        int value = stoi(text, &consumed);

        if (consumed != text.size() ||
            value < minimum ||
            value > maximum) {

            throw invalid_argument("Invalid number");
        }

        return value;
    }

    double parseRate(const string& text) {

        size_t consumed = 0;

        double rate = stod(text, &consumed);

        if (consumed != text.size() ||
            !isfinite(rate) ||
            rate < 0.01 ||
            rate > 10000.0) {

            throw invalid_argument(
                "Daily rate must be between $0.01 and $10,000"
            );
        }

        return rate;
    }


    // =====================================
    // VEHICLE VALIDATION
    // =====================================

    void validateVehicle(const VehicleData& data) {

        if (data.make.empty() ||
            data.model.empty() ||
            data.make.size() > 50 ||
            data.model.size() > 50) {

            throw invalid_argument(
                "Make and model are required (maximum 50 characters)"
            );
        }

        if (data.category != "Sedan" &&
            data.category != "SUV" &&
            data.category != "Hatchback" &&
            data.category != "Luxury") {

            throw invalid_argument(
                "Invalid category"
            );
        }

        if (data.status != "Available" &&
            data.status != "Maintenance") {

            throw invalid_argument(
                "Invalid vehicle status"
            );
        }

        if (data.year < 1980 ||
            data.year > 2100) {

            throw invalid_argument(
                "Year must be between 1980 and 2100"
            );
        }

        if (!isfinite(data.dailyRate) ||
            data.dailyRate < 0.01 ||
            data.dailyRate > 10000.0) {

            throw invalid_argument(
                "Invalid daily rental rate"
            );
        }
    }


    VehicleData fromForm(const UploadForm& form) {

        VehicleData data;

        data.make = trim(getField(form, "make"));
        data.model = trim(getField(form, "model"));
        data.category = getField(form, "category");
        data.status = getField(form, "status");

        data.year = parseInt(
            getField(form, "year"), 1980, 2100
        );

        data.dailyRate = parseRate(
            getField(form, "dailyRate")
        );

        validateVehicle(data);

        return data;
    }


    VehicleData fromJson(
        const crow::request& request) {

        if (request.body.empty() ||
            request.body.size() > 4096) {

            throw invalid_argument(
                "Invalid update request size"
            );
        }

        auto json = crow::json::load(request.body);

        if (!json ||
            json.t() != crow::json::type::Object ||
            !json.has("make") ||
            !json.has("model") ||
            !json.has("year") ||
            !json.has("category") ||
            !json.has("dailyRate") ||
            !json.has("status")) {

            throw invalid_argument(
                "Missing vehicle information"
            );
        }

        if (json["make"].t() != crow::json::type::String ||
            json["model"].t() != crow::json::type::String ||
            json["category"].t() != crow::json::type::String ||
            json["status"].t() != crow::json::type::String ||
            json["year"].t() != crow::json::type::Number ||
            json["dailyRate"].t() != crow::json::type::Number) {

            throw invalid_argument(
                "Invalid vehicle field types"
            );
        }

        VehicleData data;

        data.make = trim(
            string(json["make"].s())
        );

        data.model = trim(
            string(json["model"].s())
        );

        data.category =
            string(json["category"].s());

        data.status =
            string(json["status"].s());

        double yearNumber =
            json["year"].d();

        if (!isfinite(yearNumber) ||
            yearNumber < 1980 ||
            yearNumber > 2100 ||
            floor(yearNumber) != yearNumber) {

            throw invalid_argument(
                "Invalid model year"
            );
        }

        data.year = static_cast<int>(yearNumber);

        data.dailyRate =
            json["dailyRate"].d();

        validateVehicle(data);

        return data;
    }


    // =====================================
    // IMAGE FORMAT VALIDATION
    // =====================================

    string imageExtension(const string& bytes) {

        if (bytes.size() >= 4 &&
            (unsigned char)bytes[0] == 0xFF &&
            (unsigned char)bytes[1] == 0xD8 &&
            (unsigned char)bytes[2] == 0xFF &&
            (unsigned char)bytes[bytes.size() - 2] == 0xFF &&
            (unsigned char)bytes[bytes.size() - 1] == 0xD9) {

            return ".jpg";
        }

        const string pngSignature(
            "\x89PNG\r\n\x1A\n", 8
        );

        if (bytes.size() >= 24 &&
            bytes.compare(0, 8, pngSignature) == 0 &&
            bytes.compare(12, 4, "IHDR") == 0) {

            return ".png";
        }

        return "";
    }


    // =====================================
    // MULTIPART FORM
    // =====================================

    UploadForm readMultipart(
        const crow::request& request) {

        string type =
            request.get_header_value("Content-Type");

        if (type.find("multipart/form-data") != 0) {

            throw invalid_argument(
                "Expected multipart/form-data"
            );
        }

        if (request.body.empty() ||
            request.body.size() > MAX_REQUEST) {

            throw invalid_argument(
                "Upload request is empty or too large"
            );
        }

        crow::multipart::message multipart(request);

        UploadForm form;

        for (const auto& part : multipart.parts) {

            const auto& header =
                part.get_header_object(
                    "Content-Disposition"
                );

            auto nameItem =
                header.params.find("name");

            if (nameItem == header.params.end()) {

                throw invalid_argument(
                    "Invalid upload field"
                );
            }

            string name = nameItem->second;

            if (name == "photos") {

                auto filenameItem =
                    header.params.find("filename");

                if (filenameItem ==
                    header.params.end()) {

                    throw invalid_argument(
                        "Missing image filename"
                    );
                }

                if (filenameItem->second.empty() &&
                    part.body.empty()) {

                    continue;
                }

                if (form.photos.size() >= MAX_PHOTOS) {

                    throw invalid_argument(
                        "Maximum five images per upload"
                    );
                }

                if (part.body.empty() ||
                    part.body.size() > MAX_IMAGE) {

                    throw invalid_argument(
                        "Each image must be 3 MB or less"
                    );
                }

                string extension =
                    imageExtension(part.body);

                if (extension.empty()) {

                    throw invalid_argument(
                        "Only JPEG and PNG images are supported"
                    );
                }

                Photo photo;

                photo.bytes = part.body;
                photo.extension = extension;

                form.photos.push_back(move(photo));
            }
            else {

                if (name.size() > 50 ||
                    part.body.size() > 200 ||
                    !form.fields.emplace(
                        name, part.body
                    ).second) {

                    throw invalid_argument(
                        "Invalid or duplicate form field"
                    );
                }
            }
        }

        return form;
    }


    // =====================================
    // RANDOM SAFE IMAGE NAMES
    // =====================================

    string randomHex() {

        array<unsigned char, 16> bytes{};

        NTSTATUS status = BCryptGenRandom(
            nullptr,
            bytes.data(),
            static_cast<ULONG>(bytes.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG
        );

        if (status < 0) {

            throw runtime_error(
                "Could not generate filename"
            );
        }

        const char* digits = "0123456789abcdef";

        string result;

        for (unsigned char value : bytes) {

            result += digits[value >> 4];
            result += digits[value & 15];
        }

        return result;
    }


    bool validImageName(const string& filename) {

        if (filename.rfind("vehicle_", 0) != 0 ||
            filename.size() > 128) {

            return false;
        }

        bool jpg =
            filename.size() > 4 &&
            filename.substr(filename.size() - 4) == ".jpg";

        bool png =
            filename.size() > 4 &&
            filename.substr(filename.size() - 4) == ".png";

        if (!jpg && !png) {
            return false;
        }

        if (filename.find("..") != string::npos ||
            filename.find('/') != string::npos ||
            filename.find('\\') != string::npos) {

            return false;
        }

        for (char c : filename) {

            if (!(
                (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') ||
                c == '_' || c == '.'
                )) {

                return false;
            }
        }

        return true;
    }


    // =====================================
    // IMAGE FILE STORAGE
    // =====================================

    void cleanupFiles(const vector<fs::path>& files) {

        for (const fs::path& path : files) {

            error_code error;

            fs::remove(path, error);
        }
    }


    int selectedPrimary(const UploadForm& form) {

        if (form.photos.empty()) {
            return 0;
        }

        string value = getField(
            form, "primaryIndex"
        );

        if (value.empty()) {
            return 0;
        }

        return parseInt(
            value,
            0,
            static_cast<int>(form.photos.size()) - 1
        );
    }


    bool hasPrimaryImage(
        sql::Connection& connection,
        int vehicleId) {

        unique_ptr<sql::PreparedStatement> statement(
            connection.prepareStatement(
                "SELECT COUNT(*) AS total "
                "FROM vehicle_images "
                "WHERE vehicle_id = ? "
                "AND is_primary = TRUE"
            )
        );

        statement->setInt(1, vehicleId);

        unique_ptr<sql::ResultSet> result(
            statement->executeQuery()
        );

        result->next();

        return result->getInt("total") > 0;
    }


    int nextImageOrder(
        sql::Connection& connection,
        int vehicleId) {

        unique_ptr<sql::PreparedStatement> statement(
            connection.prepareStatement(
                "SELECT COALESCE(MAX(image_order), -1) "
                "AS last_order "
                "FROM vehicle_images "
                "WHERE vehicle_id = ?"
            )
        );

        statement->setInt(1, vehicleId);

        unique_ptr<sql::ResultSet> result(
            statement->executeQuery()
        );

        result->next();

        return result->getInt("last_order") + 1;
    }


    // MUST RUN INSIDE A MYSQL TRANSACTION

    void saveImages(
        sql::Connection& connection,
        int vehicleId,
        const UploadForm& form,
        bool replacePrimary,
        int primaryIndex,
        vector<fs::path>& createdFiles) {

        if (form.photos.empty()) {
            return;
        }

        fs::create_directories(PHOTO_FOLDER);

        if (replacePrimary) {

            unique_ptr<sql::PreparedStatement> clear(
                connection.prepareStatement(
                    "UPDATE vehicle_images "
                    "SET is_primary = FALSE "
                    "WHERE vehicle_id = ?"
                )
            );

            clear->setInt(1, vehicleId);
            clear->executeUpdate();
        }

        int order = nextImageOrder(
            connection, vehicleId
        );

        unique_ptr<sql::PreparedStatement> insert(
            connection.prepareStatement(
                "INSERT INTO vehicle_images "
                "(vehicle_id, image_path, image_order, is_primary) "
                "VALUES (?, ?, ?, ?)"
            )
        );

        for (size_t i = 0; i < form.photos.size(); i++) {

            const Photo& photo = form.photos[i];

            string filename =
                "vehicle_" +
                to_string(vehicleId) +
                "_" +
                randomHex() +
                photo.extension;

            fs::path path =
                PHOTO_FOLDER / filename;

            if (fs::exists(path)) {

                throw runtime_error(
                    "Image filename collision"
                );
            }

            createdFiles.push_back(path);

            ofstream file(
                path,
                ios::binary | ios::trunc
            );

            if (!file) {

                throw runtime_error(
                    "Could not create image file"
                );
            }

            file.write(
                photo.bytes.data(),
                static_cast<streamsize>(
                    photo.bytes.size()
                    )
            );

            file.close();

            if (!file) {

                throw runtime_error(
                    "Could not save image"
                );
            }

            string imageUrl =
                "/images/vehicles/" + filename;

            bool primary =
                replacePrimary &&
                static_cast<int>(i) == primaryIndex;

            insert->setInt(1, vehicleId);
            insert->setString(2, imageUrl);
            insert->setInt(3, order++);
            insert->setInt(4, primary ? 1 : 0);

            insert->executeUpdate();
        }
    }


    // =====================================
    // LOCK VEHICLE DURING CHANGES
    // =====================================

    bool lockVehicle(
        sql::Connection& connection,
        int vehicleId) {

        unique_ptr<sql::PreparedStatement> statement(
            connection.prepareStatement(
                "SELECT vehicle_id FROM vehicles "
                "WHERE vehicle_id = ? FOR UPDATE"
            )
        );

        statement->setInt(1, vehicleId);

        unique_ptr<sql::ResultSet> result(
            statement->executeQuery()
        );

        return result->next();
    }


    // =====================================
    // GET IMAGE INFORMATION
    // =====================================

    bool findImage(
        sql::Connection& connection,
        int imageId,
        int& vehicleId,
        string& imagePath,
        bool& isPrimary) {

        unique_ptr<sql::PreparedStatement> statement(
            connection.prepareStatement(
                "SELECT vehicle_id, image_path, is_primary "
                "FROM vehicle_images WHERE image_id = ?"
            )
        );

        statement->setInt(1, imageId);

        unique_ptr<sql::ResultSet> result(
            statement->executeQuery()
        );

        if (!result->next()) {
            return false;
        }

        vehicleId = result->getInt("vehicle_id");

        imagePath =
            string(result->getString("image_path").c_str());

        isPrimary =
            result->getInt("is_primary") != 0;

        return true;
    }

}


// ==========================================
// ALL MANAGER ENDPOINTS
// ==========================================

void registerManagerRoutes(
    crow::SimpleApp& app,
    const string& mysqlPassword) {

    auto password =
        make_shared<const string>(mysqlPassword);


    // ======================================
    // SERVE VEHICLE IMAGE FILES
    // ======================================

    CROW_ROUTE(app, "/images/vehicles/<string>")
        ([](string filename) {

        if (!validImageName(filename)) {

            return crow::response(
                404, "Image not found"
            );
        }

        fs::path path =
            PHOTO_FOLDER / filename;

        if (!fs::is_regular_file(path)) {

            return crow::response(
                404, "Image not found"
            );
        }

        crow::response response;

        response.set_static_file_info(
            path.string()
        );

        response.set_header(
            "X-Content-Type-Options", "nosniff"
        );

        return response;
            });


    // ======================================
    // CREATE NEW VEHICLE + PHOTOS
    // POST /api/manager/vehicles
    // ======================================

    CROW_ROUTE(app, "/api/manager/vehicles")
        .methods(crow::HTTPMethod::Post)
        ([password](const crow::request& request) {

        if (!authorizeWriteOrigin(request)) {

            return fail(403, "Forbidden");
        }

        int managerId = 0;

        if (!authorizeManager(
            request, *password, managerId)) {

            return fail(
                403, "Manager access required"
            );
        }

        UploadForm form;
        VehicleData data;
        int primaryIndex = 0;

        try {

            form = readMultipart(request);

            data = fromForm(form);

            primaryIndex =
                selectedPrimary(form);
        }
        catch (const exception& error) {

            return fail(400, error.what());
        }

        unique_ptr<sql::Connection> connection;
        vector<fs::path> createdFiles;

        bool transaction = false;
        bool committed = false;

        try {

            connection = connectDatabase(*password);

            connection->setAutoCommit(false);
            transaction = true;

            unique_ptr<sql::PreparedStatement> statement(
                connection->prepareStatement(
                    "INSERT INTO vehicles "
                    "(make, model, year, category, daily_rate, status) "
                    "VALUES (?, ?, ?, ?, ?, ?)"
                )
            );

            statement->setString(1, data.make);
            statement->setString(2, data.model);
            statement->setInt(3, data.year);
            statement->setString(4, data.category);
            statement->setDouble(5, data.dailyRate);
            statement->setString(6, data.status);

            statement->executeUpdate();

            unique_ptr<sql::Statement> idStatement(
                connection->createStatement()
            );

            unique_ptr<sql::ResultSet> idResult(
                idStatement->executeQuery(
                    "SELECT LAST_INSERT_ID()"
                )
            );

            if (!idResult->next()) {

                throw runtime_error(
                    "Could not read new vehicle ID"
                );
            }

            int vehicleId = idResult->getInt(1);

            saveImages(
                *connection,
                vehicleId,
                form,
                true,
                primaryIndex,
                createdFiles
            );

            connection->commit();
            committed = true;
            transaction = false;

            crow::json::wvalue response;

            response["status"] = "success";
            response["vehicleId"] = vehicleId;

            response["photosUploaded"] =
                static_cast<int>(form.photos.size());

            response["message"] =
                "Vehicle created successfully";

            cout << "Manager " << managerId
                << " created vehicle " << vehicleId
                << endl;

            return jsonReply(201, response);
        }
        catch (const exception& error) {

            if (transaction && connection) {
                try { connection->rollback(); }
                catch (...) {}
            }

            if (!committed) {
                cleanupFiles(createdFiles);
            }

            cerr << "Create vehicle error: "
                << error.what() << endl;

            return fail(
                500, "Unable to create vehicle"
            );
        }
            });


    // ======================================
    // UPDATE ALL VEHICLE DETAILS
    // POST /api/manager/vehicles/ID/update
    // ======================================

    CROW_ROUTE(app, "/api/manager/vehicles/<int>/update")
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int vehicleId) {

                if (!authorizeWriteOrigin(request)) {

                    return fail(403, "Forbidden");
                }

                int managerId = 0;

                if (!authorizeManager(
                    request, *password, managerId)) {

                    return fail(
                        403, "Manager access required"
                    );
                }

                if (vehicleId <= 0) {

                    return fail(400, "Invalid vehicle ID");
                }

                VehicleData data;

                try {
                    data = fromJson(request);
                }
                catch (const exception& error) {
                    return fail(400, error.what());
                }

                try {

                    auto connection =
                        connectDatabase(*password);

                    unique_ptr<sql::PreparedStatement> statement(
                        connection->prepareStatement(
                            "UPDATE vehicles SET "
                            "make = ?, model = ?, year = ?, "
                            "category = ?, daily_rate = ?, status = ? "
                            "WHERE vehicle_id = ?"
                        )
                    );

                    statement->setString(1, data.make);
                    statement->setString(2, data.model);
                    statement->setInt(3, data.year);
                    statement->setString(4, data.category);
                    statement->setDouble(5, data.dailyRate);
                    statement->setString(6, data.status);
                    statement->setInt(7, vehicleId);

                    int changed = statement->executeUpdate();

                    // MySQL may return 0 when values are unchanged.

                    unique_ptr<sql::PreparedStatement> check(
                        connection->prepareStatement(
                            "SELECT vehicle_id FROM vehicles "
                            "WHERE vehicle_id = ?"
                        )
                    );

                    check->setInt(1, vehicleId);

                    unique_ptr<sql::ResultSet> result(
                        check->executeQuery()
                    );

                    if (!result->next()) {
                        return fail(404, "Vehicle not found");
                    }

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["vehicleId"] = vehicleId;

                    response["message"] =
                        "Vehicle information saved";

                    cout << "Manager " << managerId
                        << " updated vehicle " << vehicleId
                        << " (changed rows: " << changed << ")"
                        << endl;

                    return jsonReply(200, response);
                }
                catch (const exception& error) {

                    cerr << "Update vehicle error: "
                        << error.what() << endl;

                    return fail(
                        500, "Unable to update vehicle"
                    );
                }
            });


    // ======================================
    // ADD PHOTOS TO EXISTING VEHICLE
    // POST /api/manager/vehicles/ID/images
    // ======================================

    CROW_ROUTE(app, "/api/manager/vehicles/<int>/images")
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int vehicleId) {

                if (!authorizeWriteOrigin(request)) {

                    return fail(403, "Forbidden");
                }

                int managerId = 0;

                if (!authorizeManager(
                    request, *password, managerId)) {

                    return fail(
                        403, "Manager access required"
                    );
                }

                if (vehicleId <= 0) {

                    return fail(400, "Invalid vehicle ID");
                }

                UploadForm form;
                int primaryIndex = 0;

                try {

                    form = readMultipart(request);

                    if (form.photos.empty()) {

                        throw invalid_argument(
                            "Choose at least one photograph"
                        );
                    }

                    primaryIndex =
                        selectedPrimary(form);
                }
                catch (const exception& error) {
                    return fail(400, error.what());
                }

                unique_ptr<sql::Connection> connection;
                vector<fs::path> createdFiles;

                bool transaction = false;
                bool committed = false;

                try {

                    connection = connectDatabase(*password);

                    connection->setAutoCommit(false);
                    transaction = true;

                    if (!lockVehicle(
                        *connection, vehicleId)) {

                        connection->rollback();
                        transaction = false;

                        return fail(
                            404, "Vehicle not found"
                        );
                    }

                    bool makePrimary =
                        getField(form, "makePrimary") == "true";

                    // If no primary exists, automatically
                    // select one of the new photographs.

                    bool replacePrimary =
                        makePrimary ||
                        !hasPrimaryImage(
                            *connection, vehicleId
                        );

                    saveImages(
                        *connection,
                        vehicleId,
                        form,
                        replacePrimary,
                        primaryIndex,
                        createdFiles
                    );

                    connection->commit();
                    committed = true;
                    transaction = false;

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["vehicleId"] = vehicleId;

                    response["photosUploaded"] =
                        static_cast<int>(form.photos.size());

                    response["message"] =
                        "Photographs uploaded";

                    return jsonReply(200, response);
                }
                catch (const exception& error) {

                    if (transaction && connection) {
                        try { connection->rollback(); }
                        catch (...) {}
                    }

                    if (!committed) {
                        cleanupFiles(createdFiles);
                    }

                    cerr << "Upload error: "
                        << error.what() << endl;

                    return fail(
                        500, "Unable to upload images"
                    );
                }
            });


    // ======================================
    // SET EXISTING IMAGE AS PRIMARY
    // POST /api/manager/images/ID/primary
    // ======================================

    CROW_ROUTE(app, "/api/manager/images/<int>/primary")
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int imageId) {

                if (!authorizeWriteOrigin(request)) {

                    return fail(403, "Forbidden");
                }

                int managerId = 0;

                if (!authorizeManager(
                    request, *password, managerId)) {

                    return fail(
                        403, "Manager access required"
                    );
                }

                if (imageId <= 0) {
                    return fail(400, "Invalid image ID");
                }

                try {

                    auto connection =
                        connectDatabase(*password);

                    int vehicleId = 0;
                    string path;
                    bool alreadyPrimary = false;

                    if (!findImage(
                        *connection, imageId,
                        vehicleId, path, alreadyPrimary)) {

                        return fail(
                            404, "Photograph not found"
                        );
                    }

                    connection->setAutoCommit(false);

                    try {

                        if (!lockVehicle(
                            *connection, vehicleId)) {

                            connection->rollback();

                            return fail(404, "Vehicle not found");
                        }

                        // Confirm the image still exists.

                        if (!findImage(
                            *connection, imageId,
                            vehicleId, path, alreadyPrimary)) {

                            connection->rollback();

                            return fail(
                                404, "Photograph not found"
                            );
                        }

                        unique_ptr<sql::PreparedStatement> clear(
                            connection->prepareStatement(
                                "UPDATE vehicle_images "
                                "SET is_primary = FALSE "
                                "WHERE vehicle_id = ?"
                            )
                        );

                        clear->setInt(1, vehicleId);
                        clear->executeUpdate();

                        unique_ptr<sql::PreparedStatement> set(
                            connection->prepareStatement(
                                "UPDATE vehicle_images "
                                "SET is_primary = TRUE "
                                "WHERE image_id = ? "
                                "AND vehicle_id = ?"
                            )
                        );

                        set->setInt(1, imageId);
                        set->setInt(2, vehicleId);

                        if (set->executeUpdate() != 1) {
                            throw runtime_error(
                                "Photograph no longer exists"
                            );
                        }

                        connection->commit();
                    }
                    catch (...) {
                        try { connection->rollback(); }
                        catch (...) {}
                        throw;
                    }

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["vehicleId"] = vehicleId;

                    response["message"] =
                        "Main photograph updated";

                    return jsonReply(200, response);
                }
                catch (const exception& error) {

                    cerr << "Primary image error: "
                        << error.what() << endl;

                    return fail(
                        500, "Unable to change main photograph"
                    );
                }
            });


    // ======================================
    // DELETE AN EXISTING PHOTOGRAPH
    // POST /api/manager/images/ID/delete
    // ======================================

    CROW_ROUTE(app, "/api/manager/images/<int>/delete")
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request,
            int imageId) {

                if (!authorizeWriteOrigin(request)) {

                    return fail(403, "Forbidden");
                }

                int managerId = 0;

                if (!authorizeManager(
                    request, *password, managerId)) {

                    return fail(
                        403, "Manager access required"
                    );
                }

                if (imageId <= 0) {
                    return fail(400, "Invalid image ID");
                }

                try {

                    auto connection =
                        connectDatabase(*password);

                    int vehicleId = 0;
                    string imagePath;
                    bool wasPrimary = false;

                    if (!findImage(
                        *connection,
                        imageId,
                        vehicleId,
                        imagePath,
                        wasPrimary)) {

                        return fail(
                            404, "Photograph not found"
                        );
                    }

                    connection->setAutoCommit(false);

                    try {

                        if (!lockVehicle(
                            *connection, vehicleId)) {

                            connection->rollback();
                            return fail(404, "Vehicle not found");
                        }

                        if (!findImage(
                            *connection,
                            imageId,
                            vehicleId,
                            imagePath,
                            wasPrimary)) {

                            connection->rollback();

                            return fail(
                                404, "Photograph not found"
                            );
                        }

                        unique_ptr<sql::PreparedStatement> remove(
                            connection->prepareStatement(
                                "DELETE FROM vehicle_images "
                                "WHERE image_id = ?"
                            )
                        );

                        remove->setInt(1, imageId);
                        remove->executeUpdate();

                        // Automatically choose another
                        // main photo when necessary.

                        if (wasPrimary) {

                            unique_ptr<sql::PreparedStatement> findNext(
                                connection->prepareStatement(
                                    "SELECT image_id "
                                    "FROM vehicle_images "
                                    "WHERE vehicle_id = ? "
                                    "ORDER BY image_order, image_id "
                                    "LIMIT 1"
                                )
                            );

                            findNext->setInt(1, vehicleId);

                            unique_ptr<sql::ResultSet> next(
                                findNext->executeQuery()
                            );

                            if (next->next()) {

                                int nextId =
                                    next->getInt("image_id");

                                unique_ptr<sql::PreparedStatement> set(
                                    connection->prepareStatement(
                                        "UPDATE vehicle_images "
                                        "SET is_primary = TRUE "
                                        "WHERE image_id = ?"
                                    )
                                );

                                set->setInt(1, nextId);
                                set->executeUpdate();
                            }
                        }

                        connection->commit();
                    }
                    catch (...) {
                        try { connection->rollback(); }
                        catch (...) {}
                        throw;
                    }

                    // Remove the physical image only when
                    // its stored path matches our safe format.

                    const string prefix =
                        "/images/vehicles/";

                    if (imagePath.rfind(prefix, 0) == 0) {

                        string filename =
                            imagePath.substr(prefix.size());

                        if (validImageName(filename)) {

                            error_code error;

                            fs::remove(
                                PHOTO_FOLDER / filename,
                                error
                            );

                            if (error) {
                                cerr << "Image cleanup warning: "
                                    << error.message() << endl;
                            }
                        }
                    }

                    crow::json::wvalue response;

                    response["status"] = "success";
                    response["vehicleId"] = vehicleId;

                    response["message"] =
                        "Photograph deleted";

                    return jsonReply(200, response);
                }
                catch (const exception& error) {

                    cerr << "Delete image error: "
                        << error.what() << endl;

                    return fail(
                        500, "Unable to delete photograph"
                    );
                }
            });

}
