#include "ReceivedPacketDispatchTestSupport.h"

// ReceivedPacketDispatch tests: GroundItems. Split from ReceivedPacketDispatchTests.cpp.
int RunReceivedPacketDispatchGroundItemsTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };

    auto sendItem = LoadHexFixture("testdata/protocol/send_item_0x182_24.hex");
    check(sendItem.size() == 24, "fixture SendItem compartilhado possui 24 bytes");
    if (sendItem.size() != 24)
        return failures;
    sendItem.push_back(0); // sentinel to test an oversized frame without reading outside the buffer.
    int itemDelivered = 0;
    const auto receiveItem = [&](const PacketView& frame) {
        ++itemDelivered;
        check(frame.data == sendItem.data() && frame.size == 24 && frame.opcode == 0x182,
            "valid SendItem keeps the frame and is delivered once");
    };
    for (std::size_t size = 0; size < 24; ++size)
        check(!received_packet::Dispatch({0x182, sendItem.data(), size}, receiveItem),
            "truncated SendItem rejected before the copy");
    check(!received_packet::Dispatch({0x182, sendItem.data(), 25}, receiveItem),
        "oversized SendItem rejected");
    check(!received_packet::Dispatch({0xFAA, sendItem.data(), 24}, receiveItem),
        "known contracts cannot swap metadata");
    check(!received_packet::Dispatch({0x119, sendItem.data(), 24}, receiveItem),
        "unknown metadata does not bypass the SendItem Type");
    sendItem[0] = 23;
    check(!received_packet::Dispatch({0x182, sendItem.data(), 24}, receiveItem),
        "SendItem rejects a mismatched declared Size");
    sendItem[0] = 24;
    check(itemDelivered == 0, "invalid SendItem does not call the consumer");
    check(received_packet::Dispatch({0x182, sendItem.data(), 24}, receiveItem) && itemDelivered == 1,
        "exact SendItem delivered without retry");
    // Go builder frame: Carry[62] destination and an eight-byte STRUCT_ITEM.
    std::array<char, kPickupConfirmationPacketSize + 1> pickup{};
    pickup[0] = static_cast<char>(kPickupConfirmationPacketSize);
    pickup[4] = 0x71;
    pickup[5] = 0x01;
    pickup[kPickupConfirmationDestTypeOffset] = 1;
    pickup[kPickupConfirmationDestPosOffset] = 62;
    pickup[kPickupConfirmationItemOffset] = 0x34;
    pickup[kPickupConfirmationItemOffset + 1] = 0x12;
    for (std::size_t i = 2; i < 8; ++i)
        pickup[kPickupConfirmationItemOffset + i] = static_cast<char>(i);
    const auto pickupBefore = pickup;
    int pickupCalls = 0;
    const auto receivePickup = [&](const PacketView& view) {
        ++pickupCalls;
        check(view.data == pickup.data() && view.size == kPickupConfirmationPacketSize &&
            view.opcode == MSG_CNFGetItem_Opcode,
            "CNFGetItem preserves the 28-byte frame");
    };
    for (std::size_t n = 0; n < kPickupConfirmationPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(), n}, receivePickup),
            "truncated CNFGetItem rejected before the handler");
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, nullptr,
        kPickupConfirmationPacketSize}, receivePickup), "null CNFGetItem rejected");
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize + 1}, receivePickup), "oversized CNFGetItem rejected");
    check(!received_packet::Dispatch({0x119, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup), "CNFGetItem Type cannot be hidden");
    pickup[0] = static_cast<char>(kPickupConfirmationPacketSize - 1);
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup), "mismatched CNFGetItem Size rejected");
    pickup[0] = static_cast<char>(kPickupConfirmationPacketSize);
    pickup[4] = 0x19;
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup), "mismatched CNFGetItem opcode rejected");
    pickup[4] = 0x71;
    check(pickupCalls == 0, "invalid CNFGetItem does not reach the consumer");
    check(received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup) && pickupCalls == 1,
        "valid CNFGetItem delivered once");
    check(pickup[kPickupConfirmationDestTypeOffset] == 1 &&
        pickup[kPickupConfirmationDestPosOffset] == 62 &&
        static_cast<unsigned char>(pickup[kPickupConfirmationItemOffset]) == 0x34 &&
        static_cast<unsigned char>(pickup[kPickupConfirmationItemOffset + 1]) == 0x12,
        "CNFGetItem preserves destination and item at the native offsets");
    check(pickup == pickupBefore, "gate preserves every CNFGetItem byte");
    check(!IsPickupCarrySlot(-1), "CNFGetItem rejects a negative slot");
    for (int slot = 0; slot < kPickupVisibleCarrySlotCount; ++slot)
        check(IsPickupCarrySlot(slot), "CNFGetItem accepts every visible slot of the 9x7 Carry");
    check(!IsPickupCarrySlot(kPickupVisibleCarrySlotCount),
        "CNFGetItem rejects structural slot 63 without a visual cell");
    // Same frame produced by wire.CNFDropItem(1, 62, 3, 2200, 2100).
    std::array<char, kDropConfirmationPacketSize + 1> drop{};
    drop[0] = static_cast<char>(kDropConfirmationPacketSize);
    drop[4] = 0x75;
    drop[5] = 0x01;
    drop[kDropConfirmationSourceTypeOffset] = 1;
    drop[kDropConfirmationSourcePosOffset] = 62;
    drop[kDropConfirmationRotateOffset] = 3;
    drop[kDropConfirmationGridXOffset] = static_cast<char>(0x98);
    drop[kDropConfirmationGridXOffset + 1] = 0x08;
    drop[kDropConfirmationGridYOffset] = 0x34;
    drop[kDropConfirmationGridYOffset + 1] = 0x08;
    const auto dropBefore = drop;
    int dropCalls = 0;
    const auto receiveDrop = [&](const PacketView& view) {
        ++dropCalls;
        check(view.data == drop.data() && view.size == kDropConfirmationPacketSize &&
            view.opcode == MSG_CNFDropItem_Opcode,
            "CNFDropItem preserves the 28-byte frame");
    };
    for (std::size_t n = 0; n < kDropConfirmationPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(), n}, receiveDrop),
            "truncated CNFDropItem rejected before the handler");
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, nullptr,
        kDropConfirmationPacketSize}, receiveDrop), "null CNFDropItem rejected");
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize + 1}, receiveDrop), "oversized CNFDropItem rejected");
    check(!received_packet::Dispatch({0x119, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop), "CNFDropItem Type cannot be hidden");
    drop[0] = static_cast<char>(kDropConfirmationPacketSize - 1);
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop), "mismatched CNFDropItem Size rejected");
    drop[0] = static_cast<char>(kDropConfirmationPacketSize);
    drop[4] = 0x19;
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop), "mismatched CNFDropItem opcode rejected");
    drop[4] = 0x75;
    check(dropCalls == 0, "invalid CNFDropItem does not reach the consumer");
    check(received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop) && dropCalls == 1,
        "valid CNFDropItem delivered once");
    check(static_cast<unsigned char>(drop[kDropConfirmationGridXOffset]) == 0x98 &&
        static_cast<unsigned char>(drop[kDropConfirmationGridXOffset + 1]) == 0x08 &&
        static_cast<unsigned char>(drop[kDropConfirmationGridYOffset]) == 0x34 &&
        static_cast<unsigned char>(drop[kDropConfirmationGridYOffset + 1]) == 0x08,
        "CNFDropItem preserves rotation and coordinates at the native offsets");
    check(drop == dropBefore, "gate preserves every CNFDropItem byte");
    check(!IsDropCarrySlot(-1), "CNFDropItem rejects a negative Carry slot");
    for (int slot = 0; slot < kDropVisibleCarrySlotCount; ++slot)
        check(IsDropCarrySlot(slot), "CNFDropItem accepts every visible Carry slot");
    check(!IsDropCarrySlot(kDropVisibleCarrySlotCount),
        "CNFDropItem rejects structural Carry slot 63");
    check(!IsDropCargoSlot(-1), "CNFDropItem rejects a negative Cargo slot");
    for (int slot = 0; slot < kDropUsableCargoSlotCount; ++slot)
        check(IsDropCargoSlot(slot), "CNFDropItem accepts every usable Cargo slot");
    check(!IsDropCargoSlot(kDropUsableCargoSlotCount),
        "CNFDropItem rejects the first reserved Cargo slot");
    // Same frame produced by wire.CreateItem for an item appearing on the ground.
    std::array<char, kGroundItemCreatePacketSize + 1> createItem{};
    createItem[0] = static_cast<char>(kGroundItemCreatePacketSize);
    createItem[4] = 0x6E;
    createItem[5] = 0x02;
    createItem[kGroundItemCreateGridXOffset] = static_cast<char>(0x98);
    createItem[kGroundItemCreateGridXOffset + 1] = 0x08;
    createItem[kGroundItemCreateGridYOffset] = 0x34;
    createItem[kGroundItemCreateGridYOffset + 1] = 0x08;
    createItem[kGroundItemCreateItemIDOffset] = 0x10;
    createItem[kGroundItemCreateItemIDOffset + 1] = 0x27;
    createItem[kGroundItemCreateItemOffset] = static_cast<char>(0xAB);
    createItem[kGroundItemCreateItemOffset + 1] = 0x0F;
    createItem[kGroundItemCreateRotateOffset] = 1;
    createItem[kGroundItemCreateStateOffset] = 2;
    createItem[kGroundItemCreateHeightOffset] = 3;
    createItem[kGroundItemCreateFlagOffset] = 4;
    createItem[kGroundItemCreateOwnerOffset] = 7;
    const auto createItemBefore = createItem;
    int createItemCalls = 0;
    const auto receiveCreateItem = [&](const PacketView& view) {
        ++createItemCalls;
        check(view.data == createItem.data() && view.size == kGroundItemCreatePacketSize &&
            view.opcode == MSG_CreateItem_Opcode,
            "CreateItem preserves the 32-byte frame");
    };
    for (std::size_t n = 0; n < kGroundItemCreatePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(), n},
            receiveCreateItem), "truncated CreateItem rejected before the handler");
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, nullptr,
        kGroundItemCreatePacketSize}, receiveCreateItem), "null CreateItem rejected");
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize + 1}, receiveCreateItem), "oversized CreateItem rejected");
    check(!received_packet::Dispatch({0x119, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem), "CreateItem Type cannot be hidden");
    createItem[0] = static_cast<char>(kGroundItemCreatePacketSize - 1);
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem), "mismatched CreateItem Size rejected");
    createItem[0] = static_cast<char>(kGroundItemCreatePacketSize);
    createItem[4] = 0x19;
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem), "mismatched CreateItem opcode rejected");
    createItem[4] = 0x6E;
    check(createItemCalls == 0, "invalid CreateItem does not reach the consumer");
    check(received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem) && createItemCalls == 1,
        "valid CreateItem delivered once");
    check(static_cast<unsigned char>(createItem[kGroundItemCreateItemOffset]) == 0xAB &&
        static_cast<unsigned char>(createItem[kGroundItemCreateItemOffset + 1]) == 0x0F &&
        createItem[kGroundItemCreateRotateOffset] == 1 &&
        createItem[kGroundItemCreateStateOffset] == 2 &&
        createItem[kGroundItemCreateHeightOffset] == 3 &&
        createItem[kGroundItemCreateFlagOffset] == 4 &&
        createItem[kGroundItemCreateOwnerOffset] == 7,
        "CreateItem preserves item, flags and owner at the native offsets");
    check(createItem == createItemBefore, "gate preserves every CreateItem byte");
    check(!IsGroundItemDefinitionIndex(-1) && !IsGroundItemDefinitionIndex(0),
        "CreateItem rejects missing indices");
    check(IsGroundItemDefinitionIndex(1) &&
        IsGroundItemDefinitionIndex(kGroundItemDefinitionCount - 1),
        "CreateItem accepts the loaded ItemList bounds");
    check(!IsGroundItemDefinitionIndex(kGroundItemDefinitionCount),
        "CreateItem rejects an index past the ItemList");
    std::array<char, kGroundItemUpdatePacketSize + 1> updateItem{};
    updateItem[0] = static_cast<char>(kGroundItemUpdatePacketSize);
    updateItem[4] = 0x74;
    updateItem[5] = 0x03;
    updateItem[kGroundItemUpdateItemIDOffset] = 0x10;
    updateItem[kGroundItemUpdateItemIDOffset + 1] = 0x27;
    updateItem[kGroundItemUpdateStateOffset] = 0x34;
    updateItem[kGroundItemUpdateStateOffset + 1] = 0x12;
    updateItem[kGroundItemUpdateHeightOffset] = 5;
    const auto updateItemBefore = updateItem;
    int updateItemCalls = 0;
    const auto receiveUpdateItem = [&](const PacketView& view) {
        ++updateItemCalls;
        check(view.data == updateItem.data() && view.size == kGroundItemUpdatePacketSize &&
            view.opcode == MSG_UpdateItem_Opcode,
            "UpdateItem preserves the 20-byte frame");
    };
    for (std::size_t n = 0; n < kGroundItemUpdatePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(), n},
            receiveUpdateItem), "truncated UpdateItem rejected before the handler");
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, nullptr,
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "null UpdateItem rejected");
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize + 1}, receiveUpdateItem), "oversized UpdateItem rejected");
    check(!received_packet::Dispatch({0x119, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "UpdateItem Type cannot be hidden");
    updateItem[0] = static_cast<char>(kGroundItemUpdatePacketSize - 1);
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "mismatched UpdateItem Size rejected");
    updateItem[0] = static_cast<char>(kGroundItemUpdatePacketSize);
    updateItem[4] = 0x19;
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "mismatched UpdateItem opcode rejected");
    updateItem[4] = 0x74;
    check(updateItemCalls == 0, "invalid UpdateItem does not reach the consumer");
    check(received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem) && updateItemCalls == 1,
        "valid UpdateItem delivered once");
    check(static_cast<unsigned char>(updateItem[kGroundItemUpdateStateOffset]) == 0x34 &&
        static_cast<unsigned char>(updateItem[kGroundItemUpdateStateOffset + 1]) == 0x12 &&
        updateItem[kGroundItemUpdateHeightOffset] == 5,
        "UpdateItem preserves i16 state and i8 height");
    check(updateItem == updateItemBefore, "gate preserves every UpdateItem byte");

    std::array<char, kGroundItemRemovePacketSize + 1> removeItem{};
    removeItem[0] = static_cast<char>(kGroundItemRemovePacketSize);
    removeItem[4] = 0x6F;
    removeItem[5] = 0x01;
    removeItem[kGroundItemRemoveItemIDOffset] = 0x10;
    removeItem[kGroundItemRemoveItemIDOffset + 1] = 0x27;
    const auto removeItemBefore = removeItem;
    int removeItemCalls = 0;
    const auto receiveRemoveItem = [&](const PacketView& view) {
        ++removeItemCalls;
        check(view.data == removeItem.data() && view.size == kGroundItemRemovePacketSize &&
            view.opcode == MSG_RemoveItem_Opcode,
            "RemoveItem preserves the 16-byte frame");
    };
    for (std::size_t n = 0; n < kGroundItemRemovePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(), n},
            receiveRemoveItem), "truncated RemoveItem rejected before the handler");
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, nullptr,
        kGroundItemRemovePacketSize}, receiveRemoveItem), "null RemoveItem rejected");
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize + 1}, receiveRemoveItem), "oversized RemoveItem rejected");
    check(!received_packet::Dispatch({0x119, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem), "RemoveItem Type cannot be hidden");
    removeItem[0] = static_cast<char>(kGroundItemRemovePacketSize - 1);
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem), "mismatched RemoveItem Size rejected");
    removeItem[0] = static_cast<char>(kGroundItemRemovePacketSize);
    removeItem[4] = 0x19;
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem), "mismatched RemoveItem opcode rejected");
    removeItem[4] = 0x6F;
    check(removeItemCalls == 0, "invalid RemoveItem does not reach the consumer");
    check(received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem) && removeItemCalls == 1,
        "valid RemoveItem delivered once");
    check(static_cast<unsigned char>(removeItem[kGroundItemRemoveItemIDOffset]) == 0x10 &&
        static_cast<unsigned char>(removeItem[kGroundItemRemoveItemIDOffset + 1]) == 0x27,
        "RemoveItem preserves the ID at the native offset");
    check(removeItem == removeItemBefore, "gate preserves every RemoveItem byte");

    // Both instance counters use MSG_STANDARDPARM: a 16-byte envelope
    // with the integer value at offset 12. The callback receives the same
    // buffer, so the test also guards against normalization/copying.
    return failures;
}
