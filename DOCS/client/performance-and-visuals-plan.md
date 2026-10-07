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
Start-Process .\project.exe -WorkingDirectory .\tmproject\client748
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

### 6. Accumulation check - pending field data

With the counters, confirm or rule out: effects with lifetime 0 whose owner is
cleared by `DisconnectEffectFromMob` (no effect deletes itself when its owner
disappears); `ObjectManager::CleanUp` restarting at the parent after every
deletion (quadratic in large wars). Fix the source, never by clearing lists.

## Validation states

- AUTOMATED TESTED: `ClientSettingsTests`, `PerfTraceTests`,
  `WeatherPacketTests`, `LightingProfileTests`, `GroundNeighborPolicyTests`,
  `FieldCachePolicyTests`, `RuntimeAssetLimitsTests`, updated
  `SceneDisconnectContractTests`; WYD-Go `weather_test.go`, the full Go suite
  and vet.
- CLIENT-TESTED: light profile with five clients and the server-selection
  scene (anisotropy and the cache code path at startup).
- Pending real play: field crossings and corners (cache and corner policy),
  weather changes and damage in game, lighting tuning, accumulation (item 6).
