
#include <iostream>
#include <string>
#include <memory>

#include "VehicleRepository.h"
#include "Vehicle.h"

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;

bool VehicleRepository::loadVehicles(
    InventoryManager& inventory,
    const string& password)
{
    try
    {
        

        sql::Driver* driver =
            sql::mysql::get_driver_instance();

        unique_ptr<sql::Connection> connection(
            (*driver).connect(
                "tcp://127.0.0.1:3306",
                "vehicle_app",
                password
            )
        );

        (*connection).setSchema("vehicle_rental_db");

        cout << "Connected to MySQL successfully"
            << endl;


        // STEP 2: Retrieve all vehicles

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


        // STEP 3: Convert database rows to Vehicle objects

        int count = 0;

        while ((*result).next())
        {
            int id =
                (*result).getInt("vehicle_id");

            string make =
                (*result).getString("make").c_str();

            string model =
                (*result).getString("model").c_str();

            int year =
                (*result).getInt("year");

            string category =
                (*result).getString("category").c_str();

            double dailyRate =
                (*result).getDouble("daily_rate");

            string status =
                (*result).getString("status").c_str();


            Vehicle vehicle(
                id,
                make,
                model,
                year,
                category,
                dailyRate,
                status
            );


            // STEP 4: Add vehicle to the hash map

            inventory.addVehicle(vehicle);

            count++;
        }


        cout << "Vehicles loaded from MySQL: "
            << count << endl;

        return true;
    }
    catch (const sql::SQLException& error)
    {
        cout << "MySQL error: "
            << error.what() << endl;

        return false;
    }
    catch (const exception& error)
    {
        cout << "Error: "
            << error.what() << endl;

        return false;
    }
}
