# Hell Gate store (0x2BD)

Classification: `EXTENSAO_COORDENADA` on the server, restoring a native 7.48
client flow. The client side and the packet are native (`PARIDADE_NATIVA`
evidence below); the server rules are a reconstruction approved by the project
owner on 2026-10-03, because no 7.48 server source records them.

## Native client flow

| Step | Native function | Behavior |
| --- | --- | --- |
| Open | `FUN_0045ee28` → `FUN_0044a3a9` | Clicking an NPC with `ID >= 1000` and head 51 while the player is at `x/128` 13-14, `y/128` 28 stores the NPC ID and toggles panel 6185. It is checked before every merchant-type branch. |
| Choose | `FUN_004662c5` | Buttons 6193-6196 select the category and show its text from `UI/hellStoredesc.txt`; 6192 opens the confirmation box. |
| Confirm | `FUN_004640e5` | Finds free inventory space (2x4 weapon, 2x3 armor, 1x1 amulet, 2x4 random) and sends 0x2BD. |

`UI/hellStoredesc.txt` (in the 7.48 runtime) states the rules: a random item
of the category, random options, no refinement, unlimited purchases, the
inventory must have space, and items can be sold back to the store.

## Packet

Client to server, 24 bytes (native size gate 24):

| Offset | Type | Field |
| --- | --- | --- |
| 0 | header | `Type = 0x2BD`, `ID` = player |
| 12 | u16 | store NPC ID |
| 14 | i16 | category: 1 weapon, 2 armor, 3 amulet, 4 random |
| 16 | i16 | Carry slot, `x + 9y` in the 7.48 9x7 grid |
| 20 | i32 | zero; ignored |

The server answers with `UpdateCarry` then `UpdateEtc` on success. Domain and
save/admission refusals use a message panel; malformed, unavailable or reserved
participants are rejected without a purchase response.

## Server rules

- NPC `Hell_Store` (head 51) spawns at (1746, 3670), in the Hell Gate entrance
  room. Global NPCs stay visible inside the Hell Gate instance.
- Prices: weapon 9,000,000, armor 6,000,000, amulet 5,000,000, random
  7,000,000 gold. Random picks one of the three categories with equal chance.
- Pools, derived from the catalog:
  - weapons tier 9-10: base-line weapons (index 800-949, position 64/192)
    with `EF_ITEMLEVEL 4` and an ancient version; 20 items (Gram, Bhopal
    Sword, Kaladbolg, and so on). Ancients are excluded;
  - armor tier 7-8: base-line armor (index 1000-1799, positions 2, 4, 8, 16,
    32) with `EF_ITEMLEVEL 4`, an ancient version and `EF_MOBTYPE 0`; 120
    items (Dwarven Plate and Golden Embersed bands for every class and part);
  - amulets tier 1-3: Silver, Gold and Mystic, 551-562.
- Options: every item keeps `EF_SANC +0` (no refinement). Armor draws one pair
  from the native `SetItemBonus2` pools also used by Repliction
  (`data/repliction.json`). Weapons use the weapon branch of the native drop
  generator `SetItemBonus` at level difference 0 (two slots of `EF_ATTSPEED`,
  `EF_DAMAGE`, `EF_SPECIALALL`, `EF_MAGIC` or `EF_UNIQUE`). Amulets get no
  options, as `SetItemBonus` skips accessories.
- Refusals: wrong session ID (security violation), NPC not visible or too far,
  NPC is not `Hell_Store`, unknown category, slot outside the Carry or
  occupied, not enough gold, save failure (gold and slot roll back).

Code: `wydgo748/internal/game/hell_store.go`; client open path:
`TMFieldScene::MouseClick_NPC` in `TMFieldSceneInput.cpp`.

## Critical persistence migration and activation

The internal S2 migration is `MODERNIZACAO_COMPATIVEL`, not a new store contract.
When a coordinator is configured, a detached account proposal contains the charge
and the already materialized item UID/options. Live inventory/gold and the two
success frames are applied only after successful identity-checked World delivery.
Pending requests cannot reroll or charge again. Queue admission failure leaves
economic state unchanged without inline persistence. A reserved trade counterpart
prevents cancellation; otherwise cancellation remains at the existing point,
including before later validation/admission failures.

The bare `NewWorld` constructor without persistence options retains the
synchronous save/rollback path for legacy fixtures. The production server
configures `WithCriticalPersistence` and `WithCriticalAccountContainment`:
an unknown account-only outcome isolates its participants behind a quarantine,
without success publication or a stale final save. Shared guild/instance
failures enter global maintenance. Neither policy certifies an unknown outcome
as rollback. The October 7 audit identifies other shared-object containment
gaps; production activation does not imply that all S2 failure paths are closed.
See the [critical persistence plan](../server/critical-persistence-plan.md).

## Validation

- Server: `hell_store_test.go` (pools, option tables, purchase per category,
  refusals, rollback) and `internal/data/hell_store_npc_test.go` (spawn inside
  the native region on a walkable tile).
- Coordinator path: `critical_persistence_purchases_test.go` covers detached
  proposals, repetition/rejection, trade reservations, unknown outcomes,
  disconnect/readmission, shutdown snapshot contents and actual World-loop
  progress. Activation and autosave have separate tests and recorded evidence
  in the critical persistence plan; these purchase tests alone do not prove
  those guarantees or production-local containment for every purchase failure.
- Client: `HellStoreClickContractTests.cpp`; architecture tests pass.
- Pending: `CLIENT-TESTED` (open, buy each category, refusals in the real
  client), and selling back to the store, which uses the 0x37A sale flow and
  is not yet connected to `Hell_Store`.
