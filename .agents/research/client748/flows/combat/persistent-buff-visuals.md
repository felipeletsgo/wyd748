---
id: persistent-buff-visuals
title: Persistent buff scales, particles, and material colors
subsystem: combat
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-15
---

# Persistent buff visuals

## Question

Why is the yellow Skill Amp aura too small, and which other buff parameters
diverge from the original 7.48 executable? Audit affect dispatch without
equating a source inventory or parameter comparison with complete visual parity.

## Evidence boundary

- Primary authority: read-only `references/client748/WYD.exe`, identified by
  the SHA256 above. No historical executable was patched.
- Ghidra project: `tmproject/build/native-research/WYD748.gpr`; reusable local
  exports: `tmproject/build/native-research/decompiled/FUN_*.c`.
- The user supplied a screenshot of the undersized yellow plane. It motivates
  the investigation; constructor arguments in the native binary establish scale.
- This record covers persistent affect visuals, weapon emissions, and actor
  material tint. One-shot skill dispatch is a separate boundary documented in
  [skill-visual-dispatch.md](skill-visual-dispatch.md).
- C++ source checks and a Release build are not a pixel comparison or gameplay
  execution. The native parameter contract is resolved; visual validation is open.

## Native 7.48 flow

### Callers

`FUN_00512922` (human frame update) calls `FUN_00506f9d` with server time.
`FUN_0052433d` projects affect slots into human flags and both weapon swings.
Human material rendering consumes those flags in `FUN_005161af`.
The weapon effect's virtual frame callback executes `FUN_004da607`.

### Callees

`FUN_00506f9d` creates billboard planes through `FUN_004cb900`, particles through
`FUN_004c8f70`, Soul through `FUN_004d3d20`, and the latent Life Drain effect
through `FUN_004ce5e0`. Scene attachment is `FUN_0054ac09`; removal uses the
object manager at `0x013B71E8`, virtual slot `0x100`.
`FUN_004da607` uses the same billboard particle constructor and scene attachment.

Skill Amp flag is human offset `0x789`; its persistent pointer is `0x53C`.
The constructor is `(93, 0, 1.5, 1.5, 1.5, 0, 5000)`, using float bits
`0x3FC00000`. Both creation and follow use terrain height plus `0.4`, not
character head height. Fade is `3`, color `0x88888800`, bright blending.

Float constants were read from the native PE by mapping virtual addresses
through section virtual/raw offsets and decoding little-endian float32 values:
`0x005A4154=0.4`, `0x005A4158=0.2`, `0x005A415C=0.8`,
`0x005A40EC=0.3`, `0x005A41E8=0.02`, `0x005A439C=0.05`,
`0x005A4214=0.1`, `0x005A5640=0.07`, `0x005A4BD8=2.5`,
`0x005A45D4=0.01`, `0x005A40F0=3`, `0x005A414C=0.6`,
`0x005A4BE4=0.12`. Enchant constructor bits `0x38D1B717` decode to `0.0001`.

## State and lifecycle

Persistent planes are created only when their pointer is absent, then moved
each frame; losing Skill Amp deletes its object and clears its pointer. Hidden
human handling suppresses the plane. Skill Amp follows terrain while mounted.
No packet, affect duration, authoritative gameplay, or ownership policy changes.

Mana Control produces two scene-owned particles per visible frame, each with a
1500 ms lifetime and allocation guards. Magic Weapon emits three pairs per
interval, each with a 500 ms lifetime, scene ownership, allocation guards, and
clock advancement after the emission loop. Existing hide switches remain.

Critical Armor intentionally has different creation and follow offsets on foot:
head plus `0.3` at creation, head thereafter. Mounted creation and follow both
use head minus `0.3`. Soul's half-height correction is an initial anchor only;
its mesh-rotation callback determines subsequent orbit positions. Existing
death/hide guards, cleanup paths, and pointer reuse are retained.

## Wire, ABI, and resources

No wire or ABI changes. Texture IDs, mesh IDs, constructors, and scene ownership
are existing contracts. No assets were replaced or added. Ground-plane textures
are Skill Amp `93`, Aurora/Shadow `94`; Protector uses mesh `32`, Critical Armor
mesh `2838`/texture `413`, Soul mesh `2839`.

Native dispatch uses visual `36` for Critical Armor. The earlier source mapping
to `24` collided with the server's Samaritan affect; the correction below
supersedes that routing. Current affect capacity remains `32` slots versus
native `16`. Hunter's Vision plus OverExp uses the current
extension textures `439`/`440`; native standalone Hunter's Vision uses `438`.

## Current mapping

- `TMHumanEffects.cpp`: persistent planes, timed emissions, and mesh effects.
- `TMHumanRender.cpp`: Slow Slash material tint.
- `TMEffectSWSwing.cpp`: weapon emission families.
- `UiModeContractTests.cpp`: 24 additional source-contract assertions, including
  rejection of the reduced Skill Amp constructor. These do not render a frame.

All current affect switch cases were inventoried. Coverage below distinguishes
compared parameter branches from routing-only checks and unresolved composites:

| Current affect IDs | Visual route | Audit result |
| --- | --- | --- |
| 1, 2, 3, 20, 32 | Freeze, Haste, Slow Slash, Poison, Cancel | Material/trail parameters compared; Slow Slash diffuse red corrected |
| 4, 5, 6, 7, 8 | PowerUp, SkillBlack, Shield2, SpeedDown, DilpunchJewel | Routing inspected; full material/shader composites not certified |
| 9, 13, 21, 27 | Magic Weapon, Assert, Armor Class, Enchant | Native weapon parameter branches compared; three branches corrected |
| 10, 11, 19, 22, 23, 26 | Waste, Shield, Immunity, Lighten, Elemental, Dodge | Human emission parameters compared; Immunity, Lighten, Elemental corrected |
| 15, 17, 18, 36, 25, 28, 37 | Skill Amp, Aurora, Mana Control, Critical Armor, Protector, Shadow, Soul | Persistent branches compared; five corrected, Aurora/Shadow retained |
| 29, 31 | Avatar, Coin Armor | Routing inspected; complete class/material composites remain open |
| 30, 39 | Hunter's Vision, OverExp | Standalone native effect compared; current combined extension retained |
| 40, 44, 45 | Movement/attack restrictions and black flag | Routing inspected; no invented standalone emitter |
| 12, 14, 16, 24, 33, 34, 35, 38, 41, 42, 43 | No dedicated visual flag in the current affect switch | Inventory only; no new visual effect inferred |

Life Drain is also latent in native/current flags: neither inspected affect
switch activates it. Do not invent an affect mapping solely to show that effect.
The inventory is not a claim that every skill, combination, class, mount,
visibility transition, texture, and shader has been tested visually.

## Delta matrix

Every functional correction below is `PARIDADE_NATIVA`, grounded in the named
native functions and unchanged constructor/resource contracts.

| Claim | Previous source | Native contract now implemented | Evidence |
| --- | --- | --- | --- |
| Skill Amp plane | Scale `0.5`; ground offset `0.05` | Scale `1.5`; ground offset `0.4` | `FUN_00506f9d`, constructor bits and PE float |
| Lighten on foot | Scale `0.6` | Scale `1.0`; mounted `1.3` retained | `FUN_00506f9d` |
| Immunity rotation | 5000 ms period | 1000 ms period | `FUN_00506f9d` |
| Elemental emission | Interval greater than 250 ms | Interval greater than 500 ms | `FUN_00506f9d` |
| Mana Control | Both emissions commented out | Two 1500 ms particles; head spread `0.07`, ground spread `0.1` | `FUN_00506f9d` |
| Protector mesh | Scale `0.3` on all axes | Scale `0.5` on all axes | `FUN_00506f9d` |
| Critical Armor | H/V scales `0`/`0.5`; raised anchors | H/V scales `0.7`/`0.7`; native anchors | `FUN_00506f9d` |
| Soul mesh | Scale `1.4`; full-height initial anchor | Scale `0.4`; half-height initial anchor | `FUN_00506f9d` |
| Slow Slash tint | Diffuse red half pulse | Diffuse red full pulse; emission/specular remain half | `FUN_005161af` |
| Magic Weapon | 150 ms; one particle per iteration; random range 60 | 50 ms; two particles per iteration; range 5; native colors, sizes, velocities, rotation, height | `FUN_004da607` |
| Armor Class particles | Spread multiplier `0.009` | Multiplier `0.01` | `FUN_004da607`, PE float |
| Enchant particles | Velocity `0.00009` | Velocity `0.0001` | `FUN_004da607`, constructor bits |

## Decisions

Correct only proven parameter/emission divergences. Preserve canonical affect IDs,
the OverExp combination, and gameplay. Remove only incompatible commented-out
weapon emission alternatives after establishing the native two-family flow.
Translate legacy comments in touched source files; untouched older records remain
language migration debt, not a release exception.

## Samaritan and Critical Armor icon correction (2026-10-15)

Classification: `EXTENSAO_COORDENADA`, correcting the existing visual contract
without changing packet sizes, opcodes, canonical gameplay or persisted state.
Native `FUN_0052433d` already proves Critical Armor visual 36; native
`FUN_004431e4` reads the skill atlas mapping at `DAT_005b77c0`.
Those recorded roots and unchanged atlas inputs were reused, not re-exported.
Current server SkillData assigns Samaritan skill 3 to affect 24; native table
entry 24 points to unrelated skill 49 and is contradictory for that assignment.
TMProject is used only to locate consumers; external sources are not needed.

Wire visual mapping v2 is server-to-client: Samaritan 24 remains 24 and maps to
skill icon 3; Critical Armor canonical 31 with persisted ClientType 24 maps to
native visual 36 and skill icon 15. Legacy raw type 50 also maps to 36, including
old ClientType 50. Ordinary canonical 31 remains Coin Armor with icon 86.
No other server effect owns visual 36. The shared table retains all other
entries, including its zero-filled tail, and drives owner, target, party and
C.C. recognition. `TMHuman::CheckAffect` activates Critical Armor only for 36;
Samaritan must not activate that persistent mesh.

The private 140-byte 0x3B9 snapshot retains 16 eight-byte entries beginning at
12: visual byte, level byte, value WORD and duration DWORD. Public 232-byte
0x336 score snapshots and coordinated 328-byte 0x364 CreateMob snapshots retain
affect WORDs at 152 and 70 respectively: high byte visual, low byte duration.
Duration remains measured in eight-second units; values/levels are unchanged.
No client intent is trusted and no new allocation, ownership or UI lifecycle
is introduced. Invalid/expired observed entries continue to hide their controls.
Apply/remove, death, view reentry and relogin use the same central projection;
serialization never mutates authoritative affects. Logout/shutdown retain the
existing scene-owned panel and persistent-effect cleanup paths.

Client and server must be deployed as a pair; there is no capability negotiation
or new wire format. Mixed old source clients may omit Critical Armor's mesh;
old servers still alias both buffs as 24. Roll back both builds together.
Tests cover both simultaneous buffs, Coin Armor isolation, legacy states,
owner/public/reentry packets, unchanged level/value/state, production atlas
indices and removal without corrupting the remaining icon.

AUTOMATED TESTED: full Go suite and vet passed; Release/x86 build passed,
ArchitectureTests 1,229,509 checks, socket 231, crash capture 83, launcher 56.
No gameplay process was launched: CLIENT-TESTED remains pending.

## Berserk visual and buff dispatch audit (2026-10-29)

Classification: `PARIDADE_NATIVA`. Native `FUN_0052433d` sets human `+0x783`
(Speed Up) for visual 8 and `+0x784` (Speed Down) for visual 7. With Speed Up
on and Speed Down off, it enables the weapon hand effect. `TMHuman::CheckAffect`
handled visual 8 only for the later Dilpunch jewel bit and never set
`m_cSpeedUp`, so Berserk (TK 9) showed no effect. Visual 8 now sets it; the
existing hand-effect rule is unchanged.

The same audit compared every 7.48 SkillData buff with the server wire visual
and the native dispatcher. The server sends native IDs for all buffs. Native
has no persistent effect for visuals 14, 24 and 38 (Possessed, Samaritan,
Spirit Change, Improved Range); BM forms use the face transform. Magic Weapon
(9) sets only `m_cWeapon`, as native. Visuals 5 and 6 (SkillBlack, Shield2)
have no case in the native projector and stay unverified.

AUTOMATED TESTED: `Build-Client.ps1 -NoDeploy` Release/x86; ArchitectureTests
3,177,024 checks with two new `UiModeContractTests` assertions. No client was
launched; CLIENT-TESTED is pending. Ships with unpublished revision 11.

## Remaining gaps

- Actual built-client visual comparison remains open: Skill Amp on foot/mounted,
  apply/remove/hide/death/relogin, buff combinations and the other changed effects.
- Full PowerUp, SkillBlack, Shield2, Avatar and Coin Armor material/shader
  composites need a separate focused comparison before certification.
- Hunter's Vision plus OverExp is an existing non-native combination; no
  replacement is justified by standalone native evidence alone.
- Native-compatible parameters do not establish performance under dense buff
  populations; restored emission rates need runtime observation.
- Launcher/VPS publication is verified as patch 10005; actual launcher
  extraction and local runtime installation remain unverified. See the
  [publication receipt](../../../../../DOCS/client/launcher.md#published-patch-10005-2026-10-13).

## Validation

- `pwsh -NoProfile -ExecutionPolicy Bypass -File tmproject/Build-Client.ps1 -Configuration Release -NoDeploy`:
  final run PASS, after fixing a duplicate test variable in an earlier run.
- ArchitectureTests: 1,200,844 checks PASS; static assertions PASS.
- SocketReceiveTests: 231 checks PASS. Launcher tests: 56 checks, zero failures.
- Costume dependencies and all four skin-shader variants PASS.
- Release executable: `tmproject/build/TMProject748/Release/WYD.exe`.
- Artifact SHA256: `55C4A3D2E19404CC4E2ED9F529E7407CF8209B00EE7BD08FB4F44569BCEAF977`.
- Build emitted the existing DirectInput default-version informational message.
- Classification: `STATICALLY VERIFIED`, `AUTOMATED TESTED`, Release build
  and public patch 10005 deployment/download verified. The downloaded ZIP
  contains the same candidate hash. Local runtime was not overwritten.
  Not `CLIENT_TESTED` or release-ready certification.
