
/* ==========================================
   QUICKROUTE RENTALS
   MANAGER RESERVATIONS AND PAYMENTS
========================================== */

(function () {

    "use strict";

    const managerContent =
        document.getElementById("manager-content");

    if (!managerContent) {
        return;
    }


    // =====================================
    // ADD TRUCK TO ALL MANAGER DROPDOWNS
    // =====================================

    const dropdownIds = [
        "new-category",
        "edit-category",
        "filter-category"
    ];

    for (const id of dropdownIds) {

        const select =
            document.getElementById(id);

        if (!select) {
            continue;
        }

        const exists =
            Array.from(select.options).some(
                option => option.value === "Truck"
            );

        if (!exists) {

            const option =
                document.createElement("option");

            option.value = "Truck";
            option.textContent = "Truck";

            select.appendChild(option);
        }
    }


    // =====================================
    // CREATE MANAGER RESERVATION PANEL
    // =====================================

    const panel =
        document.createElement("section");

    panel.className = "panel";

    panel.id = "manager-reservations";


    const heading =
        document.createElement("h3");

    heading.textContent =
        "Customer Reservations & Payments";


    const description =
        document.createElement("p");

    description.textContent =
        "View reservations, cancel unpaid bookings, " +
        "and record payments collected at pickup.";


    const refreshButton =
        document.createElement("button");

    refreshButton.type = "button";

    refreshButton.textContent =
        "Refresh Reservations";


    const statusMessage =
        document.createElement("p");

    statusMessage.setAttribute(
        "aria-live",
        "polite"
    );


    const tableHolder =
        document.createElement("div");

    tableHolder.style.overflowX = "auto";
    tableHolder.style.marginTop = "20px";


    panel.append(
        heading,
        description,
        refreshButton,
        statusMessage,
        tableHolder
    );

    managerContent.appendChild(panel);


    // =====================================
    // API FUNCTION
    // =====================================

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
                "Invalid server response."
            );
        }

        if (
            !response.ok ||
            data.status !== "success"
        ) {
            throw new Error(
                data.message || "Request failed."
            );
        }

        return data;
    }


    // =====================================
    // ADD TEXT TABLE CELL
    // =====================================

    function addCell(row, value) {

        const cell =
            document.createElement("td");

        cell.textContent =
            String(value ?? "");

        cell.style.padding = "12px";
        cell.style.borderBottom =
            "1px solid #e3e7ef";

        row.appendChild(cell);

        return cell;
    }


    // =====================================
    // LOAD ALL MYSQL RESERVATIONS
    // =====================================

    async function loadReservations() {

        statusMessage.textContent =
            "Loading reservations...";

        tableHolder.replaceChildren();

        try {

            const data = await api(
                "/api/manager/bookings"
            );

            const bookings =
                data.bookings || [];

            statusMessage.textContent =
                `${bookings.length} reservation(s) found`;


            const table =
                document.createElement("table");

            table.style.width = "100%";
            table.style.minWidth = "850px";
            table.style.borderCollapse = "collapse";


            // TABLE HEADINGS

            const thead =
                document.createElement("thead");

            const headings =
                document.createElement("tr");

            const columns = [
                "Reservation",
                "Customer",
                "Vehicle",
                "Rental Dates",
                "Total",
                "Booking Status",
                "Payment",
                "Actions"
            ];

            for (const title of columns) {

                const cell =
                    document.createElement("th");

                cell.textContent = title;

                cell.style.padding = "12px";
                cell.style.textAlign = "left";

                headings.appendChild(cell);
            }

            thead.appendChild(headings);
            table.appendChild(thead);


            const tbody =
                document.createElement("tbody");


            // RESERVATION RECORDS

            for (const booking of bookings) {

                const row =
                    document.createElement("tr");

                addCell(
                    row,
                    "#" + booking.reservationId
                );

                addCell(
                    row,
                    booking.customerName
                );

                addCell(
                    row,
                    `${booking.year} ${booking.make} ` +
                    `${booking.model} (#${booking.vehicleId})`
                );

                addCell(
                    row,
                    `${booking.startDate} to ${booking.endDate}`
                );

                addCell(
                    row,
                    "$" + Number(
                        booking.total
                    ).toFixed(2)
                );

                addCell(
                    row,
                    booking.reservationStatus
                );

                addCell(
                    row,
                    booking.paymentStatus
                );


                const actions =
                    document.createElement("td");

                actions.style.padding = "12px";


                // MARK PAYMENT RECEIVED

                if (
                    booking.reservationStatus === "Confirmed" &&
                    booking.paymentStatus === "DueAtPickup"
                ) {

                    const paidButton =
                        document.createElement("button");

                    paidButton.type = "button";

                    paidButton.textContent =
                        "Mark Paid";

                    paidButton.style.margin = "3px";


                    paidButton.addEventListener(
                        "click",
                        async () => {

                            const confirmed = confirm(
                                "Only mark this booking paid " +
                                "if the customer has actually " +
                                "paid at pickup. Continue?"
                            );

                            if (!confirmed) {
                                return;
                            }

                            paidButton.disabled = true;

                            try {

                                await api(
                                    `/api/manager/bookings/` +
                                    `${booking.reservationId}/paid`,
                                    {
                                        method: "POST"
                                    }
                                );

                                await loadReservations();

                            }
                            catch (error) {

                                statusMessage.textContent =
                                    error.message;

                                paidButton.disabled = false;
                            }
                        }
                    );

                    actions.appendChild(paidButton);
                }


                // CANCEL UNPAID RESERVATION

                if (
                    booking.reservationStatus !== "Cancelled" &&
                    booking.paymentStatus !== "Paid"
                ) {

                    const cancelButton =
                        document.createElement("button");

                    cancelButton.type = "button";

                    cancelButton.textContent =
                        "Cancel";

                    cancelButton.style.background =
                        "#a83240";

                    cancelButton.style.margin = "3px";


                    cancelButton.addEventListener(
                        "click",
                        async () => {

                            if (!confirm(
                                `Cancel reservation #${booking.reservationId}?`
                            )) {
                                return;
                            }

                            cancelButton.disabled = true;

                            try {

                                await api(
                                    `/api/manager/bookings/` +
                                    `${booking.reservationId}/cancel`,
                                    {
                                        method: "POST"
                                    }
                                );

                                await loadReservations();

                            }
                            catch (error) {

                                statusMessage.textContent =
                                    error.message;

                                cancelButton.disabled = false;
                            }
                        }
                    );

                    actions.appendChild(cancelButton);
                }

                row.appendChild(actions);

                tbody.appendChild(row);
            }

            table.appendChild(tbody);

            tableHolder.appendChild(table);

        }
        catch (error) {

            statusMessage.textContent =
                "Unable to load reservations: " +
                error.message;
        }
    }


    // =====================================
    // REFRESH BUTTON
    // =====================================

    refreshButton.addEventListener(
        "click",
        loadReservations
    );


    // =====================================
    // VERIFY MANAGER BEFORE REQUESTING DATA
    // =====================================

    async function initialize() {

        try {

            await api("/api/manager/me");

            await loadReservations();

        }
        catch {

            statusMessage.textContent =
                "Manager authorization required.";
        }
    }

    initialize();

})();
