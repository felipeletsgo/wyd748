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
    return failures;
}
