#pragma once

#include "CharacterLoginPacket.h"
#include "../application/ports/ICharacterLoginSender.h"
#include "../application/ports/ITransport.h"

// Traduz a intencao de login para o contrato wire existente. Empresta a porta
// de transporte e nao conhece socket, controles, timer ou estado de cena.
class CharacterLoginSender final : public ICharacterLoginSender
{
public:
    // transport precisa permanecer vivo enquanto este adaptador for utilizado.
    explicit CharacterLoginSender(ITransport& transport) : transport_(transport) {}

    // Monta os 36 bytes zerados e preenche apenas opcode e slot, como a cena
    // anterior. O transporte pode modificar o buffer ate o retorno, sem rete-lo.
    bool SendCharacterLogin(int slot) override
    {
        if (slot < 0 || slot >= 4) return false;
        MSG_CharacterLogin request{};
        request.Header.ID = 0;
        request.Header.Type = MSG_CharacterLogin_Opcode;
        request.Slot = slot;
        return transport_.Send({MSG_CharacterLogin_Opcode,
            reinterpret_cast<char*>(&request), sizeof(request)});
    }

private:
    ITransport& transport_;
};
