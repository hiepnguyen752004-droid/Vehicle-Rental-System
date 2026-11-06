
/* QUICKROUTE RENTALS
   COMPLETE MANAGER INVENTORY DASHBOARD
*/

const byId = id => document.getElementById(id);

let allVehicles = [];
let selectedVehicleId = null;


// =====================================
// DISPLAY STATUS MESSAGES
// =====================================

function showMessage(id, message, success = true) {

    const element = byId(id);

    element.textContent = message;

    element.className =
        success ? "result-success" : "result-error";
}


// =====================================
// SEND HTTP REQUEST
// =====================================

async function apiRequest(url, options = {}) {

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
            "The server did not return valid JSON."
        );
    }

    if (!response.ok || data.status !== "success") {

        throw new Error(
            data.message || "Server request failed."
        );
    }

    return data;
}


// =====================================
// CHECK MANAGER ACCOUNT
// =====================================

async function checkManager() {

    try {

        const account = await apiRequest(
            "/api/manager/me"
        );

        byId("manager-account").textContent =
            "Logged in as " + account.name +
            " (Manager ID: " + account.managerId + ")";

        byId("access-denied").hidden = true;
        byId("manager-content").hidden = false;

        await loadVehicles();

    }
    catch (error) {

        byId("manager-account").textContent =
            "Manager authorization required";

        byId("manager-content").hidden = true;
        byId("access-denied").hidden = false;
    }
}


// =====================================
// LOAD ALL MYSQL VEHICLES
// =====================================

async function loadVehicles() {

    showMessage(
        "inventory-message",
        "Loading vehicles..."
    );

    try {

        const data = await apiRequest(
            "/api/vehicles?sort=id_asc"
        );

        allVehicles = data.vehicles || [];

        showMessage(
            "inventory-message",
            ""
        );

        renderVehicles();

    }
    catch (error) {

        showMessage(
            "inventory-message",
            error.message,
            false
        );
    }
}


// =====================================
// CREATE ONE VEHICLE CARD
// =====================================

function createVehicleCard(vehicle) {

    const card = document.createElement("button");

    card.type = "button";
    card.className = "vehicle-card";


    // PHOTO

    const photoBox = document.createElement("div");

    photoBox.className = "vehicle-photo";

    if (vehicle.imageUrl) {

        const image = document.createElement("img");

        image.src = vehicle.imageUrl;

        image.alt =
            vehicle.make + " " + vehicle.model;

        image.onerror = function() {

            image.remove();

            photoBox.textContent =
                "No photograph available";
        };

        photoBox.appendChild(image);

    }
    else {

        photoBox.textContent =
            "No photograph available";
    }

    card.appendChild(photoBox);


    // DETAILS

    const details = document.createElement("div");

    details.className = "vehicle-details";


    const title = document.createElement("strong");

    title.textContent =
        vehicle.year + " " +
        vehicle.make + " " +
        vehicle.model;

    details.appendChild(title);


    const info = document.createElement("small");

    info.textContent =
        "Vehicle ID: " + vehicle.id +
        " · " + vehicle.category;

    details.appendChild(info);


    const price = document.createElement("span");

    price.className = "vehicle-price";

    price.textContent =
        "$" + Number(vehicle.dailyRate).toFixed(2) +
        " / day";

    details.appendChild(price);


    const status = document.createElement("span");

    status.className = "status-pill";

    if (vehicle.status === "Maintenance") {
        status.classList.add("maintenance");
    }

    status.textContent = vehicle.status;

    details.appendChild(status);


    const hint = document.createElement("span");

    hint.className = "edit-hint";

    hint.textContent =
        "Edit Vehicle →";

    details.appendChild(hint);


    card.appendChild(details);


    // CLICK TO OPEN EDITOR

    card.addEventListener("click", function() {

        openEditor(vehicle.id);
    });

    return card;
}


// =====================================
// FILTER AND DISPLAY VEHICLES
// =====================================

function renderVehicles() {

    const grid = byId("inventory-grid");

    grid.replaceChildren();

    const keyword =
        byId("search-vehicles").value
            .trim()
            .toLowerCase();

    const category =
        byId("filter-category").value;

    const status =
        byId("filter-status").value;


    const filtered = allVehicles.filter(vehicle => {

        const text = (
            vehicle.id + " " +
            vehicle.make + " " +
            vehicle.model + " " +
            vehicle.year + " " +
            vehicle.category
        ).toLowerCase();

        const matchKeyword =
            !keyword || text.includes(keyword);

        const matchCategory =
            !category || vehicle.category === category;

        const matchStatus =
            !status || vehicle.status === status;

        return (
            matchKeyword &&
            matchCategory &&
            matchStatus
        );
    });


    byId("vehicle-count").textContent =
        filtered.length + " of " +
        allVehicles.length + " vehicles";


    for (const vehicle of filtered) {

        grid.appendChild(
            createVehicleCard(vehicle)
        );
    }


    if (filtered.length === 0) {

        const message =
            document.createElement("p");

        message.textContent =
            "No vehicles match your search.";

        grid.appendChild(message);
    }
}


// =====================================
// OPEN VEHICLE EDITOR
// =====================================

async function openEditor(vehicleId) {

    const vehicle = allVehicles.find(
        item => Number(item.id) === Number(vehicleId)
    );

    if (!vehicle) {
        return;
    }

    selectedVehicleId = Number(vehicle.id);

    byId("create-panel").hidden = true;
    byId("edit-panel").hidden = false;


    byId("edit-heading").textContent =
        "Edit Vehicle #" + vehicle.id +
        " — " + vehicle.make + " " + vehicle.model;


    byId("edit-make").value =
        vehicle.make;

    byId("edit-model").value =
        vehicle.model;

    byId("edit-year").value =
        vehicle.year;

    byId("edit-category").value =
        vehicle.category;

    byId("edit-rate").value =
        Number(vehicle.dailyRate).toFixed(2);

    byId("edit-status").value =
        vehicle.status;


    showMessage("edit-result", "");
    showMessage("photo-result", "");

    byId("edit-photo-form").reset();

    updatePrimarySelect(
        "edit-photos",
        "edit-primary"
    );


    byId("edit-panel").scrollIntoView({
        behavior: "smooth",
        block: "start"
    });

    await loadGallery();
}


// =====================================
// LOAD EXISTING VEHICLE PHOTOS
// =====================================

async function loadGallery() {

    if (!selectedVehicleId) {
        return;
    }

    const vehicleId = selectedVehicleId;

    const gallery = byId("photo-gallery");

    gallery.replaceChildren();

    const loading = document.createElement("p");

    loading.textContent =
        "Loading photographs...";

    gallery.appendChild(loading);


    try {

        const data = await apiRequest(
            "/api/vehicles/" +
            vehicleId +
            "/images"
        );

        if (selectedVehicleId !== vehicleId) {
            return;
        }

        gallery.replaceChildren();


        if (!data.images || data.images.length === 0) {

            const empty = document.createElement("p");

            empty.textContent =
                "This vehicle has no photographs yet.";

            gallery.appendChild(empty);

            return;
        }


        for (const photo of data.images) {

            const card =
                document.createElement("div");

            card.className = "gallery-card";


            // IMAGE

            const image =
                document.createElement("img");

            image.src = photo.imageUrl;

            image.alt =
                "Vehicle photograph " + photo.imageId;

            card.appendChild(image);


            // PRIMARY LABEL

            if (photo.isPrimary) {

                const label =
                    document.createElement("span");

                label.className = "primary-label";

                label.textContent =
                    "Main Photograph";

                card.appendChild(label);
            }


            const actions =
                document.createElement("div");

            actions.className = "gallery-actions";


            // MAKE PRIMARY BUTTON

            if (!photo.isPrimary) {

                const primaryButton =
                    document.createElement("button");

                primaryButton.type = "button";

                primaryButton.textContent =
                    "Make Main";

                primaryButton.addEventListener(
                    "click",
                    () => setPrimaryImage(photo.imageId)
                );

                actions.appendChild(primaryButton);
            }


            // DELETE BUTTON

            const deleteButton =
                document.createElement("button");

            deleteButton.type = "button";

            deleteButton.className = "delete-btn";

            deleteButton.textContent = "Delete";

            deleteButton.addEventListener(
                "click",
                () => deleteImage(photo.imageId)
            );

            actions.appendChild(deleteButton);

            card.appendChild(actions);

            gallery.appendChild(card);
        }

    }
    catch (error) {

        gallery.replaceChildren();

        const message =
            document.createElement("p");

        message.textContent = error.message;

        gallery.appendChild(message);
    }
}


// =====================================
// COLLECT VEHICLE INFORMATION
// =====================================

function getEditorData() {

    return {

        make:
            byId("edit-make").value.trim(),

        model:
            byId("edit-model").value.trim(),

        year:
            Number(byId("edit-year").value),

        category:
            byId("edit-category").value,

        dailyRate:
            Number(byId("edit-rate").value),

        status:
            byId("edit-status").value
    };
}


// =====================================
// SAVE EDITED VEHICLE INFORMATION
// =====================================

byId("edit-form").addEventListener(
    "submit",
    async function(event) {

        event.preventDefault();

        if (!selectedVehicleId) {
            return;
        }

        const vehicleId = selectedVehicleId;
        const button = byId("save-edits");

        button.disabled = true;

        try {

            showMessage(
                "edit-result",
                "Saving changes..."
            );

            await apiRequest(
                "/api/manager/vehicles/" +
                vehicleId +
                "/update",
                {
                    method: "POST",

                    headers: {
                        "Content-Type": "application/json"
                    },

                    body: JSON.stringify(
                        getEditorData()
                    )
                }
            );


            showMessage(
                "edit-result",
                "Vehicle information saved successfully!"
            );


            await loadVehicles();

        }
        catch (error) {

            showMessage(
                "edit-result",
                error.message,
                false
            );
        }
        finally {

            button.disabled = false;
        }
    }
);


// =====================================
// VALIDATE IMAGE SELECTION
// =====================================

function validatePhotos(input, required = false) {

    const files = Array.from(input.files);

    if (required && files.length === 0) {

        throw new Error(
            "Select at least one photograph."
        );
    }

    if (files.length > 5) {

        throw new Error(
            "Maximum five photographs per upload."
        );
    }

    for (const file of files) {

        if (
            file.type !== "image/jpeg" &&
            file.type !== "image/png"
        ) {

            throw new Error(
                "Only JPG and PNG images are supported."
            );
        }

        if (file.size > 3 * 1024 * 1024) {

            throw new Error(
                file.name + " is larger than 3 MB."
            );
        }
    }

    return files;
}


// =====================================
// UPDATE MAIN PHOTO OPTIONS
// =====================================

function updatePrimarySelect(
    fileInputId,
    selectId) {

    const input = byId(fileInputId);
    const select = byId(selectId);

    const files = Array.from(input.files);

    select.replaceChildren();

    if (files.length === 0) {

        const option =
            document.createElement("option");

        option.value = "0";
        option.textContent =
            "First photograph";

        select.appendChild(option);

        return;
    }

    files.forEach((file, index) => {

        const option =
            document.createElement("option");

        option.value = String(index);

        option.textContent =
            (index + 1) + ". " + file.name;

        select.appendChild(option);
    });
}


// =====================================
// UPLOAD PHOTOS TO SELECTED VEHICLE
// =====================================

byId("edit-photo-form").addEventListener(
    "submit",
    async function(event) {

        event.preventDefault();

        if (!selectedVehicleId) {
            return;
        }

        const vehicleId = selectedVehicleId;

        const input = byId("edit-photos");

        const button =
            byId("upload-edit-photos");

        try {

            validatePhotos(input, true);

            button.disabled = true;

            showMessage(
                "photo-result",
                "Uploading photographs..."
            );

            const data = await apiRequest(
                "/api/manager/vehicles/" +
                vehicleId +
                "/images",
                {
                    method: "POST",

                    body: new FormData(
                        byId("edit-photo-form")
                    )
                }
            );


            showMessage(
                "photo-result",
                data.photosUploaded +
                " photograph(s) uploaded!"
            );

            byId("edit-photo-form").reset();

            updatePrimarySelect(
                "edit-photos",
                "edit-primary"
            );

            await loadGallery();
            await loadVehicles();

        }
        catch (error) {

            showMessage(
                "photo-result",
                error.message,
                false
            );
        }
        finally {

            button.disabled = false;
        }
    }
);


// =====================================
// SET MAIN PHOTOGRAPH
// =====================================

async function setPrimaryImage(imageId) {

    try {

        await apiRequest(
            "/api/manager/images/" +
            imageId +
            "/primary",
            {
                method: "POST"
            }
        );

        showMessage(
            "photo-result",
            "Main photograph updated!"
        );

        await loadGallery();
        await loadVehicles();

    }
    catch (error) {

        showMessage(
            "photo-result",
            error.message,
            false
        );
    }
}


// =====================================
// DELETE PHOTOGRAPH
// =====================================

async function deleteImage(imageId) {

    const confirmed = confirm(
        "Delete this photograph permanently?"
    );

    if (!confirmed) {
        return;
    }

    try {

        await apiRequest(
            "/api/manager/images/" +
            imageId +
            "/delete",
            {
                method: "POST"
            }
        );

        showMessage(
            "photo-result",
            "Photograph deleted successfully."
        );

        await loadGallery();
        await loadVehicles();

    }
    catch (error) {

        showMessage(
            "photo-result",
            error.message,
            false
        );
    }
}


// =====================================
// OPEN ADD NEW VEHICLE FORM
// =====================================

byId("new-btn").addEventListener(
    "click",
    function() {

        selectedVehicleId = null;

        byId("edit-panel").hidden = true;
        byId("create-panel").hidden = false;

        showMessage("create-result", "");

        byId("create-panel").scrollIntoView({
            behavior: "smooth",
            block: "start"
        });
    }
);


// =====================================
// ADD NEW VEHICLE AND PHOTOS
// =====================================

byId("create-form").addEventListener(
    "submit",
    async function(event) {

        event.preventDefault();

        const button = byId("save-new");

        try {

            validatePhotos(
                byId("new-photos"),
                false
            );

            button.disabled = true;

            showMessage(
                "create-result",
                "Saving new vehicle..."
            );


            const data = await apiRequest(
                "/api/manager/vehicles",
                {
                    method: "POST",

                    body: new FormData(
                        byId("create-form")
                    )
                }
            );


            showMessage(
                "create-result",
                "Vehicle #" + data.vehicleId +
                " created with " +
                data.photosUploaded +
                " photograph(s)!"
            );


            byId("create-form").reset();

            updatePrimarySelect(
                "new-photos",
                "new-primary"
            );


            // REFRESH INVENTORY

            await loadVehicles();


            // AUTOMATICALLY OPEN NEW VEHICLE

            openEditor(data.vehicleId);

        }
        catch (error) {

            showMessage(
                "create-result",
                error.message,
                false
            );
        }
        finally {

            button.disabled = false;
        }
    }
);


// =====================================
// FILE SELECTION EVENTS
// =====================================

byId("edit-photos").addEventListener(
    "change",
    () => updatePrimarySelect(
        "edit-photos",
        "edit-primary"
    )
);

byId("new-photos").addEventListener(
    "change",
    () => updatePrimarySelect(
        "new-photos",
        "new-primary"
    )
);


// =====================================
// SEARCH AND FILTER EVENTS
// =====================================

byId("search-vehicles").addEventListener(
    "input", renderVehicles
);

byId("filter-category").addEventListener(
    "change", renderVehicles
);

byId("filter-status").addEventListener(
    "change", renderVehicles
);


// =====================================
// DASHBOARD BUTTONS
// =====================================

byId("refresh-btn").addEventListener(
    "click", loadVehicles
);

byId("close-edit").addEventListener(
    "click",
    function() {

        selectedVehicleId = null;

        byId("edit-panel").hidden = true;
    }
);

byId("close-create").addEventListener(
    "click",
    function() {

        byId("create-panel").hidden = true;
    }
);


// =====================================
// START DASHBOARD
// =====================================

checkManager();
