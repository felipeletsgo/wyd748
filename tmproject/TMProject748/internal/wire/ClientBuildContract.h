#pragma once

// Coordinated admission contract v2 (release line 7.48); see DOCS/login-dispatch.md.
// Prefix is the release line (748 << 16); Revision is its patch number, so
// Identity 0x02EC0001 is 7.48.001 and pairs with config.txt [VERSION] 48001.
// Bump Revision with the server minimum for each mandatory client update.
namespace client_build {
constexpr unsigned int Protocol = 748;
constexpr unsigned int Prefix = 0x02EC0000;
constexpr unsigned int Revision = 12;
static_assert(Prefix >> 16 == Protocol, "client build prefix must name the release line");
constexpr unsigned int Identity = Prefix | Revision;
static_assert(Revision > 0 && Revision <= 0xFFFF, "client build revision must not wrap");

template <typename LoginPacket>
inline void IdentifyLogin(LoginPacket& packet)
{
    packet.ClientVersion = Protocol;
    packet.ClientBuild = Identity;
}
}
