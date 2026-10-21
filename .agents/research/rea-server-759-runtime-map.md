# REA runtime map: WYD server 7.59 executable and hook DLL

## Scope and inputs

This is an incomplete, static-analysis record for the original server executable
and associated DLLs. The REA 6.7.0 analyses were produced with Ghidra 12.1.2,
12.1.3, and 12.1.4. The source files under `references/` are read-only evidence.

| Input | SHA-256 |
| --- | --- |
| `references/decompilacao server/dlls/TMSRVIN.exe` | `27EE313BB4B4D4A0D897DAC9A188D8249068CE701311CBDA1E24AD28C5E43642` |
| `references/decompilacao server/dlls/Core.dll` | `EB9E12B0BF79C874E8E9525D1DD90F6A5FD553D6FCEC2D475F47439CFCF37D62` |
| `references/decompilacao server/dlls/DBSRVIN.exe` | `3ADB3A1704DF4910604F0B862D69D963F4E1A017EFE5743A23E6F07AD1ECF736` |
| `references/decompilacao server/dlls/DBWYD.dll` | `58F41DA172228DC007D02B620D15B791E36344F7B559002CAC0BD66521384D85` |

The analysis artifacts are retained under the ignored `build/rea-analysis/`
root. They do not modify the input binaries. Findings below are decompiler or
instruction-flow observations, not runtime proof.

At the earlier Core/TMSRVIN checkpoint, the running investigation had examined
150 distinct function entries: 126 in `Core.dll` and 24 in `TMSRVIN.exe`. The
preceding checkpoint recorded 146 entries; that continuation separately
analyzed four previously unrepresented `TMSRVIN.exe` thunk entries:
`0x004B2DBF`, `0x004B31D4`, `0x004B2527`, and `0x004B15A5`.
This is a tally of examined targets, not a complete inventory of either
binary; referenced functions not separately analyzed are excluded. At that
checkpoint, the retained Core/TMSRVIN corpus contained 73 unique
`analyze_function` records (63 in `Core.dll`, 10 in `TMSRVIN.exe`) and 81
directly represented targets when separately returned and bundled procedure
evidence is included. Subsequent saved evidence adds ten non-thunk
`DBSRVIN.exe` function entries and 18 `DBWYD.dll` targets (its entry plus 17
analyzed functions), bringing the prior reconciled tally to 178 examined
entries and 108 directly represented targets. One additional `DBSRVIN.exe`
function entry, `FUN_004616A0`, has since been analyzed, bringing the current
tally to 179 examined entries and 109 directly represented targets.
One further `DBSRVIN.exe` entry, `FUN_0048B920`, was analyzed to resolve a
`DBWYD.dll` patch site, bringing the tally to 180 examined entries and 110
directly represented targets. One more `DBSRVIN.exe` function entry,
`FUN_0047BA20`, was analyzed to trace the guild-record persistence helper,
bringing the current tally to 181 examined entries and 111 directly
represented targets.
One additional `DBSRVIN.exe` function entry, `FUN_0048EAE0`, was analyzed as
the upstream account-record dispatcher, bringing the current tally to 182
examined entries and 112 directly represented targets.
`DBSRVIN.exe!0x487E50` was analyzed earlier and already appeared as a caller in
a message-path dossier
(`ev_ddaf4159fb1d58ef2a2d31c4fe1d28c53c7ce70ac2af53ce45a3eefdcd510ac0`),
so it increases the analyzed-function count but not the unique represented-target
count. These remain checkpoint-derived counts, not a
complete inventory; older raw outputs are not fully reconciled. Six new direct
`Core.dll` procedure records were captured for `0x1002C660`, `0x1002A240`,
`0x1002A37F`, `0x10014980`, `0x10036430`, and `0x10034ED0`, bringing the current
checkpoint to 188 examined entries and 118 directly represented targets. This
supersedes the earlier empty bounded CLI projection for `0x1002C660`. A seventh
new direct record for `DBSRVIN.exe!0x489D50` brings the current checkpoint to
189 examined entries and 119 directly represented targets. Direct decompilation
of `DBSRVIN.exe!0x48F410` adds one more unique function, bringing it to 190
examined entries and 120 directly represented targets. A query at `0x48DE7D`
resolves to the new function entry `DBSRVIN.exe!FUN_0048DE50`, bringing the
checkpoint to 191 examined entries and 121 directly represented targets. A
new direct analysis of `Core.dll!FUN_10021420` adds one unique function, bringing
the checkpoint to 192 examined entries and 122 directly represented targets.
The `DBWYD.dll` callback address `0x10001450` is still only a referenced code
label, not a procedure recognized by Ghidra, and is excluded from both counts.
Eleven additional direct procedure records have since been analyzed: six `Core.dll`
hook helpers (`FUN_10002F00`, `FUN_10002E60`, `FUN_10002D40`, `FUN_10002030`,
`__telli64`, and `FUN_1009F5A0`), `TMSRVIN.exe!FUN_0056A320`, and
`Core.dll!FUN_100248E0`, `FUN_10029FB0`, and `FUN_1002A040`, plus
`TMSRVIN.exe!FUN_004D7140`. This brings the checkpoint to 203 examined entries
and 133 directly represented targets. A further `Core.dll!FUN_1002EE50`
decompilation adds one unique target; `FUN_1002E4E0` was already represented in
the Tower fame analysis. Direct decompilation of `FUN_1002F2D0`,
`FUN_1000C160`, and `FUN_1000C200` adds three unique targets, bringing the
current checkpoint to 207 examined entries and 137 directly represented
targets. Direct decompilation of `Core.dll!FUN_1002DDA0` and the referenced
`TMSRVIN.exe!0x004B2C0C` adds two more entries and represented targets, bringing
the checkpoint to 209 examined entries and 139 directly represented targets.
The direct decompilation of `TMSRVIN.exe!FUN_004E0740` adds one further entry
and represented target, bringing the checkpoint to 210 examined entries and
140 directly represented targets. Direct decompilation of
`TMSRVIN.exe!FUN_00503360` adds another entry and represented target, bringing
the checkpoint to 211 examined entries and 141 directly represented targets.
Direct decompilation of `TMSRVIN.exe!FUN_004DEB60` and `FUN_005029F0` adds two
unique procedure bodies, bringing the checkpoint to 213 examined entries and
143 directly represented targets; the five-byte thunk at `0x004B16EA` resolves
to `FUN_004DEB60` and is not counted as another body.
A direct decompilation of `TMSRVIN.exe!FUN_00581250` adds one unique procedure
body, bringing the checkpoint to 214 examined entries and 144 directly
represented targets. The follow-up `FUN_00503360` dossier expands on a body
already counted above and adds no entry. The direct decompilation of
`TMSRVIN.exe!FUN_00575450` adds one unique body, bringing the checkpoint to 215
examined entries and 145 directly represented targets. The direct decompilation
of `TMSRVIN.exe!FUN_005818A0` adds one unique body, bringing the checkpoint to
216 examined entries and 146 directly represented targets. A caller-address
query resolves the two references to `FUN_005818A0` inside the previously
unrepresented `FUN_00580F30`; adding that unique body brings the checkpoint to
217 examined entries and 147 directly represented targets. Direct analyses of
`Core.dll!FUN_1002D150`, `TMSRVIN.exe!FUN_005755E0` and `FUN_00505610`,
`DBSRVIN.exe!FUN_0047B200`, and `DBWYD.dll!FUN_10002BB0` and `FUN_100016C0`
add six represented bodies. The separately analyzed `TMSRVIN.exe!0x004B196A`
thunk resolves to `FUN_00505610`; it adds an examined entry but not another
body. This brings the checkpoint to 224 examined entries and 153 directly
represented targets. Follow-up reads of the existing `Core.dll!FUN_100248E0`
and `TMSRVIN.exe!FUN_00580F30` evidence clarify the `0x397` branch and the
`0x004B17BC` jump/callsites, respectively, but add no new function entries.
The callbacks at `Core.dll!0x10021210` and `0x100212E0` are not procedure
entries and remain excluded from the tally. This remains a research checkpoint,
not a complete inventory of binary functions.
Direct static analysis of three `Core.dll` procedures in the existing `0x397`
event-registration path adds three examined entries and two newly represented
targets, bringing the checkpoint to 227 examined entries and 155 directly
represented targets. `FUN_1002F130` was already represented as a call target in
the dispatcher evidence, so its direct dossier adds no second target.
One new direct `DBSRVIN.exe` function dossier, `FUN_004900F0`, adds one examined
entry and one represented target, bringing the checkpoint to 228 examined
entries and 156 directly represented targets.
Two new direct `TMSRVIN.exe` function dossiers, `FUN_005656F0` and
`FUN_00567780`, add two examined entries and two represented targets, bringing
the checkpoint to 230 examined entries and 158 directly represented targets.
One additional direct Ghidra analysis examined `Core.dll!FUN_10038280`
(`0x10038280..0x100382C7`). It adds one examined entry but no new represented
target because the existing `FUN_1002F130` evidence already records the call to
it; the checkpoint is now 231 examined entries and 158 directly represented
targets. The new direct decompilation of `TMSRVIN.exe!FUN_0056C360` adds one
examined entry but no new represented target: its thunk call was already
present in the earlier `FUN_00512800` decompilation. This corrects the prior
target tally to 232 examined entries and 158 directly represented targets. A
direct analysis of `TMSRVIN.exe!FUN_00512580` adds one examined entry but no
new represented target because its thunk call is also present in
`FUN_00512800`; the current checkpoint is 233 examined entries and 158
directly represented targets. The direct REA dossier for
`TMSRVIN.exe!FUN_0050BCC0` adds one examined entry and one represented target,
bringing the checkpoint to 234 examined entries and 159 directly represented
targets. The body sets a bit in the global byte `0x61AA207` using the supplied
index masked to five bits; the business meaning of this flag remains unproven.
A further direct REA dossier for `Core.dll!FUN_10020890` adds one examined
entry but no represented target because that call target was already present
in the `FUN_1002C710` dossier. The checkpoint is now 235 examined entries and
159 directly represented targets.
Three further direct `Core.dll` dossiers, `FUN_1001C920`, `FUN_1001A370`, and
`FUN_1001CC10`, add three examined entries but no represented targets: the first
two already appear as callees in `FUN_1002C710`, and `FUN_1001CC10` already
appears as a caller in `FUN_10020890`. The checkpoint is now 238 examined
entries and 159 directly represented targets.
A further direct REA dossier for `Core.dll!FUN_1001C270`
(`ev_5fa1ed6ae16c09f560a22cff9c70e71b887dec654912b6e66bf4264b7842e7f3`,
`build/rea-analysis/work/Core-stream-facet-resolver-1001c270-20261021.rea-snapshot.json`)
adds one examined entry but no represented target because it already appears as
a callee in `FUN_1002C710`. The checkpoint is now 239 examined entries and 159
directly represented targets.
A direct dossier for `Core.dll!FUN_10014DE0`
(`ev_521adfba824c66b81b4874c93528d2f8b59dfe9ff4394aedb96a66b7070bd837`,
`build/rea-analysis/work/Core-locale-facet-key-10014de0-20261021.rea-snapshot.json`)
adds one examined entry but no represented target because it was already listed
as a callee in `FUN_1001C270`. The checkpoint is now 240 examined entries and
159 directly represented targets.
A fresh direct pass over six additional `TMSRVIN.exe` war-lifecycle bodies adds
six examined entries and six directly represented targets. The checkpoint is
now 246 examined entries and 165 directly represented targets. These findings
are static REA/Ghidra evidence; they do not establish that the paths execute in
the live server.
Direct body analyses of `Core.dll!FUN_10011FF0` and
`DBSRVIN.exe!FUN_0048DB80` add two examined entries but no represented targets,
because both were already exposed as call/thunk targets. `DBWYD.dll!FUN_10042830`
adds one examined entry and one represented target; the `TMSRVIN.exe`
`GridMulticast` body at `0x503FF0` adds one examined entry but no represented
target because it was already identified by its war-state caller. The reconciled
checkpoint is now 250 examined entries and 166 directly represented targets.
Direct analysis of `TMSRVIN.exe!FUN_00502250` adds one examined entry but no
newly represented target because its address was already present as an
unresolved call lead. Direct analysis of `FUN_00508270` adds one examined
entry but no represented target because the earlier `FUN_00567780` dossier
already records calls through its thunk. The checkpoint is now 252 examined
entries and 166 directly represented targets. Direct analysis of
`TMSRVIN.exe!FUN_00506240` adds one examined entry but no new represented
target: the call target was already exposed by the mapped `FUN_00566D60` body
at `0x56762E`. The checkpoint is now 253 examined entries and 166 directly
represented targets. Direct analysis of `TMSRVIN.exe!FUN_00502700` adds one
examined entry but no new represented target because its call target was
already exposed by `FUN_00506240`. Direct analysis of `FUN_00572CF0` and
`FUN_00573180` adds two examined entries and two directly represented targets.
The checkpoint is now 256 examined entries and 168 directly represented
targets.
One additional `TMSRVIN.exe` thunk entry, `0x4B2559`, was directly analyzed;
it resolves to `FUN_00572CF0`, bringing the checkpoint to 257 examined entries
and 169 directly represented targets.
A direct body analysis of `TMSRVIN.exe!FUN_00555600` adds one examined entry,
but no new directly represented target: its caller identity and both thunk
call sites were already present in the saved xref evidence
(`ev_bb910a34aceeb0f8808c7ef59392fee18c3c5cd2ca423d34216cba9d60731870` and
`ev_2f07b8d257b521ff5e516aea26283855007a61b2e6ff91d50656051f1b8f5afd`). The
checkpoint is therefore 258 examined entries and 169 directly represented
targets. Direct analysis of `DBSRVIN.exe!FUN_0048D8E0` adds one examined entry
and one directly represented target. The fresh direct dossier for
`Core.dll!FUN_100317E0` adds one examined entry but no represented target because
it was already present as a call target. The checkpoint is therefore 260
examined entries and 170 directly represented targets. Direct analysis also
resolves the war and ally map mutator call chain documented below.

## Executable receive and command path

- `TMSRVIN.exe!0x4B2243`
  (`ev_7121589b122d6e127499fc6aa10977fb128f27e3f7378427a5adb8bfd211cfaf`)
  is the original-executable callback target used by `Core.dll!0x100308B0`
  when a DonateStore purchase succeeds. It resolves to a thunk for
  `FUN_005045E0`, validates an active player slot, builds an 0x18-byte packet
  with opcode `0x182`, copies the supplied item payload and slot fields, then
  passes the packet to `thunk_FUN_004D7850`. This is static evidence of the
  item/inventory notification path; neither runtime transmission nor a stock
  update is established here.
- `TMSRVIN.exe!0x518790`
  (`ev_804c037ce971e4be2857e3cd2c15df659ff91466a787087456148bf43327fad3`)
  handles per-connection socket notifications, receives data, and calls
  `FUN_00519CA0`.
- The saved instruction-flow record for `TMSRVIN.exe!0x519CA0`
  (`ev_14da12a13766379cd884793af6559aec2b578801bd04e37e5a379430c55fb1a8`)
  reads the command word at packet offset `+4`. Its guarded tables and explicit
  branches cover values only up to `0xFDE`; `0x16E2` reaches the shared function
  epilogue at `0x5548F5`, not a command-specific handler. This excludes
  `0x16E2` as a command on this observed player-socket path, but does not rule
  out a separate transport or internal call path.
- `TMSRVIN.exe!0x584C70`
  (`ev_263e7f27646ad7867089a0fbcabe394d2305088eceb94ea43bc044fb80d15b54`)
  resolves inside `FUN_005849B0`, the objective-death handler. It reads the
  defeated actor's template ID and calls `FUN_00585BC0` when it equals
  `0x16E2`. Here `0x16E2` is monster ID 5858, not evidence of a packet opcode.
- `TMSRVIN.exe!0x585BC0`
  (`ev_055f72cf90b85f6ea0cde5a68d33511da5838d6cebdcb2c131d3b49c233a5cdd`)
  has separate settlement paths for the group root and matching group
  members. Member rewards use a four-value random offset from a base derived
  from the root's stored room index, with a boundary reroll. This supports the
  documented native room/group-index pool behavior, not member-list-rank
  selection.
- `TMSRVIN.exe!FUN_0056A320`
  (`ev_3fd2566f6bb317d7f257cf290dc2b95e9f0e02571c6d9608e3e64eb0f68fb4e4`,
  body `0x56A320..0x56A8B4` and `0x56A8B7..0x56A936`) validates a player slot
  and follows a connection-close/cleanup path. For linked slot states it
  snapshots per-user fields and attempts the close helper up to five times,
  sleeping 100 ms between retries. After five failures it logs a critical
  close failure; otherwise it calls follow-up helpers, then transitions the
  slot to `0x18` and invokes update helpers. This is static session-teardown
  evidence; durable persistence and the exact semantics of those helpers are
  not established.
- `Core.dll!0x1002DF30`
  (`ev_8274301e3572279283e70956cd8de083a5a771fde1fd353c63c7089ee5f3e7db`)
  computes `DAT_1074EBDC + 0x191E`. With startup value `0x4B1000`, this reaches
  executable thunk `0x4B291E`, which jumps to `TMSRVIN.exe!0x514310`.
  `GenerateMob` at that address validates its first argument as an index in
  `0..6999` and uses a `0xDC8` record stride. The returned decompilation was
  token-limited; it does not cover every spawn branch.
- `Core.dll!0x10008CF0`
  (`ev_a65cf9c4046d2f469d434e8efa9e53500588a9828d25da8f7df4a1fac742818a`)
  is a guarded boss-teleport callback. It invokes a player helper with
  `0xD13/0x535`, calls `GenerateMob(0x16E2, 0, 0)`, then scans up to twelve
  candidate slots and notifies eligible nearby players. This is another
  monster-ID use, distinct from the packet command field.

More detail on the Rune Track settlement comparison is in
[`../../DOCS/server/rune-track.md`](../../DOCS/server/rune-track.md).

## DLL startup and worker map

- Focused body `Core.dll!FUN_10011FF0`
  (`ev_1cf00f57bb4ba807d5a559b15b7f555d87c918e6abf4702cc7a0758bd05ec427`,
  body `0x10011FF0..0x10012036`, 71 bytes,
  `build/rea-analysis/work/Core.dll-FUN-10011FF0-REA-function-20261021.json`)
  calls `__CheckForDebuggerJustMyCode` and returns a DWORD from the context at
  offset `+0x14`. Its observed callers are `FUN_100110F0` and `FUN_1001D2C0`;
  this is an accessor/debug-check helper, not evidence of worker dispatch or
  lifecycle behavior. It was already exposed as a call target in earlier
  evidence, so this direct analysis adds an examined entry but no unique target.
- `Core.dll!dllmain_dispatch`
  (`ev_9b4a635add334ee22c72c6ac1c59c1b917017e6ad0f52289a2e1764e8aab851f`)
  forwards attach processing to `FUN_1003AC70`.
- `Core.dll!0x1003AC70`
  (`ev_99ad657e51fc3903c4ac929df2d1a91421954dabf8bc226b55c8f59c4eb48d6d`)
  disables thread notifications and, if `FUN_10039FA0` passes, creates
  workers at `0x100110F0` and `0x10011A60`.
- Worker `Core.dll!0x100110F0`
  (`ev_e70505cab7acc652eca7ef43c57549f3999b578abc87a04b6bb61fce984c07cb`)
  is an infinite maintenance loop with `Sleep(1000)`. Each pass obtains a
  broken-down time value through `0x10012090`/`0x10012040` and scans slot
  indices `0..999`. The observed paths include Colosseum notices and
  registration/area checks, Kefra-area kingdom and ticket checks, and pending
  cash/gift delivery. In the cash path it adds the queued amount to a per-slot
  global field, notifies the slot, and logs the delivery. The cash reader's
  per-player file path is now established below; who creates or updates those
  files remains unresolved.
  A focused complete-function analysis
  (`ev_e8b66bb1937c0595a391a4dbc788d3d5a8746181d5b59e959330f1c95dcc4708`)
  shows the cash branch under input opcode `0x37`: it obtains a positive
  per-player value through `0x1002FDD0`, invokes the void helper
  `0x100314B0(0, player)`, then adds the value to the global balance-like field
  at `0x10194A38 + slot*6000`, sends a notice, and logs the delivery. The
  worker body does not visibly clear the value returned by `0x1002FDD0` in
  this branch. The gift branch reads shared globals `0x10194A14` and
  `0x10194A18`, finds an available item slot, writes the two values to the
  selected character record, and invokes `0x100308B0`; the configuration or
  source for those globals is identified below. These are static observations,
  not proof of successful delivery or durable persistence.
- Gift configuration loader `Core.dll!0x1002F520`
  (`ev_d71444a01c53054ae0c4a9fb07a6e8361383727baee63612f061f45cc564a266`)
  reads `Gift/ItemId`, `ItemEf1`, `ItemEfv1`, `ItemEf2`, `ItemEfv2`, `ItemEf3`,
  and `ItemEfv3` from
  `../../WYD_DB/FileConfig/AutoTradeEvent.ini`. It clears the eight-byte gift
  globals and packs the configured item ID/effect bytes into
  `0x10194A14..0x10194A1B`, which the worker later copies into a free character
  item slot. The loader is called during startup by `0x10039FA0`, invoked by
  `0x1003AC70` before the startup-success path
  (`ev_f2d37226248f4231488fcfdc188b2089a4f04ec15e6ea87601e523ae791b9c9a`).
  The second call at `0x10016FAB` is a manual reload branch in the large,
  non-contiguous `FUN_10015790` dispatcher. Raw instructions
  (`ev_1b3a72281d91793532162b56ca85a4387f1985bbd3fb322ed014e7d7121826f9`)
  show the dispatcher comparing its input with the literal at `0x10152098`;
  read-only PE `.rdata` mapping resolves that literal to `+readautotrade`
  (RVA `0x152098`, file offset `0x151498`). On the matching branch it calls
  `0x1002F520`, then sends the message at `0x101520A8`, `AutoTrade reloaded!`
  (RVA `0x1520A8`, file offset `0x1514A8`), to the invoking player slot.
  This reconciles the labels: the command reloads `AutoTradeEvent.ini`, whose
  parsed `Gift/*` fields repopulate the gift globals. It does not establish
  successful durable writes or delivery after reload. The direct caller list
  is recorded in `ev_4047f56322912978d21f6e6613dbc93a8cfe944b86d4c5e11736c59ae0593743`.
- Cash reader `Core.dll!0x1002FDD0`
  (`ev_24576e2652d89f30648d9f5b24e688528a7319c5c4110cc3bfc2669c7fcd78fe`)
  formats `../../WYD_DB/Cash/%s.txt` from its string argument, opens the path,
  initializes a 32-bit value to zero, and passes its address to
  `0x1002C660` when the stream is readable. The decoded instruction flow then
  reloads that value into `EAX` after stream cleanup; the unreadable/error path
  returns zero. This resolves the apparent `void`/caller-result mismatch in
  Ghidra's pseudocode: the worker's `0x1002FDD0` call does receive the value in
  that local. Direct REA records now show `0x1002C660`
  (`ev_f6abded903ce598d864954a1795235dd7387dee64da969e8fe90311487201d0d`)
  copies the incoming 32-bit value into a local, calls `0x1002A240`, then copies
  the updated local back. `0x1002A240`
  (`ev_8078771ed141a2d248b6720daeae7997d768c692d0b26c8450785d252bbc26e8`)
  obtains a cached facet through `0x1002B4E0` and calls the stream wrapper
  `0x10036430`; the wrapper dispatches vtable slot `+0x28`
  (`ev_f9fd5bb4599eabbb84449fe1dc0c2d0665a3ff09d3b1cb6668113dc58cf9a1e5`).
  The `num_get<char, istreambuf_iterator<...>>` vtable has `0x10034ED0` at
  `+0x28` and `0x10034DF0` at `+0x2c`
  (`ev_8d7f84b60bbcd3d861dee142b323a4865852db4d754e2e25ed6320d711bcb332`).
  The `+0x28` implementation `0x10034ED0`
  (`ev_449f8b01192da0b75a49f7bc1491ec34a8c9f325e04840508b81386494318157`)
  collects a formatted numeric token through `0x10033590`, applies stream-format
  flags, and calls signed CRT `__Stolx`; the `+0x2c` implementation is the
  separately mapped unsigned `__Stoulx` path. This establishes a signed numeric
  extraction path for the queued Cash value, but not the exact textual grammar,
  validation policy, or contents of any particular Cash file. The other direct
  helper records are `0x1002A37F` (`ev_9392ad96c1dc024e46a06f8c3b0fb8a0f8fa89fce6d0250ca23d9de177b071b0`)
  for cleanup and `0x10014980`
  (`ev_a3c35b2addefcf8a8b201694730457bb978f6613c8f24b7e9b300654271244ff`),
  whose precise semantic role remains unresolved. The filename's substituted
  identity follows the worker's per-slot player argument; the file producer,
  write cadence, and deletion/consumption policy are also unresolved. Static
  file access does not prove runtime delivery or durable persistence.
- Cash companion `Core.dll!0x100314B0`
  (`ev_29744195fc98e9ce9ee4556771f2fba4d3c01bcdb8cbd3fcf9268da83c08f6d5`)
  is `void`, not a boolean predicate. It formats the same per-player Cash path,
  calls `0x10036A00(path,0x12,0x40)` and `0x10036970(path,3,0x40)`, then on its
  observed stream-state branch calls `0x1002C710(param_1)` and
  `0x10034980()` before cleanup. In the worker call, `param_1` is zero. The
  analyzed open wrapper delegates to CRT `Fiopen` through `0x10036860`; its
  `0x10018D70`/`0x10018E50` calls update and validate stream state, not the cash
  contents. `0x1002C710`
  (`ev_10f6eb4afcea486a6924a649072a905f1648419d646f55079f625a6f2b05ff79`)
  calls `0x10020890` with its incoming value as the last argument. The direct
  dossier for `0x10020890`
  (`ev_efdf9e18ef140144fad07f7327a8b1b787130ae6958b87fdbb0c5eb23d430c21`,
  saved in `build/rea-analysis/work/Core.dll-cash-helper-10020890-20261021.rea-snapshot.json`)
  is a 117-byte wrapper that forwards six arguments, including that value, to
  an unresolved virtual call through slot `+0x20`. This supports a stream/facet
  formatting operation but does not identify its concrete implementation or
  prove a write, clear, or consumption of the Cash file. `0x10034980`
  (`ev_6271484f82f30be55ea79b380308f49e42a5905accb7daae5d064d3cd82fe7cb`)
  checks another helper and, on a zero result, calls `0x1001AA90(2,0)` to
  update stream state. This chain does not prove that the queued amount is
  consumed, reset, or protected from repeat credit; do not infer any of those
  behaviors from the helper call alone.
  The direct dossier for `FUN_1001C920`
  (`ev_11dd8083bf523117ec1c632b95da5f3abfe66a7c312af8222b1fc58ac022a4b9`,
  `build/rea-analysis/work/Core.dll-cash-locale-facet-1001c920-20261021.rea-snapshot.json`)
  shows an 85-byte helper returning the pair `{0, param_1}` through a hidden
  aggregate-return pointer. Its input is obtained through
  `FUN_1001A370` (`ev_d1c1bfd2feb324f5857068af26c967c2e62c845d90eb0256c882d56aea1a7735`,
  `build/rea-analysis/work/Core.dll-cash-facet-origin-1001a370-20261021.rea-snapshot.json`),
  a 71-byte getter for the DWORD at `this+0x38`; the owning object's identity
  and that field's meaning remain unknown. A second direct caller,
  `FUN_1001CC10` (`ev_99669acacf1d1a62a8e5329c137d664f15230e5a3de87147e54983e29e4a0ddc`,
  `build/rea-analysis/work/Core-dynamic-dispatch-caller-1001cc10-20261021.rea-snapshot.json`),
  takes a `ushort`, builds the same pair, and invokes `FUN_10020890` with that
  value and stream-related state. This confirms reuse of the wrapper in a
  separate formatted-value path; it does not resolve slot `+0x20` or establish
  a file write in the Cash caller. `FUN_1001C270`
  (`ev_5fa1ed6ae16c09f560a22cff9c70e71b887dec654912b6e66bf4264b7842e7f3`,
  `build/rea-analysis/work/Core-stream-facet-resolver-1001c270-20261021.rea-snapshot.json`)
  selects a cached `_Facet_base` using locale-related helpers and, on a cache
  miss, creates, registers, and initializes a facet under `_Lockit`. Its callers
  are `FUN_1002C710`, `FUN_1001CC10`, and `std::locale::_Locimp::_Makeloc`.
  Although Ghidra inferred a `void` prototype, the instruction path preserves
  the selected facet pointer in `EAX` across cleanup, and both formatting
  callers use that result as the receiver passed to `FUN_10020890`. This narrows
  the receiver to a locale facet; its concrete subclass and the implementation
  behind virtual slot `+0x20` remain unresolved, so the Cash path still does not
  prove a file write, clear, or consumption.
  `FUN_10014DE0`
  (`ev_521adfba824c66b81b4874c93528d2f8b59dfe9ff4394aedb96a66b7070bd837`,
  `build/rea-analysis/work/Core-locale-facet-key-10014de0-20261021.rea-snapshot.json`)
  lazily assigns and returns a monotonically increasing DWORD from global
  `0x1074ecd8` under `_Lockit`. `FUN_1001C270` passes the address `0x1074de78`
  to it, then passes the returned index and locale argument to
  `FUN_10017B90`, whose recovered type is `_Facet_base*`. This confirms an
  indexed locale-facet lookup, but the identity of that static ID and the
  concrete facet subclass remain unknown.
- A defined-string search (`ev_f8a3313b2598180860ce68787bffe336ff704fc7fdd78145f33d6a9f40d0e7d5`)
  found two Cash paths: the queued-credit path above, `Cash/%s.txt`, and a
  separate `Cash/%d.txt` path, plus the `openCashStore` label and administrative
  Cash command strings. Address xrefs for `Cash/%d.txt`
  (`ev_a1f0cae8eea2f16aa35652ee47840a1b7047aa5de9d86a7cbac5ec6b37623cf7`)
  resolve to one use, inside `0x100317E0`.
- Cash file routine `Core.dll!0x100317E0`
  (`ev_63c85df9c1e318e672f97c27533cdc1584d3a01aa3256a940904bd124182fe44`;
  fresh direct dossier `ev_3f0193eb3a4be66f7ff3351257c1dbc2f22e79c2b30540d97fa094751b3e928c`,
  `build/rea-analysis/results/core-cash-classification-100317e0-exact670-20261021T145329476/Core.dll-FUN-100317E0-REA-evidence.json`)
  formats `../../WYD_DB/Cash/%d.txt` from its second argument, opens the path
  through the same stream wrappers, and calls `0x10012200(buffer, first_arg)`
  only when the stream-state check succeeds. Its only observed caller is
  `0x10015790` at `0x10015A9D`. Static call-target evidence
  (`ev_ee48f0bbd69198bd8974e15447e79b1d78c5e6fcd6dd751b3532509f0d9b4804`)
  confirms a direct call, not runtime execution. Raw instruction evidence
  (`ev_1b3a72281d91793532162b56ca85a4387f1985bbd3fb322ed014e7d7121826f9`)
  recovers the caller arguments: it pushes a pointer to a local 0x60-byte
  buffer, then a zero-extended 16-bit field at offset `+0x12` of the caller's
  object. Thus that field becomes the `%d` filename component; its semantic
  identity is not yet proven. On the successful parse/length path, the caller
  passes the same local buffer to `0x100317E0`, then formats the buffer with
  `0x100375F0` and sends the resulting text through `0x10030560` to the player
  slot. This establishes a file-backed response path, but the transfer
  direction (read, write, or update) and the file's semantic meaning remain
  unresolved; it is not evidence of a direct balance write. The body does not
  establish a purchase or producer for the file; do not conflate this path with
  the `%s.txt` queued-credit path.
- Transfer helper `Core.dll!0x10012200`
  (`ev_1ce5a33d8ce178b03a686f89fe9ec818dd5c7eb6aca023a71529d1151346f391`)
  traverses a sized range through virtual stream-like operations and checks
  transfer state. Its arguments and generic transfer behavior are visible, but
  the caller-specific data contract and whether this represents a read, write,
  or update of Cash state remain unresolved.
- Worker `Core.dll!0x10011A60`
  (`ev_bfadfd5535310739c7d7d01ce5288a9b3caa0a8f2a004abe4b61d7799d9c237d`)
  is a second infinite loop with `Sleep(1000)`. It reads the same time-helper
  pair and manages Tower of Thor scheduling: the observed gate checks weekday
  values `1..5` and two global server/world enable pairs; it announces five
  minutes before the configured start hour, invokes the spawn callback at the
  configured start, and ends/reset-clears the tower state at the configured
  end if no capture has completed. A capture countdown is decremented when the
  current second matches a global trigger; the completion path invokes several
  state-update routines, clears fields on matching `Thor's Tower` objects in
  indices `1000..0x31ff`, and broadcasts the guild victory. The no-winner end
  path also clears those object fields. The completed-defense path adds the
  configured `DefendTowerFame` amount to the defending guild's current value
  (details below). Timezone, schedule configuration source, durable
  guild-state persistence, and the identities represented by the enable pairs
  remain unverified. The same worker also toggles the `LastServerBoss` setting
  at local `23:59:55`; its static file read/write path is traced below. The
  configured timezone and runtime execution remain unverified.
- `Core.dll!0x10039FA0`
  (`ev_f24ec404a7b6e2a51d1f9770a279f4eea5fbe8aed7832105c1519877da65bfea`)
  is the gate behind that worker creation, but is not merely a boolean check:
  it initializes console output and a sequence of subsystems, then calls four
  routines (`0x1003A150`, `0x1003A320`, `0x1003A950`, `0x1003A4F0`) in order.
  A false result from any routine returns failure immediately. Only after all
  four succeed does it run a second initialization sequence, print
  `[SERVER STARTED SUCCESSFULLY]`, and return success. Static analysis now
  identifies the role of all four gates; live acceptance behavior and
  dependence on deployed configuration files have not been tested.
- `Core.dll!FUN_10021420`
  (`ev_1dfe05f034f37cf03a8fe4dbd83df0bf3f4af80d8d98954f02326feb2a282f0d`,
  body `0x10021420..0x1002149B`) is called late in the successful
  `FUN_10039FA0` startup path, after that routine sets `DAT_1074EBDC` to
  `0x4B1000` (`ev_f24ec404a7b6e2a51d1f9770a279f4eea5fbe8aed7832105c1519877da65bfea`).
  It calls `FUN_10002F00` with derived target addresses `0x519D90` and
  `0x4D7219`, code labels `0x10021210` and `0x100212E0`, and lengths 4 and 2.
  This call pattern is consistent with two startup hook registrations. Static
  callback paths and their direct callees are now traced below; successful
  installation and runtime effects remain unconfirmed.
- The two callback addresses above are not procedure entries recognized by
  Ghidra. Address-based instruction evidence decodes `0x10021210`
  (`ev_efd8c13d9805519b1612b281b369aa2a370161bef5aa0f320e198f590410f87d`)
  and `0x100212E0`
  (`ev_d0db89df048d5447a562977710e348225ea6414f165d608f10064f6da60ca899`)
  as the same first x86 instruction, `MOV EAX,[0x1074EFA0]`. Ghidra records
  incoming data references from `FUN_10021420` at `0x10021457` and
  `0x10021477`, respectively. Further instruction evidence at `0x10021215`
  (`ev_bc9e4a294b59537ab6432b67fb112c6a6ad528b8cb0254fedb49835fe8b5d601`)
  and `0x1002121C`
  (`ev_bd2938114698fd1b692eb307752c93055802fbe3714fc10a2c0c2a58db2055a3`)
  shows a TLS-vector read (`FS:[0x2c]`) followed by an indexed per-thread
  pointer read. These are static instruction/data-flow observations, not
  runtime confirmation.
- The callback at `Core.dll!0x10021210` reads the TLS-associated value and
  calls `FUN_100248E0` (`ev_09753ead2133c462bb1a7130127ff5ff42f955bacd52939a2fb176706e3b82ac`,
  body has 18 non-contiguous ranges totaling 10,807 bytes, spanning
  `0x100248E0..0x1002735C`) with the thread slot and packet pointer. The callee
  reads the packet opcode from `param_2[2]`, stores the thread slot in
  `param_2[3]`, and branches on opcodes. The recovered pseudocode contains 18
  direct equality-comparison values: `0x20D`, `0xB01`, `0xB05`, `0x291`, `0x373`,
  `0x3A6`, `0x3B5`, `0x39D`, `0x39E`, `0x367`, `0x397`, `0x27C`, `0x380`,
  `0x36C`, `0x366`, `0x368`, `0x28F`, and `0x2D2`. This is a list of direct
  comparisons in the decompilation, not proof of every packet accepted by the
  hook or the complete wire protocol. Static callback disassembly routes a
  true result to the original continuation `TMSRVIN.exe!0x519D99` and a false
  result to `0x5548F5`, the shared server-function epilogue. This supports a
  custom packet gate/dispatcher interpretation; it does not prove that the
  hook was installed or exercised in a running server.
- In the hook's `0x28F` branch, `FUN_1002EE50`
  (`ev_6141611953545d6d435b4d56b3363e001c4ad6c1e2d7dc150c903cab30a45dd1`)
  resolves the injected thread slot to a player-record address using a
  `0x928`-byte stride. The branch reads a 16-bit field at player-record offset
  `+0x12`; when nonzero, it passes that value to `FUN_1002E4E0`
  (`ev_7c77fa6d5e1aa01e075f54d62d68905b25619f3df6b7f47aaa4469357bf25585`).
  That helper reads the first DWORD of the indexed `0x3C`-byte record in the
  fame table based at `0x4D7F1A8`, matching the guild-fame getter already
  traced below. If this value is below 100, the hook sends its fame-required
  error and returns false, preventing the original continuation. A zero field
  skips this check. The branch does not debit or otherwise mutate fame, and the
  packet's native operation beyond this pre-dispatch gate remains unresolved.
  This is behavior in the examined hook DLL, not evidence that the untouched
  executable independently enforces the same gate.
- Opcode `0x397` at `0x10026A7B` enters an event-registration branch in the
  same dispatcher (`ev_09753ead2133c462bb1a7130127ff5ff42f955bacd52939a2fb176706e3b82ac`).
  It bounds the player slot, reads a 32-bit event key at player-record offset
  `+0x14`, and scans other active slots (`state == 0x16`) for the same key.
  The branch also processes event state, checks a global duplicate-registration
  condition, and gates on `FUN_1002F130()` for novice-channel eligibility.
  The event identity and key semantics remain unresolved; this does not prove
  a city-war or Tower-War path, fame accounting, settlement, or rewards.
- Three direct procedure dossiers further resolve calls from this branch
  (`Core.dll` SHA-256
  `EB9E12B0BF79C874E8E9525D1DD90F6A5FD553D6FCEC2D475F47439CFCF37D62`;
  Ghidra 12.1.2 headless evidence in
  `build/rea-analysis/results/Core.dll-event-registration-1002f130-Ghidra-evidence-20261021.json`):
  `FUN_1002F130` (`0x1002F130..0x1002F1F6`) calls `FUN_10038280` and
  `FUN_10038120`, then computes a modulo-based comparison using the returned
  local value and globals at `0x6472B4` and `0x6472F4`. The dispatcher calls it
  at `0x10026C98` for novice-channel eligibility; the constants' meaning and
  the helper's precise semantic result remain unresolved. `FUN_10028100`
  (`0x10028100..0x1002814E`), called at `0x10026B5A`, forwards its argument to
  `FUN_10023C00`; its wrapper role is established, but the argument's meaning
  is not. `FUN_10028150` (`0x10028150..0x100281A5`), called at `0x10026B67`,
  returns the difference between two pointed-to 32-bit values divided by four.
  The pointed-to structure and the returned count's meaning are unknown.
  These procedures establish static call/data flow only and provide no evidence
  of fame accounting, war settlement, or runtime behavior.
  A direct Ghidra dossier for `FUN_10038280`
  (`0x10038280..0x100382C7`, evidence file
  `build/rea-analysis/results/Core.dll-event-registration-10038280-Ghidra-evidence-20261021.json`,
  SHA-256 `9B59F8B995680B9E0F7D13A481C95AEF99C8A1C7948EB2D32439181BE2EBC1FB`)
  confirms it is a thin wrapper around `__time64` at `0x100D99D0`. Besides the
  event-eligibility caller `FUN_1002F130` at `0x1002F166`, it has a caller at
  `FUN_100311C0` / `0x100311F4`. This resolves the helper as a timestamp source,
  but not the event key, event identity, or war semantics. Since the call was
  already present in the preceding `FUN_1002F130` evidence, this adds an
  examined entry without increasing the unique represented-target count.
- `Core.dll!FUN_1002D150`
  (`ev_54636688e3919df3f993aa901a918274000071325519984314a511770a0121ac`,
  `0x1002D150..0x1002D205`, 182 bytes) is called by `FUN_100248E0` at
  `0x10026E4B`. It compares a thread-local value with `DAT_1074DF1C`, performs
  lazy initialization when needed, then invokes the initialized indirect
  callback with its four arguments unchanged. Although the recovered C
  signature is `void`, the assembly preserves the callback's `EAX` result;
  the caller rejects values above 6. This establishes callback dispatch and
  result gating, not coordinate validation or city-war/Tower-War semantics.
- In the same hook callback, opcode `0x3A6` enters a separate item-composition
  path (`FUN_100248E0`, `ev_09753ead2133c462bb1a7130127ff5ff42f955bacd52939a2fb176706e3b82ac`).
  When `DAT_10194A00` is zero, the recovered branch returns true before item
  processing. Otherwise it validates six supplied inventory-slot references
  against the player's current records, rejects invalid or mismatched item
  data and item IDs above `0x1964`, and applies the configured minimum-refine
  checks (`DAT_10194A04`) to eligible slots. `FUN_1002F2D0`
  (`ev_0afbd30812d389a2a4fb5c87a94606c754c148ca192eacde205dbc3ae50dfd5f`)
  performs exact item-ID membership against an integer range: its accessors
  `FUN_1000C160` (`ev_85eae6767f45d1495c460646e5b852be02d7635b4fe56c7355c69f7929c161ff`)
  and `FUN_1000C200` (`ev_c6771c03f1b90c71b17a10a2ef60d9fa2db6ca380ae053abf4ecab0dee66a696`)
  read fields at offsets `+4` and `+8` from their implicit object, respectively.
  The range's owner and contents are unresolved; this is not enough to call it
  a grade table. Ghidra records four references to `FUN_1000C160`, at
  `0x100085DB`, `0x100093A2`, `0x1002D582`, and `0x1002F302`
  (`ev_76410d47ab81932561ef7cd80e38e0b191b5ff65ec30fce6c90bb52074d2cf3e`).
  The branch also checks target-item metadata against two base IDs, accepts a
  selected jewel ID in `0x989..0x98C`, consumes five ingredient records before
  resolving the outcome, then compares `rand()%100` with `DAT_10194A08`.
  Failure clears the target slot; success constructs the output item and both
  paths resynchronize inventory. The configured threshold's value is unknown,
  so no effective probability is asserted. This supports an item-composition
  feature in the hook, but no evidence yet links it to the Rune Track; keep the
  two systems distinct unless a call/data bridge is found.
- The hook's opcode `0x3B5` branch is another distinct item-composition path
  in `FUN_100248E0` (same evidence). It validates up to seven submitted
  inventory-slot references against the live 8-byte records and rejects
  duplicate slot IDs. The two base items must each have refine value at least
  9; the branch requires at least 50,000,000 in the player's balance and checks
  four jewel positions for IDs in `0x989..0x98C`, requiring those IDs to match.
  Although later code contains count cases for one through four components,
  this recovered validation loop increments the count for all four positions;
  fewer-component reachability is therefore unresolved. The visible count
  thresholds are `4`, `8`, `16`, or `32`, compared with `rand()%101`, so no
  effective success probability is asserted. The 50,000,000 balance is
  deducted before the roll, on both success and failure. On the failure path
  it clears and updates five component slots. On the success path it randomly
  selects one of the two base items, consumes both base items and the
  components, computes an output item, and sends inventory updates. The branch's own
  diagnostic identifies this as an Aylin operation. This is a separate
  gold-based item feature in the hook, not evidence of a Rune Track mechanic
  or of gold participating in the war wager system.
- The same callback groups opcodes `0x39D`, `0x39E`, and `0x367` into a
  rate-limited target-validation path (same `FUN_100248E0` evidence). It scans
  up to 13 target entries subject to packet-size gates, checks target IDs and
  player-to-target state, and treats an entity whose name matches the Thor
  Tower label specially. That case clears the submitted target fields when a
  configured flag is set and the player's guild field matches a configured
  value; it also rejects players with no guild or whose accessor result does
  not match the current channel/city value. The latter two cases invoke
  `FUN_1002DDA0` (`ev_b058fc1d02192571a6270307fc77f3d7810f2373abba4428981b6b0195d7a99d`),
  a Core hook wrapper that calls the original executable at `0x004B2C0C`.
  That target (`ev_d5ba7f770b107390ff7e22515027da5fcc6966c38cfb32d5693bb244ec35e203`)
  derives a region index from the player's state, chooses coordinates using
  `rand()%15` and region-specific bases, and may replace them with one of five
  configured coordinate pairs selected by a player field. It passes the result
  to `FUN_004E0740` (`ev_f28d32b07f1419c10e4459aeff64b53733096823eb40229c0a47e80755c177b6`),
  whose diagnostic format identifies it as `GetEmptyMobGrid`: it validates the
  coordinate range, accepts the player's current cell, or searches expanding
  square neighborhoods through radius three for an unoccupied, non-`0x7F` grid
  and updates the coordinate pointers. The caller then passes those coordinates
  to `FUN_00503360` (`ev_6df810e4d612c89bbec1b4cefbd0acf040b60505f5d9a49eb785856fcf2a5710`).
  That routine validates the entity and coordinate ranges, clears its old grid
  occupancy, marks the new cell, processes neighboring entities, and writes the
  new coordinates to the live entity record. The hook therefore invokes an
  authoritative server-side reposition to a nearby free cell. The exact packet
  side effects of the neighboring updates remain unresolved; this does not prove
  a map change, a kick, or a complete tower-war rule. The per-player
  throttle uses `_clock()` and a `+600` comparison; its wall-clock duration is
  not established. This is direct evidence of guild/citizenship validation
  for a Thor Tower target in the hook, but not a complete tower-war flow, a
  fame wager, or independent enforcement by the untouched executable.
- The callback at `Core.dll!0x100212E0` calls
  `FUN_10029FB0` (`ev_8e7669d575805e5cd44a4645ee48cbbd4f46cedcfedb79375ed949e0b4a6495f`,
  body `0x10029FB0..0x1002A039`) with a 16-bit-value pointer, a context value,
  and a size-like third argument. The callee also calls `FUN_1002A040` with a
  derived global address, the context, and `0xFC8`. `FUN_1002A040`
  (`ev_a8e1d914da639a7b9bf68426fd4767d602cdc567f7f5221af9e2cbe562e84541`)
  returns `UINT_MAX` if any input is zero; otherwise it returns the unsigned
  address difference divided by the third argument. Here that is an arithmetic
  record-index calculation using `0xFC8` as the stride. Its result is stored
  but not subsequently read by `FUN_10029FB0`. That function compares the
  pointed 16-bit value with its third argument, zeroes the pointed buffer for
  that many bytes when they differ, and returns the equality result. In the static
  callback path, true sets local `-0x68` to `4` and resumes at the original
  continuation `TMSRVIN.exe!0x4D7220`; false jumps to the function epilogue at
  `0x4D7396`. The containing routine confirms this is its 16-bit packet-length
  field; the external protocol contract, the purpose of the unused record-index
  calculation, and runtime behavior remain unresolved.
- The containing routine is `TMSRVIN.exe!FUN_004D7140`
  (`ev_1c393b7db98368142370599881e281c9b781135532c39ed41e9e8ab4517c137f`).
  Its arguments are a packet-buffer pointer, a length, and a one-byte mode;
  ECX supplies its output-buffer context. Before the hook point, it writes the
  length into the packet's first two bytes. The hook therefore checks that the
  16-bit packet length matches the supplied length before the original routine
  continues its byte transform, checksum, output-buffer append, and flush path.
  On mismatch, `FUN_10029FB0` clears the input buffer for the supplied length
  and the callback returns false through the routine epilogue. On a match, the
  callback restores the original loop counter to `4`, so processing resumes
  after the four-byte header. Because the routine writes the low 16 bits of
  the supplied length immediately before this check, static control flow
  implies lengths above `0xFFFF` fail this equality check (assuming a
  non-negative length reaching the hook). This is static evidence; external
  callers and live behavior have not been validated.
- `FUN_10002F00`
  (`ev_28fd060ea8a6152f580ae224aa5b45041dd6ef5b5ed5bd141d312d6ab5f46dfb`)
  calls `FUN_10002E60` and then `FUN_10002D40`. `FUN_10002E60`
  (`ev_11f2d7a8f0204390da978005fa62e1f41c629e708dad38d848f78f2b06a8e1e6`)
  prepares an `E9` jump opcode; `FUN_10002030`
  (`ev_1e5232f0eb98e7f87b8dfd3e5286a0549d39425ab83834ee879dd869bf4a7312`)
  computes a relative displacement and copies a five-byte jump to the target
  under a temporary page-protection change. `FUN_10002D40`
  (`ev_3ed9eceda65f452d38adb1172a6b302b6f1dc41758cc720e425b0257997d7058`)
  writes requested NOP bytes and copies them using overlap-aware
  `FUN_1009F5A0`
  (`ev_f41a06703bca01d1ed01ccd57b034039b0ce107c24f2623ef48d5dcf66de3834`).
  Both paths use `__telli64`
  (`ev_63e7d1436734c79f949fa54448c4306f48e096b9cc0351f06176c1c110868105`),
  which resolves to a CRT file-position routine, while treating its return as
  an apparent address; the observed patch lengths are 5, 4, and 2. The reason
  is unresolved. These static code shapes do not establish successful hook
  installation or operational safety.
- Tower War configuration initializer `Core.dll!0x1002FA90`
  (`ev_138ea158f09bd7218c1cf3a40f7387ba422365eb6b6c245bb8d5bad8b4392055`)
  is called from `0x10039FA0` before the four startup gates. It reads
  `../../WYD_DB/FileConfig/TowerWar.ini`, section `Config`, using
  `GetPrivateProfileStringA`/`GetPrivateProfileIntA`: `TowerName` defaults to
  `Torre_de_Thor`; `StartHour` and `FinishHour` default to `0`;
  `DestroyTowerFame` and `DefendTowerFame` default to `0`; and `MinutesToWin`
  defaults to `15`. It initializes the corresponding global state. This
  establishes the file path, keys, and fallback values compiled into the DLL,
  not the actual deployed INI contents or whether the server resolves that
  relative path successfully.
- Tower War fame accounting is traced through four additional functions:
  - `Core.dll!0x1002E4E0`
    (`ev_d736d22fd58c848b9b3fd99d0be610e5448576e0c98bf042be3eba45cf5bcee1`)
    returns the first DWORD from a 60-byte record indexed by a 16-bit ID using
    `0x4d7f1a8 + (id >> 12) * 0x3c000 + (id & 0xfff) * 0x3c`. The Tower worker,
    the monster-death handler, and other callers share this getter.
  - `Core.dll!0x10030C70`
    (`ev_8b93b93afceaff515e24879f7351343cc55fc6365ec00e443bc9b1c0628dd5ce`)
    writes the supplied value to that same per-ID DWORD. It constructs a
    `0xC17` update and passes it to `0x10030660`; it also has a separate `0xC23`
    path for matching entries in a ten-slot table. The meaning of that
    secondary path's discriminator values `7` and `8` is unresolved.
  - `Core.dll!0x100313C0`
    (`ev_6914701aa6a3967536ff407e7ec63e3a92b653069d192b92a3b053f35d82fdb3`)
    builds a `0xC17` message containing the ID and current getter value, then
    passes it to `0x10030660`.
  - `Core.dll!0x1001D2C0`
    (`ev_f6dc1ef76f9b4d9a81620a69cde172f8812dfe2947c81f2f9a4753f3ded558de`)
    is a monster-death handler. In its active Tower-of-Thor branch, it obtains
    the killer's guild ID from the killer record at offset `+0x12`; when that
    ID is nonzero, it adds the configured `DestroyTowerFame` amount to the
    current per-ID value, calls `0x10030C70`, then `0x100313C0`. The Tower
    worker's completed-defense path similarly adds `DefendTowerFame` and calls
    those update routines. This supports additive guild-ID-keyed fame changes
    for tower destruction and successful defense. The functions observed so
    far do not establish whether or how this value is persisted durably.
- The Tower War update transport is now traced across the DLL/executable
  boundary:
  - `Core.dll!0x10030660`
    (`ev_5c58999ae2c802af01eac4240584ffd935160f5fb8aa50cfe47ffb558be0780f`)
    loads `ECX=0x06584FF8` and calls executable address `0x4B16AE` with the
    packet pointer and length. This is the shared helper used by the `0xC17`
    fame updates above.
  - `TMSRVIN.exe!0x4B16AE`
    (`ev_2c3fc6a89a2563c1630dd1767fd961fe660bb2e4eaac629f3d739b4367fcd278`)
    is a five-byte jump thunk to `0x4D7850`. That wrapper calls
    `0x4D6F40(packet, length)`, then calls `0x4D7570` again and returns the
    latter result. `0x4D6F40`
    (`ev_c4ccf051dede1b4fdb2b52a17abcb0d85eba4bd1601be3914e488a7b493fcec0`)
    derives a per-packet byte from transport-object state and delegates to
    `0x4D7140`.
  - `TMSRVIN.exe!0x4D7140`
    (`ev_0d35d62d7da40eb4b42418f06a9a6e6a49b7d267e87c43249ee40212fc0db15e`)
    frames the data with a length, key byte, and checksum, transforms payload
    bytes using lookup tables, and appends the result to the transport
    object's output buffer. A zero key is replaced with a value from `rand()`.
    It then calls `0x4D7570`.
  - `TMSRVIN.exe!0x4D7570`
    (`ev_73013d7395f16620ba667a3581bd4809445dbcc948f33c59471083cdee5992ae`)
    validates buffered length/offset state and calls
    `WSOCK32.DLL::send(socket, buffer + offset, remaining, 0)` once per
    invocation. Successful partial sends advance the offset; when pending
    bytes remain, `0x4D74E0`
    (`ev_5e34094444e8660031afb48b4ce469449345ee3ee9a208a0fd940168006f1244`)
    compacts them to the start of the buffer before a later attempt. A failed
    `send` calls `WSAGetLastError`; no explicit error-code-specific retry
    policy is visible in this routine. The `0x4D7850` wrapper gives the
    transport up to two `0x4D7570` calls per invocation: one through
    `0x4D7140` and one afterward.
  - Together, these static call edges show that the DLL's Tower War fame
    update helper reaches a Winsock send on the socket stored in the fixed
    transport object. They do not establish which peer owns that socket, that
    the packet was delivered at runtime, or any fan-out beyond this socket.
- First initializer, `Core.dll!0x1003A150`, has both pseudocode evidence
  (`ev_a393782d673e1f861dfc64b8122a62af1ecb9b644e065f188ead1a9da62f8b2c`)
  and instruction evidence
  (`ev_da521a722ee3848d1911e3e31dc0e43a2879173a20e96c00590ab9ad25098099`).
  It initializes a local file-like object and passes it to the open routine
  with `../../WYD_DB/FileConfig/AllowedAnct.txt`, then calls `0x1003A100`
  with that object's address in `ECX`. The caller consumes `AL` as a boolean:
  false skips entry iteration and returns `0`; true iterates entries, loads
  each via `0x1003ABD0`, logs `[AllowedItems] Loaded %d items!`, and returns
  `1`. This establishes the gate as the AllowedItems-list load.
- `Core.dll!0x1003A100`
  (`ev_ef023ffc7b024073a01408447aad1ed44c80d56bd76084a48348ab2ab6dbffbe`)
  saves its incoming `ECX`, adds `0x10`, and calls `0x100367B0`. Its caller
  observes the returned `AL`, despite Ghidra inferring a `void` prototype.
- Shared helper `Core.dll!0x100367B0`
  (`ev_9e5d870044e1b542a1530020ef7d11c4685af0cb18cb32bedd2bb5531ac30e82`)
  tests whether the DWORD at incoming `ECX + 0x4c` is nonzero and places the
  result in `AL`. In the AllowedItems path this tests a field at offset
  `0x5c` from the local object, after the open call; it is the observed
  success/readiness gate, but the field's exact class-level meaning is not
  proven. A second wrapper, `0x10036810`
  (`ev_32fce76bdf39abd048a33ece69b8fa7628afba4d3ff21cd0f601efd0a1920680`),
  passes `ECX + 0x18` to the same helper, so that call checks offset `0x64` in
  its containing object; its callers and state meaning remain untraced.
- Second initializer, `Core.dll!0x1003A320`
  (`ev_ab3bb9b5ae645b3fdfe824aea78dd8c065103d567378d8b6cbba97b509b20c43`;
  corroborating query `ev_53ecbbd77d0f45e6871fbc2816ba0e49c31ed624c1330e8145d041a113612110`),
  follows the same file-list pattern for
  `../../WYD_DB/FileConfig/CantDrop.txt`: it tests the local file-like object
  through `0x1003A100`, returns `0` if the status field is zero, otherwise
  iterates entries through `0x10039040`/`0x10039090`, loads each via
  `0x1003ABD0`, logs `[CantDrop] Loaded %d items!`, and returns `1`. This
  identifies the gate as loading the CantDrop item list. The corroborating
  Ghidra 12.1.4 query confirms that the item counter increments and the value
  is applied only after the stream-state check succeeds, then the stream is
  closed on the success path. This is a second analysis of the same function,
  not an additional unique target. Runtime acceptance was not tested.
- Third initializer, `Core.dll!0x1003A950`
  (`ev_aabda9f3e418a8054be498ce877d1de3e23095b5fe6ceee99a8222ed9239cdb9`),
  loads `../../WYD_DB/FileConfig/JoinableItems.txt`. It passes the path and
  flags through `0x100375F0`, opens the local file-like object with
  `0x10038C50`, and checks it through `0x1003A100`. A zero status returns
  `0`; otherwise the routine iterates with `0x10039040`/`0x10039090`, applies
  entries through `0x1003ABD0`, closes the object, and returns `1`. Unlike the
  first two loaders, the observed success path does not log an item count.
  This identifies the gate as loading the JoinableItems list; the path helper's
  exact purpose and runtime acceptance remain unverified.
- Read-only snapshots of all three inputs are present under
  `references/decompilacao server/descompilacao tmsrv 7.59/DestinyServer-main/WYD_DB/FileConfig/`.
  `AllowedAnct.txt` contains 269 decimal-only lines and 249 distinct values;
  `CantDrop.txt` contains 84 decimal-only lines, all distinct; and
  `JoinableItems.txt` contains 20 decimal-only lines, all distinct. Together
  with the shared extraction/apply loop, this supports one numeric item ID per
  physical line in the supplied snapshots. The binary's formatted numeric
  extraction consumes whitespace-delimited values and does not prove that
  newlines are required delimiters. Snapshot-to-binary build identity and live
  runtime acceptance remain unverified.
- The shared stream-open wrapper, `Core.dll!0x10038C50`
  (`ev_82d6023f4b10c753fb6b22316a8d5f35247484e2b407b70a40ab130fb2f53a65`),
  is called by all three list initializers. It constructs a local
  `basic_ifstream` object and delegates opening through `0x10036860`, passing
  its second argument with bit `0x1` set. The wrapper itself does not set
  numeric-format flags; it calls stream setup helpers `0x1002B0C0`
  (`ev_2627687367f09a49e5f5ff1cf0fabefd20c2c63f1c47eaed6fd6141738e77f79`)
  and `0x1002B9D0`
  (`ev_3310aef3f2a2931dbb4e83d70c6855b1a007508ba6ca5d289ce61349f414faca`).
  The former only returns its input. The latter sets the `basic_istream` vtable
  and calls `0x10019A40`
  (`ev_a713a290b26a9a67dffd908f79a9e88c6d56c50f28cea3511fede11cc1dededd`),
  which calls `0x10017F40`
  (`ev_05aacd76b31301b154cd1c1d29c50509f23b415d92136aa008d55bfe9f270553`).
  That `ios_base` initializer writes `0x201` at offset `+0x14`. The file-open
  method `0x10036860`
  (`ev_8a6dc9ca2efe25110f9524d173fcf35b6c665e333e7fc451f7b322a82304067e`)
  calls `Fiopen` and initializes a file buffer; its visible body does not write
  the `ios_base` format field.
- Fourth initializer, `Core.dll!0x1003A4F0`
  (`ev_e57dec08f8bc277574ad69bb623caaeef76ecbd18486504b3a22255c0a65e87c`),
  clears a `0xa8c`-byte global table at `0x1074E150` and parses
  `../../WYD_DB/FileConfig/DonateStore.xml`. It selects the `items` element and
  reads `id`, `price`, `stock`, `storeId`, `itemId`, `ef1`, `efv1`, `ef2`,
  `efv2`, `ef3`, and `efv3`. The `id` is range-checked to `0..26` and
  `storeId` to `0..4`; accepted records are copied as `0x14` bytes to
  `table + id*0x14 + storeId*0x21c`, consistent with 27 records per store over
  five stores. The names not shown in Ghidra's pseudocode were resolved from
  the initialized bytes at `0x10153F54`
  (`ev_324b61f4d08d52a8a737b52c5e32a79d1bd6054437f99088e0eaa8309ab92f57`).
  This proves static XML parsing/table population only; field semantics,
  and runtime acceptance remain unverified. The supplied snapshot
  `references/decompilacao server/descompilacao tmsrv 7.59/DestinyServer-main/WYD_DB/FileConfig/DonateStore.xml`
  (SHA-256 `C534847E63E6CA633EA7C8F2456D0408B9DB35683F5BC2E137BEA5D8EA906DF0`)
  contains 96 item rows: 21, 14, 28, 22, and 11 for store IDs 0 through 4.
  Ninety-five rows satisfy the observed `id` and `storeId` bounds; the
  `storeId=2, id=27` row fails the parser's `id <= 26` check. Every row has
  `stock=-1`, so this snapshot provides no finite-stock example. This is file
  evidence, not proof that the running server loaded this exact snapshot.
- Ghidra 12.1.4 cross-references for `0x1074E150`
  (`ev_adc1a739cdcf848e245384d60f8a192277999ab202de4303de7a6cab9554d413`)
  returned four sites: `0x10029A87`, `0x10029E7A`, `0x1003A541`, and
  `0x1003A879`. The operation reports addresses but not reference kinds. The
  two consumer sites were then resolved to these distinct functions:
  - `Core.dll!0x10029DC0`
    (`ev_212a93769d337d53f64bb1997bd18bd26165fa5155f5c495002174c3b1e64e3c`)
    validates a store index in `0..4` and a loaded-state global, then builds a
    response with opcode `0x17B` and length `0x154`. It iterates 27 records,
    includes records whose item ID is nonzero and below `0x1964`, copies eight
    bytes of item data and the per-slot price, appends the account's current
    balance, and calls `0x100309B0` to send it. This establishes the server
    listing path; client decoding and runtime acceptance are not covered.
    Its complete instruction listing
    (`ev_7ef4be00f5a694a597ee5523368f17183c71f967277c2a6076501d7a84b6da2b`)
    confirms the per-store base calculation and 27-slot loop. It reads the
    item ID at `record+0xc`, skips IDs outside `1..0x1963`, copies eight item
    bytes from `record+0xc` and the first dword (price) to local packet
    buffers, then sends the packet. It does not read `record+8` (stock) or
    write through the table pointer; the observed writes in this function are
    to local packet/stack state. Thus this listing path neither exposes the
    stock field nor changes it.
  - `Core.dll!0x10029990`
    (`ev_0c7444e1afb1252625e254cad675f5ebbb15032e7ce62ae5a460227f07adebf9`)
    searches five stores with 27 slots each for a matching eight-byte item
    payload. It accepts stock `-1` or positive, checks the player's balance,
    and requires a usable inventory slot. On success it copies the item data,
    subtracts the price, sends account updates, and calls `0x10029940`.
    A bounded `trace-native-values` run on this purchase function
    (`ev_445e52c395c9f192532fbe9bed83bbff049980b7539280f66e26956d2935b7af`)
    exposed two `STORE` operations, both marked dead and writing the
    `0xCCCCCCCC` debug-stack pattern. It exposed no direct store-field write,
    but the trace stopped at call depth zero and left many callee bindings
    unresolved, so indirect mutation remains unproven.
    The complete native instruction listing
    (`ev_2d1541496fea2e9c9ab499936762944da4a70e62903d2a47e994c1513e8dc5a9`)
    shows the record stock value at `record+8` is read for the `-1`/positive
    gate and the item payload is read at `record+0xc`. On the successful path,
    the account-state write is the buyer-balance update at `0x10029bbe`; no
    instruction stores through the computed record pointer. The remaining
    candidate call, `0x100308b0`, is an indirect callback rather than an
    unexamined store-table helper (details below). This establishes no stock
    decrement in this purchase body's direct path, but does not establish the
    semantic meaning of `record+8` or rule out unrelated aliases.
    `Core.dll!0x10029940`
    (`ev_7491dab194664cb2d203b49b7628a58926e54bd6ef2b30f225199caa1e4f1db4`)
    only calls `0x100075C0` in its recovered body. That function
    (`ev_795da934318bca716555629b2b2bb41915ed90da47a4093354435b59e9d9d70a`)
    sets up a local object and delegates to other helpers; neither recovered
    body directly accesses the `DonateStore` table. Two delegated branches are
    now traced:
    - `0x100072E0`
      (`ev_3f344f1df39f445028700d47d6798fbbd727f00225d981b1559eac2d7fa2c1c8`)
      calls `0x10007380`
      (`ev_df8332cff818f74d01bec658a4951174b14b0fdea28606147317209d1add5228`),
      which calls `0x10007720`
      (`ev_dd22626a05274d5de5c97c9dd4e5d805b61555e6067354f47cab742d9caa9895`).
      The last function only runs the MSVC debugger-check helper; this branch
      is compiler/runtime scaffolding, not evidence of stock mutation.
    - `0x10004F60`
      (`ev_b2cbb2e917ed7493371233b9caba1ad08804e46e68d2f9200e1653672c882521`)
      forwards to `0x10004E20`
      (`ev_19446e6df363ada1b1fd917f87c8926394ee6d77f81a8438807af8c8b671c859`),
      then `0x1000C730`
      (`ev_125b278091f00450ba71d90873f98fbf74ac58e268376f6d49067e354ee13812`),
      then `0x10004D80`
      (`ev_c50d7395039c3c0d9848fc8d925fd8e0ba8640c926fac3c5de1a25dda6c9428d`).
      `0x10004D80` optionally calls `0x1000ACB0` for sizes above `0xfff`,
      then calls `0x1009C0C0`; its other observed callers include functions
      named `deallocate`. This is consistent with a generic allocation-release
      path and does not establish a `DonateStore` write.
    - The sibling delegate `0x1000BF80`
      (`ev_eab07461a72f78237f1e84977e9e48a97b0d0bc7d83d8eaae057357638d99ddd`)
      manipulates an object through offsets `+4`, `+0x14`, and `+0x18`, and
      calls `0x1000BC50`, `0x1000B970`, `0x10004FB0`, `0x1000C780`, and
      `0x1000C460`. Its recovered body has no direct reference to the table at
      `0x1074E150`; the object's type and indirect side effects remain unknown.
    - `0x1000B970`
      (`ev_6b850cbe6e7d7ea3bd136e33f719c0ee929e553a84001654d66aa02672921e38`)
      compares an object field at `+0x18` against `0x0f` and returns whether
      it is larger. The exact object type and field meaning are unresolved; it
      has no direct reference to the DonateStore table.
    - `0x1000BC50`
      (`ev_54853a9f816c2b600fcc65e2394882370ab196f16604a22323f9393f90a9cdb5`)
      takes a lock, walks a linked chain, clears each node's first dword, and
      resets the chain head. This resembles a generic container-clear path;
      node type and relation to store stock remain unproven. No direct table
      reference appears in the recovered body.
    - `0x10004FB0`
      (`ev_f9d34c878c0f3b8b30e4affe2b082d6d8fb80488d3d52ec113b449547bd94108`)
      contains debugger and stack-check scaffolding but no recovered business
      logic or direct reference to the DonateStore table.
    - `0x1009BDB0`
      (`ev_1ba312841cafb5e93316c954f4b522f2188c5c15801011bfa0f8bbbb37a408f7`)
      is identified by Ghidra as the MSVC
      `__CheckForDebuggerJustMyCode@4` helper. It conditionally calls
      `GetCurrentThreadId`; it contains no store logic.
    - `0x1009BD10`
      (`ev_67c17f83f13713da5b0afebbea04eed5e80987d1bf23d6344d1bc4133376226a`)
      is the MSVC `_RTC_CheckStackVars` helper. It checks stack sentinels and
      calls `_RTC_StackFailure` on corruption; it contains no store logic.
    - `0x1009BDF0`
      (`ev_9e0e6a645ef6807637c6ec751094ec299c33fc72edeecac421fd786dfc71d5b4`)
      is the MSVC `__security_check_cookie` helper. It compares the supplied
      cookie with `DAT_10190580` and calls `___report_gsfailure` on mismatch;
      it contains no store logic.
    - `0x100308B0`
      (`ev_6a5318cde37a47dd45ffef36a42ad4b5bf16a66997f153c5d5c16a8cece49d84`)
      lazily initializes a function pointer to `DAT_1074EBDC + 0x1243` and
      forwards its four arguments. Ghidra lists 63 static references to
      `DAT_1074EBDC` (`ev_d6df2678aec8913ca612a15350e02fe103f3fcf9796605d9cc2d3bc56451d2fa`);
      the previously observed startup value `0x4B1000` makes this callback
      target `0x4B2243` in `TMSRVIN.exe`. Decompiling that target
      (`ev_7121589b122d6e127499fc6aa10977fb128f27e3f7378427a5adb8bfd211cfaf`)
      resolves it to a thunk for `FUN_005045E0`. The target checks the player
      slot, constructs an 0x18-byte packet with opcode `0x182`, copies the
      supplied item payload and slot fields, then passes it to
      `thunk_FUN_004D7850`. It does not reference the DonateStore table; in the
      observed purchase call this is the item/inventory notification path, not
      evidence of stock mutation.
    - `0x10002190`
      (`ev_779f989535f1b819d87ba6c3699129f295bb465c86d98c990dd570dac2b99f77`)
      also uses `DAT_1074EBDC` as a code base, adding `0x759EF` before passing
      the address to `0x10002F00`. This is consistent with the shared original-
      executable callback base, but does not identify the initialization site
      or enumerate all hook dispatch targets.
    - `0x1000C780`
      (`ev_60cc31907d589d15b3846d1b34b2e1667be5146ac79f6294489797f9099372e1`)
      forwards its two arguments to the previously observed generic release
      path at `0x10004D80`; it has no direct table reference.
    - `0x1000C460`
      (`ev_3775bba4874c5d1fc22a5e888e96931aea065eeae41d06c8183092899679849e`)
      copies one byte from its second argument to its first and has no direct
      reference to the store table.
    - `0x1000B640`
      (`ev_0707d7bfd31bf66550490df6054a176a88d6a17132148f15a8d16adb0b3b3ded`)
      only delegates to `0x1000B3C0` after debugger-check scaffolding.
      `0x1000B3C0`
      (`ev_f7266fa840c0660f2c937894e7221f2e940eb5f2b0b0b636ea9144d6e0077a19`)
      returns the saved `ECX` value in its recovered assembly after the same
      check; neither body references the `DonateStore` table.
    - `0x100041E0`
      (`ev_1abb2c019c60c9c0f236e4e03968dd0c575e3b8863edf6f99780a606b694e89f`)
      decompiles to debugger-check scaffolding followed by a return of an
      uninitialized local. Its business effect is unresolved and its
      pseudocode is not enough to rule out indirect effects.
    - An REA `trace-native-values` run seeded at `0x10029940`
      (`ev_56b43e4f4a73c391b86799585c8f62682cca83e1027b14d7c9bb625bc2825741`)
      expanded the post-purchase path through eight decompiled functions at
      depth two. It surfaced `0x1009BD10` and `0x1009BDF0`; their later direct
      analyses identify only MSVC stack/security diagnostics. Several callback
      argument bindings remain unavailable, so this bounded trace does not rule
      out every indirect alias effect.
    The purchase body and its resolved `0x100308B0` callback contain no stock
    decrement: the latter sends the item-notification packet. The meaning of
    `record+8` beyond the observed `-1`/positive gate, and any mutation through
    an unrelated alias or another entry path, remain open. The supplied XML
    snapshot has only `stock=-1` entries, with one out-of-range row. The
    listing consumer at `0x10029DC0` has now also been checked at instruction
    level: it ignores stock and only writes to its local packet buffers. This
    closes direct reads/writes by both known table consumers, but does not
    exclude an unrelated alias or another entry path; continue the broader
    callback/persistence map.
    The two initializer sites are writes, not additional consumers. Full-function
    evidence for `Core.dll!0x1003A4F0`
    (`ev_b4ff91cfefad72b4279dda422784b3de8828611e0d0d407ceb24c84af21754f1`)
    shows `0x1003A541` passing the table base to `_memset` with zero and size
    `0xa8c`, and `0x1003A879` computing the per-record destination
    `table + storeId*0x21c + id*0x14` before copying `0x14` bytes through
    `0x1009F5A0`.
- Shared list-loop helpers used by the three text-list initializers:
  `0x10039040` (`ev_08ecba032abc15d256a8f0e62ac30a06497524ebc03486057933a3ccb4e05d79`)
  forwards its caller's argument and incoming `ECX` to `0x10038380`
  (`ev_d01b5aa01a6fe8b5d05c28f56511cce81c98b2636c40582414dbed47af5a2f83`);
  the CantDrop caller supplies a 200-byte local context. The target's internal
  context semantics are not yet resolved. Its setup wrapper calls
  `0x10013D80`
  (`ev_1bea219ffd3080c71aa46dfcda9091c200b55613e8e32234146459b0f8cf23bb`),
  which copies the DWORD at its input object's `+4` into a local `+4` field
  and dispatches that object's virtual slot at vtable offset `+4`. The body
  has no tokenization or numeric conversion; the virtual target and its side
  effects remain unresolved. Ghidra records three direct callers:
  `0x100198C0`, `0x100364B0`, and `std::locale::global`, so this helper is not
  specific to the config-list parser. The additional wrapper `0x100364B0`
  (`ev_651cf471e9b601e08c5b3b62faa5d08ccdbac1c757c8d0a943863250f96f904c`)
  passes its `this+0x34` member to the same helper and is itself called by
  `0x10036860`; its concrete object role is unresolved. `0x10038380` reaches a guarded
  success path after `0x1002C0D0`/`0x1002C930`, creates a 224-byte local buffer,
  and calls several stream/object helpers before dispatching through
  `0x10039F20` (`ev_b8f7b4498a2522ab10b6b29468464b5e0b9961f1c0aa811a0838de73bf19c702`).
  That wrapper invokes the receiver's virtual method at vtable offset `+0x2c`,
  forwards eight arguments, and returns its first argument. The receiver comes
  from `0x1002B4E0`
  (`ev_383224b6d408f4736e291e1daf2bbdba264feae25fb938690e55a24f3845fd89`),
  which checks and populates the cached facet at `DAT_1074E14C` under
  `std::_Lockit`. On a cache miss it calls `0x10031DD0`
  (`ev_83f6a4ec16698d8fbbba04dc58665948eea2a0b14f68eb5c0c2a19af9e0323df`),
  registers the returned object with `std::_Facet_Register`, and stores it in
  that global. The factory only creates an object when its out-pointer is
  non-null and initially zero; the recovered body allocates 8 bytes, calls
  `0x10018C60` on its supplied context, initializes through `0x10013660` and
  `0x1002BF40`, writes the pointer to the out-slot, and returns `4` on its
  observed exits. `0x10018C60`
  (`ev_afaee15b4cb8f6c95e889d0b6853b5aa0614374802bb0c4a95c86d76452fdc2c`)
  selects either a static pointer or a value obtained through the context's
  `+4` and `+0x18` fields; those fields' concrete types are unresolved.
  `0x1002BF40`
  (`ev_729312b09d38263439248bbcadfb77c65b51643585631a4837a2afcc63c9e6a3`)
  writes the `std::num_get<char, std::istreambuf_iterator<char, std::char_traits<char>>>`
  vtable at `0x101539C0`. The vtable's `+0x2c` entry is referenced at
  `0x101539EC` and resolves to `0x10034DF0`
  (`ev_fd0f39918857193d252546a2861fda8ee31f98071c0d932ea68a92ba66e0a185`),
  which forwards the conversion to `0x100350A0`
  (`ev_c0ee954e3ea612e9ded0f036ce6352e3c4bfcd0f6aae37b85b12fb51d3002c9b`).
  That implementation collects a token through `0x10033590`
  (`ev_92b2395650929c1c3986abfd1d4c2980c25f03b55d30890c38bf9b794aec0cc3`).
  The collector reads the stream basefield mask (`flags & 0xE00`): `0x400`
  selects octal, `0x800` hexadecimal, zero autodetection, and other values
  decimal. It handles an optional sign and the conditional `0x` prefix, tracks
  grouping while consuming the candidate token, and validates grouping against
  helper-provided format data. The caller obtains those flags through
  `0x100196E0`
  (`ev_048d65562be159bee6c968be2c8ec4190edfe4d516b8349ec14e14861f3db16d`),
  whose body returns the DWORD at `this+0x14`; its call at `0x10035131` is on
  this numeric-extraction path. The getter has 59 Ghidra callers overall, so it
  is a shared `ios_base`-style accessor, not config-specific. The constructor
  chain above initializes its field to `0x201`; `0x201 & 0xE00` is zero, which
  selects the collector's autodetection mode. The analyzed list-open path has
  no visible format-field write after initialization. All values in the three
  local snapshots are unprefixed decimal tokens without a leading zero, so this
  mode interprets those supplied values as decimal. This is static path
  evidence, not proof that the server accepted those exact snapshots at
  runtime.
  The resulting token is passed to `__Stoulx`, and stream state is
  updated for empty/no-consumption, conversion-error, and EOF paths. This
  confirms unsigned-long numeric extraction and the CRT-supported base rules.
  Each list-loop iteration extracts one numeric token; the formatted parser
  consumes whitespace and does not enforce newline records. No additional
  record fields are observed in these paths. Snapshot-to-binary build identity
  and runtime acceptance remain unverified. The separate
  Ghidra body at `0x100384BF` (`ev_45ff57d206d8c9f204c1c3c40ab246fb921c2627fbd1d6caf4d270ef634a09d1`)
  is reached from the same routine's tail and contains cleanup/runtime-check
  calls. Focused analysis of `0x1002BEE0`
  (`ev_bf41d1cd2719c6ec6939b873d0889ac95fc1ed5c3177ce42fa29e36691814e0c`)
  shows it initializes the object at `ECX` with a DWORD `0` at offset `+0`,
  byte `1` at `+4`, and byte `0` at `+5`; its role is unknown and it does not
  itself read text. `0x1002BE20`
  (`ev_91937eeea7476f7c35765b1246b53e4ba5d5dcbfbc9c51fb8569dec410dc37b2`)
  calls `0x1001A370` on a secondary object subpart, stores its return DWORD,
  sets the byte at `+4` to whether that value is zero, and clears byte `+5`.
  `0x1001A370` (`ev_6cb6c3616b89b7d9fb44bc7093716b2ba8cb2772cf04c6731116f8fa3e830744`)
  is a direct getter for the DWORD at `ECX + 0x38`; it does not read or parse
  text. `0x1001A3C0`
  (`ev_dd227c49cd9f775609f75020c57d9df80d45e5282c96e770ff590c58a29307b9`)
  is likewise a direct getter, returning the DWORD at `ECX + 0x0c`. Its caller
  `0x10019930`
  (`ev_338fa2448e4e6c70a83d00762e5f7166d73c06aa79d51889b05ecd040c435db6`)
  returns true exactly when that value is zero. `0x1001AA90`
  (`ev_9aaf8d034b1b3e674bd15a5a98646d196b78b89378f60e5077700fcf95c1f0f5`)
  ORs a caller-supplied state mask with the current value and delegates to
  `0x10018D70` (`ev_2affdd0b714ed5dd7622dc219713c55648c70785007ef300c2ea7f88f3f289c9`).
  That routine tests the field at `+0x38`, adds `0x4` when it is zero, then
  forwards the combined state to `0x10018E50`
  (`ev_089ad1d73999c0a5e87052559b93fc0c03184c7888cf191ae4dce495b750661c`).
  The latter stores `state & 0x17` at `+0x0c`, intersects it with the mask at
  `+0x10`, and has a diagnostic/exception path whose strings identify bit
  `0x4` as `ios_base::badbit`, bit `0x2` as `ios_base::failbit`, and the
  remaining reported state as `ios_base::eofbit`. Thus `0x10039EB0`'s mask
  `0x6` checks the observed fail/bad bits; `0x10039090` inverts that predicate,
  so the loop continues while neither is set. This resolves the tested bits'
  diagnostic labels, but not the full object layout, the extra `0x10` bit, or
  whether every use of this shared state object belongs to the config reader.
  Across the three initializer paths, the observed loop performs one formatted
  unsigned-long extraction per iteration before applying the value. The local
  snapshots support one numeric item ID per physical line, but parser code does
  not enforce line boundaries. Exact container semantics,
  snapshot-to-binary build identity, and runtime acceptance remain open.
  `0x10039090`
  (`ev_f515cfb52dc48926f72e23b942c52a0522c9a85dac261e031a80a0113a64d39a`)
  returns the inverse of `0x10039EB0`'s boolean result. That function
  (`ev_3396a560d190849de4b213cc228f8d1a8844c8a41da18455ac5d041ebac457df`)
  tests whether `FUN_1001A3C0()` has either bit in mask `0x6` set. Thus the
  list loop continues only while the tested `failbit` and `badbit` are clear.
  The three initializers call `0x10039040`, then
  test `0x10039090`, and call `0x1003ABD0` only when that test is true.
  `0x1003ABD0` (`ev_01bc07da45e86b24f7ab76173dad2f774fda5c2abc466b8e73245fb81804abf6`)
  forwards the context to `0x10038AB0`
  (`ev_628b7077bb7b5961b1fbccbfa3cdae3d28d517a1dd78d633d59d8cbf99ec26d7`).
  The latter compares two internal pointers at context offsets `+8` and `+0xc`,
  passes a 4-byte value through `0x10038B50`, then dispatches to either
  `0x10038630` or `0x100386E0` according to that comparison. `0x10038B50`
  returns its explicit argument unchanged
  (`ev_34c8bfe09d5081afba4ff3a9747d07a74b96e97bd876a665db949c28206fc8fc`);
  it is not a field decoder.
- The equal-pointer destination `0x10038630`
  (`ev_3aa794d7111f55916c2aa13d8db2810f23c36037b7ab5198635b05fa65939dfc`)
  reads the DWORD at the context's `+8` cursor, calls `0x10038970` with that
  value and the supplied value, passes the result through `0x100396E0` and
  `0x10038A40`, calls `0x10039780` with the current cursor value twice, then
  advances the stored cursor by four bytes. `0x10038970`
  (`ev_8c821c5ab68124e3b101afb9bb47559e468cdc9959543b6c7609b8d8bb519e22`)
  returns its first explicit argument unchanged. The effects of the remaining
  helpers are unresolved; no record schema follows from this path alone.
  `0x100396E0` (`ev_a9ab3e0f8772a2f5e4163435276d99de4034631cf0292a48ade795d66f235c4d`)
  forwards its incoming `ECX` to `0x10039640`, which returns that saved value
  (`ev_8d9f6524d19f381be314ace039eb0e5f3fb0cc25cc3b38a13a0313cdb9217fee`).
  Thus this wrapper returns the same context pointer; it does not transform a
  record value. Shared helper `0x10038A40`
  (`ev_a0bfc80e7dbcbba48403708063387a02d30598fc9949c291f0b9bd6b564760c8`)
  passes `param_2` through `0x10007850` and `param_3` through `0x10038B50`,
  then copies one DWORD from the latter address to the former; `param_1` is
  unused in its observed body. `0x10007850`
  (`ev_b53e4e436546d2c439b9a50ab5de7a03bc9fb2f94934ac4fb025fc7d95d3ab72`)
  returns its second explicit argument unchanged. This establishes a 4-byte
  copy operation, not the semantic identity of either pointed-to value.
- The alternate destination `0x100386E0`
  (`ev_8e2a9d0a4d4cccc602e37b6cc8f5675b505230b1557f75042df495e7df09a2c0`)
  computes an index and element count from the incoming pointer and context
  fields at `+4`/`+8`, passes count + 1 through `0x10039480` and `0x10039C30`,
  then branches between `0x10039A30` and two `0x10039970` calls with
  range-shaped arguments. It also routes a value through `0x10038B50`,
  `0x10038970`, and `0x10038A40`. This is consistent with a vector-like
  insertion/rebuild path (inference), but helper effects, record fields, and
  line grammar remain unverified. The `0x10039970` wrapper
  (`ev_99ff6081e70d32210f89a45aa2cfa66eb22931c19df130e8bce61ae44e85d473`)
  captures the current context and forwards its range-shaped arguments to
  `0x100389B0`. Its endpoint helper `0x10038920`
  (`ev_91daa1901ca5d6e3e16947f0feeb1d98d70e8469821a68a6b580e0a65fca170b`)
  returns the DWORD stored at its input pointer; because `0x100389B0` passes
  addresses of its own local arguments, the endpoint values are unchanged.
  `0x100389B0`
  (`ev_f9e04449a90e5cedf269e8fb4ad20b213f858dabcbb9b95952fd387e878edc6a`)
  passes its destination through identity helper `0x10038970` and calls
  `0x10038570` with the unchanged endpoints and destination. `0x10038570`
  (`ev_c62e711bc2ee33d5b5335115b9380a985a396a9e1c8400a6208e6e3f036fd5ca`)
  computes the byte span between the endpoints and calls `0x1009FB20` with the
  destination, source, and span. The latter
  (`ev_250cceb2674add1d8f852e13936c890a106cedefadec77d4b3f7e033ea527935`)
  copies forward or backward when source and destination overlap, with byte,
  DWORD, and block-sized loops. This confirms a memmove-like range-copy
  primitive in the rebuild path; it does not establish the meaning of the
  range, record schema, or list operation. The `0x10039A30` wrapper
  (`ev_a7a1331476e8a817992898d8473966604e309dbabe853a6c4cd06a26ef7f1e54`)
  passes its three arguments and a trailing zero to `0x100399D0`;
  `0x100399D0` (`ev_04dd7483aea29a1727916490729dbd596a453aee998889fc4b57aee0a1594f7e`)
  obtains the current context through `0x100396E0` and forwards the arguments
  to `0x100389B0`. This closes the wrapper chain for that copy edge only.
  The sizing/storage chain is now also bounded: `0x10039480`
  (`ev_b5db27a0a249e843d5bd7984170596ec95422796efc13d2b3545cd462296df8b`)
  reads the current DWORD count from `0x10039C90`
  (`ev_0a9dab1fd4b1943a04ce3b072266a74f6e70b3a349e12ed198fea7febbad4a19`),
  whose observed calculation is `(context[+0xc] - context[+4]) >> 2`. It
  compares the requested count against a bound returned by `0x1003AB50`
  (`ev_3e514244fee67e35e0cd14d40861f815fd7d1b8611824bd946b7f493c1cda61a`),
  The complete helper chain now resolves the value: `0x1003AB00`
  (`ev_2db708cc08171455b8501d2e1917c30382dc3370727d21e7f2dc6d2ca29c939f`)
  returns `0x3fffffff`; `0x1000C940`
  (`ev_4e9705772543b9156efbaeb04e81e2e1aa10b624bf2759f2094f5d5245621aa5`)
  returns `0x7fffffff`; and `0x100066B0`
  (`ev_e1495f0b0a7c92fea4d3115be66e382ddd2ec2efd3bdc0d2967bf24ca24ed05a`)
  returns the smaller value. The effective bound is therefore the constant
  `0x3fffffff`, not a configuration-derived field. The wrapper pair `0x10039730`
  (`ev_f243922d62126aa67074fa1933e6d60e4184211d0ccb8c3f70cda5bce56cd285`)
  and `0x10039690`
  (`ev_d60d4a8affd39b1e9448b29c2737ecdc2ae9a79121109a9c39add4e2d1e927e8`)
  preserves the incoming `ECX` value and does not alter that bound. Since the
  insertion path compares element count against this value before growing the
  sequence, it acts as a maximum element count (static inference); `0x10039C30`
  independently checks the same `0x3fffffff` threshold before multiplying by
  four.
  When that bound permits, `0x10039480` returns the greater of the requested
  count and current count plus half the current count; otherwise it returns
  the bound. `0x1003AB50` carries a Ghidra type-propagation warning.
  `0x10039C30` (`ev_7d8e46d092df76de73c268873f63a23a3d1e6a8e43f6e1aa0629135c57f315af`)
  converts that count to bytes through `0x100239D0`
  (`ev_d8903c405892083c013cc89107bf55db8ee4360d88c1a6a047f94956ab86dcc1`),
  which checks the `0x3fffffff` limit before multiplying by four, then calls
  `0x10004930` (`ev_b1c2645593e14499e4d28d285fe662fb767d55fa36a5dc2a1799ca90a1c08f19`).
  That allocator returns zero for a zero-byte request, uses `operator_new`
  through `0x1000AFD0`
  (`ev_2931efdadfa392c26a42d4eda9fb2c1bfc38d13d7df5a6c6277bf264b3396ff0`)
  below `0x1000`, and otherwise calls `0x100049A0`
  (`ev_7ac25ff99bedfca8c6781f3cced8abb050c26716c003c8c723ac2323c8c06181`).
  The large-allocation helper reserves `request + 0x27`, returns a 32-byte
  aligned address, and stores the original allocation pointer and `0xFAFAFAFA`
  marker immediately before it. Together these observations support a
  four-byte-element backing-storage rebuild, but do not identify the element
  schema or the text-list grammar.
  In the equal-pointer path, `0x10039780`
  (`ev_eb047fa669018bda40fd968b14ca3c9579bcf30b39323576725303b709000060`)
  takes lock id `3`, walks a global linked structure, and unlinks entries whose
  DWORD at node offset `+8` is within the inclusive input range, clearing the
  node's leading DWORD before unlinking. Its caller supplies the same value as
  both bounds, so that call removes exact matches. The structure's purpose and
  its relationship to the config records are unknown.
- Worker `Core.dll!0x100110F0`
  (`ev_73436c632984e2658813185c660beaf9dea9993e11549cdbaeb0fedf00c080a0`)
  loops with a one-second sleep. Its observed branches include Coliseum
  registration/eligibility, Kefra restrictions and ticket checks, cash
  delivery, and gift delivery. This is a partial map, not a complete account of
  every branch.
- Worker `Core.dll!0x10011A60`
  (`ev_dc828a45006e2b3412f14038f7794b5456e214926bd29bc9dc47783ebe8c0a27`)
  also loops once per second. Its observed branches announce, start, and finish
  the Tower of Thor war, announce the winning guild, and update a
  `LastServerBoss` flag. The successful-defense branch adds configured fame to
  the guild-keyed value and sends the corresponding update. The daily gate
  compares three local-time fields against seconds `55`, minutes `59`, and
  hour `23`, so its static trigger is local `23:59:55`; the server's configured
  timezone and actual runtime execution remain unverified. When the gate
  matches, it reads `../../WYD_DB/FileConfig/LastServerBoss.txt`, writes the
  opposite integer value (`0` or `1`) back to that file, and clears process
  globals `DAT_101949c8` and `DAT_101949c9`. This gate is not conditioned on
  weekday and has no guild-fame mutation in its recovered branch. The separate
  Tower War schedule is guarded by local weekday values `1..5` (Monday through
  Friday under the CRT `tm_wday` convention). Thus, this worker does not show a
  weekly fame reset at the Saturday boundary.
- Tower fame shared-state path: getter `Core.dll!0x1002E4E0`
  (`ev_d736d22fd58c848b9b3fd99d0be610e5448576e0c98bf042be3eba45cf5bcee1`)
  reads a guild-indexed value from the table based at `0x4D7F1A8`. Setter
  `Core.dll!0x10030C70`
  (`ev_8b93b93afceaff515e24879f7351343cc55fc6365ec00e443bc9b1c0628dd5ce`)
  writes the supplied value back to the same table and sends a `0xC17` update
  containing the guild ID and value. `Core.dll!0x100313C0`
  (`ev_6914701aa6a3967536ff407e7ec63e3a92b653069d192b92a3b053f35d82fdb3`)
  constructs the same update from the getter. The defense-success branch in
  the worker calls the getter, adds `DefendTowerFame`, invokes the setter, and
  sends the update helper. TMSRVIN has 22 static references to the same base
  (`ev_90d4556ba80230b08b6c89180f0f9c73b48973cd1f1e671549866d81a00aa7c6`).
  The recovered setter clamps negative values to zero, writes the in-process
  table, and emits network updates; its body contains no file or database write
  call. This bounds the known Tower fame mutation path, but does not rule out a
  separate serializer or persistence path elsewhere; the corresponding
  `DBSRVIN.exe` guild-record file writer is traced below. This establishes
  a file-backed write path for the receiver's guild record, not a direct durable
  write from the shared `Core.dll` table.
  Four references (`0x4E940F`, `0x4E9459`, `0x4E94F1`, `0x4E9566`) lie in the
  large non-contiguous command dispatcher `FUN_004E6B90` (35,466 body bytes).
  Its recovered pseudocode identifies a `guildfame` command path: the `set`
  branch parses a guild ID and value, writes the value into the table at
  `0x4D7F1A8`, logs the change, replies to the requester, and sends a 24-byte
  `0xC17` packet containing the guild ID and value through `0x4D7850`. When the
  second token is not `set`, that path parses it as a guild ID, reads the
  table, and replies with the current fame. Evidence: complete function result
  `ev_cd5ed21a2b48f8713162982982a56367103a7702175952d63d9dc249f21c9b6b`;
  instruction at `0x4E940F` is a 32-bit store to the same table
  (`ev_88ae376132b4f7dd1fb4ac78c1100aece216e50ebab38c056b6588e69a676316`).
  Of the other 18 xrefs, 13 resolve to named procedures: `0x4F6E8F` in
  `FUN_004F6D60`; `0x50CC8F` in `FUN_0050CB80`; `0x512967`, `0x512A78`, and
  `0x512B31` in `FUN_00512800`; `0x51AB18` in `FUN_00519CA0`; `0x556619`,
  `0x5569CC`, `0x556A8D`, and `0x55C943` in `FUN_00555600`; `0x57D60B` in
  `FUN_0057BDF0`; and `0x58839F` plus `0x5883D1` in `FUN_00588340`.
  Four newly saved thunk dossiers identify the transport helpers:
  `0x4B2DBF -> 0x4D5BA0` clears socket-state fields and closes a nonzero socket
  (`ev_b5de3da4629f8a43392241f2477d65bdce4566d7044040d1f09b1864c4a5e1e0);
  `0x4B31D4 -> 0x4D5EC0` initializes/reconnects an IPv4 stream socket, binds and
  connects it, registers asynchronous message `0x402`, and sends the four-byte
  protocol magic (`ev_db46d76eb20a70cd5d714155f15894c4aba5dc92396eed8296feac3f499ab851);
  `0x4B2527 -> 0x4D6770` receives into the connection buffer at its current
  offset (`ev_652d305629f611d5a2769459b0f63636f3c74a6aadabd228c3ff05c69e299630);
  and `0x4B15A5 -> 0x4D6810` parses framed data, validates the magic, length,
  transformed payload, and checksum
  (`ev_bc14e3fafc11fb200ffc79941d16c3ccb91c8f17f0dbda2a49da47ca1c89ed34`).
  These are static code findings, not observed network behavior.
  Startup routine `FUN_00517DB0`
  (`ev_8f37740ca6d7be2e21865e5cc776ed1d4c61e362be492871fcb22a9afe0b6ff8`)
  reads primary and optional secondary IPv4 values from `localip.txt`, matches
  the primary address against the in-memory server table at `0x00B7C330` (the
  failure message names `ServerList.txt`), and selects the destination string
  at `0x05145578`. On a match it sets the DB port to `0x1D5A` (7514) and calls
  `0x4D5EC0` with that destination, port, bind address, and async event `0x402`.
  Thus the endpoint is selected from deployment configuration rather than
  encoded as one universal IP in this startup path; the actual configuration
  and a successful connection were not observed. The same routine zeroes both
  `0x061802B0` and `0x061602B0` for `0x20000` bytes each, so these in-memory
  ID maps start empty on this initialization path.
  `FUN_00555600` pseudocode
  (`ev_67bb30fa8f3f36f9aa6281b925c06c26a87756a24bec767bc6b0205675daf9dc`)
  contains guild-result update handling that writes a 32-bit value at the
  fame-field offset (`+0x38`) for one indexed record on type `1`, and clears
  that field across active records on type `4`. The bulk path scans the
  16-by-4096 record table and checks both a nonzero guild ID and a nonempty
  name before clearing. This confirms a packet-handler mutation/reset path,
  but does not identify a weekly schedule; durable writes by the corresponding
  `DBSRVIN.exe` receiver are traced below. The saved
  `FUN_00518790` procedure
  (`ev_804c037ce971e4be2857e3cd2c15df659ff91466a787087456148bf43327fad3`)
  calls `FUN_00555600` for non-error buffers returned by `0x4D6810` inside its
  `readdb`/reconnect receive loop. This establishes a receive-side caller for
  the handler. The callback has distinct player-socket and DB-socket branches:
  player events in `0x9000..0xA000` select a slot record at
  `0x061AAAD8 + slot * 0xFC8`, receive and parse packets, and dispatch valid
  commands to `FUN_00519CA0`; DB message `0x402` receives and parses on the fixed
  object `0x06584FF8`, then sends accepted payloads to `FUN_00555600`. At the
  six DB call-setup sites (`0x518CD7`, `0x518DAC`, `0x518E36`, `0x518E71`,
  `0x518ECA`, and `0x518F63`), the decoded instruction loads `ECX` with
  `0x06584FF8`; the corresponding instruction evidence IDs are
  `ev_b7d168cabf046190bc2f3c256ac08d72929afb30559261976b79a9d9e89cc272`,
  `ev_71157da0f26f4588c4abf96e82bb0654a4f037ae7cbdc2a809f954bb08e63af3`,
  `ev_62483e598925570522622cb84249c0a90592bb25c76e61da1e7ce39c90eb9cec`,
  `ev_f5969d722da46ccbd35f68b23fd1eab9b2791be5090d59f3475f81c35f252030`,
  `ev_a307330cc0029442844aadfc35c1fc0e6823d132ce581cbf707584f399f6f2d2`, and
  `ev_065a60c05f82ed878bd7dc8ca7ea11d665270dc0a8833aaa9eb81952b8d98379`.
  This corrects the earlier reading: the parser call at `0x518F63` uses the
  fixed DB object, not a per-player slot object. The traced Core.dll send caller
  also supplies `0x06584FF8` to `FUN_004D7850`, so that Core.dll send call and
  the DB receive path pass the same object address. Startup now resolves the
  endpoint source and port statically, but the concrete IP remains dependent on
  the external configuration. The matching `DBSRVIN.exe` opcode/operation
  handler is traced below, but this call-graph relationship does not prove
  actual packet delivery or successful runtime processing.
  `FUN_00512800` pseudocode
  (`ev_51a5d7ce3cfaf5669d7cf0abe1a673afc1c7c19442c9b998709e161121977b86`)
  shows the city-war challenge entry path referenced at `0x512967`, `0x512A78`,
  and `0x512B31`. It reads the applicant guild's current fame from the shared
  table at `0x4D7F1A8`, not a player-supplied bid amount. Its conditional
  admission checks include a 100-fame floor in the `local_24 == DAT_006472F4+1`
  branch only when the city's `local_6C` value is nonzero, and a 1,000-fame
  floor in the other branch; these are not one universal minimum. The procedure
  compares current fame against city-indexed values at `0x4D7E258` and, in one
  path, `0x4D7E26C`. On acceptance it stores the guild ID in `DAT_0063A064`,
  stores the current fame in one of two city-indexed slots at `0x4D7E258`, emits
  the `gld,ChallangeGuild` log, sends response `0x1E`, and invokes three update
  helpers. In the `local_6C != 0` path it calls
  `FUN_00588340(guild_id, -100)` before the later slot comparison; this caller
  shows no refund path if that comparison rejects the bid.
  `FUN_00588340` pseudocode
  (`ev_e29688a857e436196fc6a3749dd9986dd07ea469199bf71331cdd85e7f5506dd`)
  confirms that the `-100` delta subtracts 100 from guild fame, clamped at zero.
  The helper writes the shared fame table, logs the resulting value, sends the
  24-byte `0xC17` guild-fame update, and scans ten records for the guild to
  conditionally send a 28-byte `0xC23` update when its record field is 7 or 8.
   A separate TMSRVIN war-state path links `FUN_00567780`
   (`ev_26b0f4fd519bc3be4a4c139eca8bb2a6ce5eb4e1c677d55c3efa37f8326abe7f`)
   to the guild-summon helper `FUN_005656F0`
   (`ev_abd290d09cea0147f93f4ef3289917bba95902aa357339192c94a7751f5485fc`).
   In one state-transition branch, `FUN_00567780` iterates indices 0 through 4
   while excluding index 4, and calls the helper from two loops with per-entry
   values and coordinates read from global tables. The first loop requires two
   corresponding values to be nonzero; the second requires one. Both pass
   `0x1A` as the helper's fourth argument and the index (0 through 3) as its
   fifth. The helper scans up to 1,000 player slots and applies state,
   connection, matching-field, and additional partition/eligibility checks; it
   then processes its local candidate list up to the caller-supplied limit
   (26 in these war calls). For each selected slot it calls
   `FUN_00575450(slot, x, y, 0)`, emits a `sys,guild war summon` message, and
   publishes it on the system channel. This is a recovered static path, not
   proof that the transition ran or that players were moved at runtime.
   The newly decompiled `FUN_0056C360`
   (`ev_58f11a31eb7f42d50698cae88c721ea472334c07b0c849b11a157b1ba34e4295`;
   saved in `build/rea-analysis/work/TMSRVIN-war-followup.rea-snapshot.json`)
   opens `Chall.txt` in text-write mode (`"wt"`), logs if the open fails, and
   otherwise writes five `"%d %d\n"` rows from the global arrays at
   `0x4D7E258` and `0x4D7E26C`, then closes the file. Ghidra xrefs identify
   references to its thunk at `0x512F2B`, `0x53D4B4`, and `0x568903`
   (`ev_8a62bfabddee671b4ac59736a107789433a780e2b0100af660604db20c6a77ae`).
   The `0x568903` site is recorded as an unconditional call from
   `FUN_00567780` in its function dossier
   (`ev_26b0f4fd519bc3be4a4c139eca8bb2a6ce5eb4e1c677d55c3efa37f8326abe7f`).
   Direct instruction evidence confirms `0x53D4B4` is `CALL 0x4B12EE`, but
   Ghidra reports no containing procedure for that address
   (`ev_27d4d3d78f6a1666195992ee4056f48f29178408120a35edfe83960836581f43`).
   The `0x512F2B` reference lies within the already mapped challenge routine
   `FUN_00512800`; the function pseudocode independently shows a call to
   `thunk_FUN_0056C360` on the accepted-bid branch, resolving this reference as
   a call despite the xrefs projection not exposing its kind.
   Therefore the writer is statically linked to the challenge path and to
   `FUN_00567780`. The supplied TMSRVIN C listing places the block containing
   `0x53D4B4` under `sub_519CA0` (`references/decompilacao server/descompilacao
   tmsrv 7.59/TMSRVIN/TMSRVIN.c:61308-61357`): after city/challenge eligibility
   checks it decrements or clears the challenge item and sends the updated item
   slot, calls `sub_4B14F1(village)`, stores a value in `dword_63A064`, clears
   the corresponding `dword_4D7E258` challenge entry, writes a challenge log,
   sends client message ID `140`, then calls the `Chall.txt` writer thunk and
   two helpers at `0x4B27B1` and `0x4B2757`. Ghidra still reports no containing
   procedure for `0x53D4B4`; the saved `FUN_00519CA0` body has noncontiguous
   ranges and omits this address, despite including the common epilogue at
   `0x5548F5` (`ev_27d4d3d78f6a1666195992ee4056f48f29178408120a35edfe83960836581f43`,
   `ev_14da12a13766379cd884793af6559aec2b578801bd04e37e5a379430c55fb1a8`).
   Thus the listing associates the block with `sub_519CA0` and clarifies the
   callsite's challenge-settlement role, but Ghidra does not establish function
   ownership. This is static listing/code-reference evidence only, not proof
   of successful file writes, runtime scheduling, or delivery.
   The same accepted-bid branch then calls `thunk_FUN_00512580` before
   `thunk_FUN_0050CB80` (`FUN_00512800` evidence above). Its thunk resolves to
   the newly analyzed body `FUN_00512580`
   (`ev_8f46c3d178defaae33fff743cfcc5d47b1684830391e569d8f5453e65adba4f8`,
   saved in `build/rea-analysis/results/TMSRVIN-city-war-helper-512580-20261021.json`).
   This helper opens `Guild.txt` in text-write mode (`"wt"`); on failure it
   logs `err,Writing Guild error - can't open Guild.txt(Local)` on the system
   channel. On success it performs six five-entry formatted-output loops over
   global arrays and one ten-entry loop over another global table, then closes
   the file. The decompiler's wide format literals are garbled, so the exact
   row format and data semantics remain unknown. This statically links the
   `Guild.txt` writer to the accepted challenge path; it does not prove a
    successful write or runtime persistence.
    The separate call at `0x53D404` resolves through thunk `0x4B14F1` to the
    newly analyzed `FUN_0050BCC0`
    (`ev_de1215efc04601350044a779124be592144fe7fb333120506d36536bd3c97a3e`,
    saved in `build/rea-analysis/work/TMSRVIN-0x50BCC0-20261021.rea-snapshot.json`).
    That 65-byte body sets `byte_61AA207 |= 1 << (village & 0x1f)`; the adjacent
    `sub_50BC70` listing tests the same city-indexed bit. This confirms a
    per-index flag mutation on the accepted challenge path, but its lifecycle
    and role in later war resolution are still unknown. Do not confuse this
    call with `0x53D4B4`, which is the separate `Chall.txt` writer call above.
  Six newly analyzed bodies add the following city/tower-war lifecycle detail;
  all use the verified `TMSRVIN.exe` input SHA-256 in the scope table above:
  - `FUN_005122B0` (`0x5122B0..0x512579`,
    `ev_082ec93f4c955a1ed0ed2ee5545074cd5dd8ceb45bb9ff18910f6b229f6f8f7b`,
    `build/rea-analysis/results/TMSRVIN-guild-state-loader-005122B0-REA-evidence-20261021.json`)
    reads `Guild.txt` and initializes five guild-war slots, including owner and
    challenger values, zone fields, types, and result counters. The body also
    contains file-open and zero-counter error handling. This establishes a
    static file-backed state-load path, not successful runtime loading.
  - `FUN_00566D60` (`0x566D60..0x567639` and `0x56763F..0x567734`,
    `ev_f100ca23ec30715d4d81ddbe3f6fecf1d418739f6c423d753c307fd8ab90e228`,
    `build/rea-analysis/results/TMSRVIN-guild-war-settlement-00566D60-REA-evidence-20261021.json`)
    processes active mode-22 guild-war participants, selects a winner, updates
    ownership, resets challenger/result state, emits guild-result messages,
    adjusts guild fame through `FUN_00588340`, updates world state, and invokes
    the mapped `FUN_00512580` `Guild.txt` writer. The analyzed body has a
    five-byte gap between its recovered ranges. A focused Ghidra xref query for
    thunk `0x4B19F6` (`ev_71e87bc0477d9ed901a29c2f25389b3f442e254c8c0708f4d3cedc36501df12f`,
    `build/rea-analysis/work/TMSRVIN-war-settlement-thunk-xrefs-20261021.rea-snapshot.json`)
    returns caller address `0x5688F4`. The `FUN_00567780` body dossier records an
    unconditional call there to the thunk, which resolves to this settlement
    body. The supplied TMSRVIN C listing associates that block with
    `WeekMode == 3` and the configured `GuildDay`/hour/minute guard, or the
    `dword_64737C == 4` override
    (`references/decompilacao server/descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c:72644-72655`).
    This locates a static settlement trigger inside the state machine; it does
    not establish what invokes `FUN_00567780` or prove runtime execution.
  - `FUN_0056C230` (`0x56C230..0x56C29A`,
    `ev_ea5f613e6c9144d92b32b0694b778d97d08f3b111cfe1df66b53fe197e350ff3`,
    `build/rea-analysis/results/TMSRVIN-challenge-reset-0056C230-REA-evidence-20261021.json`)
    clears the five challenge/bid slots, challenger IDs, and bid values.
  - `FUN_0056C2A0` (`0x56C2A0..0x56C354`,
    `ev_1769daa624e22c19df1982f67982ae25fc02844f615643d942dbc1d4f445f7fc`,
    `build/rea-analysis/results/TMSRVIN-challenge-loader-0056C2A0-REA-evidence-20261021.json`)
    calls that reset helper, then reads five challenge rows from `Chall.txt`;
    its file-open failure path logs an error. This complements the already
    mapped writer `FUN_0056C360`, but does not prove persistence succeeds.
  - `FUN_0057DDE0` (`0x57DDE0..0x57DF25`,
    `ev_67e78bde50764395b6f25ddd0555166ab992fa9bb4dd90c0f10a43b368f060ce`,
    `build/rea-analysis/results/TMSRVIN-mantle-zone-score-0057DDE0-REA-evidence-20261021.json`)
    totals red/blue mantle-zone values across the five war slots and sends a
    16-byte status update via the mapped `0x4D7850` transport helper. Opcode
    field `0x80A` is observed; its exact external meaning remains unresolved.
  - `FUN_005778D0` (`0x5778D0..0x577B8C`,
    `ev_5f54f53a8c85744615212020f912159bddb30df8098e1f9854d76642385da08a`,
    `build/rea-analysis/results/TMSRVIN-war-state-broadcast-005778D0-REA-evidence-20261021.json`)
    synchronizes war ownership/result values into world/grid objects, uses
    object IDs `3145 + result`, and multicasts 32-byte updates through
    `GridMulticast` at `0x503FF0`. Interpreting these records as specific NPC
    types is not proven.
  Earlier xref records for these six bodies mostly exposed import thunks. The
  new call edge resolves the settlement inside `FUN_00567780`, but the caller or
  scheduler of that state machine remains unresolved. NPC/packet-entry callers,
  the complete tower-war schedule, and a reward/medal path also remain open.
  - The `GridMulticast` body `FUN_00503FF0` (`0x503FF0..0x5043E5`, 1,014 bytes,
    `ev_cce22edfee441925c834ea18a96dcfb9934c9894811ee8a012401cfce8ae80de`,
    `build/rea-analysis/results/TMSRVIN-grid-multicast-00503FF0-REA-evidence-20261021.json`)
    is the implementation behind thunk `0x4B1960` used by the mapped war-state
    synchronization. It scans nearby grid cells, skips the excluded entity ID,
    clears cells whose entity records are absent, and for eligible active slots
    below 1,000 sends the supplied packet through `FUN_004D6F40`. For packet
    field `0x338`, it first copies three values from the entity record into the
    packet. Error branches log `grid -2/-3 empty` and invoke slot cleanup. This
    grounds the caller's grid update in a spatial scan/send path, but does not
    identify the game-object types represented by the records or resolve the
    war scheduler/reward gaps above.
   Xrefs for the `0x4B290A` thunk resolve to `0x4E9E69`, `0x5687A6`, and
   `0x56882D`; the latter two are callsites in `FUN_00567780`, while the thunk
   itself adds no unique procedure body
   (`ev_b228d7b1381ff4ece7d83312d4ce9988eb78774c16d34c128448657686f3e9ea`).
   The `0x4E9E69` reference lies in the already mapped
   `FUN_004E6B90` dispatcher
   (`ev_cd5ed21a2b48f8713162982982a56367103a7702175952d63d9dc249f21c9b6b`):
   its `gsummon` branch calls the same helper with `0x27` as argument four and
   `-1` as argument five. Since candidate enrollment in `FUN_005656F0` is
   guarded by `-1 < param_5 < 4`, this call appears to leave the candidate list
   empty and perform no relocations. Treat this as a static inconsistency and
   likely no-op, not a runtime-confirmed failure.
  `FUN_004F6D60` pseudocode
  (`ev_4673820710a1fafc9398adcabe2f8e7446285e51bc0896b410a864b4e5db4f0e`)
  has a `case 0x32` inventory exchange. For a guild member, it scans the
  inventory for item IDs `0x0FAA` or `0x0FAB`, removes one matching item, and
  credits the guild fame table by 1,000 or 10,000 respectively through
  `FUN_00588340`. Its guard permits the exchange only while the pre-credit
  fame is below 1,000,001; it does not clamp the post-credit value, so the
  resulting fame can exceed that threshold. No gold payment is visible in this
  branch. This is a positive fame source, but its caller/UI identity and
  runtime reachability are not established.
  `FUN_0057BDF0` pseudocode
  (`ev_8a6c5acd9338588270b107be726a58d5806d1373a7692d4bcf8005170e08b5de`)
  runs in the character-load path. For a guild found in the ten-record guild
  table, it reads fame and selects one of several inventory item/effect
  constants when fame exceeds 1,000, with boundaries at 1,500, 3,000, 5,000,
  8,000, and 11,000. It inserts the selected payload into an available
  character inventory slot. The exact player-facing meaning of that payload
  is unresolved; this is not evidence of fame persistence or weekly reset.
  `FUN_0050CB80` pseudocode
  (`ev_704c15724cff64e60f7d2e8169a66e170e1924a58c548c2b80da7b4634d4577f`)
  copies five city-war board entries and sends a `0x80D` packet. It reads the
  guild's fame and status to prepare a `0xC23` update when the guild entry is
  nonzero, but the loaded fame local is not used in that packet and this
  routine does not mutate fame.
  The `0x51AB18` access in `FUN_00519CA0` reads guild fame as a gate: the path
  requires at least 500 fame and at least 10,000,000 player gold, then deducts
  the gold and sends a 20-byte response. It does not write fame. The exact
  command purpose is not established, so this is not evidence of a city-war
  fee.
  Instruction-level decoding classifies the five references for which Ghidra
  supplied no containing procedure name:
  - `0x52590B` reads fame and formats a `"Challange, %s:%d"` log entry.
  - `0x53D2B5` reads fame and later includes it in the
    `"gld,Cash - ChallangeGuild %s (%d %d %d)"` city-war challenge log.
  - `0x54C2E5` is in the text-command branch matching `getout`. If the
    player's state byte is `9`, it clears the fame value for the packed guild
    ID and emits a `0xC17` fame update with value zero, followed by a companion
    update. This is a conditional command-path reset, not evidence of a weekly
    timer; the semantic meaning of state `9` is unresolved.
  - `0x54E93C` and `0x54E9F9` read fame into two formatted guild-list rows and
    send response opcode `0x216`; neither is a mutation.
  The same command dispatcher contains the literal administrative command
  `resetguildfame` at `0x4EA22F`. Its branch emits a 24-byte `0xC17` packet
  with guild ID zero, type `4`, and fame zero. This matches the separate
  `FUN_00555600` type-4 handler's bulk clearing of fame fields for active guild
  records. The recovered command branch sends this packet through
  `0x4D7850`; it does not directly store to the shared fame table or directly
  call `FUN_00555600`.
  `DBSRVIN.exe!FUN_00471210`
  (`ev_c6090371cfada3010c88e82829cc7e1ee0f92569b77c01f96fd80c819777d5af`)
  contains a matching `case 0xC17`. For operation type `1`, packet offsets
  `+0x0C`, `+0x10`, and `+0x14` carry the packed guild index, operation type,
  and new fame value. After validating the packed index, the handler writes the
  value into the selected in-memory guild record at `+0x38`, then copies the
  full `0x3C`-byte record and calls `FUN_0047BA20`. No visible range check is
  applied to the new fame value. An invalid index logs
  `err,UPDATE GUILD guild index error %d` and skips the mutation. A persistence
  failure logs `err,guild update fail` and exits without rolling back the
  already-mutated in-memory value. On success, the handler broadcasts the
  updated 24-byte packet through `FUN_004847A0` to active server slots; when
  `0x02735B78` is active it also constructs and sends a `0x1EC`-byte message.
  The outer handler returns `1` on both common and error exits, so that return
  value is not a success signal. For operation type `4`, it scans the
  16-by-4096 guild table, skips inactive/empty records, clears each active
  record's fame field at `+0x38`, and calls `FUN_0047BA20` for that record. A
  failed helper call is logged and aborts the bulk path.
  `DBSRVIN.exe!FUN_0047BA20`
  (`ev_70fca9fa128b52fef4c923d83535e793360aaf6bf6bf8821a84f7ff32a15b7de`)
  formats the process-relative path `./guild/%d.bin` from the guild record ID,
  opens the file, seeks to offset zero, and writes the complete `0x3C`-byte
  record, including the fame field. Its successful path closes the file and
  returns success; error paths log failures. The helper checks for `-1` from
  `open`, `lseek`, and `write`, but it does not detect short writes or close the
  descriptor after a seek/write failure. Static code does not establish runtime
  success or the deployed working directory. Thus the administrative packet's opcode and
  operation now have a statically matched recipient and file-backed persistence
  path, while actual network delivery/acceptance remains unobserved. No weekly
  trigger is established.
  A separate `DBSRVIN.exe!FUN_004900F0` dossier
  (`ev_1cb587424b0cdcae6374ab0769a6eca8c4446a3cd9f4aa30363f34b65fd27680`,
  input SHA-256 `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`)
  scans `./account/%s/*.*` and iterates four indexed character slots. In that
  per-character export loop it emits `FAME,%s,%d,...` keyed by the indexed name,
  alongside `GUILD`/`GMASTER` rows. This is evidence of a character/account
  export field, not evidence that the row is guild fame. The dossier has no
  resolved callers, so it does not establish when this exporter runs. Guild
  fame remains the distinct `+0x38` field of `./guild/%d.bin` cleared by the
  `0xC17` type-4 path above. The string xref is recorded as
  `ev_86cdeefafb331fabea9729e50f7032a88bd1585d957698301a68e1795a23b0a0`.
  Together the examined paths establish local fame mutation, notifications,
  a positive item-exchange source, a fame-derived login payload, a conditional
  local `getout` reset, an administrative reset packet with a matching
  `DBSRVIN` file-write path, city-war logging, and fame-dependent list output.
  They do not establish a weekly scheduler or successful runtime execution.
  The shared `Core.dll` table setter itself still has no direct file/database
  write. A 12-address
  pseudocode batch timed out after 600 seconds; isolating `FUN_00555600`
  succeeded in 208.9 seconds.
- `Core.dll!0x10030170`
  (`ev_2b8f77779bde69d05f12dc1ff92a470211119e24fef9bc33639ea16250a68e72`)
  builds the `../../WYD_DB/FileConfig/%s.txt` path, checks for the file, opens
  it in read mode, skips empty and `#` comment lines, and parses the remaining
  lines. Its parser wrapper `0x1000D380`
  (`ev_72008c2afe3d36aab782f94a68567fd7444069c730e72ab61756706d670a739e`)
  delegates to `0x1000D310`
  (`ev_e18e972812026861a78d1b284bcbb4c32c20a6bb78d412e555a1d0c47ec8fb8e`),
  which calls the CRT `vsscanf` path. The shared format bytes at
  `0x10150BD0` are `%d\0\0`
  (`ev_e7bafe2f99fee821a61c05f670f800f02b89cae198c21ceaca847b47c705a1af`),
  so this setting is read as an integer.
- `Core.dll!0x10031050`
  (`ev_aa75a96cb36766f7ae2b9979d008917594e76b20a63e3a6ac49b42070007d3de`)
  removes an existing file at the same path, opens it with `w+`, passes the
  integer and shared `%d` format through `0x10037FF0`
  (`ev_5a7def5d5d5a51e1d7c15bed69fa6d7cf259f7e7`) and `0x10002FF0`
  (`ev_2816ac33769f2c0642af56da7ffc4b94bb2b3675d3e05232bcfb0fee0146a744`),
  then closes the file. The apparent `vfscanf` name at `0x100D87A0` is a Ghidra
  FID conflict: its body forwards to `common_vfprintf<>`, confirming this is
  the output-formatting path
  (`ev_fcbe26ab5bcd20cfc16f736996108bdf5731c4eb0d786ae0315419137ba63c79`).
  The argument context accessor `0x10002F90`
  (`ev_7065c5842a793bf888b2cf57ee2cdb1d17df3fb442a0c2bfddb35d323472070`)
  returns a fixed global context. These observations establish the static file
  read/write path, not successful writes or timing under a live server.
- The worker's time source is also bounded: `0x10012090`
  (`ev_c799c3d9bc0d64c0cbf4757a2389aa411dc4e664018f29e525a1c5122c54882b`)
  calls `__time64`, then `0x10012040`
  (`ev_f2f57f1ad650e1909d42743d9021f3b151513f5ddfec77ce532867145e478a8d`)
  passes that timestamp to the local-time conversion wrapper at `0x100D9650`
  (`ev_0ef52b6a7452391dcee60a1263781c2bfbd58832cf8d78c0391408265cb198c9`).
  Although Ghidra reports an FID conflict between 32-bit and 64-bit localtime
  variants, the caller supplies the value just produced by `__time64`. The
  common body at `0x100D8AE0`
  (`ev_bd65f6fa513ebde6ac482d770e829d3d16095f77afec5da6a4000aa90003c1fa`)
  obtains the CRT time buffer and invokes `localtime_s<>`. The same `tm` fields
  are used by the Tower pre-start branch (`sec=10`, `min=55`, configured
  `hour-1`) to announce the event five minutes ahead, corroborating the
  `tm_sec`, `tm_min`, `tm_hour` ordering used for the `LastServerBoss` gate.

## TMSRVIN.exe: opcode 0x36C packet construction

- `FUN_004DEB60` (`ev_3f00abc10c3c9171cc179da2c0118ee816eacfafa190982f9bb87843a3362d35`,
  input SHA-256 `27ee313bb4b4d4a0d897dac9a188d8249068ce701311cbda1e24ad28c5e43642`)
  constructs a 0x34-byte packet with length `0x34`, opcode `0x36C`, and a
  16-bit player-slot field. It also copies a global DWORD, reads two 16-bit
  values from the slot-indexed table at stride `0x928`, and writes constants
  into fields at offsets `+0x10`, `+0x14`, and `+0x18`. It also copies the
  low 16 bits of two parameters to offsets `+0x30` and `+0x32`; the remaining
  packet fields are not semantically resolved. The thunk at `0x004B16EA`
  (`ev_78c49989b3d8e8640e3b73df01d0ddd2b00ae0f87d61dc120ba9ba511916beff`)
  resolves directly to this body.
- Ghidra reports 16 static calls to that thunk: ten occur in seven recognized
  procedures (`FUN_005029F0`, `FUN_00572640`, `FUN_00572AE0`, `FUN_00575450`,
  `FUN_005755E0`, `FUN_00581250`, and `FUN_005818A0`); six callsites have no
  containing procedure assigned. The six addresses are outside the recovered
  body ranges of `FUN_00519CA0`, so they do not establish callers in that
  dispatcher.
- `FUN_005029F0` (`ev_da820883b3ad66c22532dfda13b41f43f181e0fd1415f5fe3126eab08a0df910`)
  reads the same two slot-indexed values, passes them and the slot to
  `FUN_004DEB60`, then calls `FUN_004D7850(buffer, 0x34)` when the slot is below
  1,000. REA's xrefs query returned no direct references to `FUN_005029F0`
  (`ev_6f6821eeecf61f2ac503170e4734db4f2bafdc9815fd822da8daf9ba744b99b8`),
  so its upstream trigger is not established.
- `FUN_00581250` (`ev_d4bcd61cf77fd0fbecbb880f531670e46000aa112cd8351455bbe0b1cf623361`,
  body `0x581250..0x581874`) calls `FUN_004DEB60` at `0x5815AE` and `0x5817A5`
  in two state-processing branches, then passes each constructed buffer to
  `FUN_00503360`. Its error format mentions a mob whose HP is zero, but this
  does not identify the `0x36C` packet's application-level meaning.
- `FUN_00575450` (`ev_859aab1765a7b4e9b8b9716c1a0addd3c5da2c1c47056a1a28e139eca11722c5`,
  body `0x575450..0x5755BC`) resolves coordinates through `FUN_004E0740` or
  `FUN_004E0310`, then constructs the `0x36C` packet. For entity slots below
  1,000 it calls `FUN_004D6F40` directly, then calls `FUN_00503360` to update
  the grid and process neighboring entities. In this caller, the sequence is
  consistent with a position/grid update; it does not establish that every
  `0x36C` callsite has the same purpose.
- `FUN_005755E0` (`ev_9bd63e6a6a8ef2e095f2d8ca598c23967f3f7be9abc3836999ad1c1c98d2926d`,
  body `0x5755E0..0x5756D0`) resolves coordinates with `FUN_004E0740`, builds
  the `0x36C` packet through `FUN_004DEB60`, and calls `FUN_004D6F40` for slots
  below 1,000. It then updates the grid through `FUN_00503360` and calls
  `FUN_004CE1C0` and `FUN_00504ED0`. This supports another spatial/entity
  update path, not city-war or Tower-War semantics.
- `FUN_005818A0` (`ev_b2b48d629a243d3287f3a18b99f9a0cb7381bdae84e3ae830e8561551b5b2bbb`,
  body `0x5818A0..0x582159`) contains three branches that construct the `0x36C`
  packet from slot-indexed coordinate fields and pass it to `FUN_00503360`.
  Its diagnostic string identifies a `battleprocessor` path, placing this
  caller in battle processing; the evidence does not distinguish ordinary
  combat from city war or Tower War.
- `FUN_00580F30` (`ev_e5e88c03280e4d2755c0cf6f1c7f2dc57a2d71537f2ba4cb6e00d5c649cb0b29`,
  body `0x580F30..0x58124F`) iterates entity records in modulo-based batches.
  Records whose state field is `4` are passed to `FUN_00581250`; records whose
  state field is `5` are passed through the jump thunk at `0x004B17BC` to
  `FUN_005818A0`. Instruction evidence resolves that stub as a five-byte
  unconditional `JMP 0x005818A0` and shows callsites at `0x5810F5` and
  `0x581162` (`ev_5ad701820d6b3fe76478d6b788b6e605e5122acaa1aeaf6d8d4da4d17d1ecbcc`).
  This locates the latter in a shared entity-state processing path, not
  specifically in a city-war or Tower-War dispatcher.
- `FUN_00503360`'s earlier dossier
  (`ev_6df810e4d612c89bbec1b4cefbd0acf040b60505f5d9a49eb785856fcf2a5710`)
  established grid occupancy and neighboring-entity processing. The follow-up
  dossier (`ev_a0d8bc8dcf594779e766707b55d9eff61f64993b9211e77f09191efcb2bada72`)
  shows it forwarding the supplied packet pointer and its length field to
  `FUN_004D6F40` for in-grid player slots below 1,000. `FUN_004D6F40` is already
  traced to `FUN_004D7140` and `FUN_004D7570`, which frame the packet and reach
  Winsock `send` (`ev_c4ccf051dede1b4fdb2b52a17abcb0d85eba4bd1601be3914e488a7b493fcec0`,
  `ev_0d35d62d7da40eb4b42418f06a9a6e6a49b7d267e87c43249ee40212fc0db15e`,
  `ev_73013d7395f16620ba667a3581bd4809445dbcc948f33c59471083cdee5992ae`).
  Together, these static paths support an outbound spatial-multicast send
  attempt for the constructed `0x36C` packet; they do not prove runtime delivery
  or establish city war, Tower War, fame, or reward semantics.
- `Core.dll!FUN_100248E0` separately compares against opcode `0x36C` in its
  recovered packet-gate list (`ev_09753ead2133c462bb1a7130127ff5ff42f955bacd52939a2fb176706e3b82ac`).
  This overlap does not identify the packet's feature: the available evidence
  does not connect it to city war, Tower War, fame settlement, or a reward.
  Runtime receipt and the origin of the other six callsites remain unverified.

## TMSRVIN.exe: opcode 0x3AF player-slot message

- The five-byte thunk at `0x004B196A` resolves to `FUN_00505610`
  (`ev_3f27e903e388eec77632eac361f0ba36eaab811905563d865fbdccc4f3251df4`).
  The target body (`ev_6bc4d58875867a16e0db11f9ebad322cbd569a292dbdfa14aac0ff48280cadb0`,
  `0x505610..0x5056FB`) accepts slots 1 through 999 only when the slot state
  equals `0x16` and a second slot-indexed field is nonzero. It constructs a
  16-byte message with opcode `0x3AF`, includes the 16-bit slot and a DWORD from
  the table at `0x07D84AD8 + slot * 0x928`, then calls `FUN_004D6F40`. When that
  call returns zero, it invokes `FUN_0056A320(slot, 7)`. The opcode's gameplay
  meaning and any relation to war, fame, or rewards are not established.

## TMSRVIN.exe: keyed packet fan-out

- `FUN_00502250` (`0x502250..0x5022DF`, 144 bytes;
  `ev_a63768ccf8eec824d259160bfa9b5207afdb30a5a33c7a54eaafbb72c5b4e3f8`,
  `build/rea-analysis/results/TMSRVIN-function-00502250-REA-evidence-20261021.json`)
  scans 1,000 slot records. It selects records whose state is `0x16` and whose
  16-bit field at `0x07D84AD2 + slot * 0x928` equals `param_1`; each match is
  passed to `FUN_00508270(slot, 0, param_2, param_3)`. The body is reached via
  thunk `0x4B1DF7`, and its call at `0x5022C2` goes through thunk `0x4B1041`.
- `FUN_00508270` (`0x508270..0x50840E`, 415 bytes;
  `ev_b6ee0cbae127e35ba87e5360d7d9c445dc60ec96ee7020f03cacbb5d12a4a2d6`,
  `build/rea-analysis/results/TMSRVIN-function-00508270-REA-evidence-20261021.json`)
  validates the slot and payload pointer, then builds a `0x90`-byte packet.
  The observed header stores `0x90` at offset `+0` and `0x106` at `+4`; these
  values alone do not establish the protocol meaning of that field. Text copied
  into the packet is capped at `0x80` bytes. With mode `param_2 == 1`, the
  helper writes the slot and reads its grid coordinates from the slot table,
  then calls `FUN_00503FF0` (`GridMulticast`) with excluded ID zero. Otherwise,
  for slots below 1,000, it calls `FUN_004D6F40` with the packet and size `0x90`;
  a zero return invokes `FUN_0056A320(slot, 0x14)`. In the currently mapped
  caller, mode is zero, so each selected slot follows this packet-sender branch.
- The key's business meaning, the packet field `0x106`, payload semantics,
  caller identity, and runtime delivery remain unresolved. This is static
  evidence of a keyed active-slot fan-out path, not proof of a particular
  feature such as guild, party, or war messaging.

## TMSRVIN.exe: war and ally ID-pair maps

- Startup `FUN_00517DB0` clears both maps (`0x20000` bytes apiece); its saved
  dossier is `build/rea-analysis/results/TMSRVIN.exe.app-entry.json`
  (`ev_8f37740ca6d7be2e21865e5cc776ed1d4c61e362be492871fcb22a9afe0b6ff8`).
  Ghidra xrefs report 26 exact reference sites for `0x061802B0`
  (`ev_10c818379821b756d81c45b4ae116ec06b6a417801fdd2fb9328a0d1059d0e72`)
  and 14 for `0x061602B0`
  (`ev_ec4b06e0bb8c3331bfa43732de7d929587005cbdae383be86cb514a481125745`).
  The xrefs operation does not expose reference kinds; the role assignments
  below come from directly analyzed function bodies, not from counts alone.
- `FUN_00572CF0` (`0x572CF0..0x573141`, 1,101 bytes across two body ranges;
  `ev_b2fcccade57b13ffcb955f481a7b875a01dca442731fb0c65493d55f0728fc1c`,
  `build/rea-analysis/results/TMSRVIN-function-from-xref-00573004-REA-evidence-20261021.json`)
  operates on 16-bit IDs in `0x061802B0`, bounded by `DAT_00647770`. Its
  embedded `dowar` error strings and state transitions identify this as a war
  pairing/request map: a request writes `ID A -> ID B` and sends message `0xD2`;
  a reciprocal request sends `0xD3`, retains the reciprocal entries, and
  refreshes matching active sessions through `FUN_00506240`. Clearing with a
  zero target removes both entries and sends `0xD4` when the pair is reciprocal;
  if it is not reciprocal, it sends `0xDD` and clears only the caller's entry.
  The clear path also calls `FUN_00588340` for both IDs with `-100` and `100`;
  the meaning of that adjustment is not established here. These are static
  control-flow and message-ID observations, not proof of client-visible packet
  semantics or live war behavior.
- Ghidra xrefs identify one callsite to thunk `0x4B2559` at `0x556135`
  (`ev_bb910a34aceeb0f8808c7ef59392fee18c3c5cd2ca423d34216cba9d60731870`).
  A separate xrefs query finds one reference site, `0x55615F`, to the ally
  thunk `0x4B277A` (`ev_2f07b8d257b521ff5e516aea26283855007a61b2e6ff91d50656051f1b8f5afd`);
  the `FUN_00573180` dossier identifies that thunk as its incoming reference.
  Both sites lie in `FUN_00555600`'s `0x556118..0x55767A` body range, recorded
  in the saved `0x4B16AE` thunk dossier
  (`ev_2c3fc6a89a2563c1630dd1767fd961fe660bb2e4eaac629f3d739b4367fcd278`).
  Direct analysis maps `0x4B2559` to the five-byte thunk for
  `FUN_00572CF0` (`ev_f73024e66c7f8ad3eb017bb794b3fea8ea26be209db73494f8634a1f843a2b83`),
  establishing its war-pair target. Direct analysis of `FUN_00555600`
  (`ev_4824a03512114fbfeb823c7315f97b18d8c0049f4f1acc4f671e48d36885a553`,
  `build/rea-analysis/results/TMSRVIN-FUN-00555600-REA-evidence-20261021.json`)
  confirms the callsites: the dispatch guard requires bit `0x400` in the
  16-bit selector at byte offset `+4` and bounds the 16-bit field at `+6` to
  999. When the latter field is zero, selector `0xE0E` calls
  `thunk_FUN_00572CF0(param_1[3], param_1[4])`, while selector `0xE12` calls
  `thunk_FUN_00573180(param_1[3], param_1[4])`. This establishes distinct
  server-side routing of the two payload values to the war and ally ID-pair
  mutators. Their wire-level field names, client interpretation, and the
  response-code semantics remain unresolved.
- `FUN_00573180` (`0x573180..0x57340E`, 650 bytes across two body ranges;
  `ev_8ad0cebb857872ce0f53b5a6eb5ed1d8ea5f17cc9431165ac8d48ebd97e28a65`,
  `build/rea-analysis/results/TMSRVIN-function-from-xref-0057325A-REA-evidence-20261021.json`)
  operates on `0x061602B0`; its `doally` error string and operations support
  identifying this as an ally-ID map. A nonzero target is stored as
  `ID A -> ID B` when A's slot is empty and emits message `0x11F`; a zero target
  clears both entries for an existing in-range pair and emits `0x11E`. It scans
  active sessions for A and calls `FUN_00506240` after the state transition.
  The exact message payloads, client interpretation, and whether this map
  represents an accepted alliance or a pending invitation remain unresolved.
- In the large request dispatcher `FUN_00519CA0`, static instruction evidence
  shows reciprocal-link checks before emitting response codes: the
  `0x061602B0` ally map is checked in both directions before a call carrying
  `0x121`, and the `0x061802B0` war map before one carrying `0xD6`
  (`ev_14da12a13766379cd884793af6559aec2b578801bd04e37e5a379430c55fb1a8`).
  This links both maps to request validation, but the response-code and packet
  meanings remain unresolved.
- The individual `FUN_00572CF0` and `FUN_00573180` dossiers list their incoming
  thunk references, while the separate `FUN_00555600` analysis above resolves
  the caller and dispatch cases. `FUN_00506240` reads the two maps by the slot's
  16-bit ID: it validates the war-map value by requiring a reverse
  mapping to the original ID, then passes that value (or zero) and the ally-map
  value to `FUN_00502700`. The mapped `FUN_00566D60` settlement path calls
  `FUN_00506240` at `0x56762E`, but that does not make the helper itself a war
  settlement or reward routine.

- `FUN_00506240` (`0x506240..0x50633A`, 251 bytes;
  `ev_d61f6cb5cc7177e32615edc9f91922b97b1b4fdb3855775dcd5278e7aa71f7b0`,
  `build/rea-analysis/results/TMSRVIN-function-00506240-REA-evidence-20261021.json`)
  reads the 16-bit field at `0x07D84AD2 + slot * 0x928`, the same slot-record
  field tested by `FUN_00502250`. It looks up two values in separate tables at
  `0x061802B0 + key * 2` and `0x061602B0 + key * 2`; the first value is accepted
  only if a reverse lookup maps it back to the original key. It then calls
  `FUN_00502700(slot, 30000, 0x3A8, validated_value_or_zero, param_2,
  second_table_value)` through thunk `0x4B2315`.
- `FUN_00502700` (`0x502700..0x502792`, 147 bytes;
  `ev_b8314eaf7065e46f80d252f280402da777ae89787540d60e5b0183fe9948315e`,
  `build/rea-analysis/results/TMSRVIN-function-00502700-REA-evidence-retry2-20261021.json`)
  derives a transport context at `0x061AAAD8 + slot * 0xFC8` and passes a
  24-byte stack buffer to `FUN_004D6F40`. In the mapped caller, the values
  `0x3A8` and `30000` are written at buffer offsets `+4` and `+6`, while
  arguments are also written at `+12`, `+16`, and `+20`. The traced sender
  `FUN_004D7140` (`ev_0d35d62d7da40eb4b42418f06a9a6e6a49b7d267e87c43249ee40212fc0db15e`,
  `build/rea-analysis/results/TMSRVIN.exe-war-network-method-4d7140-REA-evidence-20261019.json`)
  constructs the transport header at `+0..+3`, overwrites `+4..+7` with
  `DAT_061AA0F0`, then transforms bytes from `+4` through the declared length
  into the connection's output buffer. Thus the caller's `0x3A8` and `30000`
  do not survive as those payload bytes; offsets `+8..+11` remain the explicit
  `0xCCCCCCCC` stack-fill sentinel in this builder and are consumed by the
  sender's transform. This is a suspicious packet construction, but the
  expected packet schema and runtime effect are unresolved; no defect is
  claimed from static analysis alone.

## Limits and next research

- No live server execution or in-game behavior was tested. Do not describe
  these observations as runtime-validated parity.
- The full `FUN_00519CA0` decompilation previously exceeded available memory.
  The saved instruction-flow evidence was used instead; avoid repeating the
  same whole-function decompilation without a narrower target or method.
- The config-list size, allocation, range-copy wrappers, tested stream state
  bits, cached `std::num_get` facet, unsigned-long conversion path, default
  autodetected numeric base, whitespace-token behavior, and effective
  `0x3fffffff` element-count bound are now traced. Remaining list uncertainty is
  the concrete meaning of the 200-byte stream object's fields at `+4`/`+0xc`,
  the linked structure modified by `0x10039780`, snapshot-to-binary identity,
  and runtime acceptance. The supplied snapshots use one numeric ID per line,
  while the parser itself does not enforce line boundaries. Next, trace the
  producer/consumer semantics for both Cash filename variants and identify
  the meaning of the caller object's `+0x12` field and the producer for its
  numeric Cash file. The arguments and response flow at `0x10015A9D` are now
  statically traced. The `+readautotrade` trigger at `0x10016FAB` and its
  `AutoTrade reloaded!` response are also statically recovered. The recovered
  weekly-reset candidate is now bounded: `0x10011A60` has no weekly fame reset,
  and `0x10030C70` has no direct durable write. The connector's configuration
  source, port, and event are now statically resolved. `DBSRVIN`'s `0xC17`
  type-4 receiver and its writer for the full guild record are also statically
  traced; remaining fame gaps are the weekly trigger and whether the outbound
  reset packet is actually delivered and processed at runtime.
  Continue the cash/gift worker's file producer/consumer trace separately. The fame-base
  xrefs are now classified at instruction level; do not repeat that sweep unless
  new evidence appears. Continue into the post-purchase helper chain only if
  proving finite-stock mutation is required. The `LastServerBoss` file
  path is resolved above and should not be re-opened without new evidence. The
  transport path from `0x10030660` to
  Winsock `send` is statically traced, but the fixed socket's configured
  endpoint values and runtime delivery remain unverified. After those bounded
  paths, map remaining relevant functions in `Core.dll` and the executable.
  Any new `0x16E2` protocol claim needs a separate observed transport path.

## DBSRV: exchange-stone name swap

- Input: `references/decompilacao server/dlls/DBSRVIN.exe` (1,010,688 bytes),
  analyzed with Ghidra 12.1.4 headless (scalar search for 0x306/0x307).
- `DBSRVIN.exe!FUN_00471210` (`0x4744d9`, `0x474513`; repeated at
  `0x4755fc`/`0x475636`) runs at account load: over the four characters it
  records the last slot whose item at STRUCT_MOB offset 244 (`Equip[13]`) is
  774 (Master_Switch_Stone) and the last whose item is 775
  (Slave_Switch_Stone). With both present it swaps the two 16-byte names,
  zeroes both item indices and logs `etc,name swap %s %s`.
- `TMSRVIN.c` only references 774/775 in the SwapItem trade guard.
- Applied in `wydgo748/internal/game/switch_stones.go`; WYD-Go additionally
  rejects ambiguous stones and guild members, because its guild roster is
  keyed by name.

## DBSRVIN: request dispatcher and time wrappers

- `DBSRVIN.exe!FUN_00471210`
  (`ev_c6090371cfada3010c88e82829cc7e1ee0f92569b77c01f96fd80c819777d5af`,
  input SHA-256 `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`)
  is the procedure containing query address `0x4744d9`; that address is not a
  separate caller function. The procedure has 13 noncontiguous body ranges,
  38,113 bytes of code, 8,756 captured instructions, and 972 basic blocks. It
  is a large request dispatcher with character and guild database operations.
- Within that dispatcher, the `0xE0E` branch logs `guild war request` with two
  request fields, bounds-checks the first field to `1..65535`, writes a
  16-bit value from a local calendar-time buffer to a global indexed table,
  and forwards both request fields to active server slots. The `0xE12` branch
  logs `guild ally request`, writes a 16-bit value from its local calendar-time
  buffer to another indexed table, and forwards both fields to active server
  slots. These labels and forwarding paths do not by themselves identify a
  city-war or tower-war settlement mechanism, fame accounting, betting rules,
  persistence, or the meaning of the indexed values.
- `DBSRVIN.exe!FUN_0047B200`
  (`ev_09b7e4468d200d635c4c821b7bb7c63720a3093833db5473364aafd238a4d899`,
  body `0x47B200..0x47B2A1`) builds a 0x14-byte local message, stores its
  length at offset `+0`, copies `param_3` and `param_2` to offsets `+4` and
  `+6`, and places `param_4` and `param_5` at `+0xC` and `+0x10`. It passes the
  message and length to `FUN_004847A0`. No opcode is set in this body, and the
  available evidence does not connect it to the `0xE0E` branch or establish a
  war-related meaning.
- `DBSRVIN.exe!FUN_00470320`
  (`ev_77f496ad6f68444eda4bb0435edde8a452e3e3c2df065a2c8a038b62f136c6bc`)
  wraps `__localtime32`. The dispatcher calls it after `FUN_00470360` and uses
  its returned `EAX` as a pointer to copy nine 32-bit calendar fields, despite
  the decompiler's `void` signature. This return use is supported by the
  caller pseudocode; exact CRT type recovery remains imperfect.
- `DBSRVIN.exe!FUN_00470360`
  (`ev_d584971bee84225a53c0d162b858fbf85c31b2b2da96d0fa8f43061d123f1738`)
  wraps `__time32` and is called with the destination passed to that CRT call.
  Ghidra also types this wrapper as `void`; its output and return semantics
  should not be inferred beyond the call and the observed dispatcher flow.
- The `__localtime32` wrapper has two callers, and the `__time32` wrapper has
  six. The dispatcher branches are static database-server evidence only; no
  runtime or player-socket path has been established here.

## DBSRVIN: new-account record path

- `DBSRVIN.exe!FUN_0046FBB0`
  (`ev_4111b75c9c318cc1c5369bab1da7bfb97a320757f493e6b9d0f6bcef833a6114`,
  input SHA-256 `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`)
  handles the `./Newaccount/%s/%s` path. It uppercases the supplied name,
  special-cases `COM`/`LPT` followed by one digit, derives a directory
  component through `FUN_004616A0`, and checks whether the corresponding file
  can already be opened. The existing-file and special-name paths return
  without entering the record-initialization branch.
- On the missing-file branch it initializes an in-memory record and fills
  defaults according to its sixth parameter. Values 3 and 5 call the
  `__time32`/`__localtime32` wrappers, copy nine 32-bit fields to a shared
  calendar buffer, and derive another value from `DAT_0052667C` using offsets
  `0x23280` or `0x46500`. The function then delegates the record to
  `FUN_0047C480(buffer, 0, 1)`; the callee behavior is traced below.
- `DBSRVIN.exe!FUN_0047C480`
  (`ev_3396365a2d3c47973ef14dda0120152741916d77ffae34ab8cda8e1d85c9a7cf`,
  input SHA-256 `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`)
  routes its second parameter `0`, `1`, or `2` to `./Newaccount/%s/%s`,
  `./account/%s/%s`, or `./OLDaccount/%s/%s`, respectively; other values and
  `COM`/`LPT` plus one digit return without writing. It calls `__time32` and,
  when the third parameter is nonzero, stores that value at record offset
  `+0xB8`. It then opens the selected path, seeks to offset zero, and calls
  `__write` with the record and size `0x1F80` (8,064 bytes). The observed
  success branch closes the file and returns 1; error branches log and return
  0. This confirms a static file-write path for the caller's
  `FUN_0047C480(buffer, 0, 1)` call, not runtime execution or durable-storage
  guarantees.
- `DBSRVIN.exe!FUN_004616A0`
  (`ev_02e35b7d770b4eecdeb67e796e7f38ee6e5040a717027772bc707df285bbcca2`,
  input SHA-256 `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`)
  is the target of the Ghidra thunk `thunk_FUN_004616A0` at `0x45643C`;
  `FUN_0046FBB0` and `FUN_0047C480` call that thunk at `0x46FCAC` and
  `0x47C584`, respectively. It derives the account-directory component from
  the first byte of the supplied name: ASCII letters are copied as a one-byte prefix;
  non-ASCII leading bytes are converted to an index using the first two input
  bytes and compared against 18 thresholds at `DAT_00522AE0`, then select a
  two-byte prefix from `DAT_00522AB8`. Values outside the accepted multibyte
  range, or non-ASCII bytes that do not take that path, use `"etc"`. The
  caller uppercases the name before this helper in the new-account path. This
  is static function evidence, not proof of filesystem behavior at runtime.
- A read-only byte query of the table region
  (`ev_c1f74e67d48495a9f408c7c91ed415a992a9ddfaca06c496b435cfa0f2eca9d4`)
  resolves the 18 thresholds at `0x522AE0..0x522B24` as little-endian values
  `171, 291, 432, 560, 646, 773, 902, 1031, 1104, 1237, 1353, 1561, 1696,
  1770, 1890, 1997, 2103, 2208`. The preceding bytes contain 19 two-byte
  values at `0x522AB8..0x522ADB`: `A4A1, A4A2, A4A4, A4A7, A4A8, A4A9, A4B1,
  A4B2, A4B3, A4B5, A4B6, A4B7, A4B8, A4B9, A4BA, A4BB, A4BC, A4BD, A4BE`,
  followed by `0000`. The helper accepts only computed indices 0 through 17,
  so the 19th value `A4BE` is not selected by its success branch. When all 18
  thresholds have been crossed (computed index 18, corresponding to values
  from 2208 through the accepted upper bound `0x92D`), the helper instead
  returns `"etc"`. This is a static table/control-flow discrepancy; intent and
  runtime effect remain unverified.
- `DBSRVIN.exe!FUN_0048EAE0`
  (`ev_fd0c854eecce12e39fcfc9a59982d420a34ddc82a4e17e568acdb3ec0a0f1f99`,
  input SHA-256 `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`)
  is an upstream dispatcher for the account-record path. Its branch for
  `param_2 == "_A_ _A_"` parses `param_1` with `"%s %s %d %d %d"` and forwards
  the parsed fields through the call at `0x48EBF8` to
  `thunk_FUN_0046FBB0` (`ev_ba015f6275baa07e3da239d3957e7f714a84b7e180888bf5bffd3d0944d69e4f`),
  which resolves to `FUN_0046FBB0`. This statically connects the dispatcher to
  the new-account path above; whether this command represents account creation
  or another operation, and whether the path executes at runtime, remain
  unverified.
- `DBSRVIN.exe!FUN_0048DB80`
  (`ev_5381d4efaf35c529dedeb2507582003b46523b2293b4e33bb3e7660d2fb223f1`,
  body `0x48DB80..0x48DC8B`, 268 bytes,
  `build/rea-analysis/work/DBSRVIN-account-callee-0048db80-REA-20261021-retry.json`)
  is reached from `FUN_0048EAE0` through thunk `0x455EB0` in the file-open-error
  fallback. It formats a timestamped CSV-like line, writes it through
  `_fprintf` when the log stream is non-null, then displays the same text with
  `SetWindowTextA`. This is static evidence of a logging/status helper, not
  successful account persistence or login acceptance.

## DBSRVIN.exe: periodic callback and weekly fame-reset check

- `DBSRVIN.exe!FUN_0048D8E0`
  (`ev_373ff41b4c6e5127d6ed8fbcbc5c4d8602a641a33741716a4a2c54037b49c51b`,
  `build/rea-analysis/results/DBSRVIN-weekly-reset-timer-0048D8E0-REA-20261021.json`)
  increments `DAT_027355AC` each time it runs. Every `0x78` counter cycles it
  calls the mapped `FUN_0048F410` item-file importer. On multiples of `0x1E`,
  it calls `FUN_00487E50`; the argument is `0` at multiples of 600 and `1`
  otherwise. The mapped `FUN_00487E50` builds the online-player snapshot and
  sends opcode `0x300E`. A thunk xref query
  (`ev_a9ae1e67011566cbe8dda1e2e0a2cd9b8f995cc25ce93ff1ae0d49fd681ced16`,
  `build/rea-analysis/results/DBSRVIN-weekly-reset-timer-thunk-0045514A-xrefs-REA-20261021.json`)
  locates a caller at `0x489E1C`, inside the already mapped
  `FUN_00489D50` window-message/socket handler. The timer's cadence in wall-clock
  time is not established, and its 600-cycle branch is online-list maintenance,
  not a weekly guild-fame reset. This rules out this callback as the reset
  mechanism; the weekly reset trigger elsewhere remains unresolved.

## DBWYD.dll: attach, process discovery, and server patch flow

- Input SHA-256 for all records below:
  `58f41da172228dc007d02b620d15b791e36344f7b559002cac0bd66521384d85`.
- `DBWYD.dll!entry` at `0x100022BB`
  (`ev_e0f77b330e3a8f48767b64bc69ae9948e0b4e4b8ca96acd4b703bfd0b1d449da`)
  is an x86 three-parameter stdcall entry. When its second parameter equals 1,
  it initializes the security cookie, then forwards all parameters to
  `FUN_1000217B`; it returns with `RET 0xC`.
- `FUN_1000217B`
  (`ev_02367e2ebf74f6ef9c35d561c1b2fb9a54554677496c689895f53ea31e964112`)
  performs the CRT/raw DLL attach and detach dispatch and calls
  `FUN_10001700`. On attach (`param_2 == 1`), `FUN_10001700`
  (`ev_a10ed87358f86722c4e7d4f7789aded7689549fe40d28fda1dea94a63a4541b3`)
  temporarily changes protection for `0x401000..0x5FFFFF`, disables thread
  attach notifications, and enters its initialization path. A separate error
  branch calls `FUN_10001890`
  (`ev_8c8c196cdc91587a19adfef82cdbff369255841d3fff322385cbc3cdb0e633a7`),
  which displays `DB ERROR!` / `Error: DBWYD.dll`. These are static branches,
  not observed runtime outcomes.
- `FUN_10001000`
  (`ev_b2c7e8fb0e8ec3ba2aba9cb916a754fa56b58a944a141f538780a22cb154d075`)
  allocates a console, redirects stdout to `CONOUT$`, enumerates process IDs,
  and calls `FUN_10001180(pid, "DBSRVIN.exe")` for each. The process helper
  (`ev_f41c8fe4c7a18d54b31af5c2d6cd39c8ff4394793e9ce35fb2e4197009496714`)
  opens each PID with access mask `0x410`, obtains the first enumerated module,
  compares its base name case-insensitively with `DBSRVIN.exe`, and, on a match,
  stores `module_base + 0x55000` in `DAT_1004DC08`. This establishes the intended
  host process name and computed base; it does not establish that a live
  process was found during this analysis. The decompiled `FUN_10001000`
  returns a low byte of 1 on both visible branches, so that return value alone
  is not evidence that process enumeration or patching succeeded.
- A follow-up checked the separately alleged target `0x432FCF` against both
  analyzed `DBSRVIN.exe` images. Neither image has initialized bytes or a file
  offset there; in the primary image the address is in the uninitialized
  `.textbss` block (`0x401000..0x454C01`), so no containing procedure or code
  flow can be recovered at that address. The DLL's process helper first adds
  `0x55000` to the module base, and the setup then adds `0x32FCF`, resolving
  the supplied images' actual computed hook to `0x487FCF`. Therefore
  `0x432FCF` is not a file-backed inline-patch target in either analyzed image;
  this conclusion is limited to those artifacts and does not establish runtime
  patch success (`ev_ad912c72b6d03202bdb2b0db110db919fe010aa4237a46860404ce93d3c281e8`,
  `ev_accaa16bf1591f2390172240b1e0fc44a5b4bacc7186a3e257e27ea47dcf1379`,
  `ev_8a4669c68632db2d9065c8fb6abc971aac7a06fc8c9816ad963ba8e00b291ac8`,
  `ev_f41c8fe4c7a18d54b31af5c2d6cd39c8ff4394793e9ce35fb2e4197009496714`,
  `ev_b2c7e8fb0e8ec3ba2aba9cb916a754fa56b58a944a141f538780a22cb154d075`).
- `FUN_10001390`
  (`ev_60782910cc8e72566387ba6b673501136029e76fd40ac059640437115ab2c437`)
  temporarily applies `VirtualProtect`, writes one byte, then restores the
  prior protection. `FUN_100013D0`
  (`ev_a9f88114b136d24dedc1e77f20c59c35aef9d985663c305d2361a84270b25ea6`)
  does the same for one DWORD. These helpers are used on fixed addresses in
  the server image. The direct setup calls write byte `0x90` at
  `0x48A839`, `0x48A83A`, `0x48A84E`, `0x48A84F`, and `0x48F441`, and byte
  `0x3F` at `0x48CC84`.
- A targeted patch-writer audit accounts for all ten direct `DBWYD.dll`
  references to its `VirtualProtect` import and confirms two two-byte NOP
  patches in `DBSRVIN.exe!FUN_00489D50`: original `EB A8` at
  `0x48A839..0x48A83A` and original `75 0E` at `0x48A84E..0x48A84F` are each
  replaced with `90 90`. Both writes are made through `FUN_10001390` from the
  startup writer path. The audit found no additional statically resolved
  `E8`/`E9` inline-hook installer chain or callback, but indirect/computed flows
  and the semantics of unresolved path/string rewrite targets remain outside
  that bounded result (`ev_0b7e927b97b879fc52dbcf3ffa1e511a315c2cb8d37ef0689622c69c897458b3`,
  `ev_5839785915b280901c04ed0e5b36bb6f531797b1be8cc10f2cd4b98ad21617c4`,
  `ev_6e83e3f43b8658b25d4a4534a2b23ce55cfcb923004ff66eb6e70421cfa0b53b`,
  `ev_e3011c32ca996d2a7f6b3af79002a837b97484302bbca1db5679cb0e6a293ea5`).
  This confirms the bytes and static write path, not execution or the runtime
  effect of either bypass.
- Direct decompilation of `DBSRVIN.exe!FUN_00489D50`
  (`ev_4f070fd13662efff86448e27dacc3e091bf6e9a9644e6b4dea06a8f6c88dc7af`)
  identifies it as a window-message/socket handler. Its visible branches process
  Winsock accept (`0x403`), admin accept (`0x405`), admin-socket read (`0x406`),
  admin-client read (`0x407`), and game-server read (`0x464`) events. This places
  the patched bytes inside that network-event procedure, but the recovered
  pseudocode does not include either patch address, so their exact case,
  predicates, and application-level effect remain unresolved.
- At `DBSRVIN.exe!0x48F441`, the original instruction is the one-byte `PUSH EAX`
  (`ev_e3fb633f400880b580360858c3b48b53b500884b9507abe42fb93b5865d9f3ef`),
  and the containing procedure is `FUN_0048F410`. Direct decompilation
  (`ev_e5935cb578e726f0374fa11acbf8d0c2412065edd85ce10d84e3a199b554b728`)
  identifies that function as an item-file importer: it enumerates
  `Q:/serv%2.2d/item/*.*`, parses each file, attempts to update account cargo,
  and moves or deletes files based on validation and save results. The byte
  window (`ev_514e9715ec1b22c4099b447eb3deb5ecb7530ada45c0736e8fd089c3e0c0e2b1`)
  shows the patched `PUSH EAX` supplying `DAT_00524B8C` to the initial
  `_sprintf(local_214,"Q:/serv%2.2d/item/*.*",DAT_00524B8C)` call. Replacing
  it with `NOP` would remove the explicit server-index argument, but the same
  startup routine `FUN_10001250`
  (`ev_6edfd537ffef3b3740f9a7822636db9ad2307be00c12b2b1bcad1aaed223dc5b`)
  also replaces the following `PUSH imm32` string pointer at `0x48F443` with
  `0x1004321C`, the DLL string `../../WYD_DB/ImportItem/*.*`. The new format has
  no `%2.2d` conversion, so removing the server-index vararg is a coordinated
  static path redirection, not evidence of a malformed `sprintf` call. The
  same setup routine writes `0x08` at `0x48F455`; the byte window shows this is
  the immediate of `ADD ESP,0x0C` beginning at `0x48F453`. The direct address
  query (`ev_2e3f096c70db7ea741e7f8f64cf2a53e175d4fafddadbeded411e548b221b9bf`)
  correctly classifies `0x48F455` as inside, not at the start of, that
  instruction. Reducing the cleanup from 12 to 8 bytes matches removal of the
  one four-byte server-index argument and independently supports a coordinated
  call-site rewrite. The intended shared import directory is supported by
  these writes; execution of the startup patcher and successful runtime import
  remain unverified.
- The full instruction listing for the same importer
  (`ev_e5c6730b02773eb9c1319ddf791e4bc1ec6c1763635471ea416af6b468cde8de`)
  confirms two matching filename-format call sites. At `0x48F4F9`, the DLL
  removes the server-index `PUSH ECX`, replaces the format pointer at
  `0x48F4FB` with `../../WYD_DB/ImportItem/%s`, and changes the `ADD ESP` byte
  at `0x48F50D` from `0x10` to `0x0C`. At `0x48F51B`, it makes the same
  three-part change for the error path, replacing the format at `0x48F51D`
  with `../../WYD_DB/err_ImportItem/%s` and reducing cleanup at `0x48F52F` from
  `0x10` to `0x0C`. These paired path, argument, and stack-cleanup changes
  statically support redirects for both imported files and rejected/error
  files while retaining the filename `%s`. The matching patch writes are in
  `FUN_10001250` evidence above; runtime execution and filesystem outcomes are
  not established.
- The same setup routine also redirects the update-file importer
  `DBSRVIN.exe!FUN_0048DE50` (`ev_815ebbefe42d5c25f66a1716dfbf9d8d205153331a0a3833c87ee4d8b2ea8b09`,
  queried at `0x48DE7D`; body `0x48DE50..0x48E38C`). Its original pseudocode
  enumerates `Q:/serv%2.2d/update/*.*`, opens each matching file as text, parses
  two bounded strings and up to two integer fields, and calls
  `FUN_004708F0`; when that call returns nonzero, it deletes the processed
  file. The patch writer replaces the glob format with
  `../../WYD_DB/Update/*.*` at `0x48DE7F`, writes a NOP at `0x48DE7D`, and
  writes byte `0x08` at `0x48DE91`. The filename formatter is similarly
  redirected to `../../WYD_DB/Update/%s` at `0x48DF1A`, with a NOP at
  `0x48DF18` and byte `0x0C` at `0x48DF2C`. The original formats include the
  server-index conversion and argument, while the replacement formats do not;
  this statically supports a paired path/argument adjustment. The original
  cleanup immediates have not yet been separately decoded, and runtime file
  processing remains unverified. The separate `data%2.2d.csv` redirection at
  `0x4880FE` is now traced to a 4-byte pointer write by `DBWYD.dll!FUN_100013D0`
  (`ev_a9f88114b136d24dedc1e77f20c59c35aef9d985663c305d2361a84270b25ea6`),
  called by the setup routine with that address as its destination
  (`ev_6edfd537ffef3b3740f9a7822636db9ad2307be00c12b2b1bcad1aaed223dc5b`).
  The backing bytes are available and begin `68 4C 6D 50 00` at
  `0x4880FE` (`ev_820be638c79ef4658069afdc746f9d9622409b1bad45ebe8037359cc2537ad65`,
  address mapping `ev_d138cc72f106576624dca7ba43d69dc542454de0856214577c96f624256a0658`).
  A linear x86 decode would read these as a `PUSH` immediate, but Ghidra still
  reports the address as `undecodable` with no containing procedure
  (`ev_77d0e1bb856cc071caf3ecac69cc357588463bed1fb27e07fdae1efc403fbfbe`), and
  the 4-byte write starts at that candidate opcode. Thus the pointer redirection
  is evidenced, while the code/data classification, caller flow, and resulting
  file behavior remain unresolved. A bounded audit found no additional direct
  writer, but does not rule out indirect writes. This site adds no function to
  the tally.
- The `0x48CC84` write targets the immediate byte of the instruction beginning
  at `DBSRVIN.exe!0x48CC7E`: `CMP dword ptr [EBP-0xA900], 0x40`, followed by
  `JGE 0x48CD58`. In `FUN_0048B920`, the decompilation identifies this as a
  loop over 64 records with an 8-byte comparison stride, nested inside a
  four-entry outer loop; changed records are passed to `FUN_0048B3A0`. Replacing
  the comparison immediate `0x40` with `0x3F` makes the loop process indices
  `0..62` instead of `0..63`, omitting its final record on each outer pass.
  The record's application-level meaning and the runtime effect are not
  established by this static analysis (`ev_6edfd537ffef3b3740f9a7822636db9ad2307be00c12b2b1bcad1aaed223dc5b`,
  `ev_385a6978da911e960d3d6f1eeb7621ebf3dd2e9aa6809834dde187ec2663e4e8`,
  `ev_4b703d2a0d8552fac6a123b57796f64c0b52be585c97c79cbd9946537388446a`).
  The follow-up boundary audit reports that `0x48CC84` is not itself an
  instruction start; this is consistent with the earlier decode because the
  patched byte is inside the immediate operand of the instruction at `0x48CC7E`,
  not a separate branch or third hook (`ev_5ca75344695986075ca062b87394623a9987d500f6070e496aba24e9f465d87a`,
  `ev_3df5fddc7d8e4e59a4f3d2db087912b1dadac6de0ba45fdf9b03718a3d3f7a8b`).
- `FUN_10001250`
  (`ev_6edfd537ffef3b3740f9a7822636db9ad2307be00c12b2b1bcad1aaed223dc5b`)
  applies additional immediate-value and byte writes in the server image.
  Among them, it writes pointers to DLL strings for
  `../../WYD_DB/ImportItem/*.*`, `../../WYD_DB/ImportItem/%s`,
  `../../WYD_DB/err_ImportItem/%s`, `../../WYD_DB/data%2.2d.csv`,
  `../../WYD_DB/Update/*.*`, and `../../WYD_DB/Update/%s`, alongside nearby
  byte values `0x08`, `0x0C`, and `0x90`. This establishes attempted path/string
  redirection in the patch setup; the exact semantics of every modified server
  instruction and resulting runtime file behavior remain unverified.
- The main setup also calls `FUN_10001560` with target
  `module_base + 0x87FCF`, callback address `0x10001450`, and tail length 2.
  `FUN_10001000` derives that target by adding `0x32FCF` to the stored base;
  for the PE image base `0x400000`, this is VA `0x487FCF`.
  `FUN_10001530`'s assembly shows opcode `0xE8`; `FUN_10001470`
  (`ev_e785c84de979d2f63d663d2284e0d9fae60ac9a32bc21df0d621fa4552d3a375`)
  computes a 32-bit relative displacement from the supplied callback and writes
  the resulting five-byte `CALL rel32` at the target. `FUN_10001590`
  (`ev_c8dc2f87a35bb7f19718d99abd803fe70f771154cef5bb35a91a84f5b2dc6947`)
  then writes two `0x90` bytes at target `+5`. This confirms the static patch
  shape (`E8 <rel32> 90 90`), not that it was applied to a running server. The
  original target bytes (`ev_eaae9effac4c8b9d6fc39ef5e1489706b4a6f31e181bdb7e834a7f91619e806b`)
  begin with a seven-byte `CMP dword ptr [0x02735B78], 0` at `0x487FCF`,
  followed by `JBE 0x488076` at `0x487FD6`. Thus the injected call and two NOPs
  replace the comparison while leaving the original conditional branch in
  place.
- A second setup call from `FUN_10001000`
  (`ev_b2c7e8fb0e8ec3ba2aba9cb916a754fa56b58a944a141f538780a22cb154d075`)
  invokes `FUN_10001650(0x479CB1, 0x479D3C, 1)`. `FUN_10001650`
  (`ev_4b63b13e10f9309c42a9849175f81143297e92feddb35c8fe810086589f27455`)
  calls `FUN_10001620` to write an `E9 rel32` jump, then writes the requested
  NOP tail at target `+5`. The `FUN_10001620` assembly
  (`ev_1cbf3833ab6e758bf57d629911dcded79103cdcaff8559e081620854bd4b4ea8`)
  passes opcode `0xE9` and destination `0x479D3C` to the shared patch writer,
  which emits a five-byte relative jump. `FUN_10001650` then appends one NOP,
  producing `E9 <rel32> 90` over the six-byte branch beginning at
  `DBSRVIN.exe!0x479CB1`.
- The original target instructions
  (`ev_7fb43673c323c4ff5e4e97479982174325aa806f97633695167f88db2c2d036c`)
  are in `FUN_00471210`. At `0x479CB1`, the server compares the dword at
  `0x02735B78` with zero and executes `JBE 0x479D3C`; when nonzero, it falls
  through to build a `0x1F98`-byte message with opcode `0x1005`, copy a
  `0x1F80`-byte per-slot payload, and call the object at `0x02735B78`.
  The DLL's unconditional jump skips this block in every case, suppressing the
  message path when the original condition would have fallen through.
- The called `0x4557DA` thunk resolves to `FUN_004847A0`
  (`ev_c3d8ee51f124358f132eed752f7349acd8912a2904fa89d241259b4f13016870`),
  which calls `FUN_00483E90` and then `FUN_004844C0`. `FUN_00483E90`
  (`ev_3ab53cdab626d1c0c2b12ef0639eed1102a07af22a1cf12e312ff39739fa8f39`)
  derives a byte-transform key from object state and forwards the message to
  `FUN_00484090` (`ev_57008876b9f4e1ae8021322a89ebf60e620a238a8bedfad23b7432279fc67395`).
  That routine frames the message, transforms its payload, appends it to the
  object's outgoing buffer, and invokes the flush method. `FUN_004844C0`
  (`ev_190b29e76b0f31dac6d16cc8fd70819b6565d054d2750c629cf159eb2caa1f0e`)
  validates the buffer offsets and calls Winsock `send`, advancing partial
  sends or clearing the queue on completion/error. Thus the skipped `0x1005`
  block is a real outbound network-message path in static code, not just local
  packet construction; no runtime transmission was observed.
- The callback target `0x10001450` is passed from the setup at
  `0x100010D9` (`ev_6647e499daa674a895b8e591c3b27331df5854f9a5315f51c6b0e8a23ee41ddc`)
  and maps to file offset `0x850`
  (`ev_4ed5c5c136e36088b15f86a2f78effe8fb5b7a2750412750ad514d3d3dfd294a`).
  It lies between `FUN_10001410` and `FUN_10001470`; no Ghidra procedure starts
  at or contains the callback label. However, raw callback bytes
  (`ev_bcaaed583af0da86dc3275f8d5f2cf99536ad6be20350b26b45235a65624af52`)
  decode as `LEA EAX,[EBP-0x108]`, call `FUN_10001410`, clean one stack
  argument, compare `dword ptr [0x02735B78]` with zero, and return. `FUN_10001410`
  (`ev_f0020a170350d31e33d24af869401bd9277df6cbc8a2dab19408b2807b90a7d4`)
  writes its string argument to `C:/Dump/Onlines.txt`. The static flow therefore
  attributes this callback to writing the caller's `EBP-0x108` buffer and then
  restoring the comparison flags consumed by the original `JBE` at target
  `+7`; this interpretation is supported by the callback xref from setup and
  the original target bytes, but is not runtime-observed.
- The original-server analysis resolves target `0x487FCF` inside
  `FUN_00487E50` (`ev_aa5e09ec1af70e6c78a2addbfee5b5f1e2da58f7634fb7072b40c5f631aca329`,
  body `0x487E50..0x4880A4`). Its pseudocode builds a 260-byte local text
  buffer by iterating over ten slots and appending each formatted value with
  `%s %d `. This is the caller buffer passed by the DLL callback; the dump path
  `Onlines.txt` supports identifying it as an online-list snapshot, though the
  meaning of every slot/value is not established. The same function then gates
  on `DAT_02735B78`, builds a `0x1EC`-byte message with opcode `0x300E`, and
  calls the already analyzed message sender. The hook therefore dumps the
  generated ten-slot list before the original comparison and leaves the
  original send/skip branch decision intact; actual file writes and network
  sends remain unobserved.
- The additional helper analyses show `FUN_100017B0`
  (`ev_2783e5332785633ca448b842c2aea8bc74b9123aea88fb0bdf29f8fb3a212e1c`)
  performing TLS/static initialization and registering an exit callback, while
  `FUN_10001170` (`ev_7b8d9503fb472b9412c6b435004990431f87ff5122a9c378136d08c5785438ef`)
  is a return-only stub with no observed side effects.
- `DBWYD.dll!FUN_10042830`
  (`ev_10bb1cfb17c268cb0896401416971a2c3ed29eacd8e2f192e7910673025f1055`,
  body `0x10042830..0x1004283E`, 15 bytes,
  `build/rea-analysis/results/DBWYD-shutdown-callback-10042830-REA-evidence-20261021.json`)
  is the exit callback registered by `FUN_100017B0` through `_atexit`. Its
  body passes `DAT_1004DC18` to `FUN_10001870`; until that callee is analyzed,
  the teardown purpose and side effects remain unknown. Registration is static
  evidence and does not establish process-exit execution.
- `DBWYD.dll!FUN_10002BB0`
  (`ev_45308504aca773294a83ccd3de0935caeae6c76c4b59f8141c18fcd2b32cb524`,
  body `0x10002BB0..0x100030E1`) is an optimized overlap-safe copy routine:
  it selects backward copying for an overlapping destination range and forward
  copying otherwise. The DLL's patch writers `FUN_10001470` and `FUN_10001590`
  call it, as do CRT routines. Ghidra reports complex control-flow recovery
  near `0x10002DD5`, so low-level path details remain tentative.
- `DBWYD.dll!FUN_100016C0`
  (`ev_2bdb19c772b757d7998f127b76e6eec8972a3c752311ce0a9831359d07f60809`)
  formats wide-character output to the CRT stdout stream through
  `__vfwprintf_l`. Its `FUN_10001000` caller (`ev_b2c7e8fb0e8ec3ba2aba9cb916a754fa56b58a944a141f538780a22cb154d075`)
  passes the startup text `DBSRV loaded!`; this establishes a static log-output
  path only, not successful DLL attachment or server readiness.
  The callback code block is not a Ghidra function entry and remains excluded
  from the function tally. Its instruction-level behavior is now decoded, but
  process attachment, successful patch writes, redirected DB operations, and
  runtime behavior have not been observed. All findings are static analysis of
  the exact DLL and related address computations.
