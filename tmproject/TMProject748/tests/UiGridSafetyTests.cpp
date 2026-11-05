#include "../internal/ui/GridInputPolicy.h"
#include "../internal/ui/GridItemPresentation.h"
#include "../internal/ui/RenderControlList.h"
#include "../internal/ui/TooltipPolicy.h"
#include "../internal/ui/EditableTextBounds.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace
{
struct FakeGeom
{
    FakeGeom* m_pNextGeom = nullptr;
};

struct FakeGeomList
{
    FakeGeom* pHeadGeom = nullptr;
    FakeGeom* pTailGeom = nullptr;
};

struct FakeRow
{
    int writes = 0;
};
}

int RunUiGridSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    check(std::strcmp(grid_item_presentation::SplitCaption("Digite a quantidade para dividir."),
        "Digite a quantidade para dividir.") == 0 &&
        std::strcmp(grid_item_presentation::SplitCaption(" \t\r\n"),
            "Enter the quantity to split.") == 0 &&
        std::strcmp(grid_item_presentation::EntranceTicketCaption(nullptr),
            "Use this Entrance Ticket to enter?") == 0,
        "A26-07 localized prompts use the resource or the English fallback");

    // A26-02: Carry control 337 is 215 x 167 units with 9 x 7 cells. Every point
    // inside the rectangle maps to the cell that SGridControl::FrameMove2 draws
    // under it (origin k * extent / count), at every UI scale.
    {
        const float scales[] = { 1.0f, 1.2f, 1.25f, 1.6f, 1.8f };
        bool inRange = true;
        bool matchesDrawing = true;
        for (const float scale : scales)
        {
            const float width = 215.0f * scale;
            const float height = 167.0f * scale;
            const float extents[2] = { width, height };
            const int counts[2] = { 9, 7 };
            for (int axis = 0; axis < 2; ++axis)
            {
                const float extent = extents[axis];
                const int count = counts[axis];
                const float drawnCell = extent / static_cast<float>(count);
                for (int step = 0; step < static_cast<int>(extent * 4.0f); ++step)
                {
                    const float offset = static_cast<float>(step) * 0.25f;
                    if (offset >= extent)
                        break;
                    const int cell = grid_cell_geometry::CellAt(offset, extent, count);
                    if (cell < 0 || cell >= count)
                        inRange = false;
                    else
                    {
                        const bool startsBefore = static_cast<float>(cell) * drawnCell <= offset;
                        const bool endsAfter = offset < static_cast<float>(cell + 1) * drawnCell ||
                            cell == count - 1;
                        if (!startsBefore || !endsAfter)
                            matchesDrawing = false;
                    }
                }
            }
        }
        check(inRange, "A26-02 every point inside Carry maps to a cell in 9 x 7");
        check(matchesDrawing, "A26-02 hit-test cell equals the drawn cell at scales 1.0..1.8");
        // The integer cell size (23) mapped these points to column 9 and row 7.
        check(grid_cell_geometry::CellAt(214.0f, 215.0f, 9) == 8 &&
            grid_cell_geometry::CellAt(166.0f, 167.0f, 7) == 6,
            "A26-02 last pixels of Carry stay in the last column and row");
        // Column 7 is drawn from 167.22 to 191.11; 185 was column 8 before.
        check(grid_cell_geometry::CellAt(185.0f, 215.0f, 9) == 7,
            "A26-02 right part of a column selects that column");
        check(grid_cell_geometry::CellAt(-0.5f, 215.0f, 9) < 0 &&
            grid_cell_geometry::CellAt(-23.0f, 215.0f, 9) < 0 &&
            grid_cell_geometry::CellAt(215.0f, 215.0f, 9) >= 9,
            "A26-02 points outside the grid are outside the cell range");
        check(grid_cell_geometry::CellAt(10.0f, 215.0f, 0) == -1 &&
            grid_cell_geometry::CellAt(10.0f, 0.0f, 9) == -1 &&
            grid_cell_geometry::CellAt(std::nanf(""), 215.0f, 9) == -1,
            "A26-02 degenerate grids give no cell");
        check(grid_cell_geometry::LegacyCellSize(215.0f, 9) == 23 &&
            grid_cell_geometry::LegacyCellSize(225.0f, 9) == 25 &&
            grid_cell_geometry::LegacyCellSize(10.0f, 0) == 0,
            "A26-02 SwapItem keeps its integer footprint size");
    }

    // A26-03: Delete clears a shortcut only when the selected icon is on the page.
    {
        check(skill_belt_delete::ShortcutIndex(-1, true) == -1 &&
            skill_belt_delete::ShortcutIndex(-1, false) == -1,
            "A26-03 a selection on the other page clears nothing");
        check(skill_belt_delete::ShortcutIndex(3, false) == 3 &&
            skill_belt_delete::ShortcutIndex(3, true) == 13 &&
            skill_belt_delete::ShortcutIndex(9, true) == 19,
            "A26-03 page two clears 10 + slot");
        check(skill_belt_delete::ShortcutIndex(10, false) == -1,
            "A26-03 slot past the page is rejected");
    }

    // A26-05: the traversal layer only grows; layers past the table share the
    // top layer, lower layers are unchanged.
    {
        check(render_control_list::ClampTraversalLayer(0, 30) == 0 &&
            render_control_list::ClampTraversalLayer(28, 30) == 28 &&
            render_control_list::ClampTraversalLayer(29, 30) == 29,
            "A26-05 layers inside the table are unchanged");
        check(render_control_list::ClampTraversalLayer(30, 30) == 29 &&
            render_control_list::ClampTraversalLayer(63, 30) == 29 &&
            render_control_list::ClampTraversalLayer(-3, 30) == 0 &&
            render_control_list::ClampTraversalLayer(4, 0) == -1,
            "A26-05 layers past the table use the top layer");

        // Forty visible parents, one leaf each, in traversal order.
        FakeGeomList lists[30]{};
        std::vector<FakeGeom> leaves(40);
        int queued = 0;
        int rawRejected = 0;
        for (int parent = 0; parent < 40; ++parent)
        {
            const int layer = parent + 1;
            FakeGeomList probe[30]{};
            FakeGeom probeNode;
            if (!render_control_list::AppendUnique(probe, 30, static_cast<std::size_t>(layer), &probeNode))
                ++rawRejected;
            const int clamped = render_control_list::ClampTraversalLayer(layer, 30);
            if (render_control_list::AppendUnique(lists, 30, static_cast<std::size_t>(clamped), &leaves[parent]))
                ++queued;
        }
        check(rawRejected == 11, "A26-05 raw layers 30..40 are rejected by the 30-layer table");
        check(queued == 40, "A26-05 every leaf of 40 visible parents is queued");
        for (auto& list : lists)
            render_control_list::Clear(list);
    }

    // A27-01: the "nig" request is throttled; the tooltip still refreshes.
    {
        check(nightmare_tooltip::ShouldRequestTime(0, 123456),
            "A27-01 the first hover requests the server time");
        check(!nightmare_tooltip::ShouldRequestTime(100000, 101000) &&
            !nightmare_tooltip::ShouldRequestTime(100000, 160000) &&
            nightmare_tooltip::ShouldRequestTime(100000, 160001),
            "A27-01 a reply younger than 60 s suppresses the request");
        check(!nightmare_tooltip::ShouldRequestTime(0xFFFFF000u, 0x00001000u),
            "A27-01 the interval survives tick wrap");

        // 100 refreshes 16 ms apart: one request, answered by the server.
        unsigned int last = 0;
        int sends = 0;
        for (unsigned int now = 50000; now < 50000 + 100 * 16; now += 16)
            if (nightmare_tooltip::ShouldRequestTime(last, now)) { ++sends; last = now; }
        check(sends == 1, "A27-01 100 rapid tooltip refreshes send one request");

        // One refresh per second for 130 s: one request per elapsed minute.
        last = 0;
        sends = 0;
        for (unsigned int now = 100000; now < 100000 + 130 * 1000; now += 1000)
            if (nightmare_tooltip::ShouldRequestTime(last, now)) { ++sends; last = now; }
        check(sends == 3, "A27-01 130 s of hovering sends three requests");

        // No server reply: keep the reply clock intact and throttle attempts.
        last = 0;
        bool hasRequest = false;
        sends = 0;
        for (unsigned int now = 0; now < 130000; now += 16)
            if (nightmare_tooltip::ShouldRequestTime(0, now, last, hasRequest))
            {
                ++sends;
                last = now;
                hasRequest = true;
            }
        check(sends == 3, "A27-01 an absent reply permits only three attempts in 130 s, including tick zero");
        check(!nightmare_tooltip::ShouldRequestTime(0, 0x10u, 0xFFFFFFF0u, true) &&
            nightmare_tooltip::ShouldRequestTime(0, 60001u, 0u, true),
            "A27-01 the request clock throttles independently across wrap and at tick zero");
    }

    // A27-02: refine percentage only for levels with a success-table entry.
    {
        struct
        {
            int rate[10];
            int after[8];
        } table{ { 5, 5, 5, 5, 4, 4, 3, 3, 2, 1 }, { 999, 999, 999, 999, 999, 999, 999, 999 } };
        bool inTable = true;
        for (int sanc = 0; sanc <= 8; ++sanc)
            inTable = inTable && refine_tooltip::SuccessPercent(10, sanc, table.rate) == 10 * table.rate[sanc + 1];
        bool pastTable = true;
        for (int sanc = 9; sanc <= 16; ++sanc)
            pastTable = pastTable && refine_tooltip::SuccessPercent(10, sanc, table.rate) == 0;
        check(inTable, "A27-02 levels 0..8 use the next table entry");
        check(pastTable, "A27-02 levels 9..16 never read past the ten entries");
        check(refine_tooltip::SuccessPercent(10, -1, table.rate) == 0,
            "A27-02 a negative level shows no percentage");
    }

    // A27-03: sixteen generated rows write only the twelve FieldScene2 rows.
    {
        FakeRow storage[16]{};
        FakeRow* rows[12]{};
        for (int i = 0; i < 12; ++i)
            rows[i] = &storage[i];
        int written = 0;
        for (int row = 0; row < 16; ++row)
            if (FakeRow* target = tooltip_rows::At(rows, row)) { ++target->writes; ++written; }
        bool tailUntouched = true;
        for (int i = 12; i < 16; ++i)
            tailUntouched = tailUntouched && storage[i].writes == 0;
        check(written == 12 && tailUntouched, "A27-03 rows past twelve are dropped");
        check(tooltip_rows::At(rows, -1) == nullptr && tooltip_rows::At(rows, 11) == &storage[11],
            "A27-03 row bounds are exact");
    }

    // C-35: append without using the destination as a printf source.
    {
        char price[128] = "Price 1,010";
        tooltip_text::Append(price, " (%s:%d%%)", "Tax", 1);
        check(std::strcmp(price, "Price 1,010 (Tax:1%)") == 0, "C-35 append keeps the existing text");

        char name[16] = "Short Sword";
        tooltip_text::Append(name, " [%c]", 'B');
        check(std::strcmp(name, "Short Sword [B]") == 0, "C-35 refine letter is appended");

        char small[8] = "abcdef";
        tooltip_text::Append(small, "%s", "XYZ");
        check(std::strlen(small) == 7 && std::strcmp(small, "abcdefX") == 0,
            "C-35 append truncates at the capacity and terminates");
        char full[4] = "abc";
        tooltip_text::Append(full, "%d", 7);
        check(std::strcmp(full, "abc") == 0, "C-35 a full buffer is unchanged");
    }

    // C-39: edit text, mask, caret and copies stay inside their arrays.
    {
        check(editable_text_bounds::MaxTextLength(20, 256, 256) == 20 &&
            editable_text_bounds::MaxTextLength(400, 256, 256) == 254 &&
            editable_text_bounds::MaxTextLength(300, 256, 100) == 98,
            "C-39 runtime limits are bounded by text and display capacity");
        check(editable_text_bounds::CaretFits(254, 256) && !editable_text_bounds::CaretFits(255, 256),
            "C-39 the caret and terminator fit the display string");

        char display[256];
        std::memset(display, 'x', sizeof(display));
        // Password mode with 200 text and 200 composition bytes wrote 400 bytes before.
        const std::size_t masked = editable_text_bounds::WriteMask(display, 200, '*');
        check(masked == 200 && display[199] == '*' && display[200] == '\0',
            "C-39 password mask equals the committed text length");
        const std::size_t clipped = editable_text_bounds::WriteMask(display, 400, '*');
        check(clipped == 255 && display[255] == '\0', "C-39 mask is clipped to the display");

        std::vector<char> party(100, 'A');
        party[80] = '^';
        party[99] = '\0';
        char name[64];
        const std::size_t copied = editable_text_bounds::CopyPrefix(name, party.data(), 80);
        check(copied == 63 && name[63] == '\0', "C-39 party name before '^' fits its buffer");
        check(editable_text_bounds::CopyPrefix(name, "ab^c", 2) == 2 && std::strcmp(name, "ab") == 0,
            "C-39 short names are copied exactly");
        check(editable_text_bounds::CopyPrefix(name, nullptr, 5) == 0 && name[0] == '\0',
            "C-39 a null source gives an empty string");
    }

    return failures;
}
