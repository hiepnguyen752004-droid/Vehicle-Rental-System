
#include "ReservationRepository.h"
#include "PaymentCalculator.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>

using namespace std;

namespace
{
    Reservation reservationFromRow(
        sql::ResultSet& result)
    {
        return Reservation(
            result.getInt("reservation_id"),
            result.getInt("customer_id"),
            result.getInt("vehicle_id"),
            result.getString("start_date").c_str(),
            result.getString("end_date").c_str(),
            result.getDouble("total_cost"),
            result.getString("status").c_str()
        );
    }
}


// Connect to database

ReservationRepository::ReservationRepository(
    const string& password)
{
    sql::Driver* driver =
        sql::mysql::get_driver_instance();

    connection.reset(
        (*driver).connect(
            "tcp://127.0.0.1:3306",
            "vehicle_app",
            password
        )
    );

    (*connection).setSchema("vehicle_rental_db");
}




bool ReservationRepository::checkAvailability(
    int vehicleId,
    const string& startDate,
    const string& endDate)
{
    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "SELECT v.vehicle_id FROM vehicles v "
            "WHERE v.vehicle_id = ? "
            "AND v.status = 'Available' "
            "AND NOT EXISTS ("
            "SELECT 1 FROM reservations r "
            "WHERE r.vehicle_id = v.vehicle_id "
            "AND r.status <> 'Cancelled' "
            "AND r.start_date <= ? "
            "AND r.end_date >= ?)"
        )
    );

    (*statement).setInt(1, vehicleId);
    (*statement).setString(2, endDate);
    (*statement).setString(3, startDate);

    unique_ptr<sql::ResultSet> result(
        (*statement).executeQuery()
    );

    return (*result).next();
}




bool ReservationRepository::createReservation(
    Reservation& reservation)
{
    int days =
        PaymentCalculator::calculateRentalDays(
            reservation.getStartDate(),
            reservation.getEndDate()
        );

    if (days <= 0)
        return false;

    (*connection).setAutoCommit(false);

    try
    {
        

        unique_ptr<sql::PreparedStatement> vehicleQuery(
            (*connection).prepareStatement(
                "SELECT status, daily_rate "
                "FROM vehicles "
                "WHERE vehicle_id = ? "
                "FOR UPDATE"
            )
        );

        (*vehicleQuery).setInt(
            1, reservation.getVehicleId()
        );

        unique_ptr<sql::ResultSet> vehicleResult(
            (*vehicleQuery).executeQuery()
        );

        if (!(*vehicleResult).next() ||
            (*vehicleResult).getString("status") != "Available")
        {
            (*connection).rollback();
            (*connection).setAutoCommit(true);

            return false;
        }

        double dailyRate =
            (*vehicleResult).getDouble("daily_rate");

        double currentTotal =
            PaymentCalculator::calculateTotalCost(
                dailyRate, days
            );

        if (currentTotal < 0)
        {
            (*connection).rollback();
            (*connection).setAutoCommit(true);

            return false;
        }

        

        if (currentTotal != reservation.getTotalCost())
        {
            cout << "The rental rate changed. "
                << "Please search again.\n";

            (*connection).rollback();
            (*connection).setAutoCommit(true);

            return false;
        }

        

        unique_ptr<sql::PreparedStatement> conflictQuery(
            (*connection).prepareStatement(
                "SELECT reservation_id "
                "FROM reservations "
                "WHERE vehicle_id = ? "
                "AND status <> 'Cancelled' "
                "AND start_date <= ? "
                "AND end_date >= ? "
                "LIMIT 1 FOR UPDATE"
            )
        );

        (*conflictQuery).setInt(
            1, reservation.getVehicleId()
        );

        (*conflictQuery).setString(
            2, reservation.getEndDate()
        );

        (*conflictQuery).setString(
            3, reservation.getStartDate()
        );

        unique_ptr<sql::ResultSet> conflictResult(
            (*conflictQuery).executeQuery()
        );

        if ((*conflictResult).next())
        {
            (*connection).rollback();
            (*connection).setAutoCommit(true);

            return false;
        }

        

        unique_ptr<sql::PreparedStatement> insertQuery(
            (*connection).prepareStatement(
                "INSERT INTO reservations "
                "(customer_id, vehicle_id, "
                "start_date, end_date, "
                "total_cost, status) "
                "VALUES (?, ?, ?, ?, ?, 'Confirmed')"
            )
        );

        (*insertQuery).setInt(
            1, reservation.getCustomerId()
        );

        (*insertQuery).setInt(
            2, reservation.getVehicleId()
        );

        (*insertQuery).setString(
            3, reservation.getStartDate()
        );

        (*insertQuery).setString(
            4, reservation.getEndDate()
        );

        (*insertQuery).setDouble(
            5, currentTotal
        );

        (*insertQuery).executeUpdate();

        

        unique_ptr<sql::Statement> idQuery(
            (*connection).createStatement()
        );

        unique_ptr<sql::ResultSet> idResult(
            (*idQuery).executeQuery(
                "SELECT LAST_INSERT_ID()"
            )
        );

        if (!(*idResult).next())
        {
            throw runtime_error(
                "Cannot retrieve reservation ID."
            );
        }

        int newId = (*idResult).getInt(1);

        if (newId <= 0)
        {
            throw runtime_error(
                "Invalid new reservation ID."
            );
        }

        

        (*connection).commit();
        (*connection).setAutoCommit(true);

        reservation.setReservationId(newId);

        return true;
    }
    catch (...)
    {
        try
        {
            (*connection).rollback();
            (*connection).setAutoCommit(true);
        }
        catch (...)
        {
        }

        throw;
    }
}




bool ReservationRepository::cancelReservation(
    int reservationId)
{
    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "UPDATE reservations "
            "SET status = 'Cancelled' "
            "WHERE reservation_id = ? "
            "AND status <> 'Cancelled'"
        )
    );

    (*statement).setInt(1, reservationId);

    return (*statement).executeUpdate() > 0;
}




bool ReservationRepository::cancelReservationForCustomer(
    int reservationId,
    int customerId)
{
    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "UPDATE reservations "
            "SET status = 'Cancelled' "
            "WHERE reservation_id = ? "
            "AND customer_id = ? "
            "AND status <> 'Cancelled'"
        )
    );

    (*statement).setInt(1, reservationId);
    (*statement).setInt(2, customerId);

    return (*statement).executeUpdate() > 0;
}



vector<Reservation>
ReservationRepository::getReservations()
{
    vector<Reservation> reservations;

    unique_ptr<sql::Statement> statement(
        (*connection).createStatement()
    );

    unique_ptr<sql::ResultSet> result(
        (*statement).executeQuery(
            "SELECT reservation_id, customer_id, "
            "vehicle_id, start_date, end_date, "
            "total_cost, status "
            "FROM reservations "
            "ORDER BY reservation_id"
        )
    );

    while ((*result).next())
    {
        reservations.push_back(
            reservationFromRow(*result)
        );
    }

    return reservations;
}




vector<Reservation>
ReservationRepository::getReservationsForCustomer(
    int customerId)
{
    vector<Reservation> reservations;

    unique_ptr<sql::PreparedStatement> statement(
        (*connection).prepareStatement(
            "SELECT reservation_id, customer_id, "
            "vehicle_id, start_date, end_date, "
            "total_cost, status "
            "FROM reservations "
            "WHERE customer_id = ? "
            "ORDER BY reservation_id"
        )
    );

    (*statement).setInt(1, customerId);

    unique_ptr<sql::ResultSet> result(
        (*statement).executeQuery()
    );

    while ((*result).next())
    {
        reservations.push_back(
            reservationFromRow(*result)
        );
    }

    return reservations;
}
