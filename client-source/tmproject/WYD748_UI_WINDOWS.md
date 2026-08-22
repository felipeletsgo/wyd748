# WYD 7.48 field-window catalog

This catalog records top-level windows found in the original
`UI/FieldScene2.bin`. It prevents compatibility work from hiding controls by
appearance or by IDs copied from the newer TMProject resource.

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

Whenever another unknown panel appears, capture a fresh compatibility startup
log and register its root ID, captions and intended trigger here before adding
it to the startup-hidden list.

The compatibility bootstrap must also bind controls that native input code
consults while they are closed. A 2026-08-20 dump proved two such requirements:
the teleport list (`12545`) and auto-trade root (`646`). Visibility controls
startup state; a null pointer is not an acceptable substitute for a closed
window.
