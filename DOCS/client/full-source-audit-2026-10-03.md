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

The [previous batch audit](#refactoring-batch) remains a dated
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

<a id="refactoring-batch"></a>
## Earlier refactoring batch audit and follow-up

This dated audit retains its original findings, exceptions and correction evidence. It is not the current defect queue.

<a id="refactoring-batch-claude-refactoring-batch-audit--2026-10-03"></a>

<a id="refactoring-batch-decision-and-audited-snapshot"></a>
### Decision and audited snapshot

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

<a id="refactoring-batch-execution-exception-postgresql-writes"></a>
#### Execution exception: PostgreSQL writes

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

<a id="refactoring-batch-method-and-evidence-boundaries"></a>
### Method and evidence boundaries

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

<a id="refactoring-batch-finding-index"></a>
### Finding index

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

<a id="refactoring-batch-confirmed-findings-and-correction-proposals"></a>
### Confirmed findings and correction proposals

<a id="refactoring-batch-a01--p1-kefra-defeat-is-not-durable-after-persistence-failure"></a>
#### A01 — P1: Kefra defeat is not durable after persistence failure

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

<a id="refactoring-batch-a02--p2-aura-expiry-restores-obsolete-weather"></a>
#### A02 — P2: Aura expiry restores obsolete weather

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

<a id="refactoring-batch-a03--p2-opcode-matrix-drops-unknown-server-sizes"></a>
#### A03 — P2: Opcode matrix drops unknown server sizes

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

<a id="refactoring-batch-a04--p2-resource-and-effects-generators-cannot-read-the-current-tree"></a>
#### A04 — P2: Resource and effects generators cannot read the current tree

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

<a id="refactoring-batch-a05--p2-current-extraction-acceptance-gates-fail"></a>
#### A05 — P2: Current extraction acceptance gates fail

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

<a id="refactoring-batch-a06--p2-equivalence-counts-are-not-computed-correctly"></a>
#### A06 — P2: Equivalence counts are not computed correctly

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

<a id="refactoring-batch-a07--p2-literal-conversion-lacks-rollback-on-compile-exceptions"></a>
#### A07 — P2: Literal conversion lacks rollback on compile exceptions

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

<a id="refactoring-batch-a08--p2-audio-list-references-missing-assets"></a>
#### A08 — P2: Audio list references missing assets

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

<a id="refactoring-batch-a09--p3-resource-documentation-describes-obsolete-absence"></a>
#### A09 — P3: Resource documentation describes obsolete absence

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

<a id="refactoring-batch-a10--p3-affected-files-violate-english-only-delivery"></a>
#### A10 — P3: Affected files violate English-only delivery

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

<a id="refactoring-batch-probable-risks-and-unresolved-hypotheses"></a>
### Probable risks and unresolved hypotheses

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

<a id="refactoring-batch-coverage-by-subsystem"></a>
### Coverage by subsystem

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

<a id="refactoring-batch-commands-and-recorded-results"></a>
### Commands and recorded results

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

<a id="refactoring-batch-suggested-remediation-sequence"></a>
### Suggested remediation sequence

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

<a id="refactoring-batch-closure-criteria-and-delivery-state"></a>
### Closure criteria and delivery state

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

<a id="refactoring-batch-remediation-record-2026-10-03"></a>
### Remediation record (2026-10-03)

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

<a id="refactoring-batch-follow-up-correction-and-verification-2026-10-03"></a>
### Follow-up correction and verification (2026-10-03)

The user authorized corrections after independent verification found A08 open
and A10 only partially corrected. This follow-up preserves all other local
changes and does not update approved relocation digests.

<a id="refactoring-batch-a10-identified-residuals-corrected"></a>
#### A10: Identified residuals corrected

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

<a id="refactoring-batch-a08-blocked-on-verified-asset-identity"></a>
#### A08: Blocked on verified asset identity

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

<a id="refactoring-batch-follow-up-gates"></a>
#### Follow-up gates

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

<a id="refactoring-batch-supplied-kr-audio-follow-up-2026-10-03"></a>
### Supplied KR audio follow-up (2026-10-03)

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

<a id="refactoring-batch-authorized-unavailable-sound-cleanup-2026-10-03"></a>
### Authorized unavailable-sound cleanup (2026-10-03)

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
