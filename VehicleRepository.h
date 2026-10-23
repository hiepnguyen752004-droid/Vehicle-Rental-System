#pragma once

#ifndef VEHICLEREPOSITORY_H
#define VEHICLEREPOSITORY_H

#include <string>
#include "InventoryManager.h"

using namespace std;

class VehicleRepository
{
public:
    bool loadVehicles(
        InventoryManager& inventory,
        const string& password
    );
};

#endif
