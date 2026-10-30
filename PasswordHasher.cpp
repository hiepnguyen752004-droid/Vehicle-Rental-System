#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

#include "PasswordHasher.h"

#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace
{
    const unsigned long long ITERATIONS = 600000;
    const size_t SALT_SIZE = 16;
    const size_t HASH_SIZE = 32;

    string toHex(const vector<unsigned char>& bytes)
    {
        const char* digits = "0123456789abcdef";
        string output;

        for (unsigned char value : bytes)
        {
            output += digits[value >> 4];
            output += digits[value & 15];
        }

        return output;
    }

    int hexDigit(char value)
    {
        if (value >= '0' && value <= '9')
            return value - '0';

        if (value >= 'a' && value <= 'f')
            return value - 'a' + 10;

        if (value >= 'A' && value <= 'F')
            return value - 'A' + 10;

        return -1;
    }

    vector<unsigned char> fromHex(const string& hex)
    {
        vector<unsigned char> output;

        if (hex.size() % 2 != 0)
            return output;

        for (size_t i = 0; i < hex.size(); i += 2)
        {
            int a = hexDigit(hex[i]);
            int b = hexDigit(hex[i + 1]);

            if (a < 0 || b < 0)
                return {};

            output.push_back(
                static_cast<unsigned char>(
                    (a << 4) | b
                    )
            );
        }

        return output;
    }

    vector<unsigned char> deriveKey(
        const string& password,
        const vector<unsigned char>& salt)
    {
        BCRYPT_ALG_HANDLE algorithm = nullptr;

        NTSTATUS status = BCryptOpenAlgorithmProvider(
            &algorithm,
            BCRYPT_SHA256_ALGORITHM,
            nullptr,
            BCRYPT_ALG_HANDLE_HMAC_FLAG
        );

        if (status < 0)
        {
            throw runtime_error(
                "Unable to initialize password hashing."
            );
        }

        vector<unsigned char> output(HASH_SIZE);

        status = BCryptDeriveKeyPBKDF2(
            algorithm,
            reinterpret_cast<PUCHAR>(
                const_cast<char*>(password.data())
                ),
            static_cast<ULONG>(password.size()),
            const_cast<PUCHAR>(salt.data()),
            static_cast<ULONG>(salt.size()),
            ITERATIONS,
            output.data(),
            static_cast<ULONG>(output.size()),
            0
        );

        BCryptCloseAlgorithmProvider(
            algorithm, 0
        );

        if (status < 0)
        {
            throw runtime_error(
                "Unable to derive password hash."
            );
        }

        return output;
    }
}

string PasswordHasher::hashPassword(
    const string& password)
{
    vector<unsigned char> salt(SALT_SIZE);

    NTSTATUS status = BCryptGenRandom(
        nullptr,
        salt.data(),
        static_cast<ULONG>(salt.size()),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );

    if (status < 0)
    {
        throw runtime_error(
            "Unable to create random password salt."
        );
    }

    vector<unsigned char> key =
        deriveKey(password, salt);

    return "pbkdf2_sha256$600000$"
        + toHex(salt)
        + "$"
        + toHex(key);
}

bool PasswordHasher::verifyPassword(
    const string& password,
    const string& storedHash)
{
    const string prefix =
        "pbkdf2_sha256$600000$";

    if (storedHash.compare(
        0, prefix.size(), prefix) != 0)
    {
        return false;
    }

    string remaining =
        storedHash.substr(prefix.size());

    size_t separator = remaining.find('$');

    if (separator == string::npos)
        return false;

    vector<unsigned char> salt =
        fromHex(remaining.substr(0, separator));

    vector<unsigned char> expected =
        fromHex(remaining.substr(separator + 1));

    if (salt.size() != SALT_SIZE ||
        expected.size() != HASH_SIZE)
    {
        return false;
    }

    vector<unsigned char> actual =
        deriveKey(password, salt);

    unsigned int difference = 0;

    for (size_t i = 0; i < HASH_SIZE; ++i)
    {
        difference |=
            static_cast<unsigned int>(
                actual[i] ^ expected[i]
                );
    }

    return difference == 0;
}