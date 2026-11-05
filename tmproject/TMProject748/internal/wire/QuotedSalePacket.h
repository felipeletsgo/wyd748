#pragma once
#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>

// EXTENSAO_COORDENADA v1, C->S only. Legacy 0x37A replies remain 20 bytes.
constexpr unsigned short MSG_QuotedSale_Opcode = 0x7F40;
struct MSG_QuotedSale {
    MSG_STANDARD Header;
    unsigned short Version;
    unsigned short TargetID;
    unsigned short MyType;
    unsigned short MyPos;
    std::uint32_t ExpectedPrice;
    unsigned char ExpectedItem[8];
};
static_assert(sizeof(MSG_QuotedSale) == 32, "Quoted sale v1 size");
static_assert(offsetof(MSG_QuotedSale, Version) == 12, "Quoted sale version");
static_assert(offsetof(MSG_QuotedSale, TargetID) == 14, "Quoted sale merchant");
static_assert(offsetof(MSG_QuotedSale, MyType) == 16, "Quoted sale place");
static_assert(offsetof(MSG_QuotedSale, MyPos) == 18, "Quoted sale slot");
static_assert(offsetof(MSG_QuotedSale, ExpectedPrice) == 20, "Quoted sale price");
static_assert(offsetof(MSG_QuotedSale, ExpectedItem) == 24, "Quoted sale item");
