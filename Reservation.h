#pragma once
#ifndef RESERVATION_H
#define RESERVATION_H

#include <string>

using namespace std;

class Reservation
{
private:
    int reservationId;
    int customerId;
    int vehicleId;
    string startDate;
    string endDate;
    double totalCost;
    string status;

public:
    Reservation();

    Reservation(
        int id,
        int customer,
        int vehicle,
        string start,
        string end,
        double cost,
        string reservationStatus
    );

    int getReservationId() const;
    int getCustomerId() const;
    int getVehicleId() const;
    string getStartDate() const;
    string getEndDate() const;
    double getTotalCost() const;
    string getStatus() const;

    void setReservationId(int id);
    void setCustomerId(int customer);
    void setVehicleId(int vehicle);
    void setStartDate(string start);
    void setEndDate(string end);
    void setTotalCost(double cost);
    void setStatus(string reservationStatus);
};

#endif
