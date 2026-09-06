#include "../internal/application/ports/PacketView.h"
#include "../internal/application/ports/PacketDispatch.h"
#include "../internal/wire/PacketSendBoundary.h"
#include "../internal/platform/windows/SocketTransport.h"
#include "../internal/application/RequestCharacterLogin.h"
#include "../internal/wire/CharacterLoginSender.h"
#include "../internal/wire/PartyAcceptPacket.h"
#include "../internal/wire/MissingEntityRequestPacket.h"
#include "../internal/wire/RestartRecallPacket.h"
#include "../internal/wire/KeepalivePingPacket.h"
#include "../internal/wire/ChangeCityPacket.h"
#include "../internal/wire/ReqTeleportPacket.h"
#include <array>
#include <cstring>
#include <type_traits>
#include <climits>
#include <cstdio>
#include <limits>
#include "../internal/platform/network/SendBuffer.h"
#include "../internal/platform/network/ReceiveBuffer.h"

// Suite isolada de application, compilada sem os includes wire deste runner.
int RunCharacterLoginUseCaseTests(int& checks);
int RunReceivedPacketDispatchTests(int& checks);
int RunCargoSlotTests(int& checks);
int RunGridInsertionTests(int& checks);

// Backend sem socket: registra metadados e usa o mesmo guard da producao.
// Nao retém o buffer; a mutacao simula o preenchimento sincrono do cabecalho.
struct FakeSocket
{
    int calls = 0;
    int accepted = 0;
    bool result = true;
    unsigned int opcode = 0;
    std::size_t size = 0;
    int SendPacket(const MutablePacketView& packet)
    {
        ++calls;
        opcode = packet.opcode;
        size = packet.size;
        return SendValidatedPacket(packet, 12, [&](char* data, int) {
            ++accepted;
            data[1] = 23;
            return result;
        });
    }
};

// Espiao da porta: copia apenas para inspecao do teste, nunca retem o ponteiro
// temporario do caso de uso. Nenhum socket e necessario para conferir o wire.
struct RecordingTransport final : ITransport
{
    int calls = 0;
    bool result = true;
    unsigned int opcode = 0;
    std::size_t size = 0;
    std::array<char, 36> bytes{};
    bool Send(const MutablePacketView& packet) override
    {
        ++calls;
        opcode = packet.opcode;
        size = packet.size;
        if (!packet.data || packet.size != bytes.size()) return false;
        std::memcpy(bytes.data(), packet.data, bytes.size());
        return result;
    }
};

// Testes locais da fronteira de tamanho, sem socket, Win32 ou DirectX.
// Nao usam assert: as verificacoes devem permanecer ativas em Release.
int main()
{
    char storage[16] = {};
    int failures = 0;
    int checks = 0;
    const auto check = [&failures, &checks](bool condition, const char* name) {
        ++checks;
        if (!condition) {
            std::fprintf(stderr, "FAIL: %s\n", name);
            ++failures;
        }
    };

    check(!PacketView{}.HasAtLeast(0), "view vazia nao possui buffer");
    check(!PacketView{0, nullptr, 16}.HasSizeBetween(12, INT_MAX), "nulo com tamanho");
    check(!PacketView{0, storage, 0}.HasSizeBetween(12, INT_MAX), "tamanho zero");
    check(!PacketView{0, storage, 11}.HasSizeBetween(12, INT_MAX), "abaixo do minimo");
    check(PacketView{0, storage, 12}.HasSizeBetween(12, INT_MAX), "minimo inclusivo");
    check(PacketView{0, storage, 16}.HasSizeBetween(12, 16), "maximo inclusivo");
    check(!PacketView{0, storage, 16}.HasSizeBetween(12, 15), "acima do maximo");
    check(!PacketView{0, storage, 16}.HasSizeBetween(17, 16), "intervalo invertido");

    // Comprimentos sinteticos: o predicado nunca pode desreferenciar data.
    const auto intOverflow = static_cast<std::size_t>(INT_MAX) + 1;
    check(!PacketView{0, storage, intOverflow}.HasSizeBetween(12, INT_MAX), "overflow int");
    check(PacketView{0, storage, INT_MAX}.HasSizeBetween(12, INT_MAX), "limite int");
    const auto sizeMaximum = (std::numeric_limits<std::size_t>::max)();
    check(!PacketView{0, storage, sizeMaximum}.HasSizeBetween(12, INT_MAX), "limite size_t");

    const PacketView valid{77, storage, sizeof(storage)};
    check(valid.HasSizeBetween(12, INT_MAX) && valid.opcode == 77 &&
        valid.data == storage && storage[0] == 0, "validacao sem mutacao");
    // O compilador impede converter uma recepcao somente leitura em envio.
    static_assert(!std::is_convertible<PacketView, MutablePacketView>::value,
        "Recepcao nao pode remover const implicitamente");
    static_assert(std::is_same<decltype(MutablePacketView::data), char*>::value,
        "Envio exige armazenamento gravavel");
    MutablePacketView outgoing{77, storage, sizeof(storage)};
    const auto incoming = outgoing.AsReadOnly();
    check(incoming.data == storage && incoming.size == sizeof(storage) &&
        incoming.opcode == 77, "leitura empresta mesmo buffer");

    int sends = 0;
    const auto fakeSender = [&](char* data, int size) {
        ++sends;
        check(data == storage && size == sizeof(storage), "emissor recebe buffer e tamanho originais");
        data[0] = 42;
        return true;
    };
    check(SendValidatedPacket(outgoing, 12, fakeSender), "resultado de sucesso preservado");
    check(sends == 1 && storage[0] == 42, "envio unico e mutacao visivel");
    check(!SendValidatedPacket({77, nullptr, 16}, 12, fakeSender), "envio nulo rejeitado");
    check(!SendValidatedPacket({77, storage, 11}, 12, fakeSender), "envio curto rejeitado");
    check(!SendValidatedPacket({77, storage, intOverflow}, 12, fakeSender), "envio overflow rejeitado");
    check(sends == 1, "rejeicoes nao invocam emissor");
    check(!SendValidatedPacket(outgoing, 12, [&](char*, int) {
        ++sends;
        return false;
    }), "falha do emissor propagada");
    check(sends == 2, "falha nao causa retry implicito");
    // A chamada virtual exercita a porta real, sem incluir Basedef ou Win32.
    FakeSocket socket;
    {
        SocketTransport<FakeSocket> adapter(socket);
        ITransport& transport = adapter;
        check(transport.Send(outgoing), "porta propaga sucesso");
        check(socket.calls == 1 && socket.accepted == 1, "adaptador envia uma vez");
        check(socket.opcode == 77 && socket.size == sizeof(storage), "adaptador preserva metadados");
        check(storage[1] == 23, "adaptador preserva mutacao no buffer original");
        socket.result = false;
        check(!transport.Send(outgoing), "porta propaga falha");
        check(socket.calls == 2, "adaptador nao repete falha");
        check(!transport.Send({77, nullptr, 16}), "porta rejeita nulo no backend");
        check(!transport.Send({77, storage, 11}), "porta rejeita tamanho curto no backend");
        check(!transport.Send({77, storage, intOverflow}), "porta rejeita overflow no backend");
        check(socket.accepted == 2 && socket.calls == 5, "rejeicoes nao chegam ao emissor");
    }
    check(socket.calls == 5, "destruir adaptador nao fecha nem envia pelo backend");
    RecordingTransport login;
    CharacterLoginSender loginSender(login);
    for (int slot = 0; slot < 4; ++slot)
    {
        check(RequestCharacterLogin(loginSender, slot), "login aceita slot valido");
        check(login.calls == slot + 1, "login solicita envio unico");
        check(login.opcode == 0x213 && login.size == 36, "login preserva opcode e tamanho");
        // Expectativa independente do struct: todos os bytes zerados salvo
        // opcode little-endian em +4 e slot em +12, antes do enquadramento.
        std::array<char, 36> expected{};
        expected[4] = 0x13;
        expected[5] = 0x02;
        expected[12] = static_cast<char>(slot);
        check(login.bytes == expected, "login preserva os 36 bytes anteriores ao envio");
    }
    check(!RequestCharacterLogin(loginSender, -1), "login rejeita slot negativo");
    check(!RequestCharacterLogin(loginSender, 4), "login rejeita slot fora do limite");
    check(!RequestCharacterLogin(loginSender, INT_MAX), "login rejeita indice extremo");
    check(login.calls == 4, "login invalido nao envia");
    login.result = false;
    check(!RequestCharacterLogin(loginSender, 0), "login propaga falha do transporte");
    check(login.calls == 5, "login nao repete envio apos falha");

    // Fixture independente do dispatcher: 0x3AB existe apenas no sentido C->S.
    MSG_CNFParty2 partyAccept{};
    partyAccept.Header.Type = MSG_CNFParty2_Opcode;
    partyAccept.Header.ID = 0x1234;
    partyAccept.LeaderID = 0x0234;
    std::memcpy(partyAccept.LeaderName, "PartyLeader", 11);
    const auto* partyAcceptBytes = reinterpret_cast<const unsigned char*>(&partyAccept);
    check(sizeof(partyAccept) == 32 && partyAcceptBytes[4] == 0xAB &&
        partyAcceptBytes[5] == 0x03, "PartyAccept preserva opcode e frame de 32 bytes");
    check(partyAcceptBytes[12] == 0x34 && partyAcceptBytes[13] == 0x02 &&
        std::memcmp(partyAcceptBytes + 14, "PartyLeader", 11) == 0,
        "PartyAccept preserva lider e nome nos offsets nativos");
    check(partyAcceptBytes[25] == 0 && partyAcceptBytes[30] == 0 &&
        partyAcceptBytes[31] == 0, "PartyAccept zera terminador e WORD reservado");

    MSG_REQMobByID missingEntity{};
    missingEntity.Header.Type = MSG_REQMobByID_Opcode;
    missingEntity.Header.ID = 0x1234;
    missingEntity.MobID = 0x0234;
    const auto* missingEntityBytes = reinterpret_cast<const unsigned char*>(&missingEntity);
    check(sizeof(missingEntity) == 16 && missingEntityBytes[4] == 0x69 &&
        missingEntityBytes[5] == 0x03 && missingEntityBytes[12] == 0x34 &&
        missingEntityBytes[13] == 0x02,
        "MissingEntity preserva opcode, tamanho e MobID");
    check(missingEntityBytes[14] == 0 && missingEntityBytes[15] == 0,
        "MissingEntity zera WORD reservado");

    MSG_STANDARD restartRecall{};
    restartRecall.Type = MSG_Recall_Opcode;
    restartRecall.ID = 0x1234;
    const auto* restartRecallBytes = reinterpret_cast<const unsigned char*>(&restartRecall);
    check(sizeof(restartRecall) == 12 && restartRecallBytes[4] == 0x89 &&
        restartRecallBytes[5] == 0x02 && restartRecallBytes[6] == 0x34 &&
        restartRecallBytes[7] == 0x12,
        "RestartRecall preserva opcode, tamanho e ID");

    MSG_STANDARD fieldPing{};
    fieldPing.Type = MSG_Ping_Opcode;
    fieldPing.ID = 0x1234;
    const auto* fieldPingBytes = reinterpret_cast<const unsigned char*>(&fieldPing);
    check(sizeof(fieldPing) == 12 && fieldPingBytes[4] == 0xA0 &&
        fieldPingBytes[5] == 0x03 && fieldPingBytes[6] == 0x34 &&
        fieldPingBytes[7] == 0x12,
        "Keepalive Field preserva opcode, tamanho e ID local");
    MSG_STANDARD selectCharPing{};
    selectCharPing.Type = MSG_Ping_Opcode;
    check(selectCharPing.ID == 0 && sizeof(selectCharPing) == 12,
        "Keepalive SelectChar preserva ID zero e header puro");

    MSG_ChangeCity changeCity{};
    changeCity.Header.Type = MSG_ChangeCity_Opcode;
    changeCity.Header.ID = 0x1234;
    changeCity.Village = 3;
    const auto* changeCityBytes = reinterpret_cast<const unsigned char*>(&changeCity);
    check(sizeof(changeCity) == 16 && changeCityBytes[4] == 0x91 &&
        changeCityBytes[5] == 0x02 && changeCityBytes[6] == 0x34 &&
        changeCityBytes[7] == 0x12 && changeCityBytes[12] == 3,
        "ChangeCity preserva opcode, tamanho, ID e village");
    check(offsetof(MSG_ChangeCity, Village) == 12 && changeCityBytes[13] == 0 &&
        changeCityBytes[14] == 0 && changeCityBytes[15] == 0,
        "ChangeCity mantem Village DWORD no offset nativo");

    MSG_ReqTeleport teleport{};
    teleport.Header.Type = MSG_ReqTeleport_Opcode;
    teleport.Header.ID = 0x1234;
    const auto* teleportBytes = reinterpret_cast<const unsigned char*>(&teleport);
    check(sizeof(teleport) == 16 && teleportBytes[4] == 0x90 &&
        teleportBytes[5] == 0x02 && teleportBytes[6] == 0x34 &&
        teleportBytes[7] == 0x12 && teleport.Reserved == 0,
        "ReqTeleport preserva opcode, tamanho, ID e reservado zero");
    check(offsetof(MSG_ReqTeleport, Reserved) == 12 && teleportBytes[13] == 0 &&
        teleportBytes[14] == 0 && teleportBytes[15] == 0,
        "ReqTeleport mantem o payload reservado no offset nativo");

    failures += RunCharacterLoginUseCaseTests(checks);
    // Limites da fila sem soma signed e sem depender de um socket real.
    check(send_buffer::CanAppendPacket(12, 0, 131072, 12), "fila aceita cabecalho");
    check(send_buffer::CanAppendPacket(65535, 0, 131072, 12), "fila aceita limite WORD");
    check(!send_buffer::CanAppendPacket(65536, 0, 131072, 12), "fila rejeita truncamento WORD");
    check(!send_buffer::CanAppendPacket(INT_MAX, INT_MAX, 131072, 12), "fila rejeita overflow");
    check(!send_buffer::CanAppendPacket(-1, 0, 131072, 12), "fila rejeita tamanho negativo");
    check(!send_buffer::CanAppendPacket(11, 0, 131072, 12), "fila rejeita cabecalho curto");
    check(!send_buffer::CanAppendPacket(12, -1, 131072, 12), "fila rejeita indice negativo");
    check(!send_buffer::CanAppendPacket(12, 131060, 131072, 12), "fila preserva limite estrito");
    check(send_buffer::CanAppendPacket(12, 131059, 131072, 12), "fila aceita ultimo intervalo valido");
    check(send_buffer::CanAppendRaw(0, 131072, 131072), "raw aceita fila cheia vazia");
    check(receive_buffer::CanReadFrame(12, 12, 12), "recepcao aceita frame minimo");
    PacketView dispatchable{1, "1234", 4};
    check(packet_dispatch::CanDispatch(dispatchable, 4), "dispatch aceita view completa");
    check(!packet_dispatch::CanDispatch(dispatchable, 5), "dispatch rejeita view curta");
    // A politica entrega o mesmo emprestimo uma unica vez; frames rejeitados
    // nao chegam ao receptor. O callback pode observar todos os metadados.
    int dispatchCalls = 0;
    auto receiver = [&](const PacketView& frame) {
        ++dispatchCalls;
        check(frame.data == dispatchable.data && frame.size == dispatchable.size &&
            frame.opcode == dispatchable.opcode, "dispatch preserva emprestimo e metadados");
    };
    check(packet_dispatch::Dispatch(dispatchable, 4, receiver), "dispatch entrega frame completo");
    check(dispatchCalls == 1, "dispatch nao repete callback");
    check(!packet_dispatch::Dispatch(dispatchable, 5, receiver), "dispatch nao entrega frame curto");
    check(!packet_dispatch::Dispatch({1, nullptr, 4}, 4, receiver), "dispatch nao entrega nulo");
    check(!packet_dispatch::Dispatch({}, 4, receiver), "dispatch nao confunde evento local com frame");
    check(dispatchCalls == 1, "rejeicoes nao chamam receptor");
    check(receive_buffer::HasValidWindow(0, 0, 131072), "recepcao aceita janela vazia");
    check(!receive_buffer::HasValidWindow(4, 3, 131072), "recepcao rejeita cursor invertido");
    check(!receive_buffer::HasValidWindow(0, 0, 0), "recepcao rejeita capacidade nula");
    check(!receive_buffer::HasValidWindow(-1, 0, 131072), "recepcao rejeita cursor negativo");
    check(!receive_buffer::CanReadFrame(11, 12, 12), "recepcao rejeita frame curto");
    check(!receive_buffer::CanReadFrame(13, 12, 12), "recepcao rejeita frame incompleto");
    check(send_buffer::CanAppendRaw(4, 131068, 131072), "raw aceita limite exato");
    check(!send_buffer::CanAppendRaw(5, 131068, 131072), "raw rejeita overflow");
    check(!send_buffer::CanAppendRaw(INT_MAX, INT_MAX, 131072), "raw rejeita overflow signed");
    char queuedBytes[] = "abcdefgh";
    int queued = 8;
    int sent = 2;
    check(send_buffer::Compact(queuedBytes, 8, queued, sent), "compactacao aceita envio parcial");
    check(queued == 6 && sent == 0 && std::memcmp(queuedBytes, "cdefgh", 6) == 0,
        "compactacao preserva sufixo sobreposto da saida");
    check(send_buffer::Compact(queuedBytes, 8, queued, sent) && queued == 6,
        "compactacao repetida sem prefixo e idempotente");
    sent = queued;
    check(send_buffer::Compact(queuedBytes, 8, queued, sent) && queued == 0 && sent == 0,
        "compactacao esvazia envio completo");
    queued = 9;
    sent = 1;
    check(!send_buffer::Compact(queuedBytes, 8, queued, sent) && queued == 9 && sent == 1,
        "compactacao invalida nao altera indices");
    check(!send_buffer::Compact(nullptr, 8, queued, sent), "compactacao rejeita nulo");
    queued = 3;
    sent = 4;
    check(!send_buffer::Compact(queuedBytes, 8, queued, sent), "compactacao rejeita sent maior que queued");
    for (int accepted = 0; accepted < 2; ++accepted) {
        for (int flushed = 0; flushed < 2; ++flushed) {
            int stage = 0;
            const bool result = send_buffer::EnqueueAndFlush(
                [&] { check(stage++ == 0, "enqueue precede flush"); return accepted; },
                [&] { check(stage++ == 1, "flush ocorre mesmo na rejeicao"); return flushed; });
            check(result == (accepted != 0 && flushed != 0) && stage == 2,
                "resultado nao mascara falha nem repete chamadas");
        }
    }
    failures += RunReceivedPacketDispatchTests(checks);
    failures += RunCargoSlotTests(checks);
    failures += RunGridInsertionTests(checks);
    if (failures == 0) std::printf("ArchitectureTests: %d checks PASS; static assertions PASS\n", checks);
    return failures == 0 ? 0 : 1;
}
