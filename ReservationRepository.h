#pragma once

#ifndef RESERVATIONREPOSITORY_H
#define RESERVATIONREPOSITORY_H

#include <memory>
#include <string>
#include <vector>

#include <jdbc/cppconn/connection.h>
#include "Reservation.h"

using namespace std;

class ReservationRepository
{
private:
    unique_ptr<sql::Connection> connection;

public:
    ReservationRepository(const string& password);

    bool checkAvailability(
        int vehicleId,
        const string& startDate,
        const string& endDate
    );

    bool createReservation(
        Reservation& reservation
    );

    // Manager operations

    bool cancelReservation(int reservationId);

    vector<Reservation> getReservations();

    // Customer operations

    bool cancelReservationForCustomer(
        int reservationId,
        int customerId
    );

    vector<Reservation> getReservationsForCustomer(
        int customerId
    );
};

#endif
