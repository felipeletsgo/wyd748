#include "../internal/platform/windows/DeviceRestore.h"
#include <cstdio>
#include <string>

namespace {
struct Font {
    HRESULT result = S_OK;
    char id = 'n';
    std::string* order = nullptr;
    HRESULT OnResetDevice() { *order += id; return result; }
};
}

int RunDeviceRestoreTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::fprintf(stderr, "DeviceRestore: %s\n", name); }
    };
    for (int fonts = 0; fonts < 4; ++fonts) {
        for (int failure = 0; failure < 5; ++failure) {
            std::string order;
            Font normal{failure == 1 ? E_OUTOFMEMORY : S_FALSE, 'n', &order};
            Font large{failure == 2 ? E_ACCESSDENIED : S_OK, 'l', &order};
            const bool hasNormal = (fonts & 1) != 0;
            const bool hasLarge = (fonts & 2) != 0;
            const auto result = device_restore::Complete(hasNormal ? &normal : nullptr,
                hasLarge ? &large : nullptr,
                [&] { order += 's'; return failure != 3; },
                [&] { order += 'v'; return failure != 4; });
            std::string expected;
            HRESULT expectedResult = S_OK;
            if (hasNormal) expected += 'n';
            if (hasNormal && failure == 1) expectedResult = E_OUTOFMEMORY;
            else {
                if (hasLarge) expected += 'l';
                if (hasLarge && failure == 2) expectedResult = E_ACCESSDENIED;
                else {
                    expected += 's';
                    if (failure == 3) expectedResult = E_FAIL;
                    else {
                        expected += 'v';
                        if (failure == 4) expectedResult = E_FAIL;
                    }
                }
            }
            check(result == expectedResult, "every required failure propagated; absent fonts optional");
            check(order == expected, "original restore order; no later initialization after failure");
            // Simulate a subsequent base lifecycle attempt after cleanup.
            normal.result = large.result = S_OK;
            order.clear();
            check(device_restore::Complete(hasNormal ? &normal : nullptr, hasLarge ? &large : nullptr,
                [&] { order += 's'; return 1; }, [&] { order += 'v'; return 1; }) == S_OK,
                "successful retry completes");
            check(order == (hasNormal ? "n" : "") + std::string(hasLarge ? "l" : "") + "sv",
                "retry runs all required operations");
        }
    }
    return failures;
}
