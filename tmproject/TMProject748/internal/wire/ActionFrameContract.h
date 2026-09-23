#pragma once

#include <cstddef>

// Movimento, parada e ilusao compartilham o mesmo envelope 7.48. Os handlers
// legados leem destino e Route diretamente, portanto o frame deve estar
// completo antes de qualquer cast para MSG_Action.
constexpr auto MSG_Action_Opcode = 0x366;
constexpr auto MSG_Action_Stop_Opcode = 0x367;
constexpr auto MSG_Action2_Opcode = 0x368;
constexpr std::size_t kActionPacketSize = 52;
constexpr std::size_t kActionPositionOffset = 12;
constexpr std::size_t kActionSpeedOffset = 16;
constexpr std::size_t kActionEffectOffset = 20;
constexpr std::size_t kActionTargetOffset = 24;
constexpr std::size_t kActionRouteOffset = 28;
constexpr std::size_t kActionRouteSize = 24;
