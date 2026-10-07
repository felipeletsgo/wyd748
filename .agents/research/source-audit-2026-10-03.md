# Source audit — wydgo748 and tmproject (2026-10-03)

> The original audit below is historical. The [verification and repair record](#verification-and-repair-record)
> supersedes its current-status claims. Not every finding was accurate, and not
> every architectural or language issue is resolved.

Read-only audit of the Go server (`wydgo748/`, ~53k non-test lines) and the
C++ client (`tmproject/TMProject748/`, ~134k lines excluding the DirectX SDK).
Base: `HEAD 34b7dcae` plus the uncommitted working tree. Another session was
editing the server working tree during this audit; results reflect the tree at
about 20:40 local time.

This record supplements the parallel report
[full-source-audit-2026-10-03.md](../../DOCS/client/full-source-audit-2026-10-03.md)
(F01–F10) and its evidence in
`.agents/research/full-source-audit-2026-10-03/`. Findings F01–F09 are not
repeated here. S10 extends F10 with concrete player-facing locations.

Status labels follow `.agents/skills/wydgo748-feature/references/audit.md`:
`CONFIRMED`, `PROBABLE`, `HYPOTHESIS`, `NOT A DEFECT`.

## Gates run

| Gate | Result |
| --- | --- |
| `go vet ./...` | Pass |
| `go test -count=1 ./...` | Pass (15 packages). Earlier in the session `internal/game` failed 3 tests because `skills.go` adopted `resolveNPCInteraction` before the fixtures; the concurrent session has since updated the tests. PostgreSQL integration tests ran (`WYD_TEST_POSTGRES_URL` set); one schema test skipped. |
| `go test -race` | Not run: `CGO_ENABLED=0`, no C compiler. Pending gate. |
| Client `ArchitectureTests` (Release/Win32) | Pass: 1,144,287 checks + static assertions |
| Client `SocketReceiveTests` | Pass: 221 checks |
| Client full rebuild (scratch output, not installed) | Pass. 56 × C4018 signed/unsigned, 2 × C5298 in `Dependencies/.../dxfile.h` |
| Runtime client flow | Not run. No finding below is `CLIENT-TESTED`. |

Dead-code detection used name-based scanners (no `deadcode`/`staticcheck`
installed; nothing downloaded). Their output is a candidate list and was
confirmed by `rg` per symbol.

## Findings, by severity

### S1 — Relogin can run two live copies of the same character (state rollback; duplication under backlog)

`PROBABLE` (code path `CONFIRMED`, timing-dependent).

Invariant: one account has at most one authoritative in-memory copy, and a
new session loads only after the previous copy is persisted and removed.

- `internal/game/handlers.go:2330` `claimAccountSession` reclaims an account
  whose previous session `IsClosed()` before that session's disconnect command
  was processed (`account_session_test.go:41` tests this intentionally).
- `onLoginResult` (`handlers.go:186`, `:207`) then calls
  `store.LoadAccount`. The old `Player` is still in `w.players`, `InWorld`,
  unsaved since its last autosave, and its own sync save runs later in
  `onDisconnect` (`handlers.go:2285`).
- `processCommandBatch` (`world.go:1554-1583`) executes a batch per session
  round-robin, so session order is not global order: a batch holding
  `[S1 packet, S1 disconnect]` and `[S2 loginResult]` executes the S2 login
  before the S1 disconnect.
- `onEnterWorld` (`handlers.go:365`) does not check whether the character UID
  is already in the world, and `indexPlayerCharacter`
  (`gameplay_space.go:97-108`) silently overwrites `playersByCharacterUID` and
  `playersByName`.
- Autosave (`world.go:1806`) also keeps saving the stale copy until the
  disconnect arrives.

Impact: the new session starts from an older snapshot (gold/EXP/item rollback
after its next save). If the old disconnect lands after the new session made
a synchronous save (drop, trade, sale), the old snapshot overwrites it, which
can duplicate an item that left the account.

Fix direction (`MODERNIZACAO_COMPATIVEL`): when reclaiming from a closed
session that still owns a `Player`, run the old `onDisconnect` (save +
removal) before `LoadAccount`. Reject `onEnterWorld` when the UID is already
indexed by another live player. Add a test with an in-world owner,
unsaved state, and a reordered batch.

### S2 — Synchronous PostgreSQL transactions on the World goroutine

`CONFIRMED` (design bottleneck). Status 2026-10-06: fixed, enabled for
PostgreSQL and closed (race, PostgreSQL and client-runtime gates passed). See
[critical-persistence-plan.md](../../DOCS/server/critical-persistence-plan.md#activation-2026-10-06).

About 108 call sites run a serializable transaction of the whole account
inline in the single game loop: `saveAccount` ×68,
`saveAccountAndCharStateResult` ×18, `saveGuildState` ×13, and others. They
include every ground pickup (`handlers.go:2234`, `:2260`), every consumable
(`consumables.go`, 17 sites), drops, skill learning, and every disconnect
(`handlers.go:2285`). `withSerializableTx` allows up to
`postgresOperationTimeout = 10s` with 3 retries (`store/postgres.go:33`,
`:876`).

Impact: world throughput is bounded by database latency. One slow commit
freezes movement, combat and ticks for every player. A mass disconnect
(network blip) serializes N full saves in one batch. The comment above
`onDisconnect` assumes a single-player load ("disconnection is rare").

Fix direction: keep save-before-confirm, but make it asynchronous per
account. Mark the operation pending, persist through the store worker, and
apply the confirmation as a World command. Alternatively, set a short
deadline for World-loop saves. Use the async path for non-economic
counters such as CP recovery.

### S3 — Final shutdown snapshot saves poisoned players

`CONFIRMED`.

`runShutdown` (`internal/game/shutdown.go:79-90`) iterates `w.players` and
calls `saveAccountAndCharStateResult` without checking
`p.PersistencePoisoned`. `poisonAccountsAfterPersistenceFailure`
(`world.go:1675`) poisons the RAM after a failed economic commit and closes
the session. If SIGTERM is processed before that session's disconnect
command, shutdown persists the partial state the poison was meant to
discard. `autoSaveAccounts` and `onDisconnect` both honor the flag.

### S4 — Staff/control commands bypass the shutdown and maintenance barrier

`CONFIRMED`.

`handle` (`world.go:1857-1888`) dispatches kick, teleport, bosses, quiz,
global-drop, control and presence commands before the
`if w.shuttingDown` barrier (`world.go:1892`). After the final snapshot, or
after a handler panic put the world into fail-closed maintenance, staff
commands still mutate world state that will never be persisted
(for example, quiz rewards and boss state).

Also at `world.go:1893`: a `login` result command has `pkt == nil` and is
handled as a disconnect while shutting down. The socket is not closed and no
message is sent, so the client waits until its idle timeout.

### S5 — Client: dangling `g_pCurrentScene` after scene initialization fails

`CONFIRMED` (static).

`ObjectManager::SetCurrentState` (`internal/core/ObjectManager.cpp:784-791`)
calls `SetCurrentScene(pScene)` and assigns `g_pCurrentScene = pScene`
(`:809`). The previous scene is already marked deleted (`:819`). If
`InitializeScene()` fails, it `SAFE_DELETE(pScene)`s and posts `WM_CLOSE`.
`NewApp::OnCloseMessage` (`NewAppWindow.cpp`) then dereferences
`g_pCurrentScene->m_eSceneType` → use-after-free during error shutdown.

### S6 — Client: whisper command overflows fixed buffers from local input

`CONFIRMED` (static).

`TMFieldScene.cpp:2687-2688`: `str1` comes from `sscanf(Chat, "/%s", ...)`
(up to 127 bytes). It is copied with `sprintf` into
`MSG_MessageWhisper::MobName[16]` and with `strcpy` into
`m_cWhisperName[32]`. `TruncateCommandName` runs afterward. Typing
`/` + a name longer than 15 (or 31) characters corrupts the packet body or
adjacent `TMFieldScene` members. Use bounded copies before truncation;
the server already bounds the name with `cstr`.

### S7 — Client: texture getter bounds are off by one

`CONFIRMED` (static).

`TextureManager.cpp:420, 518, 718, 941, 1148, 1360` test `nIndex > MAX_*`
instead of `>=`. Index `MAX_*` reads and writes one element past
`m_ppUITexture[512]`, `m_ppEffectTexture`, `m_ppModelTexture[3000]`,
`m_ppEnvTexture`, the texture-set list and the guild-mark list. Indices come
from asset tables and server data.

### S8 — Client: texture loader trusts file length

`CONFIRMED` (static).

`TextureManager.cpp:268-310` (and copies at `:591/615`, `:800/825`,
`:1021/1045`): `_filelength` minus 4 (or 1) is used without validation, and
`_read` results are ignored. A truncated or empty asset produces a negative
or tiny `new char[]`. The DDS path writes `pBuffer[84..87]` without
checking `nLength >= 88` (heap overflow). The same 60-line loader is
duplicated four times. Also `:342` and `:382` call `SAFE_DELETE` (C++
`delete`) on an `IDirect3DTexture9*`. Use `SAFE_RELEASE`.

### S9 — Client: `TMShade` over-allocates by `sizeof(T)` (decompilation artifact)

`CONFIRMED`.

`render/effects/core/TMShade.cpp:19,21` allocate
`new unsigned short[sizeof(unsigned short) * n]` and
`new RDLVERTEX[sizeof(RDLVERTEX) * n]`. That is an element count multiplied
by the element size (`RDLVERTEX` = 24 bytes). `TMShade(70, ...)` (skill 111,
`TMFieldSceneCombatPackets.cpp:2213`, `:2325`) allocates and constructs
121k vertices (~2.9 MB) instead of 5,041 (~121 KB) on every cast. There are
59 creation sites. The `if (!m_wpIndex || !m_pVertex)` block (`:28-34`) is
dead (`new` throws). If it ran, it would leave dangling pointers that the
destructor deletes again.

### S10 — Language rule violations (release blockers)

`CONFIRMED`.

- Player-facing Portuguese in the server:
  `internal/game/quiz_round.go:151, 162, 169, 183`.
- Player-facing Korean in the client: `client748/UI/hellStoredesc.txt`
  (44 EUC-KR lines) is loaded by the Hell Store window
  (`TMFieldScene.cpp:2443-2455`, `TMFieldSceneMerchant.cpp:147`), which the
  working tree is enabling. `UI/TOTOGame.csv` and `mesh/Mesh.txt` contain
  Korean too (check whether they are player-facing).
- Migration debt (non-player-facing): about 250 Go files and ~3,000 lines of
  Portuguese comments/logs (e.g. `handlers.go` login/disconnect logs,
  `store/postgres.go`, `shutdown.go`, `cp.go`); ~113 client files. Five
  skill references are still Portuguese:
  `wydgo748-feature/references/{audit,client-ui-748,ghidra-client748}.md`,
  `wyd-client748-research/references/subsystem-map.md`,
  `wyd-client748-catalog/references/catalog-strategy.md`.
- Contract-bearing Portuguese names (`summonTemplates` "Urso", "Javali",
  "Dragao Negro"; `spawnWarTower` lookup "Torre"; reserved character names
  `handlers.go:250`) need the compatibility review required by `AGENTS.md`.

## Lower severity

| ID | Status | Location | Issue |
| --- | --- | --- | --- |
| L1 | CONFIRMED | `cp.go:15-47` | CP recovery does a sync full-account save per negative-CP player. All players who enter together share the same 450 s phase, so the saves burst in the same tick. |
| L2 | CONFIRMED | `guild_wars.go:130-136` | A bad `Timezone` logs once per second forever, and wars silently stay off. Validate at config load. |
| L3 | CONFIRMED | `operational.go:81-99` | When `maxAuthIPRateKeys` (2048) or `maxAuthAccountRateKeys` (4096) fill within one window, every new origin or account is refused for up to a minute. That allows a distributed login DoS. |
| L4 | HYPOTHESIS | `store/postgres.go:490-491` | Overflow coalesces `run` jobs by `label`. Today, only the generation-guarded `Guilds.txt` export uses it. A future `run` job with a shared label or a `done` channel would be dropped or never signaled. |
| L5 | CONFIRMED | `wydgo748/cmd/server/tm.exe` | A 22 MB generated binary inside the source tree (Git-ignored). It violates the `AGENTS.md` file-organization rule. Build into `bin/` or `build/`. |
| L6 | CONFIRMED | `.agents/skills/wydgo748-feature/references/audit.md` | The "intentionally unused" list names `magicDamage` and `mobHasActivePlayer`, which no longer exist as symbols. |
| L7 | CONFIRMED | `CPSock.cpp:178` | Blocking `connect()` on the UI thread before `WSAAsyncSelect`. An unreachable server freezes the window for the TCP timeout. This is native behavior, so treat it as a UX modernization. |
| L8 | CONFIRMED | `TMProject748.vcxproj` | 21 headers in use are not listed in the project (`CCModePolicy.h`, `QuizEventPacket.h`, `ServerWarLetterContract.h`, …), so they are missing from IDE filters and search. |
| L9 | CONFIRMED | client build | 56 × C4018 signed/unsigned comparisons (`BasedefItems.cpp:602-610, 739-747`, `TMFieldSceneSkills.cpp`, `TMHumanUI.cpp`). These are display-side requirement checks, and the server stays authoritative. |

## Dead code

Status: candidates confirmed by `rg`. Do not remove without the
caller/registration proof in `audit.md` §7.

Go, no callers (`real dead` unless noted):

- `internal/game/affects.go:681` `mobPublicExtended`
- `internal/game/skill_summons.go:73` `removeContractSummons` (superseded
  by `planContractSummon`), and the `summonTemplate.maxSummons` field (`:14`)
- `internal/game/spatial.go:110` `nearbyPlayersInGameplaySpace`
  (referenced only by a comment)
- `internal/model/kingdom.go:23` `CapeTierHero`;
  `internal/wire/codec.go:202` `NormalNameChaos`;
  `internal/wire/opcodes.go:133` `SceneEnter` (protocol constants: keep if
  they serve as contract documentation)

Go production code reached only from tests (~35 symbols). Tests exercising
these wrappers do not prove the live path: `combat.go` `hitDamage`,
`playerHitsMob`/`playerHitsPlayer`/`mobHitsPlayer` (both function and
method forms), `applyCouragePvEDamage`; `affects.go` `setAffect`,
`accumulateAffect`, `activePlayerAffect`, `applySupportSkill`,
`effectiveMob{Defense,Attack,AttackRun,Resistances}`, `activeMobAffect`,
`setPlayerOwnedMobAffect`; `boss_actions.go` `bossSkillDamage`;
`boss_spawn.go` `onBossMobKilled`, `rollBossDrops`; `drops.go`
`rollMobDrops`; `kill_rewards.go` `finalizeKillReward`; `mob_combat.go`
`nearestLivingPlayer`; `mount.go` `mountGrowth`, `grantMountHuntExp`,
`mountLongevityRecover`; `security.go` `validPlayerMovePacket`;
`handlers.go` `acceptClientAttack`; `visibility.go`
`privateWaterRuntimeIDForPlayer`; `world.go` `createGroundDrop`;
`item_instances.go` `itemInstanceInExitGrace`; `experience.go`
`expWithDoubleBuff`; `skill_pvp.go` `hasActiveAffect`, `setOwnedAffect`;
`commands.go` `commandClearInventory`; `data/volatiles.go`
`LoadVolatiles`; `store/postgres.go` `PostgresAsyncStats`. For each, either
point the test at the live entry point or move the helper into `_test.go`.

Client: the linker-based [dead-code map](../../DOCS/client/dead-code-map.md)
is authoritative and already covers what this name-based scan found
(DirectX sample-framework helpers in `DXUtil.cpp`, `DirShow.cpp`,
`dsutil.cpp`, `D3DEnumeration.cpp`; `TMDemoScene`/`TM_DEMO_STATE`). One
addition: the dead allocation-failure block in `TMShade.cpp:28-34` (S9).

## Verified non-defects

These are noted so they are not reaudited:

- Inbound framing: size bounds, checksum, per-session packet/byte rate,
  exact per-opcode sizes, phase gating and violation-based disconnect
  (`net/session.go`, `security.go:95`).
- Outbound: non-blocking queue, a per-recipient builder (`broadcast`,
  `sendTo*View`) avoids double encryption, write deadline, and writer exit
  closes the session (working tree).
- World loop: bounded batches with a 5 ms budget, round-robin fairness, and
  ticks preempting batches without loss; ground expiry uses a heap, and
  visibility uses spatial cells.
- Persistence ordering: async jobs carry sequence fences; sync saves fence
  older async snapshots; ghost-shop and trade purchases commit both accounts
  atomically before acknowledging.
- Client receive path: bounded window, full frame before key consumption,
  and close on any stream error (`CPSock.cpp`, `NewAppWindow.cpp:409`).

## Pending gates

- `go test -race` on a machine with cgo.
- A reproduction test for S1 (in-world owner + reordered batch).
- Runtime client validation for any S5–S9 fix (`CLIENT-TESTED`).

## Verification and repair record

Date: 2026-10-03. Reference: `34b7dcae4068f1b6bcfc152e4037bd9e7ccf0dfa`
plus the existing dirty working tree. This follow-up verifies this supplemental
audit and implements focused repairs; it is not another complete source census.
The original findings and validation above remain historical evidence. Existing
changes by other sessions were preserved. No commit, deployment, game launch,
operational database mutation, or extraction-approval update was performed.

### Current disposition

Priority is the follow-up assessment, not an assertion that the original report
assigned these priorities. No confirmed P0 was found in this repair scope.

| ID | Priority | Verification | Disposition |
| --- | --- | --- | --- |
| S1 | P1 | Confirmed admission/order defect; exploitation depends on timing | Fixed admission invariant; production queue-order regression added |
| S2 | P1 | Confirmed synchronous World bottleneck; timeout claim overstated | Open architecture issue; existing 500 ms configuration and one shared deadline verified |
| S3 | P1 | Confirmed poisoned aggregate could be saved at shutdown | Fixed; entire poisoned account excluded and incomplete drain reported |
| S4 | P1 | Partly incorrect: mutating staff handlers already had guards | Fixed late authentication and query-side expiry; retained existing mutation barriers |
| S5 | P1 | Confirmed dangling scene pointer on initialization failure | Fixed in source; build verified, failure-path runtime gate pending |
| S6 | P1 | Confirmed local whisper buffer overflow | Fixed with bounded parsing and rejection before packet/history writes |
| S7 | P1 | Confirmed negative/equal-capacity texture index defects | Fixed getter and loader index checks |
| S8 | P1 | Confirmed unsafe length/read handling and COM deletion | Fixed shared validated reader and COM release |
| S9 | P2 | Confirmed element-count over-allocation and unsafe failure ownership | Fixed counts, bounds and staged ownership |
| S10 | P1 release gate | Confirmed player-facing and internal language debt | Partial: changed files and Hell Store text translated; global migration open |
| L1 | P2 | Confirmed synchronized CP save burst | Mitigated to one oldest-due save per tick; synchronous latency remains |
| L2 | P2 | Not a defect in the current tree | Configuration already validates timezone and hours; timezone data embedded |
| L3 | P2 | Confirmed blanket refusal at rate-key cap | Mitigated with bounded salted overflow buckets; distributed abuse remains possible |
| L4 | P3 | Future-risk hypothesis, not a current failing consumer | No speculative store-job rewrite |
| L5 | P3 | Confirmed generated executable in source tree | Moved recoverably into ignored `wydgo748/bin/` |
| L6 | P3 | Confirmed stale skill-reference symbols | Reference corrected and translated |
| L7 | P2 | Confirmed blocking connection setup | Nonblocking connection lifecycle implemented and loopback tested |
| L8 | P3 | Confirmed incomplete project/header filter inventory | Header registration and filter coverage corrected |
| L9 | P2 | Confirmed warnings; warning count duplicated across configurations | 28 distinct comparison sites widened safely; Release and Debug build passed |

Functional safety changes are classified as `MODERNIZACAO_COMPATIVEL`:
no opcode, packet field layout, persisted identifier, or authoritative ownership
was introduced. L7 is a connection-lifecycle modernization, not native parity.
Existing native/wrapper evidence was reused; no new parity claim or Ghidra
inventory was needed. Translation changes presentation, not wire constants.

### Server repairs and remaining architecture work

**S1 — account ownership and character admission.** Locations:
`wydgo748/internal/game/handlers.go:2319` (`claimAccountSession`), `:359`
(`onEnterWorld`), and `account_session_test.go`. A closed transport no longer
releases account ownership before its World disconnect command. A competing
login is rejected before loading stale storage; normal cleanup persists/removes
the old owner and releases the reservation. Repeated world entry is ignored;
entry for a UID already owned by another player is rejected before mutation.
This intentionally uses rejection/retry rather than recursively running the old
disconnect during another session's login.

Regression: `supplemental_audit_regression_test.go:14` uses the actual queue
preparation/pop routines to place a new authentication result ahead of an older
disconnect, then runs the production dispatcher. It checks zero store reloads,
no second player, retained ownership and unchanged unsaved gold. The disconnect
remains queued. This is not a full database duplication exploit or an execution
of the entire timed `processCommandBatch` loop. Duplicate UID and repeated-entry
rejection are tested separately at `:55`.

**S2 — corrected assessment, still open.** Locations:
`wydgo748/internal/data/server_config.go:114,369`,
`wydgo748/cmd/server/main.go:271`, and
`wydgo748/internal/store/postgres.go:876`. Production passes a configurable
critical-operation timeout (default 500 ms, allowed 100–10,000 ms). The 10-second
constant is a fallback, not the production default. Transaction begin, callback,
commit and all three retries share one context deadline, so the report must not
be read as three independent 10-second deadlines in the current tree.
Many critical saves still block the World goroutine. The historical count of
108 call sites was not recounted in this follow-up and is not a fresh metric.

Recommended follow-up: introduce bounded per-account pending operations and
immutable snapshots; fence affected gameplay until a store worker completes;
return completion as a World command; publish economic confirmations only after
commit; define rollback, retries, multi-account trades, reconnect, poison and
shutdown drains explicitly. Exercise this against an isolated PostgreSQL
fixture and with race tests before replacing synchronous critical saves.
Unfenced fire-and-forget saves would reintroduce economic duplication. No such
cross-cutting persistence rewrite was implemented in this batch.

**S3 — poisoned final state.** `wydgo748/internal/game/shutdown.go:46` first
collects normalized account names with any poisoned player. It skips every
character of those aggregates, including a nominally healthy sibling that could
otherwise save the poisoned payload. Healthy accounts and instance state retain
their normal save path, and queued prior writes are flushed. Discarded poison is
reported through the shutdown error, not logged as complete final persistence.
The aggregate/sibling/healthy-save/flush regression is at
`supplemental_audit_regression_test.go:76`. Operational recovery still requires
investigating the original failed commit; discarded RAM is not recovery data.

**S4 — actual shutdown gaps.** `world.go:1892` now closes late session commands
and distinguishes an authentication result from a real packet-less disconnect.
It removes pending authentication without admitting an account. Status queries
in `quiz.go:73` and `global_drop.go:72` no longer expire events while the World
is shutting down or in fail-closed maintenance. Their normal expiry behavior
continues outside maintenance. These paths are covered at
`supplemental_audit_regression_test.go:97`.
The original general staff-bypass assertion was too broad: kick, teleport,
boss, quiz and global-drop mutation handlers already reject maintenance/shutdown
(`kick_control.go:96`, `teleport_control.go:82`, `bosses_control.go:106`,
`quiz.go:107`, `global_drop.go:106`). Read-only control/presence queries remain
available; dispatch before the common barrier alone does not prove mutation.

**L1 — CP burst mitigation.** `cp.go:16` selects the oldest due negative-CP
player, breaking ties by player ID, and performs at most one recovery save per
tick. Poisoned state is excluded. A failed save restores CP and sends no success
notification. Tests at `supplemental_audit_regression_test.go:117` cover multiple
due accounts, no early repeat and injected persistence failure. Recovery can
be delayed under backlog; one slow save can still stall the tick. This is an
explicit scheduling mitigation, not a complete solution to S2.

**L3 — bounded admission under cardinality pressure.**
`operational.go:88` reserves one quarter of the maximum key budget for salted
overflow buckets. Existing exact-key counters are not evicted/reset, existing
overflow counters stay effective, and cleanup scans at most 32 entries per new
key. Overflow permits newcomers without allocating unbounded individual keys.
Regression at `supplemental_audit_regression_test.go:152` verifies bounded map
size, admission, repeated-key blocking and expiry. A sufficiently distributed
flood can exhaust shared bucket limits or cause innocent collisions; this is
not a claim of complete distributed-DoS protection. Monitor rejection and
collision pressure before tuning budgets.

**L2 / L4 — no unnecessary fixes.** `model/guild_wars.go:28` already validates
`time.LoadLocation`, nonempty timezone and 0–23 hours; its embedded `time/tzdata`
supports Windows without host zoneinfo. L2 is stale in this tree. For L4, the
only current production asynchronous `run` consumer is generation-fenced
`Guilds.txt` export with no completion channel. Enforce unique job identity and
completion semantics if another consumer is added; a hypothetical future job
is not evidence of a present defect.

### Client repairs and coverage limits

**S5 — failed scene lifetime.** `internal/core/ObjectManager.cpp:793` clears
`g_pCurrentScene` when it points to the failing scene, before deletion and the
deferred close message. The previous scene was already scheduled for deletion
and is not resurrected. The close handler's existing null guard is retained.
Static inspection and both builds passed; an executable injected-scene-failure
fixture and actual runtime shutdown remain pending.

**S6 — whisper input.** `internal/app/scenes/TMFieldScene.cpp:2661,2667,2685`
bounds command and relocation parsing, then rejects an empty or greater-than-15
byte whisper target before fixed packet/history writes. The relocation branch
is evaluated first. `internal/application/FieldChatSubmitPolicy.h:12` centralizes
the name bound. Tests include 0, 1, 15, 16, 31, 32 and 127-byte names and null
input. Packet layout and accepted existing names are unchanged.

**S7 / S8 — texture safety.** `internal/render/resources/TextureManager.cpp`
uses `ValidIndex` for six getter collections and all four loaders, including
the UI source index. Source-index extension selection is corrected. The new
`internal/application/WrappedTexture.h` owns descriptor closure, rejects negative,
short or greater-than-64-MiB files before allocation, requires a complete
128-byte DDS or 18-byte TGA header after its wrapper prefix, handles allocation
failure and requires exact prefix/payload reads. Existing DDS signature/FourCC
and TGA footer transformations are retained. Two COM texture failure paths now
use `SAFE_RELEASE`, not C++ deletion. Tests exercise index boundaries, file
length boundaries, real temporary descriptors, decoded headers/footer and
short reads despite an apparently valid file length. They do not exercise
Direct3D texture creation or GPU device-loss behavior.

**S9 — shade allocation.** `internal/render/effects/core/TMShade.cpp:12` uses
`internal/application/ShadeGrid.h` to bound grid size to 2–255, calculate element
counts rather than byte counts, and keep vertex indices within unsigned-short
capacity. Temporary smart pointers own both allocations until success, avoiding
leaks/double deletion if the second allocation throws. A missing scene is
checked before ground access. Tests cover the 70-cell case and size limits;
allocation-failure injection remains pending. The later user-reported visual
acceptance below does not identify per-effect coverage.

**L7 — asynchronous connection.** `internal/platform/windows/CPSock.cpp:151`
registers `FD_CONNECT` before calling nonblocking `connect`. A pending flag
defers sends; the raw four-byte handshake and encrypted login remain ordered in
the normal queue. Immediate success, pending success, registration failure,
connect errors, close/reset and late notifications are handled. The socket
fixture uses a hidden message-only window and a loopback listener, checks prompt
return, pumps real Winsock notifications and validates the ordered/decrypted
bytes. It does not launch the game or demonstrate an unreachable remote server,
pending-connect cancellation, socket-handle reuse, or reconnection UI behavior.

**L8 — project integration.** `TMProject748.vcxproj` and `.vcxproj.filters`
register the 21 previously absent headers plus `ShadeGrid.h` and
`WrappedTexture.h`. Four already registered headers were missing filters; those
were added. An invalid `Source Files\\Macro` header filter was corrected.
XML validation reports zero missing internal headers, duplicate registrations,
missing header filters or undefined filters. No compile unit was dropped.

**L9 — comparisons, not warning suppression.**
`internal/core/BasedefItems.cpp:602,739`,
`internal/app/scenes/TMFieldSceneSkills.cpp`, `TMFieldScene.cpp`,
`internal/game/entities/TMHumanUI.cpp:566,1068`, and
`internal/ui/SGridInput.cpp:525` / `SGridTooltip.cpp:1210` widen unsigned scores
to signed 64-bit values for comparisons. Mastery-cost multiplication uses
`100LL`; displayed levels use a mutable `long long`, preserving the second-class
400-level adjustment and avoiding unsigned underflow for negative thresholds.
This preserves all uint32 score values without signed-32-bit truncation; no
field/ABI layout changed. The original 56 warnings represented 28 distinct
sites across two configurations. Both affected builds passed after correction.
The first build caught an erroneous `const` on adjusted display levels; that
was removed and the final build passed. There is no dedicated runtime fixture
for all 28 UI/skill paths, so warning removal is not gameplay approval.

### English, tools and generated artifacts

**S10 — partial migration only.** Changed server/client files were reviewed and
their comments, diagnostics and player messages translated. `quiz_round.go`
now emits English countdown/reward text. `handlers.go`'s death-letter display
argument changes from the legacy word to `"player"`; the consumer uses it only
as displayed text, not a discriminator. Reserved-name denylist entries remain
unchanged compatibility identifiers. Untouched summon/template/tower lookup
names are not translated without reviewing their consumers.

`tmproject/client748/UI/hellStoredesc.txt` is now ASCII English. The original
CP949 asset had 100 physical lines; that count and color prefixes are preserved.
The loader uses inclusive zero-based page ranges, so category headers are placed
at physical lines 22, 42, 62 and 82 (not the last line of the prior page).
Prices remain 9,000,000 / 6,000,000 / 5,000,000 / 7,000,000 gold, matching the
server. Blank color-only lines have no trailing spaces; physical offsets remain
stable. The loader skips those empty lines, so vertical padding can change;
visual spacing is not approved by the static check. Original SHA256:
`4647C7BAA68CCD4BF00E25D56B79148C849F751059104B5967A7F0771A6BD18A`.
Validation checks ASCII, 100 lines, prefixes, maximum line width and page heads.
Visual layout/word wrapping was not tested by the agent. The later user-reported
visual acceptance below does not identify individual screens.

Global language migration remains incomplete. `TOTOGame.csv` and `mesh/Mesh.txt`
still require consumer tracing to determine exposure; no direct filename hit
does not prove they are dead. Untouched Portuguese in server/client files and
four remaining skill references remains migration debt. Historical file/line
counts above were not refreshed. This batch does not make a global release ready.

**L5 — recoverable move.** The 22,765,568-byte ignored generated binary moved
from `wydgo748/cmd/server/tm.exe` to
`wydgo748/bin/tm-audit-2026-10-03.exe`. Exact paths, ignored destination,
absence of a collision and absence of a running process at the source were
checked before the native `Move-Item`. It was not deleted. Future builds should
use explicit `bin/` output. No service was stopped.

**L6 — stale review guidance.**
`.agents/skills/wydgo748-feature/references/audit.md` is now English, removes
nonexistent intentionally-unused symbols and requires current caller,
registration and contract evidence. The skill's frontmatter was not changed.
The optional `quick_validate.py` check could not run because the available
Python lacks `yaml`; no dependency was installed. Repository pointer/layout
validation passed. This is a pending validator prerequisite, not a passing check.

**Dead-code claims.** Scoped searches reconfirmed the absence of production
callers for `mobPublicExtended`, `removeContractSummons`,
`nearbyPlayersInGameplaySpace`, and references beyond definition for
`maxSummons`, `CapeTierHero`, `NormalNameChaos`, and `SceneEnter` (comments
excluded). This is a deletion-candidate check, not proof of incompatibility.
Protocol constants and compatibility helpers were retained. The approximately
35 test-only wrapper claims were not individually re-established in this
follow-up; their production-vs-test coverage remains explicitly unverified.
The existing client linker dead-code map was reused, not regenerated.

### Validation after repairs

| Gate | Result and limits |
| --- | --- |
| `gofmt` on changed Go files | Pass |
| `go test -count=1 ./...` from `wydgo748/` | Pass; database fixture variables removed from the child process, so PostgreSQL-dependent tests skipped |
| `go vet ./...` | Pass |
| Focused account/admission/shutdown/CP/rate tests | Pass after strengthening the queue-order fixture; includes injected save failure |
| `Build-Client.ps1 -Configuration Release -NoDeploy` | Pass; ignored build output only |
| `Build-Client.ps1 -Configuration Debug -NoDeploy` | Pass; ignored build output only |
| ArchitectureTests, each configuration | 1,144,394 checks plus static assertions pass |
| SocketReceiveTests, each configuration | 231 checks pass; loopback-only fixture |
| Header/filter XML validation | Pass: all four missing/duplicate/undefined counts zero |
| Hell Store description validation | Pass: line count, ASCII, prefixes, widths, page offsets and whitespace |
| `extract-method-tests.py` | 21 tests pass |
| `source-relocation-tests.py` | 9 tests pass |
| `tmhuman-separation.py --complete --with-policies` | Fail: approved LabelPosition / LabelPosition2 content no longer matches the L9 edits |
| `tmfieldscene-separation.py --complete --with-policies` | Fail: changed skill dispatch/request content; chat transformation approval differences also remain |
| Separation-tool fixture suites | Fail: assertions/errors depend on the live approved tree, which now differs; do not interpret these as passing extraction equivalence |
| Top-level source relocation manifests | 9 pass, 2 fail: `core-basedef` and `ui-sgrid` contain the reviewed comparison edits |
| `Test-RepositoryLayout.ps1` without `-UpdateMap` | Pass after adding the repair record: 174 documents indexed; subsequent edits only clarify prose |
| `git diff --check` | Pass after trimming the description's blank color-line whitespace |
| Race tests | Not run: `CGO_ENABLED=0`, no suitable GCC/Clang available |
| Isolated PostgreSQL transactions/rollback/concurrency | Not run: no isolated fixture configured; operational DB deliberately not used |
| In-game visual checks | Pass reported by the user on 2026-10-03; build identity and per-flow coverage not supplied |
| Agent-executed runtime/game/device-failure flows | Not run, per the client-execution restriction; failure-path coverage is not established by the general visual report |

#### User-reported visual acceptance

On 2026-10-03, the user confirmed that the in-game visual checks passed and
were satisfactory. This is human-reported runtime visual acceptance, separate
from the automated and static results above; the agent did not launch or deploy
the client. No build identifier, screen/flow checklist, or injected-failure
results were supplied. Specific catalog entries are therefore not promoted to
`CLIENT_TESTED` by this general report alone. Device failure, failed scene
initialization, reconnect/cancellation edge cases, persistence rollback and
concurrency remain unverified where their individual gates are still pending.

Successful relocation manifests: `app-newapp`, `core-basedef-header`,
`render-device`, `render-ground`, `render-skinmesh`, `scene-selectchar`,
`scene-selectserver`, `scene-tmscene`, `ui-controls`. Failed manifests are not
silently re-approved. The extraction/digest gates need an independent semantic
review of these intentional changes before any approval update; pre-existing
chat differences need their own disposition. Build success cannot replace this.

### Changed files owned by this repair batch

Paths below are repository-relative. The surrounding working tree contains
many pre-existing changes not owned by this batch.

- Server: `wydgo748/internal/game/{handlers,account_session_test,shutdown,world,quiz,global_drop,quiz_round,cp,operational}.go`;
  new `supplemental_audit_regression_test.go` in the same directory.
- Client: `tmproject/TMProject748/internal/core/{ObjectManager,BasedefItems}.cpp`;
  `internal/app/scenes/{TMFieldScene,TMFieldSceneSkills}.cpp`;
  `internal/game/entities/TMHumanUI.cpp`; `internal/ui/{SGridInput,SGridTooltip}.cpp`;
  `internal/render/resources/TextureManager.cpp`; `internal/render/effects/core/TMShade.cpp`;
  `internal/platform/windows/CPSock.{h,cpp}`; `internal/application/FieldChatSubmitPolicy.h`;
  new `internal/application/{WrappedTexture,ShadeGrid}.h`;
  `tests/{AuditSafetyTests,SocketReceiveTests}.cpp`; `TMProject748.vcxproj` and `.filters`.
- Asset: `tmproject/client748/UI/hellStoredesc.txt`.
- Guidance/evidence: `.agents/skills/wydgo748-feature/references/audit.md` and this report.
- Recoverable local move: `wydgo748/cmd/server/tm.exe` → `wydgo748/bin/tm-audit-2026-10-03.exe`.

No source or asset was deleted by this repair batch. Installed client runtime
changes already present at entry were not repaired or overwritten by `-NoDeploy`.

### Closure status and next corrections

This repair batch is **statically verified and automated-tested within the
coverage above**, with subsequent **user-reported in-game visual acceptance**.
It is not a completed fix of every audit item. S2 and the global S10 migration
remain open; L1 and L3 are mitigations,
not elimination of all latency/abuse risks. The follow-up below closes the
extraction approval gates and validates the existing PostgreSQL fixtures.
Race and specific runtime failure-path gates remain pending. PostgreSQL fixture
success does not resolve S2's blocking World-loop architecture.

Next work should prioritize S2's account-fenced persistence design and its
database/race fixtures, then complete consumer-aware English migration.
Do not use the successful builds, assertion counts or this repair record as
authorization to deploy or as proof of gameplay parity.

### Follow-up: extraction approvals and isolated persistence validation

This 2026-10-03 follow-up supersedes the failed extraction gates and the missing
PostgreSQL fixture prerequisite in the earlier validation table. It changes
verification records and one policy digest, not client/server gameplay source.

#### Independent review before approval

Numeric method changes were reviewed against the previous approved contents.
Removing only the signed 64-bit comparison casts, reverting the two widened
label-level locals, and reverting the widened mastery multiplication reproduced
the previous normalized method digests. This accounts for the delta without
approving unrelated extraction changes. The multiplication uses `100LL` to
avoid intermediate `int` overflow; the other changes preserve signed comparison
semantics. No packet layout or ABI change was introduced by these deltas.

The chat-submit review separately established bounded scanning into the existing
128/256-byte buffers, rejection of whisper names that cannot fit the 16-byte
destination before copying or updating history, and bounded output formatting.
Reverting those exact changes reproduced the previous approved method digest.
Removing only `FitsWhisperName` reproduced the previous policy digest.

The ordinary party-chat change replaces unbounded formatting with bounded
formatting. The indexed-message path additionally validates packet extent and
decodes through `IndexedPartyMessage.h` rather than writing into shared message
table storage. The decoder bounds-checks signed indices, copies into a bounded
output, terminates it, and leaves the table unchanged. Existing regression
coverage includes short payloads, signed index extremes, unterminated table
entries, unchanged shared-table bytes, and whisper-name boundary lengths.
This is contract-preserving internal modernization, not new native parity proof.

Explicit review records retain the original extraction pins and record the
before/after digests in:

- `.agents/research/relocations/tmhuman-reviews.json`
- `.agents/research/relocations/tmfieldscene-reviews.json`
- `.agents/research/relocations/core-basedef.json`
- `.agents/research/relocations/ui-sgrid.json`

The reviewed chat policy pin is updated in
`.agents/research/tmfieldscene-separation.py`. No blanket approval option or
weakened verification rule was used. Review-chain ordering was preserved for
the existing `SGrid::RButton` extraction and its subsequent comparison change.

#### Current validation results

| Command or gate | Result and limits |
| --- | --- |
| `python .agents/research/tmhuman-separation.py --complete --with-policies` | Pass; 126 current definitions, complete ownership/project/filter checks |
| `python .agents/research/tmfieldscene-separation.py --complete --with-policies` | Pass; 282 current definitions, helper/policy/ownership/project/filter checks |
| `tmhuman-separation-tests.py` | 12 tests pass |
| `tmfieldscene-separation-tests.py` | 22 tests pass |
| `source-relocation-tests.py` | 9 tests pass |
| `extract-method-tests.py` | 21 tests pass |
| All 11 top-level relocation manifests | Pass, including previously failing `core-basedef` and `ui-sgrid` |
| `go test -count=1 -timeout=120s -v ./internal/store` with isolated PostgreSQL fixture | Pass; PostgreSQL integration and stale-autosave fence tests actually executed |
| `go test -count=1 -timeout=180s ./...` with the same fixture | Pass |
| `go vet ./...` with the same fixture | Pass |
| `Test-RepositoryLayout.ps1` without `-UpdateMap` | Pass; layout and local links, 174 indexed documents |
| `git diff --check` with repository settings; explicit report whitespace check | Pass; disabling normal CRLF conversion produces artificial whitespace failures and is not the repository gate |
| C++ Release/Debug builds | Earlier successful `-NoDeploy` results reused: this follow-up changes no product source/build dependency |
| Race validation | Still pending: no suitable CGO compiler established |
| Failure-injected client runtime flows | Still pending; general user-reported visual acceptance is unchanged |

All relocation manifests now passing: `app-newapp`, `core-basedef-header`,
`core-basedef`, `render-device`, `render-ground`, `render-skinmesh`,
`scene-selectchar`, `scene-selectserver`, `scene-tmscene`, `ui-controls`,
`ui-sgrid`.

The database used a newly initialized cluster under ignored
`build/audit-pg-80354e58c08048039e4bcfed5e5ad061/data`, bound only to
`127.0.0.1:54383`, database `wyd_audit`, role `codex_audit`. SQL verified the
database, role, cluster directory, listener address and port before tests.
Only child test processes received `WYD_TEST_POSTGRES_URL`. The operational
database and its services were not modified. The temporary cluster was stopped
after the suite and vet completed; its ignored artifacts were retained.

Executed PostgreSQL coverage includes queued stale autosaves versus critical
player-state saves, account directory reads, store round trips, character-UID
state cascade/incremental items, and guild export failure after committed state.
These fixtures validate their tested transactions and fences, not every economy
handler or a nonblocking World loop. `TestProjectAccountsUseCurrentSchema` still
skipped because its project JSON account fixtures are absent; this is a separate
fixture limitation, not a PostgreSQL test skip.

#### Remaining implementation and release gates

S2 remains a confirmed architecture issue: critical persistence still runs
synchronously on the World goroutine. Its correction needs bounded work queues,
account-scoped fencing, completion handling on the World owner, and explicit
rollback, repeated-request, reconnect, trade and shutdown behavior. Replacing
critical writes with fire-and-forget saves is not a safe correction.

S10 remains a global release blocker: known English-migration debt, including
player-facing content, still exists. Other residual items and unverified runtime
cases above are not erased by the successful automated gates. Independent
features may be developed, but persistence/economy work must account for S2 and
no global release readiness is asserted.

### Follow-up: staged S2 coordinator and World completion owner — 2026-10-04

This follow-up implements part of the S2 scheduling architecture, not its
production migration. The durable design and full closure requirements are in
[the critical persistence plan](../../DOCS/server/critical-persistence-plan.md).
Classification: `MODERNIZACAO_COMPATIVEL`; no packet, ABI, persisted schema or
native gameplay contract changes. No new Ghidra research is necessary.

#### Implemented foundation

- `wydgo748/internal/game/critical_persistence.go`: bounded worker and pending
  capacity; nonblocking admission; canonical account fences; all-or-none
  multi-account admission; immutable detached account/sidecar snapshots; atomic
  backend capability checks; explicit completion acknowledgement; admission
  close and accepted-job drain. Fences/capacity include completed results not
  yet acknowledged by the owner. Sidecar deletion retains its explicit UID.
- `wydgo748/internal/game/critical_persistence_owner.go`: owner-only
  continuation registration and delivery; successful continuation before
  acknowledgement; unknown/duplicate completion rejection; panic containment;
  retained fences and capacity after store or continuation failure. Delivered
  failures discard their callback closure rather than retaining live objects or
  permitting a mutation replay. Reconciliation/release is not yet implemented.
- `wydgo748/internal/game/world.go`: dormant completion-channel selection and
  bounded drains in the Run, command-batch and tick paths. Per-drain count is
  eight; at most one completion is drained between session commands. Command
  budgeting and tick-duration instrumentation include their completion work.
  Failure closes coordinator admission, preserves the shutdown error, enters
  maintenance and poisons online snapshots. This provisional global isolation
  must not be confused with the planned account/resource-local lifecycle layer.

The current store interfaces do not certify whether an error occurred before
or after commit. Every returned store error and recovered store panic is
therefore conservatively indeterminate; original errors remain discoverable
through `errors.Is`. Automatic rollback or repetition of an economic intention
is not justified by a generic error or timeout.

`NewWorld` does not create the coordinator. No production handler/tick producer
submits through it. Normal production critical saves remain synchronous; S2 is
still **open**. This intentionally avoids enabling a partially fenced economy.
No account/character generation checks, shared-resource reservations,
disconnect/reconnect reconciliation or final-drain migration are claimed.

#### Automated coverage and results

There are 18 top-level coordinator/owner tests, plus five failure subtests, in
`critical_persistence_test.go` and `critical_persistence_owner_test.go`.
Coverage includes nested snapshot isolation, participant/capacity rejection,
identity normalization, atomic-store capability refusal, explicit sidecar
deletion, unrelated-account progress during a held save, acknowledgement order,
worker panic containment, continuation panic/error containment, duplicate
delivery, bounded completion drains and accepted-job shutdown drain.

`TestCriticalWorldBatchAndTicksAdvanceDuringHeldSave` holds the injected store
call while exercising the actual World command-batch and tick methods. The
unrelated control command and tick advance without applying a precommit result;
the batch subsequently delivers the completed continuation. This verifies those
scheduling hooks with a staged coordinator, not the unmigrated economy handlers
or the full `World.Run` goroutine under production load.

`TestCriticalWorldFailureEntersMaintenanceAndPreservesShutdownError` verifies
that an indeterminate result cannot release its fence or be hidden by a later
shutdown request. This does not validate final-drain ordering of accepted
gameplay operations; that integration remains pending.

| Command/gate | Result and limit |
| --- | --- |
| `go test -count=1 -timeout=60s -run TestCritical -v ./internal/game` | Pass; all 18 top-level tests and five subtests executed |
| `go test -count=20 -timeout=60s -run TestCritical ./internal/game` | Pass; repeated scheduling/failure foundation coverage |
| `go test -count=1 -timeout=180s ./...` | Pass; this invocation had no isolated PostgreSQL fixture configured |
| `go vet ./...` | Pass |
| `go build -o bin/tm-check.exe ./cmd/server` | Pass; ignored local build output, not deployed or run |
| `Test-RepositoryLayout.ps1 -UpdateMap` | Pass; new persistence plan indexed, 175 documents and local links validated |
| `git diff --check` and explicit changed/untracked file whitespace checks | Pass with repository CRLF settings |
| PostgreSQL transaction/autosave fences | Earlier isolated fixture results reused; store implementation and schema unchanged; not newly exercised in this follow-up |
| Race tests | Pending; `CGO_ENABLED=0`, `CC=gcc`, suitable compiler not established |
| C++ builds/visual tests | No affected C++/asset/build input in this follow-up; earlier evidence retained, no client execution/deploy |

The character-sidecar model comments (`internal/model/charstate.go`) were also
translated to English. `CharStateVersion`, JSON field names and persisted
identifiers are unchanged; the migration record is updated. S10 remains open:
this does not translate remaining server/player-facing or global repository
content. No files were removed by this follow-up.

#### Next required implementation

Implement reservation and lifecycle coverage before connecting a producer:
retain participant accounts through disconnect; guard reconnect and reused
session/character identities; cover other players' combat/reward/pickup and
shared-object mutations; exclude reserved/indeterminate state from autosave;
reconcile unknown outcomes without replay; drain accepted continuations before
final snapshots. Then migrate a complete operation family and test its real
production path. Neither green foundation tests nor the dormant World hooks
close S2 or approve release.

### Follow-up: persistence fences and shutdown ordering — 2026-10-04

This batch advances the staged S2 integration and fixes an active shutdown
mutation bug. It supersedes the previous follow-up's pending final-drain and
common-wrapper autosave/readmission gates, not its remaining production
migration requirements. Classification: `MODERNIZACAO_COMPATIVEL`. Wire layouts,
persisted schemas and store adapters are unchanged; no new native research is
required. The canonical
[persistence plan](../../DOCS/server/critical-persistence-plan.md) records the
limited scope of the new reservations.

#### Confirmed bug corrected: mutation after final persistence

- **Location:** `wydgo748/internal/game/world.go`, `World.tick`; shutdown entry
  and final persistence in `wydgo748/internal/game/shutdown.go`.
- **Trigger:** a shutdown or maintenance barrier has stopped packet handling,
  but the World ticker continues running before process exit.
- **Impact:** timer/gameplay mutations and additional autosave enqueueing can
  occur after the final snapshot, weakening shutdown durability and potentially
  replacing the intended terminal state. A packet-only barrier is insufficient.
- **Evidence:** the tick path previously lacked the matching early return.
  `TestShutdownTickBarrierProtectsFinalSnapshotAndMaintenance` checks both
  graceful shutdown and maintenance using actual tick/packet methods: HP, tick
  counter and disconnect deadline remain unchanged, and no additional account
  save occurs. This test is not a complete runtime shutdown test.
- **Origin:** present before this batch, including with the dormant coordinator;
  not attributed to the new worker implementation.
- **Correction:** deliver bounded critical completions first, then return before
  gameplay/timer/autosave processing whenever the World barrier is set. Keep
  completion processing alive while accepted transactions settle.

#### Additional staged integration

- `critical_persistence.go` now reserves all account character UIDs and the
  explicit sidecar UID, including deletion. Shared UIDs across distinct batch
  accounts and overlapping pending UIDs are rejected before admission. Account
  and UID fences survive worker completion until owner acknowledgement; failure
  retains both and the pending capacity.
- New `critical_persistence_fences.go` centralizes actor-only fence queries.
  Common account/sidecar persistence wrappers in `world.go` and `charstate.go`
  reject reserved state before reading, clearing live sidecar fields, writing,
  or queuing an autosave. Periodic autosave omits reserved players. UID checks
  also protect an alias/replacement account referencing a reserved character.
- `handlers.go`, `claimAccountSession`, rejects pending canonical account
  admission even after the previous session ownership is released. This guards
  gameplay readmission; it does not claim that authentication performs no reads.
- `shutdown.go` closes admission and retains bounded reply channels while the
  actor continues processing accepted completions. Final RAM snapshots and
  store flush run only after all accepted continuations are delivered. Repeated
  requests share the final result; at most 64 waiters are retained. A caller
  timeout reports incomplete shutdown and does not cancel an accepted write.
- An unknown commit/continuation outcome preserves the original failure and
  skips the terminal snapshot/flush, preventing uncertain durable state from
  being overwritten. Explicit-shutdown disconnect commands close transports
  without removing live aggregates or performing another save. Those aggregates
  remain retained until process exit. No reconciliation/retry is implemented.

#### Validation in this batch

The two new test files are `critical_persistence_fences_test.go` and
`critical_persistence_shutdown_test.go`. Together with the existing foundation
tests, the selected filter executes 29 top-level tests and seven subtests.
Coverage includes all-character/deleted-sidecar fencing, shared-UID rejection,
precommit read/save/autosave exclusion, account readmission after ownership
release, retained failure fences, held-worker shutdown, all-accepted-job drain,
repeated callers, timeout without cancellation, waiter capacity and the tick
barrier. Tests use held adapters and actual World methods; they do not run the
complete `World.Run` loop or production economy handlers through the queue.

| Command/gate | Result and limit |
| --- | --- |
| `go test -count=20 -timeout=60s -run 'TestCritical\|TestShutdownTickBarrier' ./internal/game` | Pass; 29 top-level tests and seven subtests per iteration |
| Focused critical/shutdown/session regression selections | Pass |
| `go test -count=1 -timeout=180s ./...` | Pass; no isolated PostgreSQL fixture configured in this invocation |
| `go vet ./...` | Pass |
| `go build -o bin/tm-check.exe ./cmd/server` | Pass; ignored local output only, not run or deployed |
| `Test-RepositoryLayout.ps1` without `-UpdateMap` | Pass; layout/local links validated, 175 indexed documents |
| `git diff --check`, explicit batch whitespace and authored-English inspection | Pass; includes the new untracked files and uses normal repository CRLF settings; not a global English-migration audit |
| PostgreSQL transaction/autosave fencing | Earlier isolated-fixture evidence reused; adapters/schema unchanged, not newly exercised |
| Race tests | Still pending: suitable CGO toolchain not established; no unsupported rerun |
| C++/assets/runtime | Inputs unchanged; earlier automated evidence and user-reported visual acceptance retained; no client execution/install/deploy |

#### Remaining fixes and recommendations

S2 remains **open**: `NewWorld` does not initialize a coordinator, and no
production handler or tick producer submits to it. Critical production writes
still block the World actor. Do not enable this foundation merely to remove
synchronous calls. Next, cover normal disconnect deferral and retained
account/session/character generations; combat, buffs, rewards and other
players' mutations; shared trades/shops/ground resources; remaining direct
account/sidecar store calls; guild/instance aggregate forms; and durable outcome
reconciliation. Then migrate one complete operation family and test the actual
production path, including held persistence and lifecycle failures. S10's
global English-migration debt also remains open and blocks release readiness.

No files were removed. Operational services/data and the installed client were
not changed. This batch is automated-tested integration progress, not complete
S2 remediation, gameplay parity proof or global release approval.

### Follow-up: pending-write lifecycle and disconnect retention — 2026-10-04

This batch supersedes the previous follow-up's pending normal-disconnect and
participant-identity checks for the staged coordinator. S2 remains open and
disabled in production. Classification: `MODERNIZACAO_COMPATIVEL`; no wire,
ABI, schema or adapter changes, and no new native research requirement.

#### Staged hazard corrected: disconnect releases an unsettled account

- **Location:** `wydgo748/internal/game/handlers.go`, `World.onDisconnect`;
  new `critical_persistence_lifecycle.go`, `deferCriticalDisconnect` and
  `resumeCriticalDisconnects`.
- **Trigger:** a physical disconnect reaches World while an accepted critical
  transaction is queued, running or completed but not acknowledged.
- **Impact:** the common final save rejects the reserved account as busy, but
  ordinary cleanup can still release account/character ownership. A new session
  or late completion could then operate on the wrong lifecycle. This is a
  staged-integration hazard, not an active asynchronous gameplay defect:
  `NewWorld` still does not initialize the coordinator.
- **Correction:** close the transport and release authentication accounting,
  but retain pending live aggregates and ownership. Repeated disconnects retain
  one entry per session. After successful World delivery, resume normal cleanup
  in batches of at most eight eligible entries; never clean a replacement
  player using the old disconnect identity. Unknown outcomes retain ownership
  under the maintenance barrier rather than overwrite uncertain durable state.
- **Evidence:** `TestCriticalLifecycleDisconnectDefersCleanupUntilOwnerDelivery`
  holds the worker, dispatches repeated real disconnect commands, rejects
  readmission and closed-transport packets, verifies no premature final save,
  then checks exactly one final save/cleanup after actor acknowledgement.
  Additional tests cover unknown outcomes and explicit shutdown.
- **Remaining correction:** final disconnect persistence and trade/shop/spatial
  cleanup are still synchronous. Migrate their full ownership/resource flow
  before enabling nonblocking production producers.

#### Identity-bound completion and admission

The new actor-only `World.submitCriticalWrite` API captures exact participant
session/player/account/character pointers, canonical ownership, character slot
and UID, world indexes and an opaque lifecycle token. Character reset clears
the token: restoring the same pointers, IDs and slot cannot resurrect an old
binding. Account/character pointer replacement and account-slice reallocation
also invalidate the binding. Workers receive only detached snapshots.

Admission rejects invalid participant sets, closed/poisoned sessions, missing
ownership, unsupported stores, busy/full queues and stopping World without
installing tokens or scheduling callbacks. Proposed detached account copies
must match their authenticated participants' canonical names. Producer-level
validation of proposed state and all shared resources remains required.

Completion validates **every** retained participant before invoking the producer
continuation. Any mismatch enters maintenance and retains fences; no partial
multi-account publication occurs. A store error/unknown outcome cannot publish
or roll back. A prior maintenance failure suppresses a later successful result.
Low-level `criticalPersistenceOwner.submit` remains a foundation interface, not
a lifecycle-safe API for future production handlers.

`World.handle` now discards queued packets from closed transports and blocks
reserved-player packet dispatch before movement or handler mutation. Common
`advancePlayerMovement` also blocks reserved state for both packet and tick
callers, including completed-but-unacknowledged writes. This does not guard
other players' combat targets, buffs, rewards, shared objects or administrative
commands. Movement resumes against its existing deadline after acknowledgement;
this foundation does not introduce a new movement packet contract.

#### Validation and coverage limits

The new `critical_persistence_lifecycle_test.go` adds 11 top-level tests and
28 subtests. The critical/shutdown filter now executes 40 top-level tests and
35 subtests, confirmed from Go JSON test events. It covers invalid admission,
identity reuse, character reset/reentry, stale account-slice pointers, held
disconnects, unknown outcomes, reserved logout and movement, busy/full rejection,
all-participant validation, selection-session success, shutdown and maintenance.
Tests use actual World methods with injected held adapters. The worker adapter
and final-save counting adapter in the lifecycle fixture are deliberately
separate; these are not migrated production economy paths or complete runtime
integration evidence.

| Command/gate | Result and limit |
| --- | --- |
| `go test -count=20 -timeout=120s -run 'TestCritical\|TestShutdownTickBarrier\|TestCharacterRuntimeIsFullyReset' ./internal/game` | Pass; repeated foundation/lifecycle and character-reset regression coverage |
| `go test -count=1 -timeout=60s -json -run 'TestCritical\|TestShutdownTickBarrier' ./internal/game` | Pass; confirmed 40 top-level tests and 35 subtests |
| `go test -count=1 -timeout=180s ./...` | Pass; no isolated PostgreSQL fixture configured |
| `go vet ./...` | Pass |
| `go build -o bin/tm-check.exe ./cmd/server` | Pass; ignored local output, not run or deployed |
| `Test-RepositoryLayout.ps1` without `-UpdateMap` | Pass; layout and local links validated, 175 documents indexed |
| `git diff --check`, explicit batch whitespace and authored-English inspection | Pass; includes the new untracked lifecycle files; not a global language audit |
| PostgreSQL/race | Adapter/schema evidence reused; no new PostgreSQL run. Race gate still pending a supported CGO compiler |
| C++/runtime | Inputs unchanged; earlier evidence and user-reported visual acceptance retained separately, no new client execution/deploy |

An initial test compile failed because it used a nonexistent session helper;
the test now uses `QueuedPacketsForTest`. An initial logout fixture was rejected
by security before reaching the handler; it now uses the valid inbound-frame
helper. Passing assertions therefore exercise the intended dispatcher path.

Remaining S2 work: target-side combat/buff/reward and shared-resource fences;
remaining direct store callers; nonblocking final disconnect persistence;
guild/instance aggregate forms; definite-rejection versus unknown-outcome
adapter contracts and reconciliation; and migration/validation of complete
production operation families. Do not initialize the queue merely because the
foundation passes. S10's remaining English migration also blocks release.

Changed product files in this follow-up: `world.go`, `handlers.go`,
`character_session.go`, `player_movement.go`, `critical_persistence_owner.go`,
and the new lifecycle implementation/test pair. The existing persistence plan,
language-migration record and this audit record were updated. No files were
removed; no installed client, operational services or data were changed.

### Follow-up: target-side combat and tick reservations (2026-10-04)

**S2 remains open.** This bounded batch guards additional consumers of staged
critical-persistence reservations. `NewWorld` still does not initialize the
coordinator; no production operation family has been migrated to submit writes.
The findings below are confirmed hazards when a reservation is present, not a
claim that the disabled coordinator currently causes production data loss.
The functional classification is `MODERNIZACAO_COMPATIVEL`: internal ownership
guards, with no new wire, ABI, schema or native-behavior boundary. Existing
native evidence is retained; no new Ghidra research was necessary.

#### Confirmed reservation hazards and corrections

| Location | Trigger, impact and origin | Applied correction |
| --- | --- | --- |
| `wydgo748/internal/game/combat.go:15`, `skill_pvp.go:175,184` | Another player attacks a reserved target, or a direct multi-target skill encounters a reserved later participant. Packet admission only checks its sender; caster sacrifice, teleport, target effects or combat publication could precede a reservation rejection. Preexisting consumers did not know about the staged S2 coordinator. | Reject reserved attacker/target before PvP authorization, including war exceptions. Preserve a reserved caster's hide affect. Preflight the caster and every direct-skill target before costs, movement or mutation. Invalid participant sets also reject before mutation. |
| `wydgo748/internal/game/gameplay_space.go:61`, `mob_combat.go:26,217,241` | Mob perception/actions, direct attacks or boss damage reach a reserved player without a player packet. HP, Mana Control MP, mount damage, attacker identity and death/publication could change while the snapshot is pending. Retargeting alone could also consume the retained target or chase deadline. Existing combat consumers lacked reservation checks. | Common target authorization excludes reserved players. Retained target chasing pauses before target loss/movement; active attack authorization pauses before the attack deadline. Direct attack and common damage boundaries reject before damage/roll/publication. |
| `wydgo748/internal/game/cp.go:16,77` | A due negative-CP tick mutates before a fenced save rejects, consuming its recovery deadline despite rollback. Neutral/initial deadlines also change without a save. A kill batch can alter earlier participants or war state before encountering a reserved later victim. Existing consumers lacked pre-mutation guards. | Skip reserved players before any deadline mutation; preflight the killer and every victim before penalties, cleanup or publication. Existing recovery cadence and atomic-save behavior remain intact. |
| `wydgo748/internal/game/timed_items.go:152,188` | Expiry/entry preparation mutates equipment, inventory, Cargo or inactive characters before the save wrapper reports busy. Rollback at the save boundary is not a substitute for reservation admission. Existing consumers lacked pre-mutation guards. | Entry preparation returns busy and periodic expiry skips the reserved account before removals/activation. Expiry resumes at the next eligible sweep using the original absolute timestamp; reservation does not extend item lifetime. |

These are activation-blocking S2 hazards, now corrected for the listed entry
points. They are not proof that all cross-player/shared-resource mutation paths
are guarded. In particular, support/periodic effects, rewards, summon-owner
state, trades, ghost shops, guild/instance aggregates and administrative flows
still require complete participant/resource review before queue activation.
Mob home-leash/instance-area correction and boss scheduling are not globally
frozen by the target-retention guard; no such coverage is claimed.

#### Regression evidence

`critical_persistence_combat_test.go` adds 10 top-level tests and 26 subtests.
Held-save cases call actual World combat/tick methods with an injected adapter,
first while its worker is blocked and again after worker completion but before
owner acknowledgement. They check character snapshots, attacker/death state,
packets, CP deadlines, persistence counts and mob chase/attack state as relevant.
After acknowledgement, the same consumers must actually resume damage, Mana
Control, caster costs, movement, CP recovery, atomic penalties or expiry.

Coverage includes physical/magic PvP, direct mob attacks, boss spells, common
precomputed damage, active mob actions and real retained-target chasing;
Explosion Bash mana/movement and HP-sacrifice skills with a reserved caster,
first target or later target; hide removal; invalid direct-skill participant
sets; negative/neutral/initial CP deadlines; killer/later-victim penalties; and
expired equipment, inventory, Cargo and an inactive account character.
The expiry case uses distinct persisted item UIDs. The separate lethal-skill
case decrypts actual outbound packets to verify the English death notification.
Entry preparation also covers empty accounts, valid live metadata and invalid
live metadata. Busy rejection must precede both a normal unchanged return and
metadata validation; these cases cannot pass merely because a save rolls back
an earlier mutation. The periodic expiry snapshot assertions establish no
observable removal/publication; pre-mutation ordering additionally relies on
inspection of the guard before `expireTimedAccountItems`.

The injected worker and synchronous counting store are deliberately separate.
These tests verify reservation consumers, not migrated production save
producers, durable PostgreSQL integration or gameplay runtime acceptance.

| Command/gate | Result and limitation |
| --- | --- |
| `go test -count=20 -timeout=120s ./internal/game -run 'TestCritical\|TestManaControl\|TestMobCombat\|TestMobAttack\|TestPvP\|TestChaos\|TestTimed\|TestPlayerSkill\|TestSkillPvP'` | Pass; repeated critical/combat/CP/item coverage including the new real chase case |
| Focused `go test -count=1 -timeout=60s -json` with the new-test filter | Pass; 10 top-level tests and 26 subtests, not a count of behavioral assertions |
| `go test -count=1 -timeout=180s ./...` | Pass for the changed product batch; no isolated PostgreSQL fixture configured |
| `go vet ./...` | Pass |
| Repository layout/local links without `-UpdateMap`; `git diff --check`; explicit new-file whitespace and affected-file English inspection | Pass; bounded authored-file validation, not a global English audit |
| PostgreSQL/race | Unchanged adapter/schema evidence reused. No new database run; race remains pending the previously recorded supported-CGO-toolchain prerequisite, not retried here |
| Client/build/runtime | C++ inputs unchanged; no C++ rebuild, game execution or deploy. Earlier user-reported visual acceptance remains separate from this server-side batch |

An initial test compilation exposed an incomplete `wire.AttackHit` fixture
call. The fixture now supplies all existing packet-builder parameters. No
production packet-builder signature or contract changed.

Changed in this follow-up: `combat.go`, `mob_combat.go`, `gameplay_space.go`,
`skill_pvp.go`, `cp.go`, `timed_items.go` and the new
`critical_persistence_combat_test.go`, plus the existing persistence plan,
language-migration record and this audit. No files were removed; no operational
data/services or installed client files were changed.

Next S2 batch: pre-mutation guards for support/periodic effects and reward
recipients, including their shared aggregates. Complete direct-store and
disconnect migration, definite-rejection/unknown-outcome reconciliation and
production-operation tests remain required. S10's English migration remains
open. Passing this bounded batch does not authorize enabling the coordinator
or claiming release readiness.

### Follow-up: support, periodic effects and regeneration reservations (2026-10-04)

**S2 remains open.** This batch continues the target-side reservation work
above. The coordinator is still disabled in `NewWorld`, with no production
operation family migrated to submit writes. These findings concern actual
consumers under an injected reservation; they do not establish production data
loss from the disabled coordinator. Classification: `MODERNIZACAO_COMPATIVEL`.
No wire, ABI, schema, formula or new native boundary changed; existing evidence
was reused without restarting Ghidra research.

#### Confirmed hazards and applied corrections

| Priority and location | Trigger, impact and origin | Correction and limitation |
| --- | --- | --- |
| P1 activation gate: `wydgo748/internal/game/affects.go:274,413`, `skills.go:490` | A support cast selects a reserved explicit target or later party member. Caster MP/cooldown, an earlier target's healing/buff, revival cost or teleport can change before any fenced save detects the conflict. Preexisting support consumers did not participate in the staged S2 reservation system. | Reject reserved casters/explicit targets and the whole selected party cast before costs. Direct application separately preflights all participants, including invalid character/target pointers, before applying any result. The tests exercise heal, revival, teleport, cleanse and buff entry points; this is not a complete summon/shared-state migration. |
| P1 activation gate: `wydgo748/internal/game/affects.go:942,1017,1103` | A world tick bypasses packet admission and changes reserved target HP/MP, affect expiry or `NextTick`. A reserved effect owner can also cause damage/rewards through an unreserved target. Resolving source validity first can discard a disconnected reserved owner's mob affect before acknowledgement. Existing periodic consumers lacked these guards. | Skip reserved players before any per-player tick mutation. Check the retained stable-UID owner before source validity, expiry and damage in player/mob effects; pause the individual owner-linked effect. Direct area damage also rejects reserved casters. Unrelated effects on an unreserved target may still advance. Party reward recipients and other shared aggregates remain uncovered. |
| P1 activation gate: `wydgo748/internal/game/regen.go:31` | Regeneration restores HP/MP or consumes `NextRegen` while a detached critical snapshot is still pending, including worker-completed but unacknowledged writes. This preexisting tick bypasses sender admission. | Check reservation before deadline advancement or regeneration. After owner acknowledgement, existing three-second cadence resumes; dead characters are not revived. |

Reservations preserve stored deadlines while blocked, not elapsed buff lifetime.
On resume, the existing absolute-time expiry/source rules apply; missed periodic
damage is not replayed. Stable owner checks are not a substitute for guarding
every kill-reward recipient or summoned/shared object.

#### Regression and integration evidence

`critical_persistence_effects_test.go` adds **7 top-level tests and 38 subtests**,
confirmed by structured `go test -json` events, not an assertion count. Reserved
cases invoke actual support/skill/tick entry points with the injected held-save
coordinator twice: while the worker is held and after completion but before
World acknowledgement. Snapshot, runtime position/deadline and outbound packet
checks prevent partial support, HP/MP changes and premature effect removal.
After acknowledgement, tests require actual resumed mutation/publication,
including teleport position, revival cost, poison Mana Control, area damage and
invalid disconnected-owner cleanup. Six invalid support-participant scenarios
prove rejection before a valid earlier target can be healed.

An initial party-heal test failed because its fixture provided neither mastery
nor base healing, making its resume assertion impossible. The fixture now
supplies `InstanceValue: 100`; no production healing formula was changed.

| Command/gate | Result and limitation |
| --- | --- |
| `go test -count=20 -timeout=120s ./internal/game -run 'TestCritical\|TestManaControl\|TestMobCombat\|TestMobAttack\|TestPvP\|TestChaos\|TestTimed\|TestPlayerSkill\|TestSkillPvP\|Test.*Support\|TestTick.*Affect\|Test.*Regen\|Test.*Heal\|Test.*Revive\|Test.*Resurrection'` | Pass; repeated critical/combat/support/effect/regen regression coverage |
| `go test -count=1 -timeout=60s -json ./internal/game -run 'TestCritical(Support\|PartySupport\|PlayerEffect\|EffectOwner\|PlayerRegen\|AreaEffect)'` | Pass; 7 top-level tests and 38 subtests |
| `go test -count=1 -timeout=180s ./...` | Pass; game 3.924 s and webadmin 7.404 s. No isolated PostgreSQL fixture configured |
| `go vet ./...` | Pass |
| Repository layout/local links without `-UpdateMap`, `git diff --check`, explicit new-file whitespace and affected-file English review | Pass; 175 documents indexed. Git emitted existing LF-to-CRLF notices, not whitespace errors. Bounded review, not a global English audit |
| PostgreSQL/race | No adapter/schema change; earlier evidence reused. No new database run. Race remains pending the previously recorded supported-CGO-toolchain prerequisite and was not retried |
| Client/build/runtime | No C++ inputs or build dependencies changed; no rebuild, game execution or deploy. Earlier user-reported visual acceptance remains separate from this server batch |

Changed in this follow-up: `affects.go`, `skills.go`, `regen.go` and the new
`critical_persistence_effects_test.go`, plus this audit, the existing persistence
plan and language-migration record. No files were removed; operational data,
services and installed client files were not changed.

Next S2 batch: preflight kill-reward recipients and their shared aggregates,
then summon-owner/shared-state consumers. Complete direct-store/nonblocking
disconnect migration, adapter outcome/reconciliation contracts and production
operation tests remain required before enabling the coordinator. S10 remains
open; this batch cannot be used to declare the entire audit fixed or a global
English release ready.

### Follow-up: summon reservations and party reward preflight (2026-10-04)

**S2 remains open.** The coordinator remains disabled in `NewWorld`, and no
production persistence producer was migrated in this batch. These are confirmed
activation hazards exercised through actual World consumers under injected
reservations, not claims of production loss from a disabled queue.
Classification: `MODERNIZACAO_COMPATIVEL`; no packet layout, schema, combat
formula, face ID or native boundary changed. Existing evidence was reused.

#### Confirmed hazards and applied corrections

| Priority and location | Trigger, impact and origin | Correction and limitation |
| --- | --- | --- |
| P1 activation gate: `wydgo748/internal/game/skill_summons.go:313,161,109` | A summon tick runs while its owner has a held or completed-but-unacknowledged write. It can rebind gameplay space, follow, consume deadlines and attack, including PvP kills that change owner state. The preexisting tick did not participate in the staged S2 reservation system. | Reject reserved owners before all tick mutations, including passive mount pets and thorn-wall rebinding. Direct BM creation and contract planning also reject reserved owners and missing characters. Existing post-save contract publication was not changed into an asynchronous continuation. |
| P1 activation gate: `wydgo748/internal/game/skill_summons.go:276` | A valid PvP command points to a reserved target. Existing PvP authorization rejects it, then target selection clears the owner's command and possibly follows or rebinds the summon. | Distinguish a paused command from an invalid command. Check primary and fallback reservations before cleanup, movement, rebinding or attack deadlines. Resume the retained command after acknowledgement. A valid primary command still takes priority over an unrelated reserved fallback. |
| P1 activation gate: `wydgo748/internal/game/skill_summons.go:284,295`; `wydgo748/internal/game/critical_persistence_fences.go:39` | A summon kills a mob for an unreserved owner while an eligible party EXP recipient is reserved. Reward planning can mutate that recipient's character before persistence detects a conflict, and a late guard would leave already-published lethal damage requiring reconciliation. This was an unguarded preexisting producer. | Preflight the killer and every eligible reward recipient before damage or owner-command cleanup. Do not remove reserved recipients from EXP eligibility or change bonuses. Tests verify zero damage, reward mutation, save or publication during both reservation phases, then one two-account reward commit after acknowledgement. This helper is integrated only into summon targeting so far, not every physical/skill/effect producer or guild/instance aggregate. |
| P2 language debt: `wydgo748/internal/game/skill_summons.go:24,227,240,410,413` | Generated summon names, creation diagnostics and death-notification kind still contained Portuguese. Existing debt conflicts with English-only delivery. | Translate all authored prose in this affected file. Preserve faces, ordering, formulas and the `^` display suffix. Names are generated display text with no name-based consumer found; death kind only formats the whisper. Tests spawn all eight templates and decrypt the real lethal summon notification. External contract-summon names remain data-migration scope. |

#### Regression and validation evidence

`critical_persistence_summons_test.go` adds **8 top-level tests and 23 subtests**,
confirmed by `go test -json` run events. Held writes use the actual staged
coordinator, with checks during worker execution and after completion but before
World acknowledgement. Tests verify frozen character snapshots, owner commands,
summon/mob state, deadlines, outbound queues and save counts, followed by actual
resumed movement, Mana Control damage, summon creation and whole-party rewards.
Dead, out-of-sector and experience-capped party members are not incorrectly
treated as EXP recipients; a lower-priority held fallback does not block an
unrelated valid primary command.

| Command/gate | Result and limitation |
| --- | --- |
| `go test -count=20 -timeout=120s ./internal/game -run 'TestCriticalSummon\|TestSummon\|TestManaControl\|TestMobKill\|TestBossDrop\|Test.*Contract\|Test.*Cria\|Test.*PartyExp'` | Pass; repeated summon/combat/reward/contract/mount regressions |
| `go test -count=1 -timeout=60s -json ./internal/game -run 'TestCriticalSummon\|TestSummonTemplatesSpawnEnglish\|TestSummonDeathNotificationUsesEnglish'` | Pass; 8 top-level tests and 23 subtests |
| `go test -count=1 -timeout=180s ./...` | Pass; no new isolated PostgreSQL validation |
| `go vet ./...` | Pass |
| PostgreSQL/race | Adapter/schema unchanged; previous evidence reused. No isolated database fixture configured. Race remains pending the previously recorded supported-CGO-toolchain prerequisite; no unsupported rerun |
| Client/build/runtime | No C++ inputs/build dependencies changed; no rebuild, client launch or deploy. Earlier user-reported visual acceptance remains separate |

Repository layout and local links pass without `-UpdateMap` (175 documents).
`git diff --check` passes; existing LF-to-CRLF notices are not whitespace errors.
The new test and fence files also receive an explicit untracked-file whitespace
check. Affected-file prose is reviewed in English; this is not a global audit.

Changed: `skill_summons.go`, `critical_persistence_fences.go`, new
`critical_persistence_summons_test.go`, this audit, the existing persistence plan
and language record. No files were removed; installed client files, operational
data and services were not modified.

Next S2 work: integrate reward-participant preflight before damage/cost/deadline
mutation in physical/Iron Spear, offensive skill and periodic-effect producers.
Cover mount tick/reconciliation and shared guild/instance/drop aggregates, then
complete production producer/nonblocking-disconnect migration, adapter outcome
reconciliation and actual slow-write integration gates. Summon removal during
lifecycle cleanup and post-commit contract publication must retain their proper
acknowledgement ordering; an early-return guard alone must not silently discard
an already-committed item operation. S10 remains open across untouched active
source, resources, data and documentation. This batch does not authorize queue
activation or global release readiness.

### Follow-up: reward/mount guards and detached CP/egg timers (2026-10-04)

**S2 remains open; this is a partial implementation, not closure.** `NewWorld`
still leaves the coordinator disabled. CP recovery and equipped-egg elapsed
hours now have detached, identity-bound persistence paths under an injected
coordinator; ordinary production still uses their existing synchronous paths.
Most economic operations, normal final disconnect saves and shared aggregates
remain synchronous. Do not infer production activation from green tests.

Classification: `MODERNIZACAO_COMPATIVEL`. Packet layouts, schema, equipment and
effect IDs, combat formulas and authoritative ownership are unchanged. English
mount display text changes intentionally; no new native boundary was introduced
and existing evidence was reused. This is not a new repository-wide audit.

#### Confirmed activation hazards and corrections

| Priority and current location | Trigger, impact and origin | Applied correction and remaining limitation |
| --- | --- | --- |
| P1: `wydgo748/internal/game/handlers.go:1718,1975`; `skills.go:543`; `affects.go:977,1057,1110` | Physical/Iron Spear, offensive skill or periodic poison/area damage reaches a mob while an eligible party reward recipient is reserved. Damage, caster costs, cooldowns or effect deadlines can change before reward persistence detects the conflict. These preexisting producers did not participate in the staged reservation layer. | Preflight the killer and eligible recipients before those mutations, including direct secondary/area entry points. Delay `SkillReady` allocation until an accepted cast. Tests hold an actual write both during worker execution and after completion before acknowledgement, then verify resumed lethal damage and the party EXP commit. This is consumer fencing, not asynchronous kill-reward planning or shared-resource migration. |
| P1: `wydgo748/internal/game/mount.go:149,304,598` and mount mutation entry points | A reserved owner can lose food/HP/longevity, change incubation clocks/items, spawn/remove/rebind its follower, earn mount EXP or consume feed while its aggregate snapshot is pending. A late persistence guard cannot undo unrelated clocks/entities or already-sent packets. This is a preexisting consumer gap before queue activation. | Guard ticks, item/incubation boundaries, follower reconciliation, hunt EXP, damage absorption and rider death. Recheck the reservation immediately after submitting egg hours in the outer mount tick. Explicit lifecycle summon removal is not blindly blocked: later producer migration must order already-committed follower changes after acknowledgement rather than discard them. |
| P1/S2 migration: `wydgo748/internal/game/cp.go:43,64` | Due negative-CP recovery performs its account save on World. Simply moving the save would expose CP or consume its deadline before commit. | Build a detached proposal; reserve identity/account/UID; apply CP and the next 450-second deadline on successful owner completion; retain packet order and omit confirmation to a closed transport. Admission rejection changes no CP/deadline and performs no inline fallback. Unknown outcome retains fences and enters maintenance without publication/retry. This staged path is not enabled by the production constructor. |
| P1/S2 migration: `wydgo748/internal/game/mount.go:328,351` | Charging equipped-egg hours synchronously can block World. An asynchronous save alone would expose new item state/clock progress or let same-tick follower reconciliation consume an uncommitted proposal. | Persist a detached slot-14 item proposal. Apply charged hours, fractional deadline and completion clock reset only after identity-checked acknowledgement. Keep effect 84 and the existing whole-hour/remaining-hour cap. No live mutation, confirmation or inline fallback on rejected admission; no rollback/replay on unknown commit. The coordinator remains disabled in `NewWorld`. |
| P2 language debt: `wydgo748/internal/game/mount.go:566,986` and `handlers.go` attack log | Generated hatchling name, feed/revival messages, mount comments and diagnostics contain Portuguese. Existing affected-file debt violates English-only delivery. | Translate all authored mount prose and the remaining attack log. Preserve faces, item/effect/slot IDs, formulas and established identifiers. `Hatchling^` is generated display text with no name-based consumer found; the suffix is unchanged. Tests materialize the follower and assert the English feed/revival results returned by the mount-feed helper; they do not decode outgoing feed packets. This does not close S10 or the global language migration. |

#### Actual actor-loop and lifecycle evidence

`critical_persistence_loop_test.go` executes the same actor loop as `World.Run`,
not a surrogate queue alone. The only test-specific controls are injected tick
delivery and a joined stop signal. A due CP tick submits a real detached
recovery; while its worker is held, unrelated regeneration and overview commands
continue through three further ticks. No CP packet is emitted before commit.
After release the actor consumes the completion, confirms recovery and releases
the fence. A second test injects a lost commit acknowledgement: World enters
maintenance, closes transports and retains ownership without another mutation.

**Coverage limit:** autosave is deliberately scheduled outside this test window
because its synchronous fallback remains pending migration. These tests do not
prove every normal World command is nonblocking, nor do they execute PostgreSQL
or exercise production coordinator initialization.

`critical_persistence_cp_test.go` also sends actual World disconnect commands
while recovery is held, repeats disconnect and attempts queued logout/readmission.
Ownership and indexes remain intact through completed-but-unacknowledged work.
Successful delivery installs committed CP before the old final save, performs
cleanup once and then permits readmission. Unknown commit prevents final save,
confirmation and readmission. **The resumed final save remains synchronous**;
this ordering test does not close the nonblocking-disconnect requirement.

Egg tests cover one/two/three/overdue hours, held and unacknowledged phases,
saturation, stopping, identity rejection, missing account and unknown outcome.
They retain item snapshots, both clocks, the follower and outbound queue until
acknowledgement. A subsequent eligible tick reconciles the stale follower and
food clock; it does not repeat charged hours. Mount-consumer tests cover direct
and tick paths, clock initialization/reset, follower spawn/removal/rebinding,
EXP, damage, rider death, feed, refinement and accelerated hatching.

#### Validation

All Go commands below ran from `wydgo748/`.

| Command/gate | Result and limitation |
| --- | --- |
| `go test -count=20 -timeout=120s ./internal/game -run 'TestCriticalRewardRecipients\|TestCriticalChaos\|TestCriticalEgg\|TestCriticalMount\|TestMountFeedMessages'` | Pass; repeated reward/CP/mount/egg/actor-loop/lifecycle regressions |
| `go test -count=1 -timeout=60s -json ./internal/game -run 'TestCriticalRewardRecipients\|TestCriticalChaos\|TestCriticalEgg\|TestCriticalMount\|TestMountFeedMessages'` | Pass; 14 top-level tests and 40 subtests, counted from run events; zero failures |
| `go test -count=1 -timeout=180s ./...` | Pass, including game, store, webadmin and wire; missing service-dependent fixtures are not new transaction evidence |
| `go vet ./...` | Pass |
| `Test-RepositoryLayout.ps1` (repository root, without `-UpdateMap`) | Pass; 175 documents, current map and checked links |
| `git diff --check` and explicit read-only whitespace/final-newline checks for seven affected untracked files | Pass; the separate checks cover new test files, the persistence plan and this audit, which tracked-only diff validation omits |
| PostgreSQL/race | No adapter/schema change; earlier evidence reused. No isolated database fixture configured. Race validation remains pending the previously recorded supported-CGO-toolchain prerequisite; no unsupported rerun |
| Client/build/runtime | No C++ inputs/build dependencies changed; no rebuild, client execution, deployment or operational service/data changes. Earlier user-reported visual acceptance remains separate |

Changed in this follow-up: `cp.go`, `mount.go`, `handlers.go`, `skills.go`,
`affects.go`, `world.go`, staged-owner/lifecycle comments and the CP expectation
in `critical_persistence_combat_test.go`; new
`critical_persistence_rewards_test.go`, `critical_persistence_cp_test.go`,
`critical_persistence_mounts_test.go`, `critical_persistence_incubation_test.go`
and `critical_persistence_loop_test.go`; this audit, the existing persistence
plan and language record. No file removal belongs to this batch. In particular,
the preexisting deletion of the installed `project.exe` was not performed or
restored here.

**Remaining closure gates:** migrate all other critical economy producers into
detached operation plans; make final disconnect persistence nonblocking; support
atomic guild/instance forms and shared-object/admin reservations; certify store
transaction outcomes and reconciliation; remove normal-loop synchronous
autosave/direct-store fallbacks; activate `NewWorld` only after those migrations;
then run complete slow-write, isolated database and supported race integration.
The reservation and CP/egg work above resolves the earlier follow-up's immediate
physical/skill/effect/mount tasks, not these independent closure gates. S2 and
S10 remain open; neither audit completion nor global release readiness is claimed.

### Follow-up: staged asynchronous final account disconnect saves — 2026-10-04

**S2 remains open.** This batch supersedes the preceding follow-up's statement
that the injected-owner path performs a synchronous final account save. The
production constructor still leaves the coordinator disabled. Default saves,
economic operations, autosave and shared-instance cleanup remain synchronous.
Classification: `MODERNIZACAO_COMPATIVEL`; no wire, schema, asset or native
boundary changed. No new Ghidra work or client execution was required.
Repository HEAD remained `34b7dcae4068f1b6bcfc152e4037bd9e7ccf0dfa`; other
preexisting tracked/untracked changes were preserved. No files were removed.

#### Changes and correction rationale

| Location | Trigger and impact | Correction and evidence |
| --- | --- | --- |
| `wydgo748/internal/game/critical_persistence_lifecycle.go`, `resumeCriticalDisconnects`; `handlers.go`, `onDisconnect` / `finishDisconnect` | A physical disconnect previously resumed its final account/sidecar save synchronously after an earlier staged write. A slow final store call still blocked unrelated World commands/ticks. Existing S2 architecture issue. | Submit a detached final snapshot, retaining session/account/character/UID/index ownership through actor acknowledgement. Atomic sidecar-capable stores receive one account/sidecar operation, not two independent saves. The private closed-session admission path requires an exact retained disconnect; normal gameplay admission still rejects closed transports. The continuation only marks successful persistence; cleanup runs on the actor afterwards. Updated CP/lifecycle tests prove the earlier commit precedes the final snapshot and readmission waits for both acknowledgements. |
| `critical_persistence_fences.go`, `criticalPlayerReserved` / `criticalAccountStateReserved`; `critical_persistence_lifecycle.go`, `criticalDisconnect` | Final disconnect admission can be rejected because all bounded slots belong to unrelated accounts. Queue reservations alone cannot freeze the disconnected player's gameplay or prevent a cloned account save while it waits. Staged integration hazard exposed by this migration. | Retain an intent fence independently of actual queue fences. Existing guarded consumers and canonical account wrappers honor it. Full/busy admission retries on bounded drains without inline fallback, a new lifecycle token or live mutation. Permanent stopping/unsupported/identity rejection retains ownership and records one diagnostic rather than replaying every tick. Tests include a full queue, frozen due CP recovery, detached-account wrapper fencing, unsupported non-atomic sidecars and repeated disconnect commands. This does not prove every shared consumer is guarded. |
| `wydgo748/internal/game/shutdown.go`, `persistShutdownState` | A queue-rejected disconnect on character selection retains an account with no active character. The previous terminal loop only saved in-world characters and could discard that newly retained account. Staged lifecycle gap, not a claim that all selection-screen shutdown behavior was previously covered. | Include retained account-only disconnects in the terminal synchronous snapshot. Intent fences stop blocking only after gameplay stops; accepted unresolved writes still drain first. Tests cover accepted final writes, account-only accepted/rejected admission and terminal shutdown after unknown final outcomes. An unknown outcome skips terminal writes/flush and retains failure rather than overwriting durable state. |

#### Verification and limits

Changed Go files: `handlers.go`, `world.go`, `shutdown.go`,
`critical_persistence_lifecycle.go`, `critical_persistence_fences.go`,
`critical_persistence_cp_test.go`, `critical_persistence_lifecycle_test.go`,
`critical_persistence_loop_test.go`, and the new
`critical_persistence_disconnect_test.go`. The durable
[critical-persistence plan](../../DOCS/server/critical-persistence-plan.md) and
this audit record were updated; no documentation duplicate was created.

The atomic disconnect test verifies detached account/character data, affect and
currency sidecars, held and completed-but-unacknowledged ownership, repeated
disconnects and cleanup/readmission. Unknown final-store errors, worker panic
and reused lifecycle identity enter maintenance without replay or cleanup.
Admission rejection is separately tested as a known rejection, not a commit
outcome. `TestCriticalDisconnectWorldLoopContinuesDuringFinalSave` drives the
real World actor, holds the final store call, advances three unrelated ticks and
overview commands, and joins the actor before inspecting its maps. Autosave is
deliberately outside the fixture window, and no item instance is present.

All Go commands ran from `wydgo748/`:

- `go test -count=1 -timeout=60s ./internal/game -run 'TestCritical(ChaosRecovery|Lifecycle)'` — pass after migrating the existing ordering tests.
- `go test -count=1 -timeout=60s ./internal/game -run 'TestCritical(Disconnect|ChaosRecovery|Lifecycle)|TestShutdown'` — pass for the new lifecycle/account-only cases.
- `go test -count=20 -timeout=90s ./internal/game -run 'TestCritical(Disconnect|ChaosRecovery|Lifecycle)|TestShutdown'` — pass, including the real actor-loop regression.
- `go test -count=20 -timeout=90s ./internal/game -run 'TestCriticalDisconnect'` — pass after adding explicit terminal-no-overwrite checks to the unknown-final-outcome cases.
- `go test -count=1 -timeout=180s ./...` — pass; game 3.651 s, webadmin 7.305 s. Service-dependent missing fixtures are not new transaction evidence.
- `go vet ./...` — pass.

Repository validation for this follow-up: `Test-RepositoryLayout.ps1` without
`-UpdateMap` passed (175 documents and local links); `git diff --check` passed
with line-ending notices only. A separate trailing-whitespace check passed for
the eight affected untracked files, which ordinary diff checking does not cover.
`gofmt -l` returned no affected files. The affected Go prose scan found no
Portuguese candidates or non-ASCII text, followed by inspection of authored text.

Authored prose in the affected Go files was inspected for the English-only
requirement. The affected English documentation distinguishes this batch from
historical evidence. Native/wire/store adapter inputs were unchanged; previous
evidence was reused. Isolated PostgreSQL validation remains pending because no
fixture is configured, and race validation remains pending the previously
recorded supported-CGO-toolchain prerequisite; neither unsupported gate was
rerun. No client rebuild, deployment, installed executable change, game launch,
operational database mutation or service restart occurred.

**Remaining work / next executable step:** design a detached instance-state
aggregate and shared-instance reservation for disconnect membership, preserving
UID-based Water reattachment and immediate durable cleanup before readmission.
`finishDisconnect` still calls `flushInstanceStateIfDirty` synchronously after
account acknowledgement, outside the continuation. Trade/shop/party cleanup
also touches shared live objects whose reservations need integration. Do not
enable the queue merely because the final account write moved off World.
Then migrate other economic producers and autosave/direct-store fallbacks,
certify adapter outcomes/reconciliation, and run complete held-write, isolated
database and supported race integration before production activation or S2
closure. Static/automated evidence here is not `CLIENT_TESTED` or release approval.

### Follow-up: staged serialized instance flush and disconnect membership — 2026-10-04

**S2 remains open.** This batch supersedes only the previous follow-up's claim
that shared-instance cleanup still performs inline I/O under the injected owner.
`NewWorld` still does not initialize that owner; normal production saves remain
synchronous. Classification: `MODERNIZACAO_COMPATIVEL`. No wire, schema, asset,
native contract, deployed executable or service changed. HEAD remained
`34b7dcae4068f1b6bcfc152e4037bd9e7ccf0dfa`; unrelated changes were preserved.

#### Corrections and evidence

| Location | Scenario, origin and impact | Correction and limits |
| --- | --- | --- |
| `wydgo748/internal/game/critical_persistence.go`, `criticalWrite` / `submit` / `execute` / `acknowledge` | The staged coordinator had no instance aggregate. Moving only account disconnect saves left the existing S2 synchronous membership flush on World. Independently queuing snapshots could reorder the single durable instance aggregate. | Add detached instance-only and exact atomic account/instance forms. Clone nested party-run maps and every member-UID slice. Hold one global instance writer reservation through actor acknowledgement, including completed-but-unacknowledged work. Reject unsupported adapters and combined account/sidecar/instance admission before writing; do not substitute separate transactions. Guild forms remain pending. |
| `critical_persistence_instances.go:19`, `queueInstanceState`; `instance_state.go:542`, `markInstanceStateDirty` / `flushInstanceStateIfDirty` | Live dirty state can change after an older snapshot is admitted. Blindly clearing the dirty flag on its completion loses the newer mutation; inline fallback can overwrite queued state. Staged integration hazards exposed by this migration. | Use opaque actor-only dirty-generation tokens. A completion clears dirty state only for its captured generation. Full/busy admission leaves state dirty and retries later; permanent admission rejection logs once and retains state. Normal staged flush never writes inline, including during shutdown/maintenance. Existing inline instance wrappers reject overlap before writing. This is non-confirming snapshot coalescing, not save-before-confirm economic planning or a freeze of every instance consumer. |
| `critical_persistence_lifecycle.go`, `resumeCriticalDisconnects`; `handlers.go:2312`, `finishDisconnectAfterInstances` | After account acknowledgement, slow instance persistence blocked World. Releasing account/UID/player ownership first also permits reentry before durable membership cleanup. Existing S2 lifecycle issue; an older snapshot cannot cover a disconnect arriving later. | Detach membership once on the actor, retaining resumable Water member and leader UIDs. Hold disconnect-intent identity until a covering instance snapshot is acknowledged, then perform final cleanup once. Capture and validate every retained waiter before admission and again before applying any waiter/dirty mutation. A later disconnect waits for its own covering snapshot. Unknown store errors, panic and identity mismatch retain fences, enter maintenance and suppress replay/terminal overwrites. Shared trade/shop/party consumer reservations remain incomplete. |
| `critical_persistence_instances_test.go`, shutdown and actual actor-loop regressions | Counter-only tests or direct helper calls would not prove snapshot contents, shutdown ordering or continued World responsiveness. Test coverage gap, not production runtime approval. | The shutdown capture verifies the later `GamblePool` value, not merely write counts. The real actor-loop fixture holds the membership write while three unrelated ticks, regeneration and overview requests advance; ownership remains until acknowledgement. Autosave is deliberately postponed, so this test does not prove autosave or other economy paths. |

Coordinator tests cover detached nested data, exact atomic account participation,
unsupported forms, global serialization through acknowledgement, queue-full
retention, later dirty generations and rejection of inline overlap. Lifecycle
tests cover held/completed-unacknowledged ownership, repeated disconnects,
UID-based Water continuation, a second disconnect behind an older snapshot,
unsupported instance admission and explicit shutdown of retained identities.
Unknown outcomes do not retry accepted work or overwrite it with final state.

#### Validation

From `wydgo748/`:

- `go test -count=1 -timeout=60s ./internal/game -run 'TestCriticalInstance|TestCriticalPersistence'` — pass.
- `go test -count=1 -timeout=60s ./internal/game -run 'TestCritical|TestShutdown|TestInstance|TestSaveAccountsAndInstanceState'` — pass after producer integration.
- `go test -count=20 -timeout=90s ./internal/game -run 'TestCriticalInstance|TestCriticalDisconnectInstance|TestCriticalDisconnectLaterMembership|TestCriticalDisconnectUnsupportedInstance'` — pass, latest 0.523 s, including the actual-loop and snapshot-content regressions.
- `go test -count=1 -timeout=180s ./...` — pass; game 3.516 s, webadmin 7.247 s. Fixture-dependent skips remain outside this evidence.
- `go vet ./...` — pass.
- `go build -o bin/tm-check.exe ./cmd/server` — pass; ignored build output only, not deployment or execution.

The first new atomic snapshot expectation failed because the store snapshot
normalizes entry positions; the instance-only expectation also distinguished a
zero-length slice from nil. Expectations were corrected to the established
persistence snapshot contract and relevant account participation. These were
fixture defects, not permission to remove meaningful state assertions. A
literal wildcard `gofmt` invocation failed on Windows; explicit paths succeeded.
A guessed documentation-validator filename did not exist; the actual
`Test-RepositoryLayout.ps1` is the required gate. None of these failed commands
was counted as successful validation.

Repository gates: `pwsh -NoProfile -ExecutionPolicy Bypass -File
tools/repository/Test-RepositoryLayout.ps1` passed without `-UpdateMap` (175
documents and local links). `git diff --check` passed with line-ending notices
only. Explicit affected-path `gofmt -l` returned no files; `go vet
./internal/game` passed after the final test-only snapshot assertion. A separate
trailing-whitespace check passed for all nine affected paths, including
untracked files not covered by ordinary diff checking. The affected Go prose
scan returned no Portuguese candidates or non-ASCII text; authored English
prose was also inspected. No global language audit was repeated.

Changed Go files in this batch: `critical_persistence.go`,
`critical_persistence_lifecycle.go`, `handlers.go`, `world.go`,
`instance_state.go`, and new `critical_persistence_instances.go` and
`critical_persistence_instances_test.go`. The existing
[critical-persistence plan](../../DOCS/server/critical-persistence-plan.md) and
this audit were updated. No files were removed. Portuguese prose encountered
in the touched `instance_state.go` was translated without changing persisted
values or identifiers. This is not a global language-migration completion claim.

Isolated PostgreSQL transaction validation remains pending because no fixture
is configured. Race validation still needs the previously recorded supported
CGO toolchain. These prerequisites were not changed, so unsupported gates were
not rerun. No C++ inputs changed, hence no client build or execution was needed.

**Next executable work:** migrate single-account economic producers with
detached proposed state and commit-bound publication, then multi-account/shared
resource producers and guild forms. Finish shared-instance consumer reservations
and exact combined forms when required; certify adapter outcomes/reconciliation;
remove normal autosave/direct-store synchronous fallback. Only then enable the
constructor and validate the integrated production paths with held writes,
isolated database transactions and race checks. This staged instance batch is
`AUTOMATED TESTED`, not `CLIENT_TESTED`, S2 closure or release approval.

### Follow-up: detached cargo gold transfers — 2026-10-04

**S2 remains open.** Cargo deposits and withdrawals now use a detached account
proposal under the injected critical-persistence owner. `NewWorld` still does
not initialize it; default production persistence remains synchronous until the
remaining families, reconciliation, autosave and activation gates are complete.
Classification: `MODERNIZACAO_COMPATIVEL`. The existing native
[cargo gold contract](client748/flows/transport/cargo-gold-transfer-confirmations.md)
covers unchanged request/response layouts; no new binary research was needed.

| Location | Scenario, impact and origin | Correction and evidence |
| --- | --- | --- |
| `wydgo748/internal/game/handlers.go:719`, `onCargoGold`; `critical_persistence_cargo.go:11`, `queueCargoGoldTransfer` | Existing S2 path changes live balances then blocks World on saving. A naive queue wrapper would expose uncommitted balances and confirm before persistence. | Validate the living in-world participant, banker, exact 16-byte request, amount and caps before planning. Queue a detached account with the selected character's proposed gold and proposed cargo balance; apply only those two fields inside the identity-bound successful continuation. Do not replace live character/account/score identity or normalized entry positions. Admission rejection leaves economic state unchanged and never falls back to a synchronous save. |
| `critical_persistence_cargo.go:36`, `publishCargoGoldTransfer`; `critical_persistence_cargo_test.go:53` | A held or completed-but-unacknowledged write could accept a competing transfer or publish success too early. The native wire has no transaction ID, so permanent duplicate suppression would incorrectly reject later legitimate transfers. | Reserve the participant through acknowledgement. Tests cover both directions and first/second character slots, unchanged sibling fields and identity, then a new identical post-acknowledgement transfer. Assert exact encrypted transfer, cargo balance and character balance frames in their existing order only after the continuation applies committed balances. Exact 2,000,000,000 cap and one-gold boundary tests also pass. |
| `critical_persistence_cargo_test.go:129`, admission; `:182`, domain; `:232`, capacity recovery; `:267`, banker context | Invalid inputs, a full/stopping queue or invalid account/character ownership must not mutate balances, install lifecycle tokens/fences or silently write inline. A reserved intention must not first mutate banker context. | Both directions exercise zero/over-limit/insufficient/destination-limit, dead/out-of-world, hidden/distant banker and truncated/oversized packets; admission covers missing owner/index, bad slot/pointer, closed/poisoned/stopping participants. Rejection sends only the existing domain error or a new admission error panel when appropriate. Capacity recovery proves a rejected request is not retained/replayed and a later new request can commit once. Reservation is checked before banker resolution. |
| `critical_persistence_cargo_test.go:288`, uncertain outcomes; `:376`, disconnect; `:455`, shutdown | Unknown commit or stale identity must not apply/confirm/replay the transfer or overwrite it with a final old-balance snapshot. Disconnect must not free ownership before final persistence; shutdown must wait for accepted work. Existing S2 lifecycle requirements. | Error/panic/identity tests retain fences and readmission denial, enter maintenance without balance publication, and suppress terminal overwrite. Disconnect tests independently hold the transfer and final atomic account/sidecar write through completed-but-unacknowledged phases. The final snapshot includes committed balances; no packets go to the closed transport. Shutdown checks terminal snapshot contents and repeated callers, not merely save counts. |
| `critical_persistence_cargo_test.go:582`, actual dispatcher/World loop; `:629`, maintenance | Helper-only success would not demonstrate continued World processing during the migrated operation or integration of unknown outcomes. | Actual `OpDeposit`/`OpWithdraw` dispatch submits the proposal. Three unrelated ticks, regeneration and overview requests progress during a held write; repeated pending intentions do not create another job. Commit acknowledgement publishes the exact existing three frames. An actual-loop unknown outcome enters maintenance and stops further gameplay mutation without confirmation/replay. Autosave is deliberately postponed; these tests do not validate autosave or production initialization. |

#### Validation

From `wydgo748/`:

- `go test -count=1 -timeout=60s ./internal/game -run 'TestCargoGoldContract|TestCriticalChaosRecovery'` — pass after the producer patch, 0.033 s.
- `go test -count=1 -timeout=60s ./internal/game -run 'TestCriticalCargoGold|TestCargoGoldContract'` — pass for staged admission/proposal/failure tests and again after lifecycle/actual-loop coverage, latest 0.042 s.
- `go test -count=20 -timeout=90s ./internal/game -run 'TestCriticalCargoGold|TestCargoGoldContract'` — pass after capacity-recovery and banker-context coverage, 0.499 s.
- `go test -count=1 -timeout=180s ./...` — pass; game 3.780 s, webadmin 7.278 s. Fixture-dependent skips are not database transaction evidence.
- `go vet ./...` — pass.
- `go build -o bin/tm-check.exe ./cmd/server` — pass; ignored output confirmed with `git check-ignore`, no deployment or execution.

Changed source files: `handlers.go`, new `critical_persistence_cargo.go` and
new `critical_persistence_cargo_test.go`. The existing critical-persistence plan
and this audit are updated; no files were removed. The affected producer/handler
and test prose is English. A global language migration was not repeated or
declared complete. Original synchronous cargo contract tests remain valid for
the disabled-owner path; uncertain staged outcomes deliberately use conservative
maintenance rather than pretending a rollback is safe.

Isolated PostgreSQL validation still lacks a configured fixture, and race
validation still lacks the previously recorded supported CGO toolchain. No
adapter/schema inputs or those prerequisites changed, so neither unsupported
gate was rerun. No C++ inputs changed, hence no client build or execution.
Repository gates: `pwsh -NoProfile -ExecutionPolicy Bypass -File
tools/repository/Test-RepositoryLayout.ps1` passed without `-UpdateMap` (175
documents and local links). `git diff --check` passed with line-ending notices
only. Explicit affected-Go-path `gofmt -l` returned no files. A separate
trailing-whitespace check passed for all five affected paths, including untracked
files omitted by ordinary diff checking. The affected-file Portuguese candidate
scan and affected-Go non-ASCII scan returned no matches; authored English prose
was inspected as well. These are scoped gates, not a global language audit.

**Next executable work:** migrate remaining single-account economic producers,
including cargo item/timed equipment moves with their trade, appearance and mount
lifecycle effects; then multi-account/shared-resource producers and guild/instance
atomic forms. Finish shared-resource reservations, certify adapter outcomes and
reconciliation, remove normal-loop synchronous autosave/direct saves and enable
the constructor only after complete integrated validation. This cargo batch is
`AUTOMATED TESTED`, not S2 closure, `CLIENT_TESTED` or release approval.

### Follow-up: detached Hell Gate purchases — 2026-10-04

**S2 remains open and production activation is disabled.** This batch migrates
only the zero-treasury Hell Gate producer. Taxed merchant/ticket callers still
need guild/shared-resource atomic plans. Classification:
`MODERNIZACAO_COMPATIVEL`; unchanged native layouts, prices and option rules reuse
the [Hell Gate contract](../../DOCS/protocol/hell-gate-store.md).

| Location | Existing S2 hazard / scenario | Correction and evidence |
| --- | --- | --- |
| `hell_store.go:178`, `onHellStoreBuy`; `critical_persistence_purchases.go:7` | Live gold/item mutation followed by blocking persistence; a naive asynchronous save would expose or confirm an uncommitted item. | Submit a detached account containing the charged gold and materialized item. Apply only those fields after successful identity-bound delivery, retaining unrelated account/character identity. Prices, RNG tables, exact 24-byte requests, slot/NPC validations and legacy synchronous fallback remain unchanged while activation is disabled. |
| `hell_store.go:246`, publisher; `critical_persistence_purchases_test.go:54`, `:127` | Duplicate requests, queue saturation or invalid participants could grant, charge, reroll or publish early. | All four categories and both selected character slots cover held/completed-unacknowledged fences, normalized worker snapshots, unchanged siblings and live identity. Domain/admission rejection does not charge, grant or write inline. A filled post-commit slot rejects another request; a new empty slot permits a new economic UID/charge. Exact finalized frame sizes, recipients, opcode order and entire inventory/character payloads are asserted. |
| `critical_persistence_purchases_test.go:211` | Cancelling an existing trade can mutate another reserved participant before admission. | Preflight a reserved counterpart before cancellation or RNG. Both trade contexts and packet queues stay unchanged until its acknowledgement. Eligible cancellation retains the legacy position and close-trade response order, including before later failures. |
| `critical_persistence_purchases_test.go:257`, `:292`, `:384` | Unknown outcome, stale identity, disconnect or shutdown could replay a charge or overwrite committed/uncertain inventory. | Error/panic/identity tests retain fences and deny readmission, without grant/charge/publication or terminal overwrite. Final atomic account/sidecar snapshots include committed UID/options/gold. Shutdown inspects the complete terminal snapshot, waits for acknowledgement and returns repeated callers without re-saving; closed transports receive no confirmation. |
| `critical_persistence_purchases_test.go:328`, `:432`; common actor fixture `critical_persistence_cargo_test.go:516` | Helper tests alone do not prove actual dispatcher scheduling. | Held actual purchases allow three unrelated ticks, regeneration and overview commands to advance; success applies exactly once. Actual-loop unknown outcomes enter maintenance and stop gameplay without replay or publication. Shared fixture configuration occurs before actor startup; mutable state is inspected after joining the actor. Autosave is postponed and not certified. |

Validation from `wydgo748/`:

- Initial new publication tests failed because the test compared finalized
  `Header.Size` with the unfinished builder's zero size. Inspection of `Build`
  and `Session.Send` proved that finalization fills size/tick; no product change
  or payload assertion was needed. The corrected test validates size against
  the expected frame length and retains full payload/opcode/recipient ordering.
- `go test -count=1 -timeout=60s ./internal/game -run
  'TestCriticalHellStore|TestCriticalCargoGold|TestHellStore'` — pass, 0.619 s.
- `go test -count=1 -timeout=60s ./internal/game -run 'TestCriticalHellStore'`
  after additional shutdown/actual-loop failure tests — pass, 0.457 s.
- `go test -count=20 -timeout=90s ./internal/game -run
  'TestCriticalHellStore|TestCriticalCargoGold|TestHellStore|TestCargoGoldContract'`
  — pass, 17.990 s, including the modified shared actor fixture.
- `go test -count=1 -timeout=180s ./...` — pass; game 4.194 s, webadmin 7.252 s.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` — pass.

Changed this batch: `hell_store.go`, new `critical_persistence_purchases.go` and
`critical_persistence_purchases_test.go`, shared fixture in
`critical_persistence_cargo_test.go`, this audit, the existing persistence plan
and Hell Gate contract. No removals, client edits, deployment, execution or
operational data writes. Isolated PostgreSQL and supported CGO/race prerequisites
remain absent as previously recorded; green ordinary suites do not substitute
for those gates. The next producer is cargo item/timed-equipment movement,
followed by shared economy, adapter reconciliation, autosave migration and
constructor activation. This is staged `AUTOMATED TESTED`, not S2 completion.

### Follow-up: staged inventory persistence and coverage reconciliation — 2026-10-04

**S2 remains open; `NewWorld` does not activate critical persistence.** This
follow-up closes the staged deletion/splitting test gap and updates the existing
plan to reflect implemented expiry, entry preparation, zero-treasury merchant,
skill-learning and inventory paths. Those paths were previously missing from
the plan's progress summary. Classification: `MODERNIZACAO_COMPATIVEL`; existing
packet layouts and item semantics are preserved, with no new native claim.

| Location | Scenario and impact | Correction and evidence |
| --- | --- | --- |
| `wydgo748/internal/game/inventory_handlers.go`, `onDeleteItem` / `onSplitItem`; `critical_persistence_inventory.go`, `queueInventorySlotChanges` | Changing live slots before a delayed save would expose uncommitted deletion or a split and allow competing requests to operate on it. | The injected owner persists a detached full-account proposal. Successful identity-bound delivery installs all changed slots before sending the existing inventory snapshots. Admission rejection does not save inline or mutate inventory; corrective snapshots contain the old state. The nil-owner production path remains synchronous. |
| `critical_persistence_inventory_test.go`, `TestCriticalInventoryDetachedCommitAndRepeatedRequests` | Worker aliasing, wrong selected-character slots, quantity effect positions or regenerated split identifiers could corrupt committed state. | Cover source slots 0/5/62, all three amount-effect positions and both selected-character slots. Compare full normalized proposals, preserved siblings and metadata, stable live identity, and exact encrypted response bodies/order. Held and completed-but-unacknowledged operations retain reservations and reject competing requests. |
| `TestCriticalInventoryRejectionsDoNotPersistOrMutate`; `TestCriticalInventoryReservedTradeCounterpartRetainsContext` | Invalid packets, unavailable participants, saturation, stopping, or a reserved reciprocal trade partner could mutate state before rejection. | Test domain/admission failures, hidden slots, quantities, account/character ownership, full inventory and closed/poisoned/shutdown paths. A reserved trade partner retains both trade contexts; eligible cancellation preserves close-trade ordering before committed inventory publication. |
| `TestCriticalInventorySplitAllowsNewIdenticalRequestAfterAcknowledgement` | Treating every identical packet as a duplicate would reject legitimate subsequent splits: the legacy request has no transaction identifier. | Two acknowledged identical valid splits each create one distinct UID and preserve the total quantity. Pending requests remain fenced; invalid stale requests after acknowledgement do not create another write. This is reservation coverage, not permanent request deduplication. |
| `TestCriticalInventoryUnknownOutcomeNeverPublishesOrOverwrites`; `TestCriticalInventoryDisconnectFinalSnapshotContainsCommit`; `TestCriticalInventoryShutdownWaitsForCommittedSnapshot` | Unknown commits or stale identity could trigger unsafe replay/old-state final saves; disconnect or shutdown could discard a successful change. | Error/panic/identity cases enter maintenance and retain ownership without confirmation or overwrite. Disconnect independently holds the operation and final atomic account/sidecar save; snapshots contain committed items and closed transports receive nothing. Repeated shutdown callers wait for accepted work and reuse the terminal result without another save. |
| `TestCriticalInventoryActualDispatcherKeepsWorldResponsive` | Helper-only tests would not prove that a held inventory save permits World scheduling. | Dispatch real inventory intentions through the World command channel. Unrelated overview commands, regeneration and three ticks advance while persistence is held. Completion publishes once. Autosave is deliberately postponed in this fixture and is not certified. |

Validation from `wydgo748/`:

- The initial new tests exposed two fixture mistakes: an untyped opcode local
  and a one-character fixture indexed as a four-character account. Both were
  corrected in the test; neither was a product defect.
- `go test -count=1 -timeout=90s ./internal/game -run
  'TestCriticalInventory|TestDeleteItem|TestSplitItem'` — pass, 0.075 s.
- `go test -count=20 -timeout=90s ./internal/game -run
  'TestCriticalInventory|TestDeleteItem|TestSplitItem|TestCriticalSkill|TestCriticalShop'`
  — pass, 1.928 s.
- Full suite: remove `WYD_TEST_POSTGRES_URL` from the child test process, then
  `go test -count=1 -timeout=180s ./...` — pass; game 4.301 s,
  store 0.495 s, webadmin 7.295 s. Fixture-dependent PostgreSQL tests skip.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` — pass.
  `git check-ignore wydgo748/bin/tm-check.exe` confirms the output is ignored.

The inherited PostgreSQL test connection names a local `wydgo_test` database,
but its isolation has not been verified. No connection credentials were printed
and this follow-up deliberately excluded that connection from the full suite.
The earlier suite run with the inherited connection is not isolated-database
approval. `CGO_ENABLED=0` and no available GCC/Clang leave race validation pending;
no unsupported race run, toolchain installation or service change was attempted.

Inventory source changes are in `inventory_handlers.go` and the new
`critical_persistence_inventory.go`; this follow-up adds
`critical_persistence_inventory_test.go`. It also updates the existing
`DOCS/server/critical-persistence-plan.md` and this record. No removals, client
edits, deployment or game execution. Authored prose is English; this is not a
global language-migration assertion.

Repository gates passed: `Test-RepositoryLayout.ps1` without `-UpdateMap`
validated 175 documents and local links; `git diff --check` reported only
line-ending notices. Explicit affected-Go-path `gofmt -l` returned no files,
and the five affected paths passed a separate trailing-whitespace check that
includes untracked files. Scoped Portuguese-candidate scanning found no matches
in the inventory source/tests and persistence plan; affected prose was inspected.

**Next executable work:** migrate multi-account trade and its shared consumers,
including capsule ownership, atomic account proposals and reservations across
both participants. Then finish other shared economy/guild/instance operations,
store outcome certification and reconciliation, normal-loop autosave/direct I/O,
production initialization and isolated PostgreSQL/race gates. The staged tests
are `AUTOMATED TESTED`; they do not close S2 or approve runtime/release readiness.

### Follow-up: staged trade, ghost-shop and periodic autosave — 2026-10-04

**S2 remains open. Production `NewWorld` still leaves the coordinator disabled.**
Classification: `MODERNIZACAO_COMPATIVEL`. This batch changes internal
persistence scheduling, not legacy packet layouts, gameplay rules or native
parity claims. The existing server persistence plan records the contracts and
remaining gates; no new native research was needed for these internal changes.

| Location | Scenario and impact | Correction and evidence |
| --- | --- | --- |
| `wydgo748/internal/game/trade.go`, `commitTrade`; `critical_persistence_trade.go`, `queueTrade` | A held two-player save must not expose proposed items/gold, lose capsule ownership or let cancellation invalidate the trade. | Prepare full detached accounts, transfer capsules and remap account-local seal IDs privately, then submit one atomic `SaveAccounts`. Bind both reciprocal trade contexts through acknowledgement. Apply both economic results before the existing Carry/close packets. Missing capability, invalid identity and queue saturation reject without live mutation or inline fallback. |
| `critical_persistence_trade_test.go` | Helper-only success would miss shared-context consumers, stale identity and terminal snapshots. | Cover selected slots, two-way gold/items/seals, collisions, nested accounts, pending/completed-unacknowledged requests and cancellations, third participants, rejections, unknown error/panic/identity, fresh acknowledged trades, disconnect final snapshots, repeated shutdown and actual command/tick progress. These actor fixtures postpone autosave and do not certify all combined schedules. |
| `ghost_shop.go`, `buyGhostShop` / `closeGhostShop`; `critical_persistence_ghost_shop.go`, `queueGhostShopPurchase` | Competing buyers or owner closure could change a listing while buyer/seller persistence is outstanding; premature stock-out cleanup could publish an uncommitted sale. | Submit detached buyer/seller accounts in one atomic batch. Retain exact shop registration, listing values and browsing context; account reservations protect participants and owner closure. Install both account results and remove the listing before publication. Stock-out cleanup runs after acknowledgement without yielding World ownership. |
| `critical_persistence_ghost_shop_test.go` | A single purchase test would not prove subsequent valid purchases, reciprocal trade fences, disconnects or held real dispatch. | Cover selected slots, stocked/stock-out cases, full proposals, observers and competing buyers, owner toggle/close/logout, 25 domain/admission rejection cases, unknown outcomes, disconnect combinations, shutdown and actual dispatch. Additional tests prove a fresh second purchase after acknowledgement and buyer/seller direct or reciprocal-trade reservations. Exact packet bodies/order and unchanged private state are checked. |
| `world.go`, `autoSaveAccounts`; `critical_persistence_autosave.go`, `queueAccountAutoSave` | Separate asynchronous account/sidecar queues or synchronous fallback can bypass the coordinator's ownership and split an aggregate. | Under the injected owner, submit the account and selected sidecar atomically through the bounded coordinator. Sidecar-only adapters reject before either write; account-only adapters retain coordinator ownership. Busy/full admission skips to the next eligible slice without replay or inline fallback. Completion acknowledges durability only: it never reinstalls the saved account or publishes economic success. Shutdown suppresses new periodic submissions. |
| `critical_persistence_autosave_test.go`; `critical_persistence_fences_test.go`, `TestCriticalFencesRejectPersistenceReadsAndAutosavesBeforeStoreAccess` | Old snapshots could alias live nested data, overwrite later state, release ownership before delivery or permit stale terminal saves after an uncertain outcome. | Test all four selected slots, nested capsules, affects/currencies, normalized persistence positions, held/completed-unacknowledged repetition, direct reservation rejection, fresh subsequent snapshots, queue saturation, synchronous/asynchronous sidecar-only rejection, account-only adapters, seven identity/stopping rejection cases, disconnect/readmission, cached repeated shutdown and unknown error/panic/identity containment. Update the fence fixture to an authenticated/indexed account and expect an atomic coordinator save, not two legacy asynchronous writes. |
| `TestCriticalAutosaveActualDueTickKeepsUnrelatedWorldProgress` | Postponed autosave fixtures do not prove that the actual periodic World path is nonblocking. | Make autosave due in the real actor loop. Hold its store write while unrelated regeneration, overview commands and three ticks advance. Repeated Cargo requests for the reserved account are ignored. Successful acknowledgement admits a fresh economic intention; unknown completion enters maintenance. A separate transfer gate keeps readiness polling from committing multiple fresh deposits in the test itself. |

Initial test failures were investigated rather than treated as product defects:
the ghost-shop insufficient-gold response is an indexed message, not a panel;
shutdown alone does not close transports (explicit disconnect commands do).
The new autosave fixture had a missing brace and initially allowed readiness
polling to commit multiple legitimate fresh deposits. The fixture now holds the
post-autosave transfer independently. These corrections preserve actual
protocol and lifecycle expectations, not weakened success assertions.

Validation from `wydgo748/`:

- Ghost-shop focused suite: `go test -count=1 -timeout=90s ./internal/game
  -run '^TestCriticalGhostShop'` — pass, 0.053 s.
- Autosave/consumer focused suite: `go test -count=1 -timeout=90s ./internal/game
  -run 'TestCriticalAutosave|TestCriticalFencesRejectPersistenceReadsAndAutosavesBeforeStoreAccess|TestAutoSave'`
  — pass, 0.048 s.
- Integrated repetition: `go test -count=20 -timeout=180s ./internal/game
  -run 'TestCritical|TestAutoSave|TestGhostShop|TestTrade|TestCargo|TestHellStore'`
  — pass, 23.983 s.
- Remove `WYD_TEST_POSTGRES_URL` from the child process, then
  `go test -count=1 -timeout=180s ./...` — pass; game 4.231 s,
  store 0.419 s, webadmin 7.241 s. Database-dependent tests skip; this is not
  isolated-PostgreSQL transaction approval.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` — pass.
  The build stays in the existing ignored output directory; no server or client
  was launched, installed or deployed.

Current modified/new implementation and test paths are `trade.go`,
`critical_persistence_trade.go`, `critical_persistence_trade_test.go`,
`ghost_shop.go`, `critical_persistence_ghost_shop.go`,
`critical_persistence_ghost_shop_test.go`, `world.go`,
`critical_persistence_autosave.go`, `critical_persistence_autosave_test.go` and
`critical_persistence_fences_test.go`. Durable records are this audit and
`DOCS/server/critical-persistence-plan.md`. No removals or unrelated edits were
made in this batch. The worktree contains other preexisting changes.

Remaining S2 requirements include other critical producers (crafting, rewards,
ground/shared objects and administrative paths), shared consumer/lifecycle
coverage, guild/combined transaction forms, certified store rejection versus
unknown outcome and reconciliation, production initialization, isolated
PostgreSQL and supported race-toolchain gates. Global maintenance remains
conservative containment, not reconciliation. Periodic autosave is migrated
only under the injected owner; the default path remains legacy. Automated
success does not establish global S2 completion, client execution or release
readiness. The next implementation batch should close another remaining
economic producer and its shared consumers before constructor activation.

### Follow-up: staged ground drop and Tiny composition — 2026-10-04

**S2 remains open and disabled in production `NewWorld`.** Classification:
`MODERNIZACAO_COMPATIVEL`; existing gameplay rules and wire layouts are retained.

- Ground drop: `critical_persistence_ground.go`, ground handlers and registry,
  visibility, expiry, spatial/cannon consumers retain an unpublished object and
  its ID while persisting a detached source removal. Held and completed-but-
  unacknowledged operations cannot expose the object or accept guessed-ID pickup.
  Completion validates the original source/object, removes the source and only
  then publishes. The original expiry deadline is retained; unrelated expiry
  continues. Unknown outcomes retain the source and hidden-object reservations.
- Tiny: `crafting.go:onCombineTiny` and
  `critical_persistence_crafting.go:queueTinyCraft` preserve the recipe, cost,
  single roll, item identity and text-before-result order. Ingredients/gold and
  any reciprocal trade remain unchanged until successful actor completion.
  Both trade participants are reserved through one atomic account transaction;
  rejected admission never falls back to inline persistence.
- Fixed a confirmed snapshot-comparison defect in Tiny completion: comparing a
  raw peer account with its normalized snapshot could reject an unchanged nil
  capsule collection. Compare both through `accountStateSnapshot` instead.
  The reciprocal-trade test deliberately preserves the nil live collection.
- `critical_persistence_crafting_test.go` covers both outcomes, all four slots,
  extra ingredients, exact proposals/packets, repeated requests, busy peers,
  malformed/stale inputs, full/stopping/unsupported queues, unknown error/panic
  and identity drift. It also exercises disconnect combinations, final
  account/sidecar snapshots, repeated shutdown and actual dispatcher/tick
  progress under held writes. These fixtures postpone autosave; they do not
  validate all combined schedules or an isolated database.

Validation from `wydgo748/`:

- `go test -count=1 ./internal/game -run
  'TestCriticalTiny|TestCombineTiny|TestTiny|TestCombineResult|TestCombineNPC'`
  — pass, 0.067 s.
- `go test -count=50 -timeout=90s ./internal/game -run
  'TestCriticalTiny|TestCriticalGroundDrop|TestCriticalGroundPickup'`
  — pass, 4.466 s. Earlier ground-drop/cannon 100 repetitions and integrated
  ground/Cargo/trade/ghost-shop 20 repetitions remain valid for unchanged inputs.
- Child process excludes `WYD_TEST_POSTGRES_URL`, then
  `go test -count=1 -timeout=180s ./...` — pass; game 4.298 s,
  store 0.441 s, webadmin 7.248 s. Fixture-dependent database tests skip.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` — pass.
  No executable was launched or installed; no service or operational data changed.

The existing persistence plan and this audit are updated, not duplicated.
No removals or client changes. These are automated results, not client/runtime
approval. Remaining work includes other crafting/economy producers and shared
consumers, guild/combined transactions, store outcome certification and
reconciliation, production initialization, isolated PostgreSQL and supported
race-toolchain validation. Next: migrate another composition producer and its
trade/lifecycle consumers before enabling the constructor.

### Follow-up: retained-trade compositions and Lindy — 2026-10-04

**S2 remains open; the coordinator is still disabled in production `NewWorld`.**
Classification: `MODERNIZACAO_COMPATIVEL`. This batch does not change recipes,
wire layouts or native evidence requirements.

- Compositor/Agatha/Aylin use detached inventory/gold proposals while reserving
  reciprocal trade participants. The accepted composition retains the trade.
  A confirmed final-disconnect cleanup defect was fixed: its own saved intent
  must not suppress trade cancellation, and a peer's accepted final write must
  finish before cleanup destroys either retained participant identity.
- Lindy now has a detached operation in `critical_persistence_lindy.go` and an
  admission branch in `crafting_simple.go`. Unlocking 355 saves consumed
  ingredients, cape and flag; unlocking 370 saves ingredients/flag and one Fame
  debit together. Missing atomic capability rejects without inline fallback.
  Live state and existing response ordering are published only after successful
  World validation and acknowledgement. Reciprocal trades remain open.
- The optional `playerStateBatchStore` interface and PostgreSQL
  `SavePlayerStateBatch` cover the 370 reciprocal-trade case with one account/
  sidecar transaction across both reserved accounts. Existing account/state
  fencing, serializable transaction and durable item-identity checks are reused.
- Lindy tests cover both limits, all four character slots, retained trades,
  held/completed-unacknowledged repetition, invalid/admission rejection, stale
  state, unknown error/panic, actual dispatcher responsiveness, disconnect,
  owner/peer/simultaneous trade disconnect, last-Fame deletion and repeated
  shutdown. PostgreSQL tests cover pre-admission invalid ownership; the live
  atomic commit/ledger-conflict rollback/sidecar deletion test remains gated by
  an isolated database fixture.
- Two regression-test issues were corrected, not hidden: the fixture's NPC
  lookup used the wrong collection, and the expected success sequence omitted
  the existing "Processing complete." message. No production response was
  removed to accommodate the test.

Validation from `wydgo748/`:

- The preceding artisan/trade-disconnect focused gate passed 50 repetitions
  in 7.972 s. It is historical evidence for that unchanged batch, not database
  or race approval for the newly added adapter.
- `go test -count=1 -timeout=90s ./internal/game ./internal/store -run
  'TestCriticalLindy|TestPostgresPlayerStateBatch'` passed: game 0.055 s,
  store 0.018 s. After adding trade-disconnect/shutdown coverage,
  `go test -count=1 -timeout=90s ./internal/game -run 'TestCriticalLindy'`
  passed in 0.048 s.
- With `WYD_TEST_POSTGRES_URL` removed from the child process,
  `go test -count=1 -timeout=180s ./...` passed: game 4.494 s,
  store 0.493 s, webadmin 7.210 s. Fixture-dependent database tests were not
  exercised. The same suite was not restarted when its observation returned a
  running session; the original process was polled to completion.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed.
  The ignored executable was not launched or installed. No services, client
  inputs or operational data were changed; no files were removed.
- `tools/repository/Test-RepositoryLayout.ps1` passed without `-UpdateMap`,
  validating local links and the 175-document inventory. `git diff --check`
  passed with only Git line-ending warnings.

Durable records are this audit and `DOCS/server/critical-persistence-plan.md`.
Implementation additions are `critical_persistence_simple_crafting.go`,
`critical_persistence_lindy.go` and `postgres_player_state_batch.go`, with their
focused tests. Existing integration points include `crafting_simple.go`,
`charstate.go`, `critical_persistence.go`, `trade.go` and
`critical_persistence_lifecycle.go`. Other dirty-tree edits are preserved.

Remaining S2 closure work is unchanged in scope: migrate other critical
economy producers and shared consumers, add necessary guild/combined atomic
forms, certify definite store rejection versus unknown commit and recovery,
remove normal-loop synchronous fallbacks, enable production initialization
only after safe migration, and run isolated-PostgreSQL/supported race gates.
Global maintenance remains containment, not reconciliation. This completed
staged batch does not mean S2, runtime validation or release readiness is complete.

### Follow-up: extraction and alchemy staged migration — 2026-10-04

Classification: `MODERNIZACAO_COMPATIVEL`. This batch changes internal
persistence scheduling under the injected coordinator, not wire layouts.

- `crafting_skills.go` now prepares extraction and alchemy proposals without
  changing live inventory or canceling trades before durable completion.
  A reservation is checked before cooldown or RNG consumption. Existing
  Portuguese comments in this affected file were translated to English;
  established skill identifiers remain unchanged.
- `critical_persistence_crafting.go` validates recipe identity, result,
  probability, gold and retained skill inputs, reserves reciprocal participants,
  and saves detached account snapshots atomically. Missing capability rejects
  admission without inline fallback. World validates retained state before
  installation, acknowledgement and publication.
- Extraction retains the strict `roll < chance` boundary, target UID/lifecycle,
  damage conversion, whole catalyst consumption and item-only responses.
  Alchemy retains all ten recipes, optional ingredient consumption,
  secondary-skill stack size, first ingredient UID, `roll <= chance`, and the
  existing item/UpdateEtc/text/result order. Both cancel reciprocal trades only
  after acknowledgement, as required by their existing successful flow.
- New `critical_persistence_skill_crafting_test.go` exercises actual handler
  dispatch, held and completed-but-unacknowledged repetition, all four character
  slots, probability boundaries, reciprocal trades, all alchemy recipes,
  admission rejection, stale state, unknown error/panic containment,
  unrelated World tick progress and owner/peer/simultaneous disconnects.
  Final saves preserve committed state; closed connections receive no packets.
- Initial test failures identified a duplicate helper name and a fixture that
  changed base mastery while retaining a different runtime mastery. Both were
  corrected in the tests without weakening assertions or production behavior.

Validation from `wydgo748/`:

- Existing extraction/alchemy and staged artisan focused tests passed in
  0.185 s before the new tests were added. The expanded
  `go test -count=1 -timeout=90s ./internal/game -run 'TestCriticalSkillCraft'`
  passed in 0.086 s. The final admission guard was covered by the subsequent
  full suite rather than restarting an unchanged focused gate.
- With `WYD_TEST_POSTGRES_URL` removed from the child process,
  `go test -count=1 -timeout=180s ./...` passed: game 4.631 s, store 0.432 s,
  webadmin 7.289 s. Fixture-dependent database tests were not exercised.
  The original running test process was polled to completion, not restarted.
- `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed.
  No executable was run or installed; no service, client input or operational
  data was changed, and no files were removed.

- Repository layout/local links passed without `-UpdateMap` (175 documents
  indexed), and `git diff --check` passed with line-ending warnings only.

S2 remains open. Production `NewWorld` still has no coordinator enabled, and
its synchronous fallback remains. Other critical producers, shared consumers,
required combined atomic forms, definite-versus-unknown commit certification,
reconciliation, isolated PostgreSQL and supported-CGO race gates remain.
Global maintenance is containment, not recovery. This record does not claim
production S2, client runtime validation or release approval.

### Follow-up: bounded validation and correction of repetitive reporting — 2026-10-05

The user reported a loop of incremental migrations without closing S2. This
follow-up records the already-started advanced crafting/native consumable batch
and stops expansion. Classification: `MODERNIZACAO_COMPATIVEL`; production
`NewWorld` still does not enable the coordinator. S2 is not complete.

- Ehre/Odin detached proposals and their focused tests passed, including
  owner/peer trade disconnect and shutdown. Native equipped refinement,
  Nightmare tickets and mastery resets now have tests for detached snapshots,
  held-write repetition, reciprocal trade reservations, post-ack publication,
  rejection, unknown/stale outcomes, owner disconnect, shutdown and World tick
  progress. All 32 native detached-commit cases passed; native trade-peer
  disconnect is not covered by these tests.
- Corrected the earlier validation claim: a consumable test command that
  reported `[no tests to run]` proved compilation only. New tests required
  fixture corrections for the 20-byte shortcut layout, initialized catalog,
  per-case rules and invalid shortcut filtering. Assertions were not weakened.
- Full integration `go test -count=1 -timeout=180s ./...` failed in game. A JSON
  diagnostic rerun isolated
  `TestLaktoreriumPowderConsumesOnlyCommittedValidAttempts/item_413_last_unit_disappears_after_successful_commit`
  at `consumables_handler_test.go:235`: the test expects the old Portuguese log
  substring, while the translated log emits `consumed=true remaining=0`.
  Item, refinement, UID and save assertions passed before the log assertion.
  The stale expectation remains pending; the batch is not integration-approved.
- Other packages passed, including store (0.427 s) and webadmin (7.399 s).
  `go vet ./...` and `go build -o bin/tm-check.exe ./cmd/server` passed.
  Test processes had `WYD_TEST_POSTGRES_URL` removed. Isolated database and
  supported-CGO race gates remain pending; no executable was run or installed.

The existing critical-persistence plan records remaining producers, shared
lifecycle/atomic forms, commit-outcome certification and reconciliation,
production activation and database/race gates. Passing staged tests does not
close any uncovered requirement or establish runtime approval. No new migration
is started as part of this reporting correction; no files are removed.
