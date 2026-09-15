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
        int vehicleId,
        string make,
        string model,
        int year,
        string category,
        double dailyRate,
        string status
    );

    
    int getVehicleId() const;
    string getMake() const;
    string getModel() const;
    int getYear() const;
    string getCategory() const;
    double getDailyRate() const;
    string getStatus() const;

    
    void setVehicleId(int vehicleId);
    void setMake(string make);
    void setModel(string model);
    void setYear(int year);
    void setCategory(string category);
    void setDailyRate(double dailyRate);
    void setStatus(string status);
};

#endif
