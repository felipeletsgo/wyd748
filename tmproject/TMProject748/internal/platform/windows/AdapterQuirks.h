#pragma once
#include <cctype>
#include <cstddef>
#include <cstring>
#include <string>

namespace adapter_quirks {
// Apply old workarounds only to the named legacy family, not its whole vendor.
struct Detected {
    int vgaId = -1; // Unknown adapters use the general rendering path.
    bool nvidia = false;
    bool tnt = false;
    bool savage = false;
    bool g400 = false;
    bool intel = false;
};

// Reads at most capacity bytes; the description need not be terminated and is
// never modified.
inline Detected Classify(const char* description, std::size_t capacity, unsigned int vendorId = 0)
{
    Detected detected;
    if (!description) return detected;
    std::size_t length = 0;
    while (length < capacity && description[length] != '\0') ++length;
    std::string upper(description, length);
    for (char& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    const auto has = [&](const char* token) { return upper.find(token) != std::string::npos; };

    const bool nvidia = vendorId == 0x10DE || (vendorId == 0 && has("NVIDIA"));
    const bool intel = vendorId == 0x8086 || (vendorId == 0 && has("INTEL"));
    if (nvidia) detected.vgaId = (has("GEFORCE2 MX") || has("GEFORCE4 MX")) ? 0 : 1;
    if (vendorId == 0x1002 || (vendorId == 0 && has("RADEON"))) detected.vgaId = 2;
    if (nvidia) {
        detected.nvidia = true;
        if (has("RIVA") || has("TNT")) detected.tnt = true;
    }
    if (has("SAVAGE") || has("SAVAGA")) detected.savage = true;
    if (has("MATROX") && has("G4")) detected.g400 = true;
    detected.intel = intel && (has("INTEL 810") || has("INTEL(R) 810") || has("INTEL(R) 82810"));
    return detected;
}
}
