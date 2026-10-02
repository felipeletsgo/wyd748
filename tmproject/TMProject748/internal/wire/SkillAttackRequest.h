#pragma once

#include "AttackFrameContract.h"

// Request preparation only: no transport, entity lookup, prediction or UI.
// Packet is the existing MSG_Attack, not a new wire layout. The scene retains
// target population and the different manual/automatic send/preview ordering.
namespace skill_attack
{
template<class Packet>
Packet Base(unsigned int actorId, unsigned char skillIndex)
{
    static_assert(sizeof(Packet) == kAttackMultiBasePacketSize,
        "Skill requests retain the canonical multi-target prefix");
    Packet request{};
    request.Header.Type = MSG_Attack_Multi_Opcode;
    request.Header.ID = static_cast<decltype(request.Header.ID)>(actorId);
    request.AttackerID = static_cast<decltype(request.AttackerID)>(actorId);
    request.CurrentMp = static_cast<decltype(request.CurrentMp)>(-1);
    request.SkillIndex = skillIndex;
    request.SkillParm = 0;
    request.Motion = -1;
    return request;
}

template<class Packet>
Packet Area(unsigned int actorId, unsigned char skillIndex,
    int currentX, int currentY, int stopX, int stopY)
{
    auto request = Base<Packet>(actorId, skillIndex);
    request.PosX = static_cast<decltype(request.PosX)>(currentX);
    request.PosY = static_cast<decltype(request.PosY)>(currentY);
    request.TargetX = static_cast<decltype(request.TargetX)>(currentX);
    request.TargetY = static_cast<decltype(request.TargetY)>(currentY);
    // NextX alone is the legacy movement-stop sentinel; NextY is not a gate.
    if (stopX) {
        request.PosX = static_cast<decltype(request.PosX)>(stopX);
        request.TargetX = request.PosX;
        request.PosY = static_cast<decltype(request.PosY)>(stopY);
        request.TargetY = request.PosY;
    }
    return request;
}

template<class Packet>
Packet Direct(unsigned int actorId, unsigned char skillIndex,
    int currentX, int currentY, int stopX, int stopY, unsigned int targetId)
{
    auto request = Base<Packet>(actorId, skillIndex);
    request.PosX = static_cast<decltype(request.PosX)>(stopX);
    request.PosY = static_cast<decltype(request.PosY)>(stopY);
    request.TargetX = static_cast<decltype(request.TargetX)>(currentX);
    request.TargetY = static_cast<decltype(request.TargetY)>(currentY);
    request.Dam[0].TargetID = static_cast<decltype(request.Dam[0].TargetID)>(targetId);
    return request;
}

template<class Packet>
int SelectEnvelope(Packet& request, int maximumTargets)
{
    static_assert(sizeof(Packet) == kAttackMultiBasePacketSize,
        "Skill envelope selection requires the canonical attack buffer");
    if (maximumTargets == 1) {
        request.Header.Type = MSG_Attack_One_Opcode;
        return static_cast<int>(kAttackOneBasePacketSize);
    }
    if (maximumTargets == 2) {
        request.Header.Type = MSG_Attack_Two_Opcode;
        return static_cast<int>(kAttackTwoBasePacketSize);
    }
    // Preserve the original opcode and all target bytes for other catalog
    // values. Clearing area targets and copying local previews stay at callers.
    return static_cast<int>(sizeof(Packet));
}
}
