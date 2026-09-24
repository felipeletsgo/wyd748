#pragma once

#include <cstddef>

// Movement, stop, and illusion share the 7.48 envelope. Legacy handlers read
// the destination and Route directly, so the frame must be complete before
// it is cast to MSG_Action.
constexpr auto MSG_Action_Opcode = 0x366;
constexpr auto MSG_Action_Stop_Opcode = 0x367;
constexpr auto MSG_Action2_Opcode = 0x368;
// Paired server/client extension: route correction has no teleport semantics.
constexpr unsigned int kActionRouteCorrectionEffect = 8;
constexpr bool IsRouteCorrectionAction(unsigned int opcode, unsigned int effect)
{
    return opcode == MSG_Action_Opcode && effect == kActionRouteCorrectionEffect;
}
constexpr std::size_t kActionPacketSize = 52;
constexpr std::size_t kActionPositionOffset = 12;
constexpr std::size_t kActionSpeedOffset = 16;
constexpr std::size_t kActionEffectOffset = 20;
constexpr std::size_t kActionTargetOffset = 24;
constexpr std::size_t kActionRouteOffset = 28;
constexpr std::size_t kActionRouteSize = 24;
