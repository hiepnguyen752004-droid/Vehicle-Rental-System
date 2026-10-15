#include <iostream>
#include <iomanip>
#include <limits>
#include <cctype>

#include "Vehicle.h"
#include "Customer.h"
#include "Reservation.h"
#include "InventoryManager.h"
#include "ReservationManager.h"
#include "PaymentCalculator.h"

using namespace std;

int main() {

    

    InventoryManager inventory;

    

    inventory.addVehicle(
        Vehicle( 1, "Toyota", "Camry", 2025,"Sedan", 60, "Available")
    );

    inventory.addVehicle(
        Vehicle(2, "Toyota", "Corolla", 2025,"Sedan", 50, "Available")
    );

    inventory.addVehicle(
        Vehicle( 3, "Toyota", "RAV4", 2024, "SUV", 75, "Available"
        )
    );

    inventory.addVehicle(
        Vehicle( 4, "Honda", "Civic", 2024, "Sedan", 55, "Available")
    );

    inventory.addVehicle(
        Vehicle( 5, "Honda", "Accord", 2025,"Sedan", 65, "Available")
    );

    inventory.addVehicle(
        Vehicle( 6, "Ford", "Explorer", 2025,"SUV", 85, "Available"
        )
    );

    inventory.addVehicle(
        Vehicle( 7, "Chevrolet", "Tahoe", 2024,"SUV", 95, "Available"
        )
    );

    inventory.addVehicle(
        Vehicle(8, "Toyota", "Camry", 2024,"Sedan", 55, "Available"
        )
    );

    inventory.addVehicle(
        Vehicle( 9, "Toyota", "Camry", 2025,"Sedan", 65, "Available")
    );

    inventory.addVehicle(
        Vehicle(10, "BMW", "X5", 2025,"SUV", 120, "Available")
    );

   

    Customer customer(
        1,
        "Austin Customer",
        "Austin@example.com",
        "demo-only",
        "000-000-0000"
    );

  

    ReservationManager reservationManager;

    int nextReservationId = 1;

    cout << fixed << setprecision(2);

    cout << "Vehicle Rental Management System\n";

    cout << "Customer: "
        << customer.getName() << "\n";

  

    while (true) {

        

        cout << "1. Search and reserve vehicle\n";
        cout << "2. View reservations\n";
        cout << "3. Cancel reservation\n";
        cout << "4. Exit\n";

        cout << "Enter option: ";

        int option;

        if (!(cin >> option)) {

            if (cin.eof()) {
                break;
            }

            cin.clear();

            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n'
            );

            cout << "Please enter a number from 1 to 4.\n";

            continue;
        }

       

        if (option == 1) {

            int selectedVehicleId =
                inventory.searchVehicleStepByStep();

            if (selectedVehicleId == -1) {
                continue;
            }

         

            Vehicle selectedVehicle;

            if (!inventory.getVehicleById(
                selectedVehicleId,
                selectedVehicle
            )) {

                cout << "Selected vehicle not found.\n";

                continue;
            }

        

            string startDate;
            string endDate;

            cout << "Enter start date (YYYY-MM-DD): ";
            cin >> startDate;

            cout << "Enter end date (YYYY-MM-DD): ";
            cin >> endDate;

           

            int rentalDays =
                PaymentCalculator::calculateRentalDays(
                    startDate,
                    endDate
                );

            if (rentalDays == -1) {

                cout << "Invalid date range. "
                    << "Use YYYY-MM-DD, "
                    << "with end >= start.\n";

                continue;
            }

            

            if (!reservationManager.checkAvailability(
                selectedVehicleId,
                startDate,
                endDate
            )) {

                cout << "Booking conflict: Vehicle "
                    << selectedVehicleId
                    << " is already reserved "
                    << "for these dates.\n";

                continue;
            }

         

            double totalCost =
                PaymentCalculator::calculateTotalCost(
                    selectedVehicle.getDailyRate(),
                    rentalDays
                );

            if (totalCost < 0) {

                cout << "Unable to calculate "
                    << "the rental cost.\n";

                continue;
            }

        

            

            cout << "Vehicle: "
                << selectedVehicle.getYear() << " "
                << selectedVehicle.getMake() << " "
                << selectedVehicle.getModel() << "\n";

            cout << "Daily rate: $"
                << selectedVehicle.getDailyRate() << "\n";

            cout << "Rental days: "
                << rentalDays << "\n";

            cout << "Total cost: $"
                << totalCost << "\n";



            char confirm;

            cout << "Confirm reservation? (Y/N): ";
            cin >> confirm;

            if (toupper(
                static_cast<unsigned char>(confirm)
            ) != 'Y') {

                cout << "Reservation not created.\n";

                continue;
            }

           

            Reservation reservation(
                nextReservationId,
                customer.getCustomerId(),
                selectedVehicleId,
                startDate,
                endDate,
                totalCost,
                "Confirmed"
            );

            

            if (reservationManager.createReservation(
                reservation
            )) {

                cout << "Your reservation ID is "
                    << nextReservationId
                    << ".\n";

                nextReservationId++;
            }
        }

        

        else if (option == 2) {

            reservationManager.viewReservations();
        }


        else if (option == 3) {

            int reservationId;

            cout << "Enter Reservation ID to cancel: ";

            if (!(cin >> reservationId)) {

                if (cin.eof()) {
                    break;
                }

                cin.clear();

                cin.ignore(
                    numeric_limits<streamsize>::max(),
                    '\n'
                );

                cout << "Invalid Reservation ID.\n";

                continue;
            }

            reservationManager.cancelReservation(
                reservationId
            );
        }

       

        else if (option == 4) {

            cout << "end";

            break;
        }

        else {

            cout << "Please choose option "
                << "1, 2, 3, or 4.\n";
        }
    }

    return 0;
}
