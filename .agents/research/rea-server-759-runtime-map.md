# REA runtime map: WYD server 7.59 executable and hook DLL

## Scope and inputs

This is an incomplete, static-analysis record for the original server executable
and its hook DLL. The REA 6.7.0 analyses were produced with Ghidra 12.1.2 and
12.1.4. The source files under `references/` are read-only evidence.

| Input | SHA-256 |
| --- | --- |
| `references/decompilacao server/dlls/TMSRVIN.exe` | `27EE313BB4B4D4A0D897DAC9A188D8249068CE701311CBDA1E24AD28C5E43642` |
| `references/decompilacao server/dlls/Core.dll` | `EB9E12B0BF79C874E8E9525D1DD90F6A5FD553D6FCEC2D475F47439CFCF37D62` |

The analysis artifacts are retained under the ignored `build/rea-analysis/`
root. They do not modify the input binaries. Findings below are decompiler or
instruction-flow observations, not runtime proof.

At this checkpoint, the running investigation has examined 142 distinct
function entries: 126 in `Core.dll` and 16 in `TMSRVIN.exe`. The preceding
checkpoint recorded 139 entries; this turn separately analyzed
`Core.dll!0x100317E0`, `Core.dll!0x10012200`, and `TMSRVIN.exe!0x4E6B90`.
This is a tally of examined targets, not a complete inventory of either
binary; referenced functions not separately analyzed are excluded. The
retained per-function result files currently contain 69 unique
`analyze_function` records (63 in `Core.dll`, 6 in `TMSRVIN.exe`); including
the separately returned evidence for `0x10034980`, `0x100317E0`, `0x10012200`,
and `0x4E6B90`, 73 targets are directly represented in the saved corpus or
returned analysis evidence. The larger running tally is therefore provisional
until the missing analysis outputs are reconciled.

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
  (`ev_804c037ce971e4be2857e3cd2c15df659ff91466a787456148bf43327fad3`)
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
  initializes the parsed 32-bit value to zero, and passes its address to
  `0x1002C660` when the stream is readable. The decoded instruction flow then
  reloads that value into `EAX` after stream cleanup; the unreadable/error path
  returns zero. This resolves the apparent `void`/caller-result mismatch in
  Ghidra's pseudocode: the worker's `0x1002FDD0` call does receive the parsed
  value. The filename's substituted identity follows the worker's per-slot
  player argument; the file producer, write cadence, and deletion/consumption
  policy are not established. Static file access does not prove runtime
  delivery or durable persistence.
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
  is a larger state-oriented routine, not an evident scalar file writer; its
  effect on this file remains unresolved. `0x10034980`
  (`ev_6271484f82f30be55ea79b380308f49e42a5905accb7daae5d064d3cd82fe7cb`)
  checks another helper and, on a zero result, calls `0x1001AA90(2,0)` to
  update stream state. This chain does not prove that the queued amount is
  consumed, reset, or protected from repeat credit; do not infer any of those
  behaviors from the helper call alone.
- A defined-string search (`ev_f8a3313b2598180860ce68787bffe336ff704fc7fdd78145f33d6a9f40d0e7d5`)
  found two Cash paths: the queued-credit path above, `Cash/%s.txt`, and a
  separate `Cash/%d.txt` path, plus the `openCashStore` label and administrative
  Cash command strings. Address xrefs for `Cash/%d.txt`
  (`ev_a1f0cae8eea2f16aa35652ee47840a1b7047aa5de9d86a7cbac5ec6b37623cf7`)
  resolve to one use, inside `0x100317E0`.
- Cash file routine `Core.dll!0x100317E0`
  (`ev_63c85df9c1e318e672f97c27533cdc1584d3a01aa3256a940904bd124182fe44`)
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
  slot. This supports a file-backed response/read path, not a direct balance
  write. The body does not establish a purchase or producer for the file; do
  not conflate this path with the `%s.txt` queued-credit path.
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
  globals `DAT_101949c8` and `DAT_101949c9`.
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
  (`ev_0f55b57d26a4bc04dcd03d09186cad7b5413e7834900c7125d2a06619e7efca6`).
  One, at `0x4E940F` in `FUN_004E6B90`, decodes as a 32-bit store to
  `[EAX + ECX + 0x4D7F1A8]`
  (`ev_88ae376132b4f7dd1fb4ac78c1100aece216e50ebab38c056b6588e69a676316`),
  confirming that the executable also writes into this shared memory region.
  This establishes in-memory mutation and client/server update packets, not
  durable fame persistence. `FUN_004E6B90` is a large non-contiguous function
  (35,466 body bytes); its returned dossier exceeded the output window before
  its pseudocode could be inspected (`ev_2fcb6d4cb1bea9467cb8e4853230d86d49e3b58064da445e603415f1675a90a4`).
  No fame-specific file or database writer has yet been identified.
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
  `AutoTrade reloaded!` response are also statically recovered. Inspect durable
  persistence for Tower of Thor fame and the cash/gift worker; the EXE fame
  base has 22 references, so classify its readers and writers before concluding
  whether the value is durable. Continue into the post-purchase helper chain
  only if proving finite-stock mutation is required. The `LastServerBoss` file
  path is resolved above and should not be re-opened without new evidence. The
  transport path from `0x10030660` to
  Winsock `send` is statically traced, but the fixed socket's owner/recipient
  scope and runtime delivery remain unverified. After those bounded paths,
  map remaining relevant functions in `Core.dll` and the executable.
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
