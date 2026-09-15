#include "Vehicle.h"

Vehicle::Vehicle()
{
    vehicleId = 0;
    make = "";
    model = "";
    year = 0;
    category = "";
    dailyRate = 0.0;
    status = "Available";
}

Vehicle::Vehicle(
    int vehicleId,
    string make,
    string model,
    int year,
    string category,
    double dailyRate,
    string status)
{
    this->vehicleId = vehicleId;
    this->make = make;
    this->model = model;
    this->year = year;
    this->category = category;
    this->dailyRate = dailyRate;
    this->status = status;
}

int Vehicle::getVehicleId() const
{
    return vehicleId;
}

string Vehicle::getMake() const
{
    return make;
}

string Vehicle::getModel() const
{
    return model;
}

int Vehicle::getYear() const
{
    return year;
}

string Vehicle::getCategory() const
{
    return category;
}

double Vehicle::getDailyRate() const
{
    return dailyRate;
}

string Vehicle::getStatus() const
{
    return status;
}

void Vehicle::setVehicleId(int vehicleId)
{
    this->vehicleId = vehicleId;
}

void Vehicle::setMake(string make)
{
    this->make = make;
}

void Vehicle::setModel(string model)
{
    this->model = model;
}

void Vehicle::setYear(int year)
{
    this->year = year;
}

void Vehicle::setCategory(string category)
{
    this->category = category;
}

void Vehicle::setDailyRate(double dailyRate)
{
    this->dailyRate = dailyRate;
}

void Vehicle::setStatus(string status)
{
    this->status = status;
}
