#pragma once
#include <windows.h>
#include "ResourceTextGdi.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace font_gdi {
struct NativeApi {
    HDC CreateContext() { return CreateCompatibleDC(nullptr); }
    HBITMAP CreateBitmap(HDC dc, const BITMAPINFO& info, unsigned int*& bits) {
        void* storage = nullptr;
        const auto bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &storage, nullptr, 0);
        bits = static_cast<unsigned int*>(storage);
        return bitmap;
    }
    HFONT CreateTypeface(int height, int weight, const char* face) {
        std::wstring decoded;
        if (!resource_text::Decode(face, static_cast<int>(std::strlen(face)), decoded)) return nullptr;
        return CreateFontW(height, 0, 0, 0, weight, 0, 0, 0, ANSI_CHARSET, 4, 0, 4, 2, decoded.c_str());
    }
    HGDIOBJ Select(HDC dc, HGDIOBJ object) { return SelectObject(dc, object); }
    COLORREF TextColor(HDC dc) { return SetTextColor(dc, 0xFFFFFF); }
    COLORREF BackgroundColor(HDC dc) { return SetBkColor(dc, 0); }
    bool DeleteContext(HDC dc) { return DeleteDC(dc) != FALSE; }
    bool DeleteResource(HGDIOBJ object) { return DeleteObject(object) != FALSE; }
};

// Clear the pixel view immediately. Retain handles when cleanup fails so that
// the owner can retry; never delete objects still selected into a live DC.
template<class Api>
bool Release(Api& api, HDC& dc, HBITMAP& bitmap, HFONT& font,
    unsigned int*& bits, BITMAPINFO& info) {
    bits = nullptr;
    info = {};
    if (dc) {
        if (!api.DeleteContext(dc)) return false;
        dc = nullptr;
    }
    bool released = true;
    if (bitmap) {
        if (api.DeleteResource(bitmap)) bitmap = nullptr;
        else released = false;
    }
    if (font) {
        if (api.DeleteResource(font)) font = nullptr;
        else released = false;
    }
    return released;
}

template<class Api>
bool Ensure(Api& api, int width, int height, int weight, const char* face,
    HDC& dc, HBITMAP& bitmap, HFONT& font, unsigned int*& bits, BITMAPINFO& info) {
    if (width <= 0 || height <= 0 || !face || !face[0] ||
        static_cast<std::size_t>(width) > (std::numeric_limits<std::ptrdiff_t>::max)() /
            sizeof(unsigned int) / static_cast<std::size_t>(width)) return false;
    if (dc && bitmap && font && bits && info.bmiHeader.biWidth == width &&
        info.bmiHeader.biHeight == -width) return true;
    if (!Release(api, dc, bitmap, font, bits, info)) return false;

    HDC nextDc = nullptr;
    HBITMAP nextBitmap = nullptr;
    HFONT nextFont = nullptr;
    unsigned int* nextBits = nullptr;
    BITMAPINFO nextInfo{};
    nextInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    nextInfo.bmiHeader.biWidth = width;
    nextInfo.bmiHeader.biHeight = -width;
    nextInfo.bmiHeader.biPlanes = 1;
    nextInfo.bmiHeader.biCompression = BI_RGB;
    nextInfo.bmiHeader.biBitCount = 32;
    const auto rollback = [&]() {
        Release(api, nextDc, nextBitmap, nextFont, nextBits, nextInfo);
        // Normally empty; failed cleanup retains ownership but not readiness.
        dc = nextDc;
        bitmap = nextBitmap;
        font = nextFont;
        bits = nullptr;
        info = {};
        return false;
    };
    nextDc = api.CreateContext();
    if (!nextDc) return rollback();
    nextBitmap = api.CreateBitmap(nextDc, nextInfo, nextBits);
    if (!nextBitmap || !nextBits) return rollback();
    nextFont = api.CreateTypeface(height, weight, face);
    if (!nextFont) return rollback();
    const auto previousBitmap = api.Select(nextDc, nextBitmap);
    if (!previousBitmap || previousBitmap == HGDI_ERROR) return rollback();
    const auto previousFont = api.Select(nextDc, nextFont);
    if (!previousFont || previousFont == HGDI_ERROR) return rollback();
    if (api.TextColor(nextDc) == CLR_INVALID || api.BackgroundColor(nextDc) == CLR_INVALID)
        return rollback();
    dc = nextDc;
    bitmap = nextBitmap;
    font = nextFont;
    bits = nextBits;
    info = nextInfo;
    return true;
}

// Keep the legacy first-token face and nonpositive-weight fallback, while
// bounding the destination and rejecting numeric overflow.
inline void ReadConfig(FILE* file, char (&face)[128], int& weight) {
    char line[256]{};
    if (std::fgets(line, sizeof(line), file))
        sscanf_s(line, "%127s", face, static_cast<unsigned int>(sizeof(face)));
    if (std::fgets(line, sizeof(line), file)) {
        char* end = nullptr;
        errno = 0;
        const long parsed = std::strtol(line, &end, 10);
        if (end != line && errno != ERANGE && parsed >= (std::numeric_limits<int>::min)() &&
            parsed <= (std::numeric_limits<int>::max)()) weight = static_cast<int>(parsed);
    }
    if (weight <= 0) weight = 500;
}
} // namespace font_gdi
