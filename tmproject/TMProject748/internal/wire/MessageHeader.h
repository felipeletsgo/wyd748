#pragma once

#include <cstddef>

// Existing wire header extracted without changing Windows WORD/BYTE/DWORD base types.
// It has no Win32-header or Basedef dependency. The transport fills Size/KeyWord/CheckSum/Tick in caller storage.
typedef struct
{
    unsigned short Size;
    unsigned char KeyWord;
    unsigned char CheckSum;
    unsigned short Type;
    unsigned short ID;
    unsigned long Tick;
} MSG_STANDARD;

// Fail explicitly when unsigned long is not 32 bits.
// A toolchain change must not silently alter the contract.
static_assert(sizeof(unsigned short) == 2 && sizeof(unsigned long) == 4,
    "Wire header requires Windows integer widths");
static_assert(sizeof(MSG_STANDARD) == 12, "Wire header size changed");
static_assert(offsetof(MSG_STANDARD, Size) == 0, "Size offset changed");
static_assert(offsetof(MSG_STANDARD, KeyWord) == 2, "KeyWord offset changed");
static_assert(offsetof(MSG_STANDARD, CheckSum) == 3, "CheckSum offset changed");
static_assert(offsetof(MSG_STANDARD, Type) == 4, "Type offset changed");
static_assert(offsetof(MSG_STANDARD, ID) == 6, "ID offset changed");
static_assert(offsetof(MSG_STANDARD, Tick) == 8, "Tick offset changed");
