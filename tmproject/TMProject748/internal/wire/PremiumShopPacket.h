#pragma once
#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

// Premium shop, EXTENSAO_COORDENADA v1 (DOCS/protocol/premium-shop.md). The
// window is the native shop (0x17C with ShopType 4); these packets carry the
// donate prices, the balance and the purchase request.
namespace premium_shop
{
    constexpr unsigned short StateOpcode = 0x7F20;
    constexpr unsigned short BuyOpcode = 0x7F21;
    constexpr unsigned int ShopType = 4;
    constexpr std::uint16_t Version = 1;
    constexpr int Slots = 27;
    constexpr std::uint16_t KindBalance = 0;
    constexpr std::uint16_t KindOpen = 1;

    struct State
    {
        MSG_STANDARD Header;
        std::uint16_t ContractVersion, Kind;
        std::uint32_t CatalogVersion;
        std::uint32_t Balance;
        std::uint32_t Prices[Slots];
    };
    struct Buy
    {
        MSG_STANDARD Header;
        std::uint16_t ContractVersion, NPC, ShopSlot, CarryHint;
        std::uint32_t CatalogVersion;
        std::uint32_t Price;
    };
    static_assert(sizeof(State) == 132 && offsetof(State, CatalogVersion) == 16 &&
        offsetof(State, Balance) == 20 && offsetof(State, Prices) == 24, "Premium state ABI");
    static_assert(sizeof(Buy) == 28 && offsetof(Buy, NPC) == 14 && offsetof(Buy, ShopSlot) == 16 &&
        offsetof(Buy, CarryHint) == 18 && offsetof(Buy, CatalogVersion) == 20 && offsetof(Buy, Price) == 24,
        "Premium buy ABI");

    inline bool Parse(const void* bytes, std::size_t size, State& out)
    {
        if (!bytes || size != sizeof(State))
            return false;
        State p{};
        std::memcpy(&p, bytes, sizeof(p));
        if (p.Header.Size != sizeof(p) || p.Header.Type != StateOpcode || p.ContractVersion != Version ||
            (p.Kind != KindBalance && p.Kind != KindOpen))
            return false;
        out = p;
        return true;
    }

    // Client view of the premium shop opened by the last ShopType 4 list.
    struct ClientState
    {
        bool Open = false;
        unsigned short NPC = 0;
        std::uint32_t CatalogVersion = 0;
        std::uint32_t Balance = 0;
        std::uint32_t Prices[Slots]{};

        // A 0x17C list decides whether the shop window is the premium one.
        void OnShopList(unsigned int shopType, unsigned short npc)
        {
            Open = shopType == ShopType;
            NPC = Open ? npc : 0;
            CatalogVersion = 0;
            std::memset(Prices, 0, sizeof(Prices));
        }
        // Kind 1 fills the prices of the open window; kind 0 only updates the balance.
        void Apply(const State& p)
        {
            Balance = p.Balance;
            if (p.Kind == KindOpen && Open)
            {
                CatalogVersion = p.CatalogVersion;
                std::memcpy(Prices, p.Prices, sizeof(Prices));
            }
        }
        std::uint32_t PriceAt(int slot) const
        {
            return Open && slot >= 0 && slot < Slots ? Prices[slot] : 0;
        }
        void Close() { OnShopList(0, 0); }
        // Builds the purchase request; false when the cell has no price yet.
        bool MakeBuy(unsigned short playerID, int slot, int carryHint, Buy& out) const
        {
            const std::uint32_t price = PriceAt(slot);
            if (!Open || !price || !CatalogVersion || carryHint < 0 || carryHint > 0xFFFF)
                return false;
            out = {};
            out.Header.Size = sizeof(out);
            out.Header.Type = BuyOpcode;
            out.Header.ID = playerID;
            out.ContractVersion = Version;
            out.NPC = NPC;
            out.ShopSlot = static_cast<std::uint16_t>(slot);
            out.CarryHint = static_cast<std::uint16_t>(carryHint);
            out.CatalogVersion = CatalogVersion;
            out.Price = price;
            return true;
        }
    };
}
