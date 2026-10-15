#include "PaymentCalculator.h"
#include <cctype>
#include <cmath>
using namespace std;

namespace {

    bool isLeapYear(int year) {
        return (year % 400 == 0) ||
            (year % 4 == 0 && year % 100 != 0);
    }

    int daysInMonth(int year, int month) {
        const int monthDays[12] = {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        if (month == 2 && isLeapYear(year))
            return 29;

        return monthDays[month - 1];
    }

    bool parseDate(
        const string& date,
        int& year,
        int& month,
        int& day
    ) {
        if (date.size() != 10 ||
            date[4] != '-' ||
            date[7] != '-') {
            return false;
        }

        for (int i = 0; i < 10; i++) {
            if (i == 4 || i == 7)
                continue;

            if (!isdigit(
                static_cast<unsigned char>(date[i])
            ))
                return false;
        }

        year = stoi(date.substr(0, 4));
        month = stoi(date.substr(5, 2));
        day = stoi(date.substr(8, 2));

        if (year < 1 || month < 1 || month > 12)
            return false;

        if (day < 1 ||
            day > daysInMonth(year, month))
            return false;

        return true;
    }

    int countDaysToDate(
        int year,
        int month,
        int day
    ) {
        const int daysBeforeMonth[12] = {
            0, 31, 59, 90, 120, 151,
            181, 212, 243, 273, 304, 334
        };

        int completedYears = year - 1;

        int total = completedYears * 365
            + completedYears / 4
            - completedYears / 100
            + completedYears / 400;

        total += daysBeforeMonth[month - 1] + day;

        if (month > 2 && isLeapYear(year))
            total++;

        return total;
    }
}

int PaymentCalculator::calculateRentalDays(
    const string& startDate,
    const string& endDate
) {
    int startYear, startMonth, startDay;
    int endYear, endMonth, endDay;

    if (!parseDate(
        startDate,
        startYear,
        startMonth,
        startDay
    ) ||
        !parseDate(
            endDate,
            endYear,
            endMonth,
            endDay
        )) {
        return -1;
    }

    int start = countDaysToDate(
        startYear, startMonth, startDay
    );

    int end = countDaysToDate(
        endYear, endMonth, endDay
    );

    if (end < start)
        return -1;

    return end - start + 1;
}

double PaymentCalculator::calculateTotalCost(
    double dailyRate,
    int rentalDays
) {
    if (dailyRate < 0 || rentalDays <= 0)
        return -1.0;

    return round(
        dailyRate * rentalDays * 100.0
    ) / 100.0;
}
