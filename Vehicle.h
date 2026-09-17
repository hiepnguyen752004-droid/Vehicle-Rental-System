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
        int vehicleYear,
        string vehicleMake,
        string vehicleModel,
        string vehicleCategory,
        string vehicleStatus,
        double rate
    );

    int getVehicleId() const;
    int getYear() const;
    string getMake() const;
    string getModel() const;
    string getCategory() const;
    string getStatus() const;
    double getDailyRate() const;

    void setVehicleId(int id);
    void setMake(string vehicleMake);
    void setModel(string vehicleModel);
    void setYear(int vehicleYear);
    void setCategory(string vehicleCategory);
    void setDailyRate(double rate);
    void setStatus(string vehicleStatus);
};

#endif
