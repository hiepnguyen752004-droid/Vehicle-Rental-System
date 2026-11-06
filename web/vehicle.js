
/* ==========================================
   QUICKROUTE RENTALS
   CUSTOMER VEHICLE RESERVATION SYSTEM
========================================== */

const el = id => document.getElementById(id);

const money = amount =>
    "$" + Number(amount).toFixed(2);

const params = new URLSearchParams(
    window.location.search
);

const vehicleId = Number(
    params.get("id")
);

let currentVehicle = null;
let loggedIn = false;
let checkedDates = null;


// ==========================================
// DISPLAY MESSAGES
// ==========================================

function showMessage(id, message, error = false) {

    const element = el(id);

    element.textContent = message;

    element.className = error
        ? "message-error"
        : "message-success";
}


// ==========================================
// FETCH API DATA
// ==========================================

async function api(url, options = {}) {

    const response = await fetch(url, {
        credentials: "same-origin",
        cache: "no-store",
        ...options
    });

    let data;

    try {
        data = await response.json();
    }
    catch {
        throw new Error(
            "Server returned an invalid response."
        );
    }

    if (!response.ok ||
        data.status !== "success") {

        throw new Error(
            data.message || "Request failed."
        );
    }

    return data;
}


// ==========================================
// LOCAL DATE
// ==========================================

function todayString() {

    const date = new Date();

    const year = date.getFullYear();

    const month = String(
        date.getMonth() + 1
    ).padStart(2, "0");

    const day = String(
        date.getDate()
    ).padStart(2, "0");

    return `${year}-${month}-${day}`;
}


// ==========================================
// LOGIN PAGE WITH RETURN LINK
// ==========================================

function loginUrl() {

    const returnPath =
        `/vehicle.html?id=${vehicleId}`;

    return "/login.html?return=" +
        encodeURIComponent(returnPath);
}


// ==========================================
// DISPLAY SELECTED MAIN PHOTO
// ==========================================

function showMainPhoto(url) {

    const holder = el("primary-photo");

    holder.replaceChildren();

    if (!url) {

        holder.textContent =
            "No photograph available";

        return;
    }

    const image = document.createElement("img");

    image.src = url;

    image.alt = currentVehicle
        ? `${currentVehicle.make} ${currentVehicle.model}`
        : "Rental vehicle";

    image.onerror = () => {

        holder.replaceChildren();

        holder.textContent =
            "Photograph unavailable";
    };

    holder.appendChild(image);
}


// ==========================================
// LOAD VEHICLE DETAILS
// ==========================================

async function loadVehicle() {

    if (!Number.isInteger(vehicleId) ||
        vehicleId <= 0) {

        showMessage(
            "page-message",
            "Invalid vehicle ID.",
            true
        );

        return;
    }

    try {

        const data = await api(
            "/api/vehicles?sort=id_asc"
        );

        currentVehicle = data.vehicles.find(
            vehicle => Number(vehicle.id) === vehicleId
        );

        if (!currentVehicle) {

            throw new Error(
                "Vehicle not found."
            );
        }

        const car = currentVehicle;

        el("car-title").textContent =
            `${car.year} ${car.make} ${car.model}`;

        el("car-meta").textContent =
            `Vehicle ID: ${car.id} | ${car.category}`;

        el("car-price").textContent =
            `${money(car.dailyRate)} / day`;

        el("vehicle-status").textContent =
            `Status: ${car.status}`;

        document.title =
            `${car.make} ${car.model} - QuickRoute Rentals`;

        el("vehicle-content").hidden = false;

        showMessage("page-message", "");

        showMainPhoto(car.imageUrl);


        // LOAD VEHICLE PHOTO GALLERY

        const gallery = await api(
            `/api/vehicles/${vehicleId}/images`
        );

        const thumbnails =
            el("photo-thumbnails");

        thumbnails.replaceChildren();

        for (const photo of gallery.images || []) {

            const button =
                document.createElement("button");

            button.type = "button";

            const image =
                document.createElement("img");

            image.src = photo.imageUrl;

            image.alt =
                `${car.make} ${car.model} photo`;

            button.appendChild(image);

            button.addEventListener(
                "click",
                () => showMainPhoto(photo.imageUrl)
            );

            thumbnails.appendChild(button);
        }

    }
    catch (error) {

        // Preserve vehicle details if only
        // the optional gallery request failed.

        if (currentVehicle) {
            console.warn(
                "Vehicle gallery unavailable:",
                error.message
            );
        }
        else {
            showMessage(
                "page-message",
                error.message,
                true
            );
        }
    }
}


// ==========================================
// CHECK CUSTOMER LOGIN
// ==========================================

async function checkLogin() {

    try {

        const account = await api(
            "/api/auth/me"
        );

        loggedIn = true;

        el("account-link").textContent =
            "My Account";

        el("manager-link").hidden =
            account.role !== "Manager";

        el("login-notice").hidden = true;

    }
    catch {

        loggedIn = false;

        el("account-link").textContent =
            "Login";

        el("login-notice").hidden = false;

        const notice = el("login-notice");

        notice.replaceChildren();

        const link =
            document.createElement("a");

        link.href = loginUrl();

        link.textContent =
            "Log in to confirm this reservation";

        notice.appendChild(link);
    }
}


// ==========================================
// RESET QUOTE WHEN DATES CHANGE
// ==========================================

function resetQuote() {

    checkedDates = null;

    el("quote-panel").hidden = true;

    el("reserve-button").disabled = true;

    showMessage("booking-message", "");
}


// ==========================================
// CHECK DATES AND GET PRICE
// ==========================================

async function checkAvailability() {

    resetQuote();

    const startDate =
        el("pickup-date").value;

    const endDate =
        el("return-date").value;

    if (!startDate || !endDate) {

        showMessage(
            "booking-message",
            "Please choose pickup and return dates.",
            true
        );

        return;
    }

    try {

        showMessage(
            "booking-message",
            "Checking vehicle availability..."
        );

        const url =
            `/api/vehicles/${vehicleId}/quote` +
            `?startDate=${encodeURIComponent(startDate)}` +
            `&endDate=${encodeURIComponent(endDate)}`;

        const data = await api(url);

        el("quote-panel").hidden = false;

        el("quote-daily").textContent =
            money(data.dailyRate);

        el("quote-days").textContent =
            String(data.days);

        el("quote-total").textContent =
            money(data.total);

        el("quote-availability").textContent =
            data.message;

        if (!data.available) {

            showMessage(
                "booking-message",
                "Vehicle unavailable for those dates.",
                true
            );

            return;
        }

        checkedDates = {
            startDate,
            endDate
        };

        el("reserve-button").disabled = false;

        showMessage(
            "booking-message",
            loggedIn
                ? "Vehicle available. Ready to reserve."
                : "Vehicle available. Login required to confirm."
        );

    }
    catch (error) {

        showMessage(
            "booking-message",
            error.message,
            true
        );
    }
}


// ==========================================
// CONFIRM RESERVATION
// ==========================================

async function confirmReservation(event) {

    event.preventDefault();

    if (!checkedDates) {

        showMessage(
            "booking-message",
            "Check vehicle availability first.",
            true
        );

        return;
    }

    const currentStart =
        el("pickup-date").value;

    const currentEnd =
        el("return-date").value;

    if (
        checkedDates.startDate !== currentStart ||
        checkedDates.endDate !== currentEnd
    ) {

        resetQuote();

        showMessage(
            "booking-message",
            "Dates changed. Check availability again.",
            true
        );

        return;
    }

    if (!loggedIn) {

        window.location.assign(loginUrl());

        return;
    }

    const button = el("reserve-button");

    button.disabled = true;

    try {

        const paymentMethod =
            document.querySelector(
                'input[name="payment"]:checked'
            ).value;

        const result = await api(
            "/api/bookings",
            {
                method: "POST",

                headers: {
                    "Content-Type": "application/json"
                },

                body: JSON.stringify({
                    vehicleId: vehicleId,
                    startDate: currentStart,
                    endDate: currentEnd,
                    paymentMethod: paymentMethod
                })
            }
        );

        const confirmation =
            el("booking-confirmation");

        confirmation.hidden = false;

        confirmation.replaceChildren();

        const lines = [
            `Reservation #${result.reservationId} confirmed!`,
            `Pickup: ${result.startDate}`,
            `Return: ${result.endDate}`,
            `Rental days: ${result.days}`,
            `Rental subtotal: ${money(result.total)}`,
            `Payment: ${
                paymentMethod === "CashOnPickup"
                    ? "Cash at pickup"
                    : "Card at pickup"
            }`,
            "Payment has not been collected yet."
        ];

        for (const line of lines) {

            const paragraph =
                document.createElement("p");

            paragraph.textContent = line;

            confirmation.appendChild(paragraph);
        }

        checkedDates = null;

        showMessage(
            "booking-message",
            "Your reservation has been saved."
        );

        await loadMyReservations();

    }
    catch (error) {

        showMessage(
            "booking-message",
            error.message,
            true
        );

        button.disabled = false;
    }
}


// ==========================================
// LOAD CUSTOMER RESERVATIONS
// ==========================================

async function loadMyReservations() {

    const holder =
        el("my-bookings-list");

    holder.replaceChildren();

    if (!loggedIn) {

        el("my-bookings-message").textContent =
            "Log in to view your reservations.";

        return;
    }

    try {

        const data = await api(
            "/api/bookings/my"
        );

        const bookings =
            data.bookings || [];

        el("my-bookings-message").textContent =
            bookings.length === 0
                ? "You have no reservations."
                : `${bookings.length} reservation(s)`;


        for (const booking of bookings) {

            const card =
                document.createElement("div");

            card.className = "reservation-card";


            const information =
                document.createElement("div");

            const heading =
                document.createElement("strong");

            heading.textContent =
                `Reservation #${booking.reservationId}`;

            information.appendChild(heading);


            const lines = [
                `${booking.year} ${booking.make} ${booking.model}`,
                `${booking.startDate} to ${booking.endDate}`,
                `Total: ${money(booking.total)}`,
                `Status: ${booking.reservationStatus}`,
                `Payment: ${booking.paymentStatus}`
            ];

            for (const line of lines) {

                const p =
                    document.createElement("p");

                p.textContent = line;

                information.appendChild(p);
            }

            card.appendChild(information);


            // ALLOW UNPAID CANCELLATIONS

            if (
                booking.reservationStatus !== "Cancelled" &&
                booking.paymentStatus !== "Paid"
            ) {

                const cancel =
                    document.createElement("button");

                cancel.type = "button";

                cancel.className = "cancel-button";

                cancel.textContent =
                    "Cancel Reservation";

                cancel.addEventListener(
                    "click",
                    async () => {

                        if (!confirm(
                            `Cancel reservation #${booking.reservationId}?`
                        )) {
                            return;
                        }

                        cancel.disabled = true;

                        try {

                            await api(
                                `/api/bookings/${booking.reservationId}/cancel`,
                                {
                                    method: "POST"
                                }
                            );

                            await loadMyReservations();

                        }
                        catch (error) {

                            showMessage(
                                "my-bookings-message",
                                error.message,
                                true
                            );

                            cancel.disabled = false;
                        }
                    }
                );

                card.appendChild(cancel);
            }

            holder.appendChild(card);
        }

    }
    catch (error) {

        showMessage(
            "my-bookings-message",
            error.message,
            true
        );
    }
}


// ==========================================
// START PAGE
// ==========================================

async function initializePage() {

    const today = todayString();

    el("pickup-date").min = today;

    el("return-date").min = today;


    el("pickup-date").addEventListener(
        "change",
        () => {

            const pickup =
                el("pickup-date").value;

            el("return-date").min =
                pickup || today;

            resetQuote();
        }
    );


    el("return-date").addEventListener(
        "change",
        resetQuote
    );


    el("check-availability").addEventListener(
        "click",
        checkAvailability
    );


    el("booking-form").addEventListener(
        "submit",
        confirmReservation
    );


    el("refresh-bookings").addEventListener(
        "click",
        loadMyReservations
    );


    await Promise.all([
        loadVehicle(),
        checkLogin()
    ]);

    await loadMyReservations();
}

initializePage();
