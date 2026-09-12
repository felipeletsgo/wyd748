#pragma once
#include "ChatMessagePacket.h"

// Extensao coordenada S->C, sem equivalente nativo 7.48 confirmado.
// Reutiliza apenas o envelope: ID=0, String[0]=0, indice int16 em offset 14,
// e CSV opcional em offset 16. O parser da cena valida conteudo/destinatario.
constexpr auto MSG_MessageIndexed_Opcode = 0x105;
constexpr auto MSG_MessageParameterized_Opcode = 0x106;
constexpr std::size_t kIndexedMessagePacketSize = sizeof(MSG_MessageChat);
static_assert(kIndexedMessagePacketSize == 108, "Indexed message envelope changed");
