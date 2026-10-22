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
index masked to five bits. Follow-up evidence for `FUN_0050BC70` shows that the
same city-indexed bit blocks a repeated challenge and that a weekly `WeekMode`
transition clears the complete byte. No direct reference was found in scoring,
ownership, or settlement logic, so this flag is supported as challenge-entry
eligibility state, not as a war result or settlement input.
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
The direct `TMSRVIN.exe!FUN_00563DA0` dossier adds one examined entry and one
represented target. Direct analysis of thunk entry `0x4B31CF` adds one examined
entry but no new represented target because it resolves to the already
represented `FUN_00567780`. The direct `FUN_005624B0` dossier, which contains a
separate timer callsite, adds one examined entry and one represented target.
The checkpoint is now 263 examined entries and 172 directly represented
targets. Direct analysis of `TMSRVIN.exe!FUN_00584030` adds one examined entry
but no represented target because it was already present behind thunk
`0x4B27F7` in the `FUN_00567780` body dossier. The checkpoint is now 264
examined entries and 172 directly represented targets. Direct analysis of
thunk entry `0x4B1CE4` adds one examined entry but no represented target because
its target, `FUN_004E0B70`, is already recorded in the `FUN_00567780` body
dossier. The checkpoint is now 265 examined entries and 172 directly
represented targets. Three further direct body analyses add one examined entry
each and no represented targets: `Core.dll!FUN_1002FA90` was already a call
target in the Tower War startup path, `TMSRVIN.exe!FUN_005076D0` was already
represented behind thunk `0x4B13B6`, and `DBWYD.dll!FUN_10001870` was already
  the exit callback's callee. Direct analysis of `DBWYD.dll!FUN_10001850`
  (`ev_29cfb0f380b17b1bc6eaef3e1e439d63cc044fd55d65f0bbe49cefc0edbe554c`,
  `build/rea-analysis/work/DBWYD-shutdown-vtable-base-10001850-20261021.rea-snapshot.json`)
  adds one examined entry but no represented target because it was already a call
  target from `FUN_10001870`. The checkpoint is now 269 examined entries and 172
  directly represented targets. Six subsequent direct analyses add six examined
  entries: `FUN_100018C0`, `FUN_1000198D`, thunk `0x100022DE`, `FUN_10020CD1`,
  `FUN_10025846`, and `FUN_100018F0`. The first five expose eight new distinct
  callee/thunk targets; the last adds none because both callees were already
  represented. The checkpoint was 275 examined entries and 180 directly
  represented targets. Two further direct analyses, `FUN_10001810`
  (`ev_a0fb93cb7e4a5b37ccea8b1d238924fc3bd52c61ecd241bdd64995e9b9b2aba1`,
  `build/rea-analysis/work/DBWYD-vftable-constructor-10001810-20261021.rea-snapshot.json`)
  and `FUN_10001830`
  (`ev_6db0868d29e5bb16ee91b78ddcbacb7e1a25b2f75818c94b654e0881d040f084`,
  `build/rea-analysis/work/DBWYD-vftable-caller-10001830-20261021.rea-snapshot.json`),
  add two examined entries; `FUN_10001830` exposes one new represented callee,
  `FUN_10001810`. The checkpoint was 277 examined entries and 181 directly
  represented targets. One further direct analysis, `DBSRVIN.exe!FUN_0048D9F0`
  (`ev_ac51e591fb7edac85ae0b0aefd2ccef4479e26a91fbca4694d48b1eadc047de3`,
  `build/rea-analysis/work/DBSRVIN-timer-id1-0048D9F0-20261021.rea-snapshot.json`),
  adds one examined entry and one directly represented target. The checkpoint
  is now 278 examined entries and 182 directly represented targets. These
  counts cover the mapped subset, not every function in the binaries.

Direct analysis of `TMSRVIN.exe!FUN_0056C860` adds one examined function entry,
bringing the examined-entry count to at least 279. Its 77 callee relationships
have not been unioned against earlier records, so 182 directly represented
targets remains the last fully reconciled checkpoint, not a current total. A
further direct analysis of `TMSRVIN.exe!FUN_0057F530` brings the examined-entry
count to at least 280; its relationships have not yet been reconciled into the
target tally. The direct reread of `FUN_00572CF0` through `0x572F88` adds detail
but no new function entry. A caller-address query at `0x55EFB6` resolves both
`0x55EFB6` and `0x55EFC7` to `FUN_0055EC40`, adding one examined entry; the
checkpoint is now at least 281. Its relationships have not been reconciled
into the directly represented-target tally. Resolving `0x4F6FAA` and
`0x512C3C` adds no distinct function body because their callers,
`FUN_004F6D60` and `FUN_00512800`, are already in this map; the examined-entry
checkpoint remains at least 281. Since that checkpoint, three additional
`Core.dll` bodies (`0x1000C8F0`, `0x10036970`, and `0x10036860`) and two
`DBWYD.dll` CRT bodies (`0x10002752` and `0x1002638D`) have been directly
examined, bringing the running examined-entry count to at least 286. The
refreshed `Core.dll!0x10012200` dossier was already represented and adds no
entry. Relationships from these new bodies have not been reconciled, so 182
directly represented targets remains the last fully reconciled checkpoint.
The direct `TMSRVIN.exe!FUN_00578700` Hell Gate analysis adds one examined
entry. Its incoming thunk and callsite are identified, but the controller's
outgoing relationships are not yet reconciled; the examined-entry count is at
least 287 while 182 remains the last fully reconciled directly represented-
target count. A direct `Core.dll!FUN_10017B90` dossier
(`ev_233b04bc161246c73d1f67a63f9b160d40631ccad7dad561721c0892b8121b34`,
`build/rea-analysis/work/Core.dll-facet-resolver-10017b90-20261022.rea-snapshot.json`)
adds one examined entry but no represented target because the function was
already exposed as a callee in `FUN_1001C270`; the count is now at least 288.
The direct `Core.dll!FUN_100316C0` analysis
(`ev_5499bdb13602d91c2e8d37a32bcdc62d032b6bb58626103c1f3dd0d1192f3f1a`)
adds one unique body, bringing the running examined-entry count to at least
  289. Direct analysis of `Core.dll!FUN_1001E490`
  (`ev_cc55d72c74381f9b9f4cc836c1d86d547b2bcdae409e0a5de5eafb1088309662`,
  `build/rea-analysis/work/Core.dll-facet-factory-1001e490-20261022.rea-snapshot.json`)
  adds one examined entry, bringing the running count to at least 290. It was
  already represented as a callee of `FUN_1001C270`; its outgoing relationships
  are not reconciled, so 182 remains the last fully reconciled directly
  represented-target count. Direct analysis of its constructor callee,
  `Core.dll!FUN_1001C7D0`
  (`ev_4b9eb1a2be4ee7555f9e3260a4b5fa5bebcee9a3c78fc47bed2bbb3e9a614147`,
  `build/rea-analysis/work/Core.dll-facet-constructor-1001c7d0-20261022.rea-snapshot.json`),
  adds one examined entry, bringing the count to at least 291. The constructor
  was already represented as a callee, so the 182-target reconciled checkpoint
  is unchanged. Direct analysis of `DBSRVIN.exe!FUN_00488E20`
  (`ev_5bbed5c29fd69750a62994fb49d549f3a81248fe4d0a33284a24bf8a9b37d918`,
  `build/rea-analysis/work/DBSRVIN-weekly-reset-alt-20261022.rea-snapshot.json`)
  adds one unique function body, bringing the running examined-entry count to
  at least 292. The body was not previously represented; this establishes a
  lower bound of at least 183 directly represented targets, while the
  intervening bodies remain unreconciled and may raise the total. Direct
  analysis of `TMSRVIN.exe!0x4B135C` adds one examined entry, bringing the
  running count to at least 293. Its dossier also directly represents the
  291-byte target body at `0x508130`; the global unique-target tally remains
  unreconciled, so 183 is still the last defensible aggregate lower bound.
  Direct analysis of `TMSRVIN.exe!0x4B308F`
  (`ev_a5fe6556e8d1579b2b0605de1b3509d4c716baf84d7a90ec0cfe045962cb883e`,
  `build/rea-analysis/work/TMSRVIN-citywar-helper-4B308F-20261022.rea-snapshot.json`)
  adds one examined entry, bringing the running count to at least 294. Its
  target body is a shared file/window log sink; the reconciled unique-target
  lower bound remains 183 because no aggregate recount was performed.
  Cash xref caller-address analyses then added three examined entries:
  `0x10011688` (`ev_61116ea10a85d248fdbc1a43119243be309a9c4cec9b9d03ececb803c2e03322`)
  and `0x100249C5` (`ev_25eebda5c556ce756c82d4e4f5215c3d3358963a2906519cf03ba05fd33e5bb2`)
  re-resolve to already represented `Core.dll!FUN_100110F0` and
  `Core.dll!FUN_100248E0`; `0x10030077`
  (`ev_a475daff438b6db3b003330256de51654e192c26677b245587e3fb32093ddb22`)
  resolves to the previously unrepresented `FUN_1002FFE0`. The running count is
  therefore at least 297 examined entries and at least 184 directly represented
  targets. This remains a lower bound, not a reconciled corpus-wide unique
  function count.
  The follow-up of the Cash open-mode path adds the direct `Fiopen` body at
  `Core.dll!0x1003D7F0` (`ev_e5cc66b60cb8bd5a08d806e446616a1001f6acfec31e68ff36f131f98e7ba252`):
  one additional examined entry and one represented target, bringing the
  running floor to at least 298 examined entries and 185 directly represented
  targets. Its 25-byte body only forwards three arguments to `_Xfiopen<>` at
  `0x1003D570`; the wrapper itself does not reveal the effective open flags.
  This is still a lower bound,
  not a reconciled corpus-wide unique function count.
  Direct analysis of `_Xfiopen<>` at `Core.dll!0x1003D570`
  (`ev_fb7948f5ec2666e42ae9d81c3a6032e0513cc11aff05a2f01c15a9547f4a56b5`)
  adds one further examined entry and represented target: at least 299 examined
  entries and 186 directly represented targets. This remains a running lower
  bound, not a reconciled corpus-wide unique function count.
  Direct analysis of `Xfsopen` at `Core.dll!0x1003D850`
  (`ev_b18268a7862faa1f3620648707c52eb32530449c5566008d552177fccb2b729c`)
  adds one further examined entry and represented target, bringing the running
  floor to at least 300 examined entries and 187 directly represented targets.
  The function is a 32-byte adapter: the selected mode index addresses a pointer
  table at `0x10155228`, whose value and the caller's share argument are passed
  to `__fsopen` at `0x100DBFE0`. This remains a lower bound, not a reconciled
  corpus-wide unique function count.
  Direct analysis of `Core.dll!__fsopen` at `0x100DBFE0`
  (`ev_80b0ce5855499eb2e850127a47aeac5f85f90c0d52982c65c52005e8b65408be`,
  saved in `build/rea-analysis/work/Core-fsopen-100DBFE0-20261022.rea-snapshot.json`)
  adds one examined entry and represented target, bringing the floor to at least
  301 examined entries and 188 directly represented targets. Its 27-byte body
  forwards the filename, mode string, and share value to
  `common_fsopen<char>` at `0x100DBAA0`. Fourteen further CRT bodies are now
  directly represented: `common_fsopen<char>` at `0x100DBAA0`
  (`ev_ba78d423c10ca01d9e31ee49f9c800e4068a5ac34c5a2c1a9033c33d69437544`),
  `open_file<char>` at `0x100DBF60`
  (`ev_dde7e981e0e8617b5db02a756f576699bd6e821502ff7b47c07bf2eddbdb881b`),
  `__openfile` at `0x10122CA0`
  (`ev_a99368540e4337dd4dc0e2977b4934054570bcc6450256021b8e0d2fb2c2921b`),
  `common_openfile<char>` at `0x101225E0`
  (`ev_c3a188795936c1c1c52fdac78ead46513cb9c89f70fbb2ed069675f88cb6e764`),
  `tsopen_s<>` at `0x10122A00`
  (`ev_908200e9e2587be202693c11a18ffb5a1c59b3ec2cc7afa952afafcd231ef351`),
  `__sopen_s` at `0x1013EEC0`
  (`ev_37ff5ccb9a757b953bc50375ff6758d17537b976142c590b29e5581a2e80fc4f`),
  `__sopen_dispatch` at `0x1013EE10`
  (`ev_7fbdd60e51f47b88597f7bad15e41df651e460e60abdc4d3df3a73cc8fd81a5a`),
  `common_sopen_dispatch<>` at `0x1013DBC0`
  (`ev_99ac5a7099501839c860ddab11b4bbaf141a953f207968a5e5bce90bf4261a33`),
  `tsopen_nolock<>` at `0x1013E100`
  (`ev_66eb740aebea7bac590539704c2ca11e97e122942e85574c7fd8f3c69c711257`),
  `__sopen_nolock` at `0x1013EE40`
  (`ev_5f6a8502f58d7ade372b49dc80748aacfe6ae5eea3543f07898443bb53151d97`),
  `__wsopen_nolock` at `0x1013EFD0`
  (`ev_b2d0560f3945972b618f56982d800e4e9be328cdbc30437288411178ef59e0f2`),
  `decode_options` at `0x1013E8F0`
  (`ev_823ece57005c70efa3cd792a90628732586fcfbf2e91ea4955062af603a4f25a`),
  `decode_sharing_flags` at `0x1013EA60`
  (`ev_7c6abb89b90c66013c8c2f3ed1fc7a5d687d183febc4e8801d96b9d0826845d6`),
  and `create_file` at `0x1013E6C0`
  (`ev_22fbae25531eb3407ec4adf4b6f624fe73c0f6bbda268195674bf24008e70036`).
  These additions raise the previous floor from 302 examined entries and 189
  directly represented targets to at least 316 and 203, respectively. The
  `std::num_put` slot-`+0x20` resolution adds the distinct
  `FUN_1001F910` body (`ev_992f67387edd2a599f0e0feb4aac107c9a7a3e61ed922b3b6284cc737a1f1111`),
  already included in the prior 302/189 floor. The exact slot bytes and supporting static output-chain
  evidence are recorded below and in
  `build/rea-analysis/results/Core-num-put-slot20-dispatch-resolution-20261022.md`.
  The subsequent Cash file-stream follow-up directly analyzed four additional
  procedure entries not previously recorded here:
  `FUN_10036B10`, `FUN_1002B700`, `FUN_1002B7A0`, and `FUN_1002C170`.
  The running lower-bound checkpoint is therefore at least 320 examined
  entries and 207 directly represented targets. This is still not a reconciled,
  corpus-wide unique-function count. Fresh saved decompilations of
  `DBSRVIN.exe!FUN_0048B920` and `Core.dll!FUN_100314B0` add detail to already
  counted bodies and do not change this tally.
  The direct `TMSRVIN.exe!FUN_004BEB70` analysis adds one examined entry but no
  new represented target because the body was already a call target in
  `FUN_005076D0`; the current lower-bound checkpoint is at least 321 examined
  entries and 207 directly represented targets.
  Direct analysis of `TMSRVIN.exe!FUN_005045E0` adds one examined entry but no
  new represented target because the body was already called by
  `FUN_005076D0`; the checkpoint is at least 322 examined entries and 207
  directly represented targets. Direct analysis of `TMSRVIN.exe!FUN_004D7850`
  adds one examined entry but no represented target because its callees were
  already represented in the transport path; the wrapper forwards the buffer
  and length to `FUN_004D6F40`, then calls `FUN_004D7570` and returns its result.
  The checkpoint is at least 323 examined entries and 207 directly represented
  targets. The focused city-war follow-up adds the previously unrepresented
  `TMSRVIN.exe!FUN_0050BC70`, bringing the checkpoint to at least 324 examined
  entries and 208 directly represented targets. This remains a running lower
  bound, not a reconciled corpus-wide unique-function count.
  The rune-index-origin follow-up adds the previously unrepresented
  `TMSRVIN.exe!FUN_005847E0`, bringing the latest checkpoint to at least 325
  examined entries and 209 directly represented targets. The bounded initial
  assignment block has no recovered function entry and is not counted; this is
  still a running lower bound, not a reconciled corpus-wide unique-function
  count. A direct REA analysis of `Core.dll!FUN_1001AC60`
  (`ev_df8018a01fdae9c92d9ad09f3c36955b0759ad24499a90eb3109f9317c8e2577`)
  adds one examined entry, raising the floor to at least 326. Its address was
  already referenced by the saved `FUN_10012200` pseudocode, so the represented-
  target floor remains 209. A direct analysis of `Core.dll!FUN_10037E40`
  (`ev_3d41f55899890bff49779416f08c0c7e0c99517e282be087c99aa87d5834cf14`)
  adds one examined entry and one represented target: the target address had
  appeared in the raw vtable bytes, but no procedure body for it was previously
  represented. The running lower bound is now at least 327 examined entries
  and 210 directly represented targets; this is not a corpus-wide count. Two
  additional direct analyses from the Cash-writer check add
  `Core.dll!FUN_10021110` and the console `WriteFile` helper, advancing the
  floor to at least 329 examined entries and 212 directly represented targets.
  Four direct file-buffer helper analyses (`FUN_10034500`, `FUN_100192E0`,
  `FUN_1001A940`, and `FUN_100348E0`) advance the examined-entry floor to at
  least 333. The represented-target floor remains at least 212 because these
  helper addresses have not been reconciled against the full set of previously
  recorded unresolved targets; this is not a corpus-wide count. A direct
  analysis of the previously unresolved `DBWYD.dll!FUN_10001E58` call target
  adds one unique body and one directly represented target. Its focused
  completion-gate follow-up directly analyzes `FUN_10001E0E` and examines the
  `__Init_thread_wait` and `__Init_thread_notify` helpers. Since those helpers
  may already be represented in earlier snapshots, this checkpoint adds only
  the newly established `FUN_10001E0E` body and target; the running lower
  bounds are at least 335 examined entries and 214 directly represented
  targets. The aggregate still is not reconciled corpus-wide.
   Two further direct analyses in the GuildMsg cleanup trace establish the
   previously unexamined `Core.dll!FUN_1002CBC0`
   (`ev_6a09cf0dd5d1e76958c5d88a1772ac4032b699c10608c4f46ad1ba69a5a4111d`,
   `build/rea-analysis/work/Core-guildmsg-cleanup-1002CBC0-20261022.rea-snapshot.json`)
   and `FUN_1002C220`
   (`ev_60c5fd9ab14b935f3525e84290aef7c44aa27c348d64d1482207e8dd6ba54834`,
   `build/rea-analysis/work/Core-guildmsg-cleanup-callee-1002C220-20261022.rea-snapshot.json`),
   raising the examined-entry lower bound to at least 337. Keep 214 as the
   last conservative directly represented-target floor until prior caller and
   call-target sets are reconciled; this is not a corpus-wide census.

   Direct instruction analysis of `Core.dll!__RTC_CheckEsp` at `0x1009BD70`
   adds one newly examined helper entry (`ev_0226f492e7044be1c79a9f5e862e149502e9cc2c70123ddbbd222721da15a9ad`,
   `build/rea-analysis/work/Core-rtc-checkesp-1009bd70-20261022.rea-snapshot.json`),
   bringing the lower bound to at least 338. Direct raw-instruction analysis
   of the already represented `Core.dll!FUN_100110F0` worker adds one examined
   entry (`ev_5e56a394f673bec2aaec1bc005e06dd6ed020a8d4f285f49ce46f40b60ce0e0e`,
   `build/rea-analysis/work/Core-cash-worker-flow-100110f0-20261022.rea-snapshot.json`),
   bringing the lower bound to at least 339. The directly represented-target
   floor remains 214 pending reconciliation. Three additional `DBSRVIN.exe`
   Sleep-callsite dossiers examined `FUN_0047E830`, `FUN_00495460`, and the CRT
   `__Once` helper at `0x004FF260`. This raises the examined-entry lower bound
   to at least 342. The first two bodies were not previously represented and
   raise the conservative directly represented-target floor to 216. The CRT
   helper is counted as examined but not added to the target floor pending
   corpus reconciliation; the `FUN_004900F0` dossier is a repeat of an already
   counted body. A direct analysis of `TMSRVIN.exe!FUN_0050CDA0` adds one new
   image-qualified body and target, bringing the floors to at least 343 examined
   entries and 217 represented targets. The new `DBSRVIN.exe!FUN_00483E90`
   dossier (`ev_c961608403ad9ed1bf2052499e934849988e1f28f46ba5bde56232138a360131`)
   repeats a body already represented by `ev_3ab53cdab626d1c0c2b12ef0639eed1102a07af22a1cf12e312ff39739fa8f39`
  for the same image hash, so it adds no unique entry or target. A new
  `DBWYD.dll!FUN_10001D4C` body (`ev_b06feae1700083cd69e7a632204bda0da5a98891ba38ddfda8b3d959d4e2f219`)
  resolves initialization of the thread-synchronization event/callback state,
  bringing the running floors to at least 344 examined entries and 218
  represented targets. A separate, newly recovered `DBWYD.dll!FUN_10001DCE`
  teardown body adds one entry and target, bringing the floors to at least 345
  examined entries and 219 represented targets. These remain lower bounds,
  not a corpus-wide count. The direct `_atexit` follow-up adds one examined
  entry but no represented target because `FUN_100017B0` already exposed its
  body as a callee; its newly analyzed `__onexit` helper adds one entry and one
  target. A separate `Core.dll` CRT time-helper analysis adds another unique
  entry and target, bringing the running floors to at least 348 examined entries
  and 221 directly represented targets. Direct analysis of `TMSRVIN.exe!FUN_00584530`
  adds one examined entry but no represented target because the controller dossier
  already exposed it behind thunk `0x4B12F3`; the running floors are now at least
  349 examined entries and 221 directly represented targets. Direct analysis of
  `GenerateMob` below adds one examined entry but no new target because the
  `Core.dll` thunk already exposed it, bringing the examined-entry floor to
  350. Of its 20 outgoing callee relationships, ten targets now have direct
  body analyses: `FUN_00503FF0`, `FUN_0056A110`, `FUN_00584110`,
  `FUN_00589AE0`, `FUN_004CCA30`, `FUN_00575830`, `FUN_004E0740`,
  `FUN_00569D50`, `FUN_004DD640`, and `FUN_004CE1C0`. The other 10 target bodies remain
  unreviewed; this relationship count includes compiler/runtime helpers and is
  not a count of gameplay functions. At this point 221 remains the conservative
  directly represented-target floor.
  Direct analysis of `FUN_005025B0` below adds one examined entry but no new
  target because it was already listed in the `FUN_00555600` callee evidence;
  the floors now stand at least at 351 examined entries and 221 directly
  represented targets. Direct analysis of `FUN_00584110` below adds one
  examined entry but no new represented target because `GenerateMob` already
  listed it as a callee. Its decoded-value helper `FUN_004B97F0` adds one more
  examined entry but no new target because it was already exposed by the
  `FUN_00584110` dossier. The running floors are therefore at least 353
  examined entries and 221 directly represented targets.
  Direct analysis of `FUN_00575830` below adds one examined entry but no new
  represented target because `GenerateMob` already listed it as a callee;
  the running floors are now at least 354 examined entries and 221 directly
  represented targets.
  Direct analysis of `FUN_00589AE0` below adds one examined entry but no new
  represented target because it was already listed as a `GenerateMob` callee;
  the floors now stand at least at 355 examined entries and 221 directly
  represented targets.
  Direct analysis of `FUN_004CCA30` below adds one examined entry but no new
  represented target because it was already listed as a `GenerateMob` callee;
  the floors now stand at least at 356 examined entries and 221 directly
  represented targets.
  Direct analysis of `FUN_0056A110` below adds one examined entry but no new
  represented target because `GenerateMob` already listed it as a callee; the
  floors now stand at least at 357 examined entries and 221 directly represented
  targets. Direct analysis of `FUN_004DD640` below likewise adds one examined
  entry but no new represented target because it was already listed as a
  `GenerateMob` callee; the floors now stand at least at 358 examined entries
  and 221 directly represented targets.
  Direct analysis of `FUN_004CE1C0` below adds one examined entry but no new
  represented target because it was already exposed by the `GenerateMob` call
  graph; the floors now stand at least at 359 examined entries and 221 directly
  represented targets. Direct analysis of `FUN_004BB4C0` below adds one
  examined entry and one newly represented callee target, bringing the running
  floors to at least 360 examined entries and 222 directly represented targets.
  Direct analysis of `FUN_004BA690` below adds one examined entry but no new
  represented target because it was already exposed as a `FUN_004CE1C0` callee.
  Its body exposes two previously unrepresented call targets, `FUN_004B7FD0`
  and `FUN_004BAB80`; the floors are now at least 361 examined entries and 224
  directly represented targets. Direct analysis of `FUN_004B7FD0` below adds
  one examined entry but no new represented target because its `FUN_004B97F0`
  callee was already represented; the floors now stand at least at 362 examined
  entries and 224 directly represented targets. Direct analysis of
  `FUN_004BAB80` below adds one examined entry and no new target because its
  only mapped callee, `FUN_004B7FD0`, was already represented; the floors now
  stand at least at 363 examined entries and 224 directly represented targets.
  Direct analysis of `FUN_004C61D0` below adds one examined entry but no new
  represented target because it was already called by `FUN_004BB4C0`; the floors
  now stand at least at 364 examined entries and 224 directly represented
  targets.

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
  members. Membership is based on each candidate's normalized leader slot
  matching the resolved root slot, not its position in a member list. The
  selector is read from the root record as
  `dword_7D8537C[586 * root_slot] - 1` and validated in `0..21`; the root-only
  mapping is `0..1 -> 1`, `2..4 -> 2`, `5..7 -> 3`, `8..10 -> 6`,
  `11..13 -> 4`, and `14..17 -> 5`. Values `18..21` produce no mapped code,
  so this body does not take the root-only item path for them. A mapped result
  creates item `5134` (`Clue_Of_Runes`) with attribute byte `0x2B`; the generic
  item helper below attempts to insert it. The root also qualifies for the
  member path: each matching candidate first computes
  `offset = stored_index + rand() % 4` and gets item `5110 + offset`. If
  `offset >= 24`, the routine discards that value and replaces it with a fresh
  standalone `rand() % 4` before adding 5110. The root can therefore receive
  both paths. This is static function and companion-data evidence, not proof
  every insertion succeeds in a live inventory. The native reward selector is
  root-record-based, not member-list-rank-based. The project's equal-chance
  selection from each room's full reward union is a deliberate fairness
  adaptation, not native parity; the native root-only `5134` path is also
  distinct. Further review of the unchanged native body is recorded in
  `build/rea-analysis/results/TMSRVIN-rune-track-reward-selection-review-20261022.md`.
- The origin of `dword_007D8537C` is now traced in
  `build/rea-analysis/results/TMSRVIN-rune-index-origin-20261022.md`. REA found
  39 direct references: 21 constant assignments covering values `1..21` in a
  registration/slot-assignment block, one non-constant write, one root-to-
  candidate copy, two clears, and 14 reads. The new procedure body
  `FUN_005847E0` scans 21 global slot cells, resolves candidate records to a
  root through the `+0x7D852E8` link, and copies the root's `+0x7D8537C` value
  to matching candidates. This supports a one-based fixed slot/root association
  propagated to candidate/member records; it does not prove the user-facing
  category is a guild, party, room, channel, or group-list position. The
  consumer guard permits raw index `22` after subtracting one, but no constant
  `22` assignment appears in these 39 references, leaving that edge case open.
  Evidence: xrefs `ev_8eacfe4fdecaa1a337c00cf1a9506e57a5dc4b17c1eaa086203c006549648e98`,
  bounded `FUN_005847E0` instructions
  `ev_3278b4ae4bfc55cbd52da99c0f57c6aca01f332441ff30dfa9e328dddbd4c290`,
  and reused consumer evidence
  `ev_055f72cf90b85f6ea0cde5a68d33511da5838d6cebdcb2c131d3b49c233a5cdd`.
  The propagation call is now statically connected to the server window timer:
  `FUN_005624B0` calls `FUN_005847E0` in its stage-1 branch, and its incoming
  call at `0x518C6E` is reached by `FUN_00518790`'s `WM_TIMER` branch for timer
  ID 0. That ID is initialized to 250 ms; the window procedure toggles
  `DAT_061AAA04` and calls the controller on every other ID-0 notification,
  implying a nominal 500 ms controller cadence when messages arrive on time.
  This resolves the trigger for root-to-candidate propagation, not the earlier
  slot-assignment block at `0x4F7E1B`: its containing procedure and business
  event remain unknown. All of this is static analysis, not timer/runtime
  observation.
- `TMSRVIN.exe!FUN_00584530`
  (`ev_e4feb1d3fbddd6971fa9012b41a36d97a20d1ce4fb8fb349b7cf0801eab88f4d`,
  body `0x584530..0x58478A`, 603 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-stage-584530-20261022-v2.rea-snapshot.json`)
  is a stage handler called by `FUN_005624B0` through thunk `0x4B12F3`.
  It passes IDs `0x161E`, `0x161F`, conditionally `0x1653..0x1655`,
  `0x1761..0x1764`, `0x1714`, and `0x16E7..0x1713` to the already mapped
  `GenerateMob` target at `0x514310`; it then selects `DAT_061AA014` as a
  pseudo-random ID in `0x16E7..0x170E` and calls `GenerateMob(0x168F,0,0)`.
  The `0x16E7..0x1713` range is reset before the random selection. Finally,
  it scans 1,000 player slots and, for state `0x16`, a nonzero field at
  `+0x7D852E4`, and selector `+0x7D8537C` in `19..21`, calls
  `FUN_005025B0(slot,30000,0x3CF,DAT_00D7E244[2*(selector-19)])`.
  The `FUN_005025B0` target was already listed through another thunk in the
  `FUN_00555600` dossier; its packet construction and sender path are now
  traced below, while the table-value meaning, client handling, and successful
  runtime effect remain unresolved. The stage dossier adds one examined
  function entry but no new represented target because `FUN_00584530` was
  already exposed behind thunk `0x4B12F3` in the `FUN_005624B0` dossier.
  Static evidence only.
- `TMSRVIN.exe!FUN_00514310` (`GenerateMob`,
  `ev_52501c885ebb3f0a3c50568d336a5f1c65f35aa0f2e3e9555c36d497e06f81cf`,
  body 8,568 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-msg-dispatch-514310-20261022.rea-snapshot.json`)
  is the target of thunk `0x4B291E` used by the Rune stage handler above and
  the `Core.dll` caller below. It validates the first argument as a
  spawn-generation index in `0..6999` and accesses definitions at `0xDC8`-byte
  strides. With zero coordinates it chooses a location from the definition's
  configured area; it then allocates an actor slot, copies template state,
  checks the selected grid, and creates or updates the actor through follow-up
  helpers. Thus the Rune-stage values passed at these call sites are
  spawn-generation entry IDs,
  not creature-species IDs or message opcodes. The matching 7.59
  `TMSRV/run/NPCGener.txt` entries identify `5863..5907` as HellTemplar,
  HellRanger, HellSorcerer, and HellShaman configurations (five maximum, group
  size four); `5908` is a single Barlog entry; `5985..5988` are single entries
  for Valdor, Bragi, Vidar, and Heimdall. The same reference maps `5662..5663`
  to LichCruntKing and `5715..5717` to ThorTower. This corroborates the meaning
  of the arguments as generator IDs, but does not establish combat stats, drops,
  experience, or the loaded effect field. Static decompilation does not prove
  that any spawn succeeds at runtime. The dossier adds one examined entry but
  no new represented target,
  since the target was already exposed by the `Core.dll` thunk evidence.
- `TMSRVIN.exe!FUN_0056A110`
  (`ev_dacea7265191056bbf655c8373314ba61f7916180bbebab9ed034e5457e1e473`,
  `build/rea-analysis/work/TMSRVIN-rune-spawn-helper-56A110-20261022.rea-snapshot.json`)
  is called by `GenerateMob` when the configured spawn count is below its
  limit. It starts from the global cursor `DAT_00647330` (reset to 1,000 when
  above `0x31FF`) and checks up to 11,800 actor-record slots, wrapping at
  `0x31FF`. A zero DWORD at `DAT_07D852E4 + slot * 0x928` is selected; the
  function stores that index as the cursor and returns it, or returns zero if
  no slot is found. `GenerateMob` rejects results below 1 or above `0x31FF` and
  logs its no-empty-mob message. This establishes a candidate actor-slot
  selection step, not successful initialization or runtime spawn behavior.
  Static evidence only.
- `TMSRVIN.exe!FUN_004DD640`
  (`ev_4b753b1e76b1da73f45614cf417f9b716befcba0adbf529d6db2c208f625d00e`,
  `build/rea-analysis/work/TMSRVIN-rune-mob-helper-4DD640-20261022.rea-snapshot.json`)
  is called by `GenerateMob` with the selected actor slot and a local buffer;
  that same buffer is then passed to `FUN_00503FF0` (`GridMulticast`). The helper
  writes observed header fields `0xE8`, `0x364`, and `30000`, includes the actor
  slot and coordinates, copies a 12-DWORD block from the actor record, and
  serializes 16 two-byte record entries with additional modifier-like bytes.
  Its body contains special handling for values including `0x312` and
  `0x938..0x955`; the gameplay meaning of these fields and header values is not
  established. This traces a mob-state serialization path into nearby-grid
  delivery, but does not identify combat-stat formulas or prove runtime delivery.
  Static evidence only.
- `TMSRVIN.exe!FUN_005025B0`
  (`ev_2a0210230e6deb1ee13677571f4061a6fe104a50836dfb366a142d3d32d730e3`,
  body `0x5025B0..0x502636`, 135 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-reward-helper-5025b0-20261022.rea-snapshot.json`)
  is called by the stage handler above through thunk `0x4B1AF5`. Given
  `(slot, 30000, 0x3CF, table_value)`, it derives the player's transport context
  at `DAT_061AAAD8 + slot * 0xFC8` and passes a 16-byte stack buffer to
  `FUN_004D6F40`. The body writes `0x3CF` at buffer offset `+4`, `30000` at
  `+6`, and the table value at `+12`; bytes `+8..+11` retain the `0xCCCCCCCC`
  stack-fill sentinel. The already-mapped sender `FUN_004D7140` constructs the
  transport header at `+0..+3`, overwrites `+4..+7`, and transforms bytes from
  `+4` onward. Thus `0x3CF` and `30000` are not evidence of a transmitted opcode
  or duration, and the table value's protocol meaning/client handling remain
  unknown. This establishes an outbound packet path, not an inventory write or
  item/reward grant. The sentinel is suspicious, but static evidence alone does
  not establish a defect.
- `TMSRVIN.exe!FUN_00584110`
  (`ev_a0240a2dbf121707c445a18e315e5337890bce20acd5be0857dff4721b582036`,
  body `0x584110..0x584205`, 246 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-mob-init-584110-20261022.rea-snapshot.json`)
  is called by `GenerateMob` after actor-template data has been copied. It reads
  a short at actor-record offset `+0xB0`; only values `0x312`, `0x790`, and
  `0x791` take the adjustment branch. That branch obtains a decoded value from
  `FUN_004B97F0`, floors it at 2, multiplies it by 10 for `0x790` or by 1,000
  for `0x791`, multiplies by the DWORD at record offset `+0`, caps at
  2,000,000,000, and writes the result at `+0x894`. These offsets and arithmetic
  are direct observations; the record fields' gameplay labels (for example,
  HP, EXP, or another stat) are not established. The enumerated Rune-stage
  wave IDs differ from the three special values, but attached-spawn coverage is
  not complete. Static evidence only.
- `TMSRVIN.exe!FUN_004B97F0`
  (`ev_ee0ff03aebd7c8567486133d5297ea741bcb335b149505123c9a640614ac57c5`,
  body `0x4B97F0..0x4B9A21`, 562 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-mob-scaling-helper-4B97F0-20261022.rea-snapshot.json`)
  is the helper used by `FUN_00584110`. It returns zero for first-word ranges
  `0x91A..0x955`, `0xC80..0xCE3`, and `0xF8C..3999`; otherwise it searches
  three trailing 16-bit slots for a `+` marker or a byte in `0x73..0x7E`,
  extracts the adjacent byte, and applies a modulo/subtract/bucketing transform
  except for first-word values `0x312`, `0x790`, and `0x791`. The code exposes
  an encoded modifier-like value, but neither its field name nor the meaning of
  its encoding is proven by this body. It has no resolved callees. Static
  evidence only.
- `TMSRVIN.exe!FUN_00575830`
  (`ev_3911464f35b4d51d0c02d09f29c0e9d65a388866716f3e16d06f70a07b3c4ab6`,
  three body ranges totaling 1,931 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-mob-secondary-575830-20261022.rea-snapshot.json`)
  is called by `GenerateMob` after the actor has been allocated and placed,
  when a spawn-generation field at `DAT_065F12A0 + spawn_index * 0xDC8` is
  nonzero.
  `GenerateMob` passes that field with `(100000, 200)`. The callee bounds the
  effect index below `0xF8`, reads its definition at a `0x60`-byte stride,
  selects one of up to 32 per-actor records beginning at `DAT_07D85148`, writes
  the configured state and duration/strength values, then refreshes the actor
  through `FUN_004CE1C0`. This is an effect/status application path, not a
  secondary mob-spawn path. The text `NPCGener` reference exposes names and
  generation limits, not this loaded field's value, so whether Rune-stage
  entries enable it remains unresolved; it does not close the separate
  attached-mob roster question. Static evidence only.
- `TMSRVIN.exe!FUN_004CE1C0`
  (`ev_0f83715d7ea5456d7a61f850387c66211f13ae02f228d55ec513b52ef70cc3bb`,
  `build/rea-analysis/work/TMSRVIN-rune-mob-refresh-helper-4CE1C0-20261022.rea-snapshot.json`)
  refreshes a record pointed to by `ECX`; callers already mapped include the
  status path `FUN_00575830` and spatial/entity path `FUN_005755E0`. It clears
  four 16-bit fields at offsets `+0x3F2..+0x3F8`. For `param_1 >= 1000` with a
  generation index in `1..6999` at record offset `+0x7FC`, it loads four
  corresponding 16-bit values from a `0xDC8`-stride table at
  `DAT_065F1252 + generation_index * 0xDC8`, caps each at 100, and stores them
  in those fields. Other branches use values returned by `FUN_004BA690`, a
  special-record fallback of 90, and record/equipment data. The body also
  recognizes specified four-item ID sets in equipment slots `+0x9C`, `+0xA4`,
  `+0xAC`, and `+0xB4`, adding 200 or 500 to the largest of four fields at
  `+0x7C..+0x82`. This confirms a shared record-refresh and derived-value path
  touched by mob effect application; the semantic labels of these offsets and
  whether Rune templates use the observed values are not established. Static
  evidence only.
- `TMSRVIN.exe!FUN_004BB4C0`
  (`ev_e375690a2a97e6ab46086e2e98b62dab37a0aad32c9cfbeb754f5f98c3c4cffb`,
  `build/rea-analysis/work/TMSRVIN-record-refresh-helper-4BB4C0-20261022.rea-snapshot.json`)
  is called by `FUN_004CE1C0` with a 32-entry block at record offset `+0x688`
  and pointers into the same record, including output words at `+0x8B0` and
  `+0x918`. It copies twelve DWORDs from `+0x2C..+0x58` to `+0x5C..+0x88`,
  then processes the entries and record/table-dependent modifiers to recompute
  multiple fields. The body clamps four 16-bit fields at `+0x3F2..+0x3F8` to
  `0..400`, four fields at `+0x84..+0x8A` to at most `0x140`, and derived
  values stored at `+0x64` and `+0x6C` to at most 1,000,000 and 100,000,
  respectively. It also has equipment-, item-, and actor-state-dependent
  branches; the semantic names of the modifier codes and affected fields remain
  unresolved. This supports a broad record-recomputation helper in the mob
  refresh path, not the gameplay meaning of each derived value. Static evidence
  only.
- `TMSRVIN.exe!FUN_004BA690`
  (`ev_793e4ce58857ab1d35087aa62b7aae13fe0e2ee2ba69b518f057e612490382b7`,
  `build/rea-analysis/work/TMSRVIN-record-value-helper-4BA690-20261022.rea-snapshot.json`)
  is called by `FUN_004CE1C0` and `FUN_004BB4C0` with a record pointer and a
  one-byte selector. For most selectors, it scans sixteen 8-byte equipment
  entries beginning at `+0x8C`, aggregates values obtained through
  `FUN_004B7FD0`, and applies additional equipment-metadata conditions. Selector
  `2` has special paired-slot handling for entries at `+0xBC` and `+0xC4`;
  selectors `3` and `0x35` have matching-metadata branches that scale the
  aggregate by 110% or 105%. Selector `0x1B` delegates to `FUN_004BAB80`, which
  returns the maximum derived value among nonempty equipment entries, then
  applies a conditional floor of 2 when the record class byte at `+0x14` is 3
  and flag `+0x30C` contains `0x80000`. The pseudocode declares this helper `void`,
  while mapped callers consume its call result; that return-type mismatch and
  the semantic names of selectors and item metadata remain unresolved. This
  narrows `FUN_004BA690` to an equipment-derived aggregation path without
  assigning gameplay labels to the values. Static evidence only.
- `TMSRVIN.exe!FUN_004B7FD0`
  (`ev_ac032a83e0dc26fea1c49ed8565b4286073d71816a30c737b1656f5c27daffa5`,
  `build/rea-analysis/work/TMSRVIN-equipment-value-reader-4B7FD0-20261022.rea-snapshot.json`)
  accepts an equipment-entry pointer and a one-byte selector. It validates item
  IDs in `1..0x1964`, reads item metadata from `0x8C`-stride tables, and builds
  a selector-specific value from fixed template fields, up to twelve template
  modifier pairs, and up to three entry option/value pairs. Several selectors
  are remapped using item-entry bytes; most general results are then adjusted
  using the refinement value from `FUN_004B97F0`, subject to selector-specific
  exclusions and special caps/scaling. Item IDs `0xF8C..0xF9F` and
  `0x91A..0x955` take dedicated table/entry-field paths, with further
  table-based formulas for `0x93A..0x955`. The returned `uint` supports its use
  as the per-entry value consumed by `FUN_004BA690`; selector meanings and the
  table-field labels remain unresolved. Static evidence only.
- `TMSRVIN.exe!FUN_004BAB80`
  (`ev_f4ce825722e87f3155baa6139be528840a62870bd6085034147277460b7c03c7`,
  `build/rea-analysis/work/TMSRVIN-equipment-selector-helper-4BAB80-20261022.rea-snapshot.json`)
  scans sixteen 8-byte equipment entries beginning at record offset `+0x8C`,
  skips entries with an empty item ID, and calls `FUN_004B7FD0` for each
  remaining entry using the same selector. It returns the maximum derived
  value, initialized to zero, rather than a sum. This identifies the reduction
  used by selector `0x1B` in `FUN_004BA690`; selector semantics remain
  unresolved. Static evidence only.
- `TMSRVIN.exe!FUN_004C61D0`
  (`ev_577d9359fa3fef83fff35ee57c248c578b8e340ef3620b4f411c5475415d12af`,
  `build/rea-analysis/work/TMSRVIN-record-gate-helper-4C61D0-20261022.rea-snapshot.json`)
  returns whether the signed integer remainder `param_1 % 10` is greater than
  5. `FUN_004BB4C0` passes its record value from offset `+0x844` as `param_1`;
  when true, that caller enters a four-index loop and tests each index through
  `FUN_004C4BA0`. The meaning of the record field and the threshold remain
  unresolved. Static evidence only.
- `TMSRVIN.exe!FUN_00589AE0`
  (`ev_c88664ac7895c2f413e37c281ec9519e06d2c9c4e33b8fa8b8a40fb90d93efd7`,
  body `0x589AE0..0x589B31`, 82 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-spawn-helper-589AE0-20261022.rea-snapshot.json`)
  is called through thunk `0x4B209F` by `GenerateMob`. For selector values in
  `0..99`, it writes the actor slot to `DAT_06145600[selector]` and writes the
  selector at `DAT_07D85364 + actor_slot * 0x928`, creating a two-way selector
  and actor-slot association. `GenerateMob` calls it for selected creature
  template values with selectors including `1..0x11`; the meaning and lifecycle
  of these selectors are not established. The function has no outgoing calls.
  Static evidence only.
- `TMSRVIN.exe!FUN_004CCA30`
  (`ev_160bed1b67b947b6876d32655cbe843177bb1f5731beac23d45413b3d5da1db3`,
  body `0x4CCA30..0x4CCBF5`, 454 bytes,
  `build/rea-analysis/work/TMSRVIN-rune-spawn-helper-4CCA30-20261022.rea-snapshot.json`)
  is called through thunk `0x4B2202` by `GenerateMob`. It clears `0x400` bytes
  from the pointer in `ECX`, then zeros additional fields and small ranges at
  offsets including `+0x788`, `+0x802`, `+0x80A`, and `+0x900`. This is a
  record reset/initialization step in the generation path; the individual field
  meanings are not established. Its other outgoing calls are `_memset` and
  compiler stack-check support. Static evidence only.
- The secondary 7.59 listing distinguishes Rune Quest Coin contribution from
  casino betting. In `TMSRVIN.c:64383-64415`, packet `0x3D1` supplies a positive
  Coin amount; the handler requires no leader link and a current target value
  of 100, caps the amount at 50,000,000, changes that target to
  `max(50, 100 - amount / 1,000,000)`, deducts the Coin, and sends the updated
  balance. The target is `dword_D7E244[2 * (stored_index - 19)]`. Direct Ghidra
  bytes at `0x5261F4` (`ev_a60a446d51fdf992319997d693624ee4aadd9fc439eb8cd5558af08b815791e7`)
  show subtraction of 19, a signed-negative branch before the upper-bound
  comparison to 21, and acceptance of offsets `0..21`; the secondary C listing
  omitted that lower-bound branch. This validates only the handler's index
  guard, not every target's gameplay use. The same listing
  (`TMSRVIN.c:84218-84256`) uses this target as a
  kill-count threshold for monster IDs 5784-5797, then generates monster 5776;
  killing 5776 follows a separate Coin payout path. This supports a quest
  progression contribution, not a wager or ordinary casino settlement. The
  event generator creates mob ID 5775 (`TMSRVIN.c:84026-84029`), and the 7.59
  reference data names generator 5775 `Freyja`
  (`TMSRV2/run/NPCGener.txt:156270-156276`, `mobname.csv:662`,
  `AllExportNpcFile.txt:252`). This ties Freyja to the Rune Quest event data,
  but does not prove that interacting with her alone opens the contribution
  prompt or establishes a live client/server flow. The control-flow detail
  above is direct REA byte evidence; monster progression and names remain based
  on secondary decompilation and reference data, not a recovered REA function
  body or runtime validation.
- `TMSRVIN.exe!FUN_005076D0`
  (`ev_80057e6b57250d4e8cf1f605a9b542ae711ed804afc99ca4b23fc7ff978acb87`,
  `build/rea-analysis/work/TMSRVIN-item-grant-5076d0-20261021.rea-snapshot.json`)
  is the item-slot insertion helper used by the Rune Track settlement. After
  its player-state guard, it scans up to 60 slots, writes the 8-byte item
  payload into the first eligible empty slot, and calls `FUN_005045E0` for the
  inventory update path. If no eligible empty slot is found, it emits message
  ID `0x56` and returns 0; the message meaning is unresolved.
- `TMSRVIN.exe!FUN_005045E0`
  (`ev_e8ff7b41584cfadd118ee082cb2c79442b3340340b707d93e3518b4c8e311f15`,
  `build/rea-analysis/work/TMSRVIN-inventory-update-5045e0-20261022.rea-snapshot.json`)
  validates a player index in `1..999` and a nonzero session field, then builds
  a 24-byte `0x182` inventory-update buffer. It copies the two supplied 32-bit
  item words into the buffer, stores the supplied item-update kind and slot,
  and passes the buffer and length to `thunk_FUN_004D7850`. The player index
  field is included only when that player's state is `0x16`; otherwise the
  field is zero. This body describes the attempted notification path, not
  runtime delivery or client application. The direct dossier for
  `FUN_004D7850` (`ev_c360da2a170bcd0cc91560d710766cfdc031ed01a82dcf5a5ead13c9bcfd94d7`,
  `build/rea-analysis/work/TMSRVIN-transport-4d7850-20261022.rea-snapshot.json`)
  shows it forwarding `(buffer, length)` to `FUN_004D6F40`, then invoking
  `FUN_004D7570` and returning that helper's result. Together with the existing
  transport dossiers below that trace the send chain to Winsock `send`, this
  connects the inventory-update path to a static socket-send attempt; it does
  not establish runtime delivery or client application.
- `TMSRVIN.exe!FUN_004BEB70`
  (`ev_d0389a81219fa7c24643fdbf059af090b45775a9c62203baab8c84be8e13c5f7`,
  `build/rea-analysis/work/TMSRVIN-inventory-slot-helper-4beb70-20261022.rea-snapshot.json`)
  defines slot eligibility: indexes 0–29 are eligible unconditionally;
  indexes 30–44 require item code `0xD8B` at record offset `+0x1E0`, and
  indexes 45–59 require the same code at `+0x1E8`. The 7.59 `itemname.csv`
  and `itemlist.csv` identify decimal item 3467 (`0xD8B`) as `Traveling_Bag`
  (`TMSRV/run/itemname.csv:2855`, `itemlist.csv:2856`). Thus the two bag
  records gate the two additional 15-slot blocks. The same data identifies
  decimal item 5134 as `Clue_Of_Runes` (`itemname.csv:3374`,
  `itemlist.csv:3385`), matching the root-only payload above. These are static
  reference-data identifications, not runtime inventory tests.
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
  `0..6999` and uses a `0xDC8` record stride. Its direct dossier
  (`ev_52501c885ebb3f0a3c50568d336a5f1c65f35aa0f2e3e9555c36d497e06f81cf`)
  confirms configured-area selection, actor-slot allocation and template copy,
  grid checking, and the actor creation/update path. This is static evidence;
  it does not establish successful live spawns.
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
  shows the cash branch gated by `*local_2c == 0x37`. Here `local_2c` is the
  local-time structure returned by `0x10012040` after `0x10012090` obtains the
  current time: the same structure is checked for hour values at `+8`, minute
  at `+4`, and second at `+0` (for example, the Colosseum notice checks second
  `10` at selected hours when minute is `0`). Thus `0x37` is `tm_sec == 55`,
  not an input opcode. The worker checks the pending-credit path during that
  second, nominally once per minute given its one-second sleep; exact scheduling
  and runtime delivery are unverified. It obtains a positive per-player value
  through `0x1002FDD0`, invokes the void helper
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
  identity follows the worker's per-slot player argument. The file producer
  and write cadence remain unresolved; the companion's later decompilation
  identifies a conditional zero-write attempt on the worker path, but not a
  successful reset or exactly-once consumption. Static file access does not
  prove runtime delivery or durable persistence.
  A cross-image check of `DBSRVIN.exe` resolves its Cash strings to branches in
  the already-counted `FUN_0048B920` (`ev_7ba4725654623c71c9b0b0a589a8b32a4ce40ed284910fba3ee21c4f8a34817a`,
  `build/rea-analysis/work/DBSRVIN-cash-dispatch-0048B920-20261022.rea-snapshot.json`).
  Under packet type `0x3C0D`, `msg_cashok` parses a `CashOK` response and routes
  a `0x50`-byte `0x4C1` message through `FUN_004847A0`; `msg_casherr` similarly
  parses `CashError` and formats its error payload. The `msg_cashreconnect` and
  `msg_cashsrverr` branches call `FUN_00471210` with opcode `0xC0B` and the
  command strings `+stopgoldcoin 0` and `+stopgoldcoin 1`, respectively. This
  establishes Cash status/control handling in the DB server, but not creation
  or updating of `WYD_DB/Cash/%s.txt`: these branches do not identify that file
  producer. It also does not rule out another producer or indirect path
  elsewhere in the supplied binaries.
- An exact ANSI-byte scan for `Cash/%s.txt` across the four hash-identified
  inputs above found the contiguous literal only in `Core.dll`, with no
  occurrence in `TMSRVIN.exe`, `DBSRVIN.exe`, or `DBWYD.dll`. REA's defined-string
  search of `DBSRVIN.exe` likewise returned no match
  (`ev_c344580e96dd3013a38bb8a764f04623084f2cddd76136c13b2d05238f0fa9c4`).
  This narrows the literal-based producer search within this corpus to `Core.dll`;
  it does not rule out a constructed or encoded path, a different filename, an
  unprovided process/binary, or indirect writes.
- The already-counted `TMSRVIN.exe!FUN_00555600` dossier
  (`ev_4824a03512114fbfeb823c7315f97b18d8c0049f4f1acc4f671e48d36885a553`,
  `build/rea-analysis/results/TMSRVIN-FUN-00555600-REA-evidence-20261021.json`)
  also contains a `0x4C1` Cash-status branch. It parses `"%s %d"` from the
  packet payload beginning at `+0xD`, resolves the player slot, and uses the
  byte at packet offset `+0xC` to select the `cash charge fail` or `cash charge
  success` log path. For slots below 1,000 whose slot state is `0x16`, it calls
  the `0x105` packet builder with code `0x195` on the fail path or `0x194` on
  the success path; the branch also chooses between the slot-table name and
  the parsed name for its log format. Other cases still format a status line
  and call the shared `FUN_00569D50` file/window logger. These log labels and
  packet-code arguments do not establish client interpretation, an actual
  billing transaction, durable credit changes, or a connection to
  `Core.dll`'s `Cash/%s.txt` reader. This is additional detail on an existing
  dossier, not a new function entry, and does not change the 320/207 running
  lower-bound checkpoint above.
- Cash companion `Core.dll!0x100314B0`
  (`ev_29744195fc98e9ce9ee4556771f2fba4d3c01bcdb8cbd3fcf9268da83c08f6d5`)
  is `void`, not a boolean predicate. Its direct procedure record
  (`ev_9d02cbdb7d32f4dfaaff86712378777c3523881fcd8d5f7c3ae618f5eb839b7a`,
  `build/rea-analysis/work/Core-cash-companion-100314B0-20261022.rea-snapshot.json`)
  shows this sequence: format the same per-player Cash path; call
  `0x10036A00(path,0x12,0x40)`; run intervening stream helpers; initialize a
  stream object; call `0x10036970(path,3,0x40)`; then test
  `0x10036810()`. If that state check is false, the numeric writer is skipped;
  if true, it calls `0x1002C710(param_1)` followed by `0x10034980()`. Both
  branches reach cleanup through `0x1002CBC0()` and `0x1002CCB0()`. In the
  worker call, `param_1` is zero. The mode analysis below establishes that the
  first open selects "w" (which truncates on successful open) and the second
  selects "r+"; together, the static flow indicates an intended conditional
  reset by writing the numeric input, zero on the worker path. The first open's
  result is not visibly checked in this body, and the second open's state check
  only gates the writer. This is evidence of an attempted reset, not proof that
  either open, output, cleanup, or durable persistence succeeds. Nor does it
  prove exactly-once credit delivery or protection from concurrent/repeated
   calls. Direct cleanup analysis links both branches to `FUN_1002CBC0` and its
   `FUN_1002C220` `basic_fstream` destructor
   (`ev_6a09cf0dd5d1e76958c5d88a1772ac4032b699c10608c4f46ad1ba69a5a4111d`,
   `ev_60c5fd9ab14b935f3525e84290aef7c44aa27c348d64d1482207e8dd6ba54834`).
   That destructor calls `FUN_1002C170`
   (`ev_623077c1fdcc81d9f82551dd6e60065072dd5dd70166bb245c8b1fb85a9674c2`,
   `build/rea-analysis/work/Core-basic-filebuf-vtable-ref-1002C1BF-20261022.rea-snapshot.json`);
   when the file-buffer flag at `+0x48` is set, it calls `FUN_100348E0`, which
   checks the `FILE*` at `+0x4C`, attempts `_fclose`, and checks that result
   (`ev_264515e7161d38af19cb2b6608f84cf2d46935a6db640b81473955c8a249ebac`,
   `build/rea-analysis/work/Core-filebuf-close-100348e0-20261022.rea-snapshot.json`).
   The destructor ignores the helper's return value. This proves a conditional
   CRT-close attempt in the static cleanup chain, not that the Cash stream takes
   the branch, the close succeeds, or bytes are durably synchronized. The
   analyzed open wrapper delegates to CRT `Fiopen` through
  `0x10036860`. The
  25-byte `Fiopen` body at `0x1003D7F0`
  (`ev_e5cc66b60cb8bd5a08d806e446616a1001f6acfec31e68ff36f131f98e7ba252`,
  saved in `build/rea-analysis/work/Core-Fiopen-1003D7F0-20261022.rea-snapshot.json`)
  forwards its three stack arguments to `_Xfiopen<>` at `0x1003D570`; Ghidra's
  library comment lists narrow- and wide-path overloads, while its recovered
  signature is low-confidence and not a source declaration. The actual file
  mode behavior is therefore inside `_Xfiopen<>` at `0x1003D570`
  (`ev_fb7948f5ec2666e42ae9d81c3a6032e0513cc11aff05a2f01c15a9547f4a56b5`,
  saved in `build/rea-analysis/work/Core-Xfiopen-1003D570-20261022.rea-snapshot.json`).
  Its 310-byte body saves input mode bits `0x4` and `0x80`, folds input bit
  `0x40` into bit `0x1` and bit `0x8` into bit `0x2`, then looks up the mode
  after masking with `0xFFFFFF3B` in a table at `0x101904F4`; a zero table entry
  returns null. For the Cash calls, `0x12` and `3` remain unchanged for lookup,
  do not set saved bit `0x4` (the later seek-to-end branch), and do not set the
  saved `0x80` pre-open-check bit. A matching table index is passed to
  `Xfsopen(path,index,share)`. The mode table at `0x101904F4` has index 2 =
  `0x12` and index 8 = `3`; Ghidra's exact initialized-memory read returns the
  expected DWORD sequence and zero sentinel at index 14
  (`ev_1530370917de24287f10155b06f834bcd2fba2917d0f099143c83fdfd7b44c31`).
  The pointer table at `0x10155228` maps index 2 to `0x10153DF0` (`"w"`) and
  index 8 to `0x1015526C` (`"r+"`), decoded from the immutable Core.dll PE bytes
  after REA resolved file offsets `0x18F6F4` for the mode table
  (`ev_cb94520a88c7ff7c634690671bd5abd171c02607828a5353099045b478c3aabb`)
  and `0x154628` for the pointer table
  (`ev_d801eb32ba2bc0af6c43944334b9e1b1f1dcdd44c67fd51c327387a5dc82665c`).
  REA's exact initialized-memory read confirms the complete 80-byte pointer
  table/string region at `0x10155228`
  (`ev_eea29108799201ba1e6409a9e8a3d170dbee696310a620e88f3151838c04933e`).
  This means Cash mode `0x12` selects CRT mode `"w"`, while mode `3` selects
  `"r+"`. Microsoft's `_fsopen` contract says `"w"` destroys existing contents
  and opens for writing, while `"r+"` requires an existing file and allows
  reading/writing; with neither `b` nor `t` suffix, text/binary translation
  depends on `_fmode`, which has not been identified here. See the
  [Microsoft `_fsopen` documentation](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fsopen-wfsopen?view=msvc-170).
  The adjacent duplicate mode sequence begins after the first table's zero
  sentinel and is outside this lookup. The 32-byte `Xfsopen` body at
  `0x1003D850`
  (`ev_b18268a7862faa1f3620648707c52eb32530449c5566008d552177fccb2b729c`,
  saved in `build/rea-analysis/work/Core-Xfsopen-1003D850-20261022.rea-snapshot.json`)
  indexes the pointer table at `0x10155228` and forwards that table value plus
  the share argument unchanged to `__fsopen` at `0x100DBFE0`. The CRT path is
  now statically followed through `common_fsopen<char>`, `open_file<char>`,
  `__openfile`, `common_openfile<char>`, `tsopen_s<>`, `_sopen_s` dispatch, and
  `__sopen_nolock`/`__wsopen_nolock` to `decode_options`,
  `decode_sharing_flags`, and `create_file`; the latter calls imported
  `CreateFileW`. The narrow wrapper converts the path to wide form and passes
  `_ShareFlag` through. The decoder maps share argument `0x40` to Win32 share
  mode `3` (`FILE_SHARE_READ | FILE_SHARE_WRITE`, without delete sharing).
  Evidence for the final five bodies is
  `ev_5f6a8502f58d7ade372b49dc80748aacfe6ae5eea3543f07898443bb53151d97`,
  `ev_b2d0560f3945972b618f56982d800e4e9be328cdbc30437288411178ef59e0f2`,
  `ev_823ece57005c70efa3cd792a90628732586fcfbf2e91ea4955062af603a4f25a`,
  `ev_7c6abb89b90c66013c8c2f3ed1fc7a5d687d183febc4e8801d96b9d0826845d6`,
  and `ev_22fbae25531eb3407ec4adf4b6f624fe73c0f6bbda268195674bf24008e70036`;
  snapshots are under `build/rea-analysis/work/` with the matching
  `Core-sopen-nolock`, `Core-wsopen-nolock`, `Core-decode-options`,
  `Core-decode-sharing-flags`, and `Core-create-file` names. This `_ShareFlag`
  value is distinct from `_OpenFlag` bit `0x40` handled by the mode decoder.
  `_fmode` and actual runtime execution remain unresolved.
  The
  `0x10018D70`/`0x10018E50` calls update and validate stream state, not the cash
  contents. `0x1002C710`
  (`ev_10f6eb4afcea486a6924a649072a905f1648419d646f55079f625a6f2b05ff79`)
  calls `0x10020890` with its incoming value as the last argument. The direct
  dossier for `0x10020890`
  (`ev_efdf9e18ef140144fad07f7327a8b1b787130ae6958b87fdbb0c5eb23d430c21`,
  saved in `build/rea-analysis/work/Core.dll-cash-helper-10020890-20261021.rea-snapshot.json`)
  is a 117-byte wrapper that forwards six arguments, including that value, to
  the virtual method at slot `+0x20`. That slot in the identified
  `std::num_put<char, std::ostreambuf_iterator<char, std::char_traits<char>>>`
  vtable (`0x101521F8`) is stored at `0x10152218` and contains pointer
  `0x1001F910` (`ev_80636a420a05fb6dcf619634e4c75c43e024a796f2a7bfb8cdd4412435d024a`),
  resolving to `FUN_1001F910` (`ev_992f67387edd2a599f0e0feb4aac107c9a7a3e61ed922b3b6284cc737a1f1111`).
  Its static body prepares formatted characters and reaches per-byte output via
  `FUN_1001EC60`, `FUN_1001F2A0`, `FUN_1001CB70`, and `FUN_1001AB90`
  (`ev_d979b650cbf175626a9ff90d0d17b605a238af48f0c05a7e3acf07a3a8ef0a93`,
  `ev_14d3b22227040d072638b11249bc5cfea5376233abf50f5a5abd63f12e0eb4ce`,
  `ev_54a10b4b93cd2946142b3a76c28d1fc0bf2b179043425ca80df760eb2a15e322`,
  `ev_9dd2b082043ca9e46254f3d68dbfdda4c30ab4ab31bc93bcecf366ca19381cc7`).
  The last routine dispatches through the destination streambuf object's vtable
  at `+0x0C`. The `basic_filebuf` RTTI/vtable analysis
  (`ev_a6c82bc4a3fc80e8b4024a487943de2c0e29e3baec9f60b568a7012ecd7e3b8`,
  `ev_01e2d6d7766f92bcf3045771854e2d8eddbe8407e5570b189eea8e0c427d889e`,
  `ev_e39b5473d4d6555cc977c40e7267e9cb472b4b3e037d6a7be763a777501158ef`,
  `ev_da34d3e9431602b320014e38c72a0705e199f34d3dd7c677edd2c364ee742d2e`,
  `ev_66308be9cd3892a1080edca578d4dac9a3ffdc56dae62bf990b477c6205d17c2`,
  `ev_456cb737502d869bc5fdcbd3569d792ddd17278a41388a5c1b629bb4121110bf`)
  identifies vtable `0x1015356C` and its `+0x0C` target `FUN_10036B10`
  (`ev_303ffc07fbc4010506e373cd11b19dd420a323cba7b012b837aba7c30686203e`).
  That 576-byte body is generic CRT `basic_filebuf` output/overflow logic and
  reaches `_fwrite`; it is not Cash-specific by itself. One vtable installation
  is in `FUN_1002B700` (`ev_f668c4047808dd8366ec10ad3c60df839fc26d236f75c628d072cb49faaacaf1`),
  called by the `basic_fstream` constructor `FUN_1002B7A0`
  (`ev_8257c06e30eba582e7b095a67926ceab9167a83c7d89231e2b04102c13e797e5`).
  That constructor's callers include the Cash reader `FUN_1002FDD0` and Cash
  companion `FUN_100314B0`, as well as the separate `GuildMsg` routes
  `FUN_1002FFE0` and `FUN_100317E0`. The second vtable reference is in
  `FUN_1002C170` (`ev_623077c1fdcc81d9f82551dd6e60065072dd5dd70166bb245c8b1fb85a9674c2`),
  a `basic_filebuf` destruction/cleanup path. This closes the static target and
  constructor gap, and supports an output attempt through the stream abstraction;
  together with the direct companion body above, it supports a conditional
  attempt to reset the Cash file, but does not prove runtime stream state,
  successful durable output, exactly-once credit consumption, or repeat-credit
  protection. `0x10034980`
  (`ev_6271484f82f30be55ea79b380308f49e42a5905accb7daae5d064d3cd82fe7cb`)
  checks another helper and, on a zero result, calls `0x1001AA90(2,0)` to
  update stream state. The companion's direct body and "w"/"r+" mode mapping
  support an intended conditional reset attempt, but do not prove its success
  or durability. They also do not prove that queued credit is consumed exactly
  once or protected from repeat credit; do not infer those guarantees from the
  helper call alone.
  The retained exact xrefs to the defined literal at `0x101535C0`
  (`ev_c0b0e71053342277bd5692b1f8a3b4f5607941a2eb576c526b42fa88e7e86cf3`)
  contain two direct uses: `0x1002FE22` in the reader and `0x100314FE` in this
  companion. Worker evidence (`ev_73436c632984e2658813185c660beaf9dea9993e11549cdbaeb0fedf00c080a0`,
  `ev_e70505cab7acc652eca7ef43c57549f3999b578abc87a04b6bb61fce984c07cb`)
  places the companion call after the positive reader result and before the
  observed accumulation branch. Combined with the direct companion decompilation,
  this establishes a conditional zero-write attempt before accumulation, not
  proof of a successful/durable clear, exactly-once consumption, or repeat-credit
  prevention. The direct literal xrefs do not rule out indirect path construction.
  Follow-up direct-body dossiers clarify the destructor boundary:
  `FUN_10034500` (`ev_8aaec168ec84f7a99854b86f548a2808c6c99bc26de202257336ce07ae2e1212`,
  `build/rea-analysis/work/Core-filebuf-finalize-10034500-20261022.rea-snapshot.json`)
  compares the exposed stream pointer with an internal sentinel before calling
  `FUN_1001A940`; `FUN_100192E0` (`ev_c067fc9a0d60f587e5d9c5790c5fa2e54f82207fb63305498c496d7fa4352255`,
  `build/rea-analysis/work/Core-filebuf-finalize-100192e0-20261022.rea-snapshot.json`)
  returns the stream pointer, and `FUN_1001A940`
  (`ev_49d1a14dc08a67497abd21dc466168f1fe65127367f318f181734e5fc910df52`,
  `build/rea-analysis/work/Core-filebuf-finalize-1001a940-20261022.rea-snapshot.json`)
  only resets the stream-buffer pointer triplet. Separately, the flagged branch
  in `FUN_1002C170` calls `FUN_100348E0`
  (`ev_264515e7161d38af19cb2b6608f84cf2d46935a6db640b81473955c8a249ebac`,
  `build/rea-analysis/work/Core-filebuf-close-100348e0-20261022.rea-snapshot.json`),
  which checks the `FILE*` at object offset `+0x4C` and attempts `_fclose`.
  This proves a conditional CRT close attempt in the destructor, not a successful
  close, durable OS synchronization, or that the specific Cash stream takes
  that branch.
  Separate write-like leads do not identify the Cash producer:
  `FUN_10021110` (`ev_cd0045ea2a2f370744d7a2fd33f4ff9a390272fd492fcaaa46dff7b262dacc5`,
  `build/rea-analysis/work/Core-file-write-candidate-10021188-20261022.rea-snapshot.json`)
  opens hard-coded `C:\bufferOutput.bin` in `"wb"`, writes four bytes, and
  closes it; no static caller was found. `write_string_to_console` writes to
  `STD_ERROR_HANDLE` through `WriteFile`
  (`ev_e57d69f75b95b8cd80e7eb8c579a48cb8cbae616f692dbef44d72d8a44eb3449`,
  `build/rea-analysis/work/Core-writefile-candidate-1012dd12-20261022.rea-snapshot.json`)
  and is reached from a CRT runtime-error route. Neither is linked to
  `WYD_DB/Cash/%s.txt`; the other unclassified `_fwrite` callsites remain
  unresolved.
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
  separate formatted-value path; the shared facet slot now resolves to
  `FUN_1001F910`, but this separate caller still does not establish a file write
  in the Cash path. `FUN_1001C270`
  (`ev_5fa1ed6ae16c09f560a22cff9c70e71b887dec654912b6e66bf4264b7842e7f3`,
  `build/rea-analysis/work/Core-stream-facet-resolver-1001c270-20261021.rea-snapshot.json`)
  selects a cached `_Facet_base` using locale-related helpers and, on a cache
  miss, creates, registers, and initializes a facet under `_Lockit`. Its callers
  are `FUN_1002C710`, `FUN_1001CC10`, and `std::locale::_Locimp::_Makeloc`.
  Although Ghidra inferred a `void` prototype, the instruction path preserves
  the selected facet pointer in `EAX` across cleanup, and both formatting
  callers use that result as the receiver passed to `FUN_10020890`. This narrows
  the receiver to a locale facet; direct constructor evidence below identifies
  its concrete subclass as `std::num_put<char,
  std::ostreambuf_iterator<char, std::char_traits<char>>>`. Its virtual slot
  `+0x20` resolves to the formatting/output body `FUN_1001F910`; the downstream
  streambuf virtual target and any Cash-file write, clear, or consumption remain
  unproven.
  `FUN_10014DE0`
  (`ev_521adfba824c66b81b4874c93528d2f8b59dfe9ff4394aedb96a66b7070bd837`,
  `build/rea-analysis/work/Core-locale-facet-key-10014de0-20261021.rea-snapshot.json`)
  lazily assigns and returns a monotonically increasing DWORD from global
  `0x1074ecd8` under `_Lockit`. `FUN_1001C270` passes the address `0x1074de78`
  to it, then passes the returned index and locale argument to
  `FUN_10017B90`, whose recovered type is `_Facet_base*`. This confirms an
  indexed locale-facet lookup, but the identity of that static ID and the
  concrete facet subclass remain unknown. The direct `FUN_10017B90` body
  (`0x10017B90..0x10017C46`, evidence `ev_233b04bc161246c73d1f67a63f9b160d40631ccad7dad561721c0892b8121b34`)
  clarifies the lookup: it indexes the locale implementation's facet table
  and returns the matching pointer. If no entry exists and the locale's
  fallback flag is set, it retries the same index against the global locale.
  This establishes the lookup/fallback mechanics, not which concrete facet
  subclass is returned for the Cash stream or how that subclass writes or
  clears the file.
  On a cache miss, `FUN_1001C270` first reuses its global cached facet at
  `DAT_1074DE80` when available. Otherwise it calls `FUN_1001E490`; the direct
  body (`ev_cc55d72c74381f9b9f4cc836c1d86d547b2bcdae409e0a5de5eafb1088309662`,
  `build/rea-analysis/work/Core.dll-facet-factory-1001e490-20261022.rea-snapshot.json`)
  shows an 8-byte allocation and delegates construction to `FUN_1001C7D0`
  using locale-related state, then stores the resulting pointer in the output
  slot. The resolver registers and initializes that facet and caches it at
  `DAT_1074DE80`. The direct body of `FUN_1001C7D0`
  (`ev_4b9eb1a2be4ee7555f9e3260a4b5fa5bebcee9a3c78fc47bed2bbb3e9a614147`,
  `build/rea-analysis/work/Core.dll-facet-constructor-1001c7d0-20261022.rea-snapshot.json`)
  sets the object's vtable to the named `std::num_put<char,
  std::ostreambuf_iterator<char, std::char_traits<char>>>` vtable at
  `0x101521F8`, then calls its constructor helper `FUN_1001EA20`. The facet
  identity and its virtual slot `+0x20` target (`FUN_1001F910`) are resolved;
  the final streambuf dispatch and resulting Cash-file effects are not.
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
  object. The same dispatcher labels this `+0x12` value as a guild ID in its
  leader-only `minhaguild` branch
  (`ev_0274727337b495a17d5de809d8efab761a62302bb8976cde9c15f1f9ce204daf`).
  The `0x10015A9D` call occurs in the leader-only `guildmsg` branch: it parses
  caller-supplied text into that 0x60-byte buffer, accepts lengths below
  `0x33`, and passes the message plus guild ID to `0x100317E0`. On the success
  path, it formats the same text and replies to the invoking player through
  `0x10030560`; this code does not establish a guild-wide broadcast. The path
  is therefore associated with a leader-supplied guild message keyed by guild
   ID, not a player Cash balance. Do not conflate it with the `%s.txt`
   queued-credit path. Fresh direct analysis of `FUN_100317E0`
   (`ev_83dc5958258b3817eb3d49766e24df10631a97441c3f1dbfbb86940e36d1871e`,
   `build/rea-analysis/work/Core-guildmsg-write-100317E0-20261022.rea-snapshot.json`)
   confirms that this route also reaches `FUN_1002CBC0` after its optional write,
   so it shares the same conditional fstream cleanup chain. This does not
   establish runtime execution or durable writes for either route.
- Direct analysis of `Core.dll!FUN_1002FFE0`
  (`ev_a475daff438b6db3b003330256de51654e192c26677b245587e3fb32093ddb22`,
  `build/rea-analysis/work/Core-cash-xrefs-callers-20261022.rea-snapshot.json`,
  body `0x1002FFE0..0x10030135`, 341 bytes) formats
  `../../WYD_DB/GuildMsg/%d.txt`, opens it through `0x10036970(path,3,0x40)`,
  and processes the stream through `0x1002A0F0` and state checks. It rejects a
  resulting string longer than `0x32` bytes before cleanup. REA observed no
  direct caller in this dossier, so its trigger and full role remain unknown.
  This is a separate GuildMsg path, not evidence for the queued Cash producer
  or consumer.
- The separate `Core.dll!FUN_100316C0` body
  (`ev_5499bdb13602d91c2e8d37a32bcdc62d032b6bb58626103c1f3dd0d1192f3f1a`,
  image SHA-256 `eb9e12b0bf79c874e8e9525d1dd90f6a5fd553d6fcec2d475f47439cfcf37d62`,
  body `0x100316C0..0x100317AD`) explains a nearby CRT `remove` xref that is
  unrelated to queued Cash consumption. It formats
  `../../WYD_DB/Dump/%s.txt`, removes that existing dump file when present,
  opens the path in `wt+`, writes two strings separated by `|`, and closes it.
  The `remove` is at `0x1003172E`; the output format is `%s|%s`. REA reports no
  direct caller for this body, so its trigger and the meaning of the two fields
  remain unknown. This classifies that deletion site but does not resolve the
  producer, writer, or consumption policy for `Cash/%s.txt`.
- Transfer helper `Core.dll!0x10012200`
  (`ev_1ce5a33d8ce178b03a686f89fe9ec818dd5c7eb6aca023a71529d1151346f391`,
  refreshed as `ev_82837b5696d7156d58ef7f2d890791c0bd81f8abcba8ae4de12c61a723aad283`)
  traverses a sized range through virtual stream-like operations and checks
  transfer state. Its second argument is passed to `0x1000C8F0`, whose body
  calls `_strlen`; in this caller the range length is therefore derived from
  the supplied C-string
  (`ev_c937ea6bccc320db45834c6373a0b7ffb06e916344ea87cad87be800b0faf569`).
  At `0x1001243C`, the helper calls `FUN_1001A370` on the adjusted stream
  object; its returned pointer is passed in `ECX` to `FUN_1001AC60` at
  `0x10012459`, together with the C-string, its length, and a zero argument.
  The direct `FUN_1001AC60` dossier shows a virtual dispatch through object
  vtable slot `+0x24` (`0x1001ACA3`), while the caller compares the returned
  low DWORD with the requested length and the high DWORD with zero. This is
  static evidence of a text-output attempt. In the `GuildMsg` constructor path,
  `FUN_1002B7A0` calls `FUN_1002B700`, which installs vtable `0x1015356C`; the
  raw table bytes resolve slot `+0x24` to `FUN_10037E40`. Its direct REA dossier
  (`ev_3d41f55899890bff49779416f08c0c7e0c99517e282be087c99aa87d5834cf14`)
  shows the zero-state branch draining buffered data and then, when bytes
  remain and the `FILE*` at object offset `+0x4c` is non-null, calling
  `_fwrite(source, 1, remaining, FILE*)`; it subtracts the number accepted by
  `_fwrite` and returns the transferred byte count. Together with the caller's
  full-length comparison, this resolves the concrete write attempt and its
  short-write check. It does not prove a successful flush/close, durable disk
  commit, or live execution.
  `0x100317E0` reaches the file-open wrapper `0x10036970`, which forwards the
  path and numeric mode/share arguments to `Fiopen` through `0x10036860`
  (`ev_9deee786f4b2990a3cbf023a09b2f34ecb9055bcea865d9cfa7ad6f148381ac0`,
  `ev_64c858ce45f047bb91f2035a8fe585d08cb8e988a4c249d979e60f24b7c03214`).
  This establishes the command-driven, guild-ID-keyed file path and the text
  supplied to its transfer helper. Whether a successful flush/close durably
  commits the bytes and whether the path executes live remain unproven.
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
  A cross-module follow-up (`build/rea-analysis/results/REA-cross-module-bridge-trace-20261022.md`)
  rechecked the startup chain and shared fame state. Its prose incorrectly
  computes `0x4B1000 + 0x68D90` as `0x51BD90`: the exact `FUN_10021420`
  pseudocode and instruction at `0x1002144D` show the offset `0x68D90`, whose
  correct result is `0x519D90`. Thus the `0x51BD90` not-found query/artifact is
  a misdirected lookup, not an unresolved hook target. The second computation,
  `0x4B1000 + 0x26219 = 0x4D7219`, is correct. Its host-code role is resolved
  by the callback and containing-routine analysis below: the hook checks the
  packet-length field in `FUN_004D7140` before its byte-transform/send path.
  The follow-up also confirms that Core and TMSRVIN touch the same
  guild-fame table at `0x04D7F1A8` and use `0x0C17` updates; this is a shared
  state/packet bridge, not proof that Core owns TMSRVIN's weekday-war schedule.
  `FUN_10039FA0` and the analyzed Core/TMSRVIN function entries were already
  represented in this map, so the follow-up adds no unique function entry.
  At that follow-up checkpoint, the running examined count was at least 287
  and the represented-target tally was reconciled through 182. Later additions
  raise the current lower-bound checkpoint to at least 324 examined entries and
  208 directly represented targets; neither count is a complete corpus census.
  Supporting evidence includes
  `ev_4d98d77f8510b516d4683ac210ad1f332d0b98be65b515195ba65cf57712ac71`,
  `ev_1dfe05f034f37cf03a8fe4dbd83df0bf3f4af80d8d98954f02326feb2a282f0d`,
  and the shared-table records `ev_8b93b93afceaff515e24879f7351343cc55fc6365ec00e443bc9b1c0628dd5ce`
  and `ev_cd5ed21a2b48f8713162982982a56367103a7702175952d63d9dc249f21c9b6b`.
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
  (`ev_677e546d869413ec03eb52e2fe02c2e50514b7e7d407ee2769c8d282f8f9e40b`,
  direct body analysis; earlier caller evidence:
  `ev_138ea158f09bd7218c1cf3a40f7387ba422365eb6b6c245bb8d5bad8b4392055`)
  is called from `0x10039FA0` before the four startup gates. It reads
  `../../WYD_DB/FileConfig/TowerWar.ini`, section `Config`, using
  `GetPrivateProfileStringA`/`GetPrivateProfileIntA`: `TowerName` defaults to
  `Torre_de_Thor`; `StartHour` and `FinishHour` default to `0`;
  `DestroyTowerFame` and `DefendTowerFame` default to `0`; and `MinutesToWin`
  defaults to `15`. It initializes the corresponding global state. This
  establishes the file path, keys, and fallback values compiled into the DLL,
  not the actual deployed INI contents or whether the server resolves that
  relative path successfully. The bundled reference sample at
  `references/decompilacao server/descompilacao tmsrv 7.59/DestinyServer-main/WYD_DB/FileConfig/TowerWar.ini`
  sets `WarActive=1`, `TowerName=Torre_de_Thor`, `StartHour=21`,
  `FinishHour=22`, `DestroyTowerFame=20`, `DefendTowerFame=150`,
  `DefenderGuildId=0`, `Channel_1=1`, `Channel_2=0`, and `MinutesToWin=15`.
  This is a bundled sample, not the deployed configuration. The analyzed
  initializer body does not read `WarActive`, `DefenderGuildId`, or the channel
  flags; whether another routine consumes those keys remains open.
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
  the `ios_base` format field. The `Fiopen` wrapper at `0x1003D7F0`
  (`ev_e5cc66b60cb8bd5a08d806e446616a1001f6acfec31e68ff36f131f98e7ba252`)
  forwards the path, mode, and share arguments to `_Xfiopen<>` at `0x1003D570`;
  the caller-to-mode conversion is not resolved by the wrapper.
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
  weekly fame reset at the Saturday boundary. Its configured schedule should
  be distinguished from the TMSRVIN weekday area event below: both reference
  the same arena bounds, but a direct cross-module call or shared runtime state
  between their schedulers has not been demonstrated.
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
  compares current fame against values at `0x4D7E258` and, in one path,
  `0x4D7E26C`. On normal acceptance it stores the challenger guild ID in
  `DAT_0063A064` and the current fame in the first bid array at
  `0x4D7E258[5 * slot + city]`, emits the `gld,ChallangeGuild` log, sends
  response `0x1E`, and invokes three update helpers. The production and exact
  meaning of the second array at `0x4D7E26C` remain unresolved. Direct analysis
  of the xref at `0x512C3C`
  (`ev_6df00e51f4792e9cb82aff07793120ebafebbbbec04e33d2bb27cbcfa363dea7`,
  `build/rea-analysis/work/TMSRVIN-guild-fame-512c3c-function-20261021.rea-snapshot.json`)
  resolves it to `FUN_00512800` (`0x512800..0x512F52`, 1,875 bytes) and confirms
  the instruction is `CALL 0x004B2879`, the thunk for `FUN_00588340`. In the
  `local_6C != 0` path it passes the applicant guild ID and `-100` before the
  later slot comparison; the procedure contains no compensating fame credit
  when that comparison rejects the bid.
  `FUN_00588340` pseudocode
  (`ev_e29688a857e436196fc6a3749dd9986dd07ea469199bf71331cdd85e7f5506dd`)
  confirms that the `-100` delta subtracts 100 from guild fame, clamped at zero.
  The helper writes the shared fame table, logs the resulting value, sends the
  24-byte `0xC17` guild-fame update, and scans ten records for the guild to
  conditionally send a 28-byte `0xC23` update when its record field is 7 or 8.
  Focused xref evidence for `0x4D7E258` and `0x4D7E26C` is recorded in
  `build/rea-analysis/results/TMSRVIN-city-war-bid-flow-20261022.md`
  (`ev_02efc621faed70baa431e6b7df623522317089fb0b0b19b7bdd461eaf24cc6c6`,
  `ev_55525d69d3410894146c55fbfb69e01ae0c0cde3593f655d676c6b47988699ff`).
  It locates reads of the second array in the challenge comparisons and the
  reset/load/write paths, but not an accepted-branch store into it; the
  producer remains an open native gap. The same follow-up confirms that the
  normal occupied-city charge is guild fame, not personal `Coin`, and that
  the `-100` debit precedes later rejection checks without a compensating
  `+100` in the recovered path.
  The alternate `case 3480` challenge path is distinct: the supplied 7.59
  listing places its block under `sub_519CA0` and shows it decrementing or
  clearing the challenge item, sending the updated inventory slot, reading
  fame for the log only, and not calling the fame helper or touching `Coin`.
  Its `gld,Cash - ChallangeGuild` label therefore does not mean a personal-gold
  debit. The block sets challenger state, clears the selected first-array
  challenge value, writes `Chall.txt`, and sends client message `140`; the
  direct call at `0x53D4B4` reaches the writer thunk, but Ghidra has no
  recovered procedure owner for that address. Keep this as listing-plus-call
  evidence, not a complete function dossier or runtime proof. The follow-up's
  challenger-state xrefs are
  `ev_6dab76770ffd7c38406c2c7d955f10e68fd108c48d87bd85696721ca0c629fce`.
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
    That 65-byte body sets `byte_61AA207 |= 1 << (village & 0x1f)`. The adjacent
    body `FUN_0050BC70`
    (`ev_d93cf87aaa33e3e79ebb4e40f558bf3f70facd2b4134153466debb95e9ac723c`)
    tests the same city-indexed bit and rejects the challenge when it is
    already set. The accepted path sets the bit; a focused reference review
    (`ev_acb1413a89df1bba8699ca0aa0728f6c0bcee472e5bc74fa944d908be8db5b95`)
    locates the setter at `0x53D404 -> 0x4B14F1 -> FUN_0050BCC0` and the guard
    at `0x512881 -> 0x4B2090 -> FUN_0050BC70`, with other direct references at
    `0x50BC9B`, `0x50BCEB`, `0x50BCF5`, and `0x5686B3`. The last site clears the
    whole byte during the weekly `WeekMode` transition
    (`ev_a33c33e3a2874f3b7635a6ee39a7eae47d599d5788f8f4b3bbeb4111e30456f0`);
    the initialized-byte read is recorded as
    `ev_1063d84173e1dfee0a515a87cb2ac1b3847d8cc6d83df2ae24f74c3ae99c2583`.
    No direct reference was found in scoring, ownership, or settlement logic.
    This establishes a challenge-eligibility guard and weekly reset, not a
    connection to the later war result. The supporting report is
    `build/rea-analysis/results/TMSRVIN-city-war-DAT61AA207-lifecycle-20261022.md`.
    Do not confuse the setter call at `0x53D404` with `0x53D4B4`, the separate
    `Chall.txt` writer call above.
  Seven newly analyzed bodies add the following city/tower-war lifecycle detail;
  all use the verified `TMSRVIN.exe` input SHA-256 in the scope table above:
  - `FUN_0050BC70` (`0x50BC70..0x50BCB6`,
    `ev_d93cf87aaa33e3e79ebb4e40f558bf3f70facd2b4134153466debb95e9ac723c`,
    documented in `build/rea-analysis/results/TMSRVIN-city-war-DAT61AA207-lifecycle-20261022.md`)
    tests the per-city bit in `byte_61AA207`; if it is already set, the
    challenge path rejects that city. This guard and the weekly whole-byte
    reset are statically observed; execution and runtime timing remain
    unverified.
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
    five-byte gap between its recovered ranges. Its score loop scans qualifying
    mode-22 player records in the war zone and adds each record's score to the
    incumbent or challenger total; when `FUN_004D1170` returns `2`, that record
    contributes an additional 400. The recovered settlement branch changes
    ownership only when the challenger total is greater, then calls
    `FUN_00588340(challenger, +50)` and `FUN_00588340(incumbent, -50)`; the
    other score branch increments a result counter and contains no fame transfer.
    These are guild-fame changes, distinct from the challenge-entry deduction.
    Direct instruction/xref follow-up confirms both settlement call sites:
    `0x5673E0` and `0x5673EE` call thunk `0x4B2879`, resolving to
    `FUN_00588340` (`ev_f100ca23ec30715d4d81ddbe3f6fecf1d418739f6c423d753c307fd8ab90e228`).
    The exact xref chain is `0x5688F4 -> 0x4B19F6 -> FUN_00566D60`
    (`ev_ca853e085a1e8e9e683160170df54d51636cca9bd22166cf7957a69335a300fe`,
    `ev_bd82a327e7dadad15dc1f33a23d2e57468cce8e48c6ff738872cff5327a0555b`).
    No direct item- or individual-reward mutation callee appears among the
    resolved settlement calls; this static boundary does not rule out indirect
    effects or work performed by other routines.
    A focused Ghidra xref query for
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
  The static caller chain for `FUN_00567780` is now resolved through the
  `TMSRVIN.exe` window timer:
  - `FUN_00517DB0` calls `SetTimer` for timer ID 1 with a 10,000 ms period at
    `0x5184ED` (and separately sets timer ID 0 to 250 ms at `0x5184CF`). The
    direct function dossier is
    `ev_dbcefa79f56d70d45d2920323bfd18634ab86a97d5a544f9a1813764f3bb3924`
    (`build/rea-analysis/work/TMSRVIN-SetTimer-init-5184CF-20261021.rea-snapshot.json`).
  - In `FUN_00518790`, the `WM_TIMER` branch (`0x113`) dispatches timer ID 1
    (`wParam == 1`) to thunk `0x4B261C` at `0x518C91`; the procedure body and
    thunk xref are recorded as
    `ev_804c037ce971e4be2857e3cd2c15df659ff91466a787087456148bf43327fad3` and
    `ev_79edd7fdc03c5177e4e25523d9a483b80eb53ef290e66b679fd9a787deecc6f2`.
  - That thunk resolves to `FUN_00563DA0`. Its direct body calls thunk
    `0x4B31CF` at `0x564736`, which resolves to `FUN_00567780`; the direct
    dossiers are `ev_31634104401869599f0065780a636dd4b7258e523acc02a69d8ad09cbfc080c8`
    and `ev_31618b2a43268873d1971e63f678312e24c294ec88bf4d90fcf1d954f7752975`.
  - The command path in `FUN_004E6B90` can reset timer ID 1 at `0x4E9DEB` to
    its supplied period, rejecting values below 5,000 ms. Its direct evidence
    is `ev_834f5405131c1211a93e0994db8803578bee602fa4a095e07d18cb3385a82531`
    (`build/rea-analysis/work/TMSRVIN-SetTimer-second-site-4E9DEB-20261021.rea-snapshot.json`).
  - The remaining in-image callsite at `0x562735`, in `FUN_005624B0`, sets timer
    ID 0 to 3,000 ms after its counter exceeds 999; it does not reconfigure the
    war timer ID 1. Direct evidence is
    `ev_b0c1fa900014a046e63bcccf17a34b33d5dfe5ddaa8f860b899029430a9ac8c6`
    (`build/rea-analysis/work/TMSRVIN-SetTimer-third-site-562735-20261021.rea-snapshot.json`).
    The `SetTimer` xref inventory also reports `0xB646730`
    (`ev_fde4c6b005e95193c9ccea8dd137f738271a8cfdf6716863e39cf3f28a551f7c`).
    REA maps it to file offset `1,672,496`
    (`ev_ade524058d1ea2fb14ccfd7c6a8cb721e503f8cbcc73955a3e54020de7f926cf`);
    the PE headers place that RVA in the file-backed `.idata` section, and a
    direct function lookup reports no procedure at the address. It is therefore
    an import/data reference, not another code callsite. The four executable
    callsites are classified; no fifth timer-setting procedure was found.
  This resolves the static schedule into the state machine and shows a 10 s
  default period with a known administrative override down to 5 s, but does
  not prove the server's live timer setting, message-loop execution, or runtime
  settlement. The separate weekday area event and its reward branch are mapped
  below. NPC/packet-entry callers, live Core.dll `TowerWar.ini` values, the
  exact relationship between the two area-event schedulers, and whether the
  accompanying item-name data exactly matches the binary's loaded resources
  remain open.
  - `TMSRVIN.exe!FUN_00567780` also contains a weekday event labeled `Base War`
    in its result log. The supplied listing excludes `tm_wday` 0 and 6 (Sunday
    and Saturday), sends payload `"5"` to active mode-22 slots during minutes
    55–57 of hour 20, then sends payload `"1"` when the minute reaches 59.
    When its state is 1 at hour 21, it spawns mob template 1079 and advances to
    state 2. In state 2, an event-slot field is incremented by 300 every five
    minutes. These are recovered conditions and values; the two text payloads
    and the incremented field's business meaning are not established. Parent
    body evidence:
    `ev_26b0f4fd519bc3be4a4c139eca8bb2a6ce5eb4e1c677d55c3efa37f8326abe7f`.
  - At hour 22 or later, the same block scans active mode-22 players whose
    `dword_7D84B34` field is positive, in the inclusive rectangle
    `x=2476..2545, y=1852..1918`.
    `FUN_00584030` (`0x584030..0x58410C`, 221 bytes;
    `ev_5a4c7253b65d264097884076d48979823d3bae90848f1cb9b3f2a2336c64060c`,
    `build/rea-analysis/results/TMSRVIN-function-00584030-REA-evidence-20261021.json`)
    confirms that the predicate checks both axes inclusively and returns 1 for
    an in-bounds position. The caller reaches it through thunk `0x4B27F7`.
    A player whose stored side matches the event owner enters the reward branch.
    The listing calls `sub_4B1CE4(player, 300)` first, then calls the item
    insertion thunk `0x4B13B6` for item 4906 and ignores its return
    (`TMSRVIN.c:72723-72724`). The thunk resolves to `FUN_005076D0`; its direct
    body analysis (`ev_80057e6b57250d4e8cf1f605a9b542ae711ed804afc99ca4b23fc7ff978acb87`,
    `build/rea-analysis/work/TMSRVIN-item-grant-5076d0-20261021.rea-snapshot.json`)
    shows a valid-player-state guard, a scan of up to 60 item slots, and writing
    the payload into the first eligible slot. If none is eligible, it emits
    message ID `0x56` and returns 0; the message text/meaning is unresolved.
    Thus the static call order allows the individual-fame increment to occur
    even if item insertion fails. Runtime inventory state and message delivery
    were not tested. The accompanying 7.59 server data maps item 4906
    to the display name translated as “Medal of Honor” in
    `DestinyServer-main/TMSRV/run/itemname.csv:3228`; the matching
    `itemlist.csv:3238` row also carries `EF_NOTRADE=1`. This identifies the
    reference item's name and listed property, but does not independently prove
    that these exact data files were loaded with the analyzed executable. REA
    resolves thunk `0x4B1CE4` to
    `FUN_004E0B70` (`ev_2b690979cc68558f4ca7dce6687394f46b3d4f555706ae4d3573ab10bf0a16bc`,
    `build/rea-analysis/work/TMSRVIN-war-followup.rea-snapshot.json`). The
    helper adds its delta to the character's fame, clamps the result to
    `0..0x63FF9C`, and stores it in the character fame fields
    (`TMSRVIN.c:26765-26804`). The same getter/setter path is labeled `set fame`
    and reports `Fame Setting %d` in the admin command handler
    (`TMSRVIN.c:29446-29448`, `:29680-29686`), establishing that this is
    individual character fame, not guild fame. The native +300 award conflicts
    with the project's chosen guild-fame-only war contract and must not be
    copied as-is. A non-matching side inside the area is repositioned through
    `DoRecall`, whose mapped path chooses a nearby free cell. If the event owner
    changed, the block also calls `sub_4B2879` with positive and computed
    negative values. In the enclosing body dossier, Ghidra classifies thunk
    `0x4B2879` as `thunk_FUN_00588340`; the direct helper dossier
    (`ev_e29688a857e436196fc6a3749dd9986dd07ea469199bf71331cdd85e7f5506dd`,
    `build/rea-analysis/results/TMSRVIN.exe-fame-00588340-retry-evidence.json`)
    logs `set guild fame`, adds the signed delta to the fame entry for guild IDs
    in `1..65535`, and floors the stored value at zero (no upper cap is visible
    in this helper). It sends a 24-byte `0xC17` update carrying the guild ID,
    type `1`, and resulting fame; for a guild found in the ten-entry war table
    on side `7` or `8`, it also sends a 28-byte `0xC23` status update. Thus the
    event's `0x4B2879` adjustments are guild fame, distinct from its separate
    +300 individual-fame award. This is static send-path evidence, not proof of
    packet delivery or durable persistence. The settlement then records the day
    result and clears the event state/mob. Listing evidence is
    `references/decompilacao server/descompilacao tmsrv 7.59/TMSRVIN/TMSRVIN.c:72694-72858`;
    the REA body dossier above provides the enclosing-function evidence.
    A focused reread of the two helper callsites in `FUN_00567780`
    (`ev_749c6022074980c352436d881695a5f4b0b145b4555946aaa6743b96d52abacb`,
    `build/rea-analysis/work/TMSRVIN-guild-fame-568ef8-function-20261021.rea-snapshot.json`)
    resolves their deltas in the branch where the current event side differs
    from the stored event owner. At `0x568EF8`, it credits the helper argument
    `local_140` by 100; that argument comes from the event slot's guild field,
    with a side-dependent global fallback when the field is zero. At `0x568F9E`,
    the delta is `-(1000 / max(DAT_00646F04, 1))`, applied to each nonzero guild
    ID in a five-entry table whose side equals `DAT_061AA518`. Because each
    negative update is independently floored at zero, the total fame removed
    need not equal the nominal aggregate. A focused follow-up on all six direct
    references to `DAT_00646F04` (`ev_ae03a9075f71f800941521acce042cc0c3ad776aed8421a4f057bfeeab06a0bd`,
    `ev_1aa8b6652415f24439572e788e283a88ffb8e88b1a6e59891179d3e32805017c`,
    and `ev_bb07d0de6595a84075f9e865ac37a805951af7d5581490d860573699ec21e12a`;
    `build/rea-analysis/results/TMSRVIN-fame-divisor-00646F04-20261022.json`)
    confirms an initialized image DWORD of 1, a minimum-value guard, the
    `1000 / value` fame deduction, and a reset to 1 after its five-entry loop.
    The already-counted `FUN_0056C860` increments the same global when a byte
    at `DAT_07D84AD0 + 0x928 * param_2` matches `DAT_0063A1FC`. This supports a
    counter-like normalization value and a probable side-matching record count;
    it does not establish the counted unit, that both functions execute in the
    same event ordering, or the relationship to the separately logged war-tax
    calculation. This refines existing dossiers and adds no unique function
    entry or represented target; runtime execution remains unverified.
  - Core.dll's `FUN_10011A60` starts its configured Tower of Thor event by
    calling `FUN_1002DBC0(0x9ac, 0x73c, 0x9f1, 0x77e)`, which converts to the
    exact same bounds (`2476,1852` through `2545,1918`). This is strong evidence
    that both code paths refer to the same arena, but their scheduler/state
    relationship is still open. The Core.dll initializer defaults
    `StartHour` and `FinishHour` to 0, so the actual start/end hours depend on
    the deployed `TowerWar.ini`; these values and runtime execution were not
    inspected. The bundled reference sample specifies start/end hours 21/22,
    but the deployed file and effective relative-path resolution remain
    unknown. TMSRVIN's weekday schedule and Core.dll's configured schedule
    therefore must not be collapsed into one fully verified event timeline.
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
  `FUN_0056C860` pseudocode
  (`ev_f06a32b5a5e4557dc57abe05a19c23ac3171de30dda75cc2d29841685c5246c1`,
  `build/rea-analysis/work/TMSRVIN-guild-fame-56f58a-function-20261021.rea-snapshot.json`)
  contains two event-state fame-credit branches guarded by
  `DAT_006477e4 == 1` and `param_2 < 1000`. They read `local_5bc` from the
  `param_1` record and use the 16-bit field at `param_2` record offset `+0x12`
  as the guild ID. Values `0x1C3..0x1D0` credit `+20`, except `0x1C5..0x1C7`
  which credit `+50`; values `0x1DB..0x1E9` credit `+20`, except `0x1DF..0x1E0`
  which credit `+50`. A separate `local_5bc == 0x437` branch contains the log
  text `Khepra Killed` and credits that guild ID by `+200`. These are static
  fame-credit paths; event meaning beyond the Khepra log, runtime reachability,
  and persistence remain unverified. No gold cost or weekly reset is shown.
  A focused xrefs query for thunk `0x4B2879` found 17 references
  (`ev_2458fc1327df8b53579bb753e4861acd61b26279c247dd2b979345d26d826ad8`,
  `build/rea-analysis/work/TMSRVIN-guild-fame-helper-xrefs-followup.rea-snapshot.json`).
  This pass maps the five sites in `FUN_0056C860`, confirms the already mapped
  settlement sites `0x5673E0` and `0x5673EE`, and resolves `0x572F88`/`0x572F96`
  to `FUN_00572CF0`. The former unresolved sites are now assigned to existing
  bodies: `0x4F6FAA` is in `FUN_004F6D60`, and `0x512C3C` is in
  `FUN_00512800`; all 17 references now have a mapped caller. The pair
  `0x55EFB6`/`0x55EFC7` resolves to
  `FUN_0055EC40`, described below. The previously pending
  sites `0x568EF8`/`0x568F9E` and `0x580609`/`0x58061F` are now mapped below.
  `FUN_0057F530` (`0x57F530..0x580A3D`, 5,390 bytes;
  `ev_971df4e0a11267eb4281a2040681c289697523f43138fa906d9ffb8b2c07754e`,
  `build/rea-analysis/work/TMSRVIN-guild-fame-580609-function-20261021.rea-snapshot.json`)
  contains the `guildraid end` log and, under `param_2 == DAT_06145628`, reads
  the guild ID from the character record at `+0x12`. It credits guild fame by
  `+600` when `(&DAT_00a3d8b0)[DAT_006472f4]` equals that guild ID and
  `DAT_061aa02c == 1`; otherwise it credits `+500`. In the `+600` branch, each
  nonzero record index in `DAT_061a0404` and `DAT_061a0408` selects twelve
  signed-short values at record offset `+0x20A`; the function calls the
  individual-fame helper `FUN_004E0B70(value, +300)` for each and also grants
  `+300` to the corresponding index. The decompiled value guard
  `(value > 0 || value < 0x3E9)` is tautological for signed integers, so it
  provides no reliable range filter. This is a guild-raid reward path with
  individual-fame awards, not evidence of city-war fame policy or runtime
  execution.
  `FUN_0055EC40` (`0x55EC40..0x55F93C`, 3,325 bytes;
  `ev_4cbe03e4ae27c686e74391b7bef0ac73a0cd748bbb947e9639ba27a4dc7c2966`,
  `build/rea-analysis/work/TMSRVIN-guild-fame-55efb6-function-20261021.rea-snapshot.json`)
  contains both remaining `0x4B2879` references at `0x55EFB6` and `0x55EFC7`.
  In its castle-charge/ownership path, when the candidate guild differs from
  the current value in `DAT_0063A1B0`, it stores the candidate guild there and
  calls `FUN_00588340(candidate, +650)` and
  `FUN_00588340(previous, -650)`. The same branch clears ten state entries and
  invokes world/board update helpers; a nearby log says `etc,charge castle`.
  This is static evidence of a paired guild-fame adjustment associated with a
  castle ownership transition, not proof of the complete war settlement,
  runtime reachability, or durable persistence.
  The `0x4F6FAA` call in `FUN_004F6D60`'s `case 0x32` is a separate guild-fame
  exchange: it removes an inventory item with ID `0xFAA` or `0xFAB`, maps those
  items to 1 or 10 units, and credits 1,000 or 10,000 guild fame through
  `FUN_00588340`. The branch logs `Siver 4 GuildFame`. This is a static fame
  acquisition path, not city-war bid settlement; runtime reachability and
  persistence are unverified (`ev_4673820710a1fafc9398adcabe2f8e7446285e51bc0896b410a864b4e5db4f0e`).
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
  In `TMSRVIN.exe!FUN_00519CA0`, the `0xE0E` request branch
  (`0x51A843..0x51ABCD`) validates guild/map/coordinate/ownership and state
  fields, then uses the fame read at `0x51AB18` as a gate: it requires at least
  500 guild fame and 10,000,000 player gold, deducts exactly 10,000,000 gold,
  synchronizes the gold through the `0x3AF` path, and sends a 20-byte response.
  No fame write, Tower-state mutation, or call to the mapped settlement or
  broadcast helpers was found in this branch. Its exact feature meaning and
  response semantics remain unresolved; it is not enough to identify a city-
  war or Tower-War fee. This is distinct from the separate `DBSRVIN.exe` `0xE0E`
  forwarding branch documented below. The project's explicit guild-fame-only
  war design must not copy this native player-gold requirement or deduction.
  A narrow REA/Ghidra follow-up analyzed `TMSRVIN.exe!0x4B135C`
  (`ev_9ab774a4dfb29e3e1aab77583745b235be6d7d8a726d48d9f68e4e3ffc879fc6`,
  `build/rea-analysis/work/TMSRVIN-citywar-callee-4B135C-20261022.rea-snapshot.json`).
  This five-byte thunk jumps to `FUN_00508130`, whose 291-byte body constructs
  a 16-byte packet with length `0x10`, opcode `0x105` at offset `+4`, and the
  third argument at offset `+0xE`. The five error callsites at
  `0x51A90D`, `0x51A9F5`, `0x51AA9D`, `0x51AAF6`, and `0x51AB5F` each push
  `(code, 0, slot)`, so the helper receives `(slot, 0, code)` and sends that
  packet through `FUN_004D6F40`. If that send helper returns zero, it invokes
  `FUN_0056A320(slot, 0x13)` for slot cleanup. With a nonzero second argument,
  the same helper instead routes through `FUN_00503FF0`; none of these five
  city-war error callsites takes that branch. This resolves the shared error-
  packet envelope, not the client-facing text or exact semantics of four
  response values. The `0x71` call is reached after the player-gold check finds
  less than `10,000,000`; it remains native evidence only and does not alter
  the project's guild-fame-only design. These failure packets are distinct
  from the branch's separately observed 20-byte response path.
  A separate exact-address query at `TMSRVIN.exe!0x4B308F`
  (`ev_a5fe6556e8d1579b2b0605de1b3509d4c716baf84d7a90ec0cfe045962cb883e`,
  `build/rea-analysis/work/TMSRVIN-citywar-helper-4B308F-20261022.rea-snapshot.json`)
  resolves this five-byte thunk to `FUN_00569D50`. The body checks two
  text arguments, formats a timestamp and identifier with them, passes the
  resulting line to `fprintf` when `DAT_061AA178` is open, and updates the
  server window through `SetWindowTextA(DAT_061600C4, ...)`. This is a shared
  diagnostic sink, not a city-war state transition. The REA read at
  `0x61A0450` (`ev_7e4382df26be0e173c4f2a3432c8a6c0d5c83fa985c17a0bb2f56ebf519f5f30`)
  returns 96 zero bytes, so that address is a mutable buffer rather than a
  recoverable static message. Existing dispatcher instructions show the
  `0x51A937` path formatting `0x6235BC`, whose string is
  `err,dowar guild index %d %d`, before logging it at `0x51A964`; the adjacent
  `0x51A9B5` path formats `0x6235A4`, `err,dowar level %d`, before logging at
  `0x51A9E2`. These are validation diagnostics, not the underlying war
  mechanic. The same generic sink receives an `Unknown Packet Type:%d ID:%d
  Size:%d KeyWord:%d` diagnostic at `0x519D5A`; REA read those bytes at
  `0x623650` (`ev_0fdcfb45362d711e2cffb0e2425c4713790fd8e622a2ba690ecb7b8b83d9c606`).
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
  The same dispatcher also handles `0xC23`: it reads two guild-ID/value pairs,
  maintains one current maximum per side (including lowering a side's value
  when the same tracked guild reports a decrease), rebuilds a 28-byte update,
  and broadcasts it to up to ten active server slots. Correlated TMSRVIN send
  paths (`FUN_00588340`, `ev_e29688a857e436196fc6a3749dd9986dd07ea469199bf71331cdd85e7f5506dd`,
  and `FUN_0050CB80`, `ev_704c15724cff64e60f7d2e8169a66e170e1924a58c548c2b80da7b4634d4577f`)
  use this 28-byte opcode for guild-ID/fame values associated with ten-record
  entries in status 7 or 8. This establishes a cross-server status aggregation
  and relay path, not guild-file persistence; the player-facing meaning of
  those two status values remains unresolved.
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

## TMSRVIN.exe: opcode 0xC19 request construction

- Direct REA/Ghidra analysis of `FUN_0050CDA0` for input SHA-256
  `27ee313bb4b4d0a4d897dac9a188d8249068ce701311cbda1e24ad28c5e43642`
  recovered body `0x50CDA0..0x50CE42`
  (`ev_2e9c70e4bb4e755569bb42927981dba3c3e54aad995e183701944d2cb63b0577`,
  `build/rea-analysis/work/TMSRVIN-0x50CDA0-c19-origin-20261022.rea-snapshot.json`).
  The body clears `0x3C0000` bytes beginning at `DAT_04D7F170`, zeroes a local
  12-byte buffer, writes length `0x0C`, opcode `0xC19` at offset `+4`, and zero
  at `+6`, then enters the existing send path. An incoming-reference query
  identifies the callsite at `0x518550` through thunk `0x4B2630`
  (`ev_b0d27d171af7735b65fe178ff8c1a6279a2c8b7dcf7e8806b223318759e63a1b`,
  `build/rea-analysis/work/TMSRVIN-0x50CDA0-c19-callers-20261022.rea-snapshot.json`).
- The observed static send chain is `FUN_004D7850` -> `FUN_004D6F40` ->
  `FUN_004D7140` -> `FUN_004D7570` -> Winsock `send`. This establishes packet
  construction and a static outbound path only; it does not establish the
  opcode's gameplay meaning or runtime transmission. This bounded TMSRVIN-side
  pass did not prove a construction/send path for `0xC1B` or `0x41E`, nor
  receipt of either response by `TMSRVIN.exe`; the separate DBSRVIN worker
  analysis below supplies their static response-side path.
- A bounded search for bytes `1B 0C` in this TMSRVIN input returned three hits
  (`ev_2e244f25d9c3320f62fd343be5d909f038fc2fcfc9f6eaf2dc4054a9c8a8683c`;
  REA created no snapshot for this search). `0x540ED6` and `0x5E110B` are bytes
  inside relative `CALL` displacements, the latter in `__get_dstbias`; `0x55D717`
  is table data. None is an opcode constructor. The search covers initialized
  provider memory and excludes uninitialized `.textbss`, so it establishes no
  TMSRVIN-side `0xC1B` constructor in the searched region, not global absence.
  It adds no function target to the running count.
- Cross-image static evidence now matches the request to a receiver-side
  operation: in `DBSRVIN.exe` SHA-256
  `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`,
  `FUN_00471210` has a `case 0xC19` call at `0x00471B0A` through thunk
  `0x00456DAB` to `FUN_0047E830`. That worker scans a fixed indexed table,
  emits `0xC1B` messages in batches of ten with 30 ms pauses between full
  batches, then emits a `0x41E` summary
  (`ev_c6090371cfada3010c88e82829cc7e1ee0f92569b77c01f96fd80c819777d5af`,
  `ev_1eefc1c2769fc45409211c77260b40e7a8d3908fb40a95908b935d02d6e2df6b`,
  `ev_5b004a739d0f302b13199e9ee0ac040676cdc4c79fb1b49733d8b2342f49c173`,
  `ev_886975fed5838b25e238ddf93a21e20e78500750c9a6044b06fb2187e874d972`).
  Together these bodies support a static `0xC19` request-to-batched-response
  path across the two images. The response fields' business meaning and any
  live transmission remain unverified; the worker showed no guild-fame,
  calendar, or reset mutation. This cross-image join adds no new function
  target to the running count.

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
- In the area-action caller `FUN_00572AE0`
  (`ev_dbf121c744fcc7d89a1ac34e3532edeec83a4679d9206f3df86d760ec3e03f34`,
  `build/rea-analysis/results/TMSRVIN-tower-area-action-572ae0-REA-evidence-20261021.json`),
  thunk `0x4B2824` resolves to `FUN_004E0740` and selects/checks coordinates
  with radius `3`; the caller stores its return value but does not branch on
  it. The next call builds a `0x36C` packet through `FUN_004DEB60`; for slots
  below 1,000 the caller sends that packet and then updates the spatial grid
  through `FUN_00503360`. The direct callee dossier for `FUN_004E0740` is
  `ev_751676f7f637c8605e086fdbf6c84e2d5442e958c9580102bbbfb1107b7ed3e0`
  (`build/rea-analysis/results/TMSRVIN-tower-callee-4e0740-REA-evidence-20261021.json`).
  This closes the coordinate/packet path for this area action, but provides no
  evidence that it mutates Tower-War scoring, ownership, fame, winner, reward,
  or termination state.
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
  On the `param_2 == 0` path, when the paired ID `local_38` is nonzero, the
  function calls `FUN_00588340(param_1, -100)` and
  `FUN_00588340(local_38, +100)` after either reciprocal or nonreciprocal
  handling. Its active-player scan compares each record's 16-bit guild ID at
  `+0x12` with those IDs. The setter floors a negative result at zero, so this
  is a paired fame adjustment, not necessarily a value-conserving transfer when
  `param_1` has less than 100 fame. The new direct reread
  (`ev_abe73735bb955f4413e843f69f4a95d200362a13b373aa6803d95fcb2f5a412b`,
  `build/rea-analysis/work/TMSRVIN-guild-fame-572f88-function-20261021.rea-snapshot.json`)
  confirms the branch; its trigger's business meaning remains unresolved. These
  are static control-flow and message-ID observations, not proof of client-visible
  packet semantics or live war behavior.
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

## TMSRVIN: Hell Gate phase controller

- `TMSRVIN.exe!FUN_00578700` was directly analyzed by REA/Ghidra 12.1.4
  (`ev_77bab0e0f83acc15e08f49c4cdd92266bb3466b27bbb672c9e9367c6c4476103`,
  input SHA-256
  `27ee313bb4b4d0a4d897dac9a188d8249068ce701311cbda1e24ad28c5e43642`).
  Its body is `0x578700..0x578CC4` (1,477 bytes, 85 basic blocks). Snapshot:
  `build/rea-analysis/work/TMSRVIN-HellGate-state-578700-20261022.rea-snapshot.json`.
- The function implements five numbered Hell Gate spawn phases. Phase 1 spawns
  template 334 and emits `hellgate_lev_1_center_boss`; phase 2 spawns templates
  335..337 and 390..395, randomly selects portal 1..3, and emits the
  corresponding `hellgate_lev_2_potal_boss` event; phase 3 spawns templates 342..377,
  initializes 36 per-slot flags, selects two values in 0..3, and emits the
  `hellgate_lev_3_hellzone_boss` event; phase 4 spawns template 333 and advances
  to phase 5; phase 5 spawns templates 378..389 and 6023..6046.
- Its minute-driven admission/cleanup path uses the server's minute value
  modulo 30. At remainder 4, with 4..7 active players and no current event, it
  scans live players in the configured Hell Gate rectangle, invokes the
  teleport helper for matching slots, and initializes event flags/phase 1 if
  at least one player matched. At remainder 29, while active and below 40
  players, it recalls in-rectangle players in batches of up to five per call;
  after the final batch it resets event flags and clears mob ranges 333..395
  and 6023..6046.
- Ghidra reports no direct callers for this body, but identifies an incoming
  unconditional-jump thunk at `0x4B18CF` (`thunk_FUN_00578700`). The existing
  `FUN_005624B0` dossier
  (`ev_b0c1fa900014a046e63bcccf17a34b33d5dfe5ddaa8f860b899029430a9ac8c6`,
  `build/rea-analysis/work/TMSRVIN-SetTimer-third-site-562735-20261021.rea-snapshot.json`)
  resolves a call from `FUN_005624B0` at `0x5639A1` to that thunk, guarded by
  `DAT_061AA18C % 4 == 0`. A focused xref follow-up resolves the incoming thunk
  `0x4B115E` to a call at `0x518C6E` in `FUN_00518790`, establishing the static
  route into `FUN_005624B0` (evidence `ev_f969c25d9dea5541fef989507e309fbe6aaf560e2e1d85e645f58d09eb375d59`,
  `ev_8ea8d8623a48b4c1030b421a0b20b56f3ceb3de7bf1b302977f86aaaa904abc4`, and
  `ev_b70f6cbf4b241cf909aa3ed5ab1f1f851d6a7ec834be0a3d3acf9cc3458ec26d`).
  In the same `FUN_00518790` body, `WM_TIMER` (`param_2 == 0x113`) with timer
  ID 0 (`param_3 == 0`) toggles `DAT_061AAA04` and calls the controller only
  when its previous value is 1 (`ev_804c037ce971e4be2857e3cd2c15df659ff91466a787087456148bf43327fad3`).
  `FUN_00517DB0` initializes ID 0 to 250 ms at `0x5184CF`
  (`ev_dbcefa79f56d70d45d2920323bfd18634ab86a97d5a544f9a1813764f3bb3924`),
  so the controller is nominally called every other timer-0 notification (about
  500 ms if delivery is on schedule); this is distinct from timer ID 1's 10 s
  default. The call route and cadence logic are static evidence only, not proof
  of timer delivery, server activation, or runtime behavior.
- A focused xrefs query for `DAT_0061AA19F`
  (`ev_c9caacdbf8713cfd94b211612e65eeb06d1f5563c11f0780e6a11f940cff5e6b`,
  `build/rea-analysis/work/TMSRVIN-HellGate-portal-selector-xrefs-20261022.rea-snapshot.json`)
  returns eight direct references. A complete masked-byte search of initialized
  blocks (`ev_94c03c1c9523dbd7f070a2731b01d7869f24746c55980cdf5118fb21c85b52fb`,
  `build/rea-analysis/results/TMSRVIN-portal-selector-search-bytes-20261022.json`)
  independently found exactly those eight occurrences; all decode as code
  references, with three direct writes at `0x578807`, `0x578B27`, and `0x578C7C`,
  and five direct reads at `0x526488`, `0x5264AC`, `0x5264D0`, `0x578814`, and
  `0x57F62E`. The full classification is in
  `build/rea-analysis/results/TMSRVIN-portal-selector-direct-references-20261022.json`.
  The scan covered 186,576,664 initialized bytes and skipped the uninitialized
  `.textbss` range `0x401000..0x4B0F84` (720,773 bytes); it cannot exclude
  computed, indirect, or dynamically constructed accesses. Inside this
  controller, the phase-2 path
  calls the random-number helper, divides by 3, adds 1, and stores the result
  as a byte at `0x578807`; it then reads the selected value at `0x578814` for
  the portal-boss event. The same global is reset to zero at `0x578B27` and
  `0x578C7C`. The already-mapped `FUN_0057F530` reads it at `0x57F62E` and
  compares it with a loop index before advancing its event-state counter.
  REA's containing-procedure resolution for three other references
  (`0x526488`, `0x5264AC`, `0x5264D0`) returned `not_in_procedure`
  (`ev_499b934e0e76b65da29acbe1355eb0e45a767cc9964cd0bb432e0adbc7c4520f`,
  `ev_151af364f5fcf836f1b2646f87b5c8f0002d51cc417f9ffdcd263022b827f633`,
  `ev_7588df6e3666d250c6d86f2f0b66f24456409c6f32de6d4e094aff2dad5373f2`);
  this is unresolved procedure ownership, not evidence that the addresses are
  data. A separate native-instruction decode at `0x526488` succeeded
  (`ev_3a306e42c584a8d449eae48ce0325ce585fe8f5b677f6887d0c7366642032901`,
  `build/rea-analysis/results/TMSRVIN-HellGate-instruction-526488-20261022.json`)
  and recovered `MOVZX EAX, byte ptr [0x061AA19F]` (bytes
  `0F B6 05 9F A1 1A 06`), confirming a read of the portal-selector global
  but not its containing routine or runtime effect. A full-output decode at
  `0x5264AC` returns the same seven-byte instruction and selector read
  (`ev_e13ff6094730ed6a85d9d543cb6cb063b3f2984ba773297c5cb7052b535980f1`,
  `build/rea-analysis/results/TMSRVIN-HellGate-instruction-5264AC-20261022.json`).
  The third site, `0x5264D0`, decodes identically
  (`ev_dd34d01632bc608bf899fb697a9df49bf8e710c3d9894be173e60e1c57154888`,
  `build/rea-analysis/results/TMSRVIN-HellGate-instruction-5264D0-20261022.json`).
  All three instructions therefore read the same selector byte, but none has
  an identified containing procedure, and these reads alone do not establish
  their role or runtime portal selection. These analyses add no unique
  function entry. A bounded FunctionManager/listing/flow follow-up over
  `[0x526360,0x5264F0]` (`ev_87bc356cced25c6ef921e3bd60b7eb42de8751bb87230dd2260ebd6efd1e3f2a`,
  `build/rea-analysis/results/TMSRVIN-portal-selector-owner-dataflow-20261022.md`)
  places the three reads in the `0xE20`, `0xE3C`, and `0xE58` selector cases;
  their branches can converge on the local reset at `0x5264E6`. Ghidra still
  assigns no containing procedure, and the bounded window has no direct call
  or jump to `FUN_00578700`, thunks `0x4B18CF`/`0x4B115E`, or `FUN_005624B0`.
  The shared global therefore does not establish ownership by the Hell Gate
  controller or a portal effect. This adds no unique function entry and is
  static evidence only. A separate call-chain follow-up traces
  `FUN_00518790 -> 0x518C6E -> thunk 0x4B115E -> FUN_005624B0 -> 0x5639A1 ->
  thunk 0x4B18CF -> FUN_00578700`; the latter call is conditional on
  `DAT_061AA18C % 4 == 0` (supporting evidence
  `ev_f969c25d9dea5541fef989507e309fbe6aaf560e2e1d85e645f58d09eb375d59`,
  `ev_8ea8d8623a48b4c1030b421a0b20b56f3ceb3de7bf1b302977f86aaaa904abc4`,
  `ev_b70f6cbf4b241cf909aa3ed5ab1f1f851d6a7ec834be0a3d3acf9cc3458ec26d`,
  and `ev_7a693e34a1b05c7890955503366e4e69707937f1915a5c02789888aef7f1e224`).
  This identifies an upstream route to the mapped Hell Gate controller, but
  does not link the three selector reads at `0x526488`, `0x5264AC`, and
  `0x5264D0` to that route. The direct phase-2 write and cleanup resets are
  identified; execution of those paths, other computed/indirect accesses, and
  actual portal traversal remain unverified.

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
   while the parser itself does not enforce line boundaries. Next, resolve who
   produces or updates the per-player files and validate runtime success,
   durability, and concurrency/exactly-once behavior for the companion's
   conditional reset attempt on the separate `Cash/%s.txt` queued-credit path.
   Static tracing now connects that companion's cleanup to the conditional
   `FUN_100348E0` `_fclose` attempt; the destructor ignores its checked return,
   and neither branch execution nor successful/durable output is established.
   The newly examined `FUN_10021110` writes an unrelated
  hard-coded `C:\bufferOutput.bin` and has no static caller; the console
  `WriteFile` helper is also unrelated. The Cash producer remains unresolved.
  Bounded xrefs completed against the matching
  Core.dll SHA/profile: `0x1002FDD0` (`ev_363c8ac60631e705ca6e53dbd37cf174698b9f04158ceb55d0b5179ec56c40ee`)
  has two direct refs, at `0x10011688` and `0x100249C5`, which resolve to the
  existing worker `FUN_100110F0` and login handler `FUN_100248E0`;
  `0x100314B0` (`ev_1e863ebf6ae4fefd220409f9699f67c8c4bacdc2aa9276b71b1e183b78451fbf`)
  likewise has two direct refs, at `0x100116AC` and `0x100249EF`, in those same
  bodies. The wrapper xrefs are `0x10036A00`
  (`ev_1b64a865353fc6db7ef66e8e2ec5fc894ace793e372f25e1f312a5593908ced0`),
  used at `0x10031547` in the Cash companion and `0x10031877` in the separate
  `FUN_100317E0` guild-message route, and `0x10036970`
  (`ev_c6ced01be586ee7fff07a3e4b2b9ddfb21555428941224c1370caf505c31ef4b`),
  used at `0x1002FE75`, `0x10030077`, `0x10031589`, and `0x100318B9` in the
  reader, `FUN_1002FFE0`, companion, and `FUN_100317E0`, respectively. Thus the
  wrapper is shared, and these xrefs do not identify a writer or prove a clear.
  The `0x10030077` body is the distinct `GuildMsg/%d.txt` routine described
  above, not a Cash producer. The earlier xrefs timeout is superseded by these
  successful queries; the complete prior failure report remains in
  `build/rea-analysis/results/Core-cash-xrefs-callers-20261022.md` for history.
  The known helper arguments `(path,0x12,0x40)` and `(path,3,0x40)` are confirmed;
  their CRT modes now resolve to `"w"` and `"r+"`, respectively. `Fiopen` at
  `0x1003D7F0` is a
  25-byte forwarding layer to `_Xfiopen<>` at `0x1003D570`
  (`ev_e5cc66b60cb8bd5a08d806e446616a1001f6acfec31e68ff36f131f98e7ba252`);
  that 310-byte callee normalizes the mode and selects an index from
  `0x101904F4`, then calls `Xfsopen` at `0x1003D850`. That 32-byte adapter
  (`ev_b18268a7862faa1f3620648707c52eb32530449c5566008d552177fccb2b729c`)
  indexes the pointer table at `0x10155228`; Cash mode `0x12` selects `"w"` and
   mode `3` selects `"r+"`. The newly decompiled companion body shows the first
   open followed by the second-state check and a numeric write on the passing
   branch; the worker supplies zero. Thus the static intent is a conditional file
   reset. Raw instructions resolve a return-type discrepancy: although the
   recovered decompilation declares `FUN_100314B0` as `void`, it loads a
   branch-local byte into `AL` (`1` on the state-check-passing numeric-write
   path at `0x100315E2`, `0` on the other path at `0x10031614`) and reaches the
   common `RET`. Exception cleanup saves and restores `EAX`; on the normal
   epilogue path, `__security_check_cookie` returns directly when the cookie
   matches and `__RTC_CheckEsp` returns directly when the stack is balanced.
   This establishes a nonzero/zero status byte reaching the caller on the
   normal path, consistent with `FUN_100110F0` testing it and incrementing the
   in-memory Cash balance only when nonzero; `FUN_100248E0` ignores the return.
   The byte indicates that the function took the numeric-write branch, not
   that the write or close succeeded or was durable. Evidence:
   `ev_d214a92983505ffeff9a6456ead533071b83b27eea149220be0f24629851575d`
   (`Core-cash-return-contract-100314B0-20261022.rea-snapshot.json`),
   `ev_0226f492e7044be1c79a9f5e862e149502e9cc2c70123ddbbd222721da15a9ad`
   (`Core-rtc-checkesp-1009bd70-20261022.rea-snapshot.json`), and
   `ev_ce138a30b63220d5cb412fe1a525d2ce0c12acca5cde6a4027ec8133b4883ba7`
   (`Core-security-cookie-helper-1009bdf0-20261022.rea-snapshot.json`). The
   worker's raw instructions confirm that the nonzero test gates an increment
   at `0x100116BB..0x100116D5` of the slot-indexed value at `0x10194A38`; the
   following block formats and sends notifications, then makes an indirect call
   through `0x101903D8`. A focused `xrefs` query finds one callsite,
   `0x1001172F` (`ev_cadc5c1df0e3665cb43fea8ecaec93e2449d59ed63bf8ae6c2c188d3ba34922a`,
   `Core-cash-indirect-call-xrefs-101903d8-20261022.rea-snapshot.json`). A
   separate initialized-memory read returns `8F304B00`, or pointer value
   `0x004B308F` (`ev_f2f5b9c35f73ae9781aca27ff231b76b3a1c62bbb582bcb0203f422230cd9349`;
   Core.dll SHA-256 `eb9e12b0bf79c874e8e9525d1dd90f6a5fd553d6fcec2d475f47439cfcf37d62`).
   In the matching TMSRVIN.exe image, `0x004B308F` is the already mapped thunk
   to `FUN_00569D50`, a shared diagnostic sink that conditionally writes to the
   open log and updates the server window (`ev_a5fe6556e8d1579b2b0605de1b3509d4c716baf84d7a90ec0cfe045962cb883e`,
   snapshot `TMSRVIN-citywar-helper-4B308F-20261022.rea-snapshot.json`). This
   resolves the static cross-image indirect edge to that diagnostic sink; it
   does not prove runtime invocation or delivery. No direct file/database
   writer is visible in this success block, so persistence of the in-memory
   balance remains unproven. The read reflects Ghidra initialized memory and
   may include loader fixups. Evidence:
   `ev_5e56a394f673bec2aaec1bc005e06dd6ed020a8d4f285f49ce46f40b60ce0e0e`
   (`Core-cash-worker-flow-100110f0-20261022.rea-snapshot.json`). The
   first-open result is not visibly checked, and the destructor's
   conditional close helper checks `_fclose` but its return is ignored by the
   destructor. This establishes neither successful output/close nor durability.
   `__fsopen` at `0x100DBFE0` is a 27-byte CRT wrapper
  forwarding these arguments to the now-traced CRT chain ending in
  `CreateFileW`; `_ShareFlag 0x40` maps to Win32 share mode `3` (read and write
   sharing, not delete sharing). The destination streambuf vtable target,
   constructor, and conditional close path are now statically traced above;
   identify who produces or updates the per-player files and determine at runtime
   whether the observed zero-write attempt succeeds and is durable. Do not treat
   it as proof of exactly-once consumption
  or repeat-credit protection.
  The `Cash/%d.txt` path is now tied to the leader-only `guildmsg` command and the
  caller object's `+0x12` field is identified as a guild ID; static flow shows a
  text-output attempt through streambuf vtable slot `+0x24`, but the concrete
  override and durable-commit semantics remain unresolved. The arguments and
  response flow at `0x10015A9D` are now
  statically traced. The `+readautotrade` trigger at `0x10016FAB` and its
  `AutoTrade reloaded!` response are also statically recovered. The recovered
  weekly-reset candidate is now bounded: `0x10011A60` has no weekly fame reset,
  and `0x10030C70` has no direct durable write. The connector's configuration
  source, port, and event are now statically resolved. `DBSRVIN`'s `0xC17`
  type-4 receiver and its writer for the full guild record are also statically
  traced; remaining fame gaps are the weekly trigger and whether the outbound
  reset packet is actually delivered and processed at runtime.
  Continue the queued-credit cash/gift worker's file-producer trace separately;
  the consumer's conditional zero-write attempt is now statically mapped, while
  its successful/durable effect and exactly-once behavior remain unproven. The fame-base
  xrefs are now classified at instruction level; do not repeat that sweep unless
  new evidence appears. Continue into the post-purchase helper chain only if
  proving finite-stock mutation is required. The `LastServerBoss` file
  path is resolved above and should not be re-opened without new evidence. The
  transport path from `0x10030660` to
  Winsock `send` and startup's endpoint selection are statically traced:
  `localip.txt` is matched against the server table, selecting the DB
  destination and port 7514. The deployment-specific address contents and
  successful runtime connection/delivery remain unverified. After those
  bounded paths, map remaining relevant functions in `Core.dll` and the
  executable.
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
  `FUN_00489D50` window-message/socket handler. Its direct decompilation
  (`ev_4f070fd13662efff86448e27dacc3e091bf6e9a9644e6b4dea06a8f6c88dc7af`,
  `build/rea-analysis/results/DBSRVIN-patch-owner-489D50-REA-decompile-20261021.json`)
  identifies message `0x113` (`WM_TIMER`): timer ID 0 dispatches
  `FUN_0048D8E0`, while timer ID 1 dispatches `FUN_0048D9F0`. The executable
  direct xrefs query for `FUN_0048D8E0`
  (`ev_1950d3b4c779cd94fa00cbbada8596b064c3700a762a32563aed610580e35597`,
  `build/rea-analysis/work/DBSRVIN-weekly-reset-callback-xrefs-20261021.rea-snapshot.json`)
  returns only `0x45514A`; this is the known thunk path into the timer callback,
  not evidence of a weekly registration site. The executable
  imports `SetTimer` at `0x85E36D0`, but Ghidra reports no xrefs to that IAT
  address (`ev_a91f696d3fb4b41d52bfaf7e251b23a13d72f9acbfd3f48084a6f150a53e5589`),
  and a complete search of initialized mapped memory finds no occurrence of
  its little-endian address (`ev_832597187a774c74445b09f5817f03640f583c909e877a298982e850832195d9`).
  That scan skipped uninitialized `.textbss`, so the timer-creation site and
  wall-clock period remain unresolved. The callback's 600-cycle branch is
  online-list maintenance, not a weekly guild-fame reset. This rules out this
  callback as the reset mechanism; the weekly reset trigger elsewhere remains
  unresolved.
- Direct analysis of timer-ID-1 target `DBSRVIN.exe!FUN_0048D9F0`
  (`ev_ac51e591fb7edac85ae0b0aefd2ccef4479e26a91fbca4694d48b1eadc047de3`,
  `build/rea-analysis/work/DBSRVIN-timer-id1-0048D9F0-20261021.rea-snapshot.json`)
  recovers a 37-byte body (`0x48D9F0..0x48DA14`) that fills its 0xC0-byte local
  stack frame with `0xCC` and returns. It has no calls or global-state reads in
  the recovered body; this timer-ID-1 target supplies no weekly fame-reset
  behavior in the analyzed build. Together, the two timer-dispatched targets
  do not account for the weekly reset trigger, which remains unresolved.

- A focused 2026-10-22 follow-up adds no unique function entry. It confirms
  `FUN_0045514A` is a five-byte thunk that jumps to `FUN_0048D8E0`
  (`ev_433ac6572663e06f8571707fefab237d2a56cc2d229116ab9481163e73b8e719`),
  and an exact named timer/calendar procedure search returned no candidates
  (`ev_7779e952b401742c2aa3b3e5f9bc52a904ec0be46e9cab6d325948a80f6d600d`).
  Direct instruction-pattern searches found no `CALL`/`JMP` to the known
  `SetTimer` IAT and no direct `GetSystemTimeAsFileTime` call across
  140,197,112 initialized bytes (`ev_99b634b1d33f07ca14344149100720a13d5c930a61dcc503edfcd9d2857c5fa7`,
  `ev_65ac8af2c61e98e9029638867a885b7fb15d0002e48aa015a2778300a4d5111f`,
  `ev_21dd28183fe60a0a3c317487f6ee361216260e6da68fb2be8ad0475793bc65c5`).
  The scan skipped uninitialized `.textbss`; these bounded negative results
  do not exclude indirect scheduling or a trigger in another module. Full
  evidence and remaining gaps are in
  `build/rea-analysis/results/DBSRVIN-weekly-reset-followup-20261022.md`.

## Cross-image bounded check for automatic weekly guild-fame reset

- An additional bounded search of `DBWYD.dll` located no distinct automatic
  weekly guild-fame reset. The Saturday/Sunday references belong to CRT
  calendar data (`ev_dbbd1a1f4319d0696a590397f980db17614ad34c2db6da448c513eada7c6ad7`);
  in particular, the Saturday data reference at `0x100449AC`
  has no incoming xrefs (`ev_a125ed2f847fb95f06f4cc71852f655f10cedd39199a7c23eaf48c22ab8e89d7`,
  `ev_ac6c3e06258f787604a07bd68d41a7e4374f224ae77a387fe248f8db3af5679`).
- The two code xrefs to `GetSystemTimeAsFileTime` resolve to CRT support, not
  guild logic: `___get_entropy` at `0x10002752..0x1000279E` is reached from
  security-cookie initialization (`ev_b57604b4a0473abec81bc2c26a374e83cd61b9ab7f9fc75883ae0300daeeffff`,
  `ev_6c5559069514badd2063d1354c1168a349c0150b5cade9f6c287fbd60c32fa61`),
  and `___acrt_GetSystemTimePreciseAsFileTime@4` at
  `0x1002638D..0x100263CB` is a CRT time wrapper
  (`ev_afdd6c649d9794e3d713140bc1150bfbd9ab854604250f81ac7ce2e22ee7c149`).
  Neither recovered body connects to a guild-fame mutation or weekly scheduler.
- The newly analyzed `Core.dll!___crtGetSystemTimePreciseAsFileTime`
  (`0x1003FD10..0x1003FD58`, 73 bytes,
  `ev_2a0ac230466d4e9a26c4ed7becd42265cd22d4efb16e4dd50b73361a47e0cb79`,
  `build/rea-analysis/work/Core-function-time-call-1003fd4f-20261021.json`)
  falls back to `GetSystemTimeAsFileTime` when `DAT_1074EDF0` is null; otherwise
  it invokes that dynamically supplied pointer after `guard_check_icall`.
  Ghidra reports no callers, and the indirect target remains unresolved. This
  adds one unique body to the running lower bounds (346 examined entries, 220
  directly represented targets), but does not identify a weekly scheduler or
  connect this CRT helper to guild-fame state.
- This narrows the examined in-image candidates but does not exclude an
  unexamined or indirect trigger, another process, or runtime configuration.
  The only statically proven fame-reset route remains the administrative
  `resetguildfame` command through `DBSRVIN` opcode `0xC17`, type 4, and its
  guild-record clear/write path. Automatic weekly reset remains unlocated;
  delivery and runtime processing of the administrative packet are also
  unverified.
- A separate bounded REA pass is recorded in
  `build/rea-analysis/weekly-guild-fame-reset-alt-20261022.md`. Its import-table
  read identifies the DBSRVIN `SetTimer` IAT at `0x089E36D0`
  (`ev_fca01da073d0c79087c2c8aeb88ad5cea58dfbaa6a3bf4c7efa285e6a8fb013e`),
  and direct xrefs at `0x004896A1` and `0x004896BF` identify the code that uses
  it (`ev_6c71483f611a09c570c718bed00a2540eb427f3cfe2538babf70080fa9a04cf0`).
  `FUN_00488E20` sets 1-second and 10-second window timers with null callbacks,
  then runs a `GetMessageA`/`TranslateMessage`/`DispatchMessageA` loop. Its
  recovered body contains no guild-fame mutation, weekly-boundary check, or
  call to the mapped `0xC17` path, so this timer route is not evidence of an
  automatic fame reset.
- The same report's initial `.textbss` limitation was followed up directly.
  In `DBSRVIN.exe`, `.textbss` spans `0x00401000..0x00454C01` (343,042 bytes),
  is loaded but uninitialized, and has no file-backed bytes. A read at
  `0x00401000` returned zero bytes; xrefs to that address were only the header
  locations `0x0040011C`, `0x00400120`, and `0x004001F4`. Procedure searches
  over `0x00401000..0x0044FFFF` and `0x00450000..0x00454C01` recovered no
  procedures, and a raw file scan found no literal 32-bit pointer targeting the
  range. These are bounded static negative results, not proof that the region
  is unused at runtime: dynamically initialized state, computed references,
  and code or registration from another module remain possible. The reset
  trigger remains `UNLOCATED`, not disproven. Evidence:
  `ev_8a4669c68632db2d9065c8fb6abc971aac7a06fc8c9816ad963ba8e00b291ac8`,
  `ev_b44dbebe97137576b3e93f4562e4a1efa2e74d5c82a4fe273af675884ca0bf41`,
  `ev_720580318ffbd90b5877d9bd4d6ad27d7cd35cad28b5d3704dcae85909340204`,
  `ev_ad7f69f77afd03451a605873bfd3bc82086ea370c34515db9d78541a6346fb23`,
  and `ev_923759aea5aa74e0cc0194bf9781d0f1ddd5edafd72e5a961b6c674c3196742a`.
  Snapshots are
  `build/rea-analysis/work/DBSRVIN-textbss-xrefs-00401000-20261022.rea-snapshot.json`,
  `build/rea-analysis/work/DBSRVIN-textbss-procedure-search-20261022.rea-snapshot.json`,
  and
  `build/rea-analysis/work/DBSRVIN-textbss-procedure-search-450000-454c01-20261022.rea-snapshot.json`.
  The TMSRVIN snapshot in the report only reconfirms the already mapped
  administrative dispatcher (`ev_98476427d7801960a9c1f22d36daaaf159419cce91f633b03444a17005bc8ac6`)
  and adds no automatic-reset edge.

- A separate import-to-call-site audit used the `DBSRVIN.exe` input hash above
  and traced PE IAT entries to x86 call sites, then to their REA xrefs and
  caller decompilations. `timeGetTime` at IAT `0x089E3744` has xrefs at
  `0x00489FEB` and `0x0048A226` inside `FUN_00489D50`; the recovered use
  timestamps socket reads through `DAT_01F60E34`, not a weekly scheduler
  (`ev_0dcaa594706ad126995e6e6a34a367dd180c0d5467abbf6c7d17314f47a46fa4`,
  `ev_25b53e73dc366a58033de5f187c546b4134426e7689e600d21b8e33606c2eb3d`).
  `GetTickCount` at IAT `0x089E3500` resolves at `0x004CAEC2` to
  `___security_init_cookie`, not guild logic
  (`ev_20cea2ca1ed694b58229aad3c71130de443ff14f4eb90f35c37ea7c073daa4cc`,
  `ev_6129f1a0ed1475681ae4c66aa109c5ef31d99e800e3038450f4fe08ac8cce558`).
  `GetSystemTimeAsFileTime` at IAT `0x089E3548` has xrefs at `0x004A119C` and
  `0x004CAE95`; the identified uses are CRT time/startup helpers, with no
  guild-fame mutation or weekly reset path
  (`ev_90c663afe70feaf5f510d439c4e938f54b79d90e3ae0ffde71ed6f590f756eab`,
  `ev_37130905844e0116803f03838e2326090e10a75869b302e67417b8438f27762a`).
  The call-site snapshots are
  `build/rea-analysis/work/DBSRVIN-weekly-reset-clock-xrefs-20261022.rea-snapshot.json`
  and `build/rea-analysis/work/DBSRVIN-weekly-reset-clock-callers-20261022.rea-snapshot.json`.
  A bounded audit of the `DBSRVIN.exe` `Sleep` IAT at `0x089E34C8`
  (`ev_460ca642734b64d501b6189fdfe204e369d8b0acebfe8e645f1d48e1246b424e`)
  resolves its four code callsites as follows:
  - `FUN_0047E830` (`0x47E830..0x47E9F8`) scans a fixed indexed table,
    batches `0xC1B` messages in groups of ten, sleeps for 30 ms between full
    batches, then sends a `0x41E` summary message. The body contains no
    guild-fame, calendar, or reset mutation
    (`ev_886975fed5838b25e238ddf93a21e20e78500750c9a6044b06fb2187e874d972`,
    `build/rea-analysis/work/DBSRVIN-weekly-reset-scheduler-20261022.rea-snapshot.json`).
    Its only observed incoming reference to the exact entry is a thunk at
    `0x00456DAB`. Following that thunk resolves its consumer to the already
    mapped `FUN_00471210`: the call at `0x00471B0A` is in dispatcher branch
    `case 0xC19`, so this worker has an observed operation-dispatch path rather
    than an observed timer/scheduler trigger. This follow-through adds no new
    function target to the running count
    (`ev_5b004a739d0f302b13199e9ee0ac040676cdc4c79fb1b49733d8b2342f49c173`,
    `build/rea-analysis/work/DBSRVIN-weekly-reset-FUN-0047E830-xrefs-retry-20261022.rea-snapshot.json`;
    `ev_1eefc1c2769fc45409211c77260b40e7a8d3908fb40a95908b935d02d6e2df6b`,
    `build/rea-analysis/work/DBSRVIN-weekly-reset-thunk-caller-00471B0A-20261022.rea-snapshot.json`).
  - `FUN_004900F0` (`0x4900F0..0x4926DA`) scans account files and emits
    per-character `FAME,%s,%d,...` export rows alongside `GUILD`/`GMASTER`
    rows. Its `Sleep(2000)` follows the end of that export; this is character
    account export, not a guild-fame reset. This is a repeat examination of
    the already mapped body, not a new function entry
    (`ev_0eb1a2bed9c9cbbe9ded889c48555c17fc37874f8709c92545184cb9b4fb226d`,
    `build/rea-analysis/work/DBSRVIN-weekly-reset-Sleep-00492685-20261022.rea-snapshot.json`).
  - `FUN_00495460` (`0x495460..0x495D06`) scans `./OLDaccount` records and
    passes qualifying account data to the account-update helper
    `FUN_0047C480`; its `Sleep(2000)` is at the end of that processing. The
    recovered body contains no guild-fame or weekly-calendar logic
    (`ev_104585e42f5ee12a9ae6a1cf774494f77159d7bb2fa427265cdf7d2418c57b47`,
    `build/rea-analysis/work/DBSRVIN-weekly-reset-Sleep-00495CB1-20261022.rea-snapshot.json`).
  - The call at `0x004FF2AE` belongs to the Visual Studio CRT `__Once`
    helper (`0x4FF260..0x4FF2B9`), which performs an interlocked one-time
    initialization and uses `Sleep(1)` while waiting for its state to become
    complete. It is not a server timer or fame path
    (`ev_35bf2017c736879cb65fe8e425b761edd8efb7c5ad602ac2a17de4e6c97fb475`,
    `build/rea-analysis/work/DBSRVIN-weekly-reset-Sleep-004FF2AE-20261022.rea-snapshot.json`).
  These four callers therefore add no automatic weekly guild-fame reset
  candidate. The result narrows this executable's `Sleep`-based leads only;
  it does not prove the reset is absent, and does not cover an indirect trigger,
  another process, or runtime configuration.
  The three remaining raw-scan addresses are now classified as import thunks,
  not game logic. REA `read_bytes` evidence at `0x0049BFCE` returns
  `FF 25 44 37 9E 08`, an indirect jump through the `timeGetTime` IAT entry
  `0x089E3744` (`ev_65be4b5451e2ec259b4d2a8c0ea0bd91c6897c55f3911693c24857be63ed4587`).
  A second REA read spanning `0x004FF340..0x004FF4BF` identifies
  `0x004FF346` as a jump through `GetSystemTimeAsFileTime` IAT `0x089E3548`
  and `0x004FF4B4` as a jump through `GetTickCount` IAT `0x089E3500`
  (`ev_25aebb41459baccb5379681014966b5f68011adecdeb6a52d5488ab7051ca24a`).
  These read-bytes operations do not produce snapshot files; their direct REA
  evidence records are the retained artifacts. The already traced callsites
  resolve to socket-read timing and CRT security/time helpers, not a weekly
  guild-fame reset. This import-thunk classification adds no function entry.

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
  is not evidence that process enumeration or patching succeeded. The two
  Ghidra pseudocode renderings for `FUN_10001180` report `+0x15400`, but each
  conflicts with the decoded instruction at `0x1000122D`, `ADD ECX, 0x55000`;
  a fresh exact-byte read returns `8B 4D F8 81 C1 00 50 05 00 89 0D 08 DC 04 10`
  (`ev_211d5b8faad4049f6a0fe9de8d1ac8d2237ff0bb2e4710e5bd5f8d56107ab719`).
  The machine instruction and bytes therefore support `+0x55000`; `+0x15400` is
  recorded as a decompiler inconsistency, not as the address calculation.
  The exact xref inventory for `DAT_1004DC08` contains four sites:
  `0x10001033` clears it, `0x1000109A` and `0x100010C8` read it to derive the
  copy and inline-hook destinations, and `0x10001233` stores the matched
  process base plus `0x55000` (`ev_2b7df0ac6ece1a38a54a399fc917d72e3781536c38486c7730669b828c454d0d`).
  This closes the statically referenced lifecycle of this global within the
  analyzed DLL; it does not establish whether process discovery succeeds.
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
  the patched bytes inside that network-event procedure. A bounded instruction
  reconstruction places both sites in case `0x405`'s fallback candidate scan:
  `0x48A839` is reached when the indexed table value at
  `0x02714F68 + index*4` is zero and originally jumps back to increment the
  scan index; after that zero check, `0x48A84E` originally rejects a value that
  differs from `DWORD PTR [0x02735AA8]`, again incrementing the index rather
  than storing it as the candidate. Replacing both branches with NOPs removes
  those two fallback filters and allows the first scanned index to reach the
  candidate store at `0x48A850`. The later per-slot state guard at `0x48A8E3`
  remains, so the static patch flow does not prove that the selected slot is
  accepted or that the admin operation completes. This closes the branch
  context only; DLL attachment, patch execution, live values, and runtime effect
  remain unobserved. See
  `build/rea-analysis/results/DBSRVIN-hook-branch-context-20261022.md` and the
  exact instruction evidence (`ev_5839785915b280901c04ed0e5b36bb6f531797b1be8cc10f2cd4b98ad21617c4`,
  `ev_6e83e3f43b8658b25d4a4534a2b23ce55cfcb923004ff66eb6e70421cfa0b53b`,
  `ev_e3011c32ca996d2a7f6b3af79002a837b97484302bbca1db5679cb0e6a293ea5`).
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
  this statically supports a paired path/argument adjustment. A Ghidra exact-byte
  read spanning both call sites (`ev_58bb22c81388627632ba0c1bf88b471df2721146117baa5f79ba85781dc4fbad`)
  confirms the original pointer operands at `0x48DE7F` and `0x48DF1A`; decoding
  those instruction boundaries gives `ADD ESP,0x0C` with cleanup immediate at
  `0x48DE91`, and `ADD ESP,0x10` with cleanup immediate at `0x48DF2C`. The
  setup writes reduce each cleanup by four bytes, matching the removed
  four-byte server-index push at `0x48DE7D` and `0x48DF18`. These paired update
  rewrites are instruction-aligned; runtime file processing remains
  unverified. The separate `data%2.2d.csv` rewrite exposes
  a one-byte target error. `FUN_10001250` passes the DLL string pointer
  `0x10043274` and destination `0x4880FE` to `FUN_100013D0`
  (`ev_6edfd537ffef3b3740f9a7822636db9ad2307be00c12b2b1bcad1aaed223dc5b`);
  the helper performs a four-byte write at its destination
  (`ev_a9f88114b136d24dedc1e77f20c59c35aef9d985663c305d2361a84270b25ea6`).
  A fresh 96-byte read from `0x4880D0`
  (`ev_659df1672adf0df7dc66e7e7d09b404421f2973af299ee3d2e2271dece299faf`)
  confirms that `0x4880FE` is the `0x68` opcode of `PUSH imm32`; its four-byte
  immediate begins at `0x4880FF` and is `4C 6D 50 00` (pointer `0x506D4C`).
  The subsequent instructions push the server-index global, format pointer,
  and local buffer before a call, then pass the buffer and the `0x506D48`
  pointer to another call. This is consistent with formatting the indexed CSV
  path and opening it, though the callees' library identities are inferred and
  runtime behavior is not observed. If the setup write executes as coded, its
  DWORD replaces the opcode and the first three bytes of the immediate, rather
  than just replacing the immediate; the intended path rewrite is therefore
  statically malformed. The correctly aligned immediate address would be
  `0x4880FF`, but no such write is present in the analyzed setup. Exact bytes
  at `0x456446` (`E9 85 1C 03 00`,
  `ev_a206b4a5971bd10b561d2768756c8f2973a722f9ccb91df954c8302c359f62ab`)
  decode as a direct relative jump to `0x4880D0`, establishing one inbound
  machine-code transfer from an unrecognized thunk region. Ghidra's xref query
  still reports no caller, and function lookups at both addresses fail to
  produce a containing procedure; the thunk's higher-level dispatch source and
  runtime reachability remain unknown. The instruction sequence is established
  by exact bytes, not a Ghidra function boundary. A bounded audit found no
  additional direct writer, but does not rule out indirect writes. This site
  adds no function to the tally.
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
  byte values `0x08`, `0x0C`, and `0x90`. The CSV string write is misaligned by
  one byte and would overwrite the `PUSH` opcode at `0x4880FE`; the audited
  ImportItem/Update path operands and call-site cleanup edits are instruction-
  aligned, as is the validation-limit operand described below. This establishes
  attempted path/string rewrites, not successful runtime redirection. The same
  setup calls `FUN_100013D0(0x3C, (undefined4 *)0x48D940)`, which writes the
  DWORD `0x3C` under temporary page protection
  (`ev_a9f88114b136d24dedc1e77f20c59c35aef9d985663c305d2361a84270b25ea6`).
  In the original `DBSRVIN.exe` bytes, `0x48D940` is the immediate operand of
  `MOV ECX,0x78` at `0x48D93F`; the following `IDIV ECX` checks its remainder
  and reaches a call at `0x48D94A` when the remainder is zero
  (`build/rea-analysis/work/agent-dbwyd-hook/DBSRVIN-48D8E0-48D970-objdump.txt`).
  Since this instruction is in the ID-0 `WM_TIMER` callback described above,
  the hook changes the importer trigger from every 120 to every 60 timer
  messages. The wall-clock cadence, intended business effect, and successful
  runtime application remain unverified.
- The same setup's `FUN_100013D0(0x1964, (undefined4 *)0x48F705)` write
  targets the four-byte immediate of `CMP dword ptr [EBP-0x588],0x1000` at
  `DBSRVIN.exe!0x48F6FF`, not an opcode or instruction boundary
  (`FUN_10001250`: `ev_6edfd537ffef3b3740f9a7822636db9ad2307be00c12b2b1bcad1aaed223dc5b`;
  DWORD helper: `ev_a9f88114b136d24dedc1e77f20c59c35aef9d985663c305d2361a84270b25ea6`;
  80-byte read from `0x48F6E0`: `ev_2f300b6324df636e5411a584f3b54fcbefc3bdfef825ba370ebe80b6f199e823`).
  The surrounding `JL` rejects negative values and `JGE` rejects values at or
  above the patched threshold, so the static upper-exclusive limit changes
  from `0x1000` (4,096) to `0x1964` (6,500), widening the accepted range from
  `0..4,095` to `0..6,499`. In the existing `FUN_0048F410` decompilation, this
  is the first numeric field parsed from the import record; that value is
  packed into the low 16 bits of `local_608` and carried through the cargo
  update/save path (`ev_e5935cb578e726f0374fa11acbf8d0c2412065edd85ce10d84e3a199b554b728`).
  Its more specific business meaning and the runtime effect of the patch remain
  unverified.
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
  A fresh direct dossier of these bodies against `DBSRVIN.exe` SHA-256
  `3adb3a1704df4910604f0b862d69d963f4e1a017efe5743a23e6f07ad1ecf736`
  confirms that `FUN_00483E90` gates on object state at `+0x20`, derives the
  transform key using the counter at `+0x40` and state bytes (or global
  `0x00B71C94`), then calls the thunk to `FUN_00484090`
  (`ev_c961608403ad9ed1bf2052499e934849988e1f28f46ba5bde56232138a360131`,
  `build/rea-analysis/work/DBSRVIN-worker-callee-00483E90-20261022.rea-snapshot.json`).
  `FUN_00484090` frames the message, transforms its payload with lookup tables,
  appends it to the outgoing queue, and calls `FUN_004844C0` to flush; its direct
  dossier is `ev_fea16fad82f659b3eb3ebd99ba84c357bbda462322832acac6c19bda30d3160e`
  (`build/rea-analysis/work/DBSRVIN-FUN_00484090-20261022.rea-snapshot.json`).
  The wrapper does not inspect or assign semantics to opcodes `0xC19`, `0xC1B`,
  or `0x41E`, and no direct guild-fame or weekly-reset mutation was identified.
  `FUN_004844C0`
  (`ev_190b29e76b0f31dac6d16cc8fd70819b6565d054d2750c629cf159eb2caa1f0e`)
  validates the buffer offsets and calls Winsock `send`, advancing partial
  sends or clearing the queue on completion/error. Thus the skipped `0x1005`
  block is a real outbound network-message path in static code, not just local
  packet construction; no runtime transmission was observed.
- The callback target `0x10001450` is passed from the setup at
  `0x100010D9` (`ev_6647e499daa674a895b8e591c3b27331df5854f9a5315f51c6b0e8a23ee41ddc`)
  and maps to file offset `0x850`
  (`ev_4ed5c5c136e36088b15f86a2f78effe8fb5b7a2750412750ad514d3d3dfd294a`).
  The separate xref classification for the `0x100010D9` push is DATA rather
  than a direct CALL/JUMP (`ev_67c0745fe0dbf634a9adffe162deea7438a2240be5b7838a3bc1c6ca7093c184`):
  setup passes this address as a callback to `FUN_10001560`, alongside the
  `DBSRVIN.exe` patch site and a two-byte tail. The patch writer emits
  `E8 rel32 90 90` (`ev_e785c84de979d2f63d663d2284e0d9fae60ac9a32bc21df0d621fa4552d3a375`,
  `ev_c8dc2f87a35bb7f19718d99abd803fe70f771154cef5bb35a91a84f5b2dc6947`);
  this is static evidence of intended callback injection, not proof that the
  DLL loaded or the patch ran.
  It lies between `FUN_10001410` and `FUN_10001470`; no Ghidra procedure starts
  at or contains the callback label. However, raw callback bytes
  (`ev_c35ec6f08ea9d4d32f44f1448b4bf3ee53cc63038feff805e1a812fade397282`,
  `ev_bcaaed583af0da86dc3275f8d5f2cf99536ad6be20350b26b45235a65624af52`)
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
- `DBWYD.dll!FUN_10001E58` (`0x10001E58..0x10001EA9`,
  `ev_071a2ca31d252607dba4c5af4f6bbf1b53d5c15b73d4f5f3ed42ffd1f4636c7c`,
  `build/rea-analysis/work/DBWYD-init-registration-helper-10001E58-20261022.rea-snapshot.json`)
  is called by `FUN_100017B0` at `0x100017D4` with `&DAT_1004DC1C`. It enters
  critical section `DAT_1004DC4C`; guard `0` becomes `-1` to claim
  initialization, and a thread observing `-1` calls
  `__Init_thread_wait(100)` and retries. After initialization, it writes
  `DAT_1004D204` into the current thread's TLS slot at offset `+4`, using
  `_tls_index`, then leaves the critical section. The owner path proceeds to
  `FUN_10001830` and registers `_atexit(FUN_10042830)`. A follow-up confirms
  `FUN_100017B0` then calls `FUN_10001E0E` at `0x10001801`; this is a sibling
  call to `FUN_10001E58`, not a call made by `FUN_10001E58` itself.
  `FUN_10001E0E` (`ev_1ffef141728d0472b0a60582fe94ab030f084a90c0505cde323a354091b22c3f`)
  increments `DAT_1004D204`, stores the new counter value through its argument
  (here `&DAT_1004DC1C`), and writes the same value to the current thread's TLS
  slot at offset `+4`. Thus the guard moves from the in-progress value `-1` to
  a completion-counter value, rather than a fixed completion constant; it then
  calls `__Init_thread_notify`.
  `__Init_thread_wait` (`ev_40d501c15cbe4d4ea8e153dd49068d8660c3ab654ee4de9b451d43ba7c1e9516`)
  waits on `DAT_1004DC48` through `WaitForSingleObjectEx`, while
  `__Init_thread_notify` (`ev_121f88ce88b5c0a91a4dcef45c73a0712210d9edb7b88b5a58a9344e30789144`)
  calls `SetEvent` and then `ResetEvent` on that same object. The helper
  `FUN_10001E58` (`ev_071a2ca31d252607dba4c5af4f6bbf1b53d5c15b73d4f5f3ed42ffd1f4636c7c`)
  calls `__Init_thread_wait(100)` at `0x10001E78`. This statically establishes
  an event-backed wait/notify path, but not the event's creation/configuration,
  how many waiters are released, or runtime scheduling behavior. The new
  `FUN_10001D4C` analysis (`ev_b06feae1700083cd69e7a632204bda0da5a98891ba38ddfda8b3d959d4e2f219`,
  `build/rea-analysis/results/DBWYD-event-xref-function-10001DBD-20261022.json`)
  resolves initialization of `DAT_1004DC4C` with
  `InitializeCriticalSectionAndSpinCount(..., 4000)`, dynamic resolution of
  `SleepConditionVariableCS`/`WakeAllConditionVariable` into
  `DAT_1004DC64`/`DAT_1004DC68`, and the fallback that creates an unnamed,
  manual-reset, initially non-signaled event in `DAT_1004DC48`. Failure to
  obtain the synchronization module or create that event reaches
  `___scrt_fastfail(7)`. The five direct event references link this setup to
  wait/notify code, but callback addresses, host API availability, exact
  initialization order, and runtime branch execution remain unobserved. The
  initial value and broader meaning of `DAT_1004D204` also remain unresolved.
  A focused instruction query classifies `0x10001D25` as a five-byte
  unconditional `CALL 0x10001D4C`; Ghidra reports no owning procedure at the
  callsite and records an incoming data reference from `0x10043194`
  (`ev_7d25c594ff7b06bbae6ab85a2c8768d4bf9826fbbc6a9b99c70bb1326ee62a4d`).
  A 32-byte read at `0x10043190` confirms the DWORD at `0x10043194` is
  `0x10001D25`, followed by `0x1000C690`, `0x1003918E`, `0x1003D4BE`, and
  zeroes (`ev_b14d9e5b85088702b452451b06c69b63221c50b7a6e18bae5b4f9f1c4d6189c0`).
  These adjacent values are consistent with pointer-like data but do not prove
  the container's type or dispatch semantics. A bounded search of nearby
  procedure names returns `FUN_10001D4C` and `FUN_10001DCE`
  (`ev_a80aab725aca9a370f5014c859ae54d2aae59fb51e7c6223455aa738c31491e4`);
  it does not assign a procedure to `0x10001D25`. The direct call is now
  established, but its owning routine, data-container semantics, dispatcher
  order, and runtime execution remain unresolved. Direct analysis of the
  nearby `FUN_10001DCE` body (`0x10001DCE..0x10001DE9`, 28 bytes;
  `ev_2a8df8fc71db62f2efd4fa1b1b60d0f9d85d8645d5bef3b6f26f181941c3197a`,
  `build/rea-analysis/results/DBWYD-init-teardown-10001DCE-20261022.json`)
  shows it calls `DeleteCriticalSection(&DAT_1004DC4C)` and conditionally calls
  `CloseHandle(DAT_1004DC48)`. Ghidra records an incoming DATA reference from
  `0x10001D36`. A separate disassembly query verifies that this address is the
  immediate operand of `PUSH 0x10001DCE`, followed by a call to `0x10001D10`;
  it is not a pointer-table slot. The direct analysis identifies `0x10001D10`
  as `_atexit`
  (`ev_ee497cae6d4e713b056c937f84575c92f40e860711fdd711750a08eac0422e23`,
  `build/rea-analysis/results/DBWYD-registration-helper-10001D10-20261022-retry.json`),
  called from `FUN_100017B0`. Its 21-byte body calls `__onexit` at
  `0x10001CC0`; that 45-byte helper
  (`ev_1e528385c4c19d09d2922822b357f3c6e044fefbd97635a1022bad384b53c76d`,
  `build/rea-analysis/results/DBWYD-onexit-helper-10001CC0-20261022.json`)
  selects `__crt_atexit` when `DAT_1004DC2C == -1`, otherwise
  `__register_onexit_function`, and returns the callback on success. This
  establishes a static CRT registration path for `FUN_10001DCE` from
  `FUN_100017B0`; it does not prove that the initializer runs under the actual
  Windows loader or that the callback later executes. The callback teardown is
  consistent with resources initialized by `FUN_10001D4C`, but complete
  initialization/teardown order remains unresolved. The body was absent from
  the prior mapped evidence for this image and raised the running floors to at
  least 345 examined entries and 219 directly represented targets. These are
  not a corpus-wide census. The direct `_atexit` analysis adds an examined
  entry but no represented target because its body was already listed in the
  `FUN_100017B0` callee inventory; `__onexit` adds one new body and target.
  Together with the subsequent `Core.dll` finding, the current floors are at
  least 348 examined entries and 221 directly represented targets.
  The focused
  completion follow-up is saved as
  `build/rea-analysis/work/DBWYD-init-completion-10001E0E-20261022.rea-snapshot.json`
  (`ev_2783e5332785633ca448b842c2aea8bc74b9123aea88fb0bdf29f8fb3a212e1c`,
  `ev_071a2ca31d252607dba4c5af4f6bbf1b53d5c15b73d4f5f3ed42ffd1f4636c7c`).
- `DBWYD.dll!FUN_10042830`
  (`ev_10bb1cfb17c268cb0896401416971a2c3ed29eacd8e2f192e7910673025f1055`,
  body `0x10042830..0x1004283E`, 15 bytes,
  `build/rea-analysis/results/DBWYD-shutdown-callback-10042830-REA-evidence-20261021.json`)
  is the exit callback registered by `FUN_100017B0` through `_atexit`. Its
  body passes `DAT_1004DC18` to `FUN_10001870`. Direct analysis of that callee
  (`ev_bb9c96a714a7a1f7fe3b0c14f398104df9cec51ae1b54a312e92e5501371ee94`,
  `build/rea-analysis/work/DBWYD-shutdown-callee-10001870-REA-evidence-20261021.json`)
  shows a 28-byte body that writes `DBSRV::vftable` to the supplied object and
  then calls `FUN_10001850`. Its 20-byte body
  (`ev_29cfb0f380b17b1bc6eaef3e1e439d63cc044fd55d65f0bbe49cefc0edbe554c`)
  writes the Ghidra-labeled `Singleton<DBSRV>::vftable` pointer to that object.
  A separate wrapper at `0x100018C0`
  (`ev_66877ae92b51d070612b4e2130db4fd2af4ef390d09efedec799aa2b75cc7046`,
  `build/rea-analysis/work/DBWYD-scalar-deleting-wrapper-100018c0-20261021.rea-snapshot.json`)
  calls `FUN_10001850`, then calls `FUN_1000198D` only when parameter bit 0 is
  set. Ghidra labels it with an ambiguous FID match to multiple MSVC concurrency
  scalar-deleting-destructor names; that label alone is not proof of its source
  identity. `FUN_100018F0`
  (`ev_d307d45e1a5c800892b92e0432039717ad960b017e36a9d641b1080f2a16e790`,
  `build/rea-analysis/work/DBWYD-deleting-wrapper-100018f0-20261021.rea-snapshot.json`)
  similarly calls `FUN_10001870` and conditionally calls `FUN_1000198D` when bit 0
  is set. The release chain is now traced: `FUN_1000198D`
  (`ev_2b902121f5b90addf9b949cabac8d703f2b0ce119d1d4dc0ee210de1ae7fa909`)
  calls thunk `0x100022DE`
  (`ev_ab375cbaf0ec4790a76ffedd01dff045b331b39ffb230b2159f36af60dc0e4b4`),
  which jumps to `FUN_10020CD1`
  (`ev_219ffe59decf3a58a4b983f70fee9760a457bb34615d200c68bd008d648817cc`).
  That wrapper calls `0x10025846`
  (`ev_5c570c25ece8a1c3a8bd1827a0dc57337889b8ac597c530c7af34fca55f89fba`),
  whose Ghidra FID matches `__free_base`/`_free`; its recovered body calls
  `HeapFree(DAT_1004E844, 0, pointer)` when the pointer is non-null, and sets
  `errno` from `GetLastError` if that call fails. This establishes a real heap
  release inside the conditional wrapper paths, but invocation of either wrapper
  is not established: their function dossiers report zero procedure callers and
  one incoming reference each, both classified as `DATA` with no containing
  source procedure. The references are at `0x100432C4` for `FUN_100018C0`
  (`ev_30ea8917f8ab4c825b5ff7b313cad338896c45790c8bcd5d137370759944fcae`)
  and `0x100432CC` for `FUN_100018F0`
  (`ev_336cb1df02fb3624bb5219ecda632e1e5ab7dce3d23ae58e3eec5f810531c21d`).
  The 32-byte read at `0x100432C0`
  (`ev_4e47cf67ee80e0fe2d0d03d322908a0a63a52d1b994494667ff593c58a87058a`)
  resolves the adjacent table layout: the words at `0x100432C0`, `0x100432C4`,
  `0x100432C8`, and `0x100432CC` are respectively `0x1004A740`,
  `FUN_100018C0`, `0x1004A6C0`, and `FUN_100018F0`, followed by the ASCII text
  `Error: DBWYD.dll`. Together with constructor writes, this places locator
  `0x1004A740` immediately before the base `Singleton<DBSRV>` table at
  `0x100432C4`, and locator `0x1004A6C0` immediately before the `DBSRV` table at
  `0x100432CC`.
  The 96-byte read at `0x1004A740`
  (`ev_3c449f6750dd4d7e77284cad2c524437de728de80015420f85e2f7ea30ff65eb`)
  matches a 32-bit MSVC RTTI Complete Object Locator: it points to the
  `Singleton<DBSRV>` TypeDescriptor at `0x1004DAD4` and a hierarchy descriptor
  at `0x1004A728` with one base entry at `0x1004A70C`. The 128-byte read at
  `0x1004A6C0` (`ev_44bf694491c1ab75df23a7ee8e6f42578980f96fd5852b076467d64e7d93f3db`)
  matches the corresponding locator for `DBSRV`: TypeDescriptor `0x1004DAC0`,
  hierarchy descriptor `0x1004A6D4`, and a two-entry base array listing
  `DBSRV` and `Singleton<DBSRV>`. The TypeDescriptor names are visible in the
  96-byte read at `0x1004DAC0`
  (`ev_b15a0d9546411e1781977516aa9e5891f2fecdca56c9b32851694591534db298`).
  This structure is consistent with Clang's documented Microsoft ABI RTTI
  layouts [source](https://clang.llvm.org/doxygen/CodeGen_2MicrosoftCXXABI_8cpp_source.html);
  that ABI reference corroborates the interpretation but does not replace the
  target evidence. These REA reads reflect Ghidra initialized memory, including
  loader fixups, rather than a raw-file-byte dump.
  The table words at `0x100432C4` and `0x100432CC` reference
  `FUN_100018C0` and `FUN_100018F0` as data, not direct calls. Their zero direct
  procedure callers therefore do not rule out virtual dispatch, but no runtime
  dispatch has been observed. `FUN_10001870` writes the derived table before
  calling `FUN_10001850`, consistent with derived/base destructor ordering.
  The registered exit callback calls `FUN_10001870` directly, not either
  wrapper; its observed path ends with vtable-pointer writes and does not reach
  the free helper. Registration and these control-flow observations are static
  evidence; process-exit execution remains unobserved.
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
