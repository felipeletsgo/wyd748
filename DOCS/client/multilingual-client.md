# Shared multilingual client

Status: implemented in source and generated text resources; candidate builds
and automated validation passed on 2026-10-23. The game and production native
launcher with its language extension are installed locally. The launcher keeps
the site's existing `update.htm`/ZIP protocol. Not published or validated in
gameplay. This is the client portion of the
[bilingual channels plan](../bilingual-channels-plan.md); it does not implement
channels, citizenship, server translation, or guild-war isolation.

The owner's explicit request authorizes Brazilian Portuguese localization
data under `PT/`. Implementation, tooling and documentation remain English.
This exception does not establish completion of the global language migration.

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
python -B tools/client-assets/build_client_locales.py --source 'tmproject/client748/WYD 7.48 PORTUGUES'
python -B tools/client-assets/build_client_locales.py --source 'tmproject/client748/WYD 7.48 PORTUGUES' --check
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

Hardcoded English strings, server-supplied messages and legacy untranslated
content in the canonical English corpus remain outside this resource batch.
Some legacy English resources also contain GBK-derived text requiring a
separate editorial audit. Native font rendering, accents, clipping, launcher
selection, direct launch, tooltips, quests and relaunch persistence still need
observed gameplay validation using the exact candidate and resource hashes.
No complete-translation or global-release readiness claim is made.

The repository layout check still fails on the historical installation's
`tmproject/WYD 7.48 PORTUGUES/WYD.log` and runtime `client748/dumps/` outside
sanctioned build roots. Those runtime files are outside this change and were
preserved; this check is not reported as passed.
The installation preference `client748/lang.txt` is explicitly allowed by both
the ignore rules and layout validator.
Cleanup of this run's crash-test evidence was blocked by execution policy.
The retained directory is
`tmproject/build/tests/Release/crash-capture-tests-50152-1128362296`.
The Help/layout rebuild also retained
`tmproject/build/tests/Release/crash-capture-tests-35276-1129819921`
after its cleanup command was rejected by execution policy.
No alternate deletion method was attempted.

Publication remains a separate authorized step: distribute the 7.48.001 full
client (game, both resource folders, launcher EXE and its DLL) first, then
reset the feed and activate the server gate as described in the
[launcher release line](launcher.md#release-line-748). The legacy patch
installer cannot replace its own running executable or loaded DLL, and the
patch builder refuses both files. The obsolete source launcher's separate
manifest/self-update flow remains outside the operational launcher contract.
