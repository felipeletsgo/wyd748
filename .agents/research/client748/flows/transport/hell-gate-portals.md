---
id: hell-gate-portals
title: Hell Gate portal prompt and confirmation boundary
subsystem: transport-session
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-17
---

# Hell Gate portal prompt and confirmation boundary

## Question

What bounded client behavior is proven for a 7.48 portal tile whose normalized
coordinate has no entry in the client portal table, and what must remain
unchanged when the player confirms the prompt? This record covers the prompt
selection and confirmation packet boundary only. It does not claim parity for
the complete Hell Gate encounter, admission rules, destination, rewards, or
server persistence.

## Evidence boundary

- Native export: `build/hell-gate/portal-native.tsv`.
- Native image SHA-256: `8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
- Native prompt decompilation: `tmproject/build/native-research/decompiled/FUN_004776c3.c`.
- Native confirmation decompilation: `tmproject/build/native-research/decompiled/FUN_004640e5.c`.
- Current prompt adaptation: `tmproject/TMProject748/internal/app/scenes/TMFieldSceneMovement.cpp`, `TMFieldScene::UpdateTeleportPrompt`.
- Current confirmation callback: `tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp`, message case `16`.

The supplied native export and decompilations are unchanged evidence for this
bounded correction. No Ghidra rerun was required. Server behavior is owned by
the parent work and is outside this client change.

## Native 7.48 flow

`FUN_004776c3` at `0x004776c3` samples the current field attribute, aligns the
player position to the four-unit portal grid, and searches the native portal
table. While the player is standing on attribute `0x10`, the function latches
the local prompt state and selects the prompt from the normalized portal index:

- A negative portal index selects `g_pMessageStringTable[43]`, the generic
  prompt. It does not use a portal-table target or price.
- A known portal index formats `g_pMessageStringTable[208]` with the portal
  target and may format the price with row `207` under the existing price rules.
- Both prompt paths use local message ID `16` for the confirmation dialog.

The prompt is closed and the local latch is rearmed when the player leaves
attribute `0x10`. The native function's surrounding movement and special-panel
branches remain outside this bounded prompt correction.

### Callers

The native export records an incoming reference from `0x005A42B4` to
`FUN_004776c3`. In the current client, the field update path calls
`TMFieldScene::UpdateTeleportPrompt` from `TMFieldScene.cpp` in the regular
field-frame update.

### Callees

The native prompt path reaches the message-box implementation through the
scene UI vtable. The current client maps the same local prompt lifecycle to
`m_pMessageBox->SetMessage` and `SetVisible`.

The native confirmation branch is `FUN_004640e5` at `0x004640e5`. For message
ID `16` it clears a 16-byte buffer, writes packet type `0x290`, leaves the
reserved field zero, and sends the ordinary 16-byte request through
`FUN_0055f2dd`. No destination, target index, price, reward, or other Hell
Gate state is carried in this confirmation frame.

## State and lifecycle

| State | Proven behavior | Client mapping |
| --- | --- | --- |
| Enter portal tile | A standing player with attribute `0x10` and no prompt latch receives a prompt. | `UpdateTeleportPrompt` gates on standing state, attribute `0x10`, and `m_bTeleportMsg`. |
| Known normalized index | Use the named prompt and existing optional-price branch. | Row `208` is formatted only after a bounded portal-index check; row `207` is read only in the same valid-index branch. |
| Unknown normalized index | Use the generic prompt and do not dereference a portal-table row. | Row `43` is selected for `nPosIndex < 0`; `m_nLastPotal` records `-1`; the old ignore-and-latch branch is removed. |
| Previous portal comparison | Compare table coordinates only when both current and previous indices are valid. | `hasPortalIndex && hasLastPortal` guards both table dereferences. |
| Confirm message | Message ID `16` sends the ordinary 16-byte `0x290` request. | Existing `TMFieldScene.cpp` case `16` and `MSG_ReqTeleport` are unchanged. |
| Leave portal tile | Clear the prompt latch and hide the message-16 dialog. | Existing attribute-clear path remains in place. |

## Wire, ABI, and resources

The client-to-server confirmation remains a 16-byte `0x290` frame. The client
does not add a destination, portal index, price, reward, or new field to this
request. The message ID `16` is local UI state used to dispatch confirmation;
it is not changed into a new wire opcode.

The correction reads existing message-table rows `43`, `207`, `208`, `160`,
and `258` through bounds-checked access. It does not change resource IDs,
portal-table layout, ABI, packet size, or server authority.

## Current mapping

Before this correction, `UpdateTeleportPrompt` treated a negative normalized
portal index as ignored input and set `m_bTeleportMsg`, preventing the generic
Hell Gate prompt. The same function could also evaluate the previous-portal
comparison with a valid previous index and an invalid current index, which
would dereference `g_TeleportTable[-1]`.

The bounded adaptation now records the normalized index, selects row `43` for
an unknown index, selects row `208` only for a valid index, and reads row `207`
only when the valid-index price branch is active. Message-table reads for the
special-panel rows are also guarded. The last-portal comparison and every
portal-table access in this function are covered by valid-index checks. The
confirmation callback and wire layout are unchanged.

## Delta matrix

| Claim | Native 7.48 evidence | Current mapping | Decision |
| --- | --- | --- | --- |
| Unknown normalized index shows a prompt | `FUN_004776c3` records the negative index and passes generic row `43`. | `m_nLastPotal` records `-1`, `pPrompt` defaults to `messageAt(43)`, and no portal-table row is read. | Implement bounded `PARIDADE_NATIVA`. |
| Known normalized index uses a named prompt | `FUN_004776c3` formats row `208` with the known target. | Row `208` and `szTarget` are used only after `hasPortalIndex`. | Preserve native named-prompt behavior. |
| Optional price is not part of the unknown-index path | Native generic branch does not index the portal table or format a price. | Price calculation and row `207` access remain inside `hasPortalIndex`. | Preserve the generic prompt boundary. |
| Previous portal comparison cannot use an invalid current index | Native flow distinguishes the negative index before named-table access. | `hasPortalIndex && hasLastPortal` guards current and previous table reads. | Add compatibility-safe bounds guards. |
| Confirmation uses the existing ordinary request | `FUN_004640e5` sends a zero-reserved 16-byte `0x290` frame for message `16`. | Existing `MSG_ReqTeleport` send path is untouched. | Keep the wire contract unchanged. |

## Decisions

- Classify the prompt selection and local message-16 confirmation boundary as
  `PARIDADE_NATIVA`; this classification does not cover the entire Hell Gate
  encounter.
- Treat the added table and format guards as
  `MODERNIZACAO_COMPATIVEL`; they prevent invalid reads without changing the
  native prompt or wire contract.
- Keep the server authoritative for admission, destination, rewards, and
  persistence. No client-side destination or outcome was invented.
- Reuse the supplied export and decompilations. No Ghidra rerun is required for
  unchanged evidence and this bounded input transition.

## Gaps

Server admission and persistence, the destination selected after confirmation,
rewards, and complete Hell Gate encounter lifecycle remain outside this record.
The client was not installed, launched, or exercised in gameplay for this
change, so this record remains `CONTRACT` and does not claim `CLIENT_TESTED`.

Subsequent publication delivered this exact candidate through patch 10034,
paired with active server release `20261017T221343Z` after authorized restart.
Public ZIP/executable hashes and active server identity/readiness passed. See
[the publication receipt](../../../../../DOCS/current-state.md#active-release--2026-10-17-hell-gate-route-and-defense-patch-10034).
Publication does not replace the pending client/gameplay checks above.

## Validation

- `git diff --check -- tmproject/TMProject748/internal/app/scenes/TMFieldSceneMovement.cpp` — passed; Git reported only its normal LF-to-CRLF working-copy warning.
- `C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe tmproject/TMProject748/TMProject748.vcxproj /t:ClCompile /p:Configuration=Release /p:Platform=Win32 /p:BuildProjectReferences=false /m /nologo /v:minimal` — passed with exit code `0`; compile-only, no link, install, deploy, or client launch. The initial object refresh emitted the existing DirectX-header warning `C5298` for a spurious `U+1A` character; the final incremental run rebuilt `TMFieldSceneMovement.cpp` and passed without a new warning.
- `python -B .agents/skills/wyd-client748-research/scripts/validate_research.py --repo C:\Users\xereca\Documents\PROJETOS\wyd-go` — the new record reported `OK`, while the repository-wide run reported pre-existing missing sections in `flows/combat/persistent-buff-visuals.md` and `flows/ui/instance-runtime-counter-controls.md`; those unrelated records were not changed.
- Focused gameplay validation — not run. No client installation was changed and no running client was stopped.
- Full candidate build: `pwsh -NoProfile -File tmproject/Build-Client.ps1 -NoDeploy` passed with exit code `0`; log `build/hell-gate/client-build.log`. Linked candidate `tmproject/build/TMProject748/Release/WYD.exe` has SHA-256 `4183973219F67EC0F22D29276E42541DD9793FA1EB0A54BADFF7919938F60C41`. The final incremental build reported no new warning. No install, launch, publication or restart followed; this is build evidence only.
- Cleanup limitation: the earlier direct compile created an obsolete duplicate `tmproject/TMProject748/build/` tree (177 task-owned files). After the canonical build succeeded, execution policy rejected removal of that exact duplicate. No alternate deletion was attempted; the sanctioned incremental output is unchanged. Repository layout validation therefore still reports this path, independently of the successful source and candidate checks.
