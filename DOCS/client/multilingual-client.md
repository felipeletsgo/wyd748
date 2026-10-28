# Shared multilingual client

Status: the multilingual full-client baseline was published as 7.48.001;
audit corrections were published as mandatory patch 7.48.002, followed by the
caption legibility correction in mandatory patch 7.48.003 on 2026-10-24,
with paired server minimum revision 3 active. Candidate builds and automated
validation passed. The installed local game executable was verified as 7.48.003
on October 25; a subsequent caption candidate is built locally only (see below).
The production native launcher and its language extension
are unchanged. The launcher keeps the site's existing `update.htm`/ZIP protocol.
Actual launcher installation and gameplay validation remain pending.
See the [patch receipt](launcher.md#published-patch-748003-2026-10-24) and
[active server receipt](../current-state.md). This is the client portion of the
[bilingual channels plan](../bilingual-channels-plan.md); it does not implement
channels, citizenship, server translation, or guild-war isolation.

The owner confirmed on 2026-10-24 that the product supports Brazilian and
global players through English and Brazilian Portuguese localization. `PT/`
is supported product content under the current repository rules.
Implementation, tooling and documentation remain English. This policy does
not establish complete localization of existing server or website surfaces.

## Preference and resource contract

One executable and launcher share all models, textures, maps, sounds, layouts,
gameplay tables, connections and server contracts. `EN/` and `PT/` contain only
text resources. The launcher provides a game-language selector and atomically
saves `EN` or `PT` plus CRLF in installation-root `lang.txt`. The game reads
the same preference when started directly. It applies on the next launch.
Language does not select a channel or change citizenship.

Missing, malformed or oversized preferences default to English. Surrounding
ASCII whitespace is accepted. Arbitrary preference values never become paths.
A save failure preserves the previous preference, restores the selector and
blocks Start with an error. Updates preserve `lang.txt`: it is excluded from
native patch generation. The older source launcher's unused manifest parser
also rejects this preference as a replace target.

The operational `WYDLauncher.exe` remains the adapted OnGame native launcher.
Its main dialog now includes a language dropdown below the original controls.
A source-built `WYDLauncherLanguage.dll` handles that dropdown through a
current-thread activation hook and an ANSI-compatible window subclass. It
reads the preference relative to the launcher executable, atomically saves on
selection, and saves again before forwarding Start to the original handler.
Window destruction restores the original procedure and releases its state.
Settings, browser/site content, ZIP updates and game launch remain native.

The extension is loaded by an added import and a register-preserving entry
shim, which calls its initializer before returning to the original CRT entry
at RVA `0x12C59`. Initialization failure exits before the patcher starts;
a missing DLL is rejected by the Windows loader. The pinned baseline has a
fixed image base and no relocation directory. The builder rejects an unknown
hash, ASLR image, changed entry, or different main-dialog layout instead of
guessing addresses. This is a local UI modernization, not a claim of native
7.48 language-selector parity.

The game initializes its asset root before resolving language resources. The
normal root is the executable directory; the existing development override
`WYD_ASSET_ROOT` also scopes the preference. An explicit text allowlist resolves
PT to EN to the canonical resource when a file is absent. English never falls
back to PT. This is a missing-file fallback, not a general malformed-file
recovery mechanism; existing loaders retain their validation behavior.

## Catalog and custom content preservation

Canonical `ItemList.bin`, `Itemname.bin`, `ItemListWide.csv`, `SkillData.bin`
and all item assets remain shared and unchanged by localization generation.
The localized ItemList keeps its native 140-byte records and checksum because
the existing file format requires them. Only its 64-byte name fields are used:
the loader validates every other byte against the canonical catalog before
publishing any translated name. A mismatched Itemname companion, invalid
checksum, missing terminator or changed gameplay byte rejects the whole name
overlay and retains canonical names. Localized records cannot supply stats,
models, requirements, positions or effects to the game.

The importer compares historical item identity fields before reusing names or
help. It imports 2,859 historical name entries, with 242 explicit PT aliases
covering current additions and reassigned IDs, including KR items. Proper
names and established identifiers remain unchanged. Five EN aliases remove
remaining Portuguese labels from current costume names. All 6,500 IDs have
matching localized ItemList and Itemname records.

Current custom help is the authority for values. Explicit translations preserve
the current Lac behavior, Repliction +6/+9 rules, custom commands, stack and
Combat Control instructions, store prices, quest coordinates and skill
formulas. Historical-only item-help IDs are not introduced. `SkillData.bin`
contains gameplay records rather than skill prose and is deliberately shared;
the 102 skill descriptions are localized through `skillhelp.dat`.

Each language currently has 30 files: ItemList/Itemname names, item/skill help,
indexed UI strings, message/effect tables, character descriptions, interface
and command help, mix/store descriptions, portal labels, quest subjects/pages,
and a caption sidecar. The sidecar translates 259 nonempty button/text captions
without copying or modifying binary scene layouts. Absent optional canonical
tables are not invented. No geometry, texture or item asset is imported from
the historical package.

## Reproducible updates

The historical Portuguese installation is an explicit read-only input and is
not a release payload. Generate from the repository root:

```powershell
python -B tools/client-assets/build_client_locales.py --source 'references/client748/WYD 7.48 PORTUGUES'
python -B tools/client-assets/build_client_locales.py --source 'references/client748/WYD 7.48 PORTUGUES' --check
python -B tools/client-assets/test_client_locales.py
pwsh -NoProfile -File tmproject/Build-Client.ps1 -NoDeploy
pwsh -NoProfile -File tools/client-assets/Build-NativeLauncherLanguage.ps1 -NoDeploy
```

Omit `-NoDeploy` on the native launcher command to install the tested EXE and
DLL into `client748`. Its first run preserves the pinned operational launcher
under `tmproject/build/native-launcher/base/`; subsequent builds reuse that
input. `-BaseLauncher` accepts an existing backup with the exact recorded hash.
The older native branding builder's `references/OnGame/WYD/` input is currently
absent; rebuilding this extension does not depend on that missing collection.

Translation decisions live in
`tools/client-assets/client_locale_overrides.json`. Hash guards require review
when the canonical catalog, name table, help or other translated text changes.
Update translations and compatibility tests before updating those guards;
do not merely accept a new hash. Generation writes a resource hash/size audit
to ignored `build/client-locales/resource-audit.json`. Resources retain native
Windows-1252 text encoding and existing IDs, row counts and checksums.

Classification: `EXTENSAO_COORDENADA` for language selection and text routing.
The formats reuse recorded native boundaries in the
[resource compatibility matrix](resource-compatibility-matrix.md) and
[skill description contract](../../.agents/research/client748/flows/ui/skill-description-text.md).
No packet or authoritative game-state contract changes in this batch.

## Validation and remaining delivery

Local installation (2026-10-23): `tmproject/client748/WYD.exe` matches the game
candidate hash below. Both language folders pass regeneration verification.
`lang.txt` currently selects `PT`; the initial installation used the documented
English default when that preference was absent.
This client ships as the 7.48.001 full client: handshake contract v2
(`0x02EC0001`), `[VERSION] 48001` and a launcher showing 7.48.001
([client build admission](../login-dispatch.md)). It is distributed as a full
client, not a patch; the 100xx patch history is retired.

Use `tmproject/client748/WYDLauncher.exe` with its adjacent
`WYDLauncherLanguage.dll`. The native update entry point and site feed remain
unchanged. The language selector previously added to the obsolete source
launcher's `Main.cpp` has been removed. That launcher's separate
`manifest.txt` protocol is not part of this delivery. There is no alternate
multilingual launcher executable in the runtime folder. Direct launches of
`WYD.exe` still read `lang.txt`.

The previous executables are retained for rollback under
`build/client-locales/rollback-local-20261023T162633/`; a checksummed installation
game installation receipt is in `build/client-locales/local-install.json`.
The latest native launcher receipt is
`build/client-locales/native-launcher-install.json`. Its verified baseline is
`A60DCA35B68DBE36ECE57541917A433F48282518561E2F7E89282C50F3E2E901`;
the native launcher build directory also retains installation rollback copies.

- Sixteen Python tests cover resource inventory, all gameplay bytes, name
  table identity, checksums, format placeholders, quest keys/page boundaries,
  selected numerical rules, current custom help, captions and stale-input
  rejection. Regeneration check passes with zero importer fallbacks reported;
  that count describes this importer, not complete game translation.
- The full NoDeploy build passed: ArchitectureTests 3,171,529 checks,
  SocketReceiveTests 281, CrashCaptureTests 200 and launcher tests 58.
  C++ integration tests decode both shipped language packs through the real
  loaders. This is automated validation, not `CLIENT_TESTED`.
- Client candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `2A2FD180017F8CEE5F151B329EF010C803B8CC0EBD70BFDB80B6DB0AD8CCEECB`.
- Native launcher candidate: `tmproject/build/native-launcher/Release/WYDLauncher.exe`,
  SHA-256 `EEFD20961D78122157915A3AB21B7EA920B3B8596CDDF0EC87FD024114A02180`.
- Native language extension: `tmproject/build/native-launcher/Release/WYDLauncherLanguage.dll`,
  SHA-256 `003134ABA581FD9802FD0DDFA8D2048D7EA834E14043ED6B8878377E8666BF59`.
- Native language tests passed with zero failures: real Win32 controls,
  atomic preference saving, reopening, Start forwarding, hook activation,
  DLL loading and undecorated export resolution. Six Python tests passed for
  original sections/imports, branch targets, control/resource preservation,
  section permissions, baseline rejection and Windows-loader startup. The
  disposable startup fixture exits before running the native update flow;
  it does not prove a real download or gameplay. Original `.text`, `.rdata`
  and `.data` are byte-identical to the operational baseline.

The reported Help/Command screen bypassed localization because
`TMFieldScene::LoadMsgText` hides the already-localized base-scene loader.
Both entry points now resolve the selected language before opening help text;
the existing color prefixes and list parser remain unchanged. The existing
Portuguese command file supplies that screen without duplicating shared assets.
Nine indexed Portuguese captions were shortened to fit the character and skill
panels, including UIString 195 (skill points). GDI measurements using 12-pixel
Tahoma at normal and bold weights fit the available label space. The sixteen
resource tests, regeneration check and full client build passed again after
this correction. The new game executable is installed locally; its receipt is
`build/client-locales/help-layout-install.json`. The screenshots establish the
previous defects, while visual confirmation of this corrected build remains
pending. Neither the native launcher nor the server was changed by this fix.

### Caption legibility regression, October 24

The October 23 overflow implementation was published in release 7.48.002.
The owner subsequently reported missing/unreadable glyph pixels in Character,
Skill and HUD labels at 1024x768. Both the installed local runtime and the
published executable were identified by SHA-256
`80CA48E77A8F83535401B7FE1415E33C9DE873E04F55C0313C844DF7B6A66209`.
This identifies the installed artifact; it does not establish the executable
path of every running process or the exact executable on the reporter's PC.

Root cause: the previous correction bound every eligible panel-owned `SText`
and shrank its already-rasterized texture to fit the intersection of resource,
neighbor and artwork bounds. The text render state uses point sampling, which
discards thin glyph pixels during fractional reduction. Some serialized
rectangles and artwork tabs are much narrower than their captions. Passing
geometry-only checks did not establish legibility and did not prevent this
regression.

The replacement is `MODERNIZACAO_COMPATIVEL`: retain the existing font scale,
render complete glyphs, and use an ellipsis only for actual overflow. The
source text remains intact for getters, recoloring, text replacement and
restoration when a bound grows or is removed. Texture rebuilding reuses the
stored color and zero-marking mode rather than applying brightness twice.
Unchanged bounds do not rebuild the texture every frame.

Bounds now opt in only for Character resistance/stat/mastery captions
(parent 513, IDs 5768..5785, plus identity/attributes/kingdom artwork titles),
skill points (1905/5786), four skill-tree titles (1905/5788..5791), and three
skill-master titles (1889/6050..6052). Numeric values, player names, HUD labels
and unrelated panels retain their ordinary layout. Existing twelve artwork
bands, panel edges, neighbor separation and shadow allowance still apply to
the opted-in captions. Geometry, hitboxes, shared assets, localization resources,
launcher behavior and the wire contract are unchanged.

Validation: `Build-Client.ps1 -NoDeploy` passed 3,179,319 architecture checks
and static assertions, 281 socket checks, 200 crash checks and 58 launcher
checks. Real Tahoma GDI tests at four UI scales and weights 500/700 compare
fitting text masks pixel-for-pixel, including accents and texture chunk seams.
They also cover original-size overflow with ellipsis, all twelve title bands,
caption scoping and restoration from the complete source. These tests do not
execute the game's Direct3D UI. In-game legibility, shortened title usability,
class changes, panel reopen and resolution changes remain pending. This fix
is not `CLIENT_TESTED` and does not close unrelated skill-icon issues.

- Initial local candidate, superseded by the published revision-3 rebuild:
  `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `627A97DF444BDEC0847CD947B00B297878373E8FE5EC9F6835BD3643A45F1FD6`.
- Installed runtime: `tmproject/client748/WYD.exe`, unchanged during this fix,
  SHA-256 `80CA48E77A8F83535401B7FE1415E33C9DE873E04F55C0313C844DF7B6A66209`.
- The initial local check retained revision 2 and did not install or publish.
  The subsequent revision-3 rebuild was published as patch 48003 before the
  paired server activation. Published executable SHA-256:
  `882F11E6CBA65E6B290DA0C8DF7920485982F5A9F1E49005D31823D76881BBE6`.
  See the [release receipt](launcher.md#published-patch-748003-2026-10-24).
  No local install, commit or push occurred; visual gameplay remains pending.

The focused native evidence validator passes. Automatic execution review
blocked the cleanup command as `blocked by policy`; it was not retried through
another mechanism. Task leftovers are `build/ui-text-regression/` (build log)
and `tmproject/build/tests/Release/crash-capture-tests-2560-5998234/` (isolated
crash-test output). The incremental candidate remains available above.

### Native caption comparison, October 25

The owner rejected the revision-3 ellipses in the character and skill panels.
Native 7.48 Ghidra evidence confirms Tahoma 13, weight 500 in mode 2 and full
text drawing beyond decorative title notches. The previous fixed artwork bands
were narrower than the original names. A second constraint mistook a one-pixel
rectangle overlap with the next row for a same-row text collision.

The local correction removes those title bands, preserves the native font and
alignment, and distinguishes adjacent rows. Character stat/resistance/mastery
labels use a two-pixel inset and stop before the following value's actual glyph
origin. This fits complete EN/PT mastery names without moving values, shrinking
glyphs or replacing shared artwork. Ellipsis remains a fallback for actual
overflow; source bytes, locale selection, resources, hitboxes, launcher and wire
contracts are unchanged. Classification: `MODERNIZACAO_COMPATIVEL`. See the
[native evidence and measurements](../../.agents/research/client748/flows/ui/field-ui-scale-and-anchors.md#complete-native-captions-october-25).

Validation: the NoDeploy gate passed 3,176,709 architecture checks/static
assertions, 281 socket checks, 200 crash checks and 58 launcher checks. Real
GDI tests require complete representative EN/PT names with identical glyph
masks at scales 1/1.25/1.5/2, and cover the identity/name-row collision. The
final incremental solution build passed. These results do not execute the
game's Direct3D UI. Actual visual acceptance, all classes, panel reopen,
language changes, relogin and resolution changes remain pending.
The 17 locale resource tests, focused native-record validator and
`git diff --check` also passed.

- Local candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
  `1144368EED4C048C58335DC45A7837F017A10FCCBCECFD1E34FDBFA93A343A08`.
- Installed runtime: `tmproject/client748/WYD.exe`, SHA-256
  `882F11E6CBA65E6B290DA0C8DF7920485982F5A9F1E49005D31823D76881BBE6`.
- Source and candidate only: no local installation, patch publication, VPS
  action, version change, commit or push. Revision 3 remains the last published
  identity. Future publication requires the next paired client/server revision.

Automatic execution review rejected cleanup with `blocked by policy` before
the command ran. No alternate deletion mechanism was attempted. The new
task-only leftovers are:

- `C:/Users/xereca/AppData/Local/Temp/wyd-caption-native-20261025/`
- `tmproject/build/tests/Release/crash-capture-tests-17592-33167218/`

Hardcoded English strings, server-supplied messages and legacy untranslated
content in the canonical English corpus remain outside this resource batch.
Some legacy English resources also contain GBK-derived text requiring a
separate editorial audit. Native font rendering, accents, clipping, launcher
selection, direct launch, tooltips, quests and relaunch persistence still need
observed gameplay validation using the exact candidate and resource hashes.
No complete-translation or global-release readiness claim is made.

At that time, the repository-layout check failed on the installation's
`WYD.log` at its former path and runtime crash files. The full local
installation now resides in `references/client748/WYD 7.48 PORTUGUES/`.
Current crash logs reside in `client748/dumps/`. The earlier check result is
historical; this note does not claim that check passed.
The installation preference `client748/lang.txt` is explicitly allowed by both
the ignore rules and layout validator.
Cleanup of this run's crash-test evidence was blocked by execution policy.
The retained directory is
`tmproject/build/tests/Release/crash-capture-tests-50152-1128362296`.
The Help/layout rebuild also retained
`tmproject/build/tests/Release/crash-capture-tests-35276-1129819921`
after its cleanup command was rejected by execution policy.
No alternate deletion method was attempted.

The adaptive-caption builds also left the following test-only evidence after
their cleanup command was rejected by execution policy. No alternate deletion
method was attempted. Incremental build outputs remain in their normal location.

- `tmproject/build/tests/Release/crash-capture-tests-32816-1146126156`
- `tmproject/build/tests/Release/crash-capture-tests-48244-1146385578`
- `tmproject/build/tests/Release/crash-capture-tests-25516-1146469843`

The title-tab follow-up cleanup was also rejected by execution policy. No
alternate deletion method was attempted. Its remaining temporary output is:

- `tmproject/build/tests/Release/crash-capture-tests-29692-1147563671`
- `C:/Users/xereca/AppData/Local/Temp/wyd-caption-panels.png`
- `C:/Users/xereca/AppData/Local/Temp/wyd-other-caption-panels.png`

Build and research-validation diagnostics remain in `build/ui-text-fit/`.

The 7.48.001 full client (game, both resource folders, launcher EXE and its DLL)
established the published baseline and reset feed. Patch 7.48.002 and its server
gate are now active, as recorded in the
[launcher release receipt](launcher.md#published-patch-748002-2026-10-24). The legacy patch
installer cannot replace its own running executable or loaded DLL, and the
patch builder refuses both files. The obsolete source launcher's separate
manifest/self-update flow remains outside the operational launcher contract.

## October 23 audit corrections

These changes are `MODERNIZACAO_COMPATIVEL`: existing wire keywords,
opcodes, message indices, resource encodings, file selection and gameplay
state representations remain unchanged. They do not establish complete
translation or global-release readiness.

- Font drawing and geometry now explicitly decode Windows-1252 bytes for
  Unicode GDI calls, including text-control alignment and tooltip width.
  Font selection uses an explicit ANSI character set. Accented single-byte
  letters no longer pass through DBCS chunk splitting. The saved native
  `FUN_00421b8f` establishes the shared draw/measure boundary, not a native
  Unicode implementation. The existing
  [UI evidence record](../../.agents/research/client748/flows/ui/field-ui-scale-and-anchors.md)
  records that distinction and the automated coverage.
- Quest subject loading validates the numeric prefix and destination range
  before writing. The saved native `FUN_0049ec64` and caller `FUN_00441823`
  establish the ten-byte prefix, 500 entries, one-based indices, type bytes
  and overwrite order. Valid inputs keep those meanings; malformed inputs
  no longer write outside the table. See the
  [quest loading contract](../../.agents/research/client748/flows/ui/quest-subject-level-loading.md).
- Canonical and EN/PT command help now describes `--` as global chat,
  matching the authoritative broadcast path. Routing is unchanged.
  Canonical help also replaces broken historical multibyte punctuation with
  ASCII. Generator policy and its content hash were updated together.
- The server reserves `getout`, `handover`, `war`, `item_lock`, `item_unlock`
  and the existing localized aliases before nickname lookup in both chat
  packet paths. They return an explicit unavailable notice instead of
  accidentally whispering command arguments to a matching online nickname.
  Every guild rank receives the owner-required guild rejection first when
  attempting citizenship departure. Client message 391 is named
  `CitizenshipLeave` internally; its index and `getout` wire token remain
  unchanged.

The reservation is not an implementation of citizenship departure, guild
rank handover, legacy war operations or item locking. Those gameplay
operations remain pending. The official server decompilation used for the
command comparison is **7.59**, not proof of 7.48 parity. Its saved
`FUN_00519CA0` includes citizenship departure and handover branches, including
fees, rank checks, state changes and scheduling restrictions. Some field
meanings and the complete persistence/login integration remain unresolved;
the correction does not invent them or silently change fame or gold. The
owner's prohibition on citizenship departure while in any guild takes
precedence over historical leader-specific behavior. See the
[channel plan](../bilingual-channels-plan.md) and
[server gap analysis](<../../references/decompilacao server/dlls/rea-analysis/maps/server-759-gap-analysis.md>).

Validation completed:

- `Build-Client.ps1 -NoDeploy`: 3,176,611 architecture checks and static
  assertions, 281 socket checks, 200 crash checks and 58 launcher checks.
- `LocaleBoundaryTests`: independent Unicode GDI expectations, invalid-input
  handling, quest-table guards and native valid indexing/overwrite behavior.
- Python locale suite: 17 tests passed. Generator check: 60 text resources,
  2,859 item names and zero documented fallbacks. This is generation coverage,
  not evidence that every legacy sentence is translated or correct.
- `go test ./internal/game ./internal/wire`: passed. New command regressions
  cover both incoming chat paths, uppercase keywords, colliding nicknames,
  all ten guild-rank values and preservation of account/counter state.
- `git diff --check`: passed. Existing Winsock deprecation warnings remain
  outside this correction.

Candidate: `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`D5A36487DA7938E845B58FD91B697E412EE20301251F284502E3CA2F4D549085`.
The previous candidate identity above is superseded. The installed game
executable remains SHA-256
`D29873E2DFA5528E432A164CC007D6DFE729A20D31C3902F3986542D76B7BB3A`.
Canonical and EN/PT command resources in `client748/` were updated; no
candidate EXE or launcher binary was installed. Operational launcher EXE
and language DLL hashes remain
`85C9C648B389BE91CA348351590FCBEFA6752A4D4E849AFBBCB1F054CA12FDE2`
and `003134ABA581FD9802FD0DDFA8D2048D7EA834E14043ED6B8878377E8666BF59`.

The audit candidate above was superseded for publication by revision 2,
SHA-256 `80CA48E77A8F83535401B7FE1415E33C9DE873E04F55C0313C844DF7B6A66209`.
Patch 7.48.002 contains that executable and the three command-help resources;
it was publicly verified before server activation. The website now announces
the mandatory compiled-version rule. No commit/push or local executable
installation was performed. Actual client execution, non-Western Windows ACP
validation and an observed launcher-to-game update flow remain pending.

Cleanup of the new crash-test evidence was rejected by execution policy.
The retained directory is
`tmproject/build/tests/Release/crash-capture-tests-3516-1150995812`.
No alternate deletion method was attempted. This cleanup limitation does
not invalidate the successful build or imply publication.
