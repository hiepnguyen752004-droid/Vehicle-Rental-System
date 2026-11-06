
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "VehicleImageRepository.h"

#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;


// GET ALL IMAGES FOR ONE VEHICLE

vector<VehicleImage> VehicleImageRepository::getImages(
    sql::Connection& connection,
    int vehicleId)
{
    vector<VehicleImage> images;

    if (vehicleId <= 0) {
        return images;
    }

    unique_ptr<sql::PreparedStatement> statement(
        connection.prepareStatement(
            "SELECT image_id, vehicle_id, image_path, "
            "image_order, is_primary "
            "FROM vehicle_images "
            "WHERE vehicle_id = ? "
            "ORDER BY is_primary DESC, "
            "image_order ASC, image_id ASC"
        )
    );

    statement->setInt(1, vehicleId);

    unique_ptr<sql::ResultSet> result(
        statement->executeQuery()
    );

    while (result->next()) {

        VehicleImage image;

        image.imageId =
            result->getInt("image_id");

        image.vehicleId =
            result->getInt("vehicle_id");

        image.imagePath =
            string(result->getString("image_path").c_str());

        image.imageOrder =
            result->getInt("image_order");

        image.isPrimary =
            result->getInt("is_primary") != 0;

        images.push_back(image);
    }

    return images;
}


// GET PRIMARY IMAGE PATH
// IF NONE IS MARKED PRIMARY, USE THE FIRST IMAGE

string VehicleImageRepository::getPrimaryImagePath(
    sql::Connection& connection,
    int vehicleId)
{
    vector<VehicleImage> images =
        getImages(connection, vehicleId);

    if (images.empty()) {
        return "";
    }

    return images.front().imagePath;
}


// SAVE IMAGE DATABASE RECORD
// THIS DOES NOT UPLOAD OR SAVE THE ACTUAL IMAGE FILE

bool VehicleImageRepository::addImageRecord(
    sql::Connection& connection,
    int vehicleId,
    const string& imagePath,
    int imageOrder)
{
    if (vehicleId <= 0 || imageOrder < 0) {
        return false;
    }

    // ACCEPT ONLY OUR MANAGED VEHICLE IMAGE PATHS

    const string prefix = "/images/vehicles/";

    if (imagePath.rfind(prefix, 0) != 0 ||
        imagePath.size() <= prefix.size() ||
        imagePath.size() > 255 ||
        imagePath.find("..") != string::npos ||
        imagePath.find('\\') != string::npos) {

        return false;
    }

    unique_ptr<sql::PreparedStatement> statement(
        connection.prepareStatement(
            "INSERT INTO vehicle_images "
            "(vehicle_id, image_path, image_order, is_primary) "
            "VALUES (?, ?, ?, FALSE)"
        )
    );

    statement->setInt(1, vehicleId);

    statement->setString(2, imagePath);

    statement->setInt(3, imageOrder);

    int rows = statement->executeUpdate();

    return rows == 1;
}
