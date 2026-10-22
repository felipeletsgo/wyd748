#include "../internal/wire/InstanceQuestionPacket.h"
#include "../internal/wire/ReceivedPacketDispatch.h"
#include "ReceivedPacketDispatchTestSupport.h"
#include <cstdio>
#include <cstring>

int RunInstanceQuestionTests(int& checks)
{
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL instance question: %s\n", name); }
    };
    // The native portal prompt reads one attribute per 4x4 world block.
    for (const char* path : {"tmproject/client748/Env/AttributeMap.dat",
        "wydgo748/data/maps/AttributeMap.dat"}) {
        std::ifstream map(FindFixture(path), std::ios::binary);
        map.seekg((1466 / 4) * 1024 + 1342 / 4);
        char attribute = 0;
        check(static_cast<bool>(map.read(&attribute, 1)) &&
            (static_cast<unsigned char>(attribute) & 0x14) == 0x14,
            "Big Cube return portal is enabled without losing its safe-zone bit");
    }
    instance_question::Packet p{}, parsed{};
    p.Header.Size = sizeof(p);
    p.Header.Type = instance_question::Opcode;
    p.Header.ID = 17;
    p.Version = 1;
    p.DurationMs = 10000;
    std::memcpy(p.Question, "Question?", 10);
    check(instance_question::Parse(&p, sizeof(p), parsed), "valid v1 question");
    check(received_packet::ExpectedSize(p.Header.Type) == sizeof(p), "receive gate knows exact size");
    check(received_packet::CanDispatch({p.Header.Type, reinterpret_cast<char*>(&p), sizeof(p)}), "valid receive dispatch");
    for (std::size_t size = 0; size < sizeof(p); ++size) {
        check(!instance_question::Parse(&p, size, parsed), "reject every truncated prefix");
        check(!received_packet::CanDispatch({p.Header.Type, reinterpret_cast<char*>(&p), size}), "reject truncated receive frame");
    }
    auto invalid = p;
    invalid.Version = 2;
    check(!instance_question::Parse(&invalid, sizeof(invalid), parsed), "reject unknown version");
    invalid = p; invalid.Reserved = 1;
    check(!instance_question::Parse(&invalid, sizeof(invalid), parsed), "reject reserved bits");
    invalid = p; invalid.DurationMs = 10001;
    check(!instance_question::Parse(&invalid, sizeof(invalid), parsed), "reject excessive lifetime");
    invalid = p; std::memset(invalid.Question, 'A', sizeof(invalid.Question));
    check(!instance_question::Parse(&invalid, sizeof(invalid), parsed), "reject unterminated text");
    invalid = p; invalid.Question[0] = 0;
    check(!instance_question::Parse(&invalid, sizeof(invalid), parsed), "reject empty active question");
    check(!instance_question::Parse(nullptr, sizeof(p), parsed), "reject null input");
    check(!instance_question::Parse(&p, sizeof(p) + 1, parsed), "reject trailing bytes");

    instance_question::State state{};
    check(!state.Visible(100), "fresh display state is inactive");
    state.Apply(p, 100);
    check(state.Visible(100) && state.Visible(10099), "visible throughout the full answer window");
    check(!state.Visible(10100), "expires at exactly ten seconds");
    state.Apply(p, 0xFFFFFF00u);
    check(state.Visible(0xFFFFFF00u + 9999u) && !state.Visible(0xFFFFFF00u + 10000u), "clock wrap is safe");
    p.DurationMs = 0; p.Question[0] = 0;
    check(instance_question::Parse(&p, sizeof(p), parsed), "valid close packet");
    state.Apply(p, 10);
    check(!state.Visible(10), "server close hides immediately");
    p.DurationMs = 10000;
    state.Apply(p, 10); state.Clear();
    check(!state.Visible(11), "instance exit clears question");
    return failures;
}
