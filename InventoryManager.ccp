#include <iostream>
#include "InventoryManager.h"
using namespace std;

void InventoryManager::addVehicle(Vehicle vehicle)
{
    vehicles.push_back(vehicle);
}

void InventoryManager::viewVehicleBrands() const
{
    cout << "VEHICLE INVENTORY" << endl;
    
    for (const Vehicle& vehicle : vehicles)
    {
        cout << vehicle.getMake() << endl;
    }
}

void InventoryManager::searchVehicleByMake(string make) const
{
    bool found = false;

    for (const Vehicle& vehicle : vehicles)
    {
        if (vehicle.getMake() == make)
        {
            cout << endl;
            cout << "VEHICLE FOUND" << endl;
            

            cout << "Vehicle ID: "
                << vehicle.getVehicleId()
                << endl;

            cout << "Vehicle: "
                << vehicle.getYear() << " "
                << vehicle.getMake() << " "
                << vehicle.getModel()
                << endl;

            cout << "Category: "
                << vehicle.getCategory()
                << endl;

            cout << "Daily Rate: $"
                << vehicle.getDailyRate()
                << endl;

            cout << "Status: "
                << vehicle.getStatus()
                << endl;

            found = true;
        }
    }

    if (found == false)
    {
        cout << endl;
        cout << "Vehicle brand not found." << endl;
    }
}

void InventoryManager::removeVehicle(int id)
{
    for (int i = 0; i < vehicles.size(); i++)
    {
        if (vehicles[i].getVehicleId() == id)
        {
            vehicles.erase(vehicles.begin() + i);
            cout << "Vehicle removed from inventory." << endl;
            return;
        }
    }
    cout << "Vehicle not found." << endl;
}
