#pragma once

#include <cstddef>

// Cabecalho wire existente, extraido sem mudar os tipos subjacentes de
// WORD/BYTE/DWORD no alvo Windows. Nao depende dos headers Win32 ou de Basedef.
// O transporte preenche Size/KeyWord/CheckSum/Tick no armazenamento do caller.
typedef struct
{
    unsigned short Size;
    unsigned char KeyWord;
    unsigned char CheckSum;
    unsigned short Type;
    unsigned short ID;
    unsigned long Tick;
} MSG_STANDARD;

// Falhar explicitamente em plataformas onde unsigned long nao tem 32 bits:
// nao permitir que uma troca de toolchain altere o contrato silenciosamente.
static_assert(sizeof(unsigned short) == 2 && sizeof(unsigned long) == 4,
    "Wire header requires Windows integer widths");
static_assert(sizeof(MSG_STANDARD) == 12, "Wire header size changed");
static_assert(offsetof(MSG_STANDARD, Size) == 0, "Size offset changed");
static_assert(offsetof(MSG_STANDARD, KeyWord) == 2, "KeyWord offset changed");
static_assert(offsetof(MSG_STANDARD, CheckSum) == 3, "CheckSum offset changed");
static_assert(offsetof(MSG_STANDARD, Type) == 4, "Type offset changed");
static_assert(offsetof(MSG_STANDARD, ID) == 6, "ID offset changed");
static_assert(offsetof(MSG_STANDARD, Tick) == 8, "Tick offset changed");
