---
id: ground-pickup-mouse
title: Mouse clicks on ground loot under actors
subsystem: ui
status: TRACED
native_sha256: 1FA945EB589B6CD8B2E1EA1D1B1AE83E5BDE66B5251344B4AC98B025489125C1
updated: 2026-11-09
---

# Mouse clicks on ground loot under actors

## Question

How do left and right clicks reach ground loot when an actor also has hover?

## Evidence boundary

- USED: the stock PT 7.48 executable with the hash above. Inspect its field
  click branches and pickup sender in Ghidra and the disassembly.
- USED: current field input, actor and item hit tests, item removal, and
  the WYD-Go pickup request and persistence handlers.
- NOT APPLICABLE: 7.59 server templates, new UI assets, and new wire layouts.

## Native 7.48 flow

### Callers

The field mouse handler `0x0045F6C0` dispatches left-button down to physical
interaction at `0x00460424`. The no-actor branch calls the ground pickup
helper at `0x0045EEE8`. The plain right-button down branch calls the skill
handler at `0x004602D6`. It skips the skill dispatch for Ctrl.

The right-button path can call the same pickup helper without requiring a
clear actor hover. For example, the invalid skill branch calls it at
`0x00456781` and the passive skill branch calls it at `0x004567D6`.
The native path can give an active skill priority in other branches.

### Callees

The pickup helper `0x00455E3E` reads the item pointer at scene offset
`0x80` and checks its hover flag and the pickup cooldown. Actor hover is a
separate pointer at scene offset `0x7C`. For loose loot the helper uses the
Carry check, which calls `0x005171F8`. This function checks pickup distance
and position, then builds and sends the pickup packet.

## State and lifecycle

The frame refreshes independent hover pointers for actors and ground items.
The scene borrows the item pointer during the input event. The existing
pickup path owns its target and cooldown. Item removal clears both item
aliases through the packet handler and destructor. Scene replacement owns
teardown. The server confirms inventory changes and removes committed loot.

## Wire, ABI, and resources

The existing C→S request is little-endian `0x270`, 28 bytes. It carries
destination type and slot as 32-bit integers at 12 and 16. It carries
the item ID and two grid coordinates as unsigned 16-bit values at 20, 22,
and 24. The native sender writes the opcode at `0x005174C1` and sends
28 bytes at `0x00517509`. The server resolves authoritative coordinates
from the item ID. Existing confirmation and removal packets apply.

## Current mapping

Left click reaches pickup through `TMFieldScene::MobAttack`. Right click
previously entered `TMFieldScene::SkillUse` first. Its empty-shortcut guard
returned before pickup, and an active skill could also consume the click.
`OnMouseEventCompat` now checks loose item hover before plain right-click
skill dispatch. It calls `GetItemFromGround` and `TMHuman::MoveGet`.

## Delta matrix

| Claim | Native reference | Local behavior |
| --- | --- | --- |
| Left-click pickup | Physical interaction falls back to item hover | Keep the existing route |
| Right-click pickup | Skill fallbacks can use item hover over actors | Give a hovered loose item priority on a plain right click |
| Ctrl-right click | Right-button down skips skill dispatch | Keep the player menu on button up |
| Ground fixtures | Ground interaction is separate from pickup | Keep the existing fixture, cannon, and skill routes |

## Decisions

Classify the change as `MODERNIZACAO_COMPATIVEL`. Give plain right clicks
an explicit loot priority to meet the requested overlap behavior. Keep
interface priority, modified clicks, skill 31's pickup guard, and the
existing cooldown. Consume repeated clicks on loot during the cooldown.
Use the existing wire, range, ownership, and inventory rules.

## Gaps

Real-client acceptance is pending for exposed loot, loot beneath a player
or monster, and the transitions to skill, movement, and interaction input.

## Validation

- Static: Ghidra `ExportWydFlow.java` resolved the button handlers and pickup
  callees for the recorded hash. Disassembly confirmed the branch conditions.
- Automated: the eligibility check, 3,202,812 architecture checks, targeted
  server and wire pickup checks, and Release client build passed.
- Real client: pending.
