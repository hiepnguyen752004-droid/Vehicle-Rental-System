#pragma once
#ifndef RESERVATIONMANAGER_H
#define RESERVATIONMANAGER_H

#include <vector>
#include <string>
#include "Reservation.h"

using namespace std;

class ReservationManager
{
private:
    vector<Reservation> reservations;

public:
    bool checkAvailability(
        int vehicleId,
        string startDate,
        string endDate
    ) const;

    bool createReservation(Reservation reservation);

    void cancelReservation(int reservationId);

    void viewReservations() const;
};

#endif
