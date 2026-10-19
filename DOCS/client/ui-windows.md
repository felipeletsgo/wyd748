# WYD 7.48 field-window catalog

This catalog records top-level windows found in the historical 7.48
`UI/FieldScene2.bin`. It prevents the source adaptation from hiding controls by
appearance or by IDs copied from the newer TMProject resource. It does not
authorize a second-version compatibility path.

The source-built client also writes a live `UI window ...` record to
`client-debug.log` for every top-level control, including its current caption,
visibility and geometry. Editable text is deliberately excluded so accounts,
passwords and chat are never written by this diagnostic.

## Windows that must start closed

| Root ID | Window | Notes |
|---:|---|---|
| 258 | Item description | Opens only while inspecting an item. |
| 289 | Combined-server selector | Auxiliary server/channel selector; it is not part of the in-world HUD. |
| 320 | Quest | Quest browser and category tabs. |
| 332 | New quest notification | Opened only when the quest-notification state enables it. |
| 257 | Inventory/equipment | Opened by the inventory command. |
| 513 | Character | Character stats and mastery summary. |
| 576 | Trade | Two-player trade panel. |
| 626 | Gold amount | Modal amount prompt used by economic operations. |
| 632 | System menu | Change server, character or leave game. |
| 640 | Interaction menu | Party, guild, trade and challenge actions. |
| 646 | Auto Trade | Personal merchant configuration. |
| 669 | Notices | Notice reader. |
| 819 | Guild Board | Guild notice administration/editor. Its controls include `Save Notice`, `Editor`, `Erase`, `Check Bills` and `Reply`; this is the previously undocumented administration-looking window seen during compatibility testing. |
| 864 | Helper/messages | Help, memo, command and message tabs. |
| 875 | Message arrived | Notification button for the helper/message system. |
| 878 | Summon request | Notification button for an incoming summon request. |
| 880 | Quiz | Event quiz panel. |
| 1360 | Combine | Item composition panel. |
| 1793 | Shop | NPC merchant inventory. |
| 1825 | Cargo | Warehouse. |
| 1857 | Party | Legacy party-member window. |
| 1889 | Skill mastery | Mastery purchase/training panel. |
| 1905 | Skill list | Learned-skill window. |
| 2048 | Lottery | Lottery purchase window. |
| 5749 | Party/member status | Inactive member rows labelled `NONE`. |
| 6110 | Aylin Combine | Advanced composition with four result-preview grids (6129-6132). |
| 6145 | Combine | Advanced composition variant. |
| 6185 | Special store | Event/special item purchase panel. |
| 6400 | Jackpot | Jackpot/gamble controls. |
| 6432 | Second-job composition | Second-job weapon composition. |
| 6481 | Combine | Advanced composition variant. |
| 6512 | Restore System | Item restoration panel. |
| 8705 | Emotes | Smile/heart/star selector. |
| 8961 | Toto/game time | Game-number and timed purchase panel. |
| 12288 | Server/channel selection | Must not remain open after entering the world. |
| 12544 | Coordinate teleport | Internal coordinate teleport dialog. |

## Persistent field HUD roots

These controls belong to the normal 7.48 field HUD and must not be hidden by
the compatibility bootstrap:

| Root ID | Purpose |
|---:|---|
| 5723 | Character name, level and HP/MP status header. |
| 5716 | Bottom status/shortcut region. |
| 5744 | Main-menu button. |
| 5745 | C.C./macro status controls. |
| 292 | Character, inventory, skill, quest and system shortcut buttons. |
| 5377 | Chat message list. |
| 5739 / 6134 / 6135 / 12624 | Chat input, backing controls and channel tabs. |

## Aylin preview and refinement presentation

The four result grids contain independent, presentation-only copies of the
staged items' possible additional-effect combinations. They refresh when
ingredients change and clear on removal, closing, or snapshot reset. The first
gem in slot 3 selects the refinement variant; previews do not determine the
server's composition result or mutate inventory.

UI and world rendering share the native gem texture selection, including the
existing imported weapon path. Diamond, Emerald, Coral, and Garnet correspond
to drop, piercing, EXP, and absorption. No server encoding or recipe changed.
The special fixed glow for mesh IDs 2906-2909 remains unchanged.

The native evidence, tests, candidate identity, and pending in-game scenarios
are recorded in [Aylin refinement presentation](../../.agents/research/client748/flows/ui/aylin-refinement-presentation.md).
Automated checks and an incremental Release build passed on 2026-10-14; this
batch was not installed locally, published, or tested in the running client.

## Verification rule

### Helper shortcut audit and corrections — 2026-10-18

This is a source audit, not current-client gameplay validation. The Helper
screenshots are not sufficient authority to replace existing C.C. bindings.
`EventTranslator` dispatches input; actual field bindings live in
`internal/app/scenes/TMFieldSceneInput.cpp` and `TMFieldScene::OnCharEvent`.
Unless noted otherwise, the character routes below require `g_nKeyType == 0`
and no focused edit consuming input. Alive-state, transition and panel guards
remain relevant. The owner subsequently chose 1-0 instead of F1-F10 for
skill selection. Visible-list scrolling is restored without changing
C.C./consumable ownership.

| Advertised shortcut | Current source result |
| --- | --- |
| F1–F10; Shift+F1–F10 | Disabled for skill selection/assignment by explicit owner choice on 2026-10-17. Native `FUN_00454763` supports these keys, but the current project deliberately uses the number row instead. F11 is unchanged. |
| 1–0; Shift+1–0 | Current skill controls: `OnKeyShortSkill` selects slots 0-9 in the visible bank, with 0 selecting the tenth; `OnKeySkill` assigns through shifted symbols, subject to cursor/skill-panel context. Shared selection refreshes highlight, selected magic and both damage labels. Focused text input keeps priority. Not runtime-tested. |
| Minus | `OnKeyDash` cycles automatic skill selection only while autoattack is enabled. |
| Q/W/E; V; F11 | Potion, mount feed and recall-item handlers are bound. F11 searches the native 9x7 Carry grid for volatile effect 11. Actual inventory/server results are unverified. |
| C/I/S; P; M | Character, inventory, skills, party and minimap handlers are present. |
| A/D | These toggle current C.C. modes, rather than the Helper's alternate minimap/party actions. Preserve them pending evidence and an explicit conflict decision. |
| R; T; N; Tab | Run, autoattack, names and camera handlers are present; Toto focus can consume Tab first. |
| Grave | No grave-character PK binding found; `OnKeyPK` currently accepts K/k. |
| Plus | `OnKeyPlus` resizes chat. Its down-arrow scale correction is described in the [UI2 scrollbar record](port-748.md#ui2-vertical-scrollbar-2026-10-13). |
| PageUp/PageDown | Corrected from native `FUN_00454763` and visibility getter `FUN_0040c0f0`: scroll the message list when its panel is visible, otherwise chat; scroll only a visible list with a scrollbar. Focused editable controls keep priority. Real-client execution pending. |
| Up/Down while editing | Editable-text history events are connected to field chat history. |
| F | Invokes the existing equipped-item consumable shortcut, not the Helper's guild-chat action. Do not overwrite that recorded native flow. |
| H/Z/X; Enter | Help, skill-page switching, quests and chat handlers are present. |
| Right bracket | Auto Run now consumes its flag in `FrameMove`, projects camera X/Z by maximum speed times 1.5, and uses the existing route/packet path. Bounds, blocked cells, death, AirMove, movement locks and modal/shop guards prevent invalid requests. The original map restrictions remain. |
| Apostrophe | Sends `guildoff` or `guildon` from the last acknowledged state. The 16-byte `0x3A4` response updates both the keyboard state and control 299, where present. This is not recruitment, guild departure, or PK mode. |
| PrintScreen | The window handles `VK_SNAPSHOT` with `CaptureScreen`; no screenshot was captured in this batch. |
| Ctrl: see loot | Not verified. Absence of Ctrl in item-pickup packets does not establish a failure of the separate label-visibility behavior. |

Alternate keyboard mode now loads the executable's accelerator resource and
extracts `LOWORD(WM_COMMAND)` correctly. Native resource 176 proves Ctrl for
letters, numbers and numpad, Ctrl+Shift for assignment, and Alt for ASCII
punctuation. The source retains its Ctrl-punctuation aliases in mode 1.
Q/W/E and F dispatch were restored without changing their native alive gates.
Native edit focus preserves cut/copy/paste and suppresses gameplay dispatch.
Mode 0 retains unmodified character controls; A/D remain C.C. controls there.
Native A/D/Home accelerator no-ops are not advertised or installed.

The server now consumes exact lowercase `guildon`/`guildoff` tokens from
authenticated local-chat sessions before relay. It masks guild ID/rank in
entry, creation and score publications, refreshes visible observers, and
acknowledges state transitions with `0x3A4`. Repeated requests are silent.
Character reset clears the runtime-only flag; actual guild membership and
management permissions remain unchanged. Existing combat/war policy remains
authoritative and unchanged: this public-affiliation adaptation is not a
claim of full native server guild-policy parity. Current trade packets carry
no affiliation fields requiring a new mask. Focused guild tests passed;
`go test ./internal/game ./internal/wire` also passed (wire result cached).

`UI/interface.txt`, `UI/interface1.txt` and `UI/etc.txt` now document the current
number-row, modifier, consumable, movement, guild and item-click controls.
The first Interface page uses the scrollable guide in list 868 instead of
the stale F-key artwork; the two mouse diagrams, resource IDs and navigation
are retained. No image binary was modified. Text rows keep their six-digit
color prefixes. Blank instructions and recipe placeholders were removed,
not replaced with invented economic rules.

The October 18 candidate passed 3,282,308 architecture checks, 281 socket
checks, 198 crash-capture checks and 56 launcher checks. Release/x86 build
completed without errors or emitted compiler warnings. Candidate SHA-256:
`B20510B20D34651D206C72340B96D2B7314D56B19072AC25678D37183889DE3D`.
Installed `tmproject/client748/WYD.exe` remains
`251CF754409F8AE4C0251C9FDD88EDA14C6799E78A377BD0665B6E858FDAFAB0`.
Help text assets are edited in that local runtime directory, but the new EXE
is only a build candidate. No install, game launch, publication or restart.

Publication follow-up (2026-10-19): the same candidate and three help-text
assets were published as patch 10036 after the current release build checks
passed. The launcher feed preserves prior versions. An independent public
download matches ZIP SHA-256
`E4F2940353D425B0C6DEE0B7F3FC4C9F8A24305D1950EB03D6ED323DA2F4B49B`.
No local executable was replaced or launched; actual in-game validation
remains pending. See the current deployment receipt in
[current state](../current-state.md).

The updated shortcut evidence record passes the research validator. The
global validation still exits 1 for missing sections in the unchanged
`persistent-buff-visuals.md` and `instance-runtime-counter-controls.md`
records. Full diagnostics are retained in
`build/shortcut-corrections/research-validation.log`. Repository layout also
remains blocked by pre-existing misplaced artifacts; this batch did not
remove them. Execution policy blocked cleanup of the newly generated
`tmproject/build/tests/Release/crash-capture-tests-13484-713942328/`
test directory. Neither limitation invalidates the recorded build/test
results, and neither is a clean global release gate.

The command page also needs an end-to-end audit. The inspected server routes
`=` to party, `-` to guild and `--` to global/channel chat; the Helper labels
`--` as allies. The client/server recognize kingdom/king paths, but `/r`,
`*`, `/Town`, guild tax/summoning/teleport and war/alliance-ending commands
were not established end to end. Expulsion is table-driven in the client and
has a server alias; exact resource-key agreement remains unverified. These
findings do not authorize inventing server commands or changing guild policy.

The native consumable ownership is separately recorded in
[equipped-item matched shortcut](../../.agents/research/client748/flows/ui/equipped-item-matched-consumable-shortcut.md).
Function-key selection and scrolling evidence is recorded in
[field shortcuts and visible chat](../../.agents/research/client748/flows/ui/field-shortcut-selection-and-chat-scroll.md).
The full input matrix needs actual execution with chat focused/unfocused,
alive/dead state, both keyboard modes and each listed window before it can
be marked `CLIENT_TESTED`.

Whenever another unknown panel appears, capture a fresh `project.exe` startup
log and register its root ID, captions and intended trigger here before adding
it to the startup-hidden list.

The 7.48 bootstrap must also bind controls that native input code
consults while they are closed. A 2026-08-20 dump proved two such requirements:
the teleport list (`12545`) and auto-trade root (`646`). Visibility controls
startup state; a null pointer is not an acceptable substitute for a closed
window.

The AirMove path also needs the actual `FieldScene2.bin` text controls: title
`12549` and columns `12550`–`12552`. The imported initializer instead binds
`12551/12552/12553/12560`, of which the last two are absent in the 7.48
control-tree dump. The compact bootstrap now leaves the
panel closed when its list, main text, or route labels cannot be populated;
closing the panel remains possible even if the list is unavailable. This is
a compatibility guard, not proof that the transport flow is client-tested.
The five route labels are copied into their existing 18-character display
fields with a fixed format; resource strings cannot become `printf` formats.
During AirMove, the client consumes each pending displacement once before the
next frame; this prevents the visual flight from reapplying accumulated motion
and preserves the 7.48 start/end packet contract.
On landing, the client restores the pre-flight mount mesh type and look before
rebuilding the mount; the temporary flight mesh no longer overwrites the body
mesh or persists as the player's mount. This still needs an in-game check with
a real transport NPC and a mounted character.
The visual route advances only when the next waypoint exists and is not the
`(0,0)` terminator; a fully populated 10-slot route cannot read past its end.
The server still decides the destination.
