#include "InventoryManager.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;



void InventoryManager::addVehicle(Vehicle vehicle) {

    int id = vehicle.getVehicleId();

    if (vehicles.count(id) > 0) {
        cout << "Vehicle ID already exists." << endl;
        return;
    }

    vehicles.emplace(id, vehicle);
}



bool InventoryManager::getVehicleById(
    int id,
    Vehicle& result
) const {

    if (vehicles.count(id) == 0) {
        return false;
    }

    result = vehicles.at(id);

    return true;
}


void InventoryManager::removeVehicle(int id) {

    if (vehicles.erase(id) > 0) {
        cout << "Vehicle removed successfully." << endl;
    }
    else {
        cout << "Vehicle not found." << endl;
    }
}



int InventoryManager::searchVehicleStepByStep() const {

    if (vehicles.empty()) {
        cout << "No vehicles in inventory." << endl;
        return -1;
    }



    double lowestRate = numeric_limits<double>::max();
    double highestRate = numeric_limits<double>::lowest();

    for (const auto& entry : vehicles) {

        const Vehicle& vehicle = entry.second;

        if (vehicle.getDailyRate() < lowestRate) {
            lowestRate = vehicle.getDailyRate();
        }

        if (vehicle.getDailyRate() > highestRate) {
            highestRate = vehicle.getDailyRate();
        }
    }

    cout << fixed << setprecision(2);

    cout << "\nDaily rate range: $"
         << lowestRate << " - $"
         << highestRate << endl;



    double minRate;
    double maxRate;

    cout << "Enter minimum daily rate: $";

    if (!(cin >> minRate)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid price." << endl;
        return -1;
    }

    cout << "Enter maximum daily rate: $";

    if (!(cin >> maxRate)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid price." << endl;
        return -1;
    }

    if (minRate < 0 || minRate > maxRate) {
        cout << "Invalid price range." << endl;
        return -1;
    }


    vector<Vehicle> rateResults;

    for (const auto& entry : vehicles) {

        const Vehicle& vehicle = entry.second;

        if (vehicle.getDailyRate() >= minRate &&
            vehicle.getDailyRate() <= maxRate) {

            rateResults.push_back(vehicle);
        }
    }

    if (rateResults.empty()) {
        cout << "No vehicles match this price range." << endl;
        return -1;
    }


    set<string> brands;

    for (const Vehicle& vehicle : rateResults) {
        brands.insert(vehicle.getMake());
    }

    cout << "\nAvailable brands:" << endl;

    for (const string& brand : brands) {
        cout << "- " << brand << endl;
    }

    string selectedBrand;

    cout << "Enter brand: ";
    cin >> selectedBrand;

    vector<Vehicle> brandResults;

    for (const Vehicle& vehicle : rateResults) {

        if (vehicle.getMake() == selectedBrand) {
            brandResults.push_back(vehicle);
        }
    }

    if (brandResults.empty()) {
        cout << "No matching brand." << endl;
        return -1;
    }


    set<string> models;

    for (const Vehicle& vehicle : brandResults) {
        models.insert(vehicle.getModel());
    }

    cout << "\nAvailable models:" << endl;

    for (const string& model : models) {
        cout << "- " << model << endl;
    }

    string selectedModel;

    cout << "Enter model: ";
    cin >> selectedModel;

    vector<Vehicle> modelResults;

    for (const Vehicle& vehicle : brandResults) {

        if (vehicle.getModel() == selectedModel) {
            modelResults.push_back(vehicle);
        }
    }

    if (modelResults.empty()) {
        cout << "No matching model." << endl;
        return -1;
    }


    set<int> years;

    for (const Vehicle& vehicle : modelResults) {
        years.insert(vehicle.getYear());
    }

    cout << "\nAvailable years:" << endl;

    for (int year : years) {
        cout << "- " << year << endl;
    }

    int selectedYear;

    cout << "Enter year: ";

    if (!(cin >> selectedYear)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid year." << endl;
        return -1;
    }

    vector<Vehicle> yearResults;

    for (const Vehicle& vehicle : modelResults) {

        if (vehicle.getYear() == selectedYear) {
            yearResults.push_back(vehicle);
        }
    }

    if (yearResults.empty()) {
        cout << "No matching year." << endl;
        return -1;
    }


    sort(yearResults.begin(), yearResults.end(),
        [](const Vehicle& a, const Vehicle& b) {
            return a.getVehicleId() < b.getVehicleId();
        }
    );




    for (const Vehicle& vehicle : yearResults) {

        cout << "Vehicle ID: "
             << vehicle.getVehicleId() << endl;

        cout << "Vehicle: "
             << vehicle.getYear() << " "
             << vehicle.getMake() << " "
             << vehicle.getModel() << endl;

        cout << "Category: "
             << vehicle.getCategory() << endl;

        cout << "Daily Rate: $"
             << vehicle.getDailyRate() << endl;

        cout << "Status: "
             << vehicle.getStatus() << endl;

       
    }


    // using hash map

    int selectedId;

    cout << "Enter Vehicle ID to select: ";

    if (!(cin >> selectedId)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid Vehicle ID." << endl;
        return -1;
    }

    if (vehicles.count(selectedId) == 0) {
        cout << "Vehicle ID not found." << endl;
        return -1;
    }

    const Vehicle& selected = vehicles.at(selectedId);


    if (selected.getDailyRate() < minRate ||
        selected.getDailyRate() > maxRate ||
        selected.getMake() != selectedBrand ||
        selected.getModel() != selectedModel ||
        selected.getYear() != selectedYear) {

        cout << "Vehicle is not in the search results." << endl;
        return -1;
    }


    if (selected.getStatus() != "Available") {
        cout << "Vehicle is unavailable." << endl;
        return -1;
    }

    return selectedId;
}
