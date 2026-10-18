---
id: field-shortcut-selection-and-chat-scroll
title: Field skill shortcut controls and visible chat scrolling
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-17
---

## Question

Do F1-F10 select skill shortcuts in 7.48, and which list receives PageUp and
PageDown? The Helper describes these actions, but its text alone is not proof.

## Evidence boundary

- USED: read-only `references/client748/WYD.exe`, SHA-256 above, and Ghidra
  project `tmproject/build/native-research/WYD748.gpr`.
- USED: native `FUN_00454763`, `FUN_0049abde`, `FUN_0040c0f0`, field constructor
  `FUN_004343a4`, destructor `FUN_004358da`, initializer `FUN_00435b13`, and
  character-selection routine `FUN_004528c5`.
- USED: current field input, skill grids, editable-text routing and UI2 atlas.
- N/A: later TMProject sources and external guides; native evidence resolves
  these branches. The Helper is supporting context, not parity authority.
- CONTRADICTORY: the current handler's hidden-panel/hidden-list guards are the
  reverse of the native visibility tests.

## Native 7.48 flow

### Callers

`FUN_004343a4` installs vptr `005a4294` on the field object. Its keydown slot
at `005a42a0` (vtable +0x0c) points to `FUN_00454763`, confirmed by Ghidra
data references and the pointer-search result. This is virtual input dispatch,
not an unused function without direct callers. `FUN_0049abde` first offers
keydown to the active scene's control container through its +0x0c slot and
returns consumed input before field shortcuts run.

### Callees

`FUN_00454763` calls base keydown `FUN_0049abde`, `GetKeyState(VK_SHIFT)`,
assignment `FUN_0044e494`, selected-magic update `FUN_004431d9`, and skill
damage calculation `FUN_00557aff`. Visibility getter `FUN_0040c0f0` reads
receiver +0x28. Scrollbar virtual +0x90/+0x94 receives Up/Down respectively.

For keys 0x70..0x79, selected shortcut is key - 0x70, plus ten when the
second belt is visible. If the skills panel is visible, the shared 500 ms
key-time guard runs first; Shift assigns the hovered skill and refreshes
that timestamp. Selection runs afterwards, even without Shift. The active
belt highlights the selected item, then selected magic and both damage
labels are refreshed. This keydown branch is not the character/accelerator
keyboard-mode branch and does not check player HP.

PageUp (0x21) and PageDown (0x22) choose the message list only when the
message panel is visible, otherwise the chat list. They scroll only when the
chosen list is visible and owns a scrollbar, then consume the key.
Receiver instructions at 00455812/00455818 and 00455851/00455857 establish
the panel/list identity; both returns are compared with 1. The matching
PageDown instructions are 004558c3/004558c9 and 00455902/00455908.

## State and lifecycle

### Observable entry

Field keydown after existing transition and focused-control guards.

### Transition matrix

| Entry | Condition | Result |
| --- | --- | --- |
| F1-F10 | First/second belt | Select index 0-9/10-19 and refresh selected skill |
| Shift+F1-F10 | Skill panel visible, hovered skill, cooldown elapsed | Assign and select the matching active-belt slot |
| F1-F10 | Skill panel visible, key cooldown active | Consume without selection or assignment |
| PageUp/PageDown | Message panel visible | Scroll the visible message list |
| PageUp/PageDown | Message panel hidden or absent | Scroll the visible chat list |
| Any key | Focused control consumes it | Do not run the field shortcut |

### Vtables, vptrs, and receivers

Field vptr 005a4294, keydown slot +0x0c at 005a42a0. Native destructor
`FUN_004358da` reinstalls the same field vptr before teardown. Skill panel
is scene+0x27c84, belts +0x28718/+0x2871c, message panel +0x27c88,
message list +0x27c8c and default chat list +0x27ae8. Getter +0x28 is
visibility, not an inverted hidden-state predicate.

### Ownership

Scene/control-container ownership is unchanged. Shortcut selection modifies
ObjectManager selected index (+0xef5), not the item inventory. Assignment uses
the existing shortcut mutation path. No new controls or owned objects.

### Partial failure

Missing list or scrollbar consumes the scroll key without dereferencing it.
The adaptation also handles missing belt grids/ObjectManager/local human
safely; it does not manufacture an inventory-page fallback.

### Cleanup and teardown

No allocation, timer registration, new callback or pending input queue.
Existing scene destruction remains responsible for its controls.

### Shutdown

N/A: no new shutdown state or asynchronous work; normal scene teardown applies.

### Logout and relogin

No new persistence. Existing shortcut saving and field reconstruction apply;
this patch neither invents persistence nor changes transition timing.

## Wire, ABI, and resources

No new opcode, structure layout, resource ID or server behavior. Reuse current
assignment/selection and native field controls. UI2 belt highlight textures
199/200 are retained. Existing extended skill IDs/Sephira assignment stay
compatible project features rather than being removed for native parity.

## Current mapping

The owner explicitly chose 1-0 instead of F1-F10 on 2026-10-17. Consequently,
`TMFieldSceneInput.cpp::OnKeyDownEvent` owns chat scrolling but no longer
selects or assigns skills with F1-F10. This does not change the native facts
above. `OnKeySkill` implements shifted-number hover assignment;
`OnKeyShortSkill` decodes the number row (0 is the tenth slot), retaining
its existing keyboard-mode, alive-state and numpad guards. Shared
`SelectShortSkill` updates the active bank, highlight, selected magic and
HUD/Character damage labels. Focused controls still consume text first.

The existing `LoadMsgText` bindings materialize `UI/interface.txt` in list
868 and `UI/interface1.txt` in list 6074. Their selection/assignment prose
now names the number row. IDs, six-digit color prefixes, row counts and
loader format are unchanged. The old keyboard-page artwork and legacy
`UI/etc.txt` prose are not changed in this batch.

## Delta matrix

| Claim | Native evidence | Adaptation | Classification |
| --- | --- | --- | --- |
| Select with 1-0 instead of F1-F10 | Native F-key branch documented above; owner explicitly overrides controls | Remove F-key dispatch, preserve existing number-row selection/assignment and wire contract | MODERNIZACAO_COMPATIVEL; explicit control customization, not native key parity |
| Scroll visible message/chat list | FUN_00454763, 0x21/0x22; FUN_0040c0f0 | Correct both visibility guards | PARIDADE_NATIVA |
| Share safe selection implementation | Existing character contract | Extract without changing its behavior | MODERNIZACAO_COMPATIVEL |

## Decisions

Apply the owner's number-row control choice rather than keeping the native
F-key aliases. Preserve focused-edit precedence, F11, numpad semantics,
A/D C.C. and F consumable ownership. No new client/server protocol or
capability is needed. Do not create an accelerator table or claim other
advertised commands are verified.

## Gaps

Real-client execution is pending for both belts, hovered assignment, keyboard
modes, focused chat and message-panel scrolling. This record is not
`CLIENT_TESTED`. Other Helper bindings remain tracked in the UI audit.
Keyboard-page artwork and `UI/etc.txt` still advertise F-key aliases and
need a separate content/asset correction; the number-row page descriptions
explicitly state the current selection binding.

## Validation

Ghidra `ExportWydFlow.java` runs used `-readOnly -noanalysis -process WYD.exe`:
entry/instructions/pointers for 00454763, entry/instructions for 0049abde and
0040c0f0, and exact slot 005a42a0. Both runs exited zero, emitted the matching
program hash and requested search summaries, and contained no script errors.
Constructor/destructor exports confirm the concrete receiver vptr.
Source-contract checks now cover the explicit number-row choice, removal
of F-key selection/assignment, focused-input precedence, assignment guards,
bank selection and native visible-list scrolling. They are regression
coverage, not proof of keyboard/gameplay execution. Existing UI2 selection
tests target the shared helper. Architecture 1,240,477 checks, socket 231,
crash capture 83 and launcher 56 passed.

`Build-Client.ps1 -NoDeploy` exited zero, Release/x86. Candidate:
`tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`D6CC42D093FE240127F5096FA133965C225AB410D133AC79B8A6094AD8A7FA8D`.
Installed `tmproject/client748/WYD.exe` remains
`1FE4CBA89C18847C3E4C35CC9497B049B9E01DCAB12E58B36152AE7FBFE19F6A`.
No installation, launch, publication, restart, commit or push. Real client:
not run. Source changes and a candidate are not `CLIENT_TESTED` evidence.
