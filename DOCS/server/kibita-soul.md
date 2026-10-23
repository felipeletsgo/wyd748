# Kibita Mortal Soul

Implementation and evidence review: 2026-10-23. Status: source implemented and
automatically tested; installation, publication, and gameplay validation pending.
Classification: `MODERNIZACAO_COMPATIVEL`, using the existing NPC, skill,
inventory, cape, score, and affect contracts. This is not a claim of native
7.48 gameplay parity.

## Authority and recovered behavior

The requested authority is the supplied official **TMSRVIN 7.59** decompilation:
`references/decompilacao server/descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c`.
W2PP is not the implementation authority. The supplied server version is 7.59;
it must not be described as a 7.48 server decompilation.

- NPC dispatcher `sub_4F6D60` at `0x004F6D60`, case `74` (`0x4A`),
  lines 36363-36500: citizenship channel, Mortal evolution, stored level
  at least 370, class-specific Secret Stone, hero cape, and learned bit
  `0x40000000`.
- Score routine `sub_4BB4C0` at `0x004BB4C0`, affect `29`,
  lines 17166-17237: the fixed STR/INT/HP branch without that learned bit,
  and the percentage configurations with it.
- Saved Ghidra dispatcher evidence:
  `build/rea-analysis/results/TMSRVIN.exe-fame-004f6d60-evidence.json`,
  evidence ID
  `ev_4673820710a1fafc9398adcabe2f8e7446285e51bc0896b410a864b4e5db4f0e`.
  This independently confirms the dispatcher constants and mutations.
- Official binary: `references/decompilacao server/dlls/TMSRVIN.exe`, SHA-256
  `27EE313BB4B4D4A0D897DAC9A188D8249068CE701311CBDA1E24AD28C5E43642`.
  Related binary identity is recorded in the
  [server runtime map](../../.agents/research/rea-server-759-runtime-map.md).

The inspected official Kibita case does **not** contain a Lac exchange. The
score routine establishes the fixed effect, not its item recipe, duration,
schedule, or level window. Those exchange rules below come from the user's
request and explicitly retained historical policy. This bounded finding does
not establish absence throughout the official binaries.

## Exchange contract

Both exchanges require a living Mortal with citizenship in the current channel.
An initial interaction without citizenship still uses the existing configured
citizenship purchase. The configured citizenship price is outside this change.
NPC visibility, distance, gameplay instance, and quest availability are validated
by the existing NPC handler before the exchange.

### One-hour fixed Soul

- Consume one unit of Lac scrap, item `420`.
- Eligibility: displayed level below 370, equivalent to stored level below 369,
  without the learned Soul bit.
- Opening: Monday through Friday, from 21:00 inclusive to 22:00 exclusive,
  using the server clock's local timezone. This is the schedule selected by
  the user, not a recovered official Kibita timer.
- Grant affect `29` for 450 existing eight-second units, exactly one hour.
- Effective STR and INT are fixed at 2,000; maximum HP is fixed at 10,000.
  Base attributes remain unchanged. Other HP effects and the held-experience
  penalty do not override the fixed HP contract; normal calculation resumes
  when the effect expires. Current HP remains governed by existing vital logic.
- An active Soul or a full affect array rejects the exchange before payment.
  The skill is not permanently learned. No teleport is introduced.

### Learn Soul Limit with a Secret Stone

The official test is stored level `>= 370` (`0x172`). The current zero-based
level contract displays this as **level 371**. Displayed level 370 therefore
qualifies for neither exchange. This preserves the official unlock comparison
and the separately selected temporary-Soul level window.

| Mortal class | Secret Stone item |
| --- | --- |
| TransKnight, class 0 | 5334 |
| Foema, class 1 | 5336 |
| BeastMaster, class 2 | 5335 |
| Huntress, class 3 | 5337 |

The exchange consumes one unit, sets learned bit `0x40000000`, and replaces the
cape with the current kingdom's hero cape (3194/3195/3196). It enables the
existing Soul Limit skill 102; learning does not cast it automatically. A
repeated interaction cannot consume another stone.

Compatibility adaptations are explicit: the official code clears the selected
inventory slot, while the current stack contract consumes one unit. The Go
kingdom/cape model supplies the cape variant. The current live update packets
publish the result without adding a forced logout. The existing skill cast
duration remains 38 eight-second units for Mortals, independently of the
one-hour Lac effect.

## Learned Mortal Soul attributes

The official learned-Mortal branch uses the current attributes, including
equipment, and multiplies before integer division by 100. Invalid `SoulInfo`
values default to configuration 1. Each primary attribute is capped at 60,000
inside this Soul calculation.

| SoulInfo | STR bonus | INT bonus | DEX bonus | CON bonus |
| --- | --- | --- | --- | --- |
| 1 | 40% | 0 | 0 | 30% |
| 2 | 0 | 40% | 0 | 30% |
| 3 | 0 | 0 | 40% | 30% |
| 4 | 40% | 0 | 30% | 0 |
| 5 | 0 | 40% | 30% | 0 |
| 6 | 0 | 0 | 30% | 40% |
| 7 | 60% | 0 | 0 | 0 |
| 8 | 0 | 60% | 0 | 0 |
| 9 | 0 | 0 | 0 | 60% |
| 10 | 0 | 0 | 60% | 0 |

The existing Go formulas derive attack, defense, HP, MP, and speed from these
attributes. This change does not establish parity for the entire official
derived-score routine. Existing Celestial/SubCelestial Soul configurations,
Ehre recipes, and formulas remain unchanged; the Mortal table is distinct.

## Persistence and existing client boundary

Payment, cape/learned bit, and the affect sidecar share the existing account
transaction. With critical persistence enabled, the live character and outgoing
reward packets remain unchanged until actor acknowledgement of the committed
write. A pending exchange reserves the player against repetition. Certified
rejection restores the original state; the legacy synchronous path also rolls
back on save failure.

The effect sidecar stores absolute expiration. Reconnection preserves remaining
time and discards an expired effect. Owner inventory/skill updates, public and
private affects, vitals, and cape appearance use existing packets. No opcode,
layout, client source, or asset change is introduced.

Existing native 7.48 boundary evidence is reused:

- [Citizenship and cape emblems](../../.agents/research/client748/flows/gameplay/citizenship-cape-emblems.md),
  status `CONTRACT`, for owner and observer appearance.
- [Persistent buff visuals](../../.agents/research/client748/flows/combat/persistent-buff-visuals.md),
  status `CONTRACT`, for Soul presentation and affect lifecycle.
- Native reference `references/client748/WYD.exe`, SHA-256
  `8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593`.
  Current client `HasSoulSkill()` already reads learned bit 30; the server's
  existing skill validator admits skill 102 through the same bit.

## Validation and delivery

Implementation: `wydgo748/internal/game/kibita.go`, the validated NPC route in
`handlers.go`, affect calculation in `affects.go`, and the fixed-HP interaction
in `equipment.go`. Regression coverage is in `kibita_test.go`.

Passed:

- `go test ./...` and `go vet ./...` from `wydgo748/`.
- Focused tests for all four stones, duplicate requests, eligibility, invisible
  NPC rejection, schedule edges, one-hour expiry, reconnection, all ten Mortal
  configurations, invalid configurations, rounding, equipment attributes,
  60,000 caps, competing HP effects, held experience, existing skill casting,
  save rollback, and publication only after critical commit.
- The game package and its vet check were repeated after the final score fixes.

`git diff --check` passed. The documentation inventory was updated. The
repository layout validator reported eleven pre-existing ignored
`crash-monitor-p*.log` files under `tmproject/client748/dumps/`, outside its
allowed output locations; it reported no documentation placement or link
errors. Those unrelated runtime dumps and the existing client configuration
change were preserved. Release staging created during the subsequent deployment
remains at `build/deploy/wyd-deploy-20261023-kibita-soul/`: execution policy
rejected local recursive cleanup. The remote staging directory was removed by
the successful updater; local deletion was not retried.

Actual built-client skill UI, casting visuals, owner/observer appearance,
scene transition, death, and live relogin remain pending. This change is not
`CLIENT_TESTED`.

## Production delivery

Published with the owner's restart authorization on 2026-10-23 as
`/srv/wyd-go/releases/20261023T173911Z`; the game resumed listening at
17:39:16 UTC. See [current state](../current-state.md) for executable/archive
hashes, backup, rollback and verification. No client source or asset changed;
the public launcher remains at patch 10045 and both compiled handshake
constants remain at revision 3. No client patch was required or published.

Build inputs: HEAD `76626439342037f92b72518b94745ea4c795fc74` plus the
following explicit working-tree allowlist. The two new files were untracked
and included by the normal Go package build; no commit or push was performed.
All paths below are relative to `wydgo748/internal/game/`.

| File | SHA-256 at build time |
| --- | --- |
| `affects.go` | `373BCE469BFE63E3DB00CB0E625BF1CC8581B89070379514C271FC9BD33E746F` |
| `equipment.go` | `ECC98FCBA7839CD9F22D305685E43D22BA8CA190499F34240F68A76BC2F75581` |
| `handlers.go` | `FA6FA7C257D83074E20BCD6C652B4444C8E9D056A05C0E96CB9BEE3713B7F8DE` |
| `kibita.go` | `2B3FC7733ABAA03EF0BC6DA477750B1EAC4787CE8A9C2FBAC19C48051261F37B` |
| `kibita_test.go` | `3F5336EF1C4CC318C8552F3CDF1E125D95873CC5F34ED8248F7A3183B7F3BC94` |

The archive contains the required runtime data and public/admin website but
excludes private account state, local client settings and client executables.
The server process has no `TZ` override and the VPS uses `Etc/UTC`: the selected
21:00–21:59 **server-time** exchange window currently means 18:00–18:59 in
America/Sao_Paulo. Deployment did not change the host or process timezone.

Cape catalog verification also passed: server `data/itemlist.csv` and the
loaded client root `ItemList.bin` both contain hero capes 3194/3195/3196,
mesh 27, textures 27/28/29, DEF 200 and HP 220. The client loads the root
catalog through `WYD748_LoadItemList(".\\ItemList.bin", ...)`; no asset
modification was needed.
