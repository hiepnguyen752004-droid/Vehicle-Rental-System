#include <iostream>

#include "Vehicle.h"
#include "Customer.h"
#include "Reservation.h"
#include "InventoryManager.h"

using namespace std;

int main()
{
    Vehicle vehicle1(
        1,
        "Toyota",
        "Camry",
        2025,
        "Sedan",
        60.00,
        "Available"
    );

    Vehicle vehicle2(
        2,
        "Honda",
        "Civic",
        2024,
        "Sedan",
        55.00,
        "Available"
    );

    Vehicle vehicle3(
        3,
        "Ford",
        "Explorer",
        2025,
        "SUV",
        85.00,
        "Available"
    );

    Vehicle vehicle4(
        4,
        "Chevrolet",
        "Tahoe",
        2024,
        "SUV",
        95.00,
        "Unavailable"
    );


    InventoryManager inventory;

    inventory.addVehicle(vehicle1);
    inventory.addVehicle(vehicle2);
    inventory.addVehicle(vehicle3);
    inventory.addVehicle(vehicle4);


    
    inventory.viewVehicleBrands();


    
    string searchMake;

    cout << endl;
    cout << "Enter vehicle brand to search: ";
    cin >> searchMake;

    inventory.searchVehicleByMake(searchMake);


   
    Customer customer(
        1,
        "Austin Nguyen",
        "Austin@email.com",
        "password123",
        "713-555-1234"
    );


  
    Reservation reservation(
        1,
        customer.getCustomerId(),
        vehicle1.getVehicleId(),
        "2026-09-20",
        "2026-09-25",
        300,
        "Confirmed"
    );


    cout << endl;
    cout << "CUSTOMER" << endl;
    

    cout << "Customer: "
        << customer.getName()
        << endl;

    cout << "Email: "
        << customer.getEmail()
        << endl;


    cout << endl;
    cout << "RESERVATION" << endl;
    

    cout << "Reservation ID: "
        << reservation.getReservationId()
        << endl;

    cout << "Customer ID: "
        << reservation.getCustomerId()
        << endl;

    cout << "Vehicle ID: "
        << reservation.getVehicleId()
        << endl;

    cout << "Rental dates: "
        << reservation.getStartDate()
        << " to "
        << reservation.getEndDate()
        << endl;

    cout << "Total cost: $"
        << reservation.getTotalCost()
        << endl;

    cout << "Status: "
        << reservation.getStatus()
        << endl;


    return 0;
}
