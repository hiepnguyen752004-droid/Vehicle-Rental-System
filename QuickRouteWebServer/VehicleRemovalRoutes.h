#pragma once


#ifndef VEHICLE_REMOVAL_ROUTES_H
#define VEHICLE_REMOVAL_ROUTES_H

#include <crow.h>
#include <string>

using namespace std;

void registerVehicleRemovalRoutes(
    crow::SimpleApp& app,
    const string& mysqlPassword
);

#endif
