#pragma once

#ifndef BOOKING_ROUTES_H
#define BOOKING_ROUTES_H

#include <string>
#include <crow.h>

void registerBookingRoutes(
    crow::SimpleApp& app,
    const std::string& mysqlPassword
);

#endif