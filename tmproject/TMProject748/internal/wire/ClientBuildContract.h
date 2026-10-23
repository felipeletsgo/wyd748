#pragma once

// Coordinated admission contract v1; see DOCS/login-dispatch.md.
// Bump Revision with the server minimum for each mandatory client update.
namespace client_build {
constexpr unsigned int Protocol = 748;
constexpr unsigned int Prefix = 0x57470000;
constexpr unsigned int Revision = 3;
constexpr unsigned int Identity = Prefix | Revision;
static_assert(Revision > 0 && Revision <= 0xFFFF, "client build revision must not wrap");

template <typename LoginPacket>
inline void IdentifyLogin(LoginPacket& packet)
{
    packet.ClientVersion = Protocol;
    packet.ClientBuild = Identity;
}
}
