
#pragma once

#ifndef AUTH_ROUTES_H
#define AUTH_ROUTES_H

#include <string>
#include <crow.h>

using namespace std;


// Register all authentication routes
void registerAuthRoutes(
    crow::SimpleApp& app,
    const string& mysqlPassword
);


// Check if a request comes from this website
bool authorizeWriteOrigin(
    const crow::request& request
);


// Check the active Manager session
bool authorizeManager(
    const crow::request& request,
    const string& mysqlPassword,
    int& managerId
);


// Get the currently logged-in customer ID
// Used by BookingRoutes.cpp
bool getLoggedInCustomerId(
    const crow::request& request,
    int& customerId
);

#endif
