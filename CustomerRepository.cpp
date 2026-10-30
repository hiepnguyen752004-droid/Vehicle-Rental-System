#include "CustomerRepository.h"
#include "PasswordHasher.h"

#include <iostream>
#include <memory>
#include <stdexcept>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;

CustomerRepository::CustomerRepository(
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

    (*connection).setSchema(
        "vehicle_rental_db"
    );
}

bool CustomerRepository::registerCustomer(
    const string& name,
    const string& email,
    const string& password,
    const string& phone)
{
    if (name.empty() ||
        email.empty() ||
        email.size() > 150 ||
        name.size() > 100 ||
        phone.size() > 30 ||
        email.find('@') == string::npos ||
        password.size() < 12 ||
        password.size() > 128)
    {
        cout << "Invalid information. "
            << "Password must be 12-128 characters.\n";

        return false;
    }

    try
    {
        string passwordHash =
            PasswordHasher::hashPassword(password);

        unique_ptr<sql::PreparedStatement> statement(
            (*connection).prepareStatement(
                "INSERT INTO customers "
                "(name, email, password_hash, phone) "
                "VALUES (?, ?, ?, ?)"
            )
        );

        (*statement).setString(1, name);
        (*statement).setString(2, email);
        (*statement).setString(3, passwordHash);
        (*statement).setString(4, phone);

        return (*statement).executeUpdate() == 1;
    }
    catch (const sql::SQLException& error)
    {
        if (error.getErrorCode() == 1062)
        {
            cout << "This email address "
                << "is already registered.\n";
        }
        else
        {
            cout << "Registration database error: "
                << error.what() << '\n';
        }

        return false;
    }
}

bool CustomerRepository::login(
    const string& email,
    const string& password,
    LoggedInCustomer& account)
{
    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "SELECT customer_id, name, "
            "password_hash, role "
            "FROM customers WHERE email = ?"
        )
    );

    (*statement).setString(1, email);

    unique_ptr<sql::ResultSet> result(
        (*statement).executeQuery()
    );

    if (!(*result).next())
        return false;

    string storedHash =
        (*result).getString("password_hash").c_str();

    if (!PasswordHasher::verifyPassword(
        password, storedHash))
    {
        return false;
    }

    account.customerId =
        (*result).getInt("customer_id");

    account.name =
        (*result).getString("name").c_str();

    account.role =
        (*result).getString("role").c_str();

    return true;
}