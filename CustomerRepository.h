#pragma once
#ifndef CUSTOMERREPOSITORY_H
#define CUSTOMERREPOSITORY_H

#include <memory>
#include <string>
#include <jdbc/cppconn/connection.h>

using namespace std;

struct LoggedInCustomer
{
    int customerId = 0;
    string name;
    string role;
};

class CustomerRepository
{
private:
    unique_ptr<sql::Connection> connection;

public:
    CustomerRepository(
        const string& mysqlPassword
    );

    bool registerCustomer(
        const string& name,
        const string& email,
        const string& password,
        const string& phone
    );

    bool login(
        const string& email,
        const string& password,
        LoggedInCustomer& account
    );
};

#endif