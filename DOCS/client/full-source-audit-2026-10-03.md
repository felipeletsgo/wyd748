# Client and Go server source audit — 2026-10-03

> This audit snapshot is preserved below. Corrections were subsequently applied
> to the working tree; see the [correction follow-up](#correction-follow-up--2026-10-03)
> for implementation details, current test results, and unresolved gates. The
> original passing extraction results do not describe the corrected chat methods.

## Result and interpretation

This round found **seven confirmed active-path defects**, **one probable
instance-isolation problem**, **one confirmed latent defect with unproven active
reachability**, and **one English-delivery gap**. Priorities are one P1, seven P2,
and two P3 findings. No P0 was demonstrated. No product correction was made.

Release and Debug builds without deployment, the Go suite, `go vet`, extraction
checks, eleven relocation manifests, and 81 tool tests passed. Four additional
isolated evidence tests reproduced the server defects described below. Those
four passes mean that the defects were reproduced, not that they were fixed.

The source inventory covers **1,151 files and 503,034 lines**, including tests
and bundled dependencies. Every enumerated file was read and pattern-screened;
critical boundaries received the deeper review described in the coverage section.
**This is not an exhaustive manual approval of every source line.** Vendor
internals, every renderer/effect lifecycle, all combat formulas, and all event
combinations remain semantically unverified. This report closes this static and
automated audit pass with those limitations recorded; it does not certify a
bug-free source tree, 7.48 parity, or gameplay readiness.

## Snapshot and method

- Repository: `wyd-go`, branch `main`.
- Entry HEAD: `34b7dcae4068f1b6bcfc152e4037bd9e7ccf0dfa`.
- Audit date: 2026-10-03. The working tree already contained extensive local
  changes and untracked files. The inspected working tree, not HEAD alone, is the
  subject of this report. Line numbers refer to that snapshot.
- Scope: active non-ignored `.cpp`, `.h`, `.hpp`, and `.c` files under
  `tmproject/`, and `.go` files under `wydgo748/`, including their tests and
  source consumers. Build registration and selected resource contracts were
  also checked. Generated binaries, ignored historical corpora, and a complete
  recensus of runtime assets are not part of the per-file source inventory.
- Method: whole-inventory lexical screening; targeted control-flow, ownership,
  validation, and persistence review; source-to-consumer tracing; existing
  regression gates; and isolated reproductions through Go overlays.
- Product source, installed executables, operational data, services, and digest
  approvals were not modified. Builds wrote only to the existing build area.
  New evidence and this report are intentional audit artifacts.
- Classification: this audit itself is documentation/tooling without a
  functional contract change. Proposed safety fixes should preserve existing
  IDs, packet layouts, and persistence semantics. A future change to an
  undocumented legacy format needs its own compatibility review.

The [previous batch audit](refactoring-batch-audit-2026-10-03.md) remains a dated
record of the Claude/refactoring review and subsequent corrections. This report
supplements it rather than overwriting its evidence or declaring all historical
findings reopened. Passing current tests does not establish that every old or new
behavioral claim is complete.

## Finding index

P1 means a high-impact defect on an active failure path; P2 means a material
correctness, validation, or safety defect; P3 means a lower-priority latent or
delivery issue. Priority is not a claim of demonstrated remote exploitation.

| ID | Priority | Finding | Evidence status |
| --- | --- | --- | --- |
| F01 | P1 | DirectSound failure can be reported as success and dereferenced | Confirmed by source/control flow |
| F02 | P2 | Binary animation allocation does not bound subsequent reads | Confirmed by source/arithmetic |
| F03 | P2 | Text asset parsers permit overflows and non-progress loops | Confirmed by source/control flow |
| F04 | P2 | Party indexed-chat decoder writes outside the message table | Confirmed accepted-path defect; injected client execution pending |
| F05 | P2 | ApplyBonus skill learning omits current NPC interaction checks | Reproduced through the actual handler |
| F06 | P2 | Network writer exits without ending the session | Reproduced with a failing connection |
| F07 | P2 | JSON-compatible account/charstate fallback partially commits | Reproduced interface ordering; disk failure integration pending |
| F08 | P2 | Ghost shops bypass the gameplay-space visibility predicate | Publication reproduced; real event/purchase impact pending |
| F09 | P3 | ConvertBGM uses a character value as an output address | Confirmed latent defect; active caller not established |
| F10 | P3 | Active diagnostics and comments still violate English-only delivery | Confirmed examples; complete language census pending |

## Confirmed active-path defects

### F01 — P1: DirectSound errors become success or unsafe partial objects

**Location:** [dsutil.cpp](../../tmproject/TMProject748/internal/platform/windows/dsutil.cpp),
lines 78–81, 169–247, and 270–283.

**Trigger:** an audio driver/device/allocation failure while creating the first
buffer, a failure obtaining the 3D listener, or a failure after some duplicate
buffers have already been created.

**Evidence:** `CSoundManager::Create` initializes `hrRet` to `S_OK`, but an
ordinary failing `CreateSoundBuffer` result is not assigned to it. Cleanup can
therefore return success without setting the caller's `CSound*`.
`GetSoundData` tests `FAILED(Create(...))` and then dereferences that still-null
pointer. `DSERR_BUFFERTOOSMALL` is explicitly admitted into the construction
branch despite being a failing result. The allocation branches also assign the
positive Win32 `ERROR_OUTOFMEMORY` value rather than a failing HRESULT. Listener
initialization ignores `Get3DListenerInterface` before dereferencing its output.
Cleanup deletes the pointer array but does not release partially created COM
buffers. Duplicate-buffer failure does propagate its HRESULT; its separate
problem here is incomplete cleanup, not the first-buffer success bug.

**Impact:** optional audio failure can terminate the client instead of disabling
the affected sound. Partial construction can leak COM resources and make later
failures more likely. Hardware failure was not injected in a running client.

**Origin:** the unsafe branches are already present at entry HEAD. They are not
introduced by this audit; their original introduction was not attributed.

**Suggested correction:** initialize output pointers and buffer slots; return
the actual failing HRESULT; reject unusable buffers; check every listener query;
and release every successfully acquired COM object on partial failure. Use
`E_OUTOFMEMORY` or `HRESULT_FROM_WIN32` where appropriate and keep audio failure
recoverable. Prefer scoped ownership over a manual cleanup array.

**Fix gate:** inject first-buffer failure, `DSERR_BUFFERTOOSMALL`, listener-query
failure, duplicate failure, and allocation failure. Assert no null dereference,
no success with an absent output, and balanced releases. Build both configurations
and later validate real audio/device-loss behavior when client execution is allowed.

### F02 — P2: animation file reads exceed the allocation model

**Location:** [MeshManager.cpp](../../tmproject/TMProject748/internal/render/mesh/MeshManager.cpp),
lines 115–185.

**Trigger:** a missing/truncated `.bon` or `.ani` file, an oversized header, or
animation clips whose frame counts differ.

**Evidence:** `.bon` handling does not validate `_open`, `_filelength`, allocation,
or full `_read` results. A failed length can become an invalid allocation request
and invalid bone metadata. The animation first pass sums ticks but repeatedly
overwrites `numAniFrame`; allocation uses the last frame count multiplied by
the total ticks. The second pass reads each clip using that clip's frame count
and ticks, advancing `nOffset` without a checked capacity bound. For example,
clips with 10 frames/1 tick followed by 1 frame/1 tick allocate two matrices but
the first payload requests ten. Quaternion conversion also trusts that storage.
Integer overflow and short header reads are not rejected.

**Impact:** malformed or inconsistent local assets can corrupt heap memory,
abort initialization, or leave invalid animation state. This is not evidence
that an installed asset is currently corrupt or that a packet can supply this data.

**Origin:** the unchecked reads and sum/last-stride allocation already exist at
entry HEAD. Local organization does not eliminate them.

**Suggested correction:** parse and validate complete headers before allocation;
use checked size arithmetic and bounded totals; verify file lengths, alignment,
bone counts, and complete reads; and keep initialization transactional. If the
format requires a common frame stride, reject inconsistent clips. Otherwise
allocate the checked sum of per-clip sizes and preserve explicit offsets. Do
not silently redefine the legacy format to accommodate bad input.

**Fix gate:** missing files, truncated headers/payloads, zero and oversized
counts, arithmetic overflow, inconsistent frame stride, and allocation failure.
Verify cleanup and unchanged behavior with existing valid 7.48 fixtures.

### F03 — P2: text asset parsers lack bounds and guaranteed progress

**Locations:** [dsutil.cpp](../../tmproject/TMProject748/internal/platform/windows/dsutil.cpp),
lines 64–72; [MeshManager.cpp](../../tmproject/TMProject748/internal/render/mesh/MeshManager.cpp),
lines 62–65 and 104–113; [MeshManager.h](../../tmproject/TMProject748/internal/render/mesh/MeshManager.h),
lines 7 and 35–37.

**Trigger:** a non-numeric token, out-of-range ID, partial record, or long filename
in sound/mesh/animation configuration.

**Evidence:** loops compare `fscanf` only against EOF rather than the required
conversion count. A return of zero makes no progress and can loop forever. The
sound parser does not consume the remainder of a record when its ID is invalid,
so the next filename can become the non-progress token. Its `%s` is unbounded
and partial filename/channel records are not rejected. Mesh names are scanned
into 255 bytes but copied with `sprintf` into a 64-byte destination; the mesh ID
is not checked against the 3,048 entries. Animation-list IDs, type counts, names,
and partial records are likewise used before full validation.

**Impact:** corrupted local configuration can hang startup or overwrite client
memory. This is a resource-loader robustness problem, not a demonstrated remote
attack and not a reason to renumber or delete legitimate resource IDs.

**Origin:** these parsing patterns already exist at entry HEAD.

**Suggested correction:** read bounded lines, require exact field counts, check
numeric ranges before indexing, and bound names to the destination capacity.
Reject invalid input with an English diagnostic and a defined cleanup path.
Define duplicate-ID handling explicitly; retain existing compatibility decisions
instead of treating all duplicate rows as disposable.

**Fix gate:** invalid token, ID zero/upper bound, negative counts, incomplete row,
overlong destination name, duplicate IDs, and valid existing resource lists.
Every rejected record must terminate or advance predictably, never spin.

### F04 — P2: party indexed-chat decoding has an unbounded table write

**Locations:** [TMFieldSceneChat.cpp](../../tmproject/TMProject748/internal/app/scenes/TMFieldSceneChat.cpp),
lines 97–122; [TMFieldScene.cpp](../../tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp),
lines 3946–3968; [TMScene.cpp](../../tmproject/TMProject748/internal/app/scenes/TMScene.cpp),
lines 548–552; [source_client.go](../../wydgo748/internal/wire/source_client.go),
lines 274–281.

**Trigger:** a complete indexed-chat frame routed to the field scene with
nonzero sender ID, `String[0] == 1`, and an out-of-range signed relative index.
Chat and party controls must exist for the unsafe branch to execute.

**Evidence:** the decoder uses `relativeIndex + 1000` to write bytes 126 and 127
of a 2,000-entry global message table before any bounds check, then copies from
that entry. A five-byte numeric fallback buffer is also too small for `-1000`
including the terminator. The base-scene bounded handler applies only when
the sender ID is zero; nonzero IDs reach the field-scene branch. Correct packet
size does not validate the semantic index.

**Impact:** malformed inbound server data can write outside the table or corrupt
the fallback stack buffer. **The current Go `MessageIndexed` builder uses ID zero
and does not emit this party variant.** A legitimate-player exploit or normal
server emission was not demonstrated; execution in the built client is pending.

**Origin:** unsafe table accesses exist at entry HEAD. The local scene-transition
null-control guard addresses a different failure and does not bound the index.

**Suggested correction:** centralize bounded, read-only indexed decoding; validate
each supported envelope before scene-specific handling; use a local bounded copy
and adequately sized fallback. Do not mutate global strings to terminate them.
Do not remove a legacy variant solely because the current server does not emit it;
its intended contract remains a separate evidence question.

**Fix gate:** zero/nonzero sender IDs, absent controls, valid boundary indexes,
`-32768`, `-1001`, `-1000`, `999`, `1000`, and `32767`; empty table entries; and
unchanged neighboring packet framing. Cover both the base and field dispatch paths.

### F05 — P2: skill learning trusts a stale master context

**Locations:** [handlers.go](../../wydgo748/internal/game/handlers.go), lines 497–510;
[skills.go](../../wydgo748/internal/game/skills.go), lines 24–40;
[npc_interaction.go](../../wydgo748/internal/game/npc_interaction.go), line 32 onward.

**Trigger:** open a skill master, move away or lose visibility, then send
ApplyBonus type 2 with the remembered master identity and a valid skill item.

**Evidence:** the common learning helper checks life/world status, master
identity, merchant type, inventory offering, and skill prerequisites, but resolves
the remembered NPC through `mobByID` rather than `resolveNPCInteraction`. The
ordinary BuyItem route performs that interaction check first. The isolated test
moves the player 100 tiles, hides the master, confirms that the authoritative
resolver rejects the interaction, and calls the actual ApplyBonus handler. The
skill is still learned and saved once.

**Impact:** learning can occur remotely through a stale shop context. This finding
does not claim that class, gold, level, or skill-point checks are bypassed.

**Origin:** the missing common-helper check already exists at entry HEAD.

**Suggested correction:** resolve the current interaction inside the common
learning helper before mutation or persistence. Check distance, visibility, and
gameplay space consistently for every ingress. Clear stale context on rejection
where appropriate; clearing context during movement alone is insufficient.

**Fix gate:** remote, hidden, despawned, and other-instance masters must reject
without learning, spending resources, or saving. Keep positive ApplyBonus and
legacy learning tests. Convert the reproduction into a rejection assertion.

### F06 — P2: failed outbound writer leaves a live session lifetime

**Location:** [session.go](../../wydgo748/internal/net/session.go), lines 172–190.

**Trigger:** `conn.Write` fails while the separate reader has not yet failed.

**Evidence:** `writeLoop` returns on the write error without closing the connection
or signaling `done`. An isolated failing connection reproduces an exited writer
with both the connection and `Session.IsClosed()` still open. No write deadline
is installed in this path. The reader can retain ownership until its own failure,
timeout, or later queue pressure; this test is not an end-to-end TCP reproduction.

**Impact:** the server can temporarily retain a session that no longer sends
responses, delaying disconnect cleanup and account/session release. A blocked
write also lacks a bounded outbound deadline. Not every TCP write error will
produce a long-lived zombie: the reader may fail immediately as well.

**Origin:** the return-without-cleanup branch already exists at entry HEAD.

**Suggested correction:** close the session on terminal writer error, coordinate
cleanup through the existing once-protected lifetime, and define bounded write
deadlines. Preserve single disconnect notification and safe concurrent send/close
behavior; do not introduce a send-on-closed-channel race.

**Fix gate:** failing writes and a peer that stops reading, using controlled
connections or `net.Pipe`; verify reader release, one disconnect callback,
authentication-slot release, and no leaked writer. Run race tests when supported.

### F07 — P2: account/charstate fallback is not an aggregate transaction

**Locations:** [charstate.go](../../wydgo748/internal/game/charstate.go), lines 173–186;
[store/charstate.go](../../wydgo748/internal/store/charstate.go), lines 64–103;
[main.go](../../wydgo748/cmd/server/main.go), lines 279–280;
[celestial_capsule.go](../../wydgo748/internal/game/celestial_capsule.go), lines 238–243.

**Trigger:** use the JSON-compatible fallback store, persist charstate, then fail
the subsequent account write during an operation changing both representations.

**Evidence:** stores implementing `SavePlayerState` use an aggregate operation.
Otherwise `saveAccountAndCharStateForUID` calls `SaveCharState` first and
`SaveAccount` second, without compensating the first write on failure. JSONStore
has separate atomic file operations but no `SavePlayerState` implementation.
An in-memory store implementing the same fallback interfaces reproduces a changed
charstate and a failing account save. No operational files were written or a real
disk fault injected. The Celestial seal path illustrates a consumer that can
delete charstate before an account rejection and restore only its in-memory account.

**Impact:** fallback persistence can leave durable account and session state
inconsistent after failure or restart. This applies to JSON/fallback mode, not
to the PostgreSQL aggregate transaction. The PostgreSQL `SavePlayerState` path
at [postgres.go](../../wydgo748/internal/store/postgres.go), lines 1676–1759,
was reviewed separately and commits the aggregate transactionally.

**Origin:** the fallback ordering already exists at entry HEAD. This is not a
claim that the previously corrected PostgreSQL fencing issue has returned.

**Suggested correction:** implement a recoverable aggregate/journal for JSON
account and charstate updates, or reject operations requiring atomic aggregate
persistence when that capability is unavailable. Reordering the two writes or
rolling back RAM alone does not establish crash-safe atomicity.

**Fix gate:** fail each aggregate stage, restart after each durable boundary,
and verify account plus charstate recovery together. Cover deletion, replacement,
repetition, and existing JSON journal behavior, as well as PostgreSQL regression
tests against an explicitly disposable database.

## Probable impact, latent code, and delivery debt

### F08 — P2: ghost shops are outside the gameplay-space predicate

**Locations:** [visibility.go](../../wydgo748/internal/game/visibility.go), lines 162–169;
[visibility_delta.go](../../wydgo748/internal/game/visibility_delta.go), lines 127–137;
[ghost_shop.go](../../wydgo748/internal/game/ghost_shop.go), lines 285–351.

**Trigger:** a private-runtime participant and a public ghost shop occupy the
same world coordinates, or equivalent coordinates pass existing proximity checks.

**Evidence:** shop publication lacks the space predicate used for players, mobs,
and ground items. Listing and purchase paths check identities and proximity but
not a shop gameplay space. The fixture establishes authoritative private runtime
membership and then publishes a public shop at the player's coordinates; the
shop is marked visible. This proves the missing publication boundary in that
state. It does not prove that a current production event overlaps Armia shop
coordinates, or that an actual cross-instance purchase has occurred.

**Impact/status:** probable visibility and commerce isolation violation if that
coordinate combination is reachable. The missing predicate is confirmed; real
event reachability and economic impact remain pending.

**Origin:** the publication predicate omission already exists at entry HEAD;
local clone-identity corrections are not the cause identified here.

**Suggested correction:** define a stable shop namespace/runtime and enforce it
for publication, listing, and buying. A public shop should not become private
merely because its owner later enters another instance. Comparing against the
seller's current space alone would introduce a new lifecycle problem.

**Fix gate:** same-coordinate public/private viewers, listing and purchase,
owner leaving the area/instance, disconnect cleanup, and unchanged trade rollback.
Prove the intended current-event routing before describing this as an exploited
production economy bug.

### F09 — P3: ConvertBGM writes through a character value

**Location:** [DirShow.cpp](../../tmproject/TMProject748/internal/platform/windows/DirShow.cpp),
lines 43–64 and 354–389.

**Trigger:** a future or indirect caller selects music slot 13/14 and reaches
`ConvertBGM`.

**Evidence:** the destination argument is `(char*)szTemp[nLenF + 3]`, a character
value cast to an address, not an address into `szTemp`. That byte is also read
beyond the copied string terminator. The preceding `sprintf_s(szTemp, szFileName)`
uses a filename as a format string. Current direct source callers of `PlayMusic`
select slot zero; no caller of `PlayMusic2` was found. Startup nevertheless assigns
slot 13/14 origins, so blank configuration is not evidence that this code is safe.

**Impact/status:** invalid writes or a crash if activated. The coding fault is
confirmed, but active gameplay reachability was not established; this is not an
explanation for a currently observed music failure.

**Origin:** the incorrect pointer expression already exists at entry HEAD.

**Suggested correction:** construct the destination path with explicit bounds
and a constant format, then validate conversion reads and allocations. First
resolve the expected legacy `.dat`/`.bon` conversion contract with existing
evidence or a narrowly scoped native investigation; do not invent the format or
delete the path merely because direct callers are absent.

**Fix gate:** direct conversion with a known encoded fixture, short/long/empty
names, and failed reads; retain this as latent until an actual caller is demonstrated.

### F10 — P3: English-only delivery remains incomplete

**Examples:** [account-api/main.go](../../wydgo748/cmd/account-api/main.go), line 41;
[drops.go](../../wydgo748/internal/game/drops.go), line 162;
[shutdown.go](../../wydgo748/internal/game/shutdown.go), line 36;
[client_integrity.go](../../wydgo748/internal/data/client_integrity.go), line 26;
[session.go](../../wydgo748/internal/net/session.go), lines 1–3 and 171–195.

**Trigger/evidence:** normal startup failures, full-inventory drop diagnostics,
shutdown failures, malformed integrity input, and inspection of active comments
expose Portuguese text. These examples violate the repository-wide rule even
when not player-facing. No complete language census or global release approval
is asserted by this round. Protocol identifiers and persisted compatibility values
are not automatically translatable.

**Impact/status:** confirmed delivery/documentation debt, not a demonstrated
packet or persistence bug. The debt predates this audit; individual introductions
were not attributed to one contributor.

**Suggested correction:** translate touched diagnostics, comments, and associated
test expectations in bounded batches; maintain an active-source migration inventory.
Review compatibility explicitly before changing persisted or contract-bearing text.
Do not equate an English UI with a complete repository language migration.

**Fix gate:** inspect every authored string in each affected file and its tests;
run a separate active-repository language audit before claiming global completion.

## Coverage and remaining semantic gaps

### Per-file coverage

The reproducible inventory is generated by
[census.ps1](../../.agents/research/full-source-audit-2026-10-03/census.ps1).
Each entry records path, group, size, line count, test status, and lexical markers.
Its coverage label is deliberately `read and pattern-screened; not a per-line
manual approval`. Marker counts are investigation signals, not bug counts or
proof that an unmarked file is safe. UTF-8 marker decoding is not a language audit
of all legacy encodings.

- [Client file coverage](../../.agents/research/full-source-audit-2026-10-03/tmproject-coverage.json):
  686 files, 404,624 lines. First-party code and tests: 493 files, 134,089 lines;
  bundled dependencies: 193 files, 270,535 lines. The first-party total includes
  44 test files and 11,233 test lines.
- [Go file coverage](../../.agents/research/full-source-audit-2026-10-03/wydgo748-coverage.json):
  465 files, 98,410 lines; 200 production files and 265 test files.

### Subsystem coverage

All rows below received inventory screening. Deeper review is **selected flows**,
not a claim that every line in that row was manually inspected.

| Subsystem | Source files | Additional review/evidence | Not verified by this round |
| --- | ---: | --- | --- |
| Client app/scenes | 37 | Base/field dispatch, indexed messages, chat lifecycle, initialization, music consumers; extraction gates | Every scene transition, control event, or gameplay branch |
| Client application/platform | 57 | Socket receive/framing, send progression, sound/music failure handling; both builds | Real drivers, device loss, operating-system runtime integration |
| Client core/game | 81 | Human appearance consumers, separated ownership and registration, relocation checks | Every formula, animation state, and destruction interleaving |
| Client render | 136 | Mesh text/binary loaders and allocation model; shader/costume build checks | Every one of the rendering/effect lifecycles and visual correctness |
| Client UI/wire | 129 | Indexed UI packet consumers, wire registration, bounded base message decode, architecture/socket tests | Every tooltip, resource binding, UI state sequence, and undocumented variant |
| Other client first-party/test files | 53 | Build inclusion and test execution where registered | Unregistered/demo runtime behavior |
| Client bundled dependencies | 193 | Lexical screening and normal compilation | Independent security, CVE, ABI, or full ownership audit of vendor implementations |
| Go gameplay | 284 | Auth/session publication, economy, trade, Hell Store, shop lifecycle, NPC/skill ingress, inventory, drops, visibility, instance ownership, selected guild/Kefra/Celestial transitions | Every spell/formula, AI path, event coordinate combination, and lifecycle permutation |
| Go persistence | 14 | Account snapshots, synchronous/async boundaries, PostgreSQL aggregate/fencing source, JSON fallback; isolated failure reproduction | Real PostgreSQL rollback/locking/crash recovery and real disk fault injection |
| Go net/wire/network policy | 30 | Framing/session lifetime, builders, direction/consumer checks, admission boundaries and tests | Live socket stress, race detector, every accepted payload's semantics |
| Go account/API/control/web and operational entry points | 51 | Password/claim paths, staff authorization/capabilities, loopback control handling, rate/read boundaries, selected startup paths, existing tests | External HTTP deployment, live database services, complete manual review of every handler |
| Go data/model | 81 | Selected loaders, integrity checks, item/ground constraints and consumers; suite and vet | Full historical-data/formula proof against native 7.48 |
| Go loadtest command/package | 5 | Inventory, suite, vet | Live loadtest/provisioning against operational services |

The Go account/API/control/web row includes account, NPC-conversion, web,
network-admission, and server command entry points. Source counts are grouping
aids, not coverage percentages.
The per-file JSON is authoritative for exact membership.

### Contract and evidence limitations

- Source-built 0x105/0x106 indexed messages are coordinated extensions, not proof
  that stock 7.48 supports them. F04 separates the current builder from the unsafe
  alternate decoder instead of conflating their envelopes.
- Existing native evidence was reused; no broad Ghidra census was restarted.
  Memory-safety checks and internal server lifecycle failures do not require a
  speculative new native interpretation. F09's conversion semantics and the
  intended legacy party-message variant remain narrowly scoped evidence gaps.
- Tests named for 7.59 elemental ordering are historical-algorithm evidence,
  not independent proof of native 7.48 parity. Passing such tests does not
  establish the missing native validation for every combat formula.
- Prior resource/audio reconciliation evidence was not regenerated merely to
  rehash unchanged assets. This source round does not certify asset identity,
  successful playback, installed gameplay, or visual acceptance.

## Commands and results

Commands were inspected for operational side effects. Builds used `-NoDeploy`;
database-test variables were removed only from the child test process. No game,
server service, deploy, approval rewrite, or operational database migration ran.

- **STATICALLY VERIFIED:** selected control-flow/arithmetic findings and declared
  extraction/registration policies, with the coverage limits above.
- **AUTOMATED TESTED:** enabled Go/C++/tool gates and isolated defect reproductions.
  A reproducer expecting a defect is not a successful correction test.
- **CLIENT-TESTED:** not performed. No `CLIENT_TESTED` catalog promotion or runtime
  approval follows from this audit.

| Gate | Command/path | Result |
| --- | --- | --- |
| Go integration | `go test -count=1 -json ./...` from `wydgo748/`, with `WYD_TEST_POSTGRES_URL` and `WYD_WEB_TEST_DATABASE_URL` unset in that process | Exit 0; enabled tests passed; PostgreSQL gates skipped. Verbose output exceeded the display limit and was not saved as a complete raw log |
| Go static analysis | `go vet ./...` from `wydgo748/` | Exit 0 |
| Defect reproductions | `go test -count=1 -overlay ../.agents/research/full-source-audit-2026-10-03/overlay.json -run '^TestAuditEvidence' -v ./internal/game ./internal/net` | Four tests passed by reproducing the audited behavior |
| Client Release | `pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 -Configuration Release -NoDeploy` | Exit 0; artifact under `tmproject/build/TMProject748/Release/` |
| Client Debug | Same build command with `-Configuration Debug -NoDeploy` | Exit 0; artifact under `tmproject/build/TMProject748/Debug/` |
| Client build tests | ArchitectureTests, SocketReceiveTests, costume and shader checks | Both builds passed: 1,144,287 architecture assertions; 221 socket assertions; 135 costume items, 129 renderers, 774 parts; four shader variants plus negative checks |
| Human extraction | `python .agents/research/tmhuman-separation.py --complete --with-policies` | Passed; 128 baseline methods, 126 current; ownership, registration, and policy checks passed |
| Field extraction | `python .agents/research/tmfieldscene-separation.py --complete --with-policies` | Passed; 293 baseline methods, 282 current, eleven helper checks; ownership/registration passed |
| Other source relocation | `python .agents/research/source-relocation.py --manifest <manifest>` for each of the eleven manifests listed below | All passed without updating approvals |
| Tool regressions | The seven Python test scripts listed below | 81 tests passed |
| Source inventory | `census.ps1 -Root tmproject` and `census.ps1 -Root wydgo748` | 1,151 files enumerated and screened; coverage artifacts saved |
| Census helper validation | PowerShell parser check and `census.ps1 -Root invalid` | Parser passed; invalid root rejected as expected |
| Documentation | `pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1`, without `-UpdateMap` | Passed; layout and local links valid, 173 documents indexed |
| Whitespace | `git diff --check` | Exit 0; existing LF/CRLF conversion warnings, no whitespace errors |

The eleven relocation manifest names are `app-newapp.json`,
`core-basedef-header.json`, `core-basedef.json`, `render-device.json`,
`render-ground.json`, `render-skinmesh.json`, `scene-selectchar.json`,
`scene-selectserver.json`, `scene-tmscene.json`, `ui-controls.json`, and
`ui-sgrid.json`. Extraction equivalence and approved digest matches check the
declared extraction policy; they do not prove runtime behavioral equivalence.

Tool scripts under `.agents/research/`: `tmhuman-separation-tests.py` (12),
`tmfieldscene-separation-tests.py` (22), `source-relocation-tests.py` (9),
`opcode-matrix-tests.py` (3), `literal-escape-tests.py` (6),
`extract-method-tests.py` (21), and `worktree-tests.py` (8).

### Reproduction artifacts

The [game evidence tests](../../.agents/research/full-source-audit-2026-10-03/game_evidence_test.go),
[network evidence test](../../.agents/research/full-source-audit-2026-10-03/net_evidence_test.go),
and [overlay](../../.agents/research/full-source-audit-2026-10-03/overlay.json)
exercise private package paths without adding tests inside the product trees.
The overlay contains this workstation's absolute paths and must be remapped on
another checkout. Tests deliberately expect the faulty behavior; invert and
extend their assertions when implementing fixes. They are not permanent passing
acceptance tests for the current implementation.

### Gates not executed or not sufficient

- Race detector: `CGO_ENABLED=0`; a usable local C compiler was not found.
  No compiler was installed and no race result is claimed.
- PostgreSQL: five store tests gate on `WYD_TEST_POSTGRES_URL` (four in
  `postgres_test.go`, one in `postgres_fence_test.go`), and one server/web test
  gates on `WYD_WEB_TEST_DATABASE_URL`. These integration prerequisites were
  intentionally disabled rather than pointing at operational data. Source review
  and in-memory tests do not replace transaction/lock behavior on PostgreSQL.
- JSON fixture tests contain additional conditional skips if project account
  fixtures are absent. The truncated suite output does not support a definitive
  complete skip count, so none is invented here.
- Client runtime/visual/audio flow: not run, consistent with the existing Windows
  execution restriction. No installed `project.exe` was overwritten. No finding
  or catalog entry was promoted to `CLIENT_TESTED`.
- Client malformed-input runtime, sanitizers, live network stress, crash-recovery
  testing, independent vendor security audit, full language census, and complete
  manual review of all source lines: not executed/established.
- Assertion counts, builds, lexical screening, digest approval, and historical
  visual acceptance are not evidence that this audited snapshot behaves correctly
  in gameplay.

## Correction order and closure

1. Fix F01 with failure injection; it turns optional audio failure into a client
   crash. Then bound loaders in F02/F03 and the decoder in F04.
2. Close F05/F06 through common validation and lifetime ownership, preserving
   existing positive flows and testing negative paths explicitly.
3. Make F07's supported fallback contract atomic or fail closed. Validate against
   disposable storage before calling the persistence fix complete.
4. Resolve F08's shop namespace and prove event reachability; test both visibility
   and purchase. Keep its probable impact distinct from confirmed publication.
5. Treat F09 as latent until the conversion contract/caller is established, and
   handle F10 as bounded English migration work with its own coverage inventory.

New audit evidence and documentation are the only authored changes in this round.
The report is linked from the central documentation entry point and map. No
correction, commit, deployment, service change, or gameplay approval is included.

Closing read-only layout/link and whitespace checks passed. All newly authored
report/evidence text was inspected for English delivery; no existing product text
was translated during this read-only round. No files were removed. The remaining
semantic and runtime gates above are open; none should be inferred complete from
the successful automated gates.

## Correction follow-up — 2026-10-03

### Current result

The F01–F09 safety and validation changes have been implemented in the working
tree. F10 received a bounded English migration in the affected files, not a
repository-wide language certification. The original audit, line references,
reproduction overlays, and command results above remain historical evidence;
they must not be used as passing correction tests.

This batch is classified as `MODERNIZACAO_COMPATIVEL`: no packet size, opcode,
resource ID, encryption-key byte, or persisted identifier was intentionally
changed. The server remains authoritative. Unsafe resource aliases are
suppressed, and non-atomic aggregate persistence is explicitly rejected; those
failure-path changes and their compatibility limits are recorded below. No new
native parity or `CLIENT_TESTED` claim is made.

The existing dirty working tree was preserved. No commit, deployment, service
change, operational database write, asset move/removal, or game execution was
performed. Build artifacts remain under the ignored build directories. The
installed `client748/project.exe` was already deleted at entry; this batch did
not delete, restore, or replace it. No approved extraction digest was rewritten.

### Corrections and acceptance evidence

| Finding | Implemented correction | Current acceptance status |
| --- | --- | --- |
| F01 | DirectSound errors propagate as failing HRESULTs; partial buffers and wave ownership use RAII; output is published only after filling/positioning succeeds; listener initialization and the global manager lifetime are checked | Source-reviewed and compiled; isolated partial-ownership tests pass. Real DirectSound failure injection and playback remain unexecuted |
| F02 | Animation headers, stride, exact file extent, cumulative allocation, second-pass reads, and mapping indices are bounded; failed initialization frees all partial animation allocations | Arithmetic tests and current resource headers pass. Native/runtime acceptance of suppressed aliases remains open |
| F03 | Whole-line parsing rejects malformed/overlong records, extra fields, invalid IDs/counts/parts, and paths exceeding their destination | Parser regressions pass; current sound and mesh lists parse successfully |
| F04 | Indexed party messages validate their envelope and signed table index, copy bounded read-only text, and render decoded text rather than the binary payload | Boundary/immutability tests pass; the two intentional chat changes are not extraction-approved; client UI execution remains open |
| F05 | ApplyBonus re-resolves the current skill master through visibility, gameplay-space, life/definition, and distance validation | Actual handler rejects five invalid scenarios without spending or saving; valid interaction still learns and saves |
| F06 | Writer exits close the socket/session; each frame has a deadline; write errors, short writes, deadline setup errors, and queue overflow terminate the I/O lifetime | Fake-connection and stalled `net.Pipe` regressions pass, including the actual Serve disconnect callback |
| F07 | Aggregate account/charstate writes require the atomic adapter; a split adapter is rejected before either component is written | Nil/deletion and nonempty state rejection tests pass. PostgreSQL integration/crash recovery were not exercised |
| F08 | Public ghost shops use a shared public-space predicate for publication, reconciliation, list binding, purchase, and sold-item updates | Private-runtime rejection, stale visibility removal, a real bound purchase after a space transition, and the public positive purchase pass |
| F09 | ConvertBGM validates a bounded `.dat` destination, derives a `.bon` source, checks allocation/read/write results, and replaces the destination only after a committed exclusive temporary file | Path regressions and builds pass. Active conversion reachability, format semantics, and disk/crash failure injection remain unestablished |
| F10 | English diagnostics/comments replace Portuguese in the touched server files and modified skill tests | Affected-file text review passed; untouched legacy text remains migration debt |

#### F01: resource ownership and failure behavior

[dsutil.cpp](../../tmproject/TMProject748/internal/platform/windows/dsutil.cpp)
now clears the caller's sound pointer before work and rejects invalid buffer
counts. `OwnedSoundBuffers` starts with null references and releases only
acquired buffers on failure. Ownership transfers only after `CSound` construction
succeeds; a fill or positioning failure then destroys the completed owner rather
than leaking it or returning a partial object. `std::bad_alloc` becomes
`E_OUTOFMEMORY`, and the success warning `DS_NO_VIRTUALIZATION` remains valid.

Primary-buffer format and listener failures are propagated. Destruction clears
`g_pSoundManager` when it owns that global, preventing the existing failed-init
delete path from leaving a dangling manager. Buffer restoration no longer retries
indefinitely; waveform reset/read failures unlock the buffer, and a zero-progress
repeat read terminates with failure. Array destruction uses `delete[]`.

The fake COM ownership test exercises partial cleanup and successful transfer,
not the production driver. Before runtime approval, use an injectable DirectSound
adapter to fail each creation/duplication/listener/fill step and verify no leaked
reference, published partial object, or stale global.

#### F02/F03: bounded loaders without invented asset formats

[MeshManager.cpp](../../tmproject/TMProject748/internal/render/mesh/MeshManager.cpp)
uses checked matrix extents before multiplication or allocation. Both animation
passes must agree on positive counts and a common frame stride. Reads cannot
exceed the first-pass allocation; a newly appeared cut is not read into an
unplanned allocation, and a previously present cut disappearing causes failure.
Missing cuts remain sparse rather than being silently renumbered as dense input
to the file-discovery pass. Initialization failure and destruction release bone,
matrix, and quaternion allocations.

The existing native bone evidence was reused. Raw `.bon` bytes and their exact
length are preserved, including formats whose complete raw file length is not a
multiple of eight; this batch does not reinterpret them or impose an unsupported
global alignment rule. The current configured bone files happen to be aligned,
which does not establish that every historical bone file must be aligned.

Current category 3/5 resources contain **42 invalid character aliases** under
the legacy `index / 100 + 59` formula. Rejecting the entire resource would break
the current asset set. These resources remain loadable, but the out-of-bounds
alias writes are skipped. The former category 7/28 branch also indexed outside
the six-class mapping and was removed; its resources remain loadable without
that invalid alias. No replacement class/type is guessed. A future mapping
change must trace the relevant native consumer and establish the intended
observable animation before enabling a replacement.

`AssetLoadValidation` consumes complete physical lines and validates before
indexing or copying. Bone-list duplicate IDs are rejected; mesh/sound lists keep
their existing last-valid-row-wins behavior. The current asset check covered
**785 animation headers**, **one sparse missing cut**, **332 sound records**, and
**1,703 mesh records**, with zero remaining extent/parser errors. It is a static
compatibility check, not a complete resource-format or animation/playback test.

#### F04: bounded indexed chat, with an explicit approval gap

[TMFieldSceneChat.cpp](../../tmproject/TMProject748/internal/app/scenes/TMFieldSceneChat.cpp)
uses `IndexedPartyMessage` to decode the signed 16-bit code with `memcpy`, reject
indices outside the 2,000-row table, and copy at most the output capacity minus
one. Empty entries use a bounded numeric fallback. Neither packet memory nor the
global message table is modified. Chat formatting is bounded, and the indexed
path actually uses the decoded text.

The tests cover extreme codes, valid edge indices, unsupported/short envelopes,
empty entries, and a table row without a terminator. They do not establish every
legacy party-message variant or UI effect. Both
`OnPacketMessageChat(MSG_MessageChat*)` and
`OnPacketMessageChat_Index(MSG_MessageChat*)` now differ from their approved
extraction digests. This is expected from the safety fixes, but it is a **failing
integration policy gate**, not an approval. Keep the old digests until an
independent review accepts the new method bodies and their behavioral evidence.

#### F05–F08: authoritative boundaries and persistence

[npc_interaction.go](../../wydgo748/internal/game/npc_interaction.go) rejects a
missing definition, dead or unmaterialized NPC, a gameplay-space mismatch, or a
Chebyshev distance greater than eight. Skill learning uses that common resolver
rather than trusting a previously opened shop context. Existing global-NPC
visibility exceptions remain governed by the shared visibility policy.

[session.go](../../wydgo748/internal/net/session.go) closes the connection on
every writer exit so the blocked reader can finish and the Serve-owned disconnect
callback releases gameplay ownership. The default per-frame write timeout is ten
seconds. Tests cover a peer that stops reading after a real handshake, in addition
to synthetic write/deadline failures; they do not substitute for long-running
network/concurrency stress.

[charstate.go](../../wydgo748/internal/game/charstate.go) no longer attempts an
account write followed by a separate sidecar write. PostgreSQL's aggregate
adapter remains the supported atomic path. **The JSON adapter does not implement
that aggregate contract, so operations requiring account-plus-charstate atomicity
now fail closed on it, including sidecar deletion.** Standalone legacy writes
remain available where no sidecar identity/adapter is involved. This is a safety
restriction, not an implementation of atomic JSON storage. Do not enable the
aggregate-dependent JSON flows by restoring ordered writes; either implement a
durable recoverable aggregate protocol or use the transactional adapter, with
disposable-storage rollback/crash tests.

Ghost shops are defined as stationary public-world entities, independent of the
owner's later runtime membership. Opening requires public Armia; viewing and
buying require the observer to remain in public gameplay space. Closing an
existing shop is still allowed after a transition. The new purchase regression
first opens and binds a valid public listing, changes only the buyer's
authoritative space, verifies zero mutations/saves, then returns the buyer to
public space and verifies one successful purchase. This exercises the actual
economy handler rather than relying on an invalid listing to reject the request.

#### F09/F10: latent conversion and bounded language migration

[DirShow.cpp](../../tmproject/TMProject748/internal/platform/windows/DirShow.cpp)
retains the opaque key bytes and empty unsupported music slots. Conversion no
longer casts a character to an output pointer or uses a path as a format string.
An exclusive sibling temporary file prevents truncating a prior destination on
read/write failure; only a temporary file created by this call is cleaned up.
No conversion was executed, and no active legacy conversion contract was newly
approved.

English migration covers `npc_interaction.go`, `skills.go`,
`skills_integration_test.go`, `charstate.go`, `drops.go`, `shutdown.go`,
`session.go`, and the changed diagnostics in `client_integrity.go` and
`cmd/account-api/main.go`, along with new helpers/tests/documentation. Protocol
and persisted values were not translated. Existing untouched Portuguese tests,
documentation, or player-facing content remain debt; this batch does not certify
a global English release.

### Changed-file inventory for this correction batch

Paths below distinguish this batch from the much larger pre-existing local diff.
No whole product file or asset was removed by this batch.

- Client implementation: `internal/platform/windows/dsutil.cpp`,
  `internal/platform/windows/DirShow.cpp`, `internal/render/mesh/MeshManager.cpp`,
  and `internal/app/scenes/TMFieldSceneChat.cpp`, under `tmproject/TMProject748/`.
- New client helpers under `internal/application/`: `AssetLoadValidation.h`,
  `OwnedSoundBuffers.h`, and `IndexedPartyMessage.h`.
- Client regression/build registration: new `tests/AuditSafetyTests.cpp`,
  `tests/PacketViewTests.cpp`, `tests/ArchitectureTests.vcxproj`,
  `TMProject748.vcxproj`, and `TMProject748.vcxproj.filters`.
- Go game implementation under `wydgo748/internal/game/`: `npc_interaction.go`,
  `skills.go`, `charstate.go`, `ghost_shop.go`, new `ghost_shop_space.go`,
  `visibility.go`, `visibility_delta.go`, `drops.go`, and `shutdown.go`.
- Go regression coverage: new `internal/game/audit_boundary_regression_test.go`,
  updated `internal/game/skills_integration_test.go`, and new
  `internal/net/session_writer_regression_test.go`.
- Other Go implementation/diagnostics: `internal/net/session.go`,
  `internal/data/client_integrity.go`, and `cmd/account-api/main.go`.
- Documentation: this existing report, with the historical audit retained.

### Correction validation and remaining gates

The following table supersedes the original audit results only for this
correction batch. PostgreSQL test URLs were unset in the child test process,
never printed, and not changed in the user's persistent environment.

| Gate | Correction result |
| --- | --- |
| Focused Go regressions | Skill-master rejection/positive flows, public shop publication/reconciliation/purchase, non-atomic aggregate rejection, and writer failure/deadline lifecycle tests passed |
| Go integration | `go test -count=1 ./...` passed with operational database test URLs unset; final strengthened game regressions subsequently passed in a focused run |
| Go static analysis | `go vet ./...` passed |
| Client Release | `pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 -Configuration Release -NoDeploy` passed; final incremental build produced `tmproject/build/TMProject748/Release/WYD.exe` |
| Client Debug | Same command with `-Configuration Debug -NoDeploy` passed; produced `tmproject/build/TMProject748/Debug/WYDestiny.exe`. Existing `inet_addr` and `WSAAsyncSelect` deprecation warnings remain |
| C++ safety regression | 51 new isolated checks integrated into ArchitectureTests; both configurations report 1,144,338 total checks and SocketReceiveTests reports 221 checks. Costume and shader checks passed |
| Current asset compatibility | Animation/header/extent and sound/mesh list checks passed with the compatibility exceptions documented above; no resource was rewritten |
| Human extraction | `tmhuman-separation.py --complete --with-policies` passed |
| Field extraction | `tmfieldscene-separation.py --complete --with-policies` failed only for the two changed chat method digests; no missing/extra method discrepancy reported |
| Relocation policies | All eleven previously listed manifests passed without approval updates |
| Tool regressions | Six tool suites passed (59 tests). The 22-case field suite reported one error and 23 subtest failures because its baseline fixture rejects the same unapproved chat changes before the intended test-specific mutations can be checked |
| Documentation | `pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1` passed without `-UpdateMap`; 174 documents indexed and local links valid |
| Whitespace | `git diff --check` passed; existing LF/CRLF warnings remain, with no whitespace errors |

Still open: independent acceptance of the two chat digests, the blocked field
tool regression fixture, disposable PostgreSQL/atomic-storage integration,
race detection (no usable local CGO compiler), production DirectSound/loader
failure injection, conversion disk/crash recovery, animation alias behavior,
client UI/audio/visual execution, and the full English migration. No approval,
deployment, runtime parity, or gameplay readiness follows from the passing
automated gates.
