# Project continuation — 2026-10-14

Use [current project state](../../DOCS/current-state.md) first. It holds the
verified deployment/client identities, validation boundaries and next queue.
The [website matrix](../../DOCS/server/website-feature-status.md) reconciles
the historical platform plan. Older feature notes below are historical, not the
latest deployment identity.

## Entry and preservation

This task entered `main` at `2f2381f9a8e77f23eb07bb10b326df76568d3bdb` with
substantial preexisting uncommitted changes. Preserve them. No commit, push,
branch, worktree or PR was created. Do not infer that a previous VPS deployment
published every file in the dirty tree.

Already inspected: repository rules and architecture/server/client research
skills, scoped changes, operation/deployment/client records, public website
routes, installed client hashes, and read-only live VPS service/schema/log state.
Do not repeat those inventories just to resume; verify only inputs that changed.

## Latest request and changes

The owner requested publication of all pending updates and authorized player
disconnection. This is complete: server release `20261014T234237Z`, launcher
patch 10024 and its website receipt are published. Shutdown saved three accounts
with zero failures; both services restarted, public readiness returned 200 and
the bounded warning journal was empty. Public ZIP/executable hashes match the
manifest. See current-state and its deployment links for exact identities,
backup, rollback and cleanup receipts; do not duplicate them here.

Delivered batch: mount magic and native glow, green named trumpet chat separated
from top server notices, party Leave/Escape handling, Mystic Amulet slot 9,
equipment mesh sizing, normal Lac for equipped +9 earrings, Noatum generator
suppression/Erion tower placement, and 49 mortal D/E test-shop weapons.
The three existing Armia shops supply magic +28 or physical +63 stock; no new
shop generators or item IDs were added. Aylin previews/glow were already shipped
in 10023 and remain included.

Validation was reused where inputs were unchanged: full Go suite/vet and
Release/Win32 client gates passed; focused shop data checks, Linux server build,
portal build and all 60 portal tests passed for publication. No local runtime
overwrite, client launch or CLIENT_TESTED claim. The current-state acceptance
queue lists the required gameplay checks. The item-drop stack overflow is still
unresolved; preserve the dump and matching 10021 executable. Noatum suppression
does not implement the full Sunday castle-war controller. Kefra remains at
temporary 1 HP for testing; restore 28,000 HP after acceptance.

No new commit or push. Preserve the dirty tree, rollback release and player state.
Both remote staging directories were removed; local cleanup was rejected before
execution and exact ignored leftovers are recorded in current-state. Do not
bypass that rejection or repeat publication just to resume.

### Previous cape-emblem publication

The earlier request was to apply random cape symbols to existing cape owners.
That batch was published in release `20261014T020943Z-cape-emblems`;
two characters in two accounts
received symbols 5 and 6. Backup `wydgo-20261014T020944Z.dump`, zero active game
connections, transactional backfill, zero-change second pass, range check and
game/API readiness passed. Both strict account readers were replaced; do not
downgrade only a binary after adding `capeEmblem`. Site/config/data were retained,
not rebuilt. The subsequent requested client publication added launcher patch
10012; see current-state for binary receipts.

Random 1..10 `CapeEmblem` is independent of citizenship; first realm cape grant
and legacy entry migrate missing symbols. Promotion/re-equip/kingdom changes
retain it; critical rejection publishes nothing. Full Go suite/vet and focused
model/store/game race tests passed, including JSON reopen and independent Kibita
publication. The model test checks the SQL allowlist against every cape ID.

The user reported failed visual acceptance of 10011. Source repairs now cover
missing mantle texture registrations, kingdom base icons, guild names and
Foema skill requirements. Incremental Release candidate passed all gates:
SHA-256 `F358A2711A2AF7C54EB044F06DFF5B9DDDF457ED14F3F4303D4D4D1B87152319`.
It is now launcher-published as 10012; local runtime was not overwritten and
actual visual execution remains open. Fresh download matched ZIP/executable
hashes. Portal build and two public-content tests passed; only four receipt pages
were deployed, with shared-asset checks and a previous-pages backup. Server/API
PIDs remained unchanged and readiness passed. No restart or new backfill occurred.
No commit/push. Preserve the large dirty tree. Do not repeat the full suite/build
without an invalidating input. The requested publication is complete; next work
is actual launcher extraction and in-game validation, not another deployment.

### Previous Arch/citizenship continuation

Latest local continuation: the pasted Claude/Codex transcript stopped midway
through Arch/citizenship work. The four outdated CreateMob test call sites now
compile. Face metadata is projected without mutating persisted items; selection,
login and a committed Kibita purchase reach the owner and visible observers.
The client maps citizenship in both world equipment paths and selection,
refreshing an existing entity without respawning it. Arch sanitation is tested
for all classes, complete item clearing, reserved slots and critical failure.

Full Go suite and vet, focused race tests and the incremental client Release
build passed. Details and pending manual gates:
[citizenship cape emblems](../research/client748/flows/gameplay/citizenship-cape-emblems.md).
Candidate SHA-256: `FDE1DF8F88CE7D1C7099BE044235F24E6C2C846F40143BD27CDF1AF2B6DC6D10`.
The user subsequently authorized full VPS publication. Release
`20261014T011828Z` and launcher patch 10011 are live. Both services restarted at
01:18:31 UTC after zero established game connections and a successful backup.
The portal passed check/build and 60 tests; isolated player PostgreSQL integration
and a backup restore passed. Readiness, public pages, feed, manifest and fresh
ZIP/executable hashes were verified. See the updated current-state table and
deployment ledgers. No local runtime install, commit, push or visual execution.
Do not treat the pasted historical staging path as a current release receipt.

### Earlier Hidden Valley batch

The user clarified that this version has no Valley Fairy: access must be
exclusive to the activated Hidden Ruins map. Implementation adds the existing
Azran portal route, server destination/walking eligibility and expiry recall.
Existing timed-access consumption and S2 settlement are reused, not replaced.
No C++ source, assets, native reference or item duration was changed here.

New game files: `internal/game/hidden_valley.go` and `hidden_valley_test.go`.
Modified feature consumers: `teleports.go`, `player_movement.go`, `world.go`,
`data/teleports.ini` and `internal/data/teleports_test.go`. Durable coordinates
use the neutral login policy: do not assert that a persisted paid-teleport
proposal stores live Valley coordinates. The acknowledged callback owns them.

The terrain test requires installed maps, verifies 6,200 reachable cells inside
the protected rectangle and reports zero enabled starting NPC generators.
Do not claim the Valley is populated or invent monsters/drops to close that gap.

Passing: full Go suite, vet, focused timed-access/S2/Valley tests, focused race
tests and native server compilation. Store PostgreSQL fixture was configured;
dedicated player/staff database fixtures were not. No new C++ build or visual
test was needed/performed. Output `bin/tm-hidden-valley.exe` is ignored and is
not the live Linux artifact.

## Next actions and authority

The requested server/site/client publication is complete. Do not repeat it or
restart the shared VPS as a resume ritual. The user must receive 10024 through
the launcher before in-game acceptance. Do not launch the client automatically:
the user's Windows/visual-testing constraint remains in force. Repairing the
reported crash and testing the published gameplay flows are separate tasks.
Further publication requires a user request and connection/backup/release checks.
A server/data-only Valley change needs no launcher patch.

Manual client access/expiry validation, Valley content, full-client provenance,
buff/weather/idle/launcher gates, English migration, and S2 unknown-outcome
recovery remain open. Current code and newer live checks override this record.
