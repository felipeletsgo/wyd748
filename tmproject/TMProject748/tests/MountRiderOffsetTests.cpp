#include "../internal/game/entities/MountRiderOffset.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

namespace {
std::string ReadRiderSource(const char* relative)
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH)
        return {};
    auto current = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !current.empty(); ++depth) {
        std::ifstream input(current / "tmproject/TMProject748" / relative, std::ios::binary);
        if (input) {
            std::string source{std::istreambuf_iterator<char>(input), {}};
            source.erase(std::remove(source.begin(), source.end(), '\r'), source.end());
            return source;
        }
        current = current.parent_path();
    }
    return {};
}

struct RiderCase {
    int type, mountMesh0, lookMesh0, lookMesh1, riderClass;
    float mountScale, riderScale, x, y, z;
};
}

// Expected values come from emulating the KR WYD.exe rider sequence
// (0x5588cc..0x558db0); native 7.48 cases come from WYD.exe 0x504040..0x5042bd.
int RunMountRiderOffsetTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* message) {
        ++checks;
        if (!condition) {
            ++failures;
            std::printf("FAIL: %s\n", message);
        }
    };
    auto closeTo = [](float a, float b) { return std::fabs(a - b) < 1e-4f; };

    static const RiderCase krCases[] = {
        {38, 2, 2, 2, 0, 1.0f, 1.0f, -0.600000f, 1.000000f, 0.200000f},
        {38, 4, 4, 4, 0, 1.0f, 1.0f, -0.600000f, 1.000000f, 0.200000f},
        {38, 0, 0, 45, 0, 0.9f, 1.0f, -0.600000f, 1.111111f, 0.180000f},
        {38, 0, 0, 5, 0, 1.0f, 1.0f, -0.370000f, 1.000000f, 0.200000f},
        {31, 17, 17, 17, 0, 1.0f, 1.0f, -0.600000f, 1.000000f, 0.200000f},
        {31, 8, 8, 8, 0, 1.0f, 1.0f, -0.370000f, 1.000000f, 0.200000f},
        {48, 1, 1, 1, 0, 0.8f, 1.0f, -1.096000f, 1.250000f, 0.360000f},
        {49, 24, 24, 24, 0, 0.7f, 1.0f, -0.459000f, 1.428571f, 0.140000f},
        {52, 0, 0, 0, 40, 0.7f, 1.2f, -0.409000f, 1.200000f, -0.060000f},
        {50, 4, 4, 4, 0, 1.0f, 1.0f, -1.070000f, 1.000000f, 0.200000f},
        {50, 5, 5, 5, 0, 0.7f, 1.0f, -0.959000f, 1.428571f, 0.040000f},
        {51, 0, 0, 0, 40, 1.0f, 1.2f, -0.380000f, 1.200000f, 0.300000f},
        {53, 0, 0, 0, 0, 1.0f, 1.0f, -0.280000f, 1.000000f, 0.400000f},
        {59, 1, 1, 1, 0, 1.0f, 1.0f, -0.180000f, 1.000000f, -0.200000f},
        {34, 0, 0, 0, 40, 1.0f, 1.2f, -0.180000f, 1.000000f, -0.200000f},
        {35, 0, 0, 0, 0, 1.0f, 1.0f, -0.300000f, 1.000000f, -0.250000f},
        {41, 0, 0, 0, 0, 1.0f, 1.0f, -0.450000f, 1.000000f, -0.250000f},
        {29, 5, 5, 5, 0, 1.25f, 1.0f, -0.200000f, 0.800000f, -0.750000f},
        {29, 10, 10, 10, 40, 1.25f, 1.2f, -0.200000f, 0.800000f, -0.750000f},
        {29, 6, 6, 6, 0, 1.0f, 1.0f, -0.200000f, 1.100000f, -0.600000f},
        {29, 7, 7, 7, 0, 0.8f, 1.0f, -0.400000f, 1.375000f, -0.480000f},
        {29, 0, 0, 0, 0, 1.0f, 1.0f, -0.370000f, 1.200000f, -0.600000f},
        {29, 11, 11, 11, 0, 0.8f, 1.0f, -0.280000f, 1.250000f, -0.640000f},
        {29, 41, 41, 41, 0, 1.0f, 1.0f, -0.100000f, 1.000000f, -0.600000f},
        {20, 48, 48, 48, 0, 0.5f, 1.0f, 0.250000f, 2.000000f, -0.050000f},
        {20, 50, 50, 50, 0, 1.0f, 1.0f, 0.200000f, 1.000000f, -0.500000f},
        {20, 6, 6, 6, 0, 0.8f, 1.0f, 0.000000f, 1.250000f, -0.640000f},
        {20, 7, 7, 7, 40, 1.0f, 1.2f, 0.500000f, 2.040000f, -0.100000f},
        {20, 1, 1, 1, 40, 1.0f, 1.2f, 0.172414f, 0.923077f, -0.100000f},
        {25, 3, 3, 3, 0, 1.0f, 1.0f, -0.350000f, 1.000000f, -0.100000f},
        {28, 0, 0, 0, 40, 1.0f, 1.2f, -0.150000f, 1.200000f, 0.100000f},
        {39, 1, 1, 1, 0, 1.0f, 1.0f, -0.370000f, 1.000000f, 0.200000f},
        {40, 46, 46, 46, 40, 0.9f, 1.2f, -0.283000f, 1.200000f, 0.180000f},
    };
    for (const auto& c : krCases) {
        const auto o = mount_rider::Compute(c.type, c.mountMesh0, c.lookMesh0, c.lookMesh1,
            c.riderClass, c.mountScale, c.riderScale, false);
        char message[160]{};
        std::snprintf(message, sizeof(message), "KR rider offset type=%d mesh=%d/%d/%d class=%d",
            c.type, c.mountMesh0, c.lookMesh0, c.lookMesh1, c.riderClass);
        check(closeTo(o.x, c.x) && closeTo(o.y, c.y) && closeTo(o.z, c.z), message);
    }

    // KR Iceland Wolf (item 6011, type 38, mesh 2) sits 0.23 higher on the
    // saddle bone than the default the client used before.
    const auto wolf = mount_rider::Compute(38, 2, 2, 2, 0, 1.0f, 1.0f, false);
    check(closeTo(wolf.x, -0.6f), "KR wolf rider uses the KR saddle offset");

    // Native 7.48 mounts keep the native rules: Tamed Bear (type 29, mesh 0)
    // and Dragon (type 20, mesh 0) ignore the KR-only meshes and adjustments.
    const auto bear = mount_rider::Compute(29, 0, 0, 0, 0, 1.0f, 1.0f, true);
    check(closeTo(bear.x, -0.37f) && closeTo(bear.y, 1.0f) && closeTo(bear.z, -0.6f), "native bear rider");
    const auto tiger = mount_rider::Compute(29, 1, 1, 1, 0, 1.0f, 1.0f, true);
    check(closeTo(tiger.x, -0.1f) && closeTo(tiger.z, -0.6f), "native type 29 mesh 1 rider");
    const auto dragon = mount_rider::Compute(20, 0, 0, 0, 0, 1.0f, 1.0f, true);
    check(closeTo(dragon.x, 0.25f) && closeTo(dragon.z, -0.1f), "native dragon rider");
    const auto papy = mount_rider::Compute(20, 7, 7, 7, 40, 1.0f, 1.2f, true);
    check(closeTo(papy.x, 0.5f) && closeTo(papy.y, 1.2f * 1.7f), "native class-40 rider on Dragon Papy");
    const auto horse = mount_rider::Compute(31, 0, 0, 0, 40, 1.0f, 1.2f, true);
    check(closeTo(horse.x, -0.32f) && closeTo(horse.y, 1.2f) && closeTo(horse.z, 0.2f), "native default class-40 rider");

    const auto render = ReadRiderSource("internal/game/entities/TMHumanRender.cpp");
    check(render.find("mount_rider::Compute(") != std::string::npos &&
        render.find("vecPosScale.x = vecPosScale.x + 0.9f;") == std::string::npos,
        "TMHuman::Render uses the evidence-based rider offsets instead of the 7.69 adjustments");
    // The 39 KR costume mounts imported from MountDataV.bin need a visual
    // row; without one the client would draw the default horse.
    const auto mounts = ReadRiderSource("internal/game/entities/TMHumanMounts.cpp");
    static const int importedKR[] = {4236, 4237, 4238, 4239, 4240, 4242, 4243, 4250, 4251,
        4270, 4271, 4272, 4273, 4274, 4275, 4276, 4277, 4278, 4279, 4280, 4281, 4282, 4283,
        4284, 4285, 4286, 4287, 4288, 4289, 4290, 4291, 4292, 4293, 4294, 4295, 4296, 4297,
        4298, 4299};
    int visualRows = 0;
    for (const int item : importedKR) {
        char row[16]{};
        std::snprintf(row, sizeof(row), "{%d, ", item);
        if (mounts.find(row) != std::string::npos)
            ++visualRows;
    }
    check(visualRows == 39, "every imported KR costume mount has a visual row");
    return failures;
}
