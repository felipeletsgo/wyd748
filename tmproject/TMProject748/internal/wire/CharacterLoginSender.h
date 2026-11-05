#pragma once

#include "CharacterLoginPacket.h"
#include "../application/ports/ICharacterLoginSender.h"
#include "../application/ports/ITransport.h"

// Encode login intent with the existing wire contract. Borrow the transport port.
// This sender has no socket, control, timer or scene-state dependency.
class CharacterLoginSender final : public ICharacterLoginSender
{
public:
    // transport precisa permanecer vivo enquanto este adaptador for utilizado.
    explicit CharacterLoginSender(ITransport& transport) : transport_(transport) {}

    // Build 36 zeroed bytes and fill only opcode and slot, as the previous scene did.
    // The transport can modify the buffer until return but cannot retain it.
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
