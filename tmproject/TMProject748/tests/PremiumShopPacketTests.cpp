#include "../internal/wire/PremiumShopPacket.h"
#include <cstdio>

// EXTENSAO_COORDENADA v1, mirrored by wydgo748/internal/wire/premium.go.
int RunPremiumShopPacketTests(int& checks)
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        ++checks;
        if (!condition) { ++failures; std::printf("FAIL premium shop: %s\n", name); }
    };

    premium_shop::State open{};
    open.Header.Size = sizeof(open);
    open.Header.Type = premium_shop::StateOpcode;
    open.ContractVersion = 1;
    open.Kind = premium_shop::KindOpen;
    open.CatalogVersion = 3;
    open.Balance = 5000;
    open.Prices[2] = 1000;
    premium_shop::State parsed{};
    check(premium_shop::Parse(&open, sizeof(open), parsed) && parsed.Prices[2] == 1000, "valid open state parses");
    check(!premium_shop::Parse(&open, sizeof(open) - 1, parsed), "truncated state is rejected");
    premium_shop::State bad = open;
    bad.ContractVersion = 2;
    check(!premium_shop::Parse(&bad, sizeof(bad), parsed), "unknown version is rejected");
    bad = open;
    bad.Kind = 2;
    check(!premium_shop::Parse(&bad, sizeof(bad), parsed), "unknown kind is rejected");
    bad = open;
    bad.CatalogVersion = 0;
    check(premium_shop::Parse(&bad, sizeof(bad), parsed), "an empty catalog (version 0) still opens the window");

    premium_shop::ClientState shop{};
    premium_shop::Buy buy{};
    shop.Apply(open);
    check(!shop.Open && shop.PriceAt(2) == 0 && shop.Balance == 5000,
        "prices need a premium shop list first; the balance always updates");
    shop.OnShopList(1, 1100);
    shop.Apply(open);
    check(!shop.Open && !shop.MakeBuy(9, 2, 1, buy), "a gold shop never sends premium purchases");
    shop.OnShopList(premium_shop::ShopType, 1100);
    shop.Apply(open);
    check(shop.Open && shop.NPC == 1100 && shop.PriceAt(2) == 1000 && shop.PriceAt(27) == 0,
        "the premium list and state price the cells");
    check(shop.MakeBuy(9, 2, 5, buy) && buy.Header.Size == 28 && buy.Header.Type == premium_shop::BuyOpcode &&
        buy.Header.ID == 9 && buy.NPC == 1100 && buy.ShopSlot == 2 && buy.CarryHint == 5 &&
        buy.CatalogVersion == 3 && buy.Price == 1000, "purchase carries cell, version and shown price");
    check(!shop.MakeBuy(9, 4, 5, buy), "an unpriced cell cannot be bought");
    premium_shop::ClientState empty{};
    empty.OnShopList(premium_shop::ShopType, 1100);
    empty.Apply(bad);
    check(empty.Open && empty.Balance == 5000 && !empty.MakeBuy(9, 2, 5, buy),
        "an empty premium window shows the balance and sends no purchase");
    premium_shop::State balance{};
    balance.Header.Size = sizeof(balance);
    balance.Header.Type = premium_shop::StateOpcode;
    balance.ContractVersion = 1;
    balance.Balance = 4000;
    check(premium_shop::Parse(&balance, sizeof(balance), parsed), "balance state parses");
    shop.Apply(parsed);
    check(shop.Balance == 4000 && shop.PriceAt(2) == 1000, "a balance update keeps the prices");
    shop.Close();
    check(!shop.Open && shop.PriceAt(2) == 0 && !shop.MakeBuy(9, 2, 5, buy), "closing the window clears the premium state");
    return failures;
}
