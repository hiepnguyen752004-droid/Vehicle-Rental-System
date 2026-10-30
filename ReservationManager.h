#pragma once
#ifndef RESERVATIONMANAGER_H
#define RESERVATIONMANAGER_H

#include <string>
#include "Reservation.h"

using namespace std;

class ReservationRepository;

class ReservationManager
{
private:
    ReservationRepository& repository;

public:
    ReservationManager(
        ReservationRepository& repo
    );

    bool checkAvailability(
        int vehicleId,
        string startDate,
        string endDate
    ) const;

    bool createReservation(
        Reservation& reservation
    );

    // Manager operations

    void cancelReservation(
        int reservationId
    );

    void viewReservations() const;

    // Customer operations

    void cancelCustomerReservation(
        int reservationId,
        int customerId
    );

    void viewCustomerReservations(
        int customerId
    ) const;
};

#endif

