#pragma once

#include <cstddef>
#include <cstdio>

// Terrain textures in Env/EnvTextureList3.bin (KR layout, 2048 entries):
// front tiles start at index 10 and mask (back) tiles at index 1024.
constexpr int TerrainFrontTextureBase = 10;
constexpr int TerrainBackTextureBase = 1024;

// KR terrain tile record (12 bytes). Unlike the 7.48 byte layout
// (height, tile, coord, back, back coord, 3 pad, color), the KR files store
// both texture indices as 16-bit values, so front tiles can exceed 245.
struct FileTileInfo
{
    char cHeight;
    char byTileCoord;
    char byBackTileCoord;
    char byReserved;
    unsigned short wTileIndex;
    unsigned short wBackTileIndex;
    unsigned int dwColor;
};
static_assert(sizeof(FileTileInfo) == 12, "KR terrain tile record must be 12 bytes");

// A .trn record is a byte-sized name, that many name bytes, two byte-sized
// coordinates, then 4096 twelve-byte tile records. A failed read leaves the
// output arrays unusable; callers must stop scene initialization.
template <std::size_t NameCapacity, typename Tile, std::size_t TileCount>
bool ReadTerrainTileMapRecord(std::FILE* file, char (&name)[NameCapacity],
    int& posX, int& posY, Tile (&tiles)[TileCount])
{
    static_assert(sizeof(Tile) == 12, "terrain tile record must be 12 bytes");
    static_assert(TileCount == 4096, "terrain must have 4096 tiles");

    unsigned char nameLength = 0;
    unsigned char rawX = 0;
    unsigned char rawY = 0;
    if (!file || std::fread(&nameLength, 1, 1, file) != 1 ||
        nameLength >= NameCapacity ||
        std::fread(name, 1, nameLength, file) != nameLength ||
        std::fread(&rawX, 1, 1, file) != 1 ||
        std::fread(&rawY, 1, 1, file) != 1 ||
        std::fread(tiles, sizeof(Tile), TileCount, file) != TileCount)
        return false;

    name[nameLength] = '\0';
    posX = rawX;
    posY = rawY;
    return true;
}
