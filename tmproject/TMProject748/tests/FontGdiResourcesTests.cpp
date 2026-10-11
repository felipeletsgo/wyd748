#include "../internal/platform/windows/FontGdiResources.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <string>

namespace {
template<class Handle>
Handle Token(std::uintptr_t value) { return reinterpret_cast<Handle>(value); }

struct State {
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HFONT font = nullptr;
    unsigned int* bits = nullptr;
    BITMAPINFO info{};
    template<class Api> bool Ensure(Api& api, int width = 16, int height = 12,
        int weight = 500, const char* face = "Tahoma") {
        return font_gdi::Ensure(api, width, height, weight, face, dc, bitmap, font, bits, info);
    }
    template<class Api> bool Release(Api& api) {
        return font_gdi::Release(api, dc, bitmap, font, bits, info);
    }
    bool Empty() const { return !dc && !bitmap && !font && !bits && !info.bmiHeader.biSize; }
};

struct Fixture {
    std::array<unsigned int, 256> pixels{};
    bool context = false;
    bool bitmap = false;
    bool font = false;
    bool selected = false;
    bool orderingError = false;
    bool nullPixels = false;
    bool selectNull = false;
    bool failDeleteContext = false;
    int failDeleteResource = 0;
    int failAt = 0;
    int operations = 0;
    int deletes = 0;
    int fontHeight = 0;
    int fontWeight = 0;
    std::string face;
    BITMAPINFO bitmapInfo{};
    bool Next() { return ++operations != failAt; }
    HDC CreateContext() {
        if (!Next()) return nullptr;
        context = true;
        return Token<HDC>(1);
    }
    HBITMAP CreateBitmap(HDC, const BITMAPINFO& info, unsigned int*& bits) {
        bitmapInfo = info;
        if (!Next()) return nullptr;
        bitmap = true;
        bits = nullPixels ? nullptr : pixels.data();
        return Token<HBITMAP>(2);
    }
    HFONT CreateTypeface(int height, int weight, const char* name) {
        fontHeight = height;
        fontWeight = weight;
        face = name;
        if (!Next()) return nullptr;
        font = true;
        return Token<HFONT>(3);
    }
    HGDIOBJ Select(HDC, HGDIOBJ) {
        if (!Next()) return selectNull ? nullptr : HGDI_ERROR;
        selected = true;
        return Token<HGDIOBJ>(4);
    }
    COLORREF TextColor(HDC) { return Next() ? 0 : CLR_INVALID; }
    COLORREF BackgroundColor(HDC) { return Next() ? 0xFFFFFF : CLR_INVALID; }
    bool DeleteContext(HDC) {
        ++deletes;
        if (failDeleteContext) return false;
        context = false;
        selected = false;
        return true;
    }
    bool DeleteResource(HGDIOBJ object) {
        ++deletes;
        if (context && selected) { orderingError = true; return false; }
        const auto value = reinterpret_cast<std::uintptr_t>(object);
        if (value == static_cast<std::uintptr_t>(failDeleteResource)) return false;
        if (value == 2) bitmap = false;
        if (value == 3) font = false;
        return true;
    }
    bool Empty() const { return !context && !bitmap && !font; }
};
}

int RunFontGdiResourcesTests(int& checks) {
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL font GDI: %s\n", name);
        }
    };
    for (int failure = 1; failure <= 7; ++failure) {
        for (const bool nullSelection : {false, true}) {
            Fixture api;
            State state;
            api.failAt = failure;
            api.selectNull = nullSelection;
            check(!state.Ensure(api), "creation boundary rejects failure");
            check(api.operations == failure, "no creation continues after failure");
            check(state.Empty(), "failed creation publishes no resources");
            check(api.Empty() && !api.orderingError, "rollback releases every partial handle in order");
            api.failAt = 0;
            check(state.Ensure(api), "failed creation can be retried");
            check(state.Release(api) && state.Empty() && api.Empty(), "retry cleanup completes");
        }
    }
    {
        Fixture api;
        State state;
        api.nullPixels = true;
        check(!state.Ensure(api) && state.Empty() && api.Empty(), "bitmap without pixels is rolled back");
        check(api.operations == 2, "no font created without bitmap storage");
    }
    for (const int width : {0, -1, (std::numeric_limits<int>::max)()}) {
        Fixture api;
        State state;
        check(!state.Ensure(api, width) && !api.operations && state.Empty(), "invalid extent rejected before allocation");
    }
    for (const int height : {0, -1}) {
        Fixture api;
        State state;
        check(!state.Ensure(api, 16, height) && !api.operations, "invalid font height rejected");
    }
    for (const char* face : {static_cast<const char*>(nullptr), ""}) {
        Fixture api;
        State state;
        check(!state.Ensure(api, 16, 12, 500, face) && !api.operations, "missing face rejected");
    }
    {
        Fixture api;
        State state;
        check(state.Ensure(api), "complete creation succeeds");
        check(state.dc && state.bitmap && state.font && state.bits == api.pixels.data(), "complete resources published together");
        check(api.bitmapInfo.bmiHeader.biWidth == 16 && api.bitmapInfo.bmiHeader.biHeight == -16 &&
            api.bitmapInfo.bmiHeader.biBitCount == 32 && api.bitmapInfo.bmiHeader.biPlanes == 1 &&
            api.bitmapInfo.bmiHeader.biCompression == BI_RGB, "top-down 32-bit square DIB preserved");
        check(api.fontHeight == 12 && api.fontWeight == 500 && api.face == "Tahoma", "font settings forwarded unchanged");
        const int allocations = api.operations;
        check(state.Ensure(api) && api.operations == allocations, "ready cache reused without allocation");
        check(state.Release(api) && state.Empty() && api.Empty() && !api.orderingError, "context deleted before selected objects");
        const int deletes = api.deletes;
        check(state.Release(api) && api.deletes == deletes, "repeated invalidation is harmless");
        check(state.Ensure(api) && state.Release(api), "restore after invalidation succeeds");
    }
    for (int partial = 1; partial <= 7; ++partial) {
        Fixture api;
        State state;
        if (partial & 1) { state.dc = Token<HDC>(1); api.context = true; }
        if (partial & 2) { state.bitmap = Token<HBITMAP>(2); api.bitmap = true; }
        if (partial & 4) { state.font = Token<HFONT>(3); api.font = true; }
        state.bits = api.pixels.data();
        check(state.Ensure(api), "partial cache repaired");
        check(state.Release(api) && state.Empty() && api.Empty(), "partial cache leaves no orphan handles");
    }
    for (int failure = 1; failure <= 3; ++failure) {
        Fixture api;
        State state;
        check(state.Ensure(api), "cleanup failure fixture created");
        api.failDeleteContext = failure == 1;
        api.failDeleteResource = failure == 1 ? 0 : failure;
        check(!state.Release(api), "cleanup error is reported");
        check(!state.bits && !state.info.bmiHeader.biSize, "cleanup failure invalidates pixel view");
        check(failure == 1 ? state.dc && state.bitmap && state.font :
            failure == 2 ? state.bitmap && !state.dc && !state.font : state.font && !state.dc && !state.bitmap,
            "failed cleanup preserves only unreleased handles");
        const int calls = api.operations;
        check(!state.Ensure(api) && api.operations == calls, "failed old cleanup blocks replacement allocations");
        api.failDeleteContext = false;
        api.failDeleteResource = 0;
        check(state.Ensure(api) && state.Release(api) && api.Empty(), "retained handles released on next retry");
        check(!api.orderingError, "cleanup failure never deletes selected objects");
    }
    {
        Fixture api;
        State state;
        api.failAt = 7;
        api.failDeleteContext = true;
        check(!state.Ensure(api) && state.dc && state.bitmap && state.font && !state.bits,
            "rollback cleanup failure retains ownership without publishing readiness");
        api.failDeleteContext = false;
        check(state.Release(api) && state.Empty() && api.Empty(), "failed rollback cleanup is retryable");
    }
    const auto parse = [&](const std::string& content, const char* expectedFace, int initialWeight, int expectedWeight) {
        FILE* file = nullptr;
        if (tmpfile_s(&file) || !file) { check(false, "font config temporary file available"); return; }
        std::fwrite(content.data(), 1, content.size(), file);
        std::rewind(file);
        char face[128] = "Tahoma";
        int weight = initialWeight;
        font_gdi::ReadConfig(file, face, weight);
        std::fclose(file);
        check(std::strcmp(face, expectedFace) == 0, "bounded legacy face parsing");
        check(weight == expectedWeight, "bounded legacy weight parsing");
    };
    parse("Arial\n700\n", "Arial", 500, 700);
    parse("Times New Roman\n600 trailing\n", "Times", 500, 600);
    parse("\n0\n", "Tahoma", 500, 500);
    parse("Arial\n-1\n", "Arial", 700, 500);
    parse("Arial\ninvalid\n", "Arial", 700, 700);
    parse("Arial\n999999999999999999999999\n", "Arial", 700, 700);
    parse("Arial\n-999999999999999999999999\n", "Arial", 700, 700);
    parse("Arial", "Arial", 700, 700);
    parse("", "Tahoma", 0, 500);
    parse(std::string(200, 'A') + "\n650\n", std::string(127, 'A').c_str(), 500, 650);

    // Real Win32 resources, never a window, game process or visual acceptance.
    font_gdi::NativeApi native;
    State real;
    for (int cycle = 0; cycle < 16; ++cycle) {
        const bool created = real.Ensure(native);
        check(created, "headless native GDI creation");
        if (created) {
            DIBSECTION section{};
            const int copied = GetObjectA(real.bitmap, sizeof(section), &section);
            check(copied == sizeof(section) &&
                section.dsBm.bmBits == real.bits && section.dsBm.bmWidth == 16 &&
                section.dsBm.bmHeight == 16 && section.dsBmih.biBitCount == 32,
                "native DIB contract and storage");
            check(GetCurrentObject(real.dc, OBJ_BITMAP) == real.bitmap &&
                GetCurrentObject(real.dc, OBJ_FONT) == real.font, "native bitmap and font selected");
            check(GetTextColor(real.dc) == 0xFFFFFF && GetBkColor(real.dc) == 0, "native raster colors preserved");
            // GetObject normalizes the reported height. Verify actual row order
            // with an off-screen GDI write instead of depending on that sign.
            real.bits[0] = 0;
            real.bits[15 * 16] = 0;
            check(SetPixelV(real.dc, 0, 0, RGB(255, 0, 0)) && GdiFlush() &&
                (real.bits[0] & 0xFFFFFF) == 0xFF0000 && real.bits[15 * 16] == 0,
                "native top-down storage matches GDI pixel coordinates");
        }
        check(real.Release(native) && real.Empty(), "headless native reset cleanup");
    }
    return failures;
}
