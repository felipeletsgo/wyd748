# Client performance and visuals plan

Status: items 1-5 implemented 2026-10-07 (plan approved by the user);
item 6 waits for field measurements. Measurements come from the instrumented
client (`perf_trace`). Only weather (item 5) touches the wire and gameplay.

## Product constraints

- WYD is played with about five clients side by side, all visible, each with a
  role (tank, damage, healer, leveling, farming). Performance has priority
  over graphics: every per-frame cost is paid five times.
- Approved for the plan (user, 2026-10-06): light profile at **15 FPS** for
  unfocused **and** minimized windows, field cache, anisotropic filtering,
  ambient light and sky colors (high priority), server weather.
- Rejected or deferred: instancing/batching of trees, grass by sectors,
  bloom, per-pixel lighting, larger default view distance. MSAA and HD tile
  textures stay optional and off by default.

## Measurement tool

`internal/platform/diagnostics/PerfTrace.{h,cpp}` is opt-in. Set
`WYD_PERF_LOG` to a CSV path before starting the client (`{pid}` is replaced
by the process id); without it every hook is a cached flag test. One row per
second records FPS, frame time and the slowest frame, CPU time of FrameMove,
render, Present and CleanUp, D3D draw/state calls per frame (device vtable
counters, re-applied every frame because the runtime rewrites the table),
effects created/destroyed, `.trn`/`.dat` load time, scene node counts
(total, deleted, effects, humans, items, ground objects), focus/minimized
state, working set and process CPU (percent of one core). Each `.trn` and
`.dat` load also writes a `#load,<kind>,<file>,<ms>` line.

```powershell
$env:WYD_PERF_LOG = "C:\temp\wyd-perf-{pid}.csv"
Start-Process .\WYD.exe -WorkingDirectory .\tmproject\client748
```

## Baseline (2026-10-06)

Scene: server-selection demo (Armia `Field1616`, 4,027 scene nodes, 3,942
ground objects), Release build, 16 logical cores, VSync 60 Hz, sound off.

| Case | FPS | FrameMove | Render CPU | Present | Draws / SetTexture / SetRenderState per frame | CPU (one core) |
| --- | --- | --- | --- | --- | --- | --- |
| 1 client, focused | 60 | 0.31 ms | 1.9 ms | 14.4 ms | 675 / 1,258 / 2,844 | 27% |
| 5 clients, focused | 60 | 0.30 ms | 2.2 ms | 14.1 ms | 673 / 1,253 / 2,828 | 29% |
| 5 clients, unfocused (each) | 60 | 0.30 ms | 2.2–2.4 ms | 14.0 ms | same | 31–33% |
| 5 clients, minimized | 21.5 | 0.40 ms | 3.3 ms | 42.8 ms | same | 11% |

Findings:

1. Unfocused windows cost exactly as much as the focused one: about 1.5 cores
   for five clients in a light scene. The CPU per instance (about 5 ms per
   frame) is twice the measured FrameMove + render time; the rest is driver
   work inside Present.
2. A minimized window still renders the whole scene (673 draws). Only the
   desktop compositor slows it to about 21 FPS.
3. Field loading: `.trn` reads take about 0.3 ms each; building the objects of
   a `Field*.dat` takes about 60 ms each (two loads, 120–139 ms). A border
   crossing therefore stalls about four frames on every crossing.
4. Render state calls are already filtered for repeats (`RenderDevice`
   caches render states, texture stage states and textures), so a "remove
   redundant states" pass has little left to gain.
5. `EffectFrameMove` only counts effects and runs only while
   `m_bShowEffects` is set, which `RenderDevice::Unlock` clears every frame;
   it is not a per-frame cost.

Not yet measured (needs a logged-in session; use the tool above): field play
with players and monsters, wars/events, and a two-hour session to check that
node and effect counts return to normal after combat.

## Implementation

Client options live in `tmproject/client748/ClientSettings.txt`
(`internal/core/ClientSettings.h`), outside the native `config.txt`, whose
strict schema stays compatible with the 7.48 launcher. Missing keys keep the
defaults: `FocusedFps=0`, `IdleFps=15`, `IdleHideAmbient=1`, `Anisotropy=4`,
`FieldCache=4`.

### 1. Multi-client light profile - implemented

- `NewApp::Run` caps a window that is unfocused or minimized (`IsIdleWindow`)
  at `IdleFps`; a focused window uses `FocusedFps` (0 = VSync only). The wait
  is `MsgWaitForMultipleObjects`, so input, network and focus messages are
  handled at once and the full profile returns on the next frame;
  `timeBeginPeriod(1)` keeps the period accurate. Logic still follows server
  time.
- While idle, butterflies, fish and rain/snow particles are not drawn
  (`IdleHidesAmbient`).
- Measured with five clients, one focused: unfocused clients fell from about
  31% to about 6% of a core each (15 FPS, 580 draws instead of 673); the
  minimized one runs at 15 FPS with 7%. The five clients together went from
  about 1.55 to 0.52 cores.

### 2. Field loading - implemented

- Corner reload loop (`MODERNIZACAO_COMPATIVEL`): the scene keeps two fields,
  and near a field corner `TMScene::FrameMove` asked for the horizontal and
  the vertical neighbor every frame. Each request replaced the other, so the
  client rebuilt two fields (about 60 ms each) on every frame while the player
  stayed there. `GroundNeighborPolicy.h` now keeps the attached neighbor until
  the player is 4 tiles closer to the other border, and attaches only the
  nearer one when both are missing.
- Field cache: `TMScene::ReleaseFieldSlot` moves a replaced field (ground and
  objects, detached from the scene tree with `TreeNode::Unlink`) into an LRU
  of `FieldCache` entries (`FieldCachePolicy.h`); `GroundNewAttach` and the
  warp path reuse a cached field instead of reading and rebuilding it, and
  restore the `RenderDevice::m_bDungeon` value that `LoadTileMap` sets. The
  scene destructor frees the cache. `TreeNode::SetNextNode` now updates the
  old next sibling's back link and `SetPrevNode` accepts a first child.
- Cold object builds measured 43-84 ms (`#load` lines). Revisit timings and
  the working-set cost per cached field are still to be measured in play.

### 3. Lighting and sky colors - implemented, tuning pending

`Env/Lighting.txt` (`LightingProfile.h`) holds the open-field sun color per
sky state (day, rain, sunset, night), the fill light, the world ambient
(`D3DRS_AMBIENT`, previously a fixed `0x33FFFFFF`) and the sky/fog colors.
The shipped file equals the previous values (`LightingProfileTests`), so the
look is unchanged until the values are tuned with the user. Dungeon presets
and per-map fog distances stay in `TMSky`.

### 4. Anisotropic filtering - implemented

World render state block 1 uses `D3DTEXF_ANISOTROPIC` on stages 0 and 1 with
`MAXANISOTROPY` = `Anisotropy`, clamped to the device caps; UI blocks keep
linear filtering.

### 5. Server weather - implemented (`EXTENSAO_COORDENADA` for the scheduler)

- Client: `WeatherPacket.h` validates `0x18B` (16 bytes, values 0-3) and the
  login weather; cast preview, skill panel and tooltip share
  `SkillDamageWeather` (the tooltip previously passed heavy snow unchanged).
- Server: `internal/game/weather.go` schedules the weather (starts clear,
  10-25 minute periods, weights clear 60 / rain 25 / snow 10 / heavy snow 5)
  with its own RNG, sends it in `0x114` and `0x18B`, and applies the native
  damage modifiers in `baseSkillDamage` (rain: fire -10%, thunder +30%; snow:
  ice +20%) at the caster's position.
- A BerielGuard aura still clears the weather on the client display only; the
  server keeps applying the world weather near it.

#### Client weather lifecycle correction - 2026-10-12 (`MODERNIZACAO_COMPATIVEL`)

The field initializer previously attached rain and both snow layers with their
inherited visibility enabled. `SetWeather` handled only rain/snow/heavy snow,
so clear weather left the previous particles enabled. This permits simultaneous
rain and snow at bootstrap; it is a code-confirmed defect, not a visual test.

- `WeatherPresentation.h` resolves one local state from the last server value,
  player region, dungeon and aura. Bootstrap, packets, field ticks and terrain
  warps consume the same selector. Rain excludes both snow layers; heavy snow
  intentionally uses both snow layers. Clear disables all layers outside the
  permanent snow region. Indoor suppression never overwrites server weather.
- The native 7.48 roots are dispatch `FUN_00492e7d`, receiver `FUN_0048569a`,
  selection `FUN_004819d5` and sky setter `FUN_00549bbf`, from the existing
  Ghidra corpus (native SHA-256
  `8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593`).
  The regional exclusion (fields 29/30,22), snow region (27-30,21-24),
  snow-to-rain conversion in dungeon states 3/4, and layered heavy snow are
  preserved. The native selector also lacks a clear branch: explicitly
  deactivating clear weather is a lifecycle correction, not a parity claim.
- Rain owns sound 101; only the base snow layer owns sound 113. Deactivation,
  visual/idle suppression and destruction stop the owned sound. The heavy
  layer cannot stop or restart the base layer's loop.
- Activation resets particle clocks; first updates have zero elapsed time,
  unsigned timer wrap is supported, and pauses are bounded to 100 ms per tick.
  Sky blends use the existing 10/11 transitions without restarting every tick
  or replacing unrelated clear day/night states.
- Packet sizes/opcodes, assets, density settings, the server scheduler and
  authoritative damage modifiers are unchanged.

AUTOMATED TESTED: `WeatherParticlesTests` covers all 16 state transitions,
bootstrap, partial initialization, activation/repetition, clock wrap/pauses,
sound ownership and regional/dungeon/aura combinations. `FieldViewContractTests`
checks that actual scene and particle code consumes those policies. These are
policy execution and source-wiring checks, not a rendered/audio integration test.
CLIENT-TESTED remains pending for login during clear weather, rain-to-snow and
clear transitions, snow-region crossings, dungeon/aura entry/exit, sound changes,
idle recovery and logout. The candidate is built with `Build-Client.ps1 -NoDeploy`;
the local runtime was not installed. The validated executable was subsequently
published in [launcher patch 10004](launcher.md#published-patch-10004-2026-10-12),
with public feed and download hashes verified. Launcher extraction and gameplay
remain pending human validation.
Validation receipt: the Release integrated build passed with 1,200,822
architecture checks, 231 socket checks and 56 launcher checks. Existing Winsock
C4996 deprecation warnings remain; there were no build errors. Repository layout,
local documentation links and `git diff --check` passed. No Go gate was run:
this batch changes only client presentation and documentation.

### 6. Accumulation check - pending field data

With the counters, confirm or rule out: effects with lifetime 0 whose owner is
cleared by `DisconnectEffectFromMob` (no effect deletes itself when its owner
disappears); `ObjectManager::CleanUp` restarting at the parent after every
deletion (quadratic in large wars). Fix the source, never by clearing lists.

### 7. Interface scale, native anchors and weather area - implemented 2026-10-07

- `UIScale=Auto` scales the whole 7.48 layout uniformly with the resolution
  (see `field-ui-scale-and-anchors.md`); the message box and message panel
  keep the native geometry from `FUN_00493e70`. A non-native fixed X of 210
  that pushed the message panel to the right on the server screen was
  removed.
- Every anchor of the field initializer `FUN_00435b13` is ported, including
  the PK / Auto Running / Run Mode labels, the affect description, the
  HP/MP bar adjustment, the HUD button hints and the input marker. Floating
  combat text, labels, counters, 3D meshes in controls and the minimap now
  follow the interface scale.
- Tools: `tools/client-assets/Capture-ClientScreens.ps1` captures the client
  at chosen resolutions with simulated clicks; `WYD748_LAYOUT_PROBE=1` logs
  the visible window rectangles.
- Rain and snow: the classic effects kept 50 drops and 200 flakes in a 12x12
  area around the player, leaving the screen sides dry. `WeatherArea=12`
  (half size; 6 = classic) widens the area and scales the count to keep the
  density (200 drops, 800 flakes per layer); splashes keep the classic area.
  All particles of an effect now go out in one `DrawPrimitiveUP` instead of
  one call per particle (`WeatherParticles.h`), and attribute lookups stay
  inside the map.

## Validation states

- AUTOMATED TESTED: `ClientSettingsTests`, `PerfTraceTests`,
  `WeatherPacketTests`, `LightingProfileTests`, `GroundNeighborPolicyTests`,
  `FieldCachePolicyTests`, `RuntimeAssetLimitsTests`, updated
  `SceneDisconnectContractTests`; WYD-Go `weather_test.go`, the full Go suite
  and vet.
- CLIENT-TESTED: light profile with five clients and the server-selection
  scene (anisotropy and the cache code path at startup).
- CLIENT-TESTED 2026-10-07 (local server, test account): login, character
  creation and selection, field HUD, status labels, message box and panel,
  minimap at 800x600 and 1024x768 with `UIScale=Auto`.
- Pending real play: field crossings and corners (cache and corner policy),
  weather changes and damage in game, lighting tuning, accumulation (item 6).
