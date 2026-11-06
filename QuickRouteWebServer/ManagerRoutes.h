
#pragma once

#ifndef MANAGER_ROUTES_H
#define MANAGER_ROUTES_H

#include <string>
#include <crow.h>

using namespace std;

void registerManagerRoutes(
    crow::SimpleApp& app,
    const string& mysqlPassword
);

#endif
