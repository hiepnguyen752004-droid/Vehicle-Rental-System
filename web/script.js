
/* ==========================================
   QUICKROUTE RENTALS
   CUSTOMER VEHICLE INVENTORY AND SEARCH
========================================== */

"use strict";

const getElement = id =>
    document.getElementById(id);

const currency = amount =>
    "$" + Number(amount).toFixed(2);

let activeSearch = null;


// ==========================================
// READ SEARCH FORM
// ==========================================

function getSearchParameters() {

    const parameters = new URLSearchParams();

    const filters = {
        search: getElement("search-input").value.trim(),
        make: getElement("make-filter").value,
        category: getElement("category-filter").value,
        year: getElement("year-filter").value,
        status: getElement("status-filter").value,
        minPrice: getElement("min-price").value,
        maxPrice: getElement("max-price").value,
        sort: getElement("sort-filter").value
    };

    for (const [key, value] of Object.entries(filters)) {

        if (value !== "") {
            parameters.set(key, value);
        }
    }

    return parameters;
}


// ==========================================
// ADD AN OPTION TO A DROPDOWN
// ==========================================

function addOption(select, value) {

    const exists = Array.from(select.options).some(
        option => option.value.toLowerCase() ===
                  String(value).toLowerCase()
    );

    if (exists) {
        return;
    }

    const option = document.createElement("option");

    option.value = value;
    option.textContent = value;

    select.appendChild(option);
}


// ==========================================
// LOAD MAKE AND CATEGORY OPTIONS
// ==========================================

async function loadFilterOptions() {

    try {

        const response = await fetch(
            "/api/vehicles?sort=id_asc",
            {
                cache: "no-store"
            }
        );

        if (!response.ok) {
            throw new Error("Could not load filters");
        }

        const data = await response.json();

        if (data.status !== "success") {
            throw new Error("Vehicle API error");
        }

        const vehicles = data.vehicles || [];

        const makes = new Set();
        const categories = new Set();
        const statuses = new Set();

        vehicles.forEach(vehicle => {

            if (vehicle.make) {
                makes.add(vehicle.make);
            }

            if (vehicle.category) {
                categories.add(vehicle.category);
            }

            if (vehicle.status) {
                statuses.add(vehicle.status);
            }
        });

        const makeSelect = getElement("make-filter");
        const categorySelect = getElement("category-filter");
        const statusSelect = getElement("status-filter");

        Array.from(makes)
            .sort()
            .forEach(make => addOption(makeSelect, make));

        Array.from(categories)
            .sort()
            .forEach(category =>
                addOption(categorySelect, category)
            );

        Array.from(statuses)
            .sort()
            .forEach(status =>
                addOption(statusSelect, status)
            );

    }
    catch (error) {

        console.warn(
            "Unable to load dropdown options:",
            error
        );
    }
}


// ==========================================
// VEHICLE PHOTO
// ==========================================

function createVehiclePhoto(vehicle) {

    const container = document.createElement("div");

    container.className = "qr-vehicle-image";

    if (!vehicle.imageUrl) {

        container.textContent =
            "No photograph available";

        return container;
    }

    const image = document.createElement("img");

    image.src = vehicle.imageUrl;

    image.alt =
        `${vehicle.year} ${vehicle.make} ${vehicle.model}`;

    image.loading = "lazy";

    image.addEventListener("error", () => {

        container.replaceChildren();

        container.textContent =
            "Photograph unavailable";
    });

    container.appendChild(image);

    return container;
}


// ==========================================
// ADD A TEXT ELEMENT
// ==========================================

function addText(parent, tag, value, className = "") {

    const element = document.createElement(tag);

    element.textContent = value;

    if (className) {
        element.className = className;
    }

    parent.appendChild(element);

    return element;
}


// ==========================================
// CREATE VEHICLE CARD
// ==========================================

function createVehicleCard(vehicle) {

    const card = document.createElement("article");

    card.className = "qr-vehicle-card";

    const reservationUrl =
        `vehicle.html?id=${encodeURIComponent(vehicle.id)}`;


    // VEHICLE PHOTO

    card.appendChild(
        createVehiclePhoto(vehicle)
    );


    // VEHICLE TITLE

    addText(
        card,
        "h3",
        `${vehicle.year} ${vehicle.make} ${vehicle.model}`
    );


    // VEHICLE ID

    addText(
        card,
        "p",
        `Vehicle ID: ${vehicle.id}`
    );


    // CATEGORY

    addText(
        card,
        "p",
        `Category: ${vehicle.category}`
    );


    // DAILY RATE

    addText(
        card,
        "p",
        `${currency(vehicle.dailyRate)} / day`,
        "qr-price"
    );


    // AVAILABILITY STATUS

    const status = addText(
        card,
        "span",
        vehicle.status,
        "qr-status"
    );

    if (vehicle.status !== "Available") {
        status.classList.add("maintenance");
    }


    // RESERVATION LINK

    const link = document.createElement("a");

    link.href = reservationUrl;

    link.className = "qr-card-link";

    link.textContent =
        vehicle.status === "Available"
            ? "View & Reserve"
            : "View Vehicle Details";

    card.appendChild(link);


    // ALLOW CLICKING ANYWHERE ON THE CARD

    card.addEventListener("click", event => {

        if (event.target.closest("a,button,input,select")) {
            return;
        }

        window.location.assign(reservationUrl);
    });


    // KEYBOARD ACCESSIBILITY

    card.tabIndex = 0;

    card.addEventListener("keydown", event => {

        if (event.target !== card) {
            return;
        }

        if (event.key === "Enter") {

            window.location.assign(reservationUrl);
        }
    });

    return card;
}


// ==========================================
// DISPLAY SEARCH RESULTS
// ==========================================

function displayVehicles(vehicles) {

    const container = getElement("vehicle-list");

    container.replaceChildren();

    getElement("vehicle-count").textContent =
        `${vehicles.length} vehicles found`;

    getElement("vehicle-message").textContent = "";

    if (vehicles.length === 0) {

        addText(
            container,
            "p",
            "No vehicles match your search. Try different filters."
        );

        return;
    }

    const fragment = document.createDocumentFragment();

    vehicles.forEach(vehicle => {

        fragment.appendChild(
            createVehicleCard(vehicle)
        );
    });

    container.appendChild(fragment);
}


// ==========================================
// LOAD VEHICLES FROM C++ CROW AND MYSQL
// ==========================================

async function loadVehicles() {

    // Cancel previous request if a new search begins.

    if (activeSearch) {
        activeSearch.abort();
    }

    const controller = new AbortController();

    activeSearch = controller;

    const container = getElement("vehicle-list");

    getElement("vehicle-count").textContent =
        "Loading vehicles...";

    getElement("vehicle-message").textContent = "";

    const parameters = getSearchParameters();

    const url =
        "/api/vehicles?" + parameters.toString();

    try {

        const response = await fetch(url, {

            cache: "no-store",

            signal: controller.signal

        });

        const data = await response.json();

        if (!response.ok ||
            data.status !== "success") {

            throw new Error(
                data.message ||
                "Unable to retrieve vehicles"
            );
        }

        if (activeSearch !== controller) {
            return;
        }

        displayVehicles(data.vehicles || []);

    }
    catch (error) {

        if (error.name === "AbortError") {
            return;
        }

        if (activeSearch !== controller) {
            return;
        }

        container.replaceChildren();

        getElement("vehicle-count").textContent =
            "Unable to load vehicles";

        getElement("vehicle-message").textContent =
            error.message +
            ". Check that the C++ Crow server and MySQL are running.";

        console.error(
            "Vehicle search error:",
            error
        );
    }
    finally {

        if (activeSearch === controller) {
            activeSearch = null;
        }
    }
}


// ==========================================
// SEARCH FORM SUBMISSION
// ==========================================

function setupSearch() {

    const form = getElement("vehicle-search-form");

    form.addEventListener("submit", event => {

        event.preventDefault();

        loadVehicles();
    });


    // SHOW ALL VEHICLES

    getElement("reset-button").addEventListener(
        "click",
        () => {

            form.reset();

            loadVehicles();
        }
    );
}


// ==========================================
// CHECK CUSTOMER / MANAGER LOGIN
// ==========================================

async function updateNavigation() {

    const accountLink =
        getElement("account-nav-link");

    const managerLink =
        getElement("manager-nav-link");

    try {

        const response = await fetch(
            "/api/auth/me",
            {
                credentials: "same-origin",
                cache: "no-store"
            }
        );

        if (!response.ok) {
            return;
        }

        const account = await response.json();

        if (account.status !== "success") {
            return;
        }

        accountLink.textContent = "My Account";

        managerLink.hidden =
            account.role !== "Manager";

    }
    catch (error) {

        console.warn(
            "Login session unavailable:",
            error
        );
    }
}


// ==========================================
// START HOMEPAGE
// ==========================================

async function initializeHomepage() {

    setupSearch();

    await Promise.all([
        loadFilterOptions(),
        loadVehicles(),
        updateNavigation()
    ]);
}

initializeHomepage();
