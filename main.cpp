


#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <cctype>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

#include "InventoryManager.h"
#include "VehicleRepository.h"
#include "ReservationManager.h"
#include "ReservationRepository.h"
#include "PaymentCalculator.h"
#include "CustomerRepository.h"
#include "ManagerDashboard.h"

using namespace std;

namespace
{
    string askText(const string& question)
    {
        cout << question;

        string answer;
        getline(cin >> ws, answer);

        return answer;
    }


    

    string askPassword(const string& question)
    {
        cout << question;

        HANDLE inputHandle =
            GetStdHandle(STD_INPUT_HANDLE);

        DWORD originalMode = 0;

        bool hasConsole =
            inputHandle != INVALID_HANDLE_VALUE &&
            GetConsoleMode(
                inputHandle, &originalMode
            ) != 0;

        if (hasConsole)
        {
            SetConsoleMode(
                inputHandle,
                originalMode & ~ENABLE_ECHO_INPUT
            );
        }

        string password;
        getline(cin >> ws, password);

        if (hasConsole)
        {
            SetConsoleMode(
                inputHandle,
                originalMode
            );
        }

        cout << '\n';

        return password;
    }


    bool askInt(
        const string& question,
        int& value)
    {
        cout << question;

        if (cin >> value)
            return true;

        cin.clear();

        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n'
        );

        cout << "Invalid number.\n";

        return false;
    }


    

    void customerMenu(
        const LoggedInCustomer& account,
        const string& mysqlPassword,
        VehicleRepository& vehicleRepository,
        ReservationManager& reservationManager)
    {
        cout << "\nWelcome, "
            << account.name << "!\n";

        while (true)
        {
            

            cout << "1. Search and reserve vehicle\n";
            cout << "2. View my reservations\n";
            cout << "3. Cancel my reservation\n";
            cout << "4. Log out\n";

            int option;

            if (!askInt("Enter option: ", option))
                continue;

            if (option == 4)
                return;


            

            if (option == 1)
            {
                InventoryManager inventory;

                if (!vehicleRepository.loadVehicles(
                    inventory, mysqlPassword))
                {
                    cout << "Unable to load vehicles.\n";
                    continue;
                }

                int vehicleId =
                    inventory.searchVehicleStepByStep();

                if (vehicleId == -1)
                    continue;

                Vehicle selectedVehicle;

                if (!inventory.getVehicleById(
                    vehicleId,
                    selectedVehicle))
                {
                    cout << "Vehicle not found.\n";
                    continue;
                }

                string startDate =
                    askText("Start date (YYYY-MM-DD): ");

                string endDate =
                    askText("End date (YYYY-MM-DD): ");

                int rentalDays =
                    PaymentCalculator::calculateRentalDays(
                        startDate,
                        endDate
                    );

                if (rentalDays <= 0)
                {
                    cout << "Invalid rental dates.\n";
                    continue;
                }

                if (!reservationManager.checkAvailability(
                    vehicleId,
                    startDate,
                    endDate))
                {
                    cout << "This vehicle is unavailable "
                        << "for these dates.\n";

                    continue;
                }

                double cost =
                    PaymentCalculator::calculateTotalCost(
                        selectedVehicle.getDailyRate(),
                        rentalDays
                    );

                if (cost < 0)
                {
                    cout << "Cannot calculate "
                        << "the rental cost.\n";

                    continue;
                }

                cout << fixed << setprecision(2);

                

                cout << selectedVehicle.getYear() << " "
                    << selectedVehicle.getMake() << " "
                    << selectedVehicle.getModel() << '\n';

                cout << "Vehicle ID: "
                    << vehicleId << '\n';

                cout << "Rate: $"
                    << selectedVehicle.getDailyRate()
                    << " per day\n";

                cout << "Dates: "
                    << startDate << " through "
                    << endDate << '\n';

                cout << "Days: "
                    << rentalDays << '\n';

                cout << "Total: $"
                    << cost << '\n';

                string confirm =
                    askText("Confirm reservation? (Y/N): ");

                if (confirm.empty() ||
                    toupper(
                        static_cast<unsigned char>(
                            confirm[0]
                            )
                    ) != 'Y')
                {
                    cout << "Reservation not created.\n";
                    continue;
                }

                Reservation reservation(
                    0,
                    account.customerId,
                    vehicleId,
                    startDate,
                    endDate,
                    cost,
                    "Confirmed"
                );

                if (reservationManager.createReservation(
                    reservation))
                {
                    cout << "Your reservation ID is "
                        << reservation.getReservationId()
                        << ".\n";
                }
            }


            

            else if (option == 2)
            {
                reservationManager.viewCustomerReservations(
                    account.customerId
                );
            }


            

            else if (option == 3)
            {
                int reservationId;

                if (askInt(
                    "Your reservation ID to cancel: ",
                    reservationId))
                {
                    reservationManager.cancelCustomerReservation(
                        reservationId,
                        account.customerId
                    );
                }
            }

            else
            {
                cout << "Choose 1, 2, 3, or 4.\n";
            }
        }
    }
}


int main()
{
    try
    {
        

        string mysqlPassword =
            askPassword("Enter vehicle_app MySQL password: ");

        CustomerRepository customerRepository(
            mysqlPassword
        );

        VehicleRepository vehicleRepository;

        ReservationRepository reservationRepository(
            mysqlPassword
        );

        ReservationManager reservationManager(
            reservationRepository
        );

        ManagerDashboard managerDashboard(
            mysqlPassword
        );

        cout << "Connected to vehicle_rental_db.\n";


        

        while (true)
        {
            

            cout << "1. Register customer account\n";
            cout << "2. Log in\n";
            cout << "3. Exit\n";

            int option;

            if (!askInt("Enter option: ", option))
                continue;

            if (option == 3)
                break;


            

            if (option == 1)
            {
                string name =
                    askText("Full name: ");

                string email =
                    askText("Email: ");

                string phone =
                    askText("Phone: ");

                string password =
                    askPassword(
                        "Create password (at least 12 characters): "
                    );

                string confirm =
                    askPassword("Confirm password: ");

                if (password != confirm)
                {
                    cout << "Passwords do not match.\n";
                }
                else if (customerRepository.registerCustomer(
                    name,
                    email,
                    password,
                    phone))
                {
                    cout << "Account registered. "
                        << "You can now log in.\n";
                }

                fill(
                    password.begin(),
                    password.end(),
                    '\0'
                );

                fill(
                    confirm.begin(),
                    confirm.end(),
                    '\0'
                );
            }


            // Login

            else if (option == 2)
            {
                string email =
                    askText("Email: ");

                string password =
                    askPassword("Password: ");

                LoggedInCustomer account;

                bool authenticated =
                    customerRepository.login(
                        email,
                        password,
                        account
                    );

                fill(
                    password.begin(),
                    password.end(),
                    '\0'
                );

                if (!authenticated)
                {
                    cout << "Invalid email or password.\n";
                    continue;
                }

                if (account.role == "Manager")
                {
                    cout << "Manager login successful.\n";

                    managerDashboard.run(
                        reservationManager
                    );
                }
                else if (account.role == "Customer")
                {
                    customerMenu(
                        account,
                        mysqlPassword,
                        vehicleRepository,
                        reservationManager
                    );
                }
                else
                {
                    cout << "Account has an unrecognized role.\n";
                }
            }
            else
            {
                cout << "Choose 1, 2, or 3.\n";
            }
        }
    }
    catch (const exception& error)
    {
        cout << "Application error: "
            << error.what()
            << '\n';

        return 1;
    }

    return 0;
}
