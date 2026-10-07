#include "../internal/platform/diagnostics/PerfTrace.h"
#include <cstdio>
#include <cstring>
#include <string>

namespace {
int CountColumns(const char* text)
{
    int columns = 1;
    for (const char* p = text; *p && *p != '\n'; ++p)
        columns += *p == ',' ? 1 : 0;
    return columns;
}
}

int RunPerfTraceTests(int& checks)
{
    using namespace perf_trace;
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL perf trace: %s\n", name); }
    };

    Interval in;
    in.frames = 30;
    in.frameMs = 30 * 33.0;
    in.maxFrameMs = 70.0;
    in.hitches = 1;
    in.phaseMs[static_cast<int>(Phase::FrameMove)] = 30 * 2.0;
    in.phaseMs[static_cast<int>(Phase::Render)] = 30 * 10.0;
    in.phaseMs[static_cast<int>(Phase::Present)] = 30 * 4.0;
    in.drawCalls = 30 * 500;
    Snapshot snapshot;
    snapshot.sceneType = 7;
    snapshot.nodes = 1200;

    char row[512]{};
    const int length = FormatRow(row, sizeof(row), 12.0, 1.0, in, snapshot);
    check(length > 0 && length < static_cast<int>(sizeof(row)), "row fits its buffer");
    check(CountColumns(row) == CountColumns(CsvHeader()), "row and header have the same columns");
    check(std::strncmp(row, "12.0,30.0,33.00,70.0,1,2.000,6.000,4.000,", 41) == 0,
        "render time excludes Present and values are per-frame averages");
    check(std::string(row).find(",500,") != std::string::npos, "draw calls are per frame");

    Interval empty;
    const int emptyLength = FormatRow(row, sizeof(row), 0.0, 0.0, empty, Snapshot{});
    check(emptyLength > 0 && std::strncmp(row, "0.0,0.0,0.00,", 13) == 0,
        "an empty window does not divide by zero");
    return failures;
}
