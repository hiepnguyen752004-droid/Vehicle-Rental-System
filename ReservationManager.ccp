#include <iostream>
#include "ReservationManager.h"

using namespace std;


bool ReservationManager::checkAvailability(
    int vehicleId,
    string startDate,
    string endDate) const
{
    for (const Reservation& reservation : reservations)
    {
        if (reservation.getVehicleId() == vehicleId && reservation.getStatus() != "Cancelled")
        {
            if (startDate <= reservation.getEndDate() &&
                endDate >= reservation.getStartDate())
            {
                return false;
            }
        }
    }

    return true;
}


bool ReservationManager::createReservation(
    Reservation reservation)
{
    if (reservation.getStartDate() >
        reservation.getEndDate())
    {
        cout << endl;
        cout << "Invalid rental dates." << endl;
        cout << "Start date cannot be after end date."
            << endl;

        return false;
    }


    bool available = checkAvailability(
        reservation.getVehicleId(),
        reservation.getStartDate(),
        reservation.getEndDate()
    );


    if (available == false)
    {
        cout << endl;

        cout << "Reservation cannot be created."
            << endl;

        cout << "Vehicle is already reserved "
            << "during these dates."
            << endl;

        return false;
    }


    reservations.push_back(reservation);


    cout << endl;
    cout << "Reservation created successfully."
        << endl;


    return true;
}


void ReservationManager::cancelReservation(
    int reservationId)
{
    for (Reservation& reservation : reservations)
    {
        if (reservation.getReservationId() ==
            reservationId)
        {
            reservation.setStatus("Cancelled");

            cout << endl;

            cout << "Reservation cancelled successfully."
                << endl;

            return;
        }
    }


    cout << endl;
    cout << "Reservation not found." << endl;
}


void ReservationManager::viewReservations() const
{
    if (reservations.empty())
    {
        cout << endl;
        cout << "No reservations found." << endl;

        return;
    }


    cout << endl;
    cout << "RESERVATIONS" << endl;



    for (const Reservation& reservation : reservations)
    {
        cout << "Reservation ID: "
            << reservation.getReservationId()
            << endl;

        cout << "Customer ID: "
            << reservation.getCustomerId()
            << endl;

        cout << "Vehicle ID: "
            << reservation.getVehicleId()
            << endl;

        cout << "Rental Dates: "
            << reservation.getStartDate()
            << " to "
            << reservation.getEndDate()
            << endl;

        cout << "Total Cost: $"
            << reservation.getTotalCost()
            << endl;

        cout << "Status: "
            << reservation.getStatus()
            << endl;

        
          
    }
}
