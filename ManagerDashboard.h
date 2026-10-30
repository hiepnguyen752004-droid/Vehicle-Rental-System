#pragma once
#ifndef MANAGERDASHBOARD_H
#define MANAGERDASHBOARD_H

#include <memory>
#include <string>
#include <jdbc/cppconn/connection.h>

using namespace std;

class ReservationManager;

class ManagerDashboard
{
private:
    unique_ptr<sql::Connection> connection;

    void listVehicles();
    void addVehicle();
    void changeDailyRate();
    void changeVehicleStatus();

public:
    ManagerDashboard(
        const string& mysqlPassword
    );

    void run(
        ReservationManager& reservationManager
    );
};

#endif