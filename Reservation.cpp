#include "Reservation.h"

Reservation::Reservation()
{
    reservationId = 0;
    customerId = 0;
    vehicleId = 0;
    startDate = "";
    endDate = "";
    totalCost = 0.0;
    status = "Pending";
}

Reservation::Reservation(
    int id,
    int customer,
    int vehicle,
    string start,
    string end,
    double cost,
    string reservationStatus)
{
    reservationId = id;
    customerId = customer;
    vehicleId = vehicle;
    startDate = start;
    endDate = end;
    totalCost = cost;
    status = reservationStatus;
}

int Reservation::getReservationId() const
{
    return reservationId;
}

int Reservation::getCustomerId() const
{
    return customerId;
}

int Reservation::getVehicleId() const
{
    return vehicleId;
}

string Reservation::getStartDate() const
{
    return startDate;
}

string Reservation::getEndDate() const
{
    return endDate;
}

double Reservation::getTotalCost() const
{
    return totalCost;
}

string Reservation::getStatus() const
{
    return status;
}

void Reservation::setReservationId(int id)
{
    reservationId = id;
}

void Reservation::setCustomerId(int customer)
{
    customerId = customer;
}

void Reservation::setVehicleId(int vehicle)
{
    vehicleId = vehicle;
}

void Reservation::setStartDate(string start)
{
    startDate = start;
}

void Reservation::setEndDate(string end)
{
    endDate = end;
}

void Reservation::setTotalCost(double cost)
{
    totalCost = cost;
}

void Reservation::setStatus(string reservationStatus)
{
    status = reservationStatus;
}
