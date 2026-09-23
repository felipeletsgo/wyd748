#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

// As cartas 4030/4031 abrem o prompt nativo e so entao enviam um
// MSG_STANDARDPARM com o canal digitado. O clique no item nao envia Parm=0.
constexpr auto MSG_UseDeclarationOfWar_Opcode = 0xED7;
constexpr auto MSG_UseRefuseServerWar_Opcode = 0xED8;

constexpr int kDeclarationOfWarLetterItemIndex = 4030;
constexpr int kWarRejectionLetterItemIndex = 4031;
constexpr int kDeclareServerWarPromptMode = 9;
constexpr int kRefuseServerWarPromptMode = 10;

constexpr std::size_t kServerWarLetterPacketSize = 16;
constexpr std::size_t kServerWarTargetChannelOffset = 12;

constexpr bool IsEncodableServerWarTargetChannel(const long long channel)
{
    return channel > 0 &&
        channel <= (std::numeric_limits<std::int32_t>::max)();
}

constexpr int ServerWarPromptModeForItem(const int itemIndex)
{
    switch (itemIndex)
    {
    case kDeclarationOfWarLetterItemIndex:
        return kDeclareServerWarPromptMode;
    case kWarRejectionLetterItemIndex:
        return kRefuseServerWarPromptMode;
    default:
        return 0;
    }
}
