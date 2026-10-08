#include <iostream>
#include <set>
#include "InventoryManager.h"

using namespace std;

void InventoryManager::addVehicle(Vehicle vehicle)
{
    vehicles.push_back(vehicle);
}


int InventoryManager::searchVehicleStepByStep() const
{
    if (vehicles.empty())
    {
        cout << "No vehicles in inventory." << endl;
        return -1;
    }


    double lowestRate = vehicles[0].getDailyRate();
    double highestRate = vehicles[0].getDailyRate();

    double minRate;
    double maxRate;

    string make;
    string model;
    int year;


    vector<Vehicle> rateResults;
    vector<Vehicle> brandResults;
    vector<Vehicle> modelResults;
    vector<Vehicle> yearResults;


    

    for (const Vehicle& vehicle : vehicles)
    {
        if (vehicle.getDailyRate() < lowestRate)
        {
            lowestRate = vehicle.getDailyRate();
        }

        if (vehicle.getDailyRate() > highestRate)
        {
            highestRate = vehicle.getDailyRate();
        }
    }


    cout << "VEHICLE RENTAL SEARCH" << endl;
 

    cout << endl;

    cout << "Available daily rate range: $"
        << lowestRate
        << " - $"
        << highestRate
        << endl;


   

    cout << endl;

    cout << "Enter minimum daily rate: $";
    cin >> minRate;

    cout << "Enter maximum daily rate: $";
    cin >> maxRate;


    if (minRate > maxRate)
    {
        cout << endl;
        cout << "Invalid price range." << endl;

        return -1;
    }


    for (const Vehicle& vehicle : vehicles)
    {
        if (vehicle.getDailyRate() >= minRate &&
            vehicle.getDailyRate() <= maxRate)
        {
            rateResults.push_back(vehicle);
        }
    }


    if (rateResults.empty())
    {
        cout << endl;
        cout << "No vehicles found in that price range." << endl;

        return -1;
    }


    // BRAND FILTER

    set<string> brands;

    for (const Vehicle& vehicle : rateResults)
    {
        brands.insert(vehicle.getMake());
    }


    cout << endl;
    cout << "AVAILABLE BRANDS" << endl;
    


    for (string brand : brands)
    {
        cout << brand << endl;
    }


    cout << endl;
    cout << "Enter vehicle brand: ";
    cin >> make;


    for (const Vehicle& vehicle : rateResults)
    {
        if (vehicle.getMake() == make)
        {
            brandResults.push_back(vehicle);
        }
    }


    if (brandResults.empty())
    {
        cout << endl;
        cout << "Vehicle brand not found." << endl;

        return -1;
    }


    // MODEL FILTER

    set<string> models;


    for (const Vehicle& vehicle : brandResults)
    {
        models.insert(vehicle.getModel());
    }


    cout << endl;
    cout << "AVAILABLE " << make << " MODELS" << endl;
    


    for (string vehicleModel : models)
    {
        cout << vehicleModel << endl;
    }


    cout << endl;
    cout << "Enter vehicle model: ";
    cin >> model;


    for (const Vehicle& vehicle : brandResults)
    {
        if (vehicle.getModel() == model)
        {
            modelResults.push_back(vehicle);
        }
    }


    if (modelResults.empty())
    {
        cout << endl;
        cout << "Vehicle model not found." << endl;

        return -1;
    }


 
    set<int> years;


    for (const Vehicle& vehicle : modelResults)
    {
        years.insert(vehicle.getYear());
    }


    cout << endl;
    cout << "AVAILABLE YEARS" << endl;
    


    for (int vehicleYear : years)
    {
        cout << vehicleYear << endl;
    }


    cout << endl;
    cout << "Enter vehicle year: ";
    cin >> year;


    for (const Vehicle& vehicle : modelResults)
    {
        if (vehicle.getYear() == year)
        {
            yearResults.push_back(vehicle);
        }
    }


    if (yearResults.empty())
    {
        cout << endl;
        cout << "Vehicle year not found." << endl;

        return -1;
    }


    

    cout << endl;
    cout << "MATCHING VEHICLES" << endl;
    


    for (const Vehicle& vehicle : yearResults)
    {
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

        
    }


    // SELECT VEHICLE

    int selectedId;

    cout << endl;
    cout << "Enter Vehicle ID to select: ";
    cin >> selectedId;


    for (const Vehicle& vehicle : yearResults)
    {
        if (vehicle.getVehicleId() == selectedId)
        {
            if (vehicle.getStatus() != "Available")
            {
                cout << endl;
                cout << "This vehicle is currently unavailable."
                    << endl;

                return -1;
            }

            cout << endl;

            cout << "Vehicle selected: "
                << vehicle.getYear() << " "
                << vehicle.getMake() << " "
                << vehicle.getModel()
                << endl;

            return selectedId;
        }
    }


    cout << endl;
    cout << "Invalid Vehicle ID." << endl;

    return -1;
}


void InventoryManager::removeVehicle(int id)
{
    for (int i = 0; i < vehicles.size(); i++)
    {
        if (vehicles[i].getVehicleId() == id)
        {
            vehicles.erase(vehicles.begin() + i);

            cout << "Vehicle removed from inventory."
                << endl;

            return;
        }
    }

    cout << "Vehicle not found." << endl;
}
