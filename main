#include <iostream>

#include "Entity/Vehicle.h"
#include "Entity/Customer.h"
#include "Entity/Reservation.h"

using namespace std;

int main() {
    Vehicle vehicle(
        1,
        "Toyota",
        "Camry",
        2025,
        "Sedan",
        60.00,
        "Available"
    );

    Customer customer(
        1,
        "John Smith",
        "john@email.com",
        "password123",
        "713-555-1234"
    );

    Reservation reservation(
        1,
        customer.getCustomerId(),
        vehicle.getVehicleId(),
        "2026-09-20",
        "2026-09-25",
        300.00,
        "Confirmed"
    );

    cout << "Vehicle: "
        << vehicle.getYear() << " "
        << vehicle.getMake() << " "
        << vehicle.getModel()
        << endl;

    cout << "Category: "
        << vehicle.getCategory()
        << endl;

    cout << "Daily rate: "
        << vehicle.getDailyRate()
        << endl;

    cout << endl;

    cout << "customer: "
        << customer.getName()
        << endl;

    cout << "email: "
        << customer.getEmail()
        << endl;

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
