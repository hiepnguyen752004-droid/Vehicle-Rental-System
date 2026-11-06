

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

#include "AuthRoutes.h"
#include "CustomerRepository.h"

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <array>
#include <sstream>
#include <stdexcept>

using namespace std;


// ==========================================
// SESSION STORAGE
// ==========================================

namespace {

    // Session expires 30 minutes after login

    const auto SESSION_LIFETIME =
        chrono::minutes(30);

    const int SESSION_SECONDS = 1800;

    const string COOKIE_NAME = "qr_session";


    // ======================================
    // LOGIN SESSION DATA
    // ======================================

    struct LoginSession {

        int customerId = 0;

        string name;

        string role;

        chrono::steady_clock::time_point expires;
    };


    // Active sessions are stored in memory.

    unordered_map<string, LoginSession> sessions;

    mutex sessionsMutex;


    // ======================================
    // CREATE JSON RESPONSE
    // ======================================

    crow::response authJson(
        int code,
        crow::json::wvalue& data
    ) {
        crow::response response(
            code,
            data.dump()
        );

        response.set_header(
            "Content-Type",
            "application/json"
        );

        response.set_header(
            "Cache-Control",
            "no-store"
        );

        return response;
    }


    // ======================================
    // ERROR RESPONSE
    // ======================================

    crow::response authError(
        int code,
        const string& message
    ) {
        crow::json::wvalue response;

        response["status"] = "error";

        response["message"] = message;

        return authJson(
            code,
            response
        );
    }


    // ======================================
    // REMOVE LEADING/TRAILING SPACES
    // ======================================

    string trimText(const string& text) {

        size_t first =
            text.find_first_not_of(" \t\r\n");

        if (first == string::npos) {
            return "";
        }

        size_t last =
            text.find_last_not_of(" \t\r\n");

        return text.substr(
            first,
            last - first + 1
        );
    }


    // ======================================
    // GENERATE SECURE SESSION TOKEN
    // ======================================

    string generateSessionToken() {

        array<unsigned char, 32> bytes{};

        NTSTATUS status = BCryptGenRandom(
            nullptr,
            bytes.data(),
            static_cast<ULONG>(bytes.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG
        );

        if (status < 0) {

            throw runtime_error(
                "Unable to generate session token"
            );
        }


        const char* digits =
            "0123456789abcdef";

        string token;

        token.reserve(
            bytes.size() * 2
        );


        for (unsigned char value : bytes) {

            token += digits[value >> 4];

            token += digits[value & 15];
        }

        return token;
    }


    // ======================================
    // GET SESSION COOKIE
    // ======================================

    string getSessionToken(
        const crow::request& request
    ) {
        string cookieHeader =
            request.get_header_value("Cookie");

        stringstream stream(cookieHeader);

        string item;

        while (getline(stream, item, ';')) {

            item = trimText(item);

            string prefix =
                COOKIE_NAME + "=";

            if (item.rfind(prefix, 0) == 0) {

                string token =
                    item.substr(prefix.size());

                if (token.size() != 64) {
                    return "";
                }

                for (char character : token) {

                    bool hex =
                        (character >= '0' &&
                            character <= '9') ||

                        (character >= 'a' &&
                            character <= 'f');

                    if (!hex) {
                        return "";
                    }
                }

                return token;
            }
        }

        return "";
    }


    // ======================================
    // REMOVE EXPIRED SESSIONS
    // CALL WITH MUTEX LOCKED
    // ======================================

    void removeExpiredSessions() {

        auto now =
            chrono::steady_clock::now();

        for (auto it = sessions.begin();
            it != sessions.end();) {

            if (now >= it->second.expires) {

                it = sessions.erase(it);
            }
            else {

                ++it;
            }
        }
    }


    // ======================================
    // STORE A NEW SESSION
    // ======================================

    string createSession(
        const LoggedInCustomer& account
    ) {
        string token =
            generateSessionToken();

        LoginSession session;

        session.customerId =
            account.customerId;

        session.name =
            account.name;

        session.role =
            account.role;

        session.expires =
            chrono::steady_clock::now() +
            SESSION_LIFETIME;


        lock_guard<mutex> lock(
            sessionsMutex
        );

        removeExpiredSessions();

        sessions[token] = session;

        return token;
    }


    // ======================================
    // FIND ACTIVE LOGIN SESSION
    // ======================================

    bool getActiveSession(
        const crow::request& request,
        LoginSession& account
    ) {
        string token =
            getSessionToken(request);

        if (token.empty()) {
            return false;
        }


        lock_guard<mutex> lock(
            sessionsMutex
        );

        auto found =
            sessions.find(token);

        if (found == sessions.end()) {
            return false;
        }


        // Check expiration

        if (chrono::steady_clock::now() >=
            found->second.expires) {

            sessions.erase(found);

            return false;
        }


        account = found->second;

        return true;
    }


    // ======================================
    // REMOVE SESSION
    // ======================================

    void destroySession(
        const crow::request& request
    ) {
        string token =
            getSessionToken(request);

        if (token.empty()) {
            return;
        }


        lock_guard<mutex> lock(
            sessionsMutex
        );

        sessions.erase(token);
    }


    // ======================================
    // SET LOGIN COOKIE
    // ======================================

    void setLoginCookie(
        crow::response& response,
        const string& token
    ) {
        string cookie =
            COOKIE_NAME + "=" + token +
            "; Path=/" +
            "; HttpOnly" +
            "; SameSite=Strict" +
            "; Max-Age=" +
            to_string(SESSION_SECONDS);


        // Local development uses HTTP.
        // Production HTTPS must also use Secure.

        response.set_header(
            "Set-Cookie",
            cookie
        );
    }


    // ======================================
    // REMOVE LOGIN COOKIE
    // ======================================

    void clearLoginCookie(
        crow::response& response
    ) {
        response.set_header(
            "Set-Cookie",

            COOKIE_NAME +
            "=; Path=/" +
            "; HttpOnly" +
            "; SameSite=Strict" +
            "; Max-Age=0"
        );
    }


    // ======================================
    // CHECK REQUEST ORIGIN
    // ======================================

    bool sameWebsiteOrigin(
        const crow::request& request
    ) {
        string host =
            request.get_header_value("Host");

        if (host.empty()) {
            return false;
        }


        string expectedHttp =
            "http://" + host;

        string expectedHttps =
            "https://" + host;


        string origin =
            request.get_header_value("Origin");


        // Prefer Origin when present.

        if (!origin.empty()) {

            return (
                origin == expectedHttp ||
                origin == expectedHttps
                );
        }


        // Fallback for browser requests
        // that include Referer instead.

        string referer =
            request.get_header_value("Referer");

        if (referer.empty()) {
            return false;
        }


        bool httpMatch =
            referer == expectedHttp ||
            referer.rfind(
                expectedHttp + "/", 0
            ) == 0;


        bool httpsMatch =
            referer == expectedHttps ||
            referer.rfind(
                expectedHttps + "/", 0
            ) == 0;


        return httpMatch || httpsMatch;
    }


    // ======================================
    // VERIFY CURRENT ROLE FROM MYSQL
    // ======================================

    bool databaseManagerRole(
        int customerId,
        const string& mysqlPassword
    ) {
        try {

            sql::Driver* driver =
                sql::mysql::get_driver_instance();


            unique_ptr<sql::Connection> connection(
                (*driver).connect(
                    "tcp://127.0.0.1:3306",
                    "vehicle_app",
                    mysqlPassword
                )
            );


            (*connection).setSchema(
                "vehicle_rental_db"
            );


            unique_ptr<sql::PreparedStatement> statement(
                (*connection).prepareStatement(
                    "SELECT role "
                    "FROM customers "
                    "WHERE customer_id = ?"
                )
            );


            (*statement).setInt(
                1,
                customerId
            );


            unique_ptr<sql::ResultSet> result(
                (*statement).executeQuery()
            );


            if (!(*result).next()) {
                return false;
            }


            string role =
                (*result).getString("role").c_str();


            return role == "Manager";
        }
        catch (const exception& error) {

            cerr << "Manager authorization error: "
                << error.what()
                << endl;

            return false;
        }
    }

}


// ==========================================
// EXPORTED AUTHORIZATION FUNCTIONS
// ==========================================


// ======================================
// CHECK SAME-ORIGIN WRITE REQUEST
// ======================================

bool authorizeWriteOrigin(
    const crow::request& request
) {
    return sameWebsiteOrigin(request);
}


// ======================================
// GET CURRENT CUSTOMER ID
//
// THIS FIXES BookingRoutes.cpp
// getLoggedInCustomerId UNDEFINED
// ======================================

bool getLoggedInCustomerId(
    const crow::request& request,
    int& customerId
) {
    customerId = 0;

    LoginSession account;


    if (!getActiveSession(
        request,
        account
    )) {
        return false;
    }


    if (account.customerId <= 0) {
        return false;
    }


    customerId =
        account.customerId;

    return true;
}


// ======================================
// AUTHORIZE MANAGER
// ======================================

bool authorizeManager(
    const crow::request& request,
    const string& mysqlPassword,
    int& managerId
) {
    managerId = 0;

    LoginSession account;


    // Check current session

    if (!getActiveSession(
        request,
        account
    )) {
        return false;
    }


    // Check the session's role

    if (account.role != "Manager") {
        return false;
    }


    // Recheck MySQL so revoked
    // manager permissions stop working.

    if (!databaseManagerRole(
        account.customerId,
        mysqlPassword
    )) {
        return false;
    }


    managerId =
        account.customerId;

    return true;
}


// ==========================================
// REGISTER AUTHENTICATION ROUTES
// ==========================================

void registerAuthRoutes(
    crow::SimpleApp& app,
    const string& mysqlPassword
) {
    auto password =
        make_shared<const string>(
            mysqlPassword
        );


    // ======================================
    // MANAGER WEBSITE FILES
    // ======================================

    app.static_file(
        "/manager.html",
        "manager.html"
    );

    app.static_file(
        "/manager.css",
        "manager.css"
    );

    app.static_file(
        "/manager.js",
        "manager.js"
    );


    // ======================================
    // CUSTOMER REGISTRATION
    //
    // POST /api/auth/register
    // ======================================

    CROW_ROUTE(
        app,
        "/api/auth/register"
    )
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request
            ) {

                if (!authorizeWriteOrigin(request)) {

                    return authError(
                        403,
                        "Forbidden"
                    );
                }


                if (request.body.size() > 8192) {

                    return authError(
                        413,
                        "Registration request too large"
                    );
                }


                auto body =
                    crow::json::load(request.body);


                if (!body ||
                    body.t() != crow::json::type::Object ||
                    !body.has("name") ||
                    !body.has("email") ||
                    !body.has("password") ||
                    body["name"].t() !=
                    crow::json::type::String ||
                    body["email"].t() !=
                    crow::json::type::String ||
                    body["password"].t() !=
                    crow::json::type::String) {

                    return authError(
                        400,
                        "Invalid registration information"
                    );
                }


                string name =
                    trimText(
                        string(body["name"].s())
                    );

                string email =
                    trimText(
                        string(body["email"].s())
                    );

                string customerPassword =
                    string(body["password"].s());

                string phone;


                if (body.has("phone")) {

                    if (body["phone"].t() !=
                        crow::json::type::String) {

                        return authError(
                            400,
                            "Invalid phone number"
                        );
                    }

                    phone =
                        trimText(
                            string(body["phone"].s())
                        );
                }


                if (name.empty() ||
                    name.size() > 100 ||
                    email.empty() ||
                    email.size() > 150 ||
                    email.find('@') == string::npos ||
                    customerPassword.size() < 12 ||
                    customerPassword.size() > 128 ||
                    phone.size() > 30) {

                    return authError(
                        400,
                        "Invalid details. Password must "
                        "contain 12 to 128 characters."
                    );
                }


                try {

                    CustomerRepository repository(
                        *password
                    );


                    bool created =
                        repository.registerCustomer(
                            name,
                            email,
                            customerPassword,
                            phone
                        );


                    if (!created) {

                        return authError(
                            400,
                            "Unable to register. Check your "
                            "details or use a different email."
                        );
                    }


                    crow::json::wvalue response;

                    response["status"] =
                        "success";

                    response["message"] =
                        "Customer account registered successfully";

                    return authJson(
                        201,
                        response
                    );
                }
                catch (const exception& error) {

                    cerr << "Registration error: "
                        << error.what()
                        << endl;


                    return authError(
                        500,
                        "Unable to register customer account"
                    );
                }

            });


    // ======================================
    // CUSTOMER / MANAGER LOGIN
    //
    // POST /api/auth/login
    // ======================================

    CROW_ROUTE(
        app,
        "/api/auth/login"
    )
        .methods(crow::HTTPMethod::Post)
        ([password](
            const crow::request& request
            ) {

                if (!authorizeWriteOrigin(request)) {

                    return authError(
                        403,
                        "Forbidden"
                    );
                }


                if (request.body.size() > 4096) {

                    return authError(
                        413,
                        "Login request too large"
                    );
                }


                auto body =
                    crow::json::load(request.body);


                if (!body ||
                    body.t() != crow::json::type::Object ||
                    !body.has("email") ||
                    !body.has("password") ||
                    body["email"].t() !=
                    crow::json::type::String ||
                    body["password"].t() !=
                    crow::json::type::String) {

                    return authError(
                        400,
                        "Email and password are required"
                    );
                }


                string email =
                    trimText(
                        string(body["email"].s())
                    );

                string customerPassword =
                    string(body["password"].s());


                if (email.empty() ||
                    email.size() > 150 ||
                    customerPassword.empty() ||
                    customerPassword.size() > 128) {

                    return authError(
                        400,
                        "Invalid login information"
                    );
                }


                try {

                    CustomerRepository repository(
                        *password
                    );


                    LoggedInCustomer account;


                    bool loginSuccess =
                        repository.login(
                            email,
                            customerPassword,
                            account
                        );


                    if (!loginSuccess) {

                        return authError(
                            401,
                            "Incorrect email or password"
                        );
                    }


                    // Generate secure session

                    string token =
                        createSession(account);


                    // Build successful login response

                    crow::json::wvalue responseData;

                    responseData["status"] =
                        "success";

                    responseData["message"] =
                        "Login successful";

                    responseData["customerId"] =
                        account.customerId;

                    responseData["name"] =
                        account.name;

                    responseData["role"] =
                        account.role;


                    crow::response response =
                        authJson(
                            200,
                            responseData
                        );


                    // Browser stores session cookie

                    setLoginCookie(
                        response,
                        token
                    );


                    cout << "Account login: Customer ID "
                        << account.customerId
                        << " (" << account.role << ")"
                        << endl;


                    return response;
                }
                catch (const exception& error) {

                    cerr << "Login error: "
                        << error.what()
                        << endl;


                    return authError(
                        500,
                        "Unable to complete login"
                    );
                }

            });


    // ======================================
    // CHECK CURRENT ACCOUNT
    //
    // GET /api/auth/me
    // ======================================

    CROW_ROUTE(
        app,
        "/api/auth/me"
    )
        .methods(crow::HTTPMethod::Get)
        ([](
            const crow::request& request
            ) {

                LoginSession account;


                if (!getActiveSession(
                    request,
                    account
                )) {

                    return authError(
                        401,
                        "Not logged in"
                    );
                }


                crow::json::wvalue response;

                response["status"] =
                    "success";

                response["customerId"] =
                    account.customerId;

                response["name"] =
                    account.name;

                response["role"] =
                    account.role;


                return authJson(
                    200,
                    response
                );

            });


    // ======================================
    // LOG OUT
    //
    // POST /api/auth/logout
    // ======================================

    CROW_ROUTE(
        app,
        "/api/auth/logout"
    )
        .methods(crow::HTTPMethod::Post)
        ([](
            const crow::request& request
            ) {

                if (!authorizeWriteOrigin(request)) {

                    return authError(
                        403,
                        "Forbidden"
                    );
                }


                destroySession(request);


                crow::json::wvalue responseData;

                responseData["status"] =
                    "success";

                responseData["message"] =
                    "Logged out successfully";


                crow::response response =
                    authJson(
                        200,
                        responseData
                    );


                clearLoginCookie(
                    response
                );


                return response;

            });


    // ======================================
    // VERIFY MANAGER ACCOUNT
    //
    // GET /api/manager/me
    // ======================================

    CROW_ROUTE(
        app,
        "/api/manager/me"
    )
        .methods(crow::HTTPMethod::Get)
        ([password](
            const crow::request& request
            ) {

                LoginSession account;


                if (!getActiveSession(
                    request,
                    account
                )) {

                    return authError(
                        401,
                        "Not logged in"
                    );
                }


                int managerId = 0;


                if (!authorizeManager(
                    request,
                    *password,
                    managerId
                )) {

                    return authError(
                        403,
                        "Manager access required"
                    );
                }


                crow::json::wvalue response;

                response["status"] =
                    "success";

                response["managerId"] =
                    managerId;

                response["name"] =
                    account.name;

                response["role"] =
                    "Manager";


                return authJson(
                    200,
                    response
                );

            });

}
