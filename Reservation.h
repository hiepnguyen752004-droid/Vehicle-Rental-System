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
        int reservationId,
        int customerId,
        int vehicleId,
        string startDate,
        string endDate,
        double totalCost,
        string status
    );

    
    int getReservationId() const;
    int getCustomerId() const;
    int getVehicleId() const;

    string getStartDate() const;
    string getEndDate() const;

    double getTotalCost() const;
    string getStatus() const;

    
    void setReservationId(int reservationId);
    void setCustomerId(int customerId);
    void setVehicleId(int vehicleId);

    void setStartDate(string startDate);
    void setEndDate(string endDate);

    void setTotalCost(double totalCost);
    void setStatus(string status);
};

#endif
