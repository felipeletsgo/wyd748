#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <windows.h>

// Native FUN_0042e8a1 (inventory) and FUN_004f59b4 (ground) tint ankh meshes
// 737..739 with {0xFFFF4400, 0xFF3366FF, 0xFF00FF00}[mesh - 737]. A decompiled
// stack offset once replaced the table lookup in the inventory renderer.
namespace
{
std::string ReadSource(const char* relative)
{
    wchar_t executable[MAX_PATH]{};
    const auto length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length == MAX_PATH) return {};
    auto directory = std::filesystem::path(executable).parent_path();
    for (int depth = 0; depth < 8 && !directory.empty(); ++depth) {
        std::ifstream input(directory / relative, std::ios::binary);
        if (input) return {std::istreambuf_iterator<char>(input), {}};
        directory = directory.parent_path();
    }
    return {};
}
}

int RunAnkhInventoryTintTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* message) {
        ++checks;
        if (!condition) {
            ++failures;
            std::printf("FAIL: %s\n", message);
        }
    };
    const auto render = ReadSource("tmproject/TMProject748/internal/platform/windows/RenderDevicePrimitives.cpp");
    const auto ground = ReadSource("tmproject/TMProject748/internal/game/entities/TMItem.cpp");
    check(!render.empty() && !ground.empty(), "ankh renderers located");
    const auto table = render.find("dwColor[3] = { 0xFFFF4400, 0xFF3366FF, 0xFF00FF00 }");
    const auto tint = render.find("pVertex[i].diffuse = dwColor[ipControl->n3DObjIndex - 737];", table);
    check(table != std::string::npos && tint != std::string::npos,
        "inventory ankhs use the native red, blue and green vertex tint");
    const auto modulate = render.find("SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);", tint);
    const auto draw = render.find("pMesh->RenderForUI(", modulate);
    const auto restore = render.find("SetTextureStageState(0, D3DTSS_COLOROP, previousColorOp);", draw);
    check(tint != std::string::npos && modulate != std::string::npos && draw != std::string::npos &&
        restore != std::string::npos && render.find("D3DTSS_COLORARG2, D3DTA_DIFFUSE", modulate) < draw,
        "the inventory ankh draw multiplies texture by the vertex tint and restores the stage");
    check(render.find("n3DObjIndex - 2964") == std::string::npos,
        "the decompiled stack offset no longer replaces the ankh color");
    check(ground.find("pVertex[i].diffuse = dwColor[nMeshIndex - 737];") != std::string::npos,
        "ground ankhs keep the same native tint");
    return failures;
}
