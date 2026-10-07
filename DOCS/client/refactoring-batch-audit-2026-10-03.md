# Claude refactoring batch audit — 2026-10-03

## Decision and audited snapshot

The static and automated audit is complete for the reviewed snapshot, with
explicit limitations recorded below. The batch is **not accepted**: one P1
persistence defect, a client weather regression, and several acceptance-tool
defects remain. No P0 was identified. Completion of this audit is not runtime
approval, native-parity certification, or release readiness.

| Property | Audited value |
| --- | --- |
| Repository | `C:/Users/xereca/Documents/PROJETOS/wyd-go` |
| Branch | `main` |
| HEAD | `34b7dcae4068f1b6bcfc152e4037bd9e7ccf0dfa` |
| Committed batch | `71c4b313` and `34b7dcae` |
| Mechanical comparison reference | `77e8dcc7`, where applicable |
| Additional scope | Tracked working-tree changes, untracked files, deletions, and affected consumers |
| Audit mode | Review without fixes, approval updates, commits, deployment, or game execution |
| Runtime status | Not `CLIENT_TESTED` |

This document preserves the results of the preceding audit. Saving it did not
repeat the product builds, Go suite, native research, or extraction tests. Line
numbers refer to the audited working tree and can drift after corrections.
Source links below use the audit workstation's absolute paths.

### Execution exception: PostgreSQL writes

The full Go suite used `WYD_TEST_POSTGRES_URL`, targeting the local loopback
database `wydgo_test`. Some tests create accounts, persist characters, and
replace then restore the singleton guild state. Consequently, the executed
audit was **not strictly read-only**, despite its intended mode.

The constructor used by these tests can also apply schema changes when needed.
Cleanup attempts are not proof of restoration: some cleanup SQL errors are
ignored, and no pre-run database snapshot was available for independent
comparison. This is not evidence that an operational database was damaged;
the observed target was the test database. No further database tests were run
after identifying the side effects.

Relevant locations are `wydgo748/internal/store/postgres_test.go:259-263`,
`:318-322`, and `:451-500`,
`wydgo748/internal/store/postgres_fence_test.go:66-71`, and
`wydgo748/internal/store/postgres.go:206-220`.
Future validation must use an explicitly disposable, isolated database.

## Method and evidence boundaries

The audit compared extraction bodies, manifests, project registration, and
consumers; followed packet dispatch through authoritative validation,
persistence, and client responses; and reviewed lifecycle, rejection,
rollback, repeated requests, and relevant shared-state paths.

The three architecture plans, UI/opcode/resource/effects matrices, data
consistency records, Hell Store documentation, protocol catalog, and document
inventory were confronted with the current code. Existing native evidence was
reused. No concrete unresolved contract change justified a fresh broad Ghidra
survey.

Evidence is interpreted conservatively:

- A normalized body match establishes only the comparison performed by that
  normalizer, not behavioral equivalence.
- Reviewed digests establish that a particular body was approved, not that its
  behavior was exercised.
- Source-string assertions do not exercise an interleaved packet/lifecycle
  scenario.
- Successful compilation does not validate gameplay, graphics, sound, or
  native 7.48 parity.
- A skipped test does not cover its path.
- Importing historical KR resources does not independently prove native 7.48
  compatibility.

Severity uses P0 for critical immediate failures, P1 for high-impact correctness
failures, P2 for material defects or acceptance gaps, and P3 for lower-impact
documentation/compliance defects. Origin labels distinguish new local changes
from inherited or preexisting behavior; they are not blame assignments.

## Finding index

| ID | Priority | Classification | Finding |
| --- | --- | --- | --- |
| A01 | P1 | Confirmed code defect | Kefra defeat and fame are not recoverable after a failed persistence commit |
| A02 | P2 | Confirmed code defect | Aura expiry overwrites newer server weather |
| A03 | P2 | Confirmed tool defect | Opcode size comparison silently excludes new handler constants |
| A04 | P2 | Confirmed tool defect | Resource/effects matrix generation fails on locally deleted files |
| A05 | P2 | Confirmed acceptance failure | Extraction and relocation gates do not accept the current tree |
| A06 | P2 | Confirmed evidence defect | Unchanged-body counts are overstated or hard-coded |
| A07 | P2 | Confirmed tool defect | Literal conversion does not roll back a compile exception in write mode |
| A08 | P2 | Confirmed asset-reference defect | Five audio entries resolve to missing files |
| A09 | P3 | Confirmed documentation defect | Resource documentation contradicts imported audio |
| A10 | P3 | Confirmed language-compliance defect | Affected files retain or introduce non-English text |

## Confirmed findings and correction proposals

### A01 — P1: Kefra defeat is not durable after persistence failure

**Location:** [kefra.go:138](C:/Users/xereca/Documents/PROJETOS/wyd-go/wydgo748/internal/game/kefra.go:138),
with the repeat guard at line 119 and the failure test in `kefra_test.go:83-96`.

**Trigger:** Kefra's kill has already completed, but `warCommit` fails while
persisting the defeat marker and the winning guild's fame.

**Observed logic:** The failed commit rolls back its guild-state changes. The
fallback then writes `Defeated=true` only to memory, with no winning guild,
and withholds fame. There is no durable pending event or retry. Subsequent
calls return while the in-memory defeat marker is set.

**Impact:** Restart can reload the old undefeated state, reactivating Kefra,
guards, and the portal restriction before the intended weekly reset. If some
later guild-state save persists the fallback, it still lacks the winning guild
and does not restore the withheld fame. The player-visible announcement and
guard removal proceed despite failed persistence.

**Evidence:** The existing failure test explicitly accepts an in-memory defeat
with unchanged fame. It does not restart from the persisted state or test
recovery. The loss of durable state follows directly from the failure branch;
a production failure/restart was not executed.

**Origin:** New local Kefra implementation.

**Suggested correction:**

1. Give each defeat a durable identity and preserve the winning guild and
   timestamp in a recoverable record.
2. Use a transaction spanning the required authoritative state, or a durable
   event/outbox with idempotent application. A purely in-memory retry is
   insufficient for restart recovery.
3. Make defeat, portal/guard state, and fame recovery consistent. Do not let
   the in-memory early-return guard discard pending persistence work.
4. Document what players observe while durable recovery is pending.

**Acceptance tests:** Successful defeat; injected save failure followed by
restart; recovery after storage becomes available; repeated recovery and death
notifications; exactly one fame grant; guild-less defeat; reset before/after
recovery. Use a disposable store/database and verify both loaded state and
responses, not only in-memory values.

### A02 — P2: Aura expiry restores obsolete weather

**Locations:** [TMHumanEffects.cpp:2664](C:/Users/xereca/Documents/PROJETOS/wyd-go/tmproject/TMProject748/internal/game/entities/TMHumanEffects.cpp:2664),
[TMFieldSceneWorld.cpp:346](C:/Users/xereca/Documents/PROJETOS/wyd-go/tmproject/TMProject748/internal/app/scenes/TMFieldSceneWorld.cpp:346),
and [TMFieldScene.cpp:4195](C:/Users/xereca/Documents/PROJETOS/wyd-go/tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp:4195).

**Trigger:** Enter the guard aura under weather A, receive server weather B
while the aura remains active, and then let the aura expire.

**Impact:** Expiry restores A instead of the last server state B. The temporary
visual effect therefore overwrites newer authoritative weather.

**Evidence:** Aura activation snapshots `g_nWeather` only once. `SetWeather`
updates that global but not the saved snapshot. Expiry restores the snapshot.
The current tests cover the policy helpers and source text, not this event
sequence. This is a static deterministic finding, not a visual reproduction.

**Origin:** New local aura lifecycle change.

**Suggested correction:** Keep the latest server weather separate from the
effective rendered weather. Derive the visual override from aura state instead
of restoring a stale global snapshot. At minimum, ensure incoming weather
updates the authoritative value used when the override ends.

**Acceptance tests:** A → aura → B → expiry must yield B; multiple packets
during the aura; overlapping guards; aura expiry without a packet; scene
replacement/disconnect; timestamp boundary cases. Exercise the sequence through
the weather and aura state transitions, rather than checking strings.

### A03 — P2: Opcode matrix drops unknown server sizes

**Locations:** [opcode-matrix.py:40](C:/Users/xereca/Documents/PROJETOS/wyd-go/.agents/research/opcode-matrix.py:40),
constant collection at lines 287-297, and size filtering at lines 375-380.

**Trigger:** Packet-size constants reside in new, untracked Go handler files.
The inventory uses `git ls-files`, so these files are not scanned.

**Evidence:** Hell Store `0x2BD` (24 bytes), lottery `0x3B8` (32 bytes), and
store-gold `0x3D1` (16 bytes) have unknown server sizes in the generated
comparison. Dispatcher recognition still identifies them as handled; the
defect is missing size evidence, not missing dispatch. Unknown values are
discarded by the truthy-value filter.

**Impact:** A zero-mismatch result can mean that required sizes were never
compared. It cannot establish complete wire compatibility.

**Origin:** New matrix generator.

**Suggested correction:** Include tracked and untracked non-ignored files
that actually exist. Represent unknown sizes explicitly and make complete
contract validation fail or report incomplete coverage when they are required.
Do not infer a size merely to remove an unknown marker.

**Acceptance tests:** Fixtures with an untracked handler, a deleted tracked
handler, an unknown size, a real mismatch, and matching client/server sizes.
Regenerate the matrix only after these cases pass.

### A04 — P2: Resource and effects generators cannot read the current tree

**Locations:** [resource-matrix.py:74](C:/Users/xereca/Documents/PROJETOS/wyd-go/.agents/research/resource-matrix.py:74)
and [effects-matrix.py:119](C:/Users/xereca/Documents/PROJETOS/wyd-go/.agents/research/effects-matrix.py:119).

**Trigger:** `Mission.cpp` and `Mission.h` are locally deleted but remain in
the Git index inventory. Both generators read indexed paths unconditionally.

**Evidence:** Calling resource `build()` and effects `class_headers(...)`
raises `FileNotFoundError` at those reads. No output-rewriting mode was used
for the diagnostic calls.

**Impact:** Published matrices cannot be reproduced from the audited tree;
new files are also not represented by a tracked-only inventory.

**Origin:** New tools interacting with local deletions.

**Suggested correction:** Share a working-tree inventory that includes new
files and checks existence. Distinguish intentional deletions from unexpected
read errors. Test missing/untracked inputs before regenerating documentation.

**Acceptance tests:** Generation succeeds for intentional deletions and new
inputs, reports genuine read failures, and includes the expected resources
and classes. Validate the regenerated results against known fixtures.

### A05 — P2: Current extraction acceptance gates fail

**Locations:** [tmhuman-separation.py:221](C:/Users/xereca/Documents/PROJETOS/wyd-go/.agents/research/tmhuman-separation.py:221)
and `.agents/research/relocations/`.

**Evidence:** TMHuman with `--complete --with-policies` raises
`Core static data or namespace-scope code changed`. The relevant difference is
the removed free `_locationCheck(float, float, int, int)` function; the vector
overload remains. Its removal is mentioned in the review material, but the
complete gate does not accept the corresponding namespace-scope change.

Of the eleven general relocation manifests, only `scene-selectchar.json`
passes. The other ten fail on changed bodies, deleted methods, deleted units,
or stale expectations. Examples include removed webboard code in `app-newapp`,
deleted helpers in `core-basedef`, deleted diagnostics in `render-device`, and
stale scene/grid/control expectations.

**Impact:** Compilation passes, but the declared relocation acceptance gate
does not. These failures do not by themselves prove the deletions unsafe;
they mean those changes lack current acceptance by the declared checks.

**Origin:** Local changes after the accepted relocation inputs.

**Suggested correction:** Review each failure against calls, virtual dispatch,
resources, ABI, and lifecycle. Add narrowly specified allowances for reviewed
namespace changes, and update individual manifests only after that review.
Never bulk-refresh approved hashes to obtain a green result.

**Acceptance tests:** All relevant complete gates and manifests pass; a
deliberately altered body or unexpected namespace deletion still fails;
Release/Debug registration and direct source-test ownership remain checked.
Run each manifest independently and preserve its exit status—an aggregate
shell command can hide an earlier failure.

### A06 — P2: Equivalence counts are not computed correctly

**Locations:** [tmfieldscene-separation.py:348](C:/Users/xereca/Documents/PROJETOS/wyd-go/.agents/research/tmfieldscene-separation.py:348),
[tmhuman-separation.py:241](C:/Users/xereca/Documents/PROJETOS/wyd-go/.agents/research/tmhuman-separation.py:241),
and `DOCS/client/tmhuman-separation-plan.md:14`.

**Evidence:** FieldScene prints an unchanged count by subtracting only the
small `REVIEWED_METHODS` set. It does not account for the larger review and
extraction sets. Human prints a fixed statement of 127 unchanged bodies.
Comparison with `77e8dcc7`, using the tools' normalization, produced:

| Class | Baseline | Current | Identical | Changed | Added | Removed |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| TMFieldScene | 293 | 289 | 193 | 66 | 30 | 34 |
| TMHuman | 128 | 131 | 109 | 10 | 12 | 9 |

FieldScene announces 284 unchanged bodies; Human's message says 127.
Normalization ignores some textual differences, including comments. These
are normalized-body counts, not raw-byte counts or gameplay assertions.

**Impact:** CLI and documentation overstate preservation evidence. Legitimate
extractions and reviewed changes must not be counted as identical bodies.

**Origin:** Batch tools and their documentary consumers.

**Suggested correction:** Derive all categories from baseline/current method
sets. Report reconstructed extractions, reviewed semantic changes, additions,
and removals separately. Generate documentary counts from the checked result
instead of copying hard-coded claims.

**Acceptance tests:** Small fixtures with one example of every category;
category totals reconcile with both sets; CLI-output tests fail if claimed
counts diverge; approval labels do not imply behavioral execution.

### A07 — P2: Literal conversion lacks rollback on compile exceptions

**Location:** [literal-escape.py:117](C:/Users/xereca/Documents/PROJETOS/wyd-go/.agents/research/literal-escape.py:117).

**Trigger:** Baseline compilation succeeds, `--write` writes candidate source,
and candidate compilation raises an exception.

**Evidence:** The `finally` restores originals only when `args.write` is
false. The later digest-mismatch rollback is unreachable after the exception.
A memory-only reproduction, with compiler and file operations mocked, returned
an exception with `sources_restored=False`. No real source was written by this
reproduction. The finding specifically concerns write mode; it does not claim
the dry-run or outer `--against` restoration follows the same unsafe path.

**Impact:** A failed verification can leave unverified source edits behind.

**Origin:** New literal-conversion tool.

**Suggested correction:** Use a success flag set only after every compilation
and digest comparison passes. Restore originals on all failure paths, retain
candidates only after verified success, and surface restoration failures.

**Acceptance tests:** Baseline failure without writes; candidate compile
failure after writes; digest mismatch; successful dry run; successful write;
restoration error. Assert exact original bytes after each rejected operation.

### A08 — P2: Audio list references missing assets

**Location:** [soundlist.txt:47](C:/Users/xereca/Documents/PROJETOS/wyd-go/tmproject/client748/sound/soundlist.txt:47),
also lines 78, 79, 86, and 331. Loader failure handling is in
`internal/platform/windows/dsutil.cpp:272-281`.

| ID | Missing runtime file |
| --- | --- |
| 55 | `sound/menu/menu04-B.wav` |
| 95 | `sound/effect/fireball.wav` |
| 96 | `sound/effect/firearrow.wav` |
| 103 | `sound/weather/weath03.wav` |
| 363 | `sound/monster/mguardatt.wav` |

**Impact:** A request for these IDs cannot load the declared audio; the loader
logs the failure and returns null. Actual gameplay requests for each ID were
not demonstrated, and no broader audio failure is asserted.

**Evidence:** The imported files match the KR collection byte-for-byte, but
that collection already contains the dangling references. Header checks and
hashes do not exercise DirectSound/DirectShow decoding or playback.

**Origin:** Inherited reference-set defect newly introduced into active assets.

**Suggested correction:** Locate compatible evidence-backed assets or correct
demonstrably wrong paths. Preserve numeric sound IDs; do not compact the list
or invent replacements merely to satisfy existence checks.

**Acceptance tests:** Resolve every list entry relative to the runtime root;
check unique IDs, file format, loader acceptance, and eventually the affected
audio flows when client execution is explicitly permitted. Playback remains
pending under the current prohibition on game execution.

### A09 — P3: Resource documentation describes obsolete absence

**Location:** [resource-compatibility-matrix.md:216](C:/Users/xereca/Documents/PROJETOS/wyd-go/DOCS/client/resource-compatibility-matrix.md:216),
with stale runtime-presence rows around lines 97-109 and 127.

**Evidence:** It states that the runtime has no sound/music directories. The
working tree contains 349 WAV files, 13 MP3 files, and `soundlist.txt`, totaling
363 imported files. Some rows still classify existing resources as absent.

**Impact:** The document misstates remaining work and can support incorrect
compatibility conclusions.

**Origin:** New documentation made stale by local asset imports.

**Suggested correction:** Repair A04, regenerate the inventory, and review the
narrative. Keep presence, format validation, loader validation, and gameplay
validation as separate fields; include A08's unresolved paths.

**Acceptance tests:** Documentary counts and existence statements match the
current tree, links pass, and no imported file is marked runtime-tested solely
because its hash matches a historical collection.

### A10 — P3: Affected files violate English-only delivery

**Locations:** [kefra.go:18](C:/Users/xereca/Documents/PROJETOS/wyd-go/wydgo748/internal/game/kefra.go:18),
`DOCS/server/operations.md:206-208`, `wydgo748/data/NPCGener.txt:113773`,
[account-api/main.go:23](C:/Users/xereca/Documents/PROJETOS/wyd-go/wydgo748/cmd/account-api/main.go:23),
and [guild_wars.go:104](C:/Users/xereca/Documents/PROJETOS/wyd-go/wydgo748/internal/game/guild_wars.go:104).

**Evidence:** New guard descriptions use untranslated terminology; the account
API help and guild-war text retain Portuguese. Korean comments also remain in
the affected client item-effect header after encoding conversion. This audit
record describes the debt in English rather than reproducing the strings.

**Impact:** The batch does not meet repository language requirements. Some
debt is documentation/help text; some is returned by gameplay code. This is
not evidence that the entire repository has been linguistically audited.

**Origin:** Mixed: newly authored text and preexisting text left in affected
files.

**Suggested correction:** Translate human-readable content in affected files
and inspect all authored comments, diagnostics, tests, examples, and UI text.
Preserve protocol bytes, established identifiers, and persisted values unless
their translation has a separate compatibility review. Track untouched debt
without treating the record as a release exception.

**Acceptance tests:** Review the full affected files, not just added lines;
verify contract-sensitive values remain stable; test returned/player-facing
messages. A global language-completion claim requires a separate active-tree
audit.

## Probable risks and unresolved hypotheses

These are not additional confirmed gameplay bugs:

- **Database-test isolation risk:** existing tests restore singleton state and
  ignore some cleanup failures. Concurrent use of the same database could
  interfere with other writers. No concurrent overwrite was demonstrated.
  Use a disposable database per run, checked cleanup, and explicit isolation.
- **Hell Store sell-back:** the `0x37A` flow remains an acknowledged integration
  gap. Handler and purchase tests do not certify the complete client/server
  sell-back path.
- **Repeated requests and reconnects:** no additional exploit was confirmed,
  but passing handler tests is not a substitute for the combined transport,
  persistence, disconnect/reconnect, and concurrency scenarios.
- **Data differences:** item and skill disagreements need targeted contract
  evidence before deciding which side is wrong. Historical source disagreement
  alone is not a safe correction instruction.
- **Audio execution:** file presence and header checks do not prove decoding,
  mixing, or playback. The five unresolved references are confirmed; actual
  gameplay use and wider playback behavior remain unverified.

An initially considered Hell Store inventory-footprint issue was not retained
as a confirmed bug: the relevant carry update renders one-cell entries, and
the reviewed server placement uses the corresponding empty-anchor behavior.
No new overlapping-inventory defect was established.

## Coverage by subsystem

| Subsystem | Reviewed coverage | Remaining limit |
| --- | --- | --- |
| C++ architecture | Mechanical extraction, early return/break/continue context, references, effect ordering, initialization, destruction, ownership, consumer references, and Release/Debug project entries | Build/link success does not establish native runtime ABI or visual behavior; A05/A06 block acceptance claims |
| UI and scenes | Control dispatch, scene lifecycle, chat, movement/automation, inventory, trade, merchant, mixing, combat, appearance/effects, and removed-code consumers | No visual/game execution; A02 needs an event-sequence test |
| Hell Store | 24-byte request, identity/session/world checks, NPC visibility/range, category/price/pool selection, balance/space, item materialization, persistence rollback, and response paths | Combined client flow, concurrency, and documented sell-back gap |
| Unavailable requests | Lottery, war-letter, and store-gold handlers and rejection/no-mutation behavior | Matrix size coverage incomplete under A03 |
| Kefra | Kill hooks, fame/state commit, guard generators, portal gating, announcements, and weekly reset | A01 recovery missing; no failure/restart acceptance test |
| Trade and no-trade | Effect mapping, catalog checks, and affected client/server consumers | No live trade session |
| Other committed Go changes | Bosses/spawn, networking, logging, account API, and affected tests | No additional confirmed defect; race validation unavailable |
| Persistence | Transaction/failure handling and relevant tests/constructors | Database mutation exception and lack of independent cleanup verification |
| Data | Item/effect/name/skill differences, NPC definitions, generator changes, and documentary claims | Unresolved differences are not native-parity failures by default |
| Assets | Imported audio identity, formats, paths, and loader references | A08; no playback, graphics, or resource-flow runtime validation |
| Tools | Extractors, reconstruction helpers, digests, policies, matrices, and failure behavior | A03-A07; approval alone is not behavioral evidence |
| Documentation | Three architecture plans, matrices, data/Hell Store records, opcode catalog, operations, and central inventory | A09/A10; historical visual acceptance is not current-candidate acceptance |

No additional confirmed gameplay regression was identified in the reviewed
committed boss, network, or account API changes. That statement is limited to
the audit's evidence and is not a blanket safety guarantee.

## Commands and recorded results

These are results from the preceding audit, not a statement that all commands
were rerun while writing this document. Commands must be inspected for side
effects before reuse. Do not use approval-writing flags such as `--review`,
`--pin`, or conversion `--write` to bypass a failed acceptance gate.

| Command or gate | Working directory | Recorded result |
| --- | --- | --- |
| `python .agents/research/tmfieldscene-separation.py --complete --with-policies` | Repository root | Pass, but reported equivalence count is incorrect (A06) |
| `python .agents/research/tmhuman-separation.py --complete --with-policies` | Repository root | Fail: namespace-scope change (A05) |
| `python .agents/research/source-relocation.py --manifest <manifest>` for the eleven general manifests | Repository root | One pass, ten failures (A05) |
| `python .agents/research/extract-method-tests.py` | Repository root | 21 tests passed |
| `python .agents/research/tmfieldscene-separation-tests.py` | Repository root | 22 tests passed |
| `python .agents/research/tmhuman-separation-tests.py` | Repository root | 11 tests passed |
| `python .agents/research/opcode-matrix-tests.py` | Repository root | Three tests passed; missing working-tree coverage remains |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File ./tmproject/Build-Client.ps1 -Configuration Release -NoDeploy` | Repository root | Passed, including architecture/socket gates |
| Same build command with `-Configuration Debug` | Repository root | Passed, including architecture/socket gates |
| ArchitectureTests | Build targets | 1,144,278 checks passed |
| SocketReceiveTests | Build targets | 221 checks passed |
| Release costume/item/render validation | Release build | 135 items, 129 renderers, 774 parts, four shaders passed |
| `go test -count=1 ./...` | `wydgo748/` | Passed; used PostgreSQL tests with writes |
| `go vet ./...` | `wydgo748/` | Passed |
| Focused Kefra tests | `wydgo748/` | Passed existing expectations, including the problematic failure fallback |
| `go test -race ./internal/game ./internal/store ./internal/net` | `wydgo748/` | Failed prerequisite: race requires CGO; `CGO_ENABLED=0` |
| `pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1` | Repository root | Passed, 171 documents at the audit snapshot; no `-UpdateMap` |
| `git -c core.safecrlf=false diff --check` | Repository root | Passed |
| Resource `build()` / effects `class_headers(...)`, invoked without output writes | Repository root | `FileNotFoundError` (A04) |
| Literal compile-failure reproduction with in-memory mocks | Isolated mock objects | Confirmed missing restoration in write mode (A07); no real file writes |

`TestProjectAccountsUseCurrentSchema` was skipped because required account
fixtures were missing. It provides no coverage for that path.

Build warnings included signedness/deprecation warnings and a legacy DirectX
header character warning; they were not build failures. No clean rebuild,
deployment, installed-client replacement, game launch, or service change was
performed. The installed `tmproject/client748/project.exe` was already absent
before the audit and was not restored.

The built Release candidate was `tmproject/build/TMProject748/Release/WYD.exe`
(SHA-256 prefix `8AF2E24FD74998080`); the Debug candidate was
`tmproject/build/TMProject748/Debug/WYDestiny.exe`
(prefix `3E0B417AF34EB`). These are identity prefixes, not full integrity
records. Historical visual acceptance referred to a different candidate
(prefix `D567`), not these builds or the current local changes.

## Suggested remediation sequence

This is a proposal, not authorization or a record of implemented fixes.

1. **Contain validation side effects.** Establish a disposable database and
   explicit test isolation before repeating persistence/integration gates.
2. **Fix A01.** Define durable defeat/reward recovery and prove failure/restart
   behavior before accepting Kefra lifecycle or economy correctness.
3. **Fix A02.** Separate authoritative weather from visual overrides and add
   sequence-based tests.
4. **Fix A07 before further source conversion.** Prove rollback on every
   failure path with safe fixtures.
5. **Repair evidence tooling (A03, A04, A06).** Use current-tree inventories,
   explicit unknowns, and truthful comparison counts.
6. **Resolve A05 individually.** Review each changed/deleted consumer boundary
   and update only justified manifests/allowances. Preserve negative tests.
7. **Resolve A08 and refresh A09.** Preserve sound IDs, validate references,
   and distinguish imported files from playback-tested resources.
8. **Resolve affected-file language debt (A10).** Do not alter contract-bearing
   identifiers or persisted values without compatibility review.
9. **Run focused gates per correction, then one integrated batch gate.** Race
   testing requires an available CGO/compiler environment. Missing fixtures
   and prohibited runtime tests must remain explicit pending gates.

## Closure criteria and delivery state

The audit reviewed the scoped change groups and their consumers; unresolved
runtime, data-parity, concurrency, and cleanup questions are explicitly
recorded as unverified. This closes the audit report, not the defects.

Acceptance of a corrected batch requires:

- A01's persistence/recovery and A02's event-sequence tests to pass.
- All applicable extraction/relocation gates to pass without blanket digest
  refreshes, with accurate counts and negative tests.
- Contract matrices to include new files and report unknown sizes honestly.
- Tool failure paths to preserve source and resource inventories to reproduce.
- Missing audio references and contradictory documentation to be resolved.
- Affected-file English compliance and relevant client/server gates to pass.
- Explicit records for skipped tests, race prerequisites, and runtime gates.

No source correction, approved-hash update, commit, or deployment was delivered
by the audit. Ignored build/test outputs were generated. The later request to
save this report authorizes documentation changes only: this report, its
documentation entry point, and its central inventory entry.

**Validation labels:** static inspection completed; automated gates partially
passed with the failures and exceptions above; **not `CLIENT_TESTED`**. Do not
promote historical visual acceptance to the current candidate.

## Remediation record (2026-10-03)

The findings above are kept as audited. This section records the corrections
made afterwards, in the proposed order. Validation labels stay separate:
nothing below is `CLIENT_TESTED`.

| ID | Correction | Evidence |
| --- | --- | --- |
| A01 | A Kefra kill now stages the defeat (winning guild, time) and the guild fame in memory and saves them in the same transaction as the kill rewards (`saveGuildState(accounts...)` -> `SaveGameState`). If the save fails, the guild state is undone and the kill rolls back, so Kefra stays alive with his HP restored and nothing is announced: memory matches what a restart loads. A death without a killer counts only once `warCommit` persists it. A guild at the fame limit keeps the defeat without fame. | `kefra_test.go`: commit failure then restart, recovery after storage returns, exactly one fame grant, repeated deaths, guild-less defeat, no-killer path, reset after recovery, fame limit. The fake store now keeps the last durable registry and reloads it. |
| A02 | The server weather and the BerialGuard override are kept apart (`beriel_aura::WeatherOverride`). `SetWeather` and the login packet record the server weather even during the aura; expiry reapplies `SetWeather` with the latest server value. | `FieldViewContractTests.cpp`: A -> aura -> B -> expiry yields B, several packets, overlapping guards, expiry without a packet, refresh at time zero, timer wrap. |
| A03 | `opcode-matrix.py` uses the working-tree inventory (`worktree.py`: tracked and untracked, existing files). A side that receives an opcode with an unknown size is reported as gap `U` ("size not compared") instead of being treated as a match. 0x2BD, 0x3B8 and 0x3D1 now carry 24, 32 and 16. | `worktree-tests.py` (untracked, deleted, ignored files; unknown size; mismatch; match). The matrix reports U=90: those receive sizes remain uncompared. |
| A04 | `resource-matrix.py`, `effects-matrix.py` and `ui-matrix.py` share the same inventory; a real read failure still raises. All three regenerate from the current tree. | `worktree-tests.py`; regenerated matrices. |
| A05 | `source-relocation.py` accepts reviewed removals (`after: null`), pin chains and declared `removed_files`; a missing current file is a failure, not an exception. Each of the ten failing manifests was reviewed per definition (linker-dead removals, UI-batch changes, deleted files) and pinned with that justification. The TMHuman gate accepts header and namespace-scope changes only as exact recorded before/after digests. | All eleven manifests pass when run one by one; `source-relocation-tests.py` (unreviewed change, removal of a present definition, edit after review, chain, missing and removed files); new TMHuman rejection tests (changed overload, edited reviewed body, unrecorded header line). |
| A06 | Both gates derive their counts from the baseline and current definitions (`method_counts.py`): identical normalized bodies, pinned extractions, reviewed changes, added and removed are reported apart, and a changed body without a pin or review fails. The hard-coded "127" and the "284 unchanged" message are gone; the plans were corrected. | TMFieldScene: 293 baseline / 289 current, 191 identical, 1 pinned, 67 reviewed, 30 added, 34 removed. TMHuman: 128 / 131, 109 identical, 5 pinned, 5 reviewed, 12 added, 9 removed. `test_method_counts_reconcile`. |
| A07 | `literal-escape.py` keeps candidates only after every compilation and digest comparison passed in write mode; every other path restores the originals, and a restoration failure is reported. | `literal-escape-tests.py`: baseline failure, candidate failure after writes, digest mismatch, dry run, write, restoration error. |
| A08 | Not corrected (no evidence): the five sound IDs keep their entries; the gap is recorded with the nearby files that do exist. | `resource-compatibility-matrix.md`, "Runtime audio". |
| A09 | The audio narrative now separates presence, format, loader and gameplay, lists the missing root lists and the five entries. | Regenerated resource matrix. |
| A10 | The first remediation pass translated Kefra wording, guild-war notices, account API help, and various comments and diagnostics, but did not finish the affected-file review. Follow-up verification found remaining Portuguese in `TMObjectContainer.h`, `catalog_test.go`, and `guild_txn_test.go`; those residuals are corrected in the follow-up below. Contract-bearing skill names and the CP949 byte list remain unchanged. | The first pass was partial; see the scoped follow-up verification. No global language-migration completion is claimed. |

Integrated validation after the corrections: Go `vet` and `test ./...` pass
with the PostgreSQL variables unset, so no database was written; the six
PostgreSQL store tests were skipped and remain a pending gate that needs a
disposable database. Release and Debug client builds pass with
ArchitectureTests 1,144,289 checks and SocketReceiveTests 221 checks; both
relocation gates, all eleven manifests and every research test suite pass.
Pending: the PostgreSQL gate, race testing (needs CGO), A08's five audio
entries, and `CLIENT_TESTED` for every client change.

After the remediation the 16 empty stubs were removed as well (see
`dead-code-map.md`), recorded as reviewed removals in both relocation gates
and in `app-newapp.json`. Current counts: TMFieldScene 293 baseline / 282
current, 181 identical normalized bodies, 1 pinned, 71 reviewed, 29 added, 40
removed; TMHuman 128 / 126, 100 identical, 5 pinned, 9 reviewed, 12 added, 14
removed. Release and Debug builds pass with ArchitectureTests 1,144,287 checks
and SocketReceiveTests 221; all eleven manifests and the gate test suites pass.

## Follow-up correction and verification (2026-10-03)

The user authorized corrections after independent verification found A08 open
and A10 only partially corrected. This follow-up preserves all other local
changes and does not update approved relocation digests.

### A10: Identified residuals corrected

- `tmproject/TMProject748/internal/render/world/objects/TMObjectContainer.h`:
  translated the ownership/lifecycle and method comments; removed the orphaned
  persistence comment left behind by the earlier deletion of `Save`. No class
  declaration, ABI, ownership operation, or executable statement changed.
- `wydgo748/internal/data/catalog_test.go`: translated the invalid-skill
  diagnostic and described skill 5 by its numeric ID in the regeneration
  diagnostic. The authoritative skill-name expectations remain unchanged.
- `wydgo748/internal/store/guild_txn_test.go`: translated test names, a local
  variable, temporary account/file fixtures, and invalid destination-kind
  fixtures. No external references to the renamed tests were found. Actual
  journal kind constants, serialization keys, and operational data are
  unchanged. The tests still exercise rejection, crash recovery, atomic account
  and guild persistence, and derived-export failure.

The complete three source/test files were reviewed, not just added lines.
Proper names in guild fixtures are retained. This closes the residuals
identified by the follow-up; it is not an active-repository language audit.
Untouched files still emit Portuguese during the package tests, including
PostgreSQL skip diagnostics and legacy UID-migration logs. That migration debt
remains outside this narrow correction and prevents a global language-ready
release claim.

### A08: Blocked on verified asset identity

All five declared paths still lack files. The available local reference roots
(`references/WYD_KR`, `references/client748`, `references/W2PP-master`) did not
provide the exact assets; the inspected `origin/main` tree did not provide them
either. An external exact-name search did not yield a verifiable source. This
does not prove that the original assets are unavailable everywhere.

`CSoundManager::Initialize` retains the numeric ID-to-path/channel mapping from
`sound/soundlist.txt`; the missing assets cannot be recovered by renumbering the
list. No paths, IDs, channels, or audio files were changed. In particular, the
nearby menu/weather files are not evidence of equivalent sounds. No silent WAV,
unrelated sound, or guessed alias was introduced to make an existence check pass.

To finish A08, supply the original five WAV files or a verifiable reference
package with their ID mappings. Validate origin, RIFF/WAVE format, supported
encoding, channels, and list resolution before importing them. If the originals
cannot be recovered, an explicitly approved replacement/fallback policy is a
separate behavior decision; it must preserve IDs and must not be described as
native parity. Playback remains pending until client execution is permitted.
See [the exact paths and IDs](resource-compatibility-matrix.md#runtime-audio).

### Follow-up gates

| Gate | Result |
| --- | --- |
| `go test -count=1 -v ./internal/data ./internal/store` from `wydgo748/` | PASS; both PostgreSQL environment variables were unset in the child shell. Five PostgreSQL tests and the absent project-account-fixture test were skipped. No configured operational database was used. |
| `go vet ./internal/data ./internal/store` | PASS |
| TMHuman/TMFieldScene `--complete --with-policies` | PASS; unchanged definition counts and no approved-digest writes |
| `Build-Client.ps1 -Configuration Release -NoDeploy` | PASS; incremental build, ArchitectureTests 1,144,287 checks and SocketReceiveTests 221 checks. Existing signed/unsigned comparison warnings remain. The installed executable was not deployed or replaced. |
| `Test-RepositoryLayout.ps1` without `-UpdateMap`; `git diff --check` | PASS; layout and local links checked, 172 documents indexed. Git reports line-ending normalization warnings, not whitespace errors. |

Debug and the full Go/tool integration gates passed during the preceding
verification. They are not rerun for comments and isolated test-text changes;
the focused package gates and incremental Release build cover this correction.
PostgreSQL integration, race testing, missing audio assets, and gameplay remain
pending. No deployment, client launch, service modification, or commit was made.

## Supplied KR audio follow-up (2026-10-03)

After the user supplied the KR music and sound folders, a scoped comparison
found all 363 source files already present in `tmproject/client748`: 13 MP3
files, 349 WAV files, and `sound/soundlist.txt`. SHA-256 comparisons found no
differences. Existing files were not replaced and reference files were not
moved or deleted.

The two missing root lists, `Music.txt` and `sound.txt`, were copied from
`references/WYD_KR` to `tmproject/client748`, preserving their exact bytes.
Their source paths, sizes, and hashes are recorded in the
[runtime audio inventory](resource-compatibility-matrix.md#runtime-audio).
Static checks resolved all eight `Music.txt` entries using the loader's
17-column path offset and the `sound.txt` item-sound path.

A08 remains open: the 337 nonblank sound-list entries still include the same
five unavailable WAV paths. Neither the supplied runtime folders nor the KR
source contain those assets. A further inherited list defect was observed:
ID 220 appears for `argosdie.wav` and then `argosidle.wav`, so the current
indexed loader retains the latter mapping. Both files exist. No mapping was
changed because the intended ID for the death sound is not established.

The resource matrix was regenerated for the two added lists. Layout/local-link
and whitespace checks were run for this asset/documentation batch. No build is
required for these runtime lists; client decoding, loading, and playback remain
unverified. No client launch, deployment, approved-digest update, or commit was
performed. This import does not claim native parity or close A08.

## Authorized unavailable-sound cleanup (2026-10-03)

The user explicitly requested removal of the five unavailable audio references
after the KR comparison. Rechecked absence in both runtime and reference before
removing only IDs 55, 95, 96, 103, and 363 from
`tmproject/client748/sound/soundlist.txt`. No WAV file was deleted because none
existed. The original KR list remains unchanged and can restore these entries.

Static validation passed: 332 nonblank declarations remain, every declared file
exists, IDs stay within the loader's 1..511 range, and channel counts are positive.
A line-by-line comparison against the KR list excluding exactly those five IDs
confirmed that every other entry, including the inherited duplicate ID 220,
was preserved without renumbering. Hashes and the removed paths are recorded in
the [runtime audio inventory](resource-compatibility-matrix.md#runtime-audio).

The constructor initializes undeclared slots to an empty path and null pointer.
The loader assigns entries by their explicit numeric IDs, not their line order.
No direct client `GetSoundData` call for the five IDs was found; dynamic requests
are not proven absent and may still produce the existing failed-load diagnostic.
No source, ABI, protocol, or reference asset was changed.

A08's dangling configuration references are resolved by the user's removal
policy. This does not recover the sounds, establish native parity, demonstrate
that the IDs are unused, or approve gameplay. The duplicate ID 220 and actual
client loading/playback remain pending. Documentation layout/local links and
whitespace checks are the batch gates; no build, deployment, client launch,
approved-digest update, or commit is required or performed for this list cleanup.
