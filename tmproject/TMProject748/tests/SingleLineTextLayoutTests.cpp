#include "../internal/platform/windows/SingleLineTextLayout.h"
#include "../internal/platform/windows/FontGdiResources.h"
#include "../internal/ui/FieldCaptionLayout.h"
#include <cstdio>
#include "../internal/platform/windows/ResourceTextGdi.h"
#include <vector>
#include <limits>
#include <string>

int RunSingleLineTextLayoutTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            ++failures;
            std::fprintf(stderr, "FAIL single-line text layout: %s\n", name);
        }
    };
    const auto unitMeasure = [](const std::string& text) { return static_cast<float>(text.size()); };
    const std::string source = "White Magic";
    check(font_layout::Ellipsize(source, 1, 8, unitMeasure) == "Whit...", "overflow uses complete characters and ellipsis");
    check(font_layout::Ellipsize(source, 1, 100, unitMeasure) == source, "wider bounds restore the complete source");
    check(font_layout::Ellipsize(source, 1, 0, unitMeasure) == source, "removing the bound restores the complete source");
    check(font_layout::Ellipsize(source, 1, 3, unitMeasure).empty(), "unusable bounds do not compress glyphs");
    check(font_layout::Ellipsize("", 1, 10, unitMeasure).empty(), "empty text stays empty");
    check(font_layout::Ellipsize(source, 0, 10, unitMeasure).empty(), "invalid scale has no geometry");
    check(font_layout::Ellipsize(source, 1, 10, [](const std::string&) {
        return std::numeric_limits<float>::infinity();
    }).empty(), "failed measurement has no overflowing geometry");
    const short chunks[]{230, 170, 0};
    check(font_layout::Measure(chunks, 3) == 400, "all texture chunks contribute to width");
    check(field_caption_layout::IsBoundedCaption(513, 5769), "resistance caption opts in");
    check(field_caption_layout::IsBoundedCaption(1905, 5786), "skill points caption opts in");
    for (unsigned int id = 5781; id <= 5785; ++id)
        check(field_caption_layout::IsBoundedCaption(513, id), "character title opts in");
    for (unsigned int id = 5788; id <= 5791; ++id)
        check(field_caption_layout::IsBoundedCaption(1905, id), "skill title opts in");
    for (unsigned int id = 6050; id <= 6052; ++id)
        check(field_caption_layout::IsBoundedCaption(1889, id), "apprentice title opts in");
    for (const auto id : {1057u, 1121u, 5754u, 5758u})
        check(!field_caption_layout::IsBoundedCaption(513, id), "values and unaffected identity labels stay legacy");
    check(!field_caption_layout::IsBoundedCaption(257, 5788), "another panel stays legacy");
    check(!field_caption_layout::IsBoundedCaption(1905, 572), "skill point value stays legacy");

    // FieldScene2 character-panel geometry: the declared rectangles extend
    // past the panel or into the values despite their correct native IDs.
    const font_layout::Rectangle resistanceTitle{169, 115, 90, 16};
    check(font_layout::WithinPanel(resistanceTitle, 227, 2) == 56,
        "resistance title stops inside the panel even when its resource rectangle does not");
    const font_layout::Rectangle resistanceLabel{127, 148, 67, 16};
    const font_layout::Rectangle resistanceValue{174, 149, 67, 16};
    check(font_layout::BeforeNeighbor(resistanceLabel, 67, resistanceValue) == 47,
        "resistance caption reserves its neighboring numeric field");
    check(font_layout::BeforeNeighbor(resistanceLabel, 67, {174, 164, 67, 16}) == 67,
        "a value in the next row cannot shrink the current caption");
    check(font_layout::BeforeNeighbor(resistanceLabel, 67, {10, 148, 67, 16}) == 67,
        "a control on the left cannot shrink the caption");
    check(font_layout::WithinPanel({230, 115, 90, 16}, 227, 2) == 0,
        "a caption outside the panel has no drawing budget");

    // Native SText vtable 005a35c8 + 0x58 binds 0040268c, which draws
    // complete captions without clipping to the decorative texture notches.
    // The identity title at y=27 must not collide with the name row at y=42.
    check(!field_caption_layout::SharesRow({34, 27, 90, 16}, {55, 42, 90, 16}),
        "one-pixel rectangle overlap with the next row cannot truncate a title");
    check(field_caption_layout::SharesRow({67, 309, 67, 16}, {146, 310, 67, 16}),
        "one-pixel baseline offset still reserves the value in the same row");
    const auto skillTab = field_caption_layout::DrawingBounds(1905, 5788,
        {33, 40, 90, 16}, 90, 8, 2, 1);
    check(!skillTab.centered && skillTab.left == 41 && skillTab.right == 121,
        "skill title retains the native full text rectangle and left alignment");
    const auto apprenticeTab = field_caption_layout::DrawingBounds(1889, 6050,
        {14, 40, 90, 16}, 90, 2, 2, 1);
    check(!apprenticeTab.centered && apprenticeTab.left == 16 && apprenticeTab.right == 102,
        "apprentice title retains its full centered control rectangle");
    const auto resistTab = field_caption_layout::DrawingBounds(513, 5784,
        resistanceTitle, 56, 8, 2, 1);
    check(resistTab.left == 177 && resistTab.right == 223,
        "resistance title stays inside the panel without a decorative notch constraint");
    const auto nearNeighbor = field_caption_layout::DrawingBounds(1905, 5788,
        {33, 40, 90, 16}, 40, 8, 2, 1);
    check(nearNeighbor.right == 71, "a title cannot override a nearer neighbor");
    const auto unrelated = field_caption_layout::DrawingBounds(257, 5788,
        {33, 40, 90, 16}, 90, 8, 2, 1);
    check(!unrelated.centered && unrelated.left == 41 && unrelated.right == 121,
        "the same child ID in another panel retains ordinary layout");
    const auto points = field_caption_layout::DrawingBounds(1905, 5786,
        {51, 400, 60, 12}, 60, 8, 2, 1);
    check(!points.centered && points.left == 59 && points.right == 109,
        "skill points keep their own caption/value budget");

    struct CaptionInput { unsigned int parent, control; float x, y, width, leftPadding, rightPadding; const char* text; };
    // FieldScene2.bin coordinates and the shipped EN/PT locale text. Expected
    // acceptance is complete names, not merely containment of an ellipsis.
    const CaptionInput completeCaptions[]{
        {513, 5781, 34, 27, 90, 8, 2, "Meus Dados"},
        {513, 5782, 19, 115, 90, 8, 2, "Meus Dados"},
        {513, 5783, 16, 290, 90, 8, 2, "Kingdom"},
        {513, 5784, 169, 115, 56, 8, 2, "Resist."},
        {513, 5785, 125, 290, 90, 8, 2, "Skill Mastery"},
        {513, 5768, 127, 132, 55, 8, 1, "Sagrado"},
        {513, 5769, 127, 148, 55, 8, 1, "Lightning"},
        {513, 5772, 115, 216, 67, 8, 1, "Move Speed"},
        {513, 5773, 115, 232, 67, 8, 1, "Atq. M\xE1g."},
        {513, 5776, 67, 309, 87, 8, 1, "Weapon Mastery"},
        {513, 5776, 67, 309, 87, 8, 1, "Aprender Arma"},
        {513, 5777, 67, 325, 87, 8, 1, "Magia Branca"},
        {513, 5778, 67, 341, 87, 8, 1, "Magia Negra"},
        {513, 5779, 67, 357, 87, 8, 1, "Magia Especial"},
        {513, 5779, 67, 357, 87, 8, 1, "Espada Magica"},
        {1905, 5788, 33, 40, 90, 8, 2, "Magia Branca"},
        {1905, 5789, 33, 130, 90, 8, 2, "Magia Negra"},
        {1905, 5790, 33, 220, 90, 8, 2, "Magia Especial"},
        {1905, 5790, 33, 220, 90, 8, 2, "Special Magic"},
        {1905, 5788, 33, 40, 90, 8, 2, "Sobreviv\xEAncia"},
        {1889, 6050, 14, 40, 90, 2, 2, "Confian\xE7" "a"},
        {1889, 6052, 14, 220, 90, 2, 2, "Espada M\xE1gica"},
        {1905, 5786, 51, 400, 60, 8, 2, "Pts. Hab."},
    };

    // Measure the reported captions and representative values with real GDI
    // at the same font heights as the client. This does not launch the game.
    const char* captions[]{
        "Resist\xEA" "ncia", "Rel\xE2mpago", "Velocidade de Movimento",
        "Ataque M\xE1gico", "Velocidade de Ataque", "Dom\xEDnio de Habilidades",
        "Pontos de Habilidade", "Magia Branca", "Magia Negra", "Magia Especial",
        "Movement Speed", "Skill Points", "110152", "255/255", "[FelipeTr]",
        "White Magic", "Black Magic", "Special Magic", "Sephira", "Faith",
        "Transmutation", "Magic Sword", "Elemental", "Summon", "Nature",
        "Survival", "Trade", "Capture", "My Info", "Kingdom", "Skill Mastery",
        "A long caption crossing the forty-two-byte texture boundary"
    };
    for (float uiScale : {1.0f, 1.25f, 1.5f, 2.0f}) {
        for (int weight : {500, 700}) {
            HDC dc = nullptr;
            HBITMAP bitmap = nullptr;
            HFONT font = nullptr;
            unsigned int* bits = nullptr;
            BITMAPINFO info{};
            font_gdi::NativeApi api;
            const bool created = font_gdi::Ensure(api, 1024, static_cast<int>(13.0f * uiScale),
                weight, "Tahoma", dc, bitmap, font, bits, info);
            check(created, "real client-sized GDI font is available");
            if (created) {
                const auto measure = [&](const std::string& text) {
                    float width = 0;
                    for (size_t offset = 0; offset < text.size(); offset += 42) {
                        SIZE size{};
                        const int count = static_cast<int>((std::min)(size_t{42}, text.size() - offset));
                        if (!resource_text::Measure(dc, text.data() + offset, count, &size))
                            return std::numeric_limits<float>::infinity();
                        width += size.cx;
                    }
                    return width;
                };
                const auto raster = [&](const std::string& text) {
                    RECT area{0, 0, 1024, 64};
                    check(FillRect(dc, &area, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH))) != 0,
                        "test mask clears successfully");
                    int x = 0;
                    for (size_t offset = 0; offset < text.size(); offset += 42) {
                        const auto chunk = text.substr(offset, 42);
                        check(resource_text::Draw(dc, x, 0, chunk.data(), static_cast<int>(chunk.size())) != FALSE,
                            "caption rasterizes successfully");
                        x += static_cast<int>(measure(chunk));
                    }
                    GdiFlush();
                    return std::vector<unsigned int>(bits, bits + 1024 * 64);
                };
                for (const auto* caption : captions) {
                    const std::string text(caption);
                    const auto baseline = raster(text);
                    check(measure(text) > 0 && std::isfinite(measure(text)), "caption has real GDI metrics");
                    const auto fitting = font_layout::Ellipsize(text, 1, measure(text) + 1, measure);
                    check(fitting == text && raster(fitting) == baseline,
                        "fitting text retains identical glyph pixels, including accents and chunk seams");
                    for (float baseWidth : {37.0f, 40.0f, 46.0f, 60.0f, 80.0f, 120.0f}) {
                        const float budget = baseWidth * uiScale;
                        const auto display = font_layout::Ellipsize(text, 1, budget, measure);
                        check(display.empty() || measure(display) + 1 <= budget,
                            "original-size glyphs and shadow stay inside the bound");
                        if (measure(text) + 1 <= budget)
                            check(display == text, "fitting text is never shortened");
                        else if (!display.empty())
                            check(display.size() >= 3 && display.substr(display.size() - 3) == "..." &&
                                text.compare(0, display.size() - 3, display, 0, display.size() - 3) == 0,
                                "overflow retains an intact source prefix and ellipsis");
                    }
                }
                if (weight == 500) for (const auto& caption : completeCaptions) {
                    const font_layout::Rectangle rect{caption.x * uiScale, caption.y * uiScale,
                        caption.width * uiScale, 16 * uiScale};
                    const auto band = field_caption_layout::DrawingBounds(caption.parent,
                        caption.control, rect, caption.width * uiScale,
                        caption.leftPadding * uiScale, caption.rightPadding * uiScale, uiScale);
                    const float budget = band.right - band.left;
                    const std::string text(caption.text);
                    const auto display = font_layout::Ellipsize(text, 1, budget, measure);
                    if (display != text)
                        std::fprintf(stderr, "incomplete caption '%s' scale %.2f budget %.2f width %.2f\n",
                            caption.text, uiScale, budget, measure(text));
                    check(display == text && raster(display) == raster(text),
                        "shipped field names remain complete with original native-sized glyphs");
                }
            }
            check(font_gdi::Release(api, dc, bitmap, font, bits, info), "GDI test resources release successfully");
        }
    }
    return failures;
}
