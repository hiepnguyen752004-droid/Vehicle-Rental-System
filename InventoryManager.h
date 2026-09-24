#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <vector>
#include <string>
#include "Vehicle.h"
using namespace std;

class InventoryManager
{
private:
    vector<Vehicle> vehicles;

public:
    void addVehicle(Vehicle vehicle);
    void viewVehicleBrands() const;
    void searchVehicleByMake(string make) const;
    void removeVehicle(int id);
};

#endif
