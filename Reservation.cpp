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
    int reservationId,
    int customerId,
    int vehicleId,
    string startDate,
    string endDate,
    double totalCost,
    string status)
{
    this->reservationId = reservationId;
    this->customerId = customerId;
    this->vehicleId = vehicleId;
    this->startDate = startDate;
    this->endDate = endDate;
    this->totalCost = totalCost;
    this->status = status;
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

void Reservation::setReservationId(int reservationId)
{
    this -> reservationId = reservationId;
}

void Reservation::setCustomerId(int customerId)
{
    this -> customerId = customerId;
}

void Reservation::setVehicleId(int vehicleId)
{
    this -> vehicleId = vehicleId;
}

void Reservation::setStartDate(string startDate)
{
    this -> startDate = startDate;
}

void Reservation::setEndDate(string endDate)
{
    this -> endDate = endDate;
}

void Reservation::setTotalCost(double totalCost)
{
    this -> totalCost = totalCost;
}

void Reservation::setStatus(string status)
{
    this -> status = status;
}
