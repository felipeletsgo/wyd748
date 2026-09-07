#include "../internal/wire/ReceivedPacketDispatch.h"
#include "../internal/wire/CharacterLogoutRequestPacket.h"
#include "../internal/wire/TotoPurchasePacket.h"
#include "../internal/wire/ApplyBonusPacket.h"
#include "../internal/wire/UseItemPacket.h"
#include "../internal/wire/PKModePacket.h"
#include "../internal/wire/PremiumFireworkPacket.h"
#include "../internal/wire/PremiumFireworkUsePacket.h"
#include "../internal/wire/GamblePacket.h"
#include "../internal/wire/MobKillConfirmPacket.h"
#include "../internal/wire/UpdateEtcPacket.h"
#include "../internal/wire/MessagePanelPacket.h"
#include "../internal/wire/DelayStartPacket.h"
#include "../internal/wire/BillingNoticePacket.h"
#include "../internal/wire/PartyAddPacket.h"
#include "../internal/wire/PartyRemovePacket.h"
#include "../internal/wire/PartyRequestPacket.h"
#include "../internal/wire/MotionPacket.h"
#include <array>
#include <cstdio>
#include <cstring>

// Exercita a mesma fronteira chamada pelo ObjectManager, sem UI/socket/DirectX.
// Bytes conhecidos independem do construtor C++ para conferir o contrato wire.
int RunReceivedPacketDispatchTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) {
            ++failures;
            std::fprintf(stderr, "FAIL: %s\n", name);
        }
    };
    // O byte inicial desloca deliberadamente o frame para endereco nao alinhado.
    alignas(MSG_STANDARD) std::array<char, 54> storage{};
    char* bytes = storage.data() + 1;
    bytes[0] = 52;
    bytes[4] = static_cast<char>(0xAA);
    bytes[5] = 0x0F;
    for (int i = 12; i < 16; ++i) bytes[i] = static_cast<char>(0xFF);
    bytes[16] = 3;
    std::memcpy(bytes + 20, "OldCharacter", 12);
    std::memcpy(bytes + 36, "NewCharacter", 12);
    const auto original = storage;

    MSG_ReqTransper decoded{};
    std::memcpy(&decoded, bytes, 52);
    check(decoded.Header.Type == 0xFAA && decoded.Header.Size == 52,
        "bytes conhecidos preservam envelope de transferencia");
    check(decoded.Result == -1 && decoded.Slot == 3,
        "bytes conhecidos preservam offsets e signedness");
    check(std::memcmp(decoded.OldName, "OldCharacter", 12) == 0 &&
        std::memcmp(decoded.NewName, "NewCharacter", 12) == 0,
        "bytes conhecidos preservam os dois nomes");

    int delivered = 0;
    const auto receive = [&](const PacketView& frame) {
        ++delivered;
        check(frame.data == bytes && frame.size == 52 && frame.opcode == 0xFAA,
            "dispatch preserva endereco tamanho e opcode sem copia");
    };
    for (std::size_t size = 0; size < 52; ++size)
        check(!received_packet::Dispatch({0xFAA, bytes, size}, receive),
            "todo prefixo truncado e rejeitado antes do consumidor");
    check(!received_packet::Dispatch({0xFAA, nullptr, 52}, receive),
        "frame nulo rejeitado");
    check(!received_packet::Dispatch({0xFAA, bytes, 53}, receive),
        "frame de transferencia excedente rejeitado");
    check(delivered == 0, "rejeicoes nao alteram estado da cena por callback");
    check(received_packet::Dispatch({0xFAA, bytes, 52}, receive) && delivered == 1,
        "frame exato entregue uma vez");
    check(storage == original, "validacao nao modifica bytes do transporte");

    bytes[0] = 51;
    check(!received_packet::Dispatch({0xFAA, bytes, 52}, receive),
        "tamanho declarado divergente rejeitado");
    bytes[0] = 52;
    check(!received_packet::Dispatch({0x119, bytes, 52}, receive),
        "metadado nao pode esconder Type de transferencia");
    bytes[4] = 0x19;
    bytes[5] = 0x01;
    check(!received_packet::Dispatch({0xFAA, bytes, 52}, receive),
        "metadado transferencia exige Type correspondente");
    check(delivered == 1, "divergencias nao executam callback nem retry");

    int otherDelivered = 0;
    check(received_packet::Dispatch({0x119, bytes, 12}, [&](const PacketView& frame) {
        ++otherDelivered;
        check(frame.data == bytes && frame.size == 12 && frame.opcode == 0x119,
            "outro opcode preserva view do percurso legado");
    }) && otherDelivered == 1, "opcode fora deste lote mantem fallback");
    // Mesmo fixture de 24 bytes do encoder Go: ID=0x1234, cargo[127], item
    // 0x1234 com seis bytes de efeitos. Nao depende da struct de Basedef.
    char sendItem[25] = {24, 0, 0, 0, static_cast<char>(0x82), 1, 0x34, 0x12,
        0, 0, 0, 0, 2, 0, 127, 0, 0x34, 0x12, 1, 2, 3, 4, 5, 6, 0};
    int itemDelivered = 0;
    const auto receiveItem = [&](const PacketView& frame) {
        ++itemDelivered;
        check(frame.data == sendItem && frame.size == 24 && frame.opcode == 0x182,
            "SendItem valido conserva frame e entrega unica");
    };
    for (std::size_t size = 0; size < 24; ++size)
        check(!received_packet::Dispatch({0x182, sendItem, size}, receiveItem),
            "SendItem truncado rejeitado antes da copia");
    check(!received_packet::Dispatch({0x182, sendItem, 25}, receiveItem),
        "SendItem excedente rejeitado");
    check(!received_packet::Dispatch({0xFAA, sendItem, 24}, receiveItem),
        "contratos conhecidos nao podem trocar metadados");
    check(!received_packet::Dispatch({0x119, sendItem, 24}, receiveItem),
        "metadado desconhecido nao contorna Type SendItem");
    sendItem[0] = 23;
    check(!received_packet::Dispatch({0x182, sendItem, 24}, receiveItem),
        "SendItem rejeita Size declarado divergente");
    sendItem[0] = 24;
    check(itemDelivered == 0, "SendItem invalido nao chama consumidor");
    check(received_packet::Dispatch({0x182, sendItem, 24}, receiveItem) && itemDelivered == 1,
        "SendItem exato entregue sem retry");
    // Frame do builder Go: destino Carry[62] e STRUCT_ITEM de oito bytes.
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
            "CNFGetItem preserva frame de 28 bytes");
    };
    for (std::size_t n = 0; n < kPickupConfirmationPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(), n}, receivePickup),
            "CNFGetItem truncado rejeitado antes do handler");
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, nullptr,
        kPickupConfirmationPacketSize}, receivePickup), "CNFGetItem nulo rejeitado");
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize + 1}, receivePickup), "CNFGetItem excedente rejeitado");
    check(!received_packet::Dispatch({0x119, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup), "Type CNFGetItem nao pode ser ocultado");
    pickup[0] = static_cast<char>(kPickupConfirmationPacketSize - 1);
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup), "Size CNFGetItem divergente rejeitado");
    pickup[0] = static_cast<char>(kPickupConfirmationPacketSize);
    pickup[4] = 0x19;
    check(!received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup), "opcode CNFGetItem divergente rejeitado");
    pickup[4] = 0x71;
    check(pickupCalls == 0, "CNFGetItem invalido nao chega ao consumidor");
    check(received_packet::Dispatch({MSG_CNFGetItem_Opcode, pickup.data(),
        kPickupConfirmationPacketSize}, receivePickup) && pickupCalls == 1,
        "CNFGetItem valido entregue uma vez");
    check(pickup[kPickupConfirmationDestTypeOffset] == 1 &&
        pickup[kPickupConfirmationDestPosOffset] == 62 &&
        static_cast<unsigned char>(pickup[kPickupConfirmationItemOffset]) == 0x34 &&
        static_cast<unsigned char>(pickup[kPickupConfirmationItemOffset + 1]) == 0x12,
        "CNFGetItem preserva destino e item nos offsets nativos");
    check(pickup == pickupBefore, "gate preserva todos os bytes do CNFGetItem");
    check(!IsPickupCarrySlot(-1), "CNFGetItem rejeita slot negativo");
    for (int slot = 0; slot < kPickupVisibleCarrySlotCount; ++slot)
        check(IsPickupCarrySlot(slot), "CNFGetItem aceita cada slot visivel do Carry 9x7");
    check(!IsPickupCarrySlot(kPickupVisibleCarrySlotCount),
        "CNFGetItem rejeita slot estrutural 63 sem celula visual");
    // Mesmo frame produzido por wire.CNFDropItem(1, 62, 3, 2200, 2100).
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
            "CNFDropItem preserva frame de 28 bytes");
    };
    for (std::size_t n = 0; n < kDropConfirmationPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(), n}, receiveDrop),
            "CNFDropItem truncado rejeitado antes do handler");
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, nullptr,
        kDropConfirmationPacketSize}, receiveDrop), "CNFDropItem nulo rejeitado");
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize + 1}, receiveDrop), "CNFDropItem excedente rejeitado");
    check(!received_packet::Dispatch({0x119, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop), "Type CNFDropItem nao pode ser ocultado");
    drop[0] = static_cast<char>(kDropConfirmationPacketSize - 1);
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop), "Size CNFDropItem divergente rejeitado");
    drop[0] = static_cast<char>(kDropConfirmationPacketSize);
    drop[4] = 0x19;
    check(!received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop), "opcode CNFDropItem divergente rejeitado");
    drop[4] = 0x75;
    check(dropCalls == 0, "CNFDropItem invalido nao chega ao consumidor");
    check(received_packet::Dispatch({MSG_CNFDropItem_Opcode, drop.data(),
        kDropConfirmationPacketSize}, receiveDrop) && dropCalls == 1,
        "CNFDropItem valido entregue uma vez");
    check(static_cast<unsigned char>(drop[kDropConfirmationGridXOffset]) == 0x98 &&
        static_cast<unsigned char>(drop[kDropConfirmationGridXOffset + 1]) == 0x08 &&
        static_cast<unsigned char>(drop[kDropConfirmationGridYOffset]) == 0x34 &&
        static_cast<unsigned char>(drop[kDropConfirmationGridYOffset + 1]) == 0x08,
        "CNFDropItem preserva rotacao e coordenadas nos offsets nativos");
    check(drop == dropBefore, "gate preserva todos os bytes do CNFDropItem");
    check(!IsDropCarrySlot(-1), "CNFDropItem rejeita Carry negativo");
    for (int slot = 0; slot < kDropVisibleCarrySlotCount; ++slot)
        check(IsDropCarrySlot(slot), "CNFDropItem aceita cada slot visivel do Carry");
    check(!IsDropCarrySlot(kDropVisibleCarrySlotCount),
        "CNFDropItem rejeita slot estrutural 63 do Carry");
    check(!IsDropCargoSlot(-1), "CNFDropItem rejeita Cargo negativo");
    for (int slot = 0; slot < kDropUsableCargoSlotCount; ++slot)
        check(IsDropCargoSlot(slot), "CNFDropItem aceita cada slot utilizavel do Cargo");
    check(!IsDropCargoSlot(kDropUsableCargoSlotCount),
        "CNFDropItem rejeita primeiro slot reservado do Cargo");
    // Mesmo frame produzido por wire.CreateItem para uma aparicao no chao.
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
            "CreateItem preserva frame de 32 bytes");
    };
    for (std::size_t n = 0; n < kGroundItemCreatePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(), n},
            receiveCreateItem), "CreateItem truncado rejeitado antes do handler");
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, nullptr,
        kGroundItemCreatePacketSize}, receiveCreateItem), "CreateItem nulo rejeitado");
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize + 1}, receiveCreateItem), "CreateItem excedente rejeitado");
    check(!received_packet::Dispatch({0x119, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem), "Type CreateItem nao pode ser ocultado");
    createItem[0] = static_cast<char>(kGroundItemCreatePacketSize - 1);
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem), "Size CreateItem divergente rejeitado");
    createItem[0] = static_cast<char>(kGroundItemCreatePacketSize);
    createItem[4] = 0x19;
    check(!received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem), "opcode CreateItem divergente rejeitado");
    createItem[4] = 0x6E;
    check(createItemCalls == 0, "CreateItem invalido nao chega ao consumidor");
    check(received_packet::Dispatch({MSG_CreateItem_Opcode, createItem.data(),
        kGroundItemCreatePacketSize}, receiveCreateItem) && createItemCalls == 1,
        "CreateItem valido entregue uma vez");
    check(static_cast<unsigned char>(createItem[kGroundItemCreateItemOffset]) == 0xAB &&
        static_cast<unsigned char>(createItem[kGroundItemCreateItemOffset + 1]) == 0x0F &&
        createItem[kGroundItemCreateRotateOffset] == 1 &&
        createItem[kGroundItemCreateStateOffset] == 2 &&
        createItem[kGroundItemCreateHeightOffset] == 3 &&
        createItem[kGroundItemCreateFlagOffset] == 4 &&
        createItem[kGroundItemCreateOwnerOffset] == 7,
        "CreateItem preserva item, flags e owner nos offsets nativos");
    check(createItem == createItemBefore, "gate preserva todos os bytes do CreateItem");
    check(!IsGroundItemDefinitionIndex(-1) && !IsGroundItemDefinitionIndex(0),
        "CreateItem rejeita indices ausentes");
    check(IsGroundItemDefinitionIndex(1) &&
        IsGroundItemDefinitionIndex(kGroundItemDefinitionCount - 1),
        "CreateItem aceita os limites carregados do ItemList");
    check(!IsGroundItemDefinitionIndex(kGroundItemDefinitionCount),
        "CreateItem rejeita indice depois do ItemList");
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
            "UpdateItem preserva frame de 20 bytes");
    };
    for (std::size_t n = 0; n < kGroundItemUpdatePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(), n},
            receiveUpdateItem), "UpdateItem truncado rejeitado antes do handler");
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, nullptr,
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "UpdateItem nulo rejeitado");
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize + 1}, receiveUpdateItem), "UpdateItem excedente rejeitado");
    check(!received_packet::Dispatch({0x119, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "Type UpdateItem nao pode ser ocultado");
    updateItem[0] = static_cast<char>(kGroundItemUpdatePacketSize - 1);
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "Size UpdateItem divergente rejeitado");
    updateItem[0] = static_cast<char>(kGroundItemUpdatePacketSize);
    updateItem[4] = 0x19;
    check(!received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem), "opcode UpdateItem divergente rejeitado");
    updateItem[4] = 0x74;
    check(updateItemCalls == 0, "UpdateItem invalido nao chega ao consumidor");
    check(received_packet::Dispatch({MSG_UpdateItem_Opcode, updateItem.data(),
        kGroundItemUpdatePacketSize}, receiveUpdateItem) && updateItemCalls == 1,
        "UpdateItem valido entregue uma vez");
    check(static_cast<unsigned char>(updateItem[kGroundItemUpdateStateOffset]) == 0x34 &&
        static_cast<unsigned char>(updateItem[kGroundItemUpdateStateOffset + 1]) == 0x12 &&
        updateItem[kGroundItemUpdateHeightOffset] == 5,
        "UpdateItem preserva state i16 e height i8");
    check(updateItem == updateItemBefore, "gate preserva todos os bytes do UpdateItem");

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
            "RemoveItem preserva frame de 16 bytes");
    };
    for (std::size_t n = 0; n < kGroundItemRemovePacketSize; ++n)
        check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(), n},
            receiveRemoveItem), "RemoveItem truncado rejeitado antes do handler");
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, nullptr,
        kGroundItemRemovePacketSize}, receiveRemoveItem), "RemoveItem nulo rejeitado");
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize + 1}, receiveRemoveItem), "RemoveItem excedente rejeitado");
    check(!received_packet::Dispatch({0x119, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem), "Type RemoveItem nao pode ser ocultado");
    removeItem[0] = static_cast<char>(kGroundItemRemovePacketSize - 1);
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem), "Size RemoveItem divergente rejeitado");
    removeItem[0] = static_cast<char>(kGroundItemRemovePacketSize);
    removeItem[4] = 0x19;
    check(!received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem), "opcode RemoveItem divergente rejeitado");
    removeItem[4] = 0x6F;
    check(removeItemCalls == 0, "RemoveItem invalido nao chega ao consumidor");
    check(received_packet::Dispatch({MSG_RemoveItem_Opcode, removeItem.data(),
        kGroundItemRemovePacketSize}, receiveRemoveItem) && removeItemCalls == 1,
        "RemoveItem valido entregue uma vez");
    check(static_cast<unsigned char>(removeItem[kGroundItemRemoveItemIDOffset]) == 0x10 &&
        static_cast<unsigned char>(removeItem[kGroundItemRemoveItemIDOffset + 1]) == 0x27,
        "RemoveItem preserva ID no offset nativo");
    check(removeItem == removeItemBefore, "gate preserva todos os bytes do RemoveItem");

    // Os dois contadores de instancia usam MSG_STANDARDPARM: envelope de 16
    // bytes, com o valor inteiro no offset 12. O callback recebe o mesmo
    // buffer, portanto o teste tambem protege contra normalizacao/copia.
    for (unsigned int opcode : {MSG_InstanceTime_Opcode, MSG_InstanceMobs_Opcode})
    {
        std::array<char, kInstanceCounterPacketSize + 1> counter{};
        counter[0] = static_cast<char>(kInstanceCounterPacketSize);
        counter[4] = static_cast<char>(opcode & 0xFF);
        counter[5] = static_cast<char>((opcode >> 8) & 0xFF);
        counter[kInstanceCounterValueOffset] = static_cast<char>(0x78);
        counter[kInstanceCounterValueOffset + 1] = static_cast<char>(0x56);
        counter[kInstanceCounterValueOffset + 2] = static_cast<char>(0x34);
        counter[kInstanceCounterValueOffset + 3] = static_cast<char>(0x12);
        const auto counterBefore = counter;
        int counterCalls = 0;
        const auto receiveCounter = [&](const PacketView& view) {
            ++counterCalls;
            check(view.data == counter.data() && view.size == kInstanceCounterPacketSize &&
                view.opcode == opcode,
                "contador de instancia preserva frame e opcode");
        };
        for (std::size_t n = 0; n < kInstanceCounterPacketSize; ++n)
            check(!received_packet::Dispatch({opcode, counter.data(), n}, receiveCounter),
                "contador de instancia rejeita todo prefixo truncado");
        check(!received_packet::Dispatch({opcode, nullptr, kInstanceCounterPacketSize},
            receiveCounter), "contador de instancia nulo rejeitado");
        check(!received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize + 1}, receiveCounter),
            "contador de instancia excedente rejeitado");
        check(!received_packet::Dispatch({0x119, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter),
            "opcode externo nao pode ocultar contador de instancia");
        counter[4] = static_cast<char>((opcode + 1) & 0xFF);
        counter[5] = static_cast<char>(((opcode + 1) >> 8) & 0xFF);
        check(!received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter),
            "Header.Type divergente rejeitado para contador de instancia");
        counter[4] = static_cast<char>(opcode & 0xFF);
        counter[5] = static_cast<char>((opcode >> 8) & 0xFF);
        counter[0] = static_cast<char>(kInstanceCounterPacketSize - 1);
        check(!received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter),
            "Header.Size divergente rejeitado para contador de instancia");
        counter[0] = static_cast<char>(kInstanceCounterPacketSize);
        check(counterCalls == 0, "contador invalido nao chega ao consumidor");
        check(received_packet::Dispatch({opcode, counter.data(),
            kInstanceCounterPacketSize}, receiveCounter) && counterCalls == 1,
            "contador valido entregue uma vez");
        check(static_cast<unsigned char>(counter[kInstanceCounterValueOffset]) == 0x78 &&
            static_cast<unsigned char>(counter[kInstanceCounterValueOffset + 1]) == 0x56 &&
            static_cast<unsigned char>(counter[kInstanceCounterValueOffset + 2]) == 0x34 &&
            static_cast<unsigned char>(counter[kInstanceCounterValueOffset + 3]) == 0x12,
            "contador preserva valor no offset 12");
        check(counter == counterBefore, "gate preserva bytes do contador de instancia");
    }

    // RemoveMob e UpdateCargoGold compartilham o envelope de um DWORD, mas o
    // Header.ID ainda seleciona receptores distintos. Validar os dois evita
    // que um frame curto alcance tanto a arvore de entidades quanto a Field.
    for (unsigned int opcode : {MSG_RemoveMob_Opcode, MSG_UpdateCargoGold_Opcode})
    {
        std::array<char, kWorldStateParameterPacketSize + 1> worldState{};
        worldState[0] = static_cast<char>(kWorldStateParameterPacketSize);
        worldState[4] = static_cast<char>(opcode & 0xFF);
        worldState[5] = static_cast<char>((opcode >> 8) & 0xFF);
        worldState[6] = static_cast<char>(0x34);
        worldState[7] = static_cast<char>(0x12);
        worldState[kWorldStateParameterValueOffset] = static_cast<char>(0xEF);
        worldState[kWorldStateParameterValueOffset + 1] = static_cast<char>(0xCD);
        worldState[kWorldStateParameterValueOffset + 2] = static_cast<char>(0xAB);
        worldState[kWorldStateParameterValueOffset + 3] = static_cast<char>(0x89);
        const auto worldStateBefore = worldState;
        int worldStateCalls = 0;
        const auto receiveWorldState = [&](const PacketView& view) {
            ++worldStateCalls;
            check(view.data == worldState.data() &&
                view.size == kWorldStateParameterPacketSize && view.opcode == opcode,
                "estado do mundo preserva frame e opcode");
        };
        for (std::size_t n = 0; n < kWorldStateParameterPacketSize; ++n)
            check(!received_packet::Dispatch({opcode, worldState.data(), n},
                receiveWorldState), "estado do mundo rejeita todo prefixo truncado");
        check(!received_packet::Dispatch({opcode, nullptr,
            kWorldStateParameterPacketSize}, receiveWorldState),
            "estado do mundo nulo rejeitado");
        check(!received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize + 1}, receiveWorldState),
            "estado do mundo excedente rejeitado");
        check(!received_packet::Dispatch({0x119, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState),
            "opcode externo nao pode ocultar estado do mundo");
        worldState[4] = static_cast<char>((opcode + 1) & 0xFF);
        worldState[5] = static_cast<char>(((opcode + 1) >> 8) & 0xFF);
        check(!received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState),
            "Header.Type divergente rejeitado para estado do mundo");
        worldState[4] = static_cast<char>(opcode & 0xFF);
        worldState[5] = static_cast<char>((opcode >> 8) & 0xFF);
        worldState[0] = static_cast<char>(kWorldStateParameterPacketSize - 1);
        check(!received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState),
            "Header.Size divergente rejeitado para estado do mundo");
        worldState[0] = static_cast<char>(kWorldStateParameterPacketSize);
        check(worldStateCalls == 0, "estado invalido nao chega ao consumidor");
        check(received_packet::Dispatch({opcode, worldState.data(),
            kWorldStateParameterPacketSize}, receiveWorldState) && worldStateCalls == 1,
            "estado do mundo valido entregue uma vez");
        check(static_cast<unsigned char>(worldState[6]) == 0x34 &&
            static_cast<unsigned char>(worldState[7]) == 0x12,
            "estado do mundo preserva Header.ID");
        check(static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset]) == 0xEF &&
            static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset + 1]) == 0xCD &&
            static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset + 2]) == 0xAB &&
            static_cast<unsigned char>(worldState[kWorldStateParameterValueOffset + 3]) == 0x89,
            "estado do mundo preserva DWORD no offset 12");
        check(worldState == worldStateBefore,
            "gate preserva todos os bytes do estado do mundo");
    }

    // O source client e o WYD-Go usam um unico 0x181 de 28 bytes. Este fixture
    // fixa a extensao coordenada e impede aceitar silenciosamente os layouts
    // historicos de 20/36 bytes sob o mesmo opcode.
    std::array<char, kHpMpPacketSize + 1> hpMp{};
    hpMp[0] = static_cast<char>(kHpMpPacketSize);
    hpMp[4] = static_cast<char>(MSG_SetHpMp_Opcode & 0xFF);
    hpMp[5] = static_cast<char>((MSG_SetHpMp_Opcode >> 8) & 0xFF);
    hpMp[6] = 0x34;
    hpMp[7] = 0x12;
    const std::array<std::size_t, 4> hpMpOffsets{
        kHpMpCurrentHpOffset, kHpMpCurrentMpOffset,
        kHpMpMaximumHpOffset, kHpMpMaximumMpOffset
    };
    for (std::size_t field = 0; field < hpMpOffsets.size(); ++field)
    {
        const auto offset = hpMpOffsets[field];
        hpMp[offset] = static_cast<char>(0x10 + field);
        hpMp[offset + 1] = static_cast<char>(0x20 + field);
        hpMp[offset + 2] = static_cast<char>(0x30 + field);
        hpMp[offset + 3] = static_cast<char>(0x40 + field);
    }
    const auto hpMpBefore = hpMp;
    int hpMpCalls = 0;
    const auto receiveHpMp = [&](const PacketView& view) {
        ++hpMpCalls;
        check(view.data == hpMp.data() && view.size == kHpMpPacketSize &&
            view.opcode == MSG_SetHpMp_Opcode,
            "HP/MP coordenado preserva frame e opcode");
    };
    for (std::size_t n = 0; n < kHpMpPacketSize; ++n)
        check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), n},
            receiveHpMp), "HP/MP coordenado rejeita todo prefixo truncado");
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, nullptr, kHpMpPacketSize},
        receiveHpMp), "HP/MP coordenado rejeita buffer nulo");
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(),
        kHpMpPacketSize + 1}, receiveHpMp), "HP/MP coordenado rejeita frame excedente");
    check(!received_packet::Dispatch({0x119, hpMp.data(), kHpMpPacketSize},
        receiveHpMp), "opcode externo nao pode ocultar HP/MP coordenado");
    hpMp[4] = static_cast<char>((MSG_SetHpMp_Opcode + 1) & 0xFF);
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), kHpMpPacketSize},
        receiveHpMp), "HP/MP coordenado rejeita Header.Type divergente");
    hpMp[4] = static_cast<char>(MSG_SetHpMp_Opcode & 0xFF);
    hpMp[0] = static_cast<char>(kHpMpPacketSize - 1);
    check(!received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), kHpMpPacketSize},
        receiveHpMp), "HP/MP coordenado rejeita Header.Size divergente");
    hpMp[0] = static_cast<char>(kHpMpPacketSize);
    check(hpMpCalls == 0, "HP/MP invalido nao chega ao consumidor");
    check(received_packet::Dispatch({MSG_SetHpMp_Opcode, hpMp.data(), kHpMpPacketSize},
        receiveHpMp) && hpMpCalls == 1, "HP/MP coordenado entregue uma vez");
    check(static_cast<unsigned char>(hpMp[6]) == 0x34 &&
        static_cast<unsigned char>(hpMp[7]) == 0x12,
        "HP/MP coordenado preserva receptor em Header.ID");
    for (std::size_t field = 0; field < hpMpOffsets.size(); ++field)
    {
        const auto offset = hpMpOffsets[field];
        check(static_cast<unsigned char>(hpMp[offset]) == 0x10 + field &&
            static_cast<unsigned char>(hpMp[offset + 1]) == 0x20 + field &&
            static_cast<unsigned char>(hpMp[offset + 2]) == 0x30 + field &&
            static_cast<unsigned char>(hpMp[offset + 3]) == 0x40 + field,
            "HP/MP coordenado preserva recurso uint32 no offset contratado");
    }
    check(hpMp == hpMpBefore, "gate preserva todos os bytes do HP/MP coordenado");

    // Frame Go MessagePanel: ID zero, texto em +12 e NUL final em +107.
    std::array<char, 109> messagePanel{};
    messagePanel[0] = 108;
    messagePanel[4] = 0x01;
    messagePanel[5] = 0x01;
    std::memcpy(messagePanel.data() + 12, "Inventario limpo", 17);
    int panelCalls = 0;
    const auto receivePanel = [&](const PacketView& view) {
        ++panelCalls;
        check(view.data == messagePanel.data() && view.size == 108 && view.opcode == 0x101,
            "MessagePanel preserva buffer comprimento e opcode");
    };
    for (std::size_t n = 0; n < 108; ++n)
        check(!received_packet::Dispatch({0x101, messagePanel.data(), n}, receivePanel),
            "MessagePanel rejeita todos os prefixos truncados");
    check(!received_packet::Dispatch({0x101, messagePanel.data(), 109}, receivePanel),
        "MessagePanel excedente rejeitado");
    check(!received_packet::Dispatch({0x101, nullptr, 108}, receivePanel),
        "MessagePanel nulo rejeitado");
    check(!received_packet::Dispatch({0x119, messagePanel.data(), 108}, receivePanel),
        "Type MessagePanel nao pode ser ocultado");
    messagePanel[4] = 0x19;
    check(!received_packet::Dispatch({0x101, messagePanel.data(), 108}, receivePanel),
        "Type MessagePanel divergente rejeitado");
    messagePanel[4] = 0x01;
    messagePanel[0] = 107;
    check(!received_packet::Dispatch({0x101, messagePanel.data(), 108}, receivePanel),
        "Size MessagePanel divergente rejeitado");
    messagePanel[0] = 108;
    check(panelCalls == 0, "MessagePanel invalido nao chega ao consumidor");
    const auto panelBefore = messagePanel;
    check(received_packet::Dispatch({0x101, messagePanel.data(), 108}, receivePanel) && panelCalls == 1,
        "MessagePanel valido entregue uma vez");
    MSG_MessagePanel decodedPanel{};
    std::memcpy(&decodedPanel, messagePanel.data(), sizeof(decodedPanel));
    check(decodedPanel.Header.ID == 0 && std::strcmp(decodedPanel.String, "Inventario limpo") == 0 &&
        decodedPanel.String[95] == 0,
        "MessagePanel preserva ID zero texto e terminador final");
    check(messagePanel == panelBefore, "gate preserva todos os bytes do MessagePanel");
    // Fixtures independentes das structs: valida comprimento real, declarado
    // e ambos os discriminantes antes de qualquer callback de cena.
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
                "mensagem opaca preserva buffer e comprimento");
        };
        for (std::size_t n = 0; n < length; ++n)
            check(!received_packet::Dispatch({opcode, frame, n}, receiver),
                "mensagem opaca rejeita todos os prefixos truncados");
        check(!received_packet::Dispatch({opcode, frame, length + 1}, receiver), "excesso rejeitado");
        check(!received_packet::Dispatch({opcode, nullptr, length}, receiver), "nulo rejeitado");
        check(!received_packet::Dispatch({0x119, frame, length}, receiver), "Type nao pode ser ocultado");
        frame[0] = 12;
        check(!received_packet::Dispatch({opcode, frame, length}, receiver), "Size divergente rejeitado");
        frame[0] = static_cast<char>(length);
        frame[4] = 0x19;
        check(!received_packet::Dispatch({opcode, frame, length}, receiver), "opcode divergente rejeitado");
        frame[4] = static_cast<char>(opcode & 255);
        check(calls == 0, "rejeicoes opacas nao executam callback");
        check(received_packet::Dispatch({opcode, frame, length}, receiver) && calls == 1,
            "mensagem opaca entregue uma vez");
        check(raw == snapshot, "bytes opacos preservados");
    }
    // Frame Go MessageChat: 108 bytes, ID do remetente e texto em offset 12.
    std::array<char, 109> chat{};
    chat[0] = 108;
    chat[4] = 0x33;
    chat[5] = 3;
    chat[6] = 0x34;
    chat[7] = 0x12;
    std::memcpy(chat.data() + 12, "Teste", 6);
    MSG_MessageChat decodedChat{};
    std::memcpy(&decodedChat, chat.data(), 108);
    check(decodedChat.Header.ID == 0x1234 && std::strcmp(decodedChat.String, "Teste") == 0,
        "fixture chat confirma ID e offset do texto");
    int chatCalls = 0;
    const auto chatReceiver = [&](const PacketView& view) {
        ++chatCalls;
        check(view.data == chat.data() && view.size == 108, "chat preserva buffer emprestado");
    };
    for (std::size_t size = 0; size < 108; ++size)
        check(!received_packet::Dispatch({0x333, chat.data(), size}, chatReceiver), "chat truncado rejeitado");
    check(!received_packet::Dispatch({0x333, chat.data(), 109}, chatReceiver), "chat excedente rejeitado");
    check(!received_packet::Dispatch({0x333, nullptr, 108}, chatReceiver), "chat nulo rejeitado");
    check(!received_packet::Dispatch({0x119, chat.data(), 108}, chatReceiver), "Type chat nao pode ser ocultado");
    chat[4] = 0x19;
    check(!received_packet::Dispatch({0x333, chat.data(), 108}, chatReceiver), "opcode chat divergente rejeitado");
    chat[4] = 0x33;
    chat[0] = 107;
    check(!received_packet::Dispatch({0x333, chat.data(), 108}, chatReceiver), "Size chat divergente rejeitado");
    chat[0] = 108;
    check(chatCalls == 0, "chat invalido nao chega ao consumidor");
    const auto chatSnapshot = chat;
    check(received_packet::Dispatch({0x333, chat.data(), 108}, chatReceiver) && chatCalls == 1,
        "chat valido entregue uma vez");
    check(chat == chatSnapshot, "gate nao modifica texto ou header");
    for (unsigned int opcode : {0x105u, 0x106u})
    {
        std::array<char, 109> frame{};
        frame[0] = 108;
        frame[4] = static_cast<char>(opcode & 255);
        frame[5] = 1;
        // -938 little-endian, mesmo indice do convite de grupo no Go.
        frame[14] = 0x56;
        frame[15] = static_cast<char>(0xFC);
        if (opcode == 0x106) std::memcpy(frame.data() + 16, "Lider", 6);
        int calls = 0;
        const auto receiveExtension = [&](const PacketView& view) {
            ++calls;
            check(view.data == frame.data() && view.size == 108 && view.opcode == opcode,
                "extensao preserva frame para parser existente");
        };
        for (std::size_t n = 0; n < 108; ++n)
            check(!received_packet::Dispatch({opcode, frame.data(), n}, receiveExtension),
                "extensao rejeita todos os prefixos");
        check(!received_packet::Dispatch({opcode, nullptr, 108}, receiveExtension), "extensao nula");
        check(!received_packet::Dispatch({opcode, frame.data(), 109}, receiveExtension), "extensao excedente");
        check(!received_packet::Dispatch({0x119, frame.data(), 108}, receiveExtension), "Type extensao ocultado");
        frame[4] = 0x19;
        check(!received_packet::Dispatch({opcode, frame.data(), 108}, receiveExtension), "Type extensao divergente");
        frame[4] = static_cast<char>(opcode & 255);
        frame[0] = 107;
        check(!received_packet::Dispatch({opcode, frame.data(), 108}, receiveExtension), "Size extensao divergente");
        frame[0] = 108;
        check(calls == 0, "extensao invalida sem callback");
        const auto before = frame;
        check(received_packet::Dispatch({opcode, frame.data(), 108}, receiveExtension) && calls == 1,
            "extensao valida entregue uma vez");
        check(before == frame, "indice assinado e CSV preservados");
    }
    std::array<char, 81> migration{};
    migration[0] = 80;
    migration[4] = 0x2A;
    migration[5] = 5;
    migration[6] = 0x34;
    migration[7] = 0x12;
    // Dados sinteticos: conta ocupa 16 bytes e ticket ocupa 52, sem NUL.
    std::memset(migration.data() + 12, 'A', 16);
    std::memset(migration.data() + 28, 'T', 52);
    int migrationCalls = 0;
    MSG_CNFRemoveServer retained{};
    const auto retainMigration = [&](const PacketView& frame) {
        ++migrationCalls;
        check(frame.data == migration.data() && frame.size == 80,
            "migracao empresta a imagem completa");
        std::memcpy(&retained, frame.data, sizeof(retained));
    };
    for (std::size_t n = 0; n < 80; ++n)
        check(!received_packet::Dispatch({0x52A, migration.data(), n}, retainMigration),
            "migracao truncada nao pode alimentar replay");
    check(!received_packet::Dispatch({0x52A, migration.data(), 81}, retainMigration), "migracao excedente");
    check(!received_packet::Dispatch({0x52A, nullptr, 80}, retainMigration), "migracao nula");
    check(!received_packet::Dispatch({0x119, migration.data(), 80}, retainMigration), "Type migracao ocultado");
    migration[4] = 0x19;
    check(!received_packet::Dispatch({0x52A, migration.data(), 80}, retainMigration), "Type migracao divergente");
    migration[4] = 0x2A;
    migration[0] = 79;
    check(!received_packet::Dispatch({0x52A, migration.data(), 80}, retainMigration), "Size migracao divergente");
    migration[0] = 80;
    check(migrationCalls == 0 && retained.Header.Size == 0, "rejeicao preserva estado do receptor");
    check(received_packet::Dispatch({0x52A, migration.data(), 80}, retainMigration) && migrationCalls == 1,
        "migracao valida entregue uma vez");
    check(retained.Header.ID == 0x1234 && retained.AccountName[15] == 'A' && retained.TID[51] == 'T',
        "migracao preserva ID e limites dos campos inline");
    check(std::memcmp(&retained, migration.data(), 80) == 0, "imagem de migracao preservada byte a byte");
    const auto parseTicket = [&](const char* text, bool valid, int expected) {
        char ticket[52]{};
        const auto length = std::strlen(text);
        std::memcpy(ticket, text, length < sizeof(ticket) ? length : sizeof(ticket));
        int result = 99;
        check(ParseMigrationServer(ticket, 7, result) == valid, "ticket valida prefixo e capacidade");
        check(result == (valid ? expected : 99), "ticket invalido preserva saida");
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
        "ticket sem NUL respeita limite fisico e aceita sufixo opaco");
    check(!ParseMigrationServer(fullTicket, 0, parsed), "capacidade vazia rejeitada");
    std::array<char, 129> whisper{};
    whisper[0] = static_cast<char>(128);
    whisper[4] = 0x34; whisper[5] = 3;
    std::memcpy(whisper.data() + 12, "Remetente", 10);
    std::memcpy(whisper.data() + 28, "--Canal", 8);
    whisper[124] = 3;
    int whisperCalls = 0;
    const auto receiveWhisper = [&](const PacketView& view) {
        ++whisperCalls;
        check(view.data == whisper.data() && view.size == 128, "whisper preserva view");
    };
    for (std::size_t n = 0; n < 128; ++n)
        check(!received_packet::Dispatch({0x334, whisper.data(), n}, receiveWhisper), "whisper truncado rejeitado");
    check(!received_packet::Dispatch({0x334, whisper.data(), 129}, receiveWhisper), "whisper excedente");
    check(!received_packet::Dispatch({0x334, nullptr, 128}, receiveWhisper), "whisper nulo");
    check(!received_packet::Dispatch({0x119, whisper.data(), 128}, receiveWhisper), "whisper Type ocultado");
    whisper[4] = 0x19;
    check(!received_packet::Dispatch({0x334, whisper.data(), 128}, receiveWhisper), "whisper Type divergente");
    whisper[4] = 0x34; whisper[0] = 127;
    check(!received_packet::Dispatch({0x334, whisper.data(), 128}, receiveWhisper), "whisper Size divergente");
    whisper[0] = static_cast<char>(128);
    check(whisperCalls == 0, "whisper rejeitado sem callback");
    const auto whisperBefore = whisper;
    check(received_packet::Dispatch({0x334, whisper.data(), 128}, receiveWhisper) && whisperCalls == 1,
        "whisper valido entregue uma vez");
    MSG_MessageWhisper decodedWhisper{};
    std::memcpy(&decodedWhisper, whisper.data(), 128);
    check(std::strcmp(decodedWhisper.MobName, "Remetente") == 0 &&
        std::strcmp(decodedWhisper.String, "--Canal") == 0 && decodedWhisper.Color == 3,
        "fixture whisper confirma offsets nome texto e cor");
    check(whisper == whisperBefore, "whisper preserva bytes antes do handler");
    std::array<char, 13> logout{};
    logout[0] = 12;
    logout[4] = 0x16;
    logout[5] = 1;
    logout[6] = 0x34;
    logout[7] = 0x12;
    int logoutCalls = 0;
    const auto receiveLogout = [&](const PacketView& view) {
        ++logoutCalls;
        check(view.data == logout.data() && view.size == 12,
            "logout confirma view de 12 bytes");
    };
    for (std::size_t n = 0; n < 12; ++n)
        check(!received_packet::Dispatch({0x116, logout.data(), n}, receiveLogout),
            "logout truncado rejeitado");
    check(!received_packet::Dispatch({0x116, logout.data(), 13}, receiveLogout),
        "logout excedente rejeitado");
    check(!received_packet::Dispatch({0x116, nullptr, 12}, receiveLogout),
        "logout nulo rejeitado");
    check(!received_packet::Dispatch({0x119, logout.data(), 12}, receiveLogout),
        "logout Type nao pode ser ocultado");
    logout[4] = 0x19;
    check(!received_packet::Dispatch({0x116, logout.data(), 12}, receiveLogout),
        "logout Type divergente rejeitado");
    logout[4] = 0x16;
    logout[0] = 11;
    check(!received_packet::Dispatch({0x116, logout.data(), 12}, receiveLogout),
        "logout Size divergente rejeitado");
    logout[0] = 12;
    check(logoutCalls == 0, "logout invalido nao chega ao callback");
    const auto logoutBefore = logout;
    check(received_packet::Dispatch({0x116, logout.data(), 12}, receiveLogout) && logoutCalls == 1,
        "logout valido entregue uma vez");
    check(logout == logoutBefore, "gate de logout preserva header e ID");
    MSG_CharacterLogout request{};
    request.ID = 0x1234;
    request.Type = MSG_CharacterLogout_Opcode;
    request.Size = sizeof(request);
    check(sizeof(request) == 12 && request.ID == 0x1234 &&
        request.Type == 0x215 && request.Size == 12,
        "logout request C-S preserva ID, opcode e tamanho");
    std::array<char, 2105> loginConfirm{};
    loginConfirm[0] = static_cast<char>(2104 & 0xFF);
    loginConfirm[1] = static_cast<char>((2104 >> 8) & 0xFF);
    loginConfirm[4] = 0x14;
    loginConfirm[5] = 1;
    loginConfirm[6] = 0x34;
    loginConfirm[7] = 0x12;
    int loginCalls = 0;
    const auto receiveLogin = [&](const PacketView& view) {
        ++loginCalls;
        check(view.data == loginConfirm.data() && view.size == 2104,
            "login confirm preserva frame completo");
    };
    for (std::size_t n = 0; n < 2104; n += 17)
        check(!received_packet::Dispatch({0x114, loginConfirm.data(), n}, receiveLogin),
            "login confirm rejeita prefixos truncados");
    check(!received_packet::Dispatch({0x114, loginConfirm.data(), 2105}, receiveLogin),
        "login confirm excedente rejeitado");
    check(!received_packet::Dispatch({0x114, nullptr, 2104}, receiveLogin),
        "login confirm nulo rejeitado");
    check(!received_packet::Dispatch({0x119, loginConfirm.data(), 2104}, receiveLogin),
        "login confirm Type nao pode ser ocultado");
    loginConfirm[4] = 0x19;
    check(!received_packet::Dispatch({0x114, loginConfirm.data(), 2104}, receiveLogin),
        "login confirm Type divergente rejeitado");
    loginConfirm[4] = 0x14;
    loginConfirm[0] = static_cast<char>(2103 & 0xFF);
    loginConfirm[1] = static_cast<char>((2103 >> 8) & 0xFF);
    check(!received_packet::Dispatch({0x114, loginConfirm.data(), 2104}, receiveLogin),
        "login confirm Size divergente rejeitado");
    loginConfirm[0] = static_cast<char>(2104 & 0xFF);
    loginConfirm[1] = static_cast<char>((2104 >> 8) & 0xFF);
    check(loginCalls == 0, "login confirm invalido nao chega ao callback");
    const auto loginBefore = loginConfirm;
    check(received_packet::Dispatch({0x114, loginConfirm.data(), 2104}, receiveLogin) && loginCalls == 1,
        "login confirm valido entregue uma vez");
    check(loginConfirm == loginBefore, "login confirm preserva bytes do relogin");
    std::array<char, 25> arrayProbe{};
    arrayProbe[0] = 24;
    arrayProbe[4] = static_cast<char>(0xC1);
    arrayProbe[5] = 1;
    arrayProbe[12] = 99;
    arrayProbe[16] = static_cast<char>(0xFC);
    arrayProbe[17] = static_cast<char>(0xFF);
    arrayProbe[18] = static_cast<char>(0xFF);
    arrayProbe[19] = static_cast<char>(0xFF);
    int arrayCalls = 0;
    const auto receiveArray = [&](const PacketView& view) {
        ++arrayCalls;
        check(view.data == arrayProbe.data() && view.size == 24,
            "array probe preserva frame de 24 bytes");
    };
    for (std::size_t n = 0; n < 24; n += 3)
        check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), n}, receiveArray),
            "array probe truncado rejeitado");
    check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), 25}, receiveArray),
        "array probe excedente rejeitado");
    check(!received_packet::Dispatch({0x1C1, nullptr, 24}, receiveArray),
        "array probe nulo rejeitado");
    check(!received_packet::Dispatch({0x119, arrayProbe.data(), 24}, receiveArray),
        "array probe Type nao pode ser ocultado");
    arrayProbe[4] = 0x19;
    check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), 24}, receiveArray),
        "array probe Type divergente rejeitado");
    arrayProbe[4] = static_cast<char>(0xC1);
    arrayProbe[0] = 23;
    check(!received_packet::Dispatch({0x1C1, arrayProbe.data(), 24}, receiveArray),
        "array probe Size divergente rejeitado");
    arrayProbe[0] = 24;
    check(arrayCalls == 0, "array probe invalido nao chega ao callback");
    const auto arrayBefore = arrayProbe;
    check(received_packet::Dispatch({0x1C1, arrayProbe.data(), 24}, receiveArray) && arrayCalls == 1,
        "array probe valido entregue uma vez");
    MSG_REQArray decodedArray{};
    std::memcpy(&decodedArray, arrayProbe.data(), sizeof(decodedArray));
    check(decodedArray.Category == 99 && decodedArray.ByteOffset == -4,
        "array probe preserva categoria e offset signed");
    check(arrayProbe == arrayBefore, "array probe preserva bytes antes do handler");
    MSG_REQArray response = decodedArray;
    response.Header.Type = MSG_CNFArray_Opcode;
    response.Value = -128;
    check(response.Header.Type == 0x2C2 && response.Value == -128 && sizeof(response) == 24,
        "array response preserva opcode, signedness e tamanho");
    std::array<char, 17> delayStart{};
    delayStart[0] = 16;
    delayStart[4] = static_cast<char>(0xAE);
    delayStart[5] = 0x03;
    delayStart[6] = 0x34;
    delayStart[7] = 0x12;
    delayStart[12] = 1;
    int delayCalls = 0;
    const auto receiveDelay = [&](const PacketView& view) {
        ++delayCalls;
        check(view.data == delayStart.data() && view.size == 16 && view.opcode == 0x3AE,
            "DelayStart preserva frame de 16 bytes");
    };
    for (std::size_t n = 0; n < 16; ++n)
        check(!received_packet::Dispatch({0x3AE, delayStart.data(), n}, receiveDelay),
            "DelayStart truncado rejeitado");
    check(!received_packet::Dispatch({0x3AE, delayStart.data(), 17}, receiveDelay),
        "DelayStart excedente rejeitado");
    check(!received_packet::Dispatch({0x3AE, nullptr, 16}, receiveDelay),
        "DelayStart nulo rejeitado");
    check(!received_packet::Dispatch({0x119, delayStart.data(), 16}, receiveDelay),
        "Type DelayStart nao pode ser ocultado");
    delayStart[4] = 0x19;
    check(!received_packet::Dispatch({0x3AE, delayStart.data(), 16}, receiveDelay),
        "Type DelayStart divergente rejeitado");
    delayStart[4] = static_cast<char>(0xAE);
    delayStart[0] = 15;
    check(!received_packet::Dispatch({0x3AE, delayStart.data(), 16}, receiveDelay),
        "Size DelayStart divergente rejeitado");
    delayStart[0] = 16;
    check(delayCalls == 0, "DelayStart invalido nao chega ao consumidor");
    const auto delayBefore = delayStart;
    check(received_packet::Dispatch({0x3AE, delayStart.data(), 16}, receiveDelay) && delayCalls == 1,
        "DelayStart valido entregue uma vez");
    MSG_DelayStart decodedDelay{};
    std::memcpy(&decodedDelay, delayStart.data(), sizeof(decodedDelay));
    check(decodedDelay.Header.ID == 0x1234 && decodedDelay.Parm == 1,
        "DelayStart preserva ID e parametro de transicao");
    MSG_SysQuit sysQuit{};
    sysQuit.Header.Type = MSG_SysQuit_Opcode;
    sysQuit.Header.Size = sizeof(sysQuit);
    sysQuit.Header.ID = 0x1234;
    check(sizeof(sysQuit) == 16 && sysQuit.Header.Type == 0x3AE && sysQuit.Parm == 0,
        "SysQuit compartilha ABI e preserva Parm zero");
    check(delayStart == delayBefore, "gate preserva todos os bytes de DelayStart");
    std::array<char, 17> billing{};
    billing[0] = 16;
    billing[4] = static_cast<char>(0x94);
    billing[5] = 1;
    billing[12] = static_cast<char>(0xA5);
    billing[13] = static_cast<char>(0x5A);
    const auto billingBefore = billing;
    int billingCalls = 0;
    const auto receiveBilling = [&](const PacketView& view) {
        ++billingCalls;
        check(view.data == billing.data() && view.size == 16 && view.opcode == 0x194,
            "BillingNotice preserva frame opaco de 16 bytes");
    };
    for (std::size_t n = 0; n < 16; ++n)
        check(!received_packet::Dispatch({0x194, billing.data(), n}, receiveBilling),
            "BillingNotice truncado rejeitado");
    check(!received_packet::Dispatch({0x194, billing.data(), 17}, receiveBilling),
        "BillingNotice excedente rejeitado");
    check(!received_packet::Dispatch({0x194, nullptr, 16}, receiveBilling),
        "BillingNotice nulo rejeitado");
    check(!received_packet::Dispatch({0x119, billing.data(), 16}, receiveBilling),
        "Type BillingNotice nao pode ser ocultado");
    billing[4] = 0x19;
    check(!received_packet::Dispatch({0x194, billing.data(), 16}, receiveBilling),
        "Type BillingNotice divergente rejeitado");
    billing[4] = static_cast<char>(0x94);
    billing[0] = 15;
    check(!received_packet::Dispatch({0x194, billing.data(), 16}, receiveBilling),
        "Size BillingNotice divergente rejeitado");
    billing[0] = 16;
    check(billingCalls == 0, "BillingNotice invalido nao chega ao consumidor");
    check(received_packet::Dispatch({0x194, billing.data(), 16}, receiveBilling) && billingCalls == 1,
        "BillingNotice valido entregue uma vez");
    MSG_BillingNotice decodedBilling{};
    std::memcpy(&decodedBilling, billing.data(), sizeof(decodedBilling));
    check(decodedBilling.Header.Type == 0x194 && decodedBilling.OpaquePayload[0] == 0xA5 &&
        decodedBilling.OpaquePayload[1] == 0x5A,
        "BillingNotice preserva opcode e payload opaco");
    check(billing == billingBefore, "gate preserva todos os bytes de BillingNotice");
    std::array<char, 21> motion{};
    motion[0] = 20;
    motion[4] = 0x6A;
    motion[5] = 0x03;
    motion[6] = 0x34;
    motion[7] = 0x12;
    motion[12] = 100;
    motion[14] = 5;
    motion[18] = static_cast<char>(0x80);
    motion[19] = 0x3F;
    int motionCalls = 0;
    const auto receiveMotion = [&](const PacketView& view) {
        ++motionCalls;
        check(view.data == motion.data() && view.size == 20 && view.opcode == 0x36A,
            "Motion preserva frame de 20 bytes");
    };
    for (std::size_t n = 0; n < 20; ++n)
        check(!received_packet::Dispatch({0x36A, motion.data(), n}, receiveMotion),
            "Motion truncado rejeitado");
    check(!received_packet::Dispatch({0x36A, motion.data(), 21}, receiveMotion),
        "Motion excedente rejeitado");
    check(!received_packet::Dispatch({0x36A, nullptr, 20}, receiveMotion),
        "Motion nulo rejeitado");
    check(!received_packet::Dispatch({0x119, motion.data(), 20}, receiveMotion),
        "Type Motion nao pode ser ocultado");
    motion[4] = 0x19;
    check(!received_packet::Dispatch({0x36A, motion.data(), 20}, receiveMotion),
        "Type Motion divergente rejeitado");
    motion[4] = 0x6A;
    motion[0] = 19;
    check(!received_packet::Dispatch({0x36A, motion.data(), 20}, receiveMotion),
        "Size Motion divergente rejeitado");
    motion[0] = 20;
    check(motionCalls == 0, "Motion invalido nao chega ao consumidor");
    const auto motionBefore = motion;
    check(received_packet::Dispatch({0x36A, motion.data(), 20}, receiveMotion) && motionCalls == 1,
        "Motion valido entregue uma vez");
    MSG_Motion decodedMotion{};
    std::memcpy(&decodedMotion, motion.data(), sizeof(decodedMotion));
    check(decodedMotion.Header.ID == 0x1234 && decodedMotion.Motion == 100 &&
        decodedMotion.Parm == 5 && decodedMotion.Direction == 1.0f,
        "Motion preserva ID, motion, parametro e direcao");
    check(motion == motionBefore, "gate preserva todos os bytes de Motion");

    std::array<char, 41> partyAdd{};
    partyAdd[0] = 40;
    partyAdd[4] = static_cast<char>(0x7D);
    partyAdd[5] = 0x03;
    partyAdd[6] = 0x34;
    partyAdd[7] = 0x12;
    partyAdd[12] = 2;
    partyAdd[13] = 1;
    partyAdd[14] = 55;
    partyAdd[16] = 0x20;
    partyAdd[18] = 0x10;
    partyAdd[20] = 0x34;
    partyAdd[21] = 0x12;
    std::memcpy(partyAdd.data() + 22, "Membro", 7);
    int partyCalls = 0;
    const auto receiveParty = [&](const PacketView& view) {
        ++partyCalls;
        check(view.data == partyAdd.data() && view.size == 40 && view.opcode == 0x37D,
            "PartyAdd preserva frame de 40 bytes");
    };
    for (std::size_t n = 0; n < 40; ++n)
        check(!received_packet::Dispatch({0x37D, partyAdd.data(), n}, receiveParty),
            "PartyAdd truncado rejeitado");
    check(!received_packet::Dispatch({0x37D, partyAdd.data(), 41}, receiveParty),
        "PartyAdd excedente rejeitado");
    check(!received_packet::Dispatch({0x37D, nullptr, 40}, receiveParty),
        "PartyAdd nulo rejeitado");
    check(!received_packet::Dispatch({0x119, partyAdd.data(), 40}, receiveParty),
        "Type PartyAdd nao pode ser ocultado");
    partyAdd[4] = 0x19;
    check(!received_packet::Dispatch({0x37D, partyAdd.data(), 40}, receiveParty),
        "Type PartyAdd divergente rejeitado");
    partyAdd[4] = static_cast<char>(0x7D);
    partyAdd[0] = 39;
    check(!received_packet::Dispatch({0x37D, partyAdd.data(), 40}, receiveParty),
        "Size PartyAdd divergente rejeitado");
    partyAdd[0] = 40;
    check(partyCalls == 0, "PartyAdd invalido nao chega ao consumidor");
    const auto partyBefore = partyAdd;
    check(received_packet::Dispatch({0x37D, partyAdd.data(), 40}, receiveParty) && partyCalls == 1,
        "PartyAdd valido entregue uma vez");
    MSG_AddParty decodedParty{};
    std::memcpy(&decodedParty, partyAdd.data(), sizeof(decodedParty));
    check(decodedParty.Header.ID == 0x1234 && decodedParty.Party.ID == 0x1234 &&
        decodedParty.Party.Level == 55 && std::strcmp(decodedParty.Party.Name, "Membro") == 0,
        "PartyAdd preserva ID, nivel e nome do membro");
    check(partyAdd == partyBefore, "gate preserva todos os bytes de PartyAdd");
    std::array<char, 17> partyRemove{};
    partyRemove[0] = 16;
    partyRemove[4] = static_cast<char>(0x7E);
    partyRemove[5] = 0x03;
    partyRemove[6] = 0x34;
    partyRemove[7] = 0x12;
    partyRemove[12] = 0x78;
    partyRemove[13] = 0x56;
    int removeCalls = 0;
    const auto receivePartyRemove = [&](const PacketView& view) {
        ++removeCalls;
        check(view.data == partyRemove.data() && view.size == 16 && view.opcode == 0x37E,
            "PartyRemove preserva frame de 16 bytes");
    };
    for (std::size_t n = 0; n < 16; ++n)
        check(!received_packet::Dispatch({0x37E, partyRemove.data(), n}, receivePartyRemove),
            "PartyRemove truncado rejeitado");
    check(!received_packet::Dispatch({0x37E, partyRemove.data(), 17}, receivePartyRemove),
        "PartyRemove excedente rejeitado");
    check(!received_packet::Dispatch({0x37E, nullptr, 16}, receivePartyRemove),
        "PartyRemove nulo rejeitado");
    check(!received_packet::Dispatch({0x119, partyRemove.data(), 16}, receivePartyRemove),
        "Type PartyRemove nao pode ser ocultado");
    partyRemove[4] = 0x19;
    check(!received_packet::Dispatch({0x37E, partyRemove.data(), 16}, receivePartyRemove),
        "Type PartyRemove divergente rejeitado");
    partyRemove[4] = static_cast<char>(0x7E);
    partyRemove[0] = 15;
    check(!received_packet::Dispatch({0x37E, partyRemove.data(), 16}, receivePartyRemove),
        "Size PartyRemove divergente rejeitado");
    partyRemove[0] = 16;
    check(removeCalls == 0, "PartyRemove invalido nao chega ao consumidor");
    const auto removeBefore = partyRemove;
    check(received_packet::Dispatch({0x37E, partyRemove.data(), 16}, receivePartyRemove) && removeCalls == 1,
        "PartyRemove valido entregue uma vez");
    MSG_RemoveParty decodedRemove{};
    std::memcpy(&decodedRemove, partyRemove.data(), sizeof(decodedRemove));
    check(decodedRemove.Header.ID == 0x1234 && decodedRemove.Parm == 0x5678,
        "PartyRemove preserva remetente e membro removido");
    MSG_RemoveParty clearParty{};
    clearParty.Header.Type = MSG_RemoveParty_Opcode;
    clearParty.Header.Size = sizeof(clearParty);
    check(clearParty.Parm == 0, "PartyRemove preserva zero como limpeza do grupo");
    check(partyRemove == removeBefore, "gate preserva todos os bytes de PartyRemove");
    std::array<char, 45> partyRequest{};
    partyRequest[0] = 44;
    partyRequest[4] = static_cast<char>(0x7F);
    partyRequest[5] = 0x03;
    partyRequest[6] = 0x34;
    partyRequest[7] = 0x12;
    partyRequest[12] = 3;
    partyRequest[14] = 80;
    partyRequest[16] = static_cast<char>(0xE8);
    partyRequest[17] = 0x03;
    partyRequest[18] = static_cast<char>(0xBC);
    partyRequest[19] = 0x02;
    partyRequest[20] = 0x34;
    partyRequest[21] = 0x12;
    std::memcpy(partyRequest.data() + 22, "Lider", 6);
    partyRequest[40] = 0x78;
    partyRequest[41] = 0x56;
    int requestCalls = 0;
    const auto receivePartyRequest = [&](const PacketView& view) {
        ++requestCalls;
        check(view.data == partyRequest.data() && view.size == 44 && view.opcode == 0x37F,
            "PartyRequest preserva frame de 44 bytes");
    };
    for (std::size_t n = 0; n < 44; ++n)
        check(!received_packet::Dispatch({0x37F, partyRequest.data(), n}, receivePartyRequest),
            "PartyRequest truncado rejeitado");
    check(!received_packet::Dispatch({0x37F, partyRequest.data(), 45}, receivePartyRequest),
        "PartyRequest excedente rejeitado");
    check(!received_packet::Dispatch({0x37F, nullptr, 44}, receivePartyRequest),
        "PartyRequest nulo rejeitado");
    check(!received_packet::Dispatch({0x119, partyRequest.data(), 44}, receivePartyRequest),
        "Type PartyRequest nao pode ser ocultado");
    partyRequest[4] = 0x19;
    check(!received_packet::Dispatch({0x37F, partyRequest.data(), 44}, receivePartyRequest),
        "Type PartyRequest divergente rejeitado");
    partyRequest[4] = static_cast<char>(0x7F);
    partyRequest[0] = 43;
    check(!received_packet::Dispatch({0x37F, partyRequest.data(), 44}, receivePartyRequest),
        "Size PartyRequest divergente rejeitado");
    partyRequest[0] = 44;
    check(requestCalls == 0, "PartyRequest invalido nao chega ao consumidor");
    const auto requestBefore = partyRequest;
    check(received_packet::Dispatch({0x37F, partyRequest.data(), 44}, receivePartyRequest) && requestCalls == 1,
        "PartyRequest valido entregue uma vez");
    MSG_REQParty decodedRequest{};
    std::memcpy(&decodedRequest, partyRequest.data(), sizeof(decodedRequest));
    check(decodedRequest.Header.ID == 0x1234 && decodedRequest.Leader.ID == 0x1234 &&
        decodedRequest.Leader.Level == 80 && decodedRequest.Leader.MaxHp == 1000 &&
        decodedRequest.Leader.Hp == 700 && std::strcmp(decodedRequest.Leader.Name, "Lider") == 0 &&
        decodedRequest.TargetID == 0x5678,
        "PartyRequest preserva lider, HP e destino");
    check(partyRequest == requestBefore, "gate preserva todos os bytes de PartyRequest");
    MSG_BuyToto toto{};
    toto.Header.Type = MSG_BuyToto_Opcode;
    toto.Header.Size = sizeof(toto);
    toto.TargetID = 7;
    toto.TargetCarryPos = 14;
    toto.MyCarryPos = 3;
    toto.Coin = 0x7FFFFFFF;
    toto.Gindex = 80;
    toto.A_Score = 127;
    toto.B_Score = 0;
    check(sizeof(toto) == 36 && toto.TargetID == 7 && toto.MyCarryPos == 3 &&
        toto.Header.Type == 0x3CE && toto.Header.Size == 36,
        "TOTO preserva layout e campos de intencao C-S");
    MSG_ApplyBonus applyBonus{};
    applyBonus.Header.Type = MSG_ApplyBonus_Opcode;
    applyBonus.Header.Size = sizeof(applyBonus);
    applyBonus.BonusType = 2;
    applyBonus.Detail = 5000;
    applyBonus.TargetID = 42;
    check(sizeof(applyBonus) == 20 && applyBonus.BonusType == 2 &&
        applyBonus.Detail == 5000 && applyBonus.TargetID == 42 &&
        applyBonus.Header.Type == 0x277 && applyBonus.Header.Size == 20,
        "ApplyBonus preserva tipo, detalhe, mestre e tamanho");
    MSG_UseItem useItem{};
    useItem.Header.Type = MSG_UseItem_Opcode;
    useItem.Header.Size = sizeof(useItem);
    useItem.SourType = 1;
    useItem.SourPos = 17;
    useItem.DestType = 0;
    useItem.DestPos = 0;
    useItem.GridX = 8;
    useItem.GridY = 6;
    useItem.ItemID = 3377;
    check(sizeof(useItem) == 36 && useItem.SourType == 1 && useItem.SourPos == 17 &&
        useItem.GridX == 8 && useItem.GridY == 6 && useItem.ItemID == 3377 &&
        useItem.Header.Type == 0x373 && useItem.Header.Size == 36,
        "UseItem preserva origem, celula, item e tamanho");
    MSG_SetPKMode pk{};
    pk.Header.Type = MSG_SetPKMode_Opcode;
    pk.Header.Size = sizeof(pk);
    pk.Header.ID = 0x1234;
    pk.Parm = 1;
    check(sizeof(pk) == 16 && pk.Header.Type == 0x399 && pk.Header.Size == 16 &&
        pk.Header.ID == 0x1234 && (pk.Parm == 0 || pk.Parm == 1),
        "PK mode preserva identidade, dominio e tamanho");
    MSG_PremiumFirework firework{};
    firework.Header.Type = MSG_PremiumFirework_Opcode;
    firework.Header.Size = sizeof(firework);
    firework.Header.ID = 77;
    firework.Bitmap[0] = 1;
    firework.Bitmap[12] = static_cast<char>(0x80);
    check(sizeof(firework) == 36 && firework.Header.Type == 0x3CA &&
        firework.Header.Size == 36 && firework.Header.ID == 77 &&
        firework.Bitmap[0] == 1 && static_cast<unsigned char>(firework.Bitmap[12]) == 0x80,
        "premium firework preserva ID, reserva, bitmap e tamanho");
    MSG_UseItem2 fireworkUse{};
    fireworkUse.Header.Type = MSG_UseItem2_Opcode;
    fireworkUse.Header.Size = sizeof(fireworkUse);
    fireworkUse.SourType = 1;
    fireworkUse.SourPos = 62;
    fireworkUse.GridX = 2100;
    fireworkUse.GridY = 2101;
    fireworkUse.Parm[0] = 1;
    fireworkUse.Parm[12] = 0x0F;
    check(sizeof(fireworkUse) == 52 && fireworkUse.Header.Type == 0x3C9 &&
        fireworkUse.Header.Size == 52 && fireworkUse.SourType == 1 &&
        fireworkUse.SourPos == 62 && fireworkUse.GridX == 2100 &&
        fireworkUse.GridY == 2101 && fireworkUse.Parm[0] == 1 &&
        fireworkUse.Parm[12] == 0x0F,
        "premium firework request preserva slot, posicao, bitmap e tamanho");
    MSG_DoJackpotBet gamble{};
    gamble.Header.Type = MSG_DoJackpotBet_Opcode;
    gamble.Header.Size = sizeof(gamble);
    gamble.GambleType = 2;
    gamble.Bet = 100000;
    MSG_ResultGamble result{};
    result.Header.Type = MSG_ResultGamble_Opcode;
    result.Header.Size = sizeof(result);
    result.Result[0] = 14;
    result.StopPosition[2] = 9;
    result.Prize = -7;
    result.Jackpot = 0x89ABCDEFu;
    check(sizeof(gamble) == 20 && gamble.Header.Type == 0x2BE && gamble.Bet == 100000 &&
        sizeof(result) == 36 && result.Header.Type == 0x1BF && result.Prize == -7 &&
        result.Jackpot == 0x89ABCDEFu,
        "Gamble preserva aposta, resultado, premio, jackpot e tamanhos");
    std::array<char, 25> mobKill{};
    mobKill[0] = 24;
    mobKill[4] = 0x38;
    mobKill[5] = 3;
    mobKill[12] = static_cast<char>(0xD2);
    mobKill[13] = 0x04;
    mobKill[16] = 0x34;
    mobKill[17] = 0x12;
    mobKill[18] = 0x78;
    mobKill[19] = 0x56;
    mobKill[20] = 0x44;
    mobKill[21] = 0x33;
    mobKill[22] = 0x22;
    mobKill[23] = 0x11;
    int mobKillCalls = 0;
    const auto receiveMobKill = [&](const PacketView& view) {
        ++mobKillCalls;
        check(view.data == mobKill.data() && view.size == 24,
            "mob-kill preserva frame de 24 bytes");
    };
    for (std::size_t n = 0; n < 24; n += 3)
        check(!received_packet::Dispatch({0x338, mobKill.data(), n}, receiveMobKill),
            "mob-kill truncado rejeitado");
    check(!received_packet::Dispatch({0x338, mobKill.data(), 25}, receiveMobKill),
        "mob-kill excedente rejeitado");
    check(!received_packet::Dispatch({0x338, nullptr, 24}, receiveMobKill),
        "mob-kill nulo rejeitado");
    mobKill[0] = 23;
    check(!received_packet::Dispatch({0x338, mobKill.data(), 24}, receiveMobKill),
        "mob-kill Size divergente rejeitado");
    mobKill[0] = 24;
    check(received_packet::Dispatch({0x338, mobKill.data(), 24}, receiveMobKill) && mobKillCalls == 1,
        "mob-kill valido entregue uma vez");
    MSG_CNFMobKill decodedKill{};
    std::memcpy(&decodedKill, mobKill.data(), sizeof(decodedKill));
    check(decodedKill.FakeExp == 1234 && decodedKill.KilledMob == 0x1234 &&
        decodedKill.Killer == 0x5678 && decodedKill.Exp == 0x11223344u,
        "mob-kill preserva Hold, IDs e EXP uint32");
    std::array<char, 37> updateEtc{};
    updateEtc[0] = 36;
    updateEtc[4] = 0x37;
    updateEtc[5] = 3;
    updateEtc[12] = static_cast<char>(0xD2);
    updateEtc[13] = 0x04;
    updateEtc[16] = 0x44;
    updateEtc[17] = 0x33;
    updateEtc[18] = 0x22;
    updateEtc[19] = 0x11;
    updateEtc[20] = 0x08;
    updateEtc[24] = 1;
    updateEtc[26] = 2;
    updateEtc[28] = 3;
    updateEtc[30] = 4;
    updateEtc[32] = 5;
    int updateEtcCalls = 0;
    const auto receiveUpdateEtc = [&](const PacketView& view) {
        ++updateEtcCalls;
        check(view.data == updateEtc.data() && view.size == 36,
            "UpdateEtc preserva frame compacto");
    };
    for (std::size_t n = 0; n < 36; n += 4)
        check(!received_packet::Dispatch({0x337, updateEtc.data(), n}, receiveUpdateEtc),
            "UpdateEtc truncado rejeitado");
    check(!received_packet::Dispatch({0x337, updateEtc.data(), 37}, receiveUpdateEtc),
        "UpdateEtc excedente rejeitado");
    check(!received_packet::Dispatch({0x337, nullptr, 36}, receiveUpdateEtc),
        "UpdateEtc nulo rejeitado");
    updateEtc[0] = 35;
    check(!received_packet::Dispatch({0x337, updateEtc.data(), 36}, receiveUpdateEtc),
        "UpdateEtc Size divergente rejeitado");
    updateEtc[0] = 36;
    check(received_packet::Dispatch({0x337, updateEtc.data(), 36}, receiveUpdateEtc) && updateEtcCalls == 1,
        "UpdateEtc valido entregue uma vez");
    MSG_UpdateEtc decodedEtc{};
    std::memcpy(&decodedEtc, updateEtc.data(), sizeof(decodedEtc));
    check(decodedEtc.Hold == 1234 && decodedEtc.Exp == 0x11223344u &&
        decodedEtc.LearnedSkill == 8 && decodedEtc.StatusPoint == 1 &&
        decodedEtc.MasterPoint == 2 && decodedEtc.SkillPoint == 3 &&
        decodedEtc.Magic == 4 && decodedEtc.Coin == 5,
        "UpdateEtc preserva Hold, EXP, skill, pontos e gold");
    return failures;
}
