#pragma once
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>

using namespace std;

class Customer
{
private:
    int customerId;
    string name;
    string email;
    string password;
    string phone;

public:
    Customer();

    Customer(
        int customerId,
        string name,
        string email,
        string password,
        string phone
    );

    int getCustomerId() const;
    string getName() const;
    string getEmail() const;
    string getPassword() const;
    string getPhone() const;

    void setCustomerId(int customerId);
    void setName(string name);
    void setEmail(string email);
    void setPassword(string password);
    void setPhone(string phone);
};

#endif
