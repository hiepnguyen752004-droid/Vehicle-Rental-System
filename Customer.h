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
        int id,
        string customerName,
        string customerEmail,
        string customerPassword,
        string customerPhone
    );

    int getCustomerId() const;
    string getName() const;
    string getEmail() const;
    string getPassword() const;
    string getPhone() const;

    void setCustomerId(int id);
    void setName(string customerName);
    void setEmail(string customerEmail);
    void setPassword(string customerPassword);
    void setPhone(string customerPhone);
};

#endif
