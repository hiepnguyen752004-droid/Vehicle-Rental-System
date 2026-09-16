#pragma once
#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>

using namespace std;

class Vehicle
{
private:
    int vehicleId;
    string make;
    string model;
    int year;
    string category;
    double dailyRate;
    string status;

public:
    Vehicle();

    Vehicle(
        int id,
        string vehicleMake,
        string vehicleModel,
        int vehicleYear,
        string vehicleCategory,
        double rate,
        string vehicleStatus
    );

    int getVehicleId() const;
    string getMake() const;
    string getModel() const;
    int getYear() const;
    string getCategory() const;
    double getDailyRate() const;
    string getStatus() const;

    void setVehicleId(int id);
    void setMake(string vehicleMake);
    void setModel(string vehicleModel);
    void setYear(int vehicleYear);
    void setCategory(string vehicleCategory);
    void setDailyRate(double rate);
    void setStatus(string vehicleStatus);
};

#endif
