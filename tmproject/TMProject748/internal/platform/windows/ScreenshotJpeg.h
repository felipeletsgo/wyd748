#pragma once
#include <windows.h>
#include <cstdint>
#include <vector>

// Writes tightly packed 24-bit BGR rows as a JPEG file through WIC. A failed
// encoding removes the partial file.
HRESULT SaveBgr24Jpeg(const char* fileName, UINT width, UINT height, const std::vector<std::uint8_t>& pixels);
