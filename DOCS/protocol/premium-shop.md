# Premium shop and donate balance (0x7F20 / 0x7F21)

Classification: `EXTENSAO_COORDENADA` v1, approved by the project owner on
2026-10-07. Native 7.48 only classifies an NPC with face 57 (item 57,
`Premium_Trader_Nell`) as a merchant; its web premium shop (`0x3A5`, an ASP
page that received the account password in the URL) is not ported. This
contract replaces it with an authoritative in-game shop that reuses the native
shop window.

## Owner decisions

- Currency: an account-wide **donate** balance (new), credited by staff from
  the staff panel now and by the web platform later.
- Catalog: edited in the staff panel and stored in the database.
- Interface: the native 7.48 shop window; prices are shown in donate.
- Bought items are ordinary, tradeable items.
- The Wyden coins convert between donate and a tradeable item. Using one
  credits its value to the account; buying one in the premium shop costs its
  value. Their values come from their names:

| Item | Name | Donate |
| --- | --- | ---: |
| 3393 | Coin_Wyden(200b/c) | 200 |
| 3394 | Gold_Wyden(1000b/c) | 1000 |
| 3395 | Gold_Wyden(3000b/c) | 3000 |
| 3396 | Gold_Wyden(5000b/c) | 5000 |
| 3441 | Gold_Wyden(10000b/c) | 10000 |

  The previous server data was wrong (3393 gave 50 honor; the others credited
  a per-character `donate` counter with values that did not match the names
  and that nothing consumed).

## State

- `Account.Donate` (uint32, JSON `donate`) is the balance, saved with the
  account in the same transaction as any item it pays for. Cap:
  `MaxDonateBalance` = 2,000,000,000.
- `premium_shop` (PostgreSQL, one row) holds the catalog: up to 27 entries
  `{slot 0..26, item (index + 3 effects), price > 0}` and a `version` that
  increases on every staff save. The JSON store reads
  `data/premium_shop.json` instead.
- `donate_credits` is the inbox of credits written by the staff panel or the
  web platform: `id`, `account_key`, `amount > 0`, `reason`, `source`
  (`staff` | `web`), `actor`, unique `request_id` (idempotency), `created_at`,
  `applied_at`. The game server applies a credit by saving the account with
  the new balance and setting `applied_at` in one transaction, so a credit is
  applied exactly once even if the account is online.
- `donate_ledger` records every balance change in the same transaction:
  `account_key`, `delta`, `balance_after`, `kind` (`credit`, `purchase`,
  `coin_use`), `item_index`, `item_uid`, `credit_id`, `character`,
  `created_at`.

## Packets

### Opening (native packets)

1. Client: clicking an NPC with face 57 sends the native `0x27B`
   (`MSG_REQShopList`, 16 bytes, NPC ID @12).
2. Server: validates the NPC (existence, range, face 57) and replies with
   `0x17C` (236 bytes) with `ShopType = 4` (premium), the catalog items in
   their slots and `Tax = 0`, followed by `0x7F20`. Any player can open it;
   an empty catalog opens an empty window that still shows the balance.

### 0x7F20 PremiumShopState, server to client, 132 bytes

| Offset | Type | Field |
| --- | --- | --- |
| 0 | header | `Type = 0x7F20`, `ID` = player |
| 12 | u16 | contract version, 1 |
| 14 | u16 | kind: 0 balance only, 1 shop open |
| 16 | u32 | catalog version (0 for kind 0, or kind 1 with no catalog saved) |
| 20 | u32 | donate balance |
| 24 | u32[27] | price per shop slot (0 = empty; zeros for kind 0) |

Kind 0 is also sent after a credit, a coin use or a purchase.

### 0x7F21 PremiumBuy, client to server, 28 bytes

| Offset | Type | Field |
| --- | --- | --- |
| 0 | header | `Type = 0x7F21`, `ID` = player |
| 12 | u16 | contract version, 1 |
| 14 | u16 | NPC ID shown in the shop |
| 16 | u16 | shop slot 0..26 |
| 18 | u16 | Carry slot chosen by the client (hint only) |
| 20 | u32 | catalog version the client shows |
| 24 | u32 | price the client shows |

### Purchase result

- Success: the native `0x379` confirmation (the client copies the shop item
  into the Carry slot chosen by the server) and `0x7F20` kind 0 with the new
  balance.
- Rejection: a `0x101` message and no state change.

## Server validation and failures

In order; every rejection answers with a message and changes nothing:

1. Player in the world, alive, not reserved by another critical write, packet
   exactly 28 bytes, version 1, header ID = player.
2. The premium shop opened by this player is the NPC in the packet, and the
   NPC is still in range.
3. Slot holds a catalog entry, and catalog version and price equal the
   current ones ("The shop changed. Please open it again.").
4. Balance >= price ("You do not have enough donate.").
5. A free Carry cell for the item ("Your inventory is full.").
6. One critical write saves the account with the item, the debited balance
   and the ledger entry. An unknown outcome follows the existing critical
   persistence fencing.

Selling items to the premium NPC is refused. A Wyden coin use is rejected
when the balance would pass the cap.

## Lifecycle

- Opening another NPC, moving away, logout or a new `0x17C` closes the premium
  context on the server. The client clears its premium state when the shop
  window closes or another `0x17C` arrives.
- Credits are applied at login and, for online accounts, by a periodic check;
  the player then receives `0x7F20` kind 0 and a message.
- Catalog changes take effect for the next opening; an open window with the
  old version is refused on purchase.

## Staff panel

- `GET/PUT /api/v1/staff/premium-shop`: read or replace the catalog
  (27 slots, item index, effects, price); the server rejects unknown items,
  duplicate slots and non-positive prices.
- `POST /api/v1/staff/accounts/{name}/donate`: add a credit
  (`amount`, `reason`, optional `requestId`), audited.
- `GET /api/v1/staff/accounts/{name}/donate`: balance, pending credits and the
  latest ledger entries.

## Validation gates

- Go: packet builders/parsers, every rejection above, repeated and concurrent
  purchases, credit applied once, coin use, rollback on a failed write,
  catalog validation; PostgreSQL integration with `-race`.
- C++: packet ABI asserts, parser rejections, premium shop state.
- Client test by the owner: open the shop, buy, insufficient balance,
  inventory full, use a coin, staff credit while online.

## Implementation

| Part | Where |
| --- | --- |
| Balance | `model.Account.Donate` (`wydgo748/internal/model/model.go`) |
| Storage | schema v5 in `internal/store/postgres_schema.sql`; `internal/store/donate.go`, `postgres_donate.go` |
| World | `internal/game/premium_shop.go` (open, buy, coins, credits, poll), `premium_control.go` (staff capability); donate writes go through the critical coordinator (`criticalWrite.donate`) |
| Wire | `internal/wire/premium.go`; client `tmproject/TMProject748/internal/wire/PremiumShopPacket.h` |
| NPC | `data/npcs/Premium_Nell.json` (face 57; the name keeps the 12-byte wire limit), spawned at Armia (2101, 2100) by `data/NPCGener.txt` |
| Wyden coins | `data/volatiles.json` items 3393..3396 and 3441: `grant_counter` with the single counter `donate`, which the server credits to the account balance. They are no longer sold for gold (`ShopUse20B`) |
| Staff panel | `internal/webadmin/premium.go`, `internal/control/premium.go`, `api/openapi.yaml`, `web/portal/src/components/PremiumShop.astro`, `src/scripts/premium-shop.ts` |
| Client | `TMFieldSceneMerchant.cpp` (face 57 click, ShopType 4, 0x7F20), `SGridCommerce.cpp` (0x7F21), `SGridTooltip.cpp` (donate price) |

The premium shop needs `database_driver=postgres` and the critical
persistence coordinator; otherwise the NPC answers that the shop is not
available. Staff accounts need the capabilities `economy.premium.catalog`
(edit the catalog) and `economy.donate.credit` (credit and view balances) in
`staff.json`.

## Validation record

- Go: `go vet ./...` and `go test -count=1 ./...` passed, including the
  PostgreSQL store tests (`WYD_TEST_POSTGRES_URL`); premium, donate,
  character-transfer and critical tests passed with `-race` (2026-10-07).
- C++: `ArchitectureTests` 1,152,694 checks (premium packet ABI and client
  state) and the Release build passed.
- Staff panel: `astro check` with no errors and `astro build`.
- Client test by the owner (2026-10-07): catalog saved in the staff panel,
  shop opened, staff credit while online, purchase, insufficient balance,
  full inventory and Wyden coin use all worked in game (`CLIENT_TESTED`).
