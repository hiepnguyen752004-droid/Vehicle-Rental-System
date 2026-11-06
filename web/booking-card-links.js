
/* ==========================================
   QUICKROUTE RENTALS
   CLICKABLE VEHICLE CARDS + TRUCK CATEGORY
========================================== */

(function () {

    "use strict";


    // =====================================
    // ADD TRUCK CATEGORY
    // =====================================

    const categoryFilter =
        document.getElementById("category-filter");

    if (categoryFilter) {

        const alreadyExists =
            Array.from(categoryFilter.options).some(
                option => option.value === "Truck"
            );

        if (!alreadyExists) {

            const option =
                document.createElement("option");

            option.value = "Truck";
            option.textContent = "Truck";

            categoryFilter.appendChild(option);
        }
    }


    // =====================================
    // VEHICLE RESULTS CONTAINER
    // =====================================

    const vehicleList =
        document.getElementById("vehicle-list");

    if (!vehicleList) {

        console.error(
            "Vehicle results container not found."
        );

        return;
    }


    // =====================================
    // BUTTON STYLING
    // =====================================

    const styles =
        document.createElement("style");

    styles.textContent = `

        #vehicle-list .qr-reserve-link {
            display: inline-block;

            margin-top: 14px;
            padding: 11px 18px;

            background: #1769d2;
            color: white !important;

            border-radius: 7px;

            text-decoration: none;
            font-size: 14px;
            font-weight: bold;
        }

        #vehicle-list .qr-reserve-link:hover {
            background: #104b98;
        }

        #vehicle-list [data-qr-bookable="true"] {
            cursor: pointer;
        }

    `;

    document.head.appendChild(styles);


    // =====================================
    // ADD LINKS TO MYSQL VEHICLE CARDS
    // =====================================

    function updateVehicleCards() {

        const cards =
            Array.from(vehicleList.children);

        for (const card of cards) {

            if (
                card.dataset.qrBookable === "true"
            ) {
                continue;
            }

            // Your existing vehicle cards include
            // "Vehicle ID: 1", "Vehicle ID: 2", etc.

            const contents =
                card.textContent || "";

            const match = contents.match(
                /Vehicle\s*ID\s*:\s*(\d+)/i
            );

            if (!match) {
                continue;
            }

            const id = Number(match[1]);

            if (
                !Number.isInteger(id) ||
                id <= 0
            ) {
                continue;
            }


            // RESERVATION LINK

            const link =
                document.createElement("a");

            link.className =
                "qr-reserve-link";

            link.href =
                `vehicle.html?id=${id}`;

            link.textContent =
                "View & Reserve";

            link.setAttribute(
                "aria-label",
                `View and reserve vehicle ${id}`
            );

            card.appendChild(link);

            card.dataset.qrBookable =
                "true";


            // CLICKING THE REST OF THE CARD
            // ALSO OPENS THE SAME VEHICLE.

            card.addEventListener(
                "click",
                event => {

                    if (
                        event.target.closest(
                            "a,button,input,select"
                        )
                    ) {
                        return;
                    }

                    window.location.assign(
                        `vehicle.html?id=${id}`
                    );
                }
            );
        }
    }


    // =====================================
    // WATCH FOR NEW SEARCH RESULTS
    // =====================================

    const observer =
        new MutationObserver(
            updateVehicleCards
        );

    observer.observe(vehicleList, {
        childList: true
    });


    // INITIAL VEHICLE CARDS

    updateVehicleCards();

})();
