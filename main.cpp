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
        "Toyota",
        "Corolla",
        2025,
        "Sedan",
        50.00,
        "Available"
    );


    Vehicle vehicle3(
        3,
        "Toyota",
        "RAV4",
        2024,
        "Suv",
        75.00,
        "Available"
    );


    Vehicle vehicle4(
        4,
        "Honda",
        "Civic",
        2024,
        "Sedan",
        55.00,
        "Available"
    );


    Vehicle vehicle5(
        5,
        "Honda",
        "Accord",
        2025,
        "Sedan",
        65.00,
        "Available"
    );


    Vehicle vehicle6(
        6,
        "Ford",
        "Explorer",
        2025,
        "Suv",
        85.00,
        "Available"
    );


    Vehicle vehicle7(
        7,
        "Toyota",
        "Camry",
        2024,
        "Sedan",
        55.00,
        "Available"
    );


    Vehicle vehicle8(
        8,
        "Toyota",
        "Camry",
        2025,
        "Sedan",
        65.00,
        "Available"
    );

    Vehicle vehicle9(
        8,
        "Ford",
        "Explorer",
        2019,
        "Suv",
        65.00,
        "Available"
    );



    InventoryManager inventory;


  
    inventory.addVehicle(vehicle1);
    inventory.addVehicle(vehicle2);
    inventory.addVehicle(vehicle3);
    inventory.addVehicle(vehicle4);
    inventory.addVehicle(vehicle5);
    inventory.addVehicle(vehicle6);
    inventory.addVehicle(vehicle7);
    inventory.addVehicle(vehicle8);
    inventory.addVehicle(vehicle9);

    inventory.searchVehicleStepByStep();


   
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
    cout << "CUSTOMER EXAMPLE" << endl;
    

    cout << "Customer: "
        << customer.getName()
        << endl;

    cout << "Email: "
        << customer.getEmail()
        << endl;


    cout << endl;
    cout << "RESERVATION EXAMPLE" << endl;
   ;

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
