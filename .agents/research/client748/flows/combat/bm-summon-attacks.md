---
id: bm-summon-attacks
title: BM summon attack visuals and reach
subsystem: combat
status: CONTRACT
native_sha256: 1FA945EB589B6CD8B2E1EA1D1B1AE83E5BDE66B5251344B4AC98B025489125C1
updated: 2026-11-09
---

# BM summon attack visuals and reach

## Question

Which attack motions, projectiles, and ranges belong to each BM summon?

## Evidence boundary

- Client 7.48 PT: local reference `WYD.exe`, `ItemList.bin`, and `mesh/MeshList.txt`.
  The executable hash above applies to every client address in this record.
- Server 7.59: TMSRVIN decompilation and the supplied binary NPC templates.
  These sources establish the imported combat selection and distance algorithm.
- Project: 7.48 item and skill catalogs, Go server, and adapted TMProject748 client.

## Native 7.48 flow

### Callers

The attack receive path checks skill at packet offset `0x18`. Sites
`0x00487863` and `0x00487AB0` admit skill 105 to the timed attack event.
The arrow path checks that skill at `0x00455A2E`.

### Callees

At `0x0048A9BF` and `0x0048A9CB`, the attack receive path selects
106 and 108. It calls `0x0051F08B` from `0x0048AA0A`.
Variant 0 checks tiger class 35. Variant 1 at `0x0051F1FE` uses the beam
constructor without a Succubus class check.
The arrow constructor selects mesh 919 for type 105 and parameter 1
at `0x004C2E18–0x004C2E37`.

## State and lifecycle

The server selects the target, motion, effect, and damage on each attack.
The receiving client animates the visible attacker and dispatches its effect.
The server owns HP, debuff expiry, and debuff source attribution.
The owner's combat target and RuntimeID continue to govern summon eligibility.

## Wire, ABI, and resources

The existing attack prefix carries the attacker at 12, skill at 24, motion
at 28, effect parameter at 29, and the single target at 44. The adapted
`0x39D` packet retains its 60-byte DMGX form and 32-bit damage at 56.
Mesh list entry 919 resolves to `mesh/stone08.msa` in both client trees.
The 7.48 `ItemList.bin` has 6500 records of 140 bytes. Decode each record
with XOR `0x5A` and read effect 27 in the effect tuples after `0x50`.

## Current mapping

- `bm_summon_attacks.go` and `skill_summons.go` select motion, reach, and
  spell. `mob_ai.go` stops approach at the same distance used for the hit.
- `TMHumanEffects.cpp` and `TMArrow.cpp` show the rock. `TMHumanCombat.cpp`
  and `TMFieldSceneCombatPackets.cpp` route the breath, beam, and spell visuals.

## Delta matrix

| Claim | Reference | Previous project behavior | Local correction |
| --- | --- | --- | --- |
| Reach | 7.48 face `EF_RANGE` | One cell for all eight | Use catalog reach per face |
| Motion and skill | 7.59 template and `0x004DECC0` | One physical packet path | Select the near or far attack, then roll the spell slot |
| Rock | 7.48 skill 105, parameter 1 | BM never emitted it | Emit the existing rock event |
| Breath and beam | 7.48 skills 106 and 108 | BM never emitted them | Route to Fire variants 0 and 1 |
| Succubus beam | 7.48 shared Fire variant 1 | Adapted class override used MagicArrow | Route the BM summon to the shared variant |

## Decisions

`PARIDADE_NATIVA` applies to the imported attack profiles and their verified
7.48 client effects. TMSRVIN 7.59 supplies the selection and grid distance
evidence. Use the 7.48 catalog for spell duration. Keep project accuracy,
PvP rules, and the owner-command lifecycle.

## Gaps

The exact 7.48 server combat selection was unavailable. Real-client timing,
sound, animation, and rendering remain pending.

## Validation

- Static: compare template fields, item ranges, packet dispatch, and mesh identity.
- Automated: exercise all eight reaches, spell selection, diagonal movement,
  owner and observer packets, misses, and debuff immunity.
- Real client: pending.
