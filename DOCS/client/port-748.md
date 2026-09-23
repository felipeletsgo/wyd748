# Porting the buildable client to WYD-Go

## Status

The imported source originally targeted the Global/KR 7.69+ client. In this
tree, it is being adapted as a single-version Win32/DirectX 9 implementation
for the proven WYD-Go 7.48 protocol, ABI, UI, and assets. Do not retain an
executable path for the upstream version.

The installed candidate is `tmproject/client748/project.exe`. With
`Build-Client.ps1 -NoDeploy`, only the artifact in `tmproject/build/` is
updated; the installed executable remains unchanged. Historical executables
and patchers are not fallbacks, products, or validation gates.

## 7.48 asset profile

The buildable client can read `tmproject/client748/` directly through
`WYD_ASSET_ROOT`. Its loaders explicitly recognize and translate:

- 7.48 texture tables with 264-byte records into the 528-byte in-memory
  TMProject representation;
- `UITextureList.bin` as the legacy name of `UITextureListN.bin`;
- `SkillData.bin` with 104 records of 96 bytes;
- `strdef.bin` with 440 strings of 128 bytes, leaving later, nonexistent rows
  empty;
- the 18 precompiled DirectX 9 shaders required by the renderer (`skinmesh`,
  `vseffect`, and `pseffect`), imported from the official `WYDESTINY` runtime
  at the same TMProject commit;
- the 7.48-only server selection screen in `SelServerScene2.bin`, including
  runtime composition of the six `NewUI_ServerList_*` and
  `NewUI_ChannelList_*` panels absent from the RC file;
- the optional parser for old RC records with inline captions, retained for
  legacy resources without replacing the main 7.48 screen;
- `sn.bin` as a fixed binary table of 11 names and 11 group orders, instead
  of the text format expected by newer TMProject. The selection scene loads
  and validates this file before creating controls; the old text read during
  app startup was removed;
- bounds on optional names, groups, and endpoints in initial selection and
  channel switching, based on the actual local table dimensions. Channel 10
  remains valid in `serverlist.bin`. If no safe optional name exists, the UI
  uses the channel number. An aggregate source outside the table is rejected
  without reading adjacent memory. Empty or unterminated endpoints clear the
  local destination instead of retaining a previous IP. This is
  `MODERNIZACAO_COMPATIVEL` and does not change the wire contract;
- the same compacted visible row for each server group's background, label,
  and click target. This keeps groups aligned when there are gaps and retains
  the positions of the dense 7.48 configuration without changing the wire;
- the final color on every group background after the selection transition,
  rather than only the first two. Duration, logos, and server contract remain
  unchanged;
- `config.txt` as the named 7.48 configuration, preserving resolution,
  window mode, UI variant, camera, cursor, audio, and animation without
  overrides.

`ItemList.bin`, `ValidIndex.bin`, `object.bin`, `serverlist.bin`, and
`AttributeMap.dat` match the sizes consumed by the code or have only an
additional checksum ignored by the loader. Asset checks should follow the
loaders, the `wyd-client748-assets` skill, and
`tools/client-assets/Audit-ClientAssets.ps1`, which distinguishes declared
debt from unclassified missing references and casing mismatches. The current
build is described in [Build and integration](../build-and-integration.md).

A missing literal source reference alone does not prove that a 7.48 asset is
missing. In particular, `TMSkinMesh::SetCostume` still contains later-source
paths and paths overlapping the cataloged KR collection. The
`SourceMissingUnclassified` bucket is diagnostic for prioritizing research;
do not fabricate assets, remove `case` branches, or turn that bucket into a
parity gate without confirming the active flow and resource provenance.

The original 7.48 client did not have these shaders because its executable
used another rendering path. They are a runtime dependency of the buildable
client. Their compatibility with vertex declarations still needs validation
alongside the renderer; this reorganization performed neither visual
validation nor an automated shader-hash gate.

The TMProject render-target initializer was also corrected to use
`D3DPOOL_DEFAULT`, required by DirectX 9 for textures with
`D3DUSAGE_RENDERTARGET`. The imported code used `D3DPOOL_MANAGED` and
allocated the source texture twice, causing initialization failure and a leak
before the first scene.

## Cataloged costumes: selection and loading

The `Equip[13] -> TMHuman::InitObject -> TMSkinMesh::RestoreDeviceObjects`
flow now uses the full costume ID and the `Costumes-KR.json` table. Of 135
items, 130 select the 129 cataloged renderers; five base costumes delegate to
an existing renderer. Imported costumes bypass the later `SetCostume` switch.
Items outside the collection retain the existing path; absence from the
catalog alone does not justify removal.

Selection preserves the current skeleton and its variants, and uses skeleton
parity to choose King and TopRanker variants. An internal marker isolates
imported renderers from NPC types; it is not sent over the protocol. The six
parts are resolved by index, with no shared cursor: an empty part zero keeps
the native face, other empty parts are omitted, and parts outside the table,
such as weapons, retain the original path. There is no new runtime JSON parser
or change to score, bonuses, or server authority.

Classification: `MODERNIZACAO_COMPATIVEL` in source, reusing data already
validated as `PARIDADE_NATIVA`. Evidence for this work:

- Native binary/Ghidra and studied decompilation: used through the
  [KR costume record](../../.agents/research/client748/inventory/costume-native-contract.md),
  including selection, the internal marker, and skeleton/face preservation;
  no new analysis or historical-binary patch execution.
- 7.48 assets: used; manifest, 774 parts, and dependencies checked.
- Current TMProject: used to integrate selection and loading. Its later
  hardcoded mapping contradicts the manifest for imported costumes with
  different types or paths, not for the entire renderer.
- WYD-Go and tests: used; current equipment projection and slot 13 consumption
  inspected, focused client tests added. No wire change and no new server suite
  run in this batch.
- Additional guides: not applicable; no new contract was introduced.

`tools/client-assets/Export-CostumeTable.ps1` emits the C++ table for review;
`-Check` compares the compiled header with the manifest and checks referenced
files. This gate runs once per `Build-Client.ps1` invocation, before tests or
candidate installation.

Batch validation: Release build, table and dependencies checked, and 35,239
automated assertions passed. The candidate was installed with SHA-256
`9021B1FAAB444CBE85EE7141368C14890969A51499829848DE360752BC5B5E3F`.
On 2026-09-12, the user confirmed that costumes worked in game:
`CLIENT_TESTED` for the exercised flow, as well as `BUILD_VERIFIED` and
`AUTOMATED_TESTED`. That confirmation does not distinguish every item/body,
remote observer, logout/relogin, or transformation, and does not imply
exhaustive coverage of that matrix.

## Contracts

1. `model.Score` remains the sole authority for attributes.
2. `STRUCT_SCORE` is 140 bytes: 35 uint32 fields in the current coordinated
   contract.
3. Score fields received from the client never drive gameplay.
4. Additional attributes are part of score directly; no sidecar is active.
5. Migrate each packet with size and offset tests on both sides.
6. `tmproject/client748/project.exe` is the only executable candidate; the
   historical 7.48 binary is a read-only Ghidra reference.

### Rule for comparing versions

TMProject 7.69 may provide candidate architecture and algorithms. For legacy
boundaries, native 7.48/Ghidra remains the primary evidence. Classifications
and gates are in [AGENTS.md](../../AGENTS.md).

Compatible modernization preserves the proven contract. A coordinated
extension may change it when both ends are explicitly adapted and tested.
Neither should be presented as native parity. Later assets require validation
of formats, resources, loaders, and observable flow; absence from the native
client does not justify automatic removal.

Failure to read an RC file aborts initialization of a 7.48 scene. The
`FieldScene2.bin` HUD fallback applies only to a successfully loaded resource
that lacks modern control `66817`; an invalid resource must not be treated as
a legitimate layout variant.

The missing-attacker recovery path in `TMFieldScene::OnPacketAttack` now reads
damage target IDs without modifying the received frame and retains the
attacker coordinates from that frame for effect fallback. This is
`MODERNIZACAO_COMPATIVEL`: the existing `0x369/16` request and the server's
visibility and gameplay-space checks are unchanged. The native request
envelope is recorded in
[missing-entity-request](../../.agents/research/client748/flows/transport/missing-entity-request.md);
the exact target-selection predicate is not claimed as native parity. A
client-side attack/recovery run remains pending.

## Active score layout

The [canonical contract](../SCORE.md) replaces the historical 48-byte layout.
The current size is protected by `static_assert` in
`tmproject/TMProject748/internal/core/WYD748Compat.cpp` and by the encoder and
tests in `wydgo748/internal/wire/score.go` and `score_test.go`. Changes must
also cover every packet embedding the score.

## Adaptation sequence

1. Confirm the native flow and the historical reference SHA in Ghidra 7.48.
2. Locate corresponding callers, callees, structs, and assets in live source.
3. Adapt a small packet group or window at a time, removing an incompatible
   path only when evidence and a replacement contract exist.
4. Protect wire/ABI with `static_assert` and byte-for-byte tests.
5. Validate assets and compile. If installation is part of the gate, confirm
   the SHA-256 of `tmproject/client748/project.exe`. With `-NoDeploy`, record
   only the artifact in `tmproject/build/` and the pending installation.
6. Test owner, observer, failure, and relogin before promoting behavior.

Do not change several structural packets at once: `STRUCT_SCORE` is embedded
in larger structures, so each change needs a corpus and test for the final
packet. Never add protocol-variant selection during authentication or
elsewhere; this source implements only the 7.48 contract.
