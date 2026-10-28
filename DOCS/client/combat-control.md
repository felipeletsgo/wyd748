# Combat Control scheduling

## Scope and reference

The current C.C. window, buttons, thresholds and movement controls are retained.
The owner's revised request replaces the KR-client investigation with the
existing client and the supplied
[TMProject GameAuto implementation](https://github.com/MacedaoRG/w2Project-SeitTbNao/blob/dea5ab067887eb4ef42795ea1daaa9bd65623d05/Projects/TMProject/TMFieldScene.cpp#L23726).
The reviewed revision is `dea5ab067887eb4ef42795ea1daaa9bd65623d05`.
Its `GameAuto` routine is an algorithm reference, not proof of 7.48 parity.
No reference executable, UI resource or newer structure layout is imported.

Classification: `MODERNIZACAO_COMPATIVEL`. Scheduling uses existing item and
skill intentions; the server still validates mana, cooldowns, equipment,
targets and effects. No opcode, packet layout or server buff duration changes.
The established [C.C. input flow](../../.agents/research/client748/flows/ui/cc-auto-combat-state-sync.md)
and [attack envelope](../../.agents/research/client748/flows/transport/attack-frame-envelope.md)
remain applicable to those unchanged boundaries.

## Scheduling

### Manual physical target continuation (2026-10-17)

Classification: `MODERNIZACAO_COMPATIVEL`. Per the owner's explicit rule,
a valid physical click retains that enemy until death or cancellation,
independently of C.C. and without selecting a replacement enemy. The compact
field frame had omitted retained-target retries. It now schedules them at
200 ms intervals through the existing `MobAttack` path, preserving its
guild/party, PK, town, merchant, range and route validation. The existing
1000 ms client packet gate and authoritative 900 ms server gate are unchanged.
An attempt during cooldown retains the selected enemy without sending early.

Manual movement, skill use and toggling native autoattack off clear the
selection through existing input paths. Death, delayed removal, transitions
and enabling C.C. cancel the retained target; C.C. keeps its own scheduler.
Movement in progress suspends retries rather than rebuilding the route each
frame. A kill confirmation clears only the selected victim, regardless of
who killed it, not an unrelated surviving target. No MP requirement is added
to physical attacks.

The local native decompilation `FUN_004776c3` contains a 200 ms retained-target
retry into `FUN_0051a939` while native autoattack is enabled. Reusing that
scheduling interval does not prove parity for the owner's always-retain-on-
click choice. No opcode, packet layout, server cadence or damage formula
changed. Existing wire evidence remains applicable.

AUTOMATED TESTED: retry/cancellation and unsigned-wrap policy plus frame,
validation-path and victim-cleanup source contracts passed. Release/x86
compiled with `Build-Client.ps1 -NoDeploy`: architecture 1,240,451 checks,
socket 231, crash capture 83, launcher 56; asset/shader checks passed.
CLIENT-TESTED remains pending for melee/ranged combat, pursuit, cooldown
retargeting, manual cancellation, third-party kills, transitions and C.C.
No runtime installation, client launch, publication or restart was performed.

### Skill-name alignment (2026-10-16)

Skill identity is its numeric ID, not a translated label. The active client
displays skill-item names from `Itemname.bin` (item ID = 5000 + skill ID),
while its gameplay parameters load from `SkillData.bin`. The ignored local
`client748/SkillData.csv` is not a runtime input. The server loads its tracked
`wydgo748/data/catalog/SkillData.csv`, including names used in diagnostics.

The server CSV names now match all 103 defined skill labels in the client's
item-name resource and the matching server item-name catalog. The null row
103 remains `NULL`. The local CSV was aligned as well. Only names changed;
the first 22 columns, skill IDs, binary assets and gameplay rules did not.
This is a compatible metadata correction, not a new native-mechanics claim.

Notable corrections are 37 = `Lightning_Storm` (Foema/Black Magic),
79 = `Another_Soul` (Huntress/Survival), 13 = `Growth`, 74 = `Enchant_Frost`,
75 = `Hide_In_Shadow`, 85 = `Backpack`, and 86 = `Coin_Armor`.
The old CSV labels must not be used to infer a skill's behavior: a
non-aggressive timed affect alone does not establish that it belongs in
automatic buff renewal. The requested SkillData-based C.C. selection revision
is still pending; the existing scheduling policy below has not changed.

`TestSkillDataNamesMatchClientItemNames` checks the server CSV against the
decoded 68-byte client name records. This uses tracked runtime resources;
it does not depend on the ignored local CSV or a handwritten duplicate list.

AUTOMATED TESTED: all `internal/data` tests passed after this correction.
STATICALLY VERIFIED: all 104 rows retain their first 22 columns from HEAD;
the local and server CSVs match exactly, and `git diff --check` passed for
the tracked changes. No executable rebuild was needed because no source or
client runtime binary changed. No publication, restart or client execution
was performed for this metadata correction.

### Existing scheduling policy

`TMFieldSceneAutomation.cpp::GameAuto` follows the reference's priority:

1. Stop during death, scene teardown and logout/town/teleport transitions.
2. At the existing consumable scheduling opportunity, try mount HP feed,
   player HP potion, player MP potion, then hunger feed. Hunger between 1 and
   5 is independent of the configured mount HP percentage.
3. Scan all ten slots of the inactive skill belt, from left to right, for
   automatic buffs. The selected attack suffix does not limit this scan.
4. Process summon skills 56..63 using their 80-second repeat interval.
5. Support mode stops here; physical and magic modes continue to their
   existing attack/target/movement paths.

Automatic buff IDs match the reference: 3, 5, 53, 54, 9, 11, 37, 41, 43, 44,
45, 46, 64, 66, 68, 70, 71, 87, 75, 76, 77, 81, 85, 89 and 92.
Transform skills 64, 66, 68, 70 and 71 share affect identity 71.

The reference's mastery groups determine the repeat delay:
`100 * AffectTime * mastery` milliseconds. `CCModePolicy.h` owns the groups,
membership and timing arithmetic. Successful casts retain the existing
`IncSkillSel` rotation when automatic attack selection is enabled; otherwise
the previous attack selection is restored.

## Compatibility adjustments

- WYD-Go support buffs last 38..75 eight-second units (304..600 seconds),
  based on mastery clamped to 0..320. A literal reference delay can exceed
  an hour. The repeat delay is therefore capped at this server duration minus
  the 24-second refresh lead. Server duration and balance remain unchanged.
- The client does not decrement stored affect `Time`. Remaining time is
  derived from the snapshot arrival timestamp instead of treating stale
  `Time > 3` as permanent protection against recasting.
- A buff with more than three remaining units is skipped. Refresh candidates
  must also pass the mastery delay, cooldown and mana gates.
- `SkillUse` can report handled input without casting. Only a change in its
  cast timestamp ends this frame's buff scan. A failed candidate restores
  selection and allows the next candidate; there is no blanket ten-second
  failed-attempt retry delay.
- Existing scene dependency checks and insufficient-mana throttling remain.
  Invalid/empty magic selection is rejected before indexing the skill belt.

## Validation and delivery

STATICALLY VERIFIED and AUTOMATED TESTED on 2026-10-15:

- Focused policy tests cover buff membership, mastery groups, transform
  identity, hunger, duration caps, refresh boundaries and unsigned tick wrap.
- Source-contract tests check consumable/buff/support ordering and that only
  a cast ends the buff frame.
- `Build-Client.ps1 -NoDeploy` passed the integrated architecture tests
  (1,228,375 checks), socket tests (231), crash capture tests (83), launcher
  tests (56), asset/shader checks and the Release/x86 client build.
- Candidate SHA-256:
  `0B0EB676E192DB1626B03EC38E8FA06DE1F6235CF5EC9F4B4557F12E21CB7DD6`.

No runtime executable was replaced, no client patch was published and no VPS
deployment/restart was performed. CLIENT-TESTED remains pending for long-run
buff renewal, mixed attack/buff belts, low mana, support-only operation,
mount feeding and teleport/death transitions. Automated checks do not prove
those visual/in-game flows.
