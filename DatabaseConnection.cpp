
#include <iostream>
#include <string>
#include <memory>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;

void testDatabaseConnection()
{
    string password;

    cout << "Enter MySQL password: ";
    getline(cin, password);

    try
    {
        // STEP 1: Get the MySQL driver

        sql::Driver* driver =
            sql::mysql::get_driver_instance();


        // STEP 2: Connect to MySQL

        unique_ptr<sql::Connection> connection(
            (*driver).connect(
                "tcp://127.0.0.1:3306",
                "vehicle_app",
                password
            )
        );


        // STEP 3: Select the database

        (*connection).setSchema("vehicle_rental_db");

        cout << "\nConnected to MySQL successfully!"
            << endl;


        // STEP 4: Create SQL statement

        unique_ptr<sql::Statement> statement(
            (*connection).createStatement()
        );


        // STEP 5: Execute SQL query

        unique_ptr<sql::ResultSet> result(
            (*statement).executeQuery(
                "SELECT COUNT(*) FROM vehicles"
            )
        );


        // STEP 6: Display the result

        if ((*result).next())
        {
            int totalVehicles = (*result).getInt(1);

            cout << "Total vehicles in database: "
                << totalVehicles << endl;
        }
    }
    catch (const sql::SQLException& error)
    {
        cout << "\nMySQL connection error: "
            << error.what() << endl;
    }
    catch (const exception& error)
    {
        cout << "\nError: "
            << error.what() << endl;
    }
}
