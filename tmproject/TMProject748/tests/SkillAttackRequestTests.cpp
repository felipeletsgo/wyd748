#include "../internal/core/WYD748Assets.h"
#include "../internal/core/Basedef.h"
#include "../internal/wire/SkillAttackRequest.h"
#include "../internal/application/SkillRequestPolicy.h"

#include <cstdio>
#include <cstring>
#include <limits>

namespace
{
// Independent oracle for the four original scene request-initialization blocks.
// Use the real native packet definition, not a test-only layout or encoder.
MSG_Attack LegacyRequest(bool area, unsigned int actor, unsigned char skill,
    int x, int y, int nextX, int nextY, unsigned int target)
{
    MSG_Attack packet{};
    packet.Header.Type = MSG_Attack_Multi_Opcode;
    packet.Header.ID = static_cast<decltype(packet.Header.ID)>(actor);
    packet.AttackerID = static_cast<unsigned short>(actor);
    if (area) {
        packet.PosX = static_cast<unsigned short>(x);
        packet.PosY = static_cast<unsigned short>(y);
        packet.TargetX = static_cast<unsigned short>(x);
        packet.TargetY = static_cast<unsigned short>(y);
        if (nextX) {
            packet.PosX = static_cast<unsigned short>(nextX);
            packet.TargetX = packet.PosX;
            packet.PosY = static_cast<unsigned short>(nextY);
            packet.TargetY = packet.PosY;
        }
    } else {
        packet.PosX = static_cast<unsigned short>(nextX);
        packet.PosY = static_cast<unsigned short>(nextY);
        packet.TargetX = static_cast<unsigned short>(x);
        packet.TargetY = static_cast<unsigned short>(y);
        packet.FlagLocal = 0;
        packet.Dam[0].TargetID = static_cast<decltype(packet.Dam[0].TargetID)>(target);
    }
    packet.CurrentMp = static_cast<unsigned short>(-1);
    packet.SkillIndex = skill;
    packet.SkillParm = 0;
    packet.Motion = -1;
    return packet;
}
}

int RunSkillAttackRequestTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool passed, const char* message) {
        ++checks;
        if (!passed) {
            ++failures;
            std::fprintf(stderr, "FAIL skill attack request: %s\n", message);
        }
    };
    const unsigned int ids[]{0, 1, 999, 1000, 65535, 65536,
        (std::numeric_limits<unsigned int>::max)()};
    const int coordinates[][4]{
        {0, 0, 0, 0}, {123, 456, 0, 789}, {123, 456, 789, 0},
        {2362, 3927, 2369, 3934}, {-1, -2, -3, -4},
        {65535, 65536, 65537, 65538},
        {(std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)(), 0, -1},
        {23, 45, 65536, 65537}};
    for (const auto actor : ids)
    for (unsigned int skill = 0; skill <= 255; ++skill)
    for (const auto& coordinate : coordinates) {
        const auto index = static_cast<unsigned char>(skill);
        const auto target = ids[(skill + 3) % 7];
        const auto area = skill_attack::Area<MSG_Attack>(actor, index,
            coordinate[0], coordinate[1], coordinate[2], coordinate[3]);
        const auto expectedArea = LegacyRequest(true, actor, index,
            coordinate[0], coordinate[1], coordinate[2], coordinate[3], target);
        check(std::memcmp(&area, &expectedArea, sizeof(area)) == 0,
            "all 96 area bytes match the original, including stop sentinel and narrowing");
        const auto direct = skill_attack::Direct<MSG_Attack>(actor, index,
            coordinate[0], coordinate[1], coordinate[2], coordinate[3], target);
        const auto expectedDirect = LegacyRequest(false, actor, index,
            coordinate[0], coordinate[1], coordinate[2], coordinate[3], target);
        check(std::memcmp(&direct, &expectedDirect, sizeof(direct)) == 0,
            "all 96 direct bytes match the original, including target and reserved bytes");
    }
    for (int maximum : {-1, 0, 1, 2, 3, 7, 13, 14, 255})
    for (unsigned short opcode : {static_cast<unsigned short>(MSG_Attack_Multi_Opcode),
        static_cast<unsigned short>(MSG_Attack_One_Opcode), static_cast<unsigned short>(0x123)}) {
        MSG_Attack packet;
        std::memset(&packet, 0xAB, sizeof(packet));
        packet.Header.Type = opcode;
        auto expected = packet;
        int expectedSize = sizeof(MSG_Attack);
        if (maximum == 1) {
            expected.Header.Type = MSG_Attack_One_Opcode;
            expectedSize = sizeof(MSG_AttackOne);
        }
        if (maximum == 2) {
            expected.Header.Type = MSG_Attack_Two_Opcode;
            expectedSize = sizeof(MSG_AttackTwo);
        }
        check(skill_attack::SelectEnvelope(packet, maximum) == expectedSize,
            "envelope size matches the original catalog routing");
        check(std::memcmp(&packet, &expected, sizeof(packet)) == 0,
            "selection changes only the opcode, never targets, size or prediction flags");
        if (opcode == MSG_Attack_Multi_Opcode)
            check(IsClientToServerAttackPacketSize(packet.Header.Type, expectedSize),
                "normal skill envelopes remain accepted client-to-server prefixes");
    }
    for (int targetType = -256; targetType <= 256; ++targetType) {
        const bool expected = targetType == 0 || targetType == 3 ||
            targetType == 4 || targetType == 5 || targetType == 6;
        check(skill_request::UsesAreaRequest(targetType) == expected,
            "manual and automatic area routing retain all catalog cases and fallthroughs");
        const int expectedRadius = targetType == 3 ? 1 : targetType == 4 ? 2 :
            targetType == 6 ? 3 : -1;
        check(skill_request::AreaRadius(targetType) == expectedRadius,
            "area radius preserves each catalog case and the negative default");
    }
    for (int skill = -256; skill <= 256; ++skill) {
        const bool primaryAim = skill == 0 || skill == 7 || skill == 16 ||
            skill == 17 || skill == 23 || skill == 35 || skill == 39 ||
            skill == 51 || skill == 55;
        // Independent lists preserve the asymmetric manual/automatic behavior.
        const bool manualRange = skill == 0 || skill == 7 || skill == 16 ||
            skill == 17 || skill == 23 || skill == 35 || skill == 39 ||
            skill == 51 || skill == 55 || skill == 79 || skill == 97;
        const bool automaticRange = skill == 0 || skill == 7 || skill == 16 ||
            skill == 17 || skill == 23 || skill == 35 || skill == 39 ||
            skill == 51 || skill == 55 || skill == 95;
        check(skill_request::AimsAtPrimaryTarget(skill) == primaryAim,
            "primary aim retains the original nine skill indices");
        check(skill_request::ChecksPrimaryTargetRange(skill,
            skill_request::Invocation::Manual) == manualRange,
            "manual range retains skills 79 and 97 but not automatic-only 95");
        check(skill_request::ChecksPrimaryTargetRange(skill,
            skill_request::Invocation::Automatic) == automaticRange,
            "automatic range retains skill 95 but not manual-only 79 and 97");
    }
    const int positions[][4]{{0, 0, 0, 0}, {1, 0, 0, 0}, {0, 1, 0, 0},
        {1, 1, 0, 0}, {-1, -2, -1, -2},
        {(std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)(),
         (std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)()}};
    for (int distance : {-1, 0, 1, 2, 3, 4, (std::numeric_limits<int>::max)()})
    for (int radius : {-1, 0, 1, 2, 3, (std::numeric_limits<int>::max)()})
    for (const auto& position : positions) {
        const bool expected = distance <= radius && position[0] == position[2] &&
            position[1] == position[3];
        check(skill_request::ReachesAreaTarget(distance, radius, position[0],
            position[1], position[2], position[3]) == expected,
            "area geometry preserves inclusive radius and both hit coordinates");
    }
    for (int type : {-1, 0, 3, 4, 6, 256})
    for (int count : {-1, 0, 1, 2, 7, 8, 12, 13, 14,
        (std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)()})
    for (int maximum : {-1, 0, 1, 2, 7, 8, 12, 13, 14,
        (std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)()}) {
        const bool expected = (type == 3 && count > 7) || maximum <= count || count >= 13;
        check(skill_request::AreaTargetLimitReached(type, count, maximum) == expected,
            "area target cap preserves type-3 count eight, catalog cap and wire cap thirteen");
    }
    return failures;
}
