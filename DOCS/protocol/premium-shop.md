# Premium shop and donate balance (0x7F20 / 0x7F21 / 0x7F22)

Classification: `EXTENSAO_COORDENADA` v1 (2026-10-07), embedded web
purchase v3 and categorized catalog v4 (2026-10-26), requested by the project owner. Native 7.48
classifies face 57 (`Premium_Trader_Nell`) as a merchant. Its `0x3A5` flow
fetches an ASP text catalog through WinINet and fills native rows; it does
not establish a modern embedded browser contract. The legacy URL included
the account password and is not reused.

The new client embeds a local Premium Nell page through WebView2. Catalog,
Donate balance, validation and settlement remain on the game connection.
No password, HTTP checkout or second catalog is introduced. The local page
may display the account name supplied by the client; it is not sent to an
official authentication endpoint.
The native shop remains the fallback when WebView2 is unavailable.
Keep the native shop and inventory hidden while the embedded page loads.
Prepare their contents without showing them. Show the native fallback only
after browser initialization fails, an asynchronous failure, or the ready timeout.
Pending purchases still block fallback interaction until server completion.

## Owner decisions

- Currency: an account-wide **donate** balance (new), credited by staff from
  the staff panel now and by the web platform later.
- Catalog: edited in the staff panel and stored in the database.
- Interface: an embedded Premium Nell page in EN/PT, with the native 7.48
  shop window as fallback; prices are shown in Donate. The existing staff
  panel remains the only catalog editor.
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
  `{slot 0..26, item (index + 3 effects), price > 0, category 0..3}` and a `version` that
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

### 0x7F20 categorized open state, v4: 160 bytes

Current opens use contract version 4 with kind 1. Offsets 0 through 131
retain the preceding layout, with header size 160 and version 4. Offset 132
contains 27 category bytes, one per original shop slot; offset 159 is a
reserved zero byte. Category 0 is Featured (PT: Destaque), 1 Armia, 2 Erion,
and 3 Nifleheim. The client rejects unknown category values, nonzero reserved
data, truncated envelopes and a v4 balance-only packet. Balance updates keep
the v1/132-byte layout. The new client also accepts a v1 open and assigns
all its slots to Featured.

### 0x7F21 PremiumBuy, client to server, v1: 28 bytes / v3: 32 bytes

| Offset | Type | Field |
| --- | --- | --- |
| 0 | header | `Type = 0x7F21`, `ID` = player |
| 12 | u16 | purchase version: 1 native grid, 3 embedded page |
| 14 | u16 | NPC ID shown in the shop |
| 16 | u16 | shop slot 0..26 |
| 18 | u16 | Carry slot chosen by the client (hint only) |
| 20 | u32 | catalog version the client shows |
| 24 | u32 | price the client shows |
| 28 | u32 | v3 only: nonzero purchase request ID |

The v3 request ID increases for each new purchase intent within the client
process. A resend retains the original ID and payload. The server accepts
only the exact v1/28-byte or v3/32-byte envelope; the unpublished v2 web
contract is superseded and rejected. Opening state uses v4/160 bytes;
balance updates retain v1/132 bytes. Purchase envelopes are unchanged.

### Purchase result, v1

- Success: the native `0x379` confirmation (the client copies the shop item
  into the Carry slot chosen by the server) and `0x7F20` kind 0 with the new
  balance.
- Rejection: a `0x101` message and no state change.

### 0x7F22 PremiumBuyResult, server to client, 24 bytes

Only purchases using version 3 receive this completion envelope. Catalog
state retains version 1 and its original layout.

| Offset | Type | Field |
| --- | --- | --- |
| 0 | header | `Type = 0x7F22`, `ID` = player |
| 12 | u16 | contract version, 3 |
| 14 | u16 | result code, below |
| 16 | u32 | authoritative Donate balance |
| 20 | u32 | purchase request ID being completed |

| Code | Meaning |
| ---: | --- |
| 0 | Purchase committed |
| 1 | Backend unavailable |
| 2 | Another critical action is pending |
| 3 | Merchant context expired or player dead |
| 4 | Catalog, price or slot changed; reopen required |
| 5 | Insufficient Donate |
| 6 | Inventory full |
| 7 | Item unavailable |
| 8 | Purchase failed without settlement |

On success, the server sends authoritative `SendItem`, balance state and
then result 0 only after the critical coordinator acknowledges durable
settlement. The browser never copies an item from a UI grid. A certified
write rejection sends result 8 without charging or delivering an item.
Unknown persistence outcomes retain the existing fencing/disconnection
policy; they never send a speculative success or invite an automatic retry.

## Server validation and failures

In order; well-formed purchase rejections answer with a v1 message or v3
completion and change nothing:

1. Player in the world, alive, not reserved by another critical write, packet
   exactly v1/28 bytes or v3/32 bytes, header ID = player and nonzero request
   ID for v3. Malformed packets
   follow the security violation path. Closed, poisoned or shutting-down
   sessions do not initiate a purchase.
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

For v3, a bounded receipt on the player connection is installed before
purchase validation. An identical pending request waits for its original
settlement; an identical settled request replays its code with the current
balance. Older IDs and equal IDs with different payloads fail without a
new write. A newer ID cannot replace a pending receipt. Validation failures
and certified write rejections also settle the receipt. It survives shop
reopening, including packets deferred by `World.handle` during a critical
write. It is not a durable receipt across reconnection.

## Lifecycle

- Opening another NPC, moving away, logout or a new `0x17C` closes the premium
  context on the server. The client clears its premium state when the shop
  window closes or another `0x17C` arrives.
- Credits are applied at login and, for online accounts, by a periodic check;
  the player then receives `0x7F20` kind 0 and a message.
- Catalog changes take effect for the next opening; an open window with the
  old version is refused on purchase.
- The embedded page starts hidden and replaces the native panel only after
  its ready handshake. Initialization failure or a 15-second ready timeout
  retains/restores the native panel. The browser host is a child of the
  game window, centered at up to 800 by 560 client pixels. The owner stays
  enabled so its browser child can receive input. Game window input and
  accelerators are intercepted while the shop is presented; DirectInput
  samples are drained without dispatching movement, skills or camera input.
  Held game keys are released, and mouse dispatch resumes only after a
  neutral sample to prevent closing clicks from reaching the world. Packet
  processing continues on the existing UI thread.
- The bridge accepts only `ready`, `close` and `buy:0` through `buy:26`.
  Items and prices are supplied by the host, never accepted from the page.
  Navigation, new windows and permissions are denied; the embedded page has
  no network dependencies and uses text nodes for catalog names.
- Confirmation latches the purchase before sending it. Further purchases,
  shop changes and window close are blocked until a matching request-ID
  completion arrives. Stale completions cannot unlock a different purchase.
  Catalog-changed and expired results require reopening the shop. No
  automatic retry is performed.
- A known local enqueue rejection clears the latch and displays localized
  client-only result 9 (not sent; retry permitted). Once `AddMessage`
  accepts the frame, a flush failure retains pending state because delivery
  is uncertain. Request IDs never wrap; exhaustion refuses a new purchase.
- Cards and confirmation share the tooltip's additional-stat formatter,
  including refinement scaling, units and localized resource labels.
  Mastery labels are resolved per item. An expandable section preserves
  the configured effect code/value pairs without inventing descriptions
  for special metadata. All item text is inserted as text nodes.
- Death, scene destruction or disconnect closes the browser. If its process
  fails during a pending purchase, the client waits for the server result
  and reports that result in the native message panel. It does not fall
  back into another purchase while settlement is unresolved.

## Staff panel

- `GET/PUT /api/v1/staff/premium-shop`: read or replace the catalog
  (27 slots, item index, effects, price); the server rejects unknown items,
  duplicate slots and non-positive prices.
- `POST /api/v1/staff/accounts/{name}/donate`: add a credit
  (`amount`, `reason`, optional `requestId`), audited.
- `GET /api/v1/staff/accounts/{name}/donate`: balance, pending credits and the
  latest ledger entries.

To add items, use the existing **Premium Shop** editor in the staff panel:
choose a slot, item, effects and Donate price, then save. The next NPC
opening receives that catalog directly; no HTML edit or client rebuild is
needed for catalog changes. Existing version checks and staff audit apply.

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
| NPC | `data/npcs/Premium_Nell.json` (face 57; the name keeps its conservative 12-byte spelling within the 16-byte NPC field, including NUL), spawned at Armia (2101, 2100) by `data/spawns/NPCGener.txt` |
| Wyden coins | `data/content/volatiles.json` items 3393..3396 and 3441: `grant_counter` with the single counter `donate`, which the server credits to the account balance. They are no longer sold for gold (`ShopUse20B`) |
| Staff panel | `internal/webadmin/premium.go`, `internal/control/premium.go`, `api/openapi.yaml`, `web/portal/src/components/PremiumShop.astro`, `src/scripts/premium-shop.ts` |
| Client | `TMFieldSceneMerchant.cpp` (face 57, catalog, page bridge and completion), `SGridCommerce.cpp` (native fallback), `SGridTooltip.cpp` (Donate price) |
| Browser host | `tmproject/TMProject748/internal/platform/windows/PremiumNellWindow.{h,cpp}`; `Resources/PremiumNell.html` embedded as neutral UTF-8 RCDATA |

The premium shop needs `database_driver=postgres` and the critical
persistence coordinator; otherwise the NPC answers that the shop is not
available. Staff accounts need the capabilities `economy.premium.catalog`
(edit the catalog) and `economy.donate.credit` (credit and view balances) in
`staff.json`.

## Browser dependency and evidence

The client statically links the x86 WebView2 loader. Builds bootstrap the
official NuGet SDK **1.0.3856.49** into the ignored
`tmproject/build/tools/webview2/` directory through
`tools/client-assets/Initialize-WebView2.ps1`. Package SHA-256:
`BC0F76EB911B569838DC4AA8F8D325269B966BEDB592863D26211AEF3A099F1A`.
Players need the Evergreen WebView2 Runtime; this feature does not install
it automatically. Missing runtime uses the native shop fallback.

Native 7.48 evidence, SHA-256
`8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593`:
saved `FUN_00489470.c`, `FUN_004729d4.c`, `FUN_00557a41.c`,
`FUN_00407d62.c` and `FUN_004085cd.c` under the existing ignored native
research corpus establish the legacy request/HTTP/table boundary.

The modern official executable, SHA-256
`a058262d36631fd8b674603fb20d54ff18016b8f3dd38249250e953b8efc3c50`,
contains `_NN_WebShopURL`, `https://wydmisc.raidhut.com/neil/global.php`
and an 800 by 560 web window configuration. Bounded static disassembly on
2026-10-26 establishes an embedded host: `0x5965BB..0x5965F6` passes the
game HWND (`[esi+4]`) as parent to `CreateWindowExW`, with style
`0x50000000` (`WS_CHILD | WS_VISIBLE`), centered dimensions 800 by 560.
The call at `0x5965F6` uses the import at `0x87C85C` (`USER32!CreateWindowExW`)
and stores the child HWND at `0x5ED12E8`. The adjacent creation at
`0x5966A5` makes a child close button with ID 999, style `0x5080000B`,
size 86 by 21, stored at `0x5ED12EC`. Toggle flow `0x594550..0x59470C`
shows/hides these controls and navigates the hosted browser. The call at
`0x596602` reaches `0x675AD0`, which calls `OleCreate` and
`OleSetContainedObject`; browser release is at `0x675700`. These facts
were recovered without running the executable, using the existing extraction
record and bounded `objdump -d -M intel` address windows with PE import
resolution. The modern host uses OLE; WebView2 is this project's rendering
choice. The original 7.48 flow instead fetches and parses HTML into game
controls; neither reference requires a separate desktop shop window.

The modern public wrapper redirects
to an identity-dependent landing page. This is a secondary presentation
reference, not proof of 7.48 parity or an exact reproduction of the current
official store. The new page displays item names, quantity, refinement and
additional stats, with expandable configured effect data; it does not
reproduce the full native item tooltip or item artwork.

## Validation record, v1

- Go: `go vet ./...` and `go test -count=1 ./...` passed, including the
  PostgreSQL store tests (`WYD_TEST_POSTGRES_URL`); premium, donate,
  character-transfer and critical tests passed with `-race` (2026-10-07).
- C++: `ArchitectureTests` 1,152,694 checks (premium packet ABI and client
  state) and the Release build passed.
- Staff panel: `astro check` with no errors and `astro build`.
- Client test by the owner (2026-10-07): catalog saved in the staff panel,
  shop opened, staff credit while online, purchase, insufficient balance,
  full inventory and Wyden coin use all worked in game (`CLIENT_TESTED`).

## Validation record, v2 (2026-10-26)

Historical candidate, superseded by v3 before publication.

- Focused Go wire, game and webadmin Premium/Donate tests passed, including
  durable acknowledgment, explicit rejections, duplicate reservation and
  certified write rejection without debit/delivery. The control package
  had no tests matching that selection.
- C++ architecture tests passed 3,176,767 checks, including result packet
  ABI, strict bridge grammar and JSON escaping. Socket, crash-capture,
  launcher and asset build gates passed; the Release candidate compiled.
  Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `6CE0DC659AC7BAA63114160CA63E1370BF090250A76EE47DAA82E60BE0F26B4D`.
  `Build-Client.ps1 -NoDeploy` left `tmproject/client748/WYD.exe` unchanged
  (SHA-256 `B0ADA8F60BEA7A82A3F3D1F86810959A0FBF2A3685D93A5C28A1F9CEC5B3C476`).
- `node --test tmproject/TMProject748/tests/PremiumNellPageTests.mjs`:
  four page-state tests passed, covering EN/PT key coverage, English
  fallback, safe text, confirmation, pending lock, completion, catalog
  invalidation, balance, filtering and empty/offline states.
- Browser preview inspected visually with a simulated catalog; confirmation,
  pending controls and balance completion were exercised. This does not
  validate the Win32 WebView2 host or a real transaction inside the game.
- Real client/server acceptance remains pending: ready/fallback on machines
  with and without WebView2, focus restoration, death/disconnect, browser
  failure during settlement, actual inventory delivery and both locales.
  The v2 feature is not `CLIENT_TESTED`, installed or published by this work.

## Validation record, v3 audit corrections (2026-10-26)

- `go test -race ./internal/wire ./internal/game ./internal/webadmin
  -run 'Premium|Donate' -count=1` passed in all three packages. Regressions
  exercise duplicate ingress through `World.handle` during persistence,
  reopening, stale/conflicting IDs, a legitimate second intent, explicit
  failures and certified write rejection without a second charge or item.
- `node --test tmproject/TMProject748/tests/PremiumNellPageTests.mjs`:
  six tests passed, including additional-stat text in cards/confirmation,
  configured effects, EN/PT coverage, escaping and local enqueue rejection.
- `Build-Client.ps1 -NoDeploy` passed: architecture 3,176,779 checks,
  socket 281, crash capture 200, launcher 58, asset/shader gates and Release
  compilation. The initial architecture gate caught accidental removal of
  base tooltip rows during extraction; those rows were restored before
  the successful build, including the mount run-speed line.
- Final candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `A36BD2CDFB7E61F06E93A2BEC3E81EAAD7FA1ED80390AFF9F71CFD07204690B5`.
  Installed `tmproject/client748/WYD.exe` remained unchanged, SHA-256
  `B0ADA8F60BEA7A82A3F3D1F86810959A0FBF2A3685D93A5C28A1F9CEC5B3C476`.
- The repository-layout validator failed on existing runtime logs/dumps
  under `tmproject/client748/`, the original installation's `WYD.log`, and
  `tmproject/client748.zip`. The installation now resides in
  `references/client748/WYD 7.48 PORTUGUES/`; the earlier result is historical.
  The ZIP was later recycled after its file paths were confirmed in the current
  client directory.
- Execution policy rejected cleanup of this task's successful build logs:
  `build/premium-nell-audit-fixes/client-build.log` and
  `build/premium-nell-audit-fixes/client-build-final.log`. No deletion retry
  through another shell or API was attempted; incremental outputs remain.
- No runtime installation, publication or restart. The v3 extension still
  requires real client/server acceptance, including the WebView2 host,
  lifecycle/fallback and real inventory delivery in both locales. Before
  publication, coordinate client/server activation and raise both mandatory
  client build revisions according to the repository release policy.

## Published v3 host diagnosis (2026-10-26)

The v3 extension was published in mandatory client 7.48.006, feed 48006,
with server minimum revision 6 and active release `20261026T195612Z`.
The installed runtime executable matches the published SHA-256
`6DAB02915D38F7E25DA4A072FE182B9265CF50E1E22EFA097FF282C02BEB239F`.
The owner reported web startup failure with Evergreen WebView2 installed.

An isolated x86 diagnostic compiled the actual `PremiumNellWindow.cpp` host
against its current SDK and compiled `TMProject.res`, with stage logging and
a separate temporary user-data folder. On runtime **155.0.4283.62**, environment,
controller, UTF-8 resource decoding and `NavigateToString` all succeeded.
`NavigationStarting` received `data:text/html;charset=utf-8;base64,...`.
The host's exact `about:blank` filter cancelled it; `NavigationCompleted`
reported success false and status **14**, `OPERATION_CANCELED`. The baseline
failed after 2,031 ms, rather than the 15-second readiness timeout.

In a diagnostic copy, permitting the embedded data navigation reached a
`WebMessageReceived` source of `about:blank` and the `ready` message after
609 ms. This isolates the host navigation filter as the cause; reinstalling
WebView2 is not indicated by these results. No production source correction,
installed runtime replacement or further publication occurred during diagnosis.

The production correction must admit only the intended initial embedded
document, retain navigation/message-origin restrictions, and expose stage and
HRESULT diagnostics. The temporary prefix allowance was a causal experiment,
not a production security policy. A real Win32 host regression and game
acceptance are required; page-only JavaScript tests did not cover this boundary.
Purchases, focus/lifecycle behavior and both locales remain pending. Detailed
bounded transcripts are in `.agents/research/release-748006.json`.

## Host navigation correction — client 7.48.007

This contract-preserving correction is `MODERNIZACAO_COMPATIVEL`; the v3
packet and authoritative purchase validation are unchanged. The host builds
the exact UTF-8 base64 data URI from the embedded HTML resource and admits
only that initial document, or the older runtime's initial `about:blank`.
Further navigation is denied. Navigation completion is correlated with the
accepted document ID, preventing denied unrelated navigation from failing
the shop. Bridge messages still require the exact `about:blank` source and
the existing bounded action grammar. Initialization failures log their stage
and HRESULT; navigation and renderer failures include their status/kind.

Portable regression checks cover exact document admission, UTF-8 and base64
padding, unsupported destinations, repeated navigation and completion IDs.
The canonical Release build and its architecture gates passed, as did all
six EN/PT page checks. An additional isolated real-WebView host regression
was blocked before execution by automatic approval review (`blocked by
policy`), with no detailed reason. The earlier runtime diagnosis remains
causal evidence, not actual execution of the corrected production client.
Opening/closing the updated game shop, purchases, observer-independent
focus/lifecycle behavior and both locales remain pending client acceptance.
Publication and activation are recorded in `.agents/research/release-748007.json`.
The mandatory patch was verified over public HTTP/HTTPS before activating
`/srv/wyd-go/releases/20261026T222350Z` at 22:23:53 UTC on 2026-10-26.
Both channels now require revision 7; synthetic nonexistent-account probes
verify rejection of revision 6 and credential validation for revision 7.
Those checks do not exercise the WebView host or complete a purchase.

## Blank host window after 7.48.007 (2026-10-26)

The owner reported a completely white Premium Nell window. The installed
`tmproject/client748/WYD.exe` matches published 7.48.007, SHA-256
`B4E5540A5F509983716CCC66680E887837729C0808AE78D667DCBE7200F19CE6`.
The current session log identifies that executable and contains no Premium
Nell failure entry. The host only shows its window after receiving `ready`;
the screenshot therefore suggests the bridge initialized, but does not prove
that the browser painted. The page has a dark background even with an empty
catalog, so an empty catalog alone does not explain the white surface.

Source inspection found no explicit controller visibility synchronization.
The local `MODERNIZACAO_COMPATIVEL` correction keeps the controller hidden
during initialization, restores its client bounds and explicitly makes it
visible after showing the host. Show/hide and minimize/restore synchronize
controller visibility; movement notifies the controller. Bounds, visibility
and initial snapshot failures enter the existing fallback path. Failures are
latched so late bridge messages cannot reopen a failed host. Diagnostics now
record document completion/readiness and the presented controller's visibility,
bounds and HRESULTs without catalog or account contents. The v3 wire contract
and EN/PT page resources are unchanged.

The canonical `Build-Client.ps1 -NoDeploy -Configuration Release` succeeded:
architecture 3,176,791 checks, socket 281, crash capture 200, launcher 58,
asset/shader gates and Release compilation. All six page tests passed.
The candidate `tmproject/build/TMProject748/Release/WYD.exe` has SHA-256
`958804E1311CCF1B9FA693F18F6B5289D70414F00B6FCCA2460F60C622DA7689`.
These checks do not execute the WebView host or establish the white-window
root cause. The installed runtime remains unchanged. No game interaction,
candidate installation, publication or server restart was performed after
the owner requested no computer interaction. Actual display, minimize/restore,
close/reopen and purchases remain pending. The candidate still carries build
revision 7 and must receive the next coordinated mandatory revision before
any future publication; it must not replace the already published 7.48.007.

## Embedded host correction (2026-10-26)

The owner rejected the external desktop window. Inspection confirmed that
the current host used `WS_OVERLAPPEDWINDOW`, an adjusted desktop rectangle
and owner disabling. The local correction creates a hidden `WS_CHILD` host
inside the game's client area, with no caption, resize border or taskbar
entry. Its 800 by 560 bounds are centered and clipped to smaller client
areas. The existing localized page supplies close and Escape actions.
This is `MODERNIZACAO_COMPATIVEL` within the existing v3 extension, without
changing wire packets, catalog management, purchase validation or pending
settlement rules. It does not establish exact modern page appearance or
native 7.48 browser parity.

The existing main message loop coordinates owner resize/show, input and
browser focus without subclassing the owner or starting a nested pump.
The parent remains enabled; game mouse/key/IME messages, accelerators and
DirectInput gameplay dispatch are suppressed while the child is presented.
Input polling still drains current samples, releases held keys and requires
mouse release before resuming world clicks. Window activation, network
messages and shutdown remain available. Minimize/restore and movement
synchronize the controller; closing tears down the controller and host,
and restores game focus only if the browser held it in the foreground game.
Late asynchronous controller callbacks reject a dead, failed or destroyed
host. No game window was opened or controlled during this work.

Validation: the canonical Release build with `-NoDeploy` passed architecture
3,176,834 checks, socket 281, crash capture 200, launcher 58, and asset/shader
gates. The 43 new checks exercise bounded/centered client-area placement and
mouse capture/release, including lost acquisition and reopening. All six
existing page tests passed, preserving EN/PT key coverage, English fallback,
UTF-8 text and catalog escaping. No page strings or wire fields changed.
An existing source-contract assertion was updated for the new input polling
parameter; its polling and transient-delta requirements remain intact.

Final candidate: `tmproject/build/TMProject748/Release/WYD.exe`, 2,013,696
bytes, SHA-256
`5CE379167DD1AF7DBE8F6C1C34BFE3AFB3A0CA039BB4B09FD9D13C9FCED384E3`.
The installed runtime remains SHA-256
`B4E5540A5F509983716CCC66680E887837729C0808AE78D667DCBE7200F19CE6`.
The candidate still identifies as revision 7 and requires the next coordinated
mandatory revision before publication. No installation, publication, restart
or real WebView/game execution occurred. Rendering, white-surface diagnosis,
focus/typing, resize/minimize/restore, close/reopen, death/disconnect, fallback
and real purchases in both locales remain pending client acceptance.

At that time, the repository-layout check reported existing runtime crash
logs, the original installation's `WYD.log`, and `tmproject/client748.zip`
outside build roots. The installation now resides in
`references/client748/WYD 7.48 PORTUGUES/`; the earlier result is historical.
The ZIP was later recycled after its file paths were confirmed in the current
client directory.
Automatic approval review
rejected cleanup of this turn's three temporary crash-test evidence directories
with `blocked by policy`, without a detailed reason. No alternative deletion
method was attempted. Leftovers under `tmproject/build/tests/Release/`:
`crash-capture-tests-19060-188630968`,
`crash-capture-tests-15888-188758046`, and
`crash-capture-tests-23304-188809421`. Current incremental build outputs remain.

The owner subsequently authorized local installation for manual testing. The
unchanged validated candidate above was copied to `tmproject/client748/WYD.exe`;
the installed SHA-256 matches
`5CE379167DD1AF7DBE8F6C1C34BFE3AFB3A0CA039BB4B09FD9D13C9FCED384E3`.
Only the executable was replaced. This private revision-7 candidate preserves
the deployed packet/build contract, so this host correction requires no server
restart for local testing. No game was launched, no VPS release or launcher
patch was published, and actual client acceptance remains pending.

### Full client-area layout

The owner subsequently confirmed that the embedded shop looked correct; the
supplied screenshot shows its catalog, balance and a completed purchase result.
This is acceptance evidence for that scenario only. The requested presentation
adjustment now fills the entire game client area and follows owner resizing,
without the earlier 800 by 560 cap. The header and delivery footer remain
visible while the catalog scrolls independently. The close action is an X in
the upper-right corner, with existing English/Portuguese labels exposed as its
accessible name and tooltip. Pending purchases still disable close and Escape.
This is a client-only `MODERNIZACAO_COMPATIVEL`; the v3 wire and revision-7
admission contract are unchanged. No server restart is needed for local testing.

Validation passed: all six page tests (including localized close labels and
pending-purchase closure guards), architecture 3,176,834 checks, socket 281,
crash capture 200, launcher 58, asset/shader gates and canonical Release build.
The geometry checks now cover full-area sizing across the existing dimension
matrix. Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, 2,014,208
bytes, SHA-256
`7CC96BA988A486BB253B10DAC37903C323FBE7C5F33BA67A3E7A843734A7A145`.
Local installation was attempted but Windows rejected replacement because
`tmproject/client748/WYD.exe` was in use. Its previous hash remains unchanged.
No process was stopped or launched; installation awaits the owner closing the
client, and visual acceptance of the full-area layout remains pending. No VPS
release or launcher patch was published. Automatic approval review blocked
cleanup of `tmproject/build/tests/Release/crash-capture-tests-23412-189901796`
with `blocked by policy`; no alternate deletion was attempted.

### Official presentation mismatch reported by the owner

The owner supplied a modern official in-game store screenshot and clarified
that the requested result is a copy of that presentation, rather than the
custom dark/gold page implemented here. Embedded display and purchase success
in the earlier screenshot do not establish acceptance of this custom design.
The supplied official reference shows a red framed account banner, coin
balance, language controls, search, a town/category navigation strip, two
columns of framed compact item rows with item artwork and red purchase
buttons, and numbered pagination. Its external close placement is superseded
by the owner's explicit request for an upper-right close button and full-area
embedded display.

Read-only retrieval on 2026-10-26 reconfirmed that both known official
`wydmisc.raidhut.com` and historical `.com.br` `/neil/global.php` endpoints
return only a loading/redirect wrapper. Its observed destination is
`https://celular.tapiocahut.com/global_prod/landing-page?id=&server=0`;
the unauthenticated request returns HTTP 400, `The id field is required.`
The observed `/neil/` directory returns HTTP 403. No account identifier was
invented, no authenticated session was read, and no official game or browser
was controlled. Exact store HTML/CSS and artwork remain unavailable; the owner
was asked for the page URL or source assets. At that stage the full-area
candidate recorded above retained the custom design and did not reproduce
the official store. No further candidate installation was attempted.

### Captured official presentation and categories (2026-10-26)

The owner subsequently supplied the captured store HTML for several pages.
Read-only retrieval of the referenced public `StaticFiles/stylev4.css` and
art assets supplied the original banner, wallet, item frame, button sprite,
coin icons, search icon and language flags. These are a modern official web
presentation reference, not native 7.48 binary evidence. Reusing that
presentation inside the already embedded host is `MODERNIZACAO_COMPATIVEL`;
adding authoritative catalog categories and their wire metadata is
`EXTENSAO_COORDENADA` v4.

The page now uses that art, a red account banner, black background, gray
category strip, two columns of three item cards, numbered pages, search and
EN/PT controls. It fills the game client area and retains the owner's
upper-right close action. Search and category changes reset pagination;
catalog shrink clamps it. A filtered card still purchases its original shop
slot. Confirmation exposes quantity, refinement, additions and configured
effect bytes before the existing authoritative purchase flow. Page language
controls only change this presentation; they do not change account/game
language, channel or citizenship.

The staff editor adds a category selector. The API remains version 1 with
an additive optional `category` field; omitted values default to Featured.
Existing database JSON needs no migration. The server validates values 0
through 3, persists them with the existing optimistic catalog version, and
copies them to the staff response and v4 player snapshot. No existing item
is automatically reassigned to a town. The selector label and options support
EN/PT; the surrounding pre-existing staff editor is still English-only.

The local page contains no copied official authentication, checkout, tracking
pixel, third-party script or captured purchase link. CSP forbids external
scripts, frames, forms and fetch requests. Item thumbnails alone use the
fixed public pattern `https://wydmisc.raidhut.com/icons/global_prod/{id}.gif`
for numeric IDs 1 through 6499, without a referrer. Failed images show the
item ID. Thumbnail availability and modern-to-7.48 icon identity are external
dependencies; the renderer does not infer item stats or products from them.
Frames, controls and flags are embedded as RCDATA/data URLs, so they do not
depend on a live official page. Catalog, prices, item effects, Donate and
settlement continue to come from this project's server and staff panel.

Art provenance, fetched 2026-10-26: the PNG files below come from
`https://celular.tapiocahut.com/StaticFiles/`; the two GIF flags come from
`https://wydglobal.raidhut.com/media/mod_languages/images/`.
They are product assets under `tmproject/TMProject748/Resources/PremiumNell/`.

| Asset | SHA-256 |
| --- | --- |
| bar6.png | BA9224B3CFCCEEAC10A0630E42B1E2A7D251E5A655C7753F426ED21197E8952A |
| base3.png | BC56E56B2CBDE651A5AC66EA70F9A6ADBBA0FFDE86E0DD9778C58127450FFD7D |
| button5.png | E2AF0A4A50B45DF9512934A1F7406278DF8E2ABCB154966D22C274DBF57CCA65 |
| np_icon.png | 23B3862918813640D6496C5C921AF1D5F093C7BFAF06789751D5C4EBB22BB4CC |
| np_icon_small7.png | 6EC994439A657AE3C33F3CF1CAAB92ECF6D29BE47F73436A2B717CA4E9FBD166 |
| search2.png | 069BB16D6028AB4F7968896CF5A6B9522201AFE028C46E7740DFA79131AB4D41 |
| store_item_base2.png | C7E57F116FA1369BB5A916C2F92490AC7F25BBEE028E92B9007E1DC4C6C0A5D4 |
| pt_br.gif | FEF1D61DF14547C8D34E7A0F940D894D05EB55AA2D36E710E4AED19A78860AB8 |
| en.gif | 5D14E1355F6DC3FAE99DDA64BD1B376867196C66C2EB3DD07FFB18F8DD0D242B |

Focused validation passed for server wire/control/store/game Premium and
Donate tests, including real temporary PostgreSQL category persistence,
legacy JSON defaults, invalid categories and purchase/replay/rollback cases.
Nine page tests cover both locales, safe catalog/account text, category and
pagination selection, original slot preservation, thumbnail fallback and
pending settlement. The staff tests cover category editing and omitted/new
defaults in EN/PT. All 91 portal tests passed. Astro check reports zero errors,
warnings or hints, and the production portal build completed successfully.

An isolated headless Edge preview loaded all six fixture thumbnails and
reported no page exceptions. Layout checks at 1024x768, 800x560, 640x480
and 1600x900 found no document overflow, a visible upper-right close action
and positive card bounds; confirmation opened. The 1024x768 preview was
visually inspected. Preview products are test fixtures, not a production
catalog update. This is browser rendering evidence, not execution of the
built WebView host or real gameplay acceptance.

Categories require the new server, staff site and client together. A private
new client can read the active server's old catalog but sees Featured only.
The published client revision and server minimum are now 8. Mandatory patch
48008 passed public HTTP/HTTPS verification before the matching server/site
activated `/srv/wyd-go/releases/20261026T235538Z`, with explicit owner restart
and disconnection authorization. Both channels reject revision 7 and admit
revision 8 to credential validation. The old revision-7 client cannot consume
a v4 categorized open. Actual embedded display, input/focus, resize/close/reopen
and purchases with the updated catalog remain pending client acceptance. See
the [release receipt](../client/launcher.md#published-patch-748008-2026-10-26).

Initial private Release candidate (`Build-Client.ps1 -Configuration Release
-NoDeploy`): `tmproject/build/TMProject748/Release/WYD.exe`, 2,071,040 bytes,
SHA-256 `3261246FB159C9FE7BDEC1A6F1A39423CB32A8B5122AA2D76B9E71C7A696C2DC`.
Architecture 3,176,841 checks, socket 281, crash capture 200 and launcher 58
checks passed, together with asset/shader gates. The installed client was
not replaced. The repository-layout check exited 1 for pre-existing runtime
crash logs and `tmproject/client748.zip`. The ZIP was later recycled after its
file paths were confirmed in the current client directory. The scoped
whitespace check passed.

The canonical revision-8 release build has the same size, with SHA-256
`15C41E8A790C5BEDE9F8EB3F126E67F016556E86405C023F579F41AAF17AD4B2`.
The same canonical client gates and the focused wire/page/portal checks passed.

### Revision-8 open-state dispatch regression

The owner reported that the published update displayed the native merchant
grid instead of Premium Nell. The installed local executable matches the
canonical revision-8 SHA-256 above. The transport path is
`ObjectManager::OnPacketView -> received_packet::Dispatch -> TMFieldScene`.
Although the scene parser accepts the 160-byte categorized open, the earlier
dispatch gate still required the 132-byte v1 length for `0x7F20`. It silently
discarded v4 before the scene could initialize WebView2. The preceding shop
list still opened the native grid. This explains the observed fallback
without indicating a missing WebView2 runtime.

The client correction is `MODERNIZACAO_COMPATIVEL` within the existing v4
extension: dispatch delegates premium-state validation to the shared parser,
checking both the frame and metadata opcodes. It accepts v1 balances/opens
and v4 categorized opens, retaining their version, kind, size, category and
reserved-byte validation. Wire formats, purchases, catalog data and EN/PT
presentation strings are unchanged.

Regression tests now exercise the production dispatch gate before parsing and
applying the catalog, including purchase-slot preservation, legacy balance
updates, every truncated v4 prefix, oversized/null/unaligned frames, mismatched
opcodes and header size, wrong versions/kinds, invalid categories and reserved
bytes. Before the correction, the test executable exited 1 with three failures:
v4 delivery, purchasing from the delivered catalog and unaligned v4 delivery.
The canonical revision-9 client build and all client gates pass, as do the Go wire tests. The uploaded server/site archive matches all 7,797 file hashes; all 1,157 runtime data files are unchanged. Publication and activation are tracked in `.agents/research/release-748009.json`. Real embedded-client acceptance remains pending.

Automatic approval review rejected the exact-path cleanup command with
`blocked by policy`; no alternate deletion method was attempted. Current
task leftovers are `tmproject/build/tests/Release/crash-capture-tests-11880-191203218`,
`tmproject/build/tests/Release/crash-capture-tests-11252-191426671`, and
`C:/Users/xereca/AppData/Local/Temp/wyd-premium-nell-preview.cjs`. Prior blocked
paths listed above were not retried. The preview and validation logs remain
under the sanctioned `build/premium-nell-layout/` output directory for review.
