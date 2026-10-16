---
id: chat-tab-status
title: Slash chat commands, tab labels and SND status
subsystem: ui
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-15
---

# Tab labels and status commands

## Question

Why do `/tab text` and `/snd text` look up nonexistent players instead of
setting messages? How can nine tab colors cross the existing client/server
boundary without respawning characters or interrupting animation?

## Evidence boundary

- USED: native `references/client748/WYD.exe`, hash above, read-only Ghidra
  project `tmproject/build/native-research/WYD748.gpr`; cached decompilation
  `FUN_004662c5.c`. Focused `ExportWydFlow.java` and string-reference exports
  confirmed the command branch, caller and callees without modifying evidence.
- USED: current chat submission, whisper/chat handlers, entity materialization,
  label rendering, character reset and packet contract source.
- SECONDARY: `references/W2PP-master/Code/TMSrv/_MSG_MessageWhisper.cpp`,
  `/tab`, `/snd` and empty-body character lookup branches. This supplies server
  semantics, not independent proof of the original 7.48 server implementation.
- NOT USED: later TMProject or KR behavior as native 7.48 parity authority.
  No assets, meshes or UI resource IDs change.

## Native 7.48 flow

### Observable entry

Submitting slash text in FieldScene enters `FUN_004662c5`. `/tab` is a
special nickname branch; other targets take the generic whisper route.
No `snd` command literal was located in the native binary: the generic route
already transports it for authoritative server handling.

### Callers

Ghidra records a DATA reference from the UI callback vtable at `0x005a42ec`
to `FUN_004662c5 @ 0x004662c5`. The `tab` string at `0x005b7fb0` is referenced
by its comparison at `0x004684cc`. The bootstrap `FUN_00435b13` independently
references the other `tab` string at `0x005b7a2c`, from `0x004409f9`.

### Callees

The submission routine parses/formats strings through `FUN_005909d2` and
`FUN_0058f078`, compares the command, checks cast progress and sends through
`FUN_0055f2dd`. The nickname branch suppresses identical text and uses the
ordinary `0x334` whisper envelope. The color extension intentionally removes
that text-only suppression: equal text may now carry a different color.

W2PP's server stores a tab above the character and republishes CreateMob
metadata. It limits Mortal tab creation to displayed level 70 or above.
Its SND branch stores presentation text; the empty-body `/PlayerName` lookup
returns that text. Ordinary whispers do not trigger an automatic SND reply.
Those server rules are adopted explicitly, not asserted as native binary proof.

## State and lifecycle

### Transition matrix

| Event | Authoritative state | Publication |
| --- | --- | --- |
| `/tab text` or `/tab2..9 text` | Bounded text and color 0..8 | Owner and already-visible same-space observers |
| Tab command without text | Empty text and color 0 | Clear label in place |
| `/snd text` | Private bounded status | Private confirmation only |
| Empty-body `/PlayerName` | No mutation | Summary, then status if present |
| New observer or CP refresh | Existing tab retained | Complete player metadata |
| Character logout/relogin | Labels reset | New session starts empty |

### Vtables, vptrs, and receivers

The native callback reference above identifies chat entry. Source dispatch
continues through `TMFieldScene`; received metadata selects the existing
`TMHuman` by entity ID. No vptr write, new virtual method or ABI hook is added.

### Ownership

`Player` owns session strings and color. `TMHuman` owns a 26-byte nickname
array and color index; the scene-owned existing `SText` renders them.
`SetTabLabel` copies text, never retains a borrowed packet pointer, and tolerates
the owned buffer being passed back during `InitObject`.

### Partial failure

Server rejects invalid session/world state and flood attempts, explains the
Mortal level requirement, and bounds direct requests independently of the UI.
Client bounds outgoing and incoming tab copies, tolerates a missing label
control, and maps unknown color values to gray. No optimistic actor mutation
is needed before the authoritative snapshot arrives.

### Cleanup and teardown

No additional allocation or effect is created. Normal character/scene teardown
owns the existing text control. Live updates call only label setters and the
existing metadata branch, not `InitObject`, movement, vitals or cast effects.

### Shutdown

Normal World and scene shutdown discard ephemeral state; no persistence,
worker, timer or external cleanup owner is added.

### Logout and relogin

`resetCharacterRuntime` clears tab, color and SND, preventing leakage when a
connection selects another character. No database schema or saved account
format changes. Neither status nor tab survives relogin in this implementation.

## Wire, ABI, and resources

Client command transport remains `0x334`, 128 bytes: target at 12 (16 bytes),
text at 28 (96 bytes), existing color field at 124. Server also supports the
existing `0x333` chat command route. Text fields remain legacy byte strings.

Tab-color v1 is an `EXTENSAO_COORDENADA` in the already-coordinated source
CreateMob ABI, **not a claim about native 7.48's original CreateMob size**:
`0x364/328`, nickname at 300 (26 bytes, 25 text plus NUL), cape/server at 326,
color at 327 (former trailing padding). Size and all prior offsets stay fixed.
Indices 0..8 map to gray, red, green, blue, yellow, cyan, pink, orange, white.
Zero keeps the previous gray; invalid indices fall back to zero.

Trade `0x363/352` uses 326..349 as shop title, so byte 327 is never interpreted
or written as tab color there. Trade labels keep gray. Older source clients
ignore the formerly unused color byte; older servers send zero. Both updated
components are required for the new colors; no claim of native-binary color
support is made. The normal source ABI already differs from the native ABI.

SND accepts 87 bytes, reserving eight for the `Status: ` prefix in the 95-byte
notice payload. This prefix also prevents text beginning `!!`/`!#` from invoking
notice-panel control syntax. The final status notice replaces the summary on
the existing single-notice panel, matching its established presentation limit.

## Current mapping

`FieldChatSubmitPolicy.h` and `TabLabel.h` classify the nine commands, preserve
localized command precedence and implement bounded copying/palette fallback.
`TMFieldScene.cpp` retains the progress guard and sends unchanged envelopes.
`TMFieldSceneWorld.cpp` applies metadata via `TMHuman::SetTabLabel`, including
future spawns and already-visible actors; no new resource is required.

Server `commands.go` dispatches tab/SND before character lookup.
`chat_labels.go` validates state, text, level and flood quota. `playerCreatePacket`
centralizes tab retention at login, enter-view and CP refresh. Existing visibility
and private-instance isolation still control publication. The existing
`syncPlayerChaos` metadata publisher is reused; appearance still uses `0x36B`.

## Delta matrix

| Claim | Evidence | Adaptation | Classification |
| --- | --- | --- | --- |
| Tab submission and progress guard | Native `FUN_004662c5` | Retained route/guard | `PARIDADE_NATIVA` |
| Tab/SND authoritative semantics | W2PP server branches; current 7.48 contract tests | Handle commands rather than player lookup | `MODERNIZACAO_COMPATIVEL`; secondary server evidence |
| Nine colors and same-text changes | User-requested extension | New byte 327 and palette | `EXTENSAO_COORDENADA` |
| Bounded copies and safe status prefix | Current buffer/panel boundaries | Truncation, termination, control-prefix protection | `MODERNIZACAO_COMPATIVEL` |

## Decisions

Preserve the transport and in-place metadata lifecycle. No tab1 alias, server
chat broadcast, whisper autoreply or persisted setting is introduced. A command
without text clears its message; to recolor, submit the same text with another
tab command. Rate limits remain authoritative and private status is not exposed
in spawn packets.

The general incremental-update rule still applies to score, equipment, movement
and affects. No existing incremental packet carries the nickname; this narrow
command follows the reference's metadata publication through the already-present
CP metadata receiver instead of inventing a new opcode. Its existing-actor
branch does not apply snapshot coordinates or recreate the actor. Do not reuse
this exception for ordinary gameplay updates.

## Gaps

Real client: pending. Check owner/observer labels, all nine colors, clearing,
movement and buff continuity, private instances, SND lookup, death/re-entry,
AutoTrade title preservation and relogin. Native server source is unavailable;
W2PP's rules must not be promoted as independently proven 7.48 server parity.
Trade-specific colored label support is outside this contract.

## Validation

STATICALLY VERIFIED: native command references and focused read-only exports;
source routing, offsets, size asserts, metadata receiver and lifecycle.
AUTOMATED TESTED: focused Go tests passed for both command routes, visibility,
instance isolation, bounds, colors, SND lookup/non-autoreply and reset. Full
`go test -count=1 ./...` and `go vet ./...` passed. C++ pure command, copy and
palette checks passed; final executable build receipt is recorded in
`DOCS/current-state.md`. No CLIENT-TESTED promotion or publication is implied.
