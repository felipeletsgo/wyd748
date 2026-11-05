#pragma once

// Character-selection port without wire structs or opcodes.
// The implementation encodes intent without changing UI or session state.
class ICharacterLoginSender
{
public:
    virtual ~ICharacterLoginSender() = default;
    // The slot must be 0..3. This synchronous call transfers no ownership.
    // True means local send acceptance, not remote login confirmation.
    virtual bool SendCharacterLogin(int slot) = 0;
};
