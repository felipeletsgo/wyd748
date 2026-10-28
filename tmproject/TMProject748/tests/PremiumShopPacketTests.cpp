#include "../internal/wire/PremiumShopPacket.h"
#include "../internal/wire/ReceivedPacketDispatch.h"
#include <cstdio>
#include "../internal/platform/windows/PremiumNellWindow.h"
#include "../internal/application/ModalInputPolicy.h"

// EXTENSAO_COORDENADA v1/v3/v4, mirrored by wydgo748/internal/wire/premium.go.
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
    premium_shop::CategorizedState categorized{};
    categorized.Base = open;
    categorized.Base.Header.Size = sizeof(categorized);
    categorized.Base.ContractVersion = premium_shop::CategoryVersion;
    categorized.Categories[2] = 2;
    categorized.Categories[26] = 3;
    std::uint8_t categories[premium_shop::Slots]{};
    check(premium_shop::Parse(&categorized, sizeof(categorized), parsed, categories) &&
        categories[2] == 2 && categories[26] == 3, "v4 categories preserve original shop slots");

    // Exercise the production transport gate before the scene's parser. Parser
    // tests alone missed the revision-8 regression: valid v4 opens were dropped.
    premium_shop::ClientState dispatchedShop{};
    dispatchedShop.OnShopList(premium_shop::ShopType, 1100);
    int stateDeliveries = 0;
    const auto receiveState = [&](const PacketView& view) {
        ++stateDeliveries;
        premium_shop::State state{};
        std::uint8_t decoded[premium_shop::Slots]{};
        if (premium_shop::Parse(view.data, view.size, state, decoded))
            dispatchedShop.Apply(state, decoded);
    };
    const auto dispatchState = [&](const auto& state) {
        return received_packet::Dispatch({premium_shop::StateOpcode,
            reinterpret_cast<const char*>(&state), sizeof(state)}, receiveState);
    };
    const auto categorizedBefore = categorized;
    check(dispatchState(categorized) && stateDeliveries == 1 &&
        dispatchedShop.Open && dispatchedShop.CatalogVersion == 3 &&
        dispatchedShop.Balance == 5000 && dispatchedShop.Categories[2] == 2 &&
        dispatchedShop.Categories[26] == 3 && dispatchedShop.PriceAt(2) == 1000,
        "v4 open reaches the scene through the production dispatch gate");
    premium_shop::Buy dispatchedBuy{};
    check(dispatchedShop.MakeBuy(9, 2, 5, dispatchedBuy) && dispatchedBuy.ShopSlot == 2 &&
        dispatchedBuy.NPC == 1100 && dispatchedBuy.Price == 1000,
        "dispatched v4 catalog preserves the original purchase slot and price");
    check(std::memcmp(&categorizedBefore, &categorized, sizeof(categorized)) == 0,
        "dispatch preserves the complete categorized frame");
    check(dispatchState(open), "legacy v1 open still reaches the scene");
    auto balanceFrame = open;
    balanceFrame.Kind = premium_shop::KindBalance;
    balanceFrame.Balance = 4000;
    check(dispatchState(balanceFrame) && dispatchedShop.Balance == 4000 &&
        dispatchedShop.PriceAt(2) == 1000, "v1 balance dispatch keeps the open catalog prices");

    const int validDeliveries = stateDeliveries;
    const auto* categorizedBytes = reinterpret_cast<const char*>(&categorized);
    for (std::size_t size = 0; size < sizeof(categorized); ++size)
        check(!received_packet::Dispatch({premium_shop::StateOpcode, categorizedBytes, size}, receiveState),
            "dispatch rejects every truncated categorized prefix");
    check(!received_packet::Dispatch({premium_shop::StateOpcode, categorizedBytes,
        sizeof(categorized) + 1}, receiveState), "dispatch rejects a categorized tail");
    check(!received_packet::Dispatch({premium_shop::StateOpcode, nullptr,
        sizeof(categorized)}, receiveState), "dispatch rejects a null categorized frame");
    check(!received_packet::Dispatch({0x119, categorizedBytes, sizeof(categorized)}, receiveState),
        "outer opcode cannot hide categorized state");
    auto invalidState = categorized;
    invalidState.Base.Header.Type = 0x119;
    check(!dispatchState(invalidState), "categorized opcode cannot hide a different header type");
    invalidState = categorized; invalidState.Base.Header.Size = sizeof(open);
    check(!dispatchState(invalidState), "dispatch rejects mismatched categorized header size");
    invalidState = categorized; invalidState.Base.ContractVersion = premium_shop::Version;
    check(!dispatchState(invalidState), "dispatch rejects v1 with a categorized payload");
    invalidState = categorized; invalidState.Base.Kind = premium_shop::KindBalance;
    check(!dispatchState(invalidState), "dispatch rejects categorized balance state");
    invalidState = categorized; invalidState.Categories[26] = 4;
    check(!dispatchState(invalidState), "dispatch rejects invalid categories before the scene");
    invalidState = categorized; invalidState.Reserved = 1;
    check(!dispatchState(invalidState), "dispatch rejects nonzero categorized reserved byte");
    check(stateDeliveries == validDeliveries, "invalid premium frames never reach the scene");
    std::array<char, sizeof(categorized) + 1> unaligned{};
    std::memcpy(unaligned.data() + 1, &categorized, sizeof(categorized));
    check(received_packet::Dispatch({premium_shop::StateOpcode, unaligned.data() + 1,
        sizeof(categorized)}, receiveState), "dispatch accepts unaligned categorized state");

    shop.OnShopList(premium_shop::ShopType, 1100);
    shop.Apply(parsed, categories);
    check(shop.Categories[2] == 2 && shop.Categories[26] == 3 && shop.PriceAt(2) == 1000,
        "categorized state applies prices and categories together");
    check(!premium_shop::Parse(&categorized, sizeof(categorized) - 1, parsed, categories), "truncated categorized state rejected");
    categorized.Categories[26] = 4;
    check(!premium_shop::Parse(&categorized, sizeof(categorized), parsed, categories), "unknown category rejected");
    categorized.Categories[26] = 3;
    categorized.Reserved = 1;
    check(!premium_shop::Parse(&categorized, sizeof(categorized), parsed, categories), "reserved category byte rejected");
    check(premium_shop::Parse(&open, sizeof(open), parsed, categories) && categories[2] == 0 && categories[26] == 0,
        "legacy catalogs default all slots to Featured");
    shop.Close();
    check(shop.Categories[2] == 0 && shop.Categories[26] == 0, "shop teardown clears categories");
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
    premium_shop::Result result{};
    result.Header.Size = sizeof(result);
    result.Header.Type = premium_shop::ResultOpcode;
    result.ContractVersion = premium_shop::WebVersion;
    result.Code = 6;
    result.Balance = 5000;
    result.RequestID = 7;
    premium_shop::Result completion{};
    check(premium_shop::ParseResult(&result, sizeof(result), completion) && completion.Code == 6 && completion.Balance == 5000 && completion.RequestID == 7,
        "web completion preserves the authoritative failure and balance");
    check(!premium_shop::ParseResult(&result, sizeof(result) - 1, completion), "truncated completion rejected");
    result.ContractVersion = 1;
    check(!premium_shop::ParseResult(&result, sizeof(result), completion), "legacy envelope is not a web completion");
    result.ContractVersion = 2;
    check(!premium_shop::ParseResult(&result, sizeof(result), completion), "unpublished v2 completion rejected");
    result.ContractVersion = premium_shop::WebVersion;
    result.Code = 9;
    check(!premium_shop::ParseResult(&result, sizeof(result), completion), "unknown completion rejected");
    result.Code = 0;
    result.RequestID = 0;
    check(!premium_shop::ParseResult(&result, sizeof(result), completion), "uncorrelated completion rejected");
    premium_shop::WebBuy webBuy{};
    shop.OnShopList(premium_shop::ShopType, 1100);
    shop.Apply(open);
    check(shop.MakeBuy(9, 2, 5, webBuy.Purchase), "web intent retains validated catalog fields");
    webBuy.Purchase.Header.Size = sizeof(webBuy);
    webBuy.Purchase.ContractVersion = premium_shop::WebVersion;
    webBuy.RequestID = premium_shop::NextRequestID();
    check(sizeof(webBuy) == 32 && webBuy.Purchase.Header.Size == 32 && webBuy.RequestID != 0 &&
        webBuy.Purchase.Price == 1000 && webBuy.Purchase.NPC == 1100, "v3 web envelope retains the v1 prefix");
    premium_shop::PendingPurchase pending;
    bool enqueued = false;
    check(!pending.Submit(webBuy.RequestID, [&] { enqueued = true; return false; }) && enqueued && !pending,
        "known local queue rejection unlocks the purchase");
    const auto nextID = premium_shop::NextRequestID();
    check(nextID > webBuy.RequestID, "request sequence survives shop state replacement");
    check(pending.Submit(nextID, [&] { return pending.RequestID == nextID; }) && pending,
        "queue acceptance retains a latch established before sending");
    enqueued = false;
    check(!pending.Submit(nextID + 1, [&] { enqueued = true; return true; }) && !enqueued,
        "pending purchase cannot enqueue a second intent");
    check(!pending.Complete(webBuy.RequestID) && pending, "stale completion cannot release the current purchase");
    check(pending.Complete(nextID) && !pending && !pending.Complete(nextID), "only the matching completion releases once");
    check(!pending.Submit(0, [] { return true; }), "request sequence exhaustion cannot enqueue an uncorrelated purchase");
    check(premium_nell::ParseAction(L"buy:26").Slot == 26, "last web catalog slot accepted");
    for (const int width : {-1, 0, 320, 800, 1024, 1920})
        for (const int height : {-1, 0, 200, 560, 768, 1080})
        {
            const auto bounds = premium_nell::FitEmbedded(width, height);
            check(bounds.X == 0 && bounds.Y == 0 &&
                bounds.Width == (width > 0 ? width : 0) &&
                bounds.Height == (height > 0 ? height : 0), "embedded shop fills the game client area");
        }
    const auto fullArea = premium_nell::FitEmbedded(1024, 768);
    check(fullArea.X == 0 && fullArea.Y == 0 && fullArea.Width == 1024 && fullArea.Height == 768,
        "shop covers the complete game client area without an external caption");
    ModalInputPolicy input;
    check(!input.Discard(false, true, 0x80), "normal game mouse input is preserved");
    check(input.Discard(true, true, 0) && input.Discard(true, true, 0x80), "browser input never reaches the game");
    check(input.Discard(false, true, 0x80), "closing while holding a button cannot click the world");
    check(input.Discard(false, false, 0), "lost mouse acquisition cannot pretend a held button was released");
    check(input.Discard(false, true, 0) && !input.Discard(false, true, 0x80), "release re-arms the next deliberate game click");
    check(input.Discard(true, false, 0) && input.Discard(false, true, 0), "reopening and closing without a mouse sample remains safe");
    premium_nell::EmbeddedNavigation navigation;
    check(!navigation.Admit(L"about:blank", 1), "unprepared navigation is denied");
    navigation.Prepare("abc");
    for (const auto uri : {L"https://example.com", L"file:///page.html", L"data:text/html;charset=utf-8;base64,eHl6", L"about:blank#fragment"})
        check(!navigation.Admit(uri, 2), "only the exact embedded initial document is allowed");
    check(navigation.Admit(L"data:text/html;charset=utf-8;base64,YWJj", 7), "current runtime embedded document accepted");
    check(navigation.IsDocument(7) && !navigation.IsDocument(8), "navigation completion is correlated");
    check(!navigation.Admit(L"data:text/html;charset=utf-8;base64,YWJj", 8) && !navigation.Admit(L"about:blank", 9), "reloads and later blank navigations are denied");
    navigation.Prepare("a");
    check(navigation.Admit(L"data:text/html;charset=utf-8;base64,YQ==", 10), "base64 one-byte padding accepted");
    navigation.Prepare("ab");
    check(navigation.Admit(L"data:text/html;charset=utf-8;base64,YWI=", 11), "base64 two-byte padding accepted");
    navigation.Prepare("\xC3\xA7");
    check(navigation.Admit(L"data:text/html;charset=utf-8;base64,w6c=", 12), "UTF-8 locale bytes preserved");
    navigation.Prepare("abc");
    check(navigation.Admit(L"about:blank", 13) && !navigation.Admit(L"about:blank", 14), "older runtime blank document allowed once");
    for (const auto message : {L"buy:27", L"buy:-1", L"buy:1x", L"buy:100", L"buy:", L"buy:2:1", L"https://example.test"})
        check(premium_nell::ParseAction(message).Kind == premium_nell::ActionKind::None, "untrusted bridge command rejected");
    check(premium_nell::Quote(L"a\"\\\n") == L"\"a\\\"\\\\\\u000a\"", "item names are JSON data, never executable script");
    premium_nell::Item detailed{};
    detailed.Details = {L"Damage : 30 (60)", L"<script>\"\\\n"};
    detailed.Effects = {{{2, 30}, {43, 9}, {0, 0}}};
    check(premium_nell::ItemDetailsJSON(detailed) ==
        L",\"details\":[\"Damage : 30 (60)\",\"<script>\\\"\\\\\\u000a\"],\"effects\":[{\"code\":2,\"value\":30},{\"code\":43,\"value\":9},{\"code\":0,\"value\":0}]",
        "catalog details escape text and preserve all three configured byte pairs");
    return failures;
}
