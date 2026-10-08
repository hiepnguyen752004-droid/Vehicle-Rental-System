#include <iostream>

#include "Vehicle.h"
#include "Customer.h"
#include "Reservation.h"
#include "InventoryManager.h"
#include "ReservationManager.h"

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
        "SUV",
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
        "SUV",
        85.00,
        "Available"
    );


    Vehicle vehicle7(
        7,
        "Chevrolet",
        "Tahoe",
        2024,
        "SUV",
        95.00,
        "Available"
    );


    Vehicle vehicle8(
        8,
        "Toyota",
        "Camry",
        2024,
        "Sedan",
        55.00,
        "Available"
    );


    Vehicle vehicle9(
        9,
        "Toyota",
        "Camry",
        2025,
        "Sedan",
        65.00,
        "Available"
    );


    Vehicle vehicle10(
        10,
        "BMW",
        "X5",
        2025,
        "SUV",
        120.00,
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
    inventory.addVehicle(vehicle10);


    // CUSTOMER

    Customer customer(
        1,
        "Austin Nguyen",
        "Austin@email.com",
        "password123",
        "713-555-1234"
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

    int selectedVehicleId =
        inventory.searchVehicleStepByStep();


    if (selectedVehicleId == -1)
    {
        cout << endl;
        cout << "No vehicle selected."
            << endl;
        return 0;
    }



    ReservationManager reservationManager;
    string startDate;
    string endDate;


    cout << endl;
    cout << "CREATE RESERVATION" << endl;


    cout << "Enter rental start date "
        << "(YYYY-MM-DD): ";

    cin >> startDate;


    cout << "Enter rental end date "
        << "(YYYY-MM-DD): ";

    cin >> endDate;


    Reservation reservation1(
        1,
        customer.getCustomerId(),
        selectedVehicleId,
        startDate,
        endDate,
        0.00,
        "Confirmed"
    );


    reservationManager.createReservation( reservation1 );


    reservationManager.viewReservations();

    int cancelId;

    cout << endl;
    cout << "Enter Reservation ID to cancel: ";
    cin >> cancelId;

    reservationManager.cancelReservation(cancelId);
    cout << endl;
    cout << "UPDATED RESERVATIONS" << endl;

    reservationManager.viewReservations();


    return 0;
}
