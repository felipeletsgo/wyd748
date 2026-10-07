#pragma once

// Capacities of the runtime asset tables migrated from WYD KR. Loaders reject
// a table that exceeds them, so RuntimeAssetLimitsTests checks the shipped
// client748 files against these values.

// Mesh/MeshList.txt: KR indices reach 3097.
constexpr auto MAX_COMMON_MESH = 3200;

// object.bin: KR stores 4096 encrypted 16x16 masks plus a 4-byte checksum.
constexpr auto MAX_OBJECT_MASK = 4096;
constexpr auto MAX_MASK_X = 16;
constexpr auto MAX_MASK_Y = 16;

// Mesh/MeshTextureList.bin and Env/EnvTextureList3.bin (528-byte records).
constexpr auto MAX_MODEL_TEXTURE = 3048;
constexpr auto MAX_ENV_TEXTURE = 2048;
