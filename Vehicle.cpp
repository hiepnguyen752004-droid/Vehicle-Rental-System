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
    int id,
    string vehicleMake,
    string vehicleModel,
    int vehicleYear,
    string vehicleCategory,
    double rate,
    string vehicleStatus)
{
    vehicleId = id;
    make = vehicleMake;
    model = vehicleModel;
    year = vehicleYear;
    category = vehicleCategory;
    dailyRate = rate;
    status = vehicleStatus;
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

void Vehicle::setVehicleId(int id)
{
    vehicleId = id;
}

void Vehicle::setMake(string vehicleMake)
{
    make = vehicleMake;
}

void Vehicle::setModel(string vehicleModel)
{
    model = vehicleModel;
}

void Vehicle::setYear(int vehicleYear)
{
    year = vehicleYear;
}

void Vehicle::setCategory(string vehicleCategory)
{
    category = vehicleCategory;
}

void Vehicle::setDailyRate(double rate)
{
    dailyRate = rate;
}

void Vehicle::setStatus(string vehicleStatus)
{
    status = vehicleStatus;
}
