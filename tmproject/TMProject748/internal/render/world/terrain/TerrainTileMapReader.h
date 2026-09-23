#pragma once

#include <cstddef>
#include <cstdio>

// The 7.48 .trn record is a byte-sized name, that many name bytes, two
// byte-sized coordinates, then 4096 twelve-byte tile records. A failed read
// leaves the output arrays unusable; callers must stop scene initialization.
template <std::size_t NameCapacity, typename Tile, std::size_t TileCount>
bool ReadTerrainTileMapRecord(std::FILE* file, char (&name)[NameCapacity],
    int& posX, int& posY, Tile (&tiles)[TileCount])
{
    static_assert(sizeof(Tile) == 12, "7.48 terrain tile record must be 12 bytes");
    static_assert(TileCount == 4096, "7.48 terrain must have 4096 tiles");

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
