

/* QUICKROUTE RENTALS - VEHICLE WEBSITE */


/* RETRIEVE VEHICLES FROM C++ API */

async function displayVehicles() {

    const vehicleList =
        document.getElementById("vehicle-list");

    if (!vehicleList) {
        return;
    }

    vehicleList.replaceChildren();

    // SHOW LOADING MESSAGE

    const loading = document.createElement("p");

    loading.textContent =
        "Loading vehicles from MySQL...";

    vehicleList.appendChild(loading);

    try {

        // REQUEST VEHICLES FROM C++ CROW SERVER

        const response = await fetch("/api/vehicles");

        if (!response.ok) {
            throw new Error("HTTP error: " + response.status);
        }

        // CONVERT JSON RESPONSE

        const data = await response.json();

        if (data.status !== "success") {
            throw new Error("Vehicle API returned an error");
        }

        const vehicles = data.vehicles;

        vehicleList.replaceChildren();

        // DISPLAY ALL VEHICLES

        for (const vehicle of vehicles) {

            // CREATE VEHICLE CARD

            const card = document.createElement("div");

            card.className = "vehicle-card";


            // VEHICLE NAME

            const name = document.createElement("h3");

            name.textContent =
                vehicle.year + " " +
                vehicle.make + " " +
                vehicle.model;


            // VEHICLE ID

            const id = document.createElement("p");

            id.textContent =
                "Vehicle ID: " + vehicle.id;


            // VEHICLE CATEGORY

            const category = document.createElement("p");

            category.textContent =
                "Category: " + vehicle.category;


            // RENTAL PRICE

            const price = document.createElement("p");

            price.className = "vehicle-price";

            price.textContent =
                "$" + Number(vehicle.dailyRate).toFixed(2)
                + " / day";


            // VEHICLE STATUS

            const status = document.createElement("span");

            status.className = "vehicle-status";

            status.textContent = vehicle.status;

            if (vehicle.status === "Available") {

                status.classList.add("available");

            } else {

                status.classList.add("unavailable");

            }


            // ADD CONTENT TO CARD

            card.appendChild(name);
            card.appendChild(id);
            card.appendChild(category);
            card.appendChild(price);
            card.appendChild(status);


            // DISPLAY CARD

            vehicleList.appendChild(card);

        }

        console.log(
            "Vehicles loaded from MySQL:",
            vehicles.length
        );

    }
    catch (error) {

        console.error("API error:", error);

        vehicleList.replaceChildren();

        const message = document.createElement("p");

        message.textContent =
            "Unable to load vehicles. Check that the C++ server is running.";

        vehicleList.appendChild(message);

    }
}


/* BROWSE VEHICLES BUTTON */

function showMessage() {

    document.getElementById("vehicles")
        .scrollIntoView({
            behavior: "smooth"
        });

}


/* LOAD ALL VEHICLES WHEN THE PAGE OPENS */

displayVehicles();
