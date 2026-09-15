#include "Customer.h"

Customer::Customer()
{
    customerId = 0;
    name = "";
    email = "";
    password = "";
    phone = "";
}

Customer::Customer(
    int customerId,
    string name,
    string email,
    string password,
    string phone)
{
    this->customerId = customerId;
    this->name = name;
    this->email = email;
    this->password = password;
    this->phone = phone;
}

int Customer::getCustomerId() const
{
    return customerId;
}

string Customer::getName() const
{
    return name;
}

string Customer::getEmail() const
{
    return email;
}

string Customer::getPassword() const
{
    return password;
}

string Customer::getPhone() const
{
    return phone;
}

void Customer::setCustomerId(int customerId)
{
    this->customerId = customerId;
}

void Customer::setName(string name)
{
    this->name = name;
}

void Customer::setEmail(string email)
{
    this->email = email;
}

void Customer::setPassword(string password)
{
    this->password = password;
}

void Customer::setPhone(string phone)
{
    this->phone = phone;
}
