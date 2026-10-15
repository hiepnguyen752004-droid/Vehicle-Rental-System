#pragma once
#ifndef PAYMENTCALCULATOR_H
#define PAYMENTCALCULATOR_H

#include <string>
using namespace std;

class PaymentCalculator {
public:
    static int calculateRentalDays(
        const string& startDate,
        const string& endDate
    );

    static double calculateTotalCost(
        double dailyRate,
        int rentalDays
    );
};

#endif
