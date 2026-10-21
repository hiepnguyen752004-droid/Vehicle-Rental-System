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
    int id,
    string customerName,
    string customerEmail,
    string customerPassword,
    string customerPhone)
{
    customerId = id;
    name = customerName;
    email = customerEmail;
    password = customerPassword;
    phone = customerPhone;
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

void Customer::setCustomerId(int id)
{
    customerId = id;
}

void Customer::setName(string customerName)
{
    name = customerName;
}

void Customer::setEmail(string customerEmail)
{
    email = customerEmail;
}

void Customer::setPassword(string customerPassword)
{
    password = customerPassword;
}

void Customer::setPhone(string customerPhone)
{
    phone = customerPhone;
}
