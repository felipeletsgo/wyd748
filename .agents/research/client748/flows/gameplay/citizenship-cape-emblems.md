---
id: citizenship-cape-emblems
title: Citizenship metadata and cape emblems
subsystem: appearance
status: CONTRACT
native_sha256: 8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593
updated: 2026-10-14
---

# Citizenship metadata and cape emblems

## Question

Why does an existing citizen have no cape emblem, and how can a committed
Kibita purchase update it without recreating a world entity?

## Evidence boundary

Native evidence: read-only `references/client748/WYD.exe`, hash above verified
against the existing Ghidra export in `tmproject/build/native-research/decompiled/`.
The project is `tmproject/build/native-research/WYD748.gpr`. Reused the located
appearance functions rather than importing or cataloguing the executable again.
Native textures and the existing `SetCitizenMantle` mapping were not changed.

## Native 7.48 flow

### Callers

`FUN_00524ded`, `FUN_00526140` and selection `FUN_0049f0e7` invoke the native
citizen mantle mapper. The observer snapshot receiver is `FUN_004829f2`.

### Callees

`FUN_00526cf4` chooses bounded emblem textures from the base mantle. Native
resource loading uses `FUN_004be7b1`; no new resource owner is introduced.

- `FUN_00524ded` reads citizenship from the third face-effect value (input
  offset 99, face begins at 92) before assembling equipment. It calls
  `FUN_00526cf4` after obtaining the base cape texture.
- `FUN_00526140` also calls `FUN_00526cf4` in the observer equipment path.
- `FUN_004829f2` receives the native CreateMob snapshot and reads its server
  byte at native offset `0xAC` into the human citizenship member at `0x7BA`.
- `FUN_0049f0e7` calls the same cape mapper in selection. The explicit
  selection metadata initialization added here is a coordinated source fix;
  this record does not claim that an equivalent assignment was located there.
- `FUN_00526cf4` normalizes citizenship outside 0..10 to zero. Zero leaves
  the base texture unchanged. Channels 1..10 choose the server emblem; these
  are not zodiac skills or buffs. Bases 34, 35 and 36 stay unchanged.

| Base texture | Variant for citizenship c |
| --- | --- |
| 19, 3, 2, 6, 1, 0, 7 | c + 39, 49, 59, 69, 79, 89, 99 respectively |
| 25, 24, 26, 28, 27, 29 | c + 109, 119, 129, 139, 149, 159 respectively |
| 32, 31, 33 | c + 169, 179, 189 respectively |

## Wire, ABI, and resources

The native layout is not the TMProject source ABI. The coordinated 328-byte
CreateMob uses byte **326**, not native offset 172, for `Server`.
`CreateMobWithGuildRank` takes the character's authoritative appearance emblem.
Existing player spawn, self login and metadata refresh callers supply it.
NPC builders retain their default server byte.

`CharacterFaceItem` copies the face item and replaces only `Eff[5]`, the
third effect value. It never writes back to the persisted item or account.
Selection packets and EnterWorld serialize this projection, including channel
zero, so stale effect bytes cannot masquerade as server-owned appearance state.

For the single-channel extension, `CapeEmblem` (JSON `capeEmblem`) stores a
random symbol 1..10 independently of real `Citizenship`. Face `Eff[5]` and
CreateMob byte 326 project this field; missing/invalid values fall back to bounded
citizenship for legacy compatibility. These bytes control appearance only:
kingdom chat, war participation and drop bonuses still use real citizenship.

After a successful Kibita commit, the owner receives projected face slot zero
via SendItem; visible observers receive the existing CreateMob metadata refresh.
The client updates citizenship and the cape on the existing entity without
calling InitObject or InitPosition in the snapshot-refresh branch. The owner's
SendItem path updates the selection cache even for face slot zero.

## State and lifecycle

| Transition | Authoritative outcome | Client outcome |
| --- | --- | --- |
| Login or selection reload | Read persisted CapeEmblem, with legacy fallback | Apply projected face metadata and cape mapping |
| Observer enters view | Read subject appearance emblem | Apply Server before equipment mapping |
| Kibita write pending | Live citizenship and gold remain unchanged | No reward/appearance publication |
| Certified write rejection | Retain prior citizenship and gold | No emblem or observer snapshot |
| Acknowledged commit | Install citizenship and payment together | Owner face and observer snapshot refresh |
| Purchase repeated | Nonzero citizenship rejected | No second payment |
| Character logout/relogin | Retain persisted random symbol | Rebuild from authoritative projection |

The mapping reuses existing human/cape resources; it creates no new resource
owner, vtable, timer, or shutdown hook. Guild-wide citizenship propagation is
not implemented by this buyer-only transaction. An inaccurate guild-success
log was removed; that separate feature must not be claimed as delivered.

## Current mapping

Go model state flows through detached critical proposals, durable account JSON,
face projection and CreateMob metadata into the existing C++ mantle mapper.
The single-channel extension and latest source repairs are detailed below.

## Delta matrix

| Claim | Classification | Evidence and boundary |
| --- | --- | --- |
| Native mantle variant mapping | PARIDADE_NATIVA | FUN_00526cf4, bases and bounded 1..10 variants |
| Independent random symbol and metadata projection | EXTENSAO_COORDENADA | Persisted CapeEmblem, source wire tests, unchanged appearance consumers |
| Arch template sanitation | MODERNIZACAO_COMPATIVEL | Authoritative model tests; historical DBSrv is provenance, not native proof |

## Decisions

- `PARIDADE_NATIVA`: the already existing texture mapper and the two restored
  equipment-path calls, backed by the native functions above.
- `EXTENSAO_COORDENADA`: source-ABI metadata projection, selection initialization
  and in-place committed refresh across Go and TMProject.
- `MODERNIZACAO_COMPATIVEL`: Arch template sanitation on the authoritative
  server. Historical DBSrv behavior is provenance, not independent native
  7.48 runtime proof. Face slot zero and inventory slots 60..63 are preserved;
  equipment 1..15 and inventory 0..59 are cleared as whole items.

## Gaps

Actual cape, kingdom, guild and tooltip rendering remains a client gate. The
native guild loader-to-label transition is only partially traced. Publication
and source tests cannot certify visual parity; detailed pending flows are below.

## Validation

`AUTOMATED TESTED`: all four Arch classes, full-item clearing and reserved-slot
preservation; critical Arch commit/rejection; face projection for channels
0..10 without model mutation; all four selection slots; EnterWorld; selection
refresh; critical citizenship commit/rejection and visible-observer publication;
repeat-purchase denial. Client source-contract tests exercise both equipment
consumers, selection ordering and non-respawning metadata refresh.

Full Go suite and vet passed. Focused race validation passed. PostgreSQL-specific
fixture-dependent tests must not be inferred from in-memory critical stores.
Client incremental Release build passed with 1,200,858 architecture checks,
231 socket checks and 56 launcher checks; shaders and costumes passed.
The SDK emitted its default `DIRECTINPUT_VERSION` informational message.

`STATICALLY VERIFIED` and `AUTOMATED TESTED`; not `CLIENT_TESTED`.
No native visual execution, runtime install, VPS restart, launcher publication,
commit or push was performed. Candidate:
`tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`FDE1DF8F88CE7D1C7099BE044235F24E6C2C846F40143BD27CDF1AF2B6DC6D10`.

Pending manual gates: acquire citizenship on a cape-wearing character; compare
owner/observer emblems; re-equip and relog; inspect selection; create an Arch
and verify its empty visible inventory. Gate-opening changes and historical
VPS staging paths from the pasted transcript were not redeployed or certified
by this batch.

### Authorized publication — 2026-10-14

The subsequent user-requested publication installed the complete current
server/site tree as `/srv/wyd-go/releases/20261014T011828Z` and published the
candidate above as the only entry in launcher patch 10011. The game and API
restarted after zero game connections and a successful backup. Public download
and executable hashes matched; readiness and the website receipt passed.
This adds deployment evidence, not `CLIENT_TESTED`: actual launcher extraction,
Arch creation and owner/observer cape visuals remain pending. The website build
and 60 frontend tests passed; player-role PostgreSQL integration and a disposable
backup restore passed, not a new database-specific Arch/citizenship proof.

### Single-channel symbols and failed visual acceptance — 2026-10-14

The user reported failures in published 10011: cape mesh/texture and kingdom
icon absent, guild ADMIN not displayed, and Foema requirements labelled Magic
Sword. This is negative visual evidence; the earlier build/publication did not
close those manual gates.

The user chose a random symbol on first kingdom cape acquisition, then requested
assignment for existing cape owners. `EXTENSAO_COORDENADA`: the server stores
`CapeEmblem` separately from citizenship and projects it through the existing
appearance-only fields. Values 1..10 reuse the proven native mapper. Neutral
capes do not receive a new symbol; valid symbols survive promotion, re-equipping,
opposite-kingdom acquisition, entry and reload. Critical proposals are detached:
no payment, item, symbol or owner/observer publication before acknowledgement;
certified rejection leaves the old state intact. Repeated pending requests do
not reroll. Legacy entry migration is saved before entering the world.

The runtime contains native citizen textures 41..200, but its binary model-texture
directory did not register them. Source now appends missing entries without
moving existing IDs or partially updating a full table. Texture 194 shares the
native mantle mesh; `FUN_004be7b1` is the native loader boundary. The kingdom
panel accepts base coordinates 0..36, not emblem variants 40..199; source now
inverts `FUN_00526cf4` before setting the icon and handles neutral/helmet variants.
No historical binary or runtime asset was patched.

Related source repairs reuse native evidence: `FUN_00557593` and sscanf call
`005576bd` establish guild-directory parsing; the complete native loader-to-label
transition is not traced. Guild cache validation includes ADMIN, malformed rows,
group/channel bounds, reload and missing files. Skill tooltip `FUN_00418828`
establishes class/lineage grouping; source uses typed 32-bit mastery rather than
a WORD walk across unrelated score members. Tests cover all class book families
and Foema book 5046 requiring Special Magic 76 against mastery 200.

Current gates: full Go suite/vet PASS; focused model/store/game race PASS;
wire projection tests cover all ten symbols and all four selection slots.
JSON-store reopen preserves symbol, citizenship and cape. SQL fixture checks
preserve unrelated state, null slots, existing symbols and empty accounts; its
22-item allowlist is tested against every model cape ID. Incremental client
Release build PASS: 1,205,501 architecture, 231 socket and 56 launcher checks,
plus shader/costume gates. Existing Winsock C4996 warnings remain nonfatal.
Candidate `WYD.exe` SHA-256:
`F358A2711A2AF7C54EB044F06DFF5B9DDDF457ED14F3F4303D4D4D1B87152319`.
At that validation point it was not installed or launcher-published. The
subsequent authorized publication below does not make it `CLIENT_TESTED`.

VPS release `20261014T020943Z-cape-emblems` updates both strict-reader binaries,
retaining the preceding site/config/data. With zero established game connections,
both services stopped before backup `wydgo-20261014T020944Z.dump`. The SQL
transaction assigned two characters in two accounts; the second pass changed
zero rows, and the live range check found zero unassigned eligible capes.
Game/API readiness passed, public sites returned 200, and the bounded warning
journal was empty. These are database/deployment receipts, not visual proof.
Never restart the old JSON reader after backfill without coordinated data rollback.

Remaining: verify cape texture,
kingdom icon, ADMIN guild text and Foema requirements in actual gameplay;
compare owner/observer/selection symbols across relog and promotion. Current
server backfill does not by itself repair the old installed client texture table.

### Authorized client publication — 2026-10-13 America/Sao_Paulo

The user requested uploading the pending client repair. Patch 10012 now contains
only the candidate above, 1,860,608 bytes. ZIP: 818,787 bytes, SHA-256
`E5E051264833A2AB232770B6178EE23BD2F5E453177653EB4D711CA157D2C3B9`.
Fresh public download verified both hashes and the sole root entry. Plain-HTTP
launcher endpoints passed; the website receipt and pages now advertise 10012.
Portal build and two public-content tests passed. Only four receipt pages were
updated, preserving all shared assets and keeping a backup. Neither game nor API
restarted; unchanged process identities and readiness were verified. No local
runtime install, commit, push or real-client execution occurred. Actual launcher
extraction and the remaining manual flows above are still open.

### Cape refinement glow repair — 2026-10-14

This repair is separate from emblem assignment and citizenship. The full
appearance path previously set `m_ucMantuaSanc` from `TotalKill / 100`, while
the compact equipment path used `TotalKill / 10`; neither value describes
cape refinement. The existing renderer already consumes `m_ucMantuaSanc`
when assigning the cape skin mesh's refinement material.

`CapePresentation748.h` now clamps refinement to 0..15 and decodes the same
upper equipment nibble sent by `VisualItemCode`. `SetPacketMOBItem` uses
`BASE_GetItemSanc` for equipment slot 15; `SetPacketEquipItem` uses that
slot's packed refinement. Removing the cape clears stale refinement. Emblem
and citizenship projection, mesh identity and the wire ABI are unchanged.
Classification: `MODERNIZACAO_COMPATIVEL`. The native emblem functions above
do not by themselves prove this glow behavior, so this adds no parity claim.

`NativeScoreHudTests` exercises every 0..15 full/packed refinement value for
representative cape IDs, clamping and the two source entry paths. The
[local candidate receipt](../../../../../DOCS/current-state.md#local-combat-cape-and-kefra-candidate--2026-10-14)
records the incremental Release build and automated gates. No game was
launched; visible brightness at each refinement, owner/observer agreement,
equipment changes and relogin remain manual acceptance gates.
