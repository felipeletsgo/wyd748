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
| 6110 | Combine | Advanced composition variant. |
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

## Verification rule

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
