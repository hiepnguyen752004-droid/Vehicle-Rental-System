#include "ManagerDashboard.h"
#include "ReservationManager.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <memory>
#include <string>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;

namespace
{
    string promptText(const string& prompt)
    {
        cout << prompt;

        string value;
        getline(cin >> ws, value);

        return value;
    }

    bool promptInt(
        const string& prompt,
        int& value)
    {
        cout << prompt;

        if (cin >> value)
            return true;

        cin.clear();

        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n'
        );

        cout << "Please enter a valid number.\n";

        return false;
    }

    bool promptDouble(
        const string& prompt,
        double& value)
    {
        cout << prompt;

        if (cin >> value)
            return true;

        cin.clear();

        cin.ignore(
            numeric_limits<streamsize>::max(),
            '\n'
        );

        cout << "Please enter a valid price.\n";

        return false;
    }
}




ManagerDashboard::ManagerDashboard(
    const string& mysqlPassword)
{
    sql::Driver* driver =
        sql::mysql::get_driver_instance();

    connection.reset(
        (*driver).connect(
            "tcp://127.0.0.1:3306",
            "vehicle_app",
            mysqlPassword
        )
    );

    (*connection).setSchema("vehicle_rental_db");
}


// View all vehicles

void ManagerDashboard::listVehicles()
{
    unique_ptr<sql::Statement> statement(
        (*connection).createStatement()
    );

    unique_ptr<sql::ResultSet> result(
        (*statement).executeQuery(
            "SELECT vehicle_id, make, model, "
            "year, category, daily_rate, status "
            "FROM vehicles ORDER BY vehicle_id"
        )
    );

    
    cout << fixed << setprecision(2);

    while ((*result).next())
    {
        cout << "ID "
            << (*result).getInt("vehicle_id")
            << " | "
            << (*result).getInt("year")
            << " "
            << (*result).getString("make").c_str()
            << " "
            << (*result).getString("model").c_str()
            << " | "
            << (*result).getString("category").c_str()
            << " | $"
            << (*result).getDouble("daily_rate")
            << " | "
            << (*result).getString("status").c_str()
            << '\n';
    }
}




void ManagerDashboard::addVehicle()
{
    string make = promptText("Make: ");
    string model = promptText("Model: ");

    int year;

    if (!promptInt("Year: ", year))
        return;

    string category = promptText("Category: ");

    double dailyRate;

    if (!promptDouble("Daily rate: $", dailyRate))
        return;

    if (make.empty() ||
        make.size() > 50 ||
        model.empty() ||
        model.size() > 50 ||
        category.empty() ||
        category.size() > 50 ||
        year < 1900 ||
        year > 2100 ||
        !(dailyRate > 0) ||
        dailyRate > 99999999.99)
    {
        cout << "Invalid vehicle information.\n";
        return;
    }

    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "INSERT INTO vehicles "
            "(make, model, year, category, "
            "daily_rate, status) "
            "VALUES (?, ?, ?, ?, ?, 'Available')"
        )
    );

    (*statement).setString(1, make);
    (*statement).setString(2, model);
    (*statement).setInt(3, year);
    (*statement).setString(4, category);
    (*statement).setDouble(5, dailyRate);

    if ((*statement).executeUpdate() == 1)
    {
        cout << "Vehicle added to MySQL.\n";
    }
    else
    {
        cout << "Vehicle was not added.\n";
    }
}




void ManagerDashboard::changeDailyRate()
{
    int id;
    double rate;

    if (!promptInt("Vehicle ID: ", id))
        return;

    if (!promptDouble("New daily rate: $", rate))
        return;

    if (id <= 0 ||
        !(rate > 0) ||
        rate > 99999999.99)
    {
        cout << "Invalid ID or price.\n";
        return;
    }

    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "UPDATE vehicles "
            "SET daily_rate = ? "
            "WHERE vehicle_id = ?"
        )
    );

    (*statement).setDouble(1, rate);
    (*statement).setInt(2, id);

    if ((*statement).executeUpdate() > 0)
    {
        cout << "Daily rate updated.\n";
    }
    else
    {
        cout << "Vehicle not found "
            << "or rate unchanged.\n";
    }
}




void ManagerDashboard::changeVehicleStatus()
{
    int id;

    if (!promptInt("Vehicle ID: ", id))
        return;

    cout << "1. Available\n";
    cout << "2. Maintenance\n";

    int option;

    if (!promptInt("New status: ", option))
        return;

    if (option != 1 && option != 2)
    {
        cout << "Invalid status.\n";
        return;
    }

    string newStatus =
        option == 1 ? "Available" : "Maintenance";

    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "UPDATE vehicles "
            "SET status = ? "
            "WHERE vehicle_id = ?"
        )
    );

    (*statement).setString(1, newStatus);
    (*statement).setInt(2, id);

    if ((*statement).executeUpdate() > 0)
    {
        cout << "Vehicle status updated.\n";
    }
    else
    {
        cout << "Vehicle not found "
            << "or status unchanged.\n";
    }
}




void ManagerDashboard::run(
    ReservationManager& reservationManager)
{
    while (true)
    {
        

        cout << "1. View all vehicles\n";
        cout << "2. Add vehicle\n";
        cout << "3. Update daily rental rate\n";
        cout << "4. Change vehicle status\n";
        cout << "5. View all reservations\n";
        cout << "6. Cancel any reservation\n";
        cout << "7. Log out\n";

        int option;

        if (!promptInt("Enter option: ", option))
            continue;

        if (option == 7)
            return;

        try
        {
            if (option == 1)
            {
                listVehicles();
            }
            else if (option == 2)
            {
                addVehicle();
            }
            else if (option == 3)
            {
                changeDailyRate();
            }
            else if (option == 4)
            {
                changeVehicleStatus();
            }
            else if (option == 5)
            {
                reservationManager.viewReservations();
            }
            else if (option == 6)
            {
                int reservationId;

                if (promptInt(
                    "Reservation ID to cancel: ",
                    reservationId))
                {
                    reservationManager.cancelReservation(
                        reservationId
                    );
                }
            }
            else
            {
                cout << "Choose a number from 1 to 7.\n";
            }
        }
        catch (const exception& error)
        {
            cout << "Dashboard error: "
                << error.what()
                << '\n';
        }
    }
}