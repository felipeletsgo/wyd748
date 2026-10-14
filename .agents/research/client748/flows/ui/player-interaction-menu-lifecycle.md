---
id: player-interaction-menu-lifecycle
title: Player interaction menus, party exit and Escape lifecycle
subsystem: ui-lifecycle
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-14
---

# Player interaction menus, party exit and Escape lifecycle

## Question

How does native 7.48 open and close player menus, and how must party Leave
differ from merely hiding its window?

## Evidence boundary

- USED: native FUN_00435B13 binder, FUN_0055dab8/FUN_0044e1e8 openers,
  FUN_004662c5 control callback and FUN_0044df53 Escape handler.
- USED: current TMFieldScene input, OnControlEvent, OnESC and UI bindings.
- CONTRADICTORY: allowing a visible modal to consume Escape before OnESC,
  or treating party Leave as only window close, breaks the observable flow.
- NOT APPLICABLE: later-client control numbers do not override native bindings.

## Native 7.48 flow

### Callers

WM_RBUTTONUP (517) with MK_CONTROL (8) opens the player menu for a valid
target. Opening a hidden root must precede base modal input handling;
otherwise the container can consume the event without opening the menu.
FUN_0055dab8 reaches FUN_0044e1e8; control dispatch reaches FUN_004662c5.
Escape reaches FUN_0044df53.

### Callees

FUN_00435B13 binds FieldScene2 root 640 and target name 645.
Controls 641/642/643/644 mean party/guild/trade/cancel. Challenge 620 and
mode controls 639/621/622/623 remain distinct. Controls 816/817/862/863 and
912..915 retain their existing guild-mode/rank roles.
Native controls 0x748/0x749 (1864/1865) distinguish party Leave/window close.

## State and lifecycle

### Observable entry

Ctrl-right-click a valid nearby player; Escape closes an active menu through
OnESC before the base scene can consume the key. Existing input cooldown and
air-movement restrictions remain unchanged.

### Transition matrix

| Event | Transition |
| --- | --- |
| Valid target and hidden menu | Resolve target, populate name, show root 640 |
| Escape with player menu | Hide root and clear transient target |
| Escape with system/menu/help window | Close the selected existing window |
| Party control 1864 | Send 0x37E; wait for authoritative membership update |
| Party control 1865 | Hide party window without leaving |
| Scene teardown | Release scene-owned controls and target references |

### Vtables, vptrs, and receivers

The existing scene callback at vtable +0x58 and scene/grid receivers remain
unchanged. No vtable, class layout or receiver ABI is introduced.

### Ownership

The scene graph owns menu controls. The modal pointer does not own root 640.
m_pPGTOver is a transient target reference, cleared when the menu closes.
No cross-scene target pointer or new allocation is retained.

### Partial failure

Invalid/dead/missing target leaves the menu closed. Existing targeting checks
exclude the 0x8800FF00 border case and preserve self/AutoTrade handling.
Missing controls are guarded. A failed party leave request does not locally
fabricate a membership change.

### Cleanup and teardown

OnESC clears player-menu targeting and hides its root. Main-menu controls
292/5744 have their selection cleared; System, TMB_SYSTEM and HelpMemo use
their existing close paths. One-window-at-a-time Escape cascade is retained.

### Shutdown

No new asynchronous callback, timer or worker exists. Scene ownership releases
controls normally; no menu object needs an independent shutdown operation.

### Logout and relogin

The old scene destroys its controls and transient target. A new scene binds
hidden menus and reconstructs party membership from server state.

## Wire, ABI, and resources

Party Leave uses existing MSG_RemoveParty opcode 0x37E with the local human
ID and Parm 0. The server validates actual membership and broadcasts the
result. Window close sends no leave packet. Menu IDs, resource paths and
existing party wire structures are unchanged.

## Current mapping

PARIDADE_NATIVA: native player-menu bindings and Leave/window-close distinction.
MODERNIZACAO_COMPATIVEL: route Escape through existing OnESC before modal
consumption, preserving the established window cascade.

## Delta matrix

| Claim | Fault | Decision |
| --- | --- | --- |
| Hidden player menu | Base modal could consume opener | Open before base dispatch |
| Escape | Modal could consume it | Run OnESC first |
| Target lifetime | Stale transient reference | Clear on close/teardown |
| Party Leave | No membership request | Existing 0x37E request |
| Party window close | Must not leave | Hide only |
| System/main-menu selection | Could remain active | Clear existing selection |

## Decisions

Retain native IDs, authoritative party state and scene ownership.
Do not duplicate menu objects or alter unrelated social actions.

## Gaps

Actual client interaction, target disappearance and sequential Escape across
overlapping windows remain untested in game. No CLIENT_TESTED claim.
Published in patch 10024; see
[current state](../../../../../DOCS/current-state.md) for receipts and acceptance.

## Validation

Source contracts cover party opcodes/controls and Escape routing/cleanup.
ArchitectureTests passed 1,209,006 checks; Release/Win32 -NoDeploy build passed.
The installed executable was not overwritten and no client was launched.
