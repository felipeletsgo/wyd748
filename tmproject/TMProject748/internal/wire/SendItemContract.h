#pragma once

#include <cstddef>

// Envelope S->C de um slot: header(12), DestType(2), DestPos(2), Item(8).
// O payload STRUCT_ITEM permanece em Basedef; estes metadados nao duplicam
// a struct. Nativo 004B263E consome os offsets 12/14/16; Go envia 24 bytes.
constexpr auto MSG_SendItem_Opcode = 0x182;
constexpr std::size_t kSendItemPacketSize = 24;
