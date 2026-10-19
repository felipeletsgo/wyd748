---
id: field-shortcut-selection-and-chat-scroll
title: Field keyboard shortcut dispatch and visible chat scrolling
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-18
---

## Question

Which native keyboard entries reach the field actions, how does Auto Run
consume its state, and which list receives PageUp/PageDown? The Helper
describes these actions, but its text alone is not proof.

## Evidence boundary

- USED: read-only `references/client748/WYD.exe`, SHA-256 above, and Ghidra
  project `tmproject/build/native-research/WYD748.gpr`.
- USED: native `FUN_00454763`, `FUN_0049abde`, `FUN_0040c0f0`, field constructor
  `FUN_004343a4`, destructor `FUN_004358da`, initializer `FUN_00435b13`, and
  character-selection routine `FUN_004528c5`.
- USED: current field input, skill grids, editable-text routing and UI2 atlas.
- USED on October 18: native accelerator resource 176, copied through Win32
  `LoadAccelerators`/`CopyAcceleratorTable`; native accelerator dispatcher
  `FUN_004541f3`, Auto Run toggle `FUN_00452737`, frame `FUN_004776c3`,
  blocked-cell predicate `FUN_004b3962` and route entry `FUN_0046218b`.
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

### Accelerator and Auto Run entry

The immutable executable's resource 176 has 56 accelerator entries. Flags 11
mean VIRTKEY/CONTROL/NOINVERT, 15 add SHIFT, and 18 mean ASCII/ALT/NOINVERT.
Ctrl+0..9 maps to 40064..40073; Ctrl+Shift+0..9 to 40074..40083;
Ctrl+numpad0..9 to 40104..40113. Ctrl letters dispatch through
`FUN_004541f3`; Q/W/E are 40096/40101/40088 inside its HP gate, while F
(40089) precedes that gate. Apostrophe, plus, minus, left bracket and right
bracket are Alt ASCII entries 40029/40047/40048/40027/40028. Native A/D/Home
entries reach no useful field action and are intentionally omitted. The
project's existing Ctrl-punctuation aliases remain compatible additions.

`FUN_004776c3` consumes field+0x200a0 (Auto Run) and +0x200a4 (reverse).
With a live player, its target is position plus camera vector X/Z times
human maximum speed (+0x220) times 1.5, negated for reverse. Read-only PE
VA translation confirms float 1.5 at 005a430c. It checks `FUN_004b3962`
before `FUN_0046218b`; the former rejects attribute 0x80 on the four-unit
map grid, and the latter enters the normal local-human route path.
The adaptation reuses `TMHuman::GetRoute` cadence, route validation and
transition cooldowns rather than adding a second movement protocol.

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
Auto Run also requires an attached human, ground and camera, valid map
coordinates, an allowed terrain cell, a ready route and no active movement
lock, AirMove, personal shop or modal message. Out-of-bounds input is rejected
before the legacy unchecked attribute lookup. Missing guild control 299 is
safe: the keyboard uses the acknowledged state without an optimistic toggle.

### Cleanup and teardown

No allocation, timer registration, new callback or pending input queue.
Existing scene destruction remains responsible for its controls.

### Shutdown

N/A: no new shutdown state or asynchronous work; normal scene teardown applies.

### Logout and relogin

No new persistence. Existing shortcut saving and field reconstruction apply;
this patch neither invents persistence nor changes transition timing.
The server affiliation flag defaults to false and resets with the character
session. Membership is never cleared or persisted by the toggle.

## Wire, ABI, and resources

No new opcode, structure layout or resource ID. Reuse current
assignment/selection and native field controls. UI2 belt highlight textures
199/200 are retained. Existing extended skill IDs/Sephira assignment stay
compatible project features rather than being removed for native parity.

Guild input uses the existing client-to-server `0x333` message: 108 bytes,
12-byte header followed by `char String[96]`, exact lowercase `guildon` or
`guildoff`. The server-to-client `0x3A4` acknowledgement is 16 bytes, with
signed 32-bit `Parm` at offset 12: 1 means disabled, 0 enabled. Existing packet
dispatch requires that full size before the receiver runs. The client updates
its enabled flag and button-selected polarity from that acknowledgement.
Native client strings are in `FUN_004662c5`; server 7.59 and W2PP are secondary
evidence for runtime affiliation masking, not authority for native 7.48 server
parity. Guild membership/management and PK mode must remain independent.

The server adaptation consumes only the two exact bare tokens before chat
relay and identifies the actor through the authenticated session, not a
forged packet ID. Entry, CreateMob and owner/observer score publications mask
guild ID/rank through a copied character projection. Transition publication
precedes the 0x3A4 acknowledgement; repeated requests are silent no-ops.
Actual membership remains available to guild authorization and existing
combat/war policy. No combat-policy change is claimed or implemented.
Current trade offer packets contain no guild affiliation; CreateMobTrade
already uses zero guild fields. The secondary parser's optional ignored
argument behavior is not adopted: this correction restores the client-issued
bare-token flow, not full secondary-server command parity.

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
now names the number row. IDs, six-digit color prefixes and loader format are
unchanged; the indexed legacy keyboard description retains its 11 rows.
`UI/etc.txt` and the main guide now explain both keyboard modes and item
shortcuts. The first Interface page shows list 868 rather than obsolete
F-key artwork. Both mouse diagrams and page navigation remain.

## Delta matrix

| Claim | Native evidence | Adaptation | Classification |
| --- | --- | --- | --- |
| Select with 1-0 instead of F1-F10 | Native F-key branch documented above; owner explicitly overrides controls | Remove F-key dispatch, preserve existing number-row selection/assignment and wire contract | MODERNIZACAO_COMPATIVEL; explicit control customization, not native key parity |
| Scroll visible message/chat list | FUN_00454763, 0x21/0x22; FUN_0040c0f0 | Correct both visibility guards | PARIDADE_NATIVA |
| Share safe selection implementation | Existing character contract | Extract without changing its behavior | MODERNIZACAO_COMPATIVEL |
| Restore useful alternate keyboard entries and Q/W/E/F dispatch | Resource 176 and FUN_004541f3 | Load the executable table and decode the command low word | PARIDADE_NATIVA |
| Consume Auto Run and reject unsafe requests | FUN_004776c3 camera projection and ordinary route boundary | Reuse current routing, cadence and safe lifecycle gates | MODERNIZACAO_COMPATIVEL |
| Confirm guild toggle state | Existing 0x333/0x3A4 client ABI | Remove optimistic state mutation and initialize/bind the optional button | MODERNIZACAO_COMPATIVEL |
| Restore public guild toggle handling | Existing 0x333/0x3A4 client ABI, authenticated sessions | Runtime-only public ID/rank projection and transition acknowledgement, without changing membership or combat policy | MODERNIZACAO_COMPATIVEL |

## Decisions

Apply the owner's number-row control choice rather than keeping the native
F-key aliases. Preserve focused-edit precedence, F11, numpad semantics,
A/D C.C. and F consumable ownership. No new client/server protocol or
capability is needed. The October 18 extension restores the useful native
accelerator entries after checking the actual resource, not guessing from
later source. Keep the source Ctrl-punctuation aliases and safe native-edit
focus behavior as explicit compatible modernization. Unrelated advertised
commands are not validated by this batch.

## Gaps

Real-client execution is pending for both belts, hovered assignment, keyboard
modes, focused chat and message-panel scrolling. This record is not
`CLIENT_TESTED`. Other Helper bindings remain tracked in the UI audit.
The obsolete keyboard artwork is retained as an asset but no longer displayed
on the keyboard page. Help text now matches this batch; actual tab geometry,
mouse navigation, alternate-mode punctuation on the player's keyboard layout,
Auto Run steering/obstacles/transitions and guild owner/observer/relogin need
execution with the matching candidate and server. Slash-command Help remains
a separate audit, as recorded in `DOCS/client/ui-windows.md`.

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

Historical October 17 `Build-Client.ps1 -NoDeploy` exited zero, Release/x86. Candidate:
`tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`D6CC42D093FE240127F5096FA133965C225AB410D133AC79B8A6094AD8A7FA8D`.
Installed `tmproject/client748/WYD.exe` remains
`1FE4CBA89C18847C3E4C35CC9497B049B9E01DCAB12E58B36152AE7FBFE19F6A`.
No installation, launch, publication, restart, commit or push. Real client:
not run. Source changes and a candidate are not `CLIENT_TESTED` evidence.

October 18: `FieldShortcutContractTests` adds resource, dispatcher, native
alive-gate, Auto Run wiring, guild acknowledgement, Help-page and text-format
checks. Pure policy checks cover forward/reverse projection, lifecycle gates,
bounds and blocked terrain. Architecture: 3,282,308 checks; socket: 281;
crash capture: 198; launcher: 56; all passed. `Build-Client.ps1 -NoDeploy`
exited zero, Release/x86, without emitted compiler warnings. New candidate
`tmproject/build/TMProject748/Release/WYD.exe` SHA-256:
`B20510B20D34651D206C72340B96D2B7314D56B19072AC25678D37183889DE3D`.
Installed EXE SHA-256:
`251CF754409F8AE4C0251C9FDD88EDA14C6799E78A377BD0665B6E858FDAFAB0`.
Help text changes are present in the local asset directory, but no EXE copy,
game execution or publication occurred. These are automated and build
results, not `CLIENT_TESTED`.

Win32 inspection of the built PE resource 109 confirmed 55 accelerator entries,
including Ctrl+Q/W/E/F and Alt+right-bracket with the expected flags. The
candidate contains the resource; this does not establish runtime dispatch.
The repository-layout check exits 1 for pre-existing artifacts: the crash
audit under `tmproject/client748/dumps/`, `tmproject/TMProject748/build/`,
and `tmproject/client748/dumps/crash-monitor-p31348.log`. They were preserved.
Cleanup of this batch's exact temporary directory
`tmproject/build/tests/Release/crash-capture-tests-13484-713942328/` was blocked
by execution policy. No alternative deletion was attempted; the test output
remains. Current incremental build outputs are intentionally retained.

Server validation: `go test ./internal/game -run 'TestGuildDisable' -count=1`
passed six focused tests for masking, authenticated identity, retained guild
authorization, no chat leak, observer updates, transition-only ACKs,
reset/reconnect defaults and exact tokens. Integration gate
`go test ./internal/game ./internal/wire` exited zero: game passed in 20.989 s;
wire passed from cache. No standalone server candidate, deployment, restart
or gameplay execution was performed.
