
/* ==========================================
   QUICKROUTE RENTALS
   CUSTOMER AND MANAGER LOGIN
========================================== */

const loginForm =
    document.getElementById("login-form");

const accountPanel =
    document.getElementById("account-panel");

const loginMessage =
    document.getElementById("login-message");

const logoutButton =
    document.getElementById("logout-button");


// ==========================================
// SAFE RETURN DESTINATION
// ==========================================

function getReturnDestination() {

    const parameters =
        new URLSearchParams(window.location.search);

    const destination =
        parameters.get("return");

    // Only allow return to local vehicle
    // detail pages, never external websites.

    if (
        destination &&
        /^\/vehicle\.html\?id=[1-9]\d*$/.test(destination)
    ) {
        return destination;
    }

    return "/index.html";
}


// ==========================================
// SHOW EXISTING ACCOUNT
// ==========================================

function showAccount(account) {

    loginForm.hidden = true;
    accountPanel.hidden = false;

    document.getElementById(
        "welcome-message"
    ).textContent =
        "Welcome, " + account.name;

    document.getElementById(
        "account-role"
    ).textContent =
        "Account Role: " + account.role;

    loginMessage.textContent = "";


    // REMOVE PREVIOUSLY GENERATED LINKS

    const previous =
        accountPanel.querySelectorAll(
            ".qr-account-action"
        );

    previous.forEach(link => link.remove());


    // MANAGER DASHBOARD LINK

    if (account.role === "Manager") {

        const link =
            document.createElement("a");

        link.className = "qr-account-action";

        link.href = "/manager.html";

        link.textContent =
            "Open Manager Dashboard";

        link.style.display = "inline-block";
        link.style.margin = "12px 12px 12px 0";
        link.style.padding = "12px 18px";
        link.style.background = "#1769d2";
        link.style.color = "white";
        link.style.borderRadius = "7px";
        link.style.textDecoration = "none";

        accountPanel.appendChild(link);
    }


    // CONTINUE A RESERVATION AFTER LOGIN

    const destination = getReturnDestination();

    if (destination !== "/index.html") {

        const link =
            document.createElement("a");

        link.className = "qr-account-action";

        link.href = destination;

        link.textContent =
            "Continue Vehicle Reservation";

        link.style.display = "inline-block";
        link.style.margin = "12px 12px 12px 0";
        link.style.padding = "12px 18px";
        link.style.background = "#1769d2";
        link.style.color = "white";
        link.style.borderRadius = "7px";
        link.style.textDecoration = "none";

        accountPanel.appendChild(link);
    }
}


// ==========================================
// REDIRECT AFTER SUCCESSFUL LOGIN
// ==========================================

function redirectAccount(account) {

    const destination =
        getReturnDestination();

    // Return to selected vehicle if
    // customer came from its booking page.

    if (destination !== "/index.html") {

        window.location.assign(destination);

        return;
    }


    // Otherwise managers go to their
    // dashboard and customers go home.

    if (account.role === "Manager") {

        window.location.assign(
            "/manager.html"
        );

    }
    else {

        window.location.assign(
            "/index.html"
        );
    }
}


// ==========================================
// HANDLE LOGIN FORM
// ==========================================

loginForm.addEventListener(
    "submit",
    async function(event) {

        event.preventDefault();

        const email =
            document.getElementById(
                "email"
            ).value.trim();

        const password =
            document.getElementById(
                "password"
            ).value;


        loginMessage.textContent =
            "Logging in...";

        try {

            const response = await fetch(
                "/api/auth/login",
                {
                    method: "POST",

                    credentials: "same-origin",

                    headers: {
                        "Content-Type": "application/json"
                    },

                    body: JSON.stringify({
                        email,
                        password
                    })
                }
            );


            const data = await response.json();

            if (
                !response.ok ||
                data.status !== "success"
            ) {

                loginMessage.textContent =
                    data.message || "Login failed.";

                return;
            }


            document.getElementById(
                "password"
            ).value = "";


            redirectAccount(data);

        }
        catch (error) {

            loginMessage.textContent =
                "Unable to connect to the server.";

            console.error(error);
        }
    }
);


// ==========================================
// LOGOUT
// ==========================================

logoutButton.addEventListener(
    "click",
    async function() {

        try {

            const response = await fetch(
                "/api/auth/logout",
                {
                    method: "POST",
                    credentials: "same-origin"
                }
            );

            if (!response.ok) {

                throw new Error(
                    "Logout failed."
                );
            }

            window.location.assign(
                "/login.html"
            );

        }
        catch (error) {

            loginMessage.textContent =
                "Unable to log out.";

            console.error(error);
        }
    }
);


// ==========================================
// CHECK EXISTING SESSION
// ==========================================

async function checkExistingLogin() {

    try {

        const response = await fetch(
            "/api/auth/me",
            {
                credentials: "same-origin",
                cache: "no-store"
            }
        );

        if (!response.ok) {

            loginForm.hidden = false;
            accountPanel.hidden = true;

            return;
        }

        const account =
            await response.json();

        if (account.status === "success") {

            showAccount(account);
        }

    }
    catch (error) {

        console.error(
            "Session check failed:",
            error
        );
    }
}

checkExistingLogin();

