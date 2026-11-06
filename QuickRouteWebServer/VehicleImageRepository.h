#pragma once

#ifndef VEHICLE_IMAGE_REPOSITORY_H
#define VEHICLE_IMAGE_REPOSITORY_H

#include <string>
#include <vector>

#include <jdbc/cppconn/connection.h>

using namespace std;


// REPRESENTS ONE IMAGE RECORD IN MYSQL

struct VehicleImage {

    int imageId = 0;

    int vehicleId = 0;

    string imagePath;

    int imageOrder = 0;

    bool isPrimary = false;

};


// DATABASE OPERATIONS FOR VEHICLE IMAGES

class VehicleImageRepository {

public:

    // GET ALL PHOTOS BELONGING TO ONE VEHICLE

    vector<VehicleImage> getImages(
        sql::Connection& connection,
        int vehicleId
    );


    // GET THE MAIN PHOTO FOR A VEHICLE

    string getPrimaryImagePath(
        sql::Connection& connection,
        int vehicleId
    );


    // SAVE AN IMAGE RECORD AFTER THE FILE
    // HAS BEEN VALIDATED AND STORED BY THE SERVER

    bool addImageRecord(
        sql::Connection& connection,
        int vehicleId,
        const string& imagePath,
        int imageOrder = 0
    );

};

#endif
