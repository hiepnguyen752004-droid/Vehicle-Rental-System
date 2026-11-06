


/* QUICKROUTE RENTALS - COMPLETE VEHICLE SEARCH */

const searchForm =
    document.getElementById("vehicle-search-form");

const vehicleList =
    document.getElementById("vehicle-list");

const vehicleCount =
    document.getElementById("vehicle-count");

let latestRequest = 0;

let filtersLoaded = false;


// CREATE DYNAMIC DROPDOWN OPTIONS

function fillDropdown(selectId, values) {

    const select =
        document.getElementById(selectId);

    // PRESERVE THE FIRST "ALL" OPTION

    while (select.options.length > 1) {

        select.remove(1);
    }

    const uniqueValues = [
        ...new Set(values.filter(Boolean))
    ];

    uniqueValues.sort((a, b) =>
        a.localeCompare(b)
    );

    for (const value of uniqueValues) {

        const option =
            document.createElement("option");

        option.value = value;
        option.textContent = value;

        select.appendChild(option);
    }
}


// BUILD FILTER OPTIONS FROM MYSQL RECORDS

function initializeFilters(vehicles) {

    const makes = vehicles.map(
        vehicle => vehicle.make
    );

    const categories = vehicles.map(
        vehicle => vehicle.category
    );

    const statuses = vehicles.map(
        vehicle => vehicle.status
    );

    fillDropdown("make-filter", makes);

    fillDropdown("category-filter", categories);

    fillDropdown("status-filter", statuses);

    filtersLoaded = true;
}


// SHOW A MESSAGE

function showVehicleMessage(message) {

    vehicleList.replaceChildren();

    const paragraph =
        document.createElement("p");

    paragraph.className = "grid-message";

    paragraph.textContent = message;

    vehicleList.appendChild(paragraph);
}


// CREATE VEHICLE CARD

function createVehicleCard(vehicle) {

    const card =
        document.createElement("article");

    card.className = "vehicle-card";


    // OPTIONAL IMAGE SUPPORT FOR FUTURE MYSQL PHOTOS

    if (vehicle.imageUrl) {

        const image = document.createElement("img");

        image.className = "vehicle-image";

        image.src = vehicle.imageUrl;

        image.alt =
            vehicle.year + " " +
            vehicle.make + " " +
            vehicle.model;

        image.loading = "lazy";

        card.appendChild(image);
    }


    // VEHICLE NAME

    const name =
        document.createElement("h3");

    name.textContent =
        vehicle.year + " " +
        vehicle.make + " " +
        vehicle.model;


    // VEHICLE ID

    const id =
        document.createElement("p");

    id.textContent =
        "Vehicle ID: " + vehicle.id;


    // CATEGORY

    const category =
        document.createElement("p");

    category.textContent =
        "Category: " + vehicle.category;


    // PRICE

    const price =
        document.createElement("p");

    price.className = "vehicle-price";

    price.textContent =
        "$" + Number(vehicle.dailyRate).toFixed(2)
        + " / day";


    // STATUS

    const status =
        document.createElement("span");

    status.className = "vehicle-status";

    status.textContent = vehicle.status;

    if (vehicle.status === "Available") {

        status.classList.add("available");

    } else {

        status.classList.add("unavailable");

    }


    // ADD CONTENT

    card.appendChild(name);

    card.appendChild(id);

    card.appendChild(category);

    card.appendChild(price);

    card.appendChild(status);


    return card;
}


// BUILD API URL FROM ALL FILTERS

function buildApiUrl() {

    const formData = new FormData(searchForm);

    const parameters = new URLSearchParams();

    for (const [key, value] of formData.entries()) {

        const cleanValue = value.trim();

        if (cleanValue !== "") {

            parameters.set(key, cleanValue);
        }
    }

    const query = parameters.toString();

    return "/api/vehicles" +
        (query ? "?" + query : "");
}


// REQUEST FILTERED VEHICLES FROM C++

async function loadVehicles(useFilters = true) {

    const requestNumber = ++latestRequest;

    vehicleCount.textContent =
        "Loading vehicles...";

    showVehicleMessage(
        "Retrieving vehicle records from MySQL..."
    );

    try {

        let apiUrl = "/api/vehicles";

        if (useFilters) {

            apiUrl = buildApiUrl();
        }


        // CALL C++ CROW API

        const response = await fetch(apiUrl);

        const data = await response.json();

        if (!response.ok || data.status !== "success") {

            throw new Error(
                data.message || "Vehicle request failed"
            );
        }


        // IGNORE OUTDATED RESULTS

        if (requestNumber !== latestRequest) {

            return;
        }


        const vehicles = data.vehicles;


        // INITIALIZE FILTERS FROM ALL VEHICLES

        if (!filtersLoaded) {

            initializeFilters(vehicles);
        }


        // CLEAR EXISTING CARDS

        vehicleList.replaceChildren();


        // UPDATE RESULT COUNT

        vehicleCount.textContent =
            data.count +
            (data.count === 1
                ? " vehicle found"
                : " vehicles found");


        // NO MATCHES

        if (vehicles.length === 0) {

            showVehicleMessage(
                "No vehicles match your search. " +
                "Try different filters."
            );

            return;
        }


        // DISPLAY MYSQL RESULTS

        for (const vehicle of vehicles) {

            const card =
                createVehicleCard(vehicle);

            vehicleList.appendChild(card);

        }

    }
    catch (error) {

        if (requestNumber !== latestRequest) {

            return;
        }

        console.error("Vehicle search error:", error);

        vehicleCount.textContent =
            "Unable to load vehicles";

        showVehicleMessage(
            "The vehicle search is unavailable. " +
            "Check that the C++ server and MySQL are running."
        );
    }
}


// SEARCH BUTTON

searchForm.addEventListener(
    "submit",
    function(event) {

        event.preventDefault();

        loadVehicles(true);
    }
);


// RESET ALL FILTERS

document.getElementById("reset-button")
    .addEventListener("click", function() {

        searchForm.reset();

        loadVehicles(false);
    }
);


// HERO BROWSE BUTTON

function showMessage() {

    document.getElementById("vehicles")
        .scrollIntoView({
            behavior: "smooth"
        });
}


// LOAD THE COMPLETE INVENTORY ON PAGE START

loadVehicles(false);
