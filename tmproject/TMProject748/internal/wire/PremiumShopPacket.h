#pragma once
#include "MessageHeader.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

// Premium shop, EXTENSAO_COORDENADA v1/v3/v4 (DOCS/protocol/premium-shop.md).
// Categorized opens use v4; balances and legacy purchases retain v1, web buys v3.
namespace premium_shop
{
    constexpr unsigned short StateOpcode = 0x7F20;
    constexpr unsigned short BuyOpcode = 0x7F21;
    constexpr unsigned short ResultOpcode = 0x7F22;
    constexpr unsigned int ShopType = 4;
    constexpr std::uint16_t Version = 1;
    constexpr std::uint16_t WebVersion = 3;
    constexpr std::uint16_t CategoryVersion = 4;
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
    struct CategorizedState
    {
        State Base;
        std::uint8_t Categories[Slots];
        std::uint8_t Reserved;
    };
    static_assert(sizeof(CategorizedState) == 160 && offsetof(CategorizedState, Categories) == 132,
        "Premium categorized state ABI");
    struct Result
    {
        MSG_STANDARD Header;
        std::uint16_t ContractVersion, Code;
        std::uint32_t Balance;
        std::uint32_t RequestID;
    };
    struct WebBuy
    {
        Buy Purchase;
        std::uint32_t RequestID;
    };
    static_assert(sizeof(WebBuy) == 32 && offsetof(WebBuy, RequestID) == 28, "Premium web buy ABI");
    static_assert(sizeof(Result) == 24 && offsetof(Result, Balance) == 16 &&
        offsetof(Result, RequestID) == 20, "Premium result ABI");

    // One sequence per application process, never reset on shop or scene changes.
    // Exhaustion refuses another purchase instead of reusing an identifier.
    inline std::uint32_t NextRequestID()
    {
        static std::uint32_t next = 0;
        return next == UINT32_MAX ? 0 : ++next;
    }

    struct PendingPurchase
    {
        std::uint32_t RequestID = 0;
        operator bool() const { return RequestID != 0; }
        bool Complete(std::uint32_t requestID)
        {
            if (!RequestID || RequestID != requestID) return false;
            RequestID = 0;
            return true;
        }
        // A rejected enqueue emitted no frame. Flush failure after acceptance
        // is uncertain and must retain the pending purchase.
        template<class Enqueue>
        bool Submit(std::uint32_t requestID, Enqueue enqueue)
        {
            if (RequestID || !requestID) return false;
            RequestID = requestID;
            if (enqueue()) return true;
            RequestID = 0;
            return false;
        }
    };

    inline bool ParseResult(const void* bytes, std::size_t size, Result& out)
    {
        if (!bytes || size != sizeof(Result)) return false;
        Result p{};
        std::memcpy(&p, bytes, sizeof(p));
        if (p.Header.Size != sizeof(p) || p.Header.Type != ResultOpcode ||
            p.ContractVersion != WebVersion || p.Code > 8 || !p.RequestID) return false;
        out = p;
        return true;
    }
    static_assert(sizeof(State) == 132 && offsetof(State, CatalogVersion) == 16 &&
        offsetof(State, Balance) == 20 && offsetof(State, Prices) == 24, "Premium state ABI");
    static_assert(sizeof(Buy) == 28 && offsetof(Buy, NPC) == 14 && offsetof(Buy, ShopSlot) == 16 &&
        offsetof(Buy, CarryHint) == 18 && offsetof(Buy, CatalogVersion) == 20 && offsetof(Buy, Price) == 24,
        "Premium buy ABI");

    inline bool Parse(const void* bytes, std::size_t size, State& out, std::uint8_t* categories = nullptr)
    {
        if (!bytes || (size != sizeof(State) && size != sizeof(CategorizedState)))
            return false;
        State p{};
        std::memcpy(&p, bytes, sizeof(p));
        if (p.Header.Size != size || p.Header.Type != StateOpcode ||
            (p.Kind != KindBalance && p.Kind != KindOpen))
            return false;
        std::uint8_t decoded[Slots]{};
        if (size == sizeof(State))
        {
            if (p.ContractVersion != Version) return false;
        }
        else
        {
            CategorizedState extended{};
            std::memcpy(&extended, bytes, sizeof(extended));
            if (p.ContractVersion != CategoryVersion || p.Kind != KindOpen || extended.Reserved) return false;
            for (int i = 0; i < Slots; ++i)
            {
                if (extended.Categories[i] > 3) return false;
                decoded[i] = extended.Categories[i];
            }
        }
        out = p;
        if (categories) std::memcpy(categories, decoded, sizeof(decoded));
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
        std::uint8_t Categories[Slots]{};

        // A 0x17C list decides whether the shop window is the premium one.
        void OnShopList(unsigned int shopType, unsigned short npc)
        {
            Open = shopType == ShopType;
            NPC = Open ? npc : 0;
            CatalogVersion = 0;
            std::memset(Prices, 0, sizeof(Prices));
            std::memset(Categories, 0, sizeof(Categories));
        }
        // Kind 1 fills the prices of the open window; kind 0 only updates the balance.
        void Apply(const State& p, const std::uint8_t* categories = nullptr)
        {
            Balance = p.Balance;
            if (p.Kind == KindOpen && Open)
            {
                CatalogVersion = p.CatalogVersion;
                std::memcpy(Prices, p.Prices, sizeof(Prices));
                if (categories) std::memcpy(Categories, categories, sizeof(Categories));
                else std::memset(Categories, 0, sizeof(Categories));
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
