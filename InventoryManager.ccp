#include <iostream>
#include <set>
#include "InventoryManager.h"

using namespace std;

void InventoryManager::addVehicle(Vehicle vehicle)
{
    vehicles.push_back(vehicle);
}

void InventoryManager::viewVehicleBrands() const
{
    set<string> brands;

    cout << "AVAILABLE VEHICLE BRANDS" << endl;

    for (const Vehicle& vehicle : vehicles)
    {
        brands.insert(vehicle.getMake());
    }

    for (string brand : brands)
    {
        cout << brand << endl;
    }
}


void InventoryManager::searchVehicleStepByStep() const
{
 
    if (vehicles.empty())
    {
        cout << "No vehicles in inventory." << endl;
        return;
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


    cout << "Vehicle Rental Search" << endl;

    cout << endl;

    cout << "Available daily rate range: $"
        << lowestRate
        << " - $"
        << highestRate
        << endl;

    cout << "Enter minimum daily rate: $";
    cin >> minRate;

    cout << "Enter maximum daily rate: $";
    cin >> maxRate;


    if (minRate > maxRate)
    {
        cout << endl;
        cout << "Minimum rate cannot be greater than maximum rate."
            << endl;

        return;
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
        cout << "No vehicles found in that price range."
            << endl;

        return;
    }


 
    set<string> brands;


    for (const Vehicle& vehicle : rateResults)
    {
        brands.insert(vehicle.getMake());
    }


    cout << endl;
    cout << "AVAILABLE BRANDS IN YOUR PRICE RANGE"
        << endl;

    
     


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
        return;
    }

    set<string> models;


    for (const Vehicle& vehicle : brandResults)
    {
        models.insert(vehicle.getModel());
    }


    cout << endl;

    cout << "AVAILABLE "
        << make
        << " MODELS"
        << endl;


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
        return;
    }


   

    set<int> years;


    for (const Vehicle& vehicle : modelResults)
    {
        years.insert(vehicle.getYear());
    }


    cout << endl;
    cout << "Available years" << endl;
    


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
        return;
    }




    cout << endl;

    cout << "Matching vehicles" << endl;


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
