#include "../internal/render/world/terrain/TerrainQueryPolicy.h"
#include "../internal/render/world/terrain/TerrainTileMapReader.h"
#include "../internal/render/world/terrain/HeightWindowBounds.h"
#include "../internal/render/world/objects/ObjectFileRecordLayout.h"
#include "../internal/render/world/objects/HallPortalLinks.h"

#include <cmath>
#include <cstdint>
#include <cstdio>

namespace
{
struct FakeBillboard
{
    int id = 0;
};

struct FakePortalScene
{
    FakeBillboard* effect1 = nullptr;
    FakeBillboard* effect2 = nullptr;
};

// A container published its nodes at load and clears them when destroyed.
struct FakeContainer
{
    FakeBillboard effect[2];
    FakeBillboard* published[2] = { nullptr, nullptr };

    void Publish(FakePortalScene& scene)
    {
        scene.effect1 = published[0] = &effect[0];
        scene.effect2 = published[1] = &effect[1];
    }

    void Destroy(FakePortalScene& scene)
    {
        hall_portal_links::ClearIfPublished(scene.effect1, published[0]);
        hall_portal_links::ClearIfPublished(scene.effect2, published[1]);
    }
};
}

int RunWorldTerrainSafetyTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    // C-05 / A25-03 / A25-06: every resolved sample stays in the 4096-tile map.
    {
        bool resolvedOnlyInside = true;
        bool indicesInMap = true;
        bool interiorUnchanged = true;
        for (int y = -1; y <= 65; ++y)
            for (int x = -1; x <= 65; ++x)
                for (int tick = 1; tick <= 2; ++tick)
                {
                    terrain_tile_samples::Quad quad{ -1, -1, -1, -1 };
                    const bool resolved = terrain_tile_samples::Resolve(x, y, tick, tick, quad);
                    const bool inside = x >= 0 && x < 64 && y >= 0 && y < 64;
                    if (resolved != inside)
                        resolvedOnlyInside = false;
                    if (!resolved)
                        continue;
                    const int indices[4] = { quad.i00, quad.i10, quad.i01, quad.i11 };
                    for (const int index : indices)
                        if (index < 0 || index >= 4096)
                            indicesInMap = false;
                    if (x + tick <= 63 && y + tick <= 63 &&
                        (quad.i00 != x + 64 * y || quad.i10 != x + tick + 64 * y ||
                         quad.i01 != x + 64 * (y + tick) || quad.i11 != x + tick + 64 * (y + tick)))
                        interiorUnchanged = false;
                }
        check(resolvedOnlyInside, "C-05 only tiles 0..63 x 0..63 resolve");
        check(indicesInMap, "C-05 every sample index is in [0, 4095] for ticks 1 and 2");
        check(interiorUnchanged, "C-05 interior and LOD tiles keep their legacy samples");

        terrain_tile_samples::Quad quad{};
        terrain_tile_samples::Resolve(63, 10, 1, 1, quad);
        check(quad.i10 == quad.i00 && quad.i11 == quad.i01 && quad.i01 == 63 + 64 * 11,
            "A25-03 the last column repeats its own samples");
        terrain_tile_samples::Resolve(10, 63, 1, 1, quad);
        check(quad.i01 == quad.i00 && quad.i11 == quad.i10 && quad.i10 == 11 + 64 * 63,
            "A25-03 the last row repeats its own samples (GetColor no longer black)");
        terrain_tile_samples::Resolve(63, 63, 1, 1, quad);
        check(quad.i00 == 4095 && quad.i10 == 4095 && quad.i01 == 4095 && quad.i11 == 4095,
            "A25-06 the block corner is a flat tile at 4095, not index 4159");
        check(!terrain_tile_samples::Resolve(64, 63, 1, 1, quad) &&
            !terrain_tile_samples::Resolve(63, 64, 1, 1, quad),
            "A25-03 direct callers at tile 64 get no sample (indices 4096..4223 before)");
    }

    // A25-02: combat quake triggers cannot relocate the terrain block.
    {
        struct Position { int x, y; };
        struct Ground {
            Position m_vecOffset{2304, 3840};
            Position m_vecEffset{0, 0};
            unsigned int m_dwEffStart = 0;
        } ground;
        terrain_quake::Start(ground, 1000, Position{2350, 3900});
        check(ground.m_vecOffset.x == 2304 && ground.m_vecOffset.y == 3840 &&
            ground.m_vecEffset.x == 2350 && ground.m_vecEffset.y == 3900 &&
            ground.m_dwEffStart == 1000,
            "A25-02 a quake updates its center and time without changing the block origin");
    }

    // A25-01: the shake amplitude is a damped value in [-4h, 0] for two seconds.
    {
        const float h = 2.0f;
        const unsigned int start = 500000;
        bool bounded = true;
        bool activeWindow = true;
        for (int elapsed = -200; elapsed <= 2200; ++elapsed)
        {
            float amplitude = 1.0f;
            const bool active = terrain_quake::Amplitude(start + static_cast<unsigned int>(elapsed), start, h, amplitude);
            if (active && (amplitude < -8.0f || amplitude > 0.0f))
                bounded = false;
            if (active != (elapsed < 2000) || (!active && amplitude != 0.0f))
                activeWindow = false;
        }
        check(bounded, "A25-01 amplitude stays in [-8, 0] (it was about 1.7e7)");
        check(activeWindow, "A25-01 the shake lasts 2000 ms, including a frame time behind the start");
        float amplitude = 0.0f;
        terrain_quake::Amplitude(start, start, h, amplitude);
        check(amplitude == -8.0f, "A25-01 full amplitude at the start");
        terrain_quake::Amplitude(start + 1000, start, h, amplitude);
        check(amplitude == -4.0f, "A25-01 half amplitude after one second");
        terrain_quake::Amplitude(start - 100, start, h, amplitude);
        check(amplitude == -8.0f, "A25-01 a start ahead of the frame time counts as elapsed 0");
        check(!terrain_quake::Amplitude(start, 0, h, amplitude) && amplitude == 0.0f,
            "A25-01 no start, no shake");
        check(terrain_quake::Amplitude(0x00000100u, 0xFFFFFF00u, h, amplitude) &&
            std::fabs(amplitude - (-8.0f * 1488.0f / 2000.0f)) < 1e-4f,
            "A25-01 elapsed time is correct across tick wrap");
    }

    // A25-04: water and road texture ranges return walk type 0.
    {
        const int zero[] = { 14, 17, 38, 39, 77, 86, 101, 130, 149 };
        bool allZero = true;
        for (const int index : zero)
            allZero = allZero && terrain_walk_type::ForTexture(index) == 0;
        check(allZero, "A25-04 indices 14..17, 38..77, 86..101, 130..149 return 0");
        check(terrain_walk_type::ForTexture(13) == 3 && terrain_walk_type::ForTexture(18) == 3 &&
            terrain_walk_type::ForTexture(78) == 3 && terrain_walk_type::ForTexture(150) == 3,
            "A25-04 indices outside the ranges keep type 3");
        check(terrain_walk_type::ForTexture(186) == 11 && terrain_walk_type::ForTexture(205) == 8 &&
            terrain_walk_type::ForTexture(230) == 9 && terrain_walk_type::ForTexture(231) == 9,
            "A25-04 special ranges are unchanged");
    }

    // A25-07: sea grids and terrain coordinate rows are validated.
    {
        check(SeaGridIsValid(4, 4) && SeaGridIsValid(64, 56) && SeaGridIsValid(255, 255),
            "A25-07 shipped and maximum sea grids are accepted");
        check(!SeaGridIsValid(0, 4) && !SeaGridIsValid(4, 0) && !SeaGridIsValid(256, 4) &&
            !SeaGridIsValid(-1, 4) && !SeaGridIsValid(11585, 11585),
            "A25-07 grids that wrap the buffer size or indices are rejected");
        const std::uint64_t maxVertices = static_cast<std::uint64_t>(kSeaGridMax + 1) * (kSeaGridMax + 1);
        check(maxVertices - 1 <= 0xFFFFu && maxVertices * 32u < 0xFFFFFFFFull,
            "A25-07 the largest accepted grid fits 16-bit indices and a 32-bit size");

        static FileTileInfo tiles[4096]{};
        check(TerrainTileCoordinatesInRange(tiles), "A25-07 zero coordinates are valid");
        tiles[4095].byTileCoord = 7;
        tiles[17].byBackTileCoord = 31;
        check(TerrainTileCoordinatesInRange(tiles), "A25-07 rows 7 and 31 are valid");
        tiles[4095].byTileCoord = 8;
        check(!TerrainTileCoordinatesInRange(tiles), "A25-07 front row 8 is rejected");
        tiles[4095].byTileCoord = 0;
        tiles[17].byBackTileCoord = static_cast<char>(0xFF);
        check(!TerrainTileCoordinatesInRange(tiles), "A25-07 back row 255 is rejected");
        tiles[17].byBackTileCoord = 0;
    }

    // C-25 helper (applied by the BasedefWorld.cpp owner): window, zone, attribute.
    {
        const int posX = 1920;
        const int posY = 1792;
        bool insideMatchesLegacy = true;
        for (int y = posY; y < posY + 256; y += 17)
            for (int x = posX; x < posX + 256; x += 13)
                if (height_window::Index(x, y, posX, posY, 256, 256) != x + 256 * (y - posY) - posX)
                    insideMatchesLegacy = false;
        check(insideMatchesLegacy, "C-25 inside the window the index equals the legacy expression");
        check(height_window::Index(posX - 1, posY, posX, posY, 256, 256) == -1 &&
            height_window::Index(posX + 256, posY, posX, posY, 256, 256) == -1 &&
            height_window::Index(posX, posY - 1, posX, posY, 256, 256) == -1 &&
            height_window::Index(posX, posY + 256, posX, posY, 256, 256) == -1,
            "C-25 samples outside the 256 x 256 window are rejected");
        const char height[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
        bool samplesMatch = true;
        for (int y = 0; y < 3; ++y)
            for (int x = 0; x < 4; ++x)
                samplesMatch = samplesMatch && height_window::Sample(height,
                    posX + x, posY + y, posX, posY, 4, 3) == x + 4 * y;
        check(samplesMatch, "C-25 height sampling keeps the row stride inside a rectangular window");
        check(height_window::Sample(height, posX - 1, posY, posX, posY, 4, 3) == 127 &&
            height_window::Sample(height, posX + 4, posY, posX, posY, 4, 3) == 127 &&
            height_window::Sample(height, posX, posY - 1, posX, posY, 4, 3) == 127 &&
            height_window::Sample(height, posX, posY + 3, posX, posY, 4, 3) == 127 &&
            height_window::Sample(nullptr, posX, posY, posX, posY, 4, 3) == 127,
            "C-25 missing cells and null maps block without reading storage");
        check(world_zone::Contains(2100, 2100, 2052, 2052, 2171, 2163) &&
            !world_zone::Contains(2200, 2100, 2052, 2052, 2171, 2163) &&
            !world_zone::Contains(2100, 2200, 2052, 2052, 2171, 2163) &&
            world_zone::Contains(2171, 2163, 2052, 2052, 2171, 2163),
            "C-25 village test uses all four rectangle bounds");
        check(attribute_map::Cell(-1) == -1 && attribute_map::Cell(0) == 0 &&
            attribute_map::Cell(4095) == 1023 && attribute_map::Cell(4096) == 0,
            "C-25 attribute cells reject negative coordinates");
    }

    // A15-03 (request R-1): destroying a container clears only its own slots.
    {
        FakePortalScene scene;
        FakeContainer older;
        FakeContainer newer;
        older.Publish(scene);
        newer.Publish(scene);   // the hall was reloaded; the scene names the newer nodes
        older.Destroy(scene);   // cache eviction of the older container
        check(scene.effect1 == &newer.effect[0] && scene.effect2 == &newer.effect[1],
            "A15-03 evicting an older container keeps the newer portal");
        newer.Destroy(scene);
        check(scene.effect1 == nullptr && scene.effect2 == nullptr,
            "A15-03 destroying the publishing container clears the scene slots");
        FakeBillboard* slot = nullptr;
        check(!hall_portal_links::ClearIfPublished(slot, static_cast<FakeBillboard*>(nullptr)) && slot == nullptr,
            "A15-03 a container that published nothing changes nothing");
    }

    return failures;
}
