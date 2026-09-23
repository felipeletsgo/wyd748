# TMProject 7.48 architecture map

## Scope

This document records the `internal/` tree after its architectural migration.
The map is structural: file names do not prove native parity or change wire,
ABI, or lifecycle contracts.

| Domain | Responsibility |
|---|---|
| `app` | Bootstrap, scenes, and main-flow coordination |
| `core` | Shared types, tables, resources, and compatibility |
| `game` | Entities, combat, and game state |
| `platform` | Win32, input, media, and system integration |
| `render` | DirectX, world, effects, meshes, and visual resources |
| `ui` | Controls, grids, and HUD |
| `wire` | Messages, transport, and protocol |

## Active flow

```text
platform/bootstrap -> app/scenes -> wire/transport
                                  -> game/entities
                                  -> ui
                                  -> render/world + render/effects
```

`core` provides shared structures and services. `Basedef.h` remains a
compatibility facade for legacy structures. Layout constants and global
declarations have been extracted into `UiLayout.h` and `BasedefGlobals.h`
without changing structure order or layout.

## Large files and extraction decisions

Large files include `app/scenes/TMFieldScene.cpp`,
`game/entities/TMHuman.cpp`, and `ui/SGrid.cpp`. They coordinate private
state, callbacks, messages, and teardown. Automatic function-level splitting
would expose private state and could change initialization or dispatch order.
Future extraction should follow independent contracts, with a build check
after each extraction.

## Documentation status

Contract comments were added at compatibility boundaries and in the `Basedef`
loaders. `TMFieldScene.cpp` documents why its compatibility handlers remain
together. Functions without individual comments should be documented during
their next extraction, covering inputs, observable effects, ownership, and
cleanup where applicable.

## Rule for future changes

Classify each extraction as `MODERNIZACAO_COMPATIVEL`, preserve public names,
and add new files to `.vcxproj` and `.filters`. Validate with MSBuild Debug
Win32 and `git diff --check`. Do not claim `CLIENT_TESTED` without executing
the corresponding client flow.
