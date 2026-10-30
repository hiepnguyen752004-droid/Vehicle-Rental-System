
#include "ReservationManager.h"
#include "ReservationRepository.h"
#include "PaymentCalculator.h"

#include <iostream>
#include <exception>
#include <vector>

using namespace std;

namespace
{
    void printReservations(
        const vector<Reservation>& reservations)
    {
        if (reservations.empty())
        {
            cout << "No reservations found.\n";
            return;
        }

        

        for (const Reservation& reservation : reservations)
        {
            cout << "Reservation ID: "
                << reservation.getReservationId()
                << '\n';

            cout << "Customer ID: "
                << reservation.getCustomerId()
                << '\n';

            cout << "Vehicle ID: "
                << reservation.getVehicleId()
                << '\n';

            cout << "Start Date: "
                << reservation.getStartDate()
                << '\n';

            cout << "End Date: "
                << reservation.getEndDate()
                << '\n';

            cout << "Total Cost: $"
                << reservation.getTotalCost()
                << '\n';

            cout << "Status: "
                << reservation.getStatus()
                << '\n';

            
        }
    }
}




ReservationManager::ReservationManager(
    ReservationRepository& repo)
    : repository(repo)
{
}




bool ReservationManager::checkAvailability(
    int vehicleId,
    string startDate,
    string endDate) const
{
    if (PaymentCalculator::calculateRentalDays(
        startDate, endDate) <= 0)
    {
        cout << "Invalid rental dates.\n";
        return false;
    }

    try
    {
        return repository.checkAvailability(
            vehicleId,
            startDate,
            endDate
        );
    }
    catch (const exception& error)
    {
        cout << "Availability check failed: "
            << error.what()
            << '\n';

        return false;
    }
}




bool ReservationManager::createReservation(
    Reservation& reservation)
{
    if (PaymentCalculator::calculateRentalDays(
        reservation.getStartDate(),
        reservation.getEndDate()) <= 0)
    {
        cout << "Invalid rental dates.\n";
        return false;
    }

    try
    {
        if (repository.createReservation(reservation))
        {
            cout << "Reservation saved to MySQL.\n";
            return true;
        }

        cout << "Reservation not created "
            << "(unavailable or conflict).\n";

        return false;
    }
    catch (const exception& error)
    {
        cout << "Reservation error: "
            << error.what()
            << '\n';

        return false;
    }
}




void ReservationManager::cancelReservation(
    int reservationId)
{
    try
    {
        if (repository.cancelReservation(reservationId))
        {
            cout << "Reservation cancelled.\n";
        }
        else
        {
            cout << "Reservation not found "
                << "or already cancelled.\n";
        }
    }
    catch (const exception& error)
    {
        cout << "Cancellation error: "
            << error.what()
            << '\n';
    }
}




void ReservationManager::cancelCustomerReservation(
    int reservationId,
    int customerId)
{
    try
    {
        if (repository.cancelReservationForCustomer(
            reservationId,
            customerId))
        {
            cout << "Your reservation was cancelled.\n";
        }
        else
        {
            cout << "Reservation not found in your account "
                << "or already cancelled.\n";
        }
    }
    catch (const exception& error)
    {
        cout << "Cancellation error: "
            << error.what()
            << '\n';
    }
}


// Manager views all reservations

void ReservationManager::viewReservations() const
{
    try
    {
        printReservations(
            repository.getReservations()
        );
    }
    catch (const exception& error)
    {
        cout << "Unable to view reservations: "
            << error.what()
            << '\n';
    }
}


// Customer views personal reservations

void ReservationManager::viewCustomerReservations(
    int customerId) const
{
    try
    {
        printReservations(
            repository.getReservationsForCustomer(
                customerId
            )
        );
    }
    catch (const exception& error)
    {
        cout << "Unable to view your reservations: "
            << error.what()
            << '\n';
    }
}