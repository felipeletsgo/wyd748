#include "../internal/wire/ReceivedPacketDispatch.h"
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
    return failures;
}
