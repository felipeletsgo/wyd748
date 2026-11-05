#pragma once

#include <windows.h>

// EnumDisplaySettings reads dmSize and dmDriverExtra from the caller's
// DEVMODE before it writes the mode. The enumerator is a template parameter
// so tests can check that contract without a display driver.
namespace display_mode
{
    // Clears the mode and declares the structure size with no private
    // driver data, as EnumDisplaySettings requires before each call.
    template <typename Mode>
    void PrepareQuery(Mode& mode)
    {
        mode = Mode{};
        mode.dmSize = static_cast<WORD>(sizeof(Mode));
        mode.dmDriverExtra = 0;
    }

    // enumerate(index, &mode) follows EnumDisplaySettings: nonzero on success.
    // The mode is prepared again before each call.
    template <typename Enumerate, typename Mode>
    bool Query(Enumerate&& enumerate, DWORD index, Mode& mode)
    {
        PrepareQuery(mode);
        return enumerate(index, &mode) != 0;
    }

    // True when the enumeration lists the exact width, height and depth.
    template <typename Mode, typename Enumerate>
    bool HasMode(Enumerate&& enumerate, DWORD width, DWORD height, DWORD bitsPerPixel)
    {
        Mode mode{};
        for (DWORD index = 0; Query(enumerate, index, mode); ++index)
        {
            if (mode.dmPelsWidth == width && mode.dmPelsHeight == height &&
                mode.dmBitsPerPel == bitsPerPixel)
                return true;
        }
        return false;
    }

    // EnumDisplaySettingsA for the primary display.
    inline BOOL EnumeratePrimary(DWORD index, DEVMODEA* mode)
    {
        return EnumDisplaySettingsA(nullptr, index, mode);
    }
}
