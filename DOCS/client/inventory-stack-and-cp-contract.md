# Inventory stacks and CP presentation — contract v1

## Scope and evidence

This October 7, 2026 batch fixes split captions, item quantity rendering, stack
merging, and CP/name updates. The native boundary is recorded in
[the UI evidence record](../../.agents/research/client748/flows/ui/grid-item-quantity-and-cp-names.md).
No opcode, packet size, persisted field, or historical binary was changed.

Quantity rendering and player name interpolation are `PARIDADE_NATIVA`.
The blank-resource fallback, server-side stack policy, level reward, and white
NPC names are `MODERNIZACAO_COMPATIVEL`. Native NPC creation supplies raw CP 75,
which is yellow; white NPC names are deliberately a product policy, not a claim
that the native executable already does this. Historical server code awards
five CP per level, but it is supporting evidence, not native 7.48 authority.

## Quantity and split UI

Effect/add 61 is the byte quantity. Mount indices 2330–2389 use it as mount HP
and remain excluded from stack amount rendering and merging. The item visual
and its amount/refinement text are independent draw-list nodes. Both normal
grid and centered drag rendering enqueue nonempty text; teardown removes it.

Shift-click retains the existing split packet contract. The active
`client748/UI/strdef.bin` row 408 is empty. `SGridInput` therefore supplies
`Enter the quantity to split.` for absent/empty/whitespace-only captions;
nonempty resource text is preserved. No resource file was overwritten.

## Authoritative stack policy

The existing 20-byte `0x376` intention carries the clicked target in `st/sp`
and the dragged origin in `dt/dp`; the warehouse target ID retains its native
offset. Existing player/account fences, slot validation, warehouse visibility,
range, and trade-participant checks run before merge planning.

Compatible items must have the same nonzero index, positive effect-61 amount,
equal other effects, and equal activation/expiry timestamps. Effect order does
not affect compatibility, but duplicate effects and malformed empty pairs are
rejected. Distinct instances with the same nonempty UID are not merged.

Transfer is `min(255 - targetAmount, originAmount)`. The target retains its UID.
A residual origin retains its UID; a fully consumed origin is cleared. A full
target is acknowledged and resynchronized without swapping or writing. An
incompatible pair continues through the existing ordinary swap flow.

The S2 coordinator receives a detached account proposal and reserves the
account/participants. Repeated requests while pending or ready-but-unacknowledged
cannot submit or publish another mutation. Trade cancellation uses the existing
coordinated account-write path. There is no synchronous save fallback when the
coordinator is unavailable.

After durable acknowledgement, the World owner installs the two cells and sends
`0x376`, then the target and origin `0x182` snapshots. This order is mandatory:
the native acknowledgement swaps local grid cells before the authoritative
snapshots correct them. Certified rejection preserves the preimage, releases
the reservation, and resynchronizes both cells. Unknown outcomes retain failure
containment and never acknowledge, replay, or roll back a potentially committed
merge.

## Item mouse shortcuts — October 18, 2026

- Ctrl + right click: equip from inventory or unequip into the first fitting
  inventory space, subject to the existing equipment requirements.
- Shift + right click: move inventory to warehouse or warehouse to inventory
  while the warehouse is open. Ctrl + left click remains a supported alias.
- Shift + left click retains stack splitting. Plain right click retains item
  use. Shift + right click retains fairy selling when the warehouse is closed.

Ctrl takes precedence when both modifiers are held. Quick moves require an
empty cursor and a hit inside the visible relevant grid; hidden/deleted parent
panels cannot capture the shortcut. Warehouse moves also require an active
warehouse NPC ID. Destination selection respects item footprint and visible
Carry slots 0..62 or Cargo slots 0..119. Direct Cargo-to-Equip is not supported:
move the item into inventory first.

These gestures send the existing 20-byte `0x376` intention: `st/sp` is the
destination, `dt/dp` is the origin, and warehouse moves include the NPC target
ID. No optimistic grid mutation occurs; the existing authoritative server
checks and acknowledgement flow apply. Full storage or rejected equipment
does not fall through into a world action. Visible inventory and warehouse
panels take priority over the Ctrl-right world interaction menu.

Ctrl-right equipment behavior is proven in the native 7.48 binary. The
Shift-right warehouse alias and input guards are compatible project changes,
not a claim of independently established native Shift behavior. See the
[equipment and shortcut evidence](../../.agents/research/client748/flows/ui/equipment-slot-compatibility.md).
Installation, patch publication, and gameplay validation remain separate gates.

## CP and name policy

Persisted signed CP is clamped to −75…+75. Each actual level increment awards
five CP, clamped to +75. Zero rewards, EXP debt repayment without leveling,
evolution locks, and the level cap do not award CP. Multiple levels award the
corresponding multiple in the same durable character state.

All progression consumers preserve CP with level/EXP and publish the CP refresh
after their successful save/completion: combat rewards, quest rewards, quest
items, experience items, Big Cube answers, and instance quiz resolution.

Player `CreateMob` names carry `signedCP + 75` in `MobName[12]`, packet byte 30.
That byte is metadata, not display-name text. Existing entities consume CP
refreshes without recreating skins, resetting movement, or respawning. The
native palette interpolates RGB with truncation, clamps raw values above 150,
and uses black/border below 10. Signed CP zero remains yellow; +65 is nearly
white; +70 reaches the white endpoint; negative values move through red to
black. The native raw-zero red pulse remains player-only.

NPC IDs at or above 1000 always use white without a CP border, including
caret-prefixed summon/NPC names. No NPC CP is persisted or awarded.

## Validation and remaining gates

### Initial correction batch (historical checkpoint)

The table below records the first inventory/CP validation, before the later S2
corrections. Its failing game gate and fixture skips are historical, not the
current acceptance result.

| Gate | Result |
| --- | --- |
| New Go CP/stack tests | Pass: 4,681 CP/level combinations, all 65,025 positive byte-quantity pairs, compatibility/admission cases, 24 merge settlement cases, and CP commit/reject/unknown publication |
| C++ architecture executable | Pass: 1,152,647 checks and static assertions; includes blank/custom split captions, text queueing, all CP bytes, NPC policies, and refresh source contracts |
| Affected Go race batch: `internal/game`, `internal/model`, `internal/wire` | Model and wire pass; game fails only the five separately documented S2 closure acceptance cases. No race report. Seven PostgreSQL-dependent leaf cases skip because this follow-up does not restart the earlier fixture. This is not a clean acceptance gate |
| `go vet ./...` and server executable build | Pass |
| Incremental Release/Win32 client candidate build | Pass; candidate only, no installed runtime overwrite |
| Real client rendering and drag/split flow | Not run; blocked by the existing no-client-execution constraint. No `CLIENT_TESTED` promotion |

The earlier full Go/race batch included an isolated PostgreSQL 18 fixture and
passed its store integration tests. That validates existing adapters, not a new
database fault-injection test of the merge's uncertain outcome. Held-worker
tests cover the new merge/CP lifecycle. The five formerly red S2 cases and the
separate fault-injection/real-client gates are detailed in
[the S2 closure audit](../server/s2-closure-audit-2026-10-07.md).

### Current acceptance checkpoint — 2026-10-07

The corrected source/tests were published in
[`83a94da7`](https://github.com/felipeletsgo/wyd748/commit/83a94da7c531ebdebe2ccc2faedbc35b3bbffd0a).

| Gate | Latest evidence |
| --- | --- |
| S2 lifecycle and full Go suite | All 15 tested packages pass with `-race -count=1 -timeout=300s ./...` and the isolated S2 PostgreSQL fixture enabled; the five original closure failures are corrected |
| C++ policies and candidate build | Initial passing checks/build above remain applicable; no later C++ change in the S2-only correction invalidated them. Not rerun as client execution |
| Other conditional fixtures | Separate read-only web-admin PostgreSQL test and optional local JSON account schema test skipped; not passing leaf gates |
| Real gameplay | User reported apparently correct behavior on October 7; no agent-observed rendering or exhaustive owner/observer, drag/split, palette and relogin matrix |
| Unknown-outcome recovery | Containment is implemented; durable operation reconciliation, restart-safe fencing and verified unlock remain open in the [S2 recovery queue](../server/critical-persistence-plan.md#current-gaps-and-acceptance-criteria) |

The general user report does not promote individual catalog entries to
`CLIENT_TESTED`. Existing native records, policy tests, successful builds and
runtime observations remain distinct evidence categories.

## Changed files

Server: `stack_merge.go`, `stack_merge_test.go`, `progression_cp_test.go`,
`handlers.go`, `progression.go`, `critical_persistence_item_experience.go`,
`quest_rewards.go`, `quest.go`, `big_cube.go`, `item_instances.go`, and
`kill_rewards.go`, all under `wydgo748/internal/game/`.

Client: `GridItemPresentation.h`, `NameColorPolicy748.h`, `SGridInput.cpp`, and
`SGridItem.cpp` under `internal/ui/`; `TMHuman.h`, `TMHuman.cpp`, and
`TMHumanUI.cpp` under `internal/game/entities/`; `TMFieldSceneWorld.cpp` under
`internal/app/scenes/`; `GridInsertionTests.cpp`, `NativeScoreHudTests.cpp`, and
the project/filter registrations, all under `tmproject/TMProject748/`.

Documentation/evidence: this contract, the UI record, the existing S2 audit,
and the central documentation inventory. No files were removed. Build/test
logs and candidate executables remain in ignored build directories.
