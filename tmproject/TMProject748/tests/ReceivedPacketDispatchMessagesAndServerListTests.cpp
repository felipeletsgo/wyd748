#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: MessagesAndServerList. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchMessagesAndServerListTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    std::array<char, 109> messagePanel{};
    messagePanel[0] = 108;
    messagePanel[4] = 0x01;
    messagePanel[5] = 0x01;
    std::memcpy(messagePanel.data() + 12, "Inventario limpo", 17);
    int panelCalls = 0;
    const auto receivePanel = [&](const PacketView& view) {
        ++panelCalls;
        check(view.data == messagePanel.data() && view.size == 108 && view.opcode == 0x101,
            "MessagePanel preserves buffer, length and opcode");
    };
    for (std::size_t n = 0; n < 108; ++n)
        check(!received_packet::Dispatch({0x101, messagePanel.data(), n}, receivePanel),
            "MessagePanel rejects every truncated prefix");
    check(!received_packet::Dispatch({0x101, messagePanel.data(), 109}, receivePanel),
        "oversized MessagePanel rejected");
    check(!received_packet::Dispatch({0x101, nullptr, 108}, receivePanel),
        "null MessagePanel rejected");
    check(!received_packet::Dispatch({0x119, messagePanel.data(), 108}, receivePanel),
        "MessagePanel Type cannot be hidden");
    messagePanel[4] = 0x19;
    check(!received_packet::Dispatch({0x101, messagePanel.data(), 108}, receivePanel),
        "mismatched MessagePanel Type rejected");
    messagePanel[4] = 0x01;
    messagePanel[0] = 107;
    check(!received_packet::Dispatch({0x101, messagePanel.data(), 108}, receivePanel),
        "mismatched MessagePanel Size rejected");
    messagePanel[0] = 108;
    check(panelCalls == 0, "invalid MessagePanel does not reach the consumer");
    const auto panelBefore = messagePanel;
    check(received_packet::Dispatch({0x101, messagePanel.data(), 108}, receivePanel) && panelCalls == 1,
        "valid MessagePanel delivered once");
    MSG_MessagePanel decodedPanel{};
    std::memcpy(&decodedPanel, messagePanel.data(), sizeof(decodedPanel));
    check(decodedPanel.Header.ID == 0 && std::strcmp(decodedPanel.String, "Inventario limpo") == 0 &&
        decodedPanel.String[95] == 0,
        "MessagePanel preserves zero ID, text and final terminator");
    check(messagePanel == panelBefore, "gate preserves every MessagePanel byte");
    // Fixtures independent of the structs: validate the actual and declared length
    // and both discriminators before any scene callback.
    for (unsigned int opcode : {0x102u, 0x104u})
    {
        const std::size_t length = opcode == 0x102 ? 116 : 152;
        std::array<char, 154> raw{};
        char* frame = raw.data() + 1;
        frame[0] = static_cast<char>(length);
        frame[4] = static_cast<char>(opcode & 255);
        frame[5] = 1;
        const auto snapshot = raw;
        int calls = 0;
        const auto receiver = [&](const PacketView& view) {
            ++calls;
            check(view.data == frame && view.size == length,
                "opaque message preserves buffer and length");
        };
        for (std::size_t n = 0; n < length; ++n)
            check(!received_packet::Dispatch({opcode, frame, n}, receiver),
                "opaque message rejects every truncated prefix");
        check(!received_packet::Dispatch({opcode, frame, length + 1}, receiver), "excess rejected");
        check(!received_packet::Dispatch({opcode, nullptr, length}, receiver), "null frame rejected");
        check(!received_packet::Dispatch({0x119, frame, length}, receiver), "Type cannot be hidden");
        frame[0] = 12;
        check(!received_packet::Dispatch({opcode, frame, length}, receiver), "mismatched Size rejected");
        frame[0] = static_cast<char>(length);
        frame[4] = 0x19;
        check(!received_packet::Dispatch({opcode, frame, length}, receiver), "mismatched opcode rejected");
        frame[4] = static_cast<char>(opcode & 255);
        check(calls == 0, "opaque rejections do not run the callback");
        check(received_packet::Dispatch({opcode, frame, length}, receiver) && calls == 1,
            "opaque message delivered once");
        check(raw == snapshot, "opaque bytes preserved");
    }
    // Go MessageChat frame: 108 bytes, sender ID and text at offset 12.
    std::array<char, 109> chat{};
    chat[0] = 108;
    chat[4] = 0x33;
    chat[5] = 3;
    chat[6] = 0x34;
    chat[7] = 0x12;
    std::memcpy(chat.data() + 12, "Hello", 6);
    MSG_MessageChat decodedChat{};
    std::memcpy(&decodedChat, chat.data(), 108);
    check(decodedChat.Header.ID == 0x1234 && std::strcmp(decodedChat.String, "Hello") == 0,
        "chat fixture confirms the ID and the text offset");
    int chatCalls = 0;
    const auto chatReceiver = [&](const PacketView& view) {
        ++chatCalls;
        check(view.data == chat.data() && view.size == 108, "chat preserves the borrowed buffer");
    };
    for (std::size_t size = 0; size < 108; ++size)
        check(!received_packet::Dispatch({0x333, chat.data(), size}, chatReceiver), "truncated chat rejected");
    check(!received_packet::Dispatch({0x333, chat.data(), 109}, chatReceiver), "oversized chat rejected");
    check(!received_packet::Dispatch({0x333, nullptr, 108}, chatReceiver), "null chat rejected");
    check(!received_packet::Dispatch({0x119, chat.data(), 108}, chatReceiver), "chat Type cannot be hidden");
    chat[4] = 0x19;
    check(!received_packet::Dispatch({0x333, chat.data(), 108}, chatReceiver), "mismatched chat opcode rejected");
    chat[4] = 0x33;
    chat[0] = 107;
    check(!received_packet::Dispatch({0x333, chat.data(), 108}, chatReceiver), "mismatched chat Size rejected");
    chat[0] = 108;
    check(chatCalls == 0, "invalid chat does not reach the consumer");
    const auto chatSnapshot = chat;
    check(received_packet::Dispatch({0x333, chat.data(), 108}, chatReceiver) && chatCalls == 1,
        "valid chat delivered once");
    check(chat == chatSnapshot, "gate does not modify the text or header");
    for (unsigned int opcode : {0x105u, 0x106u})
    {
        std::array<char, 109> frame{};
        frame[0] = 108;
        frame[4] = static_cast<char>(opcode & 255);
        frame[5] = 1;
        // -938 little-endian, the same index as the party invitation in Go.
        frame[14] = 0x56;
        frame[15] = static_cast<char>(0xFC);
        if (opcode == 0x106) std::memcpy(frame.data() + 16, "Lider", 6);
        int calls = 0;
        const auto receiveExtension = [&](const PacketView& view) {
            ++calls;
            check(view.data == frame.data() && view.size == 108 && view.opcode == opcode,
                "extension preserves the frame for the existing parser");
        };
        for (std::size_t n = 0; n < 108; ++n)
            check(!received_packet::Dispatch({opcode, frame.data(), n}, receiveExtension),
                "extension rejects every prefix");
        check(!received_packet::Dispatch({opcode, nullptr, 108}, receiveExtension), "null extension");
        check(!received_packet::Dispatch({opcode, frame.data(), 109}, receiveExtension), "oversized extension");
        check(!received_packet::Dispatch({0x119, frame.data(), 108}, receiveExtension), "extension Type hidden");
        frame[4] = 0x19;
        check(!received_packet::Dispatch({opcode, frame.data(), 108}, receiveExtension), "mismatched extension Type");
        frame[4] = static_cast<char>(opcode & 255);
        frame[0] = 107;
        check(!received_packet::Dispatch({opcode, frame.data(), 108}, receiveExtension), "mismatched extension Size");
        frame[0] = 108;
        check(calls == 0, "invalid extension without callback");
        const auto before = frame;
        check(received_packet::Dispatch({opcode, frame.data(), 108}, receiveExtension) && calls == 1,
            "valid extension delivered once");
        check(before == frame, "signed index and CSV preserved");
    }
    std::array<char, 81> migration{};
    migration[0] = 80;
    migration[4] = 0x2A;
    migration[5] = 5;
    migration[6] = 0x34;
    migration[7] = 0x12;
    // Synthetic data: the account fills 16 bytes and the ticket fills 52, without NUL.
    std::memset(migration.data() + 12, 'A', 16);
    std::memset(migration.data() + 28, 'T', 52);
    int migrationCalls = 0;
    MSG_CNFRemoveServer retained{};
    const auto retainMigration = [&](const PacketView& frame) {
        ++migrationCalls;
        check(frame.data == migration.data() && frame.size == 80,
            "migration borrows the complete image");
        std::memcpy(&retained, frame.data, sizeof(retained));
    };
    for (std::size_t n = 0; n < 80; ++n)
        check(!received_packet::Dispatch({0x52A, migration.data(), n}, retainMigration),
            "truncated migration cannot feed a replay");
    check(!received_packet::Dispatch({0x52A, migration.data(), 81}, retainMigration), "oversized migration");
    check(!received_packet::Dispatch({0x52A, nullptr, 80}, retainMigration), "null migration");
    check(!received_packet::Dispatch({0x119, migration.data(), 80}, retainMigration), "migration Type hidden");
    migration[4] = 0x19;
    check(!received_packet::Dispatch({0x52A, migration.data(), 80}, retainMigration), "mismatched migration Type");
    migration[4] = 0x2A;
    migration[0] = 79;
    check(!received_packet::Dispatch({0x52A, migration.data(), 80}, retainMigration), "mismatched migration Size");
    migration[0] = 80;
    check(migrationCalls == 0 && retained.Header.Size == 0, "rejection preserves the receiver state");
    check(received_packet::Dispatch({0x52A, migration.data(), 80}, retainMigration) && migrationCalls == 1,
        "valid migration delivered once");
    check(retained.Header.ID == 0x1234 && retained.AccountName[15] == 'A' && retained.TID[51] == 'T',
        "migration preserves the ID and the bounds of the inline fields");
    check(std::memcmp(&retained, migration.data(), 80) == 0, "migration image preserved byte for byte");
    const auto parseTicket = [&](const char* text, bool valid, int expected) {
        char ticket[52]{};
        const auto length = std::strlen(text);
        std::memcpy(ticket, text, length < sizeof(ticket) ? length : sizeof(ticket));
        int result = 99;
        check(ParseMigrationServer(ticket, 7, result) == valid, "ticket validates prefix and capacity");
        check(result == (valid ? expected : 99), "invalid ticket preserves the output");
    };
    parseTicket("*0", true, 0);
    parseTicket("*6:ticket", true, 6);
    parseTicket("* +2resto", true, 2);
    parseTicket("*7", false, 0);
    parseTicket("*-1", false, 0);
    parseTicket("*", false, 0);
    parseTicket("2", false, 0);
    parseTicket("*999999999999999999999999999999", false, 0);
    char fullTicket[52];
    std::memset(fullTicket, 'x', sizeof(fullTicket));
    fullTicket[0] = '*'; fullTicket[1] = '3';
    int parsed = 99;
    check(ParseMigrationServer(fullTicket, 7, parsed) && parsed == 3,
        "ticket without NUL respects the physical limit and accepts an opaque suffix");
    check(!ParseMigrationServer(fullTicket, 0, parsed), "empty capacity rejected");
    char endpointSource[64]{};
    char endpointDestination[128]{};
    std::memcpy(endpointSource, "127.0.0.1", 10);
    check(CopyServerEndpoint(endpointDestination, endpointSource) &&
        std::strcmp(endpointDestination, "127.0.0.1") == 0,
        "terminated endpoint copied without exceeding the cell");
    std::memset(endpointSource, 'x', sizeof(endpointSource));
    std::memcpy(endpointDestination, "stale-data", 10);
    check(!CopyServerEndpoint(endpointDestination, endpointSource) &&
        endpointDestination[0] == '\0',
        "endpoint without NUL rejected and previous destination cleared");
    std::memcpy(endpointDestination, "stale-data", 10);
    std::memset(endpointSource, 0, sizeof(endpointSource));
    check(!CopyServerEndpoint(endpointDestination, endpointSource) &&
        endpointDestination[0] == '\0',
        "empty endpoint rejected and previous destination cleared");
    std::memset(endpointSource, 'm', sizeof(endpointSource));
    endpointSource[sizeof(endpointSource) - 1] = '\0';
    check(CopyServerEndpoint(endpointDestination, endpointSource) &&
        endpointDestination[sizeof(endpointSource) - 2] == 'm' &&
        endpointDestination[sizeof(endpointSource) - 1] == '\0',
        "63-byte endpoint accepted at the physical limit");
    char serverGroups[10][11][64]{};
    check(LastConfiguredServerGroup(serverGroups) == -1,
        "empty server list does not access a missing group");
    serverGroups[0][0][0] = 'a';
    serverGroups[1][0][0] = 'b';
    check(LastConfiguredServerGroup(serverGroups) == 1,
        "lista parcial termina no primeiro grupo vazio");
    for (auto& group : serverGroups)
        group[0][0] = 'a';
    check(LastConfiguredServerGroup(serverGroups) == 9,
        "ten occupied groups end at the table's physical limit");
    int orderedGroups[11]{};
    for (int group = 0; group < 10; ++group)
        orderedGroups[group] = group + 1;
    auto visibleGroups = VisibleServerGroupSlots(orderedGroups, serverGroups, 9);
    check(visibleGroups.count == 10 && visibleGroups.slots[0] == 9 && visibleGroups.slots[9] == 0,
        "dez grupos visiveis preservam o ultimo slot e a ordem inversa");
    serverGroups[9][0][0] = 0;
    check(visibleGroups.count == 10 && visibleGroups.slots[0] == 9,
        "row snapshot does not change when the aggregate clears status endpoints");
    serverGroups[9][0][0] = 'a';
    orderedGroups[4] = 0;
    orderedGroups[7] = 2;
    serverGroups[1][0][0] = 0;
    visibleGroups = VisibleServerGroupSlots(orderedGroups, serverGroups, 9);
    check(visibleGroups.count == 7 && visibleGroups.slots[0] == 9 &&
        visibleGroups.slots[2] == 6 && visibleGroups.slots[6] == 0,
        "sparse rows match the slots actually inserted in the list");
    check(VisibleServerGroupSlots(orderedGroups, serverGroups, -1).count == 0,
        "list without groups creates no selectable rows");
    const int sparseChannels[10]{1, 3, 10};
    check(ChannelForVisibleRow(sparseChannels, 3, 0) == 1 &&
        ChannelForVisibleRow(sparseChannels, 3, 1) == 3 &&
        ChannelForVisibleRow(sparseChannels, 3, 2) == 10,
        "compacted row keeps the channel's real endpoint");
    check(ChannelForVisibleRow(sparseChannels, 0, 0) == -1 &&
        ChannelForVisibleRow(sparseChannels, 3, -1) == -1 &&
        ChannelForVisibleRow(sparseChannels, 3, 3) == -1 &&
        ChannelForVisibleRow(sparseChannels, 11, 0) == -1,
        "stale or out-of-list selection does not resolve an endpoint");
    char channelNames[10][10][9]{};
    std::memcpy(channelNames[0][9], "Canal 10", 9);
    check(ServerChannelNameAt(channelNames, 0, 9) == channelNames[0][9] &&
        !ServerChannelNameAt(channelNames, 0, 10) &&
        !ServerChannelNameAt(channelNames, 10, 0) &&
        !ServerChannelNameAt(channelNames, 0, -1),
        "channel name respects the ten cells per group");
    std::memset(channelNames[0][9], 'x', sizeof(channelNames[0][9]));
    check(!ServerChannelNameAt(channelNames, 0, 9),
        "name without terminator does not spill into the next cell");
    std::memcpy(serverGroups[0][10], "127.0.0.1", 10);
    char selectedEndpoint[64]{};
    check(CopyServerEndpointAt(selectedEndpoint, serverGroups, 0, 10) &&
        std::strcmp(selectedEndpoint, "127.0.0.1") == 0,
        "last endpoint channel remains valid");
    check(!CopyServerEndpointAt(selectedEndpoint, serverGroups, 0, 11) &&
        selectedEndpoint[0] == 0 &&
        !CopyServerEndpointAt(selectedEndpoint, serverGroups, -1, 1) &&
        !CopyServerEndpointAt(selectedEndpoint, serverGroups, 10, 1),
        "out-of-bounds aggregate endpoint is discarded");
    return failures;
}
