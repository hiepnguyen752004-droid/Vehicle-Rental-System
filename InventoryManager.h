#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <unordered_map>
#include "Vehicle.h"

using namespace std;

class InventoryManager {
private:
    unordered_map<int, Vehicle> vehicles;

public:
    void addVehicle(Vehicle vehicle);
    bool getVehicleById(int id, Vehicle& result) const;
    void removeVehicle(int id);
    int searchVehicleStepByStep() const;
};

#endif