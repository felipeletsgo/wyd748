#pragma once

#include <cstddef>

struct stTextureListInfo;
struct STRUCT_SPELL;
struct STRUCT_ITEMLIST;

// Selects the asset directory before any relative client file is opened.
// WYD_ASSET_ROOT takes precedence; otherwise the executable directory is used.
bool WYD748_InitializeAssetRoot();

// Loads both the legacy 7.48 texture table (264-byte records) and the newer
// TMProject table (528-byte records) into the modern in-memory representation.
bool WYD748_LoadTextureList(
    const char* path,
    const char* legacyFallbackPath,
    stTextureListInfo* destination,
    std::size_t destinationCount);

// Translates the 104 x 96-byte 7.48 SkillData table into TMProject's larger
// spell table. Modern 104-byte records remain supported as well.
bool WYD748_LoadSkillData(
    const char* path,
    STRUCT_SPELL* destination,
    std::size_t destinationCount);

// Expands the compact 140-byte ItemList rows used by 7.48 into TMProject's
// wider in-memory item definition without allowing record boundaries to drift.
bool WYD748_LoadItemList(
    const char* path,
    STRUCT_ITEMLIST* destination,
    std::size_t destinationCount);

// Loads the variable-length 7.48 string table into TMProject's 2000-row
// buffer without treating missing newer rows as a malformed partial read.
bool WYD748_LoadMessageStrings(
    const char* path,
    char* destination,
    std::size_t destinationCount,
    std::size_t stringWidth);

// Reads the fixed 143-byte server-name table used by WYD 7.48. The source
// TMProject expects a text file, so this adapter preserves the original binary
// names and their group-order integers without rewriting sn.bin.
bool WYD748_LoadServerNameList(
    const char* path,
    char (*names)[16],
    std::size_t nameCount,
    int* groupOrder,
    std::size_t groupOrderCount);

// Loads the bracketed text configuration shipped with the 7.48 client into
// TMProject's 14-value runtime contract without changing config.txt on disk.
bool WYD748_LoadClientConfig(
    const char* path,
    short* version,
    short* values,
    std::size_t valueCount);

// Distinguishes legacy RC records with inline captions from modern records
// that contain UIString indexes. Detection is structural and never relies on
// a filename, allowing the original 7.48 UI binaries to remain untouched.
bool WYD748_IsLegacyRCFile(const char* path);

// Converts semantic control IDs from the 7.48 RC files to the IDs referenced
// by the newer scene source. IDs outside the verified mapping are unchanged.
unsigned int WYD748_TranslateControlID(unsigned int controlID);
