# WYD 7.48 UI, HUD and grids

Read with [native evidence](ghidra-client748.md). This reference retains proven
IDs and historical observations; it does not make every listed scenario
`CLIENT-TESTED`. Reuse valid flow records and reopen only the affected gap,
not all callers/callees after every edit.

## Authority and identity

For parity, use the historical `references/client748/WYD.exe` and Ghidra,
then resources actually loaded by 7.48 and comparable original screenshots.
Candidate runtime logs explain the implementation, not native intent.
TMProject 7.59 is secondary implementation evidence.

`tmproject/client748/screenshot/Capture0084.jpg` is a historical HUD/inventory
anchor. `tmproject/client748/client-debug.log` is volatile: link it to the exact
launched executable path/hash and resources before using values. The normal
installed runtime is `WYD.exe`; an old `project.exe` is not automatically current.

## Native anchors

| Function | Recovered contract |
| --- | --- |
| `FUN_0040d13e` | `SGridControlItem` constructor; legacy meshes/UI use 24 per logical dimension. UI2 sprites use 23 for items 5000..5102, otherwise 32, multiplied by `g_pItemGridXY`. |
| `FUN_0040e817` | Ordinary-grid scale caps at `(logicalHeight * 0.3) / mesh->m_fMaxZ` when oversized. Historical patched footprint 1 changes the input, not the renderer. |
| `FUN_0040dd00` | Item frame/position and centering within its native cell/region. |
| `FUN_00435b13` | HUD texts 1029/1030/1031/1032/1033/1040; Skill 1905, Apprentice 1889, Inventory 257, six ItemMix roots, Party 1857/1863/5742 and responsive layout. |
| `FUN_004431e4` | Current/next-threshold EXP, ATT/selected-skill damage, DEF/companion base defense, four-quarter EXP and Character Att Speed/C.POINT/HOLD/Kingdom updates. |
| `FUN_0044da6f` | Party root 1857 toggle with button 5742 selection inverse to visibility. |
| `FUN_00492e7d` | Party invite/add/remove dispatch for `0x37F/0x37D/0x37E`. |

Addresses belong to the recorded historical hash in `ghidra-client748.md`.

## Compact HUD and Character fields

| ID | Meaning |
| --- | --- |
| 1029 / 1030 | Primary ATT / selected-skill damage |
| 1031 / 1032 | Current EXP / absolute next-level threshold |
| 1033 / 1040 | Primary DEF / companion base/native ability defense |
| 1171 | Progress within the current EXP quarter |
| 1172 / 1173 / 1174 | First / second / third completed-quarter indicators |

The 2026-08-23 runtime snapshot under parent 5716 recorded:

```text
1032 x=110.4 y=16.6
1031 x=42.4  y=16.6
1040 x=150.7 y=38.4
1033 x=118.8 y=38.4
1030 x=58.4  y=38.4
1029 x=24.4  y=38.4
```

These identify the loaded resource, not new hardcoded coordinates. Modern IDs
5718..5720 may name unrelated native children. `Capture0084.jpg` shows absolute
EXP such as `139188261 / 140000000`, with ATT/DEF pairs below. Divide the
current-to-next-level interval into four: 1172..1174 use completed textures;
1171 receives only the current quarter's value/maximum, not a newer progress UI.

Parent 5716 starts near width 196.3 in the 1024x768 resource. If stretched to
fill the lower center, transform children 1029..1040 and 1171 horizontally too;
stretching only the background crowds values at the left. Keep ornaments
1168 and 1172..1174 right-anchored at native size.

Character bindings from `FUN_00435b13` / `FUN_004431e4`:

```text
1110  Att Speed
1168  HOLD ornament, visible for positive Hold/FakeExp only
1376  C.POINT text using message 304
1377  Kingdom emblem, visible for valid cape/kingdom only
```

Modern `65768/65771..65774` does not replace 1377. HOLD/Kingdom begin hidden
and recalculate on state updates; missing modern tooltip controls are optional.
`flows/ui/character-stat-fields-update.md` covers callers/callees and lifecycle.
CP/Chaos is not `MSG_UpdateEtc::Hold`: that field carries persisted Held EXP
debt; CP stays on the legacy CreateMob boundary. Positive Hold shows 1168;
1376 displays value/percentage over 10% of the current-level interval. See
`flows/combat/pvp-death-held-exp-lifecycle.md` for that lifecycle.

## Item context, scale and appearance

| Context | Logical occupancy | Render/drag |
| --- | --- | --- |
| Inventory/cargo | 1x1 | Centered, contained within 24x24 cell |
| Ordinary NPC shop | Adapted items use 1x1 | Contained without invading neighboring columns |
| Equipment | Irregular slot region | Large representation fitted to the full region |
| Drag over inventory/cargo/shop | Destination footprint | Destination scale and one-cell highlight |
| Drag over equipment | Destination slot region | Representation/highlight cover the whole region |

Stock uses mesh maximum Z and original logical footprint height:

```text
targetExtent = max(itemCellHeight, 1) * 0.3
scale = min(1, targetExtent / meshMaxZ)
```

Historical patched `WYD.exe` changes all 16 `g_pItemGridXY` entries to `{1,1}`
and masks to the first cell. The recorded byte comparison leaves the renderer
unchanged, so ordinary-grid target extent becomes 0.3. Both `FUN_0040e6aa`
(virtual +0x8C) and `FUN_0040e817` (+0x90) use mesh +0x354 MaxZ, not AABB/radius
or item-family overrides. Equipment retains its own 1.0/context scale; drag
recalculates for destination, not stale source context.

The 23/24/32 sizes are logical 800x600 units. Apply `m_fWidthRatio` and
`m_fHeightRatio` consistently to `m_nWidth/m_nHeight` and `m_GCObj.width/height`,
not again to model `fScale`. `FUN_0040fc3e` derives origin from width/columns
and height/rows; `FUN_0040dd00` centers by half the region. Keep blue valid and
red invalid destination feedback across the complete equipment region.

The historical centered footprint does not prove visual acceptance for every
family. A dull/missing glow is not automatically a scale problem: distinguish
wrong icon/mesh, material/texture, blend/depth/draw-order state and stale
create/update state. Preserve native render-state restoration. A tooltip name
does not prove icon identity, and a cape icon does not prove a kingdom change.
Implement in active source/assets; never execute a historical patcher.

## AutoTrade

`FUN_004110f5` materializes panel 626, price edit 627, caption 630 and confirm
667, without the inherited `m_pChatSelectPanel`. Guard all optional accesses:
open, invalid price, cancel, close, reentry and shared input. Do not fabricate
a modern control. Historical crash `client-crash-20260826-154302.dmp` in
`SGridControl::TradeItem`, RVA `0x718AC`, demonstrates this null access.

Native sale blacklist: `508, 509, 522, 526..537, 747, 3200..3299`. Item 4905
is not blocked by this 7.48 rule. Server revalidation of item, slot, quantity
and price remains mandatory regardless of client permission.

For this flow, test permitted item/valid price, invalid price without crash,
cancel/close paths, publication, genuinely blocked item versus 4905, and reentry
without phantom items, wrong messages or stale focus.

## ItemMix and artisan routing

All six panels have native uses; none is free UI for an invented system.

| Mix | Panel / slots / execute | Native NPC condition | Opcode |
| --- | --- | --- | --- |
| 1 | 1360 / 1367..1374 / 1375 | Compositor head 54; chunk 19,13 mode 0, chunk 25,13 mode 1 | `0x3A6` |
| 2 | 6110 / 6112..6119 / 6111 | Aylin head 55 | `0x3B5` |
| 3 | 6145 / 6147..6152 / 6146 | Agatha head 56 | `0x3BA` |
| 4 | 6432 / 6436..6438 / 6434 | Tiny head 68 outside chunk 19,15 | `0x3C0` |
| 5 | 6481 / 6483..6489 / 6482 | Lindy chunk 13,13; Odin chunk 25,13, low Merchant = 8 | `0x2C3` Lindy; `0x2D2` Odin |
| 6 | 6512 / 6516..6518 / 6514 | Ehre head 68 in chunk 19,15 | `0x2D3` |

`FUN_00435b13` centers each root separately and places Inventory to the right
at `centerX + Inventory.width*0.5 + 10`, both vertically centered. Reapply when
opening an artisan because Shop/Trade/AutoTrade move shared Inventory root 257.
Serialized/residual position is not the final native layout. Ordered toggle/
cleanup pairs:

```text
FUN_00449384 / FUN_004487E2
FUN_00449632 / FUN_004489C5
FUN_004498E0 / FUN_00448C38
FUN_00449B8E / FUN_00448E0B
FUN_00449E3C / FUN_00448FDE
FUN_0044A0FB / FUN_004491B1
```

Mix2 result grids are 6129..6132. In `MouseClick_MixNPC`, Ehre precedes Tiny
fallback; specific head-67 cases precede fallback. Preserve chunk/Merchant
conditions instead of routing only by name/head. Mix5 also has native `0x2C4`
for head 67, chunk 28,24, NPC ID other than 1033. At the original investigation
the server lacked its authoritative handler/recipe (`DORMANT`); check current
source before reusing that finding. Absence does not authorize inventing a
recipe or repurposing Lindy/Odin's panel.

Modern 65857/65861+, button 81923 and path 86019 do not replace these IDs.
Modern grid numbers were reused: resolve destinations by visible panel and
exact IDs/pointers; do not renumber ABI or let quickslot branches intercept them.

## Party and Quest

Party uses root 1857, list 1863, button 5742. `FUN_00435b13` positions root/
button at `x=0`, `y=viewportHeight-root.height-165`. `FUN_0044da6f` shares the
toggle among button, `P`, packets, `Esc` and competing UI; button selection is
inverse to panel visibility. `0x37F/0x37D/0x37E` update invitation/add/removal;
left-click invitation emits `0x3AB`; Ctrl-click removes and right-click supports
support skills. The container owns controls and list owns items. Hiding the
panel does not leave the party; distinguish leave action from visibility.
Teardown converges at FieldScene deleting destructor `FUN_004358da`.
Modern `475136/475138/65799` and optional AutoParty do not replace native IDs.
See `flows/ui/party-panel-layout-lifecycle.md`; repeat affected paths after relogin.

Quest uses toggle 315, root 320, list/content pairs 321/322, 325/326, 327/328,
335/334, close 323, title 324, tabs 329/330/331/333 and memo 332.
`FUN_00441823` centers root `(viewport-panel)/2` on both axes and loads
`QuestSubjects.txt` through `QuestSubjects4.txt` plus `QuestMessage.txt`.
`FUN_004662C5` handles toggles, tabs, list selection, memo and closure;
`FUN_0049E50F` rejects a null destination. `X` routes to 315, not imported 65793;
`FUN_00453C59` sends `Esc` to `FUN_0044DF53`; teardown reaches `FUN_004358DA`.
Share visibility across click, `X`, close, `Esc` and AirMove. Select only one
list/content pair, protect optional controls, and recreate bindings/text after
scene change/relogin. See `flows/ui/quest-panel-layout-lifecycle.md`.

## Skill windows, closure and messages

For each affected window prove loaded IDs, creation, input/callback, initial
state, modality, focus, ownership and destruction. Native `Esc`, `X` and
Cancel paths converge; hiding a drawing must not leave mouse capture/modal
state active. System menu placement follows native centered resource/lifecycle.

Skill 1905 toggles independently. Apprentice 1889 is NPC-driven and starts
hidden. `FUN_0044c15c` opens both: 1889 centered and 1905 right at
`centerX + skillWidth*0.5 + 10`. Closing hides the pair and recenters 1905 for
its independent toggle. The master grid in `FieldScene2.bin` is 1894,
type 16, parent 1889, position 31,63, size 191x241, 8x4. ID 6128 is description
control `TML_SKILLM_DESC`, not a grid. Native descriptions must come from the
identified native text/resources; calculated stat labels or generated prose
are not substitutes. Validate content coverage and tooltip position in learned,
apprentice and equipped/quickslot contexts separately.

For login/notice/confirmation messages, identify native panel, texture/alpha,
text writer, visibility and callbacks before changing layout. Compare the same
original scenario; test click/`Esc` without stranded focus. Do not conflate
top-of-screen notices, ordinary chat and player-attributed trumpet messages.

Scene2 `FUN_004A8F14` centers native root 4622 in X/Y; source translation is
`P_SERVER_SEL` 65537. Children, labels, lists and hitboxes follow that root;
do not add a second vertical offset. Proven independent adjustments: channel
width 140, Server `y += 5`, Channel `x += 8`, `y += 5`, and logo branches for
1024/1280/1600. See `flows/ui/server-selection-layout-lifecycle.md`.
Selection overlays must not be replaced by channel-availability bypasses or
weaken server validation after connection.

## Proportional acceptance

Select affected scenarios, not this whole inventory after every edit:

- Server/channel overlay, aligned names and translucent native message panels.
- Correct callbacks, native `Esc` priority and no remaining input capture.
- Character fields, existing controls, no stray black bar.
- Contained ordinary-grid icons; full-region equipment/drag and valid feedback.
- Move/equip/drop and shop price, insufficient gold, sale confirmation.
- Party list/leave callbacks and scene/relogin lifecycle.
- Imported mounts without ABI corruption; correct HUD EXP/ATT/DEF composition.
- Icon identity and native glow across relevant item families; remote appearance
  uses owner/observer creation and incremental update checks when affected.

Use focused tests and incremental build for source changes, asset identity/
loader checks for asset changes. Build-only uses PowerShell 7 and `-NoDeploy`.
Only install/run when in scope and allowed. Record exact executable hash,
loaded assets, resolution and scenario; preserve crash logs/dumps before any
rotation. Test applicable click/drag/valid-invalid destinations/closure/error/
relogin and compare equivalent screenshots. Report each issue as `IMPLEMENTED`,
`STATICALLY VERIFIED`, `AUTOMATED TESTED` or `CLIENT-TESTED`; compilation alone
does not close visual acceptance. Keep blocked or unexecuted checks explicit.
