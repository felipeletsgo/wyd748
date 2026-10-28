#include "../internal/core/QuestSubjectLevel.h"
#include "../internal/platform/windows/ResourceTextGdi.h"
#include "../internal/platform/windows/FontGdiResources.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>

int RunLocaleBoundaryTests(int& checks) {
    int failures = 0;
    const auto check = [&](bool passed, const char* name) {
        ++checks;
        if (!passed) { ++failures; std::fprintf(stderr, "FAIL locale boundary: %s\n", name); }
    };
    struct Guarded { char before = 'L'; char levels[500]{}; char after = 'R'; } data;
    check(quest_subject::Apply(data.levels, sizeof(data.levels), "001 FFFFFF LV 1", 97), "first level");
    check(quest_subject::Apply(data.levels, sizeof(data.levels), "500 FF00FF LV 500", 98), "last level");
    check(data.levels[0] == 97 && data.levels[499] == 98, "native one-based indices");
    check(quest_subject::Apply(data.levels, sizeof(data.levels), "001 00FFFF", 99) && data.levels[0] == 99,
        "later files override an existing level");
    for (const char* row : {"", "\n", "# comment", "000 FFFFFF", "-01 FFFFFF", "501 FFFFFF",
        "9999999999", "001", "001 ZZZZZZ", "001 FFZZFF", "1FF FFFFFF"}) {
        const auto before = data;
        check(!quest_subject::Apply(data.levels, sizeof(data.levels), row, 100), "malformed row rejected");
        check(std::memcmp(&before, &data, sizeof(data)) == 0, "rejected row leaves destination untouched");
    }
    check(data.before == 'L' && data.after == 'R', "quest table guards intact");
    check(!quest_subject::Apply(nullptr, 500, "001 FFFFFF", 1) &&
        !quest_subject::Apply(data.levels, 0, "001 FFFFFF", 1), "invalid destination rejected");

    std::wstring decoded;
    const char accented[] = "Resist\xeancia / Rel\xe2mpago / \x80";
    check(resource_text::Decode(accented, static_cast<int>(sizeof(accented) - 1), decoded) &&
        decoded == L"Resist\u00eancia / Rel\u00e2mpago / \u20ac", "explicit Windows-1252 accents and Euro");
    check(!resource_text::Decode(nullptr, 1, decoded) && !resource_text::Decode("x", -1, decoded),
        "invalid input rejected");
    check(resource_text::Decode("", 0, decoded) && decoded.empty(), "empty text supported");
    HDC dc = nullptr; HBITMAP bitmap = nullptr; HFONT font = nullptr;
    unsigned int* bits = nullptr; BITMAPINFO info{};
    font_gdi::NativeApi api;
    const bool ready = font_gdi::Ensure(api, 512, 16, 500, "Tahoma", dc, bitmap, font, bits, info);
    check(ready, "real GDI fixture");
    if (ready) {
        SIZE actual{}, expected{};
        constexpr wchar_t wide[] = L"Resist\u00eancia / Rel\u00e2mpago / \u20ac";
        check(resource_text::Measure(dc, accented, static_cast<int>(sizeof(accented) - 1), &actual) &&
            GetTextExtentPoint32W(dc, wide, static_cast<int>(std::wcslen(wide)), &expected) &&
            actual.cx == expected.cx && actual.cy == expected.cy, "production width matches Unicode glyphs");
        check(resource_text::Draw(dc, 0, 0, accented, static_cast<int>(sizeof(accented) - 1)), "production drawing");
        check(!resource_text::Measure(dc, accented, 1, nullptr), "invalid extent rejected");
    }
    check(font_gdi::Release(api, dc, bitmap, font, bits, info), "GDI cleanup");
    return failures;
}
