#pragma once
#include <windows.h>
#include <string>

// Localized resources and the existing wire carry Windows-1252 bytes. Decode
// at the GDI boundary so drawing and geometry do not depend on the system ACP.
namespace resource_text {
inline bool Decode(const char* text, int length, std::wstring& result) {
    result.clear();
    if (!text || length < 0) return false;
    if (!length) return true;
    const int count = MultiByteToWideChar(1252, 0, text, length, nullptr, 0);
    if (!count) return false;
    result.resize(static_cast<std::size_t>(count));
    return MultiByteToWideChar(1252, 0, text, length, &result[0], count) == count;
}
inline BOOL Draw(HDC dc, int x, int y, const char* text, int length) {
    std::wstring decoded;
    return Decode(text, length, decoded) &&
        TextOutW(dc, x, y, decoded.c_str(), static_cast<int>(decoded.size()));
}
inline BOOL Measure(HDC dc, const char* text, int length, SIZE* size) {
    if (!size) return FALSE;
    *size = {};
    std::wstring decoded;
    return Decode(text, length, decoded) &&
        GetTextExtentPoint32W(dc, decoded.c_str(), static_cast<int>(decoded.size()), size);
}
}
