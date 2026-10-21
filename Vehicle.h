#pragma once
#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>
using namespace std;

class Vehicle
{
private:
    int vehicleId;
    int year;
    string make;
    string model;
    string status;
    string category;
    double dailyRate;

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

    // Getters
    int getVehicleId() const;
    int getYear() const;
    string getMake() const;
    string getModel() const;
    string getCategory() const;
    string getStatus() const;
    double getDailyRate() const;

    // Setters
    void setVehicleId(int id);
    void setMake(string vehicleMake);
    void setModel(string vehicleModel);
    void setYear(int vehicleYear);
    void setCategory(string vehicleCategory);
    void setDailyRate(double rate);
    void setStatus(string vehicleStatus);
};

#endif