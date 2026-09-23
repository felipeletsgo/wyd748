# English-language migration

The project targets the global, English-language WYD client. English is the
only language for new or edited human-readable content. The repository is not
yet fully migrated; this document tracks the remaining work without treating
a partial translation as completion.

## Completed entry points

- Root repository rules and all active skill entry points.
- The emulator and cross-cutting contract references in the server feature
  skill are in English. The latter no longer permits Portuguese logs or
  comments; both now follow the repository-wide rule. Its session-continuity
  reference is also in English.
- Root README and documentation README.
- Generated documentation inventory labels and layout-validator messages.
- `tmproject/Build-Client.ps1` help, diagnostics, and validation status.
- The staff portal's static and dynamic UI text, including forms, event controls,
  moderation, teleportation, status messages, and English date/number formatting.
- The in-source 7.48 client CC-mode control labels, including the compact
  auto-combat panel modes and positions, the item position, and the selected
  server endpoint diagnostic. The compact panel retains its existing control
  IDs, textures, and state mapping; in-client visual validation remains pending.
- The runtime `tmproject/client748/sn.bin` group labels: `Channel` and
  `Featured` replace the Portuguese and Korean labels while retaining all
  eleven fixed-width slots, group orders, and the 143-byte file layout.
- Character-selection preview loading now validates the original four-row
  `selchar.txt` resource before item lookups; its new diagnostic and adjacent
  source comments are in English. Visual client validation remains pending.
- Effect-name loading now bounds every fixed-width row and rejects malformed
  assets without publishing partial data. The shipped English 7.48 names and
  their indexes are preserved; the optional subeffect file is not shipped.
  The adjacent Basedef documentation is now in English. In-client validation
  remains pending.
- `UIString.txt` loading now preserves labels containing spaces and rejects
  malformed or out-of-range rows without publishing a partial table. The
  original resource bytes and numeric IDs remain unchanged. In-client
  validation remains pending.
- `Itemname.bin` loading now validates 68-byte records, indexes, and duplicates
  before overlaying decoded names onto the 7.48 item table. The shipped 2,883
  records, resource bytes, item IDs, and non-name fields remain unchanged.
  Legacy Portuguese comments in the affected `Basedef.cpp` were translated;
  in-client validation remains pending.
- The main [7.48 client port guide](client/port-748.md),
  [score contract](SCORE.md), and [build guide](build-and-integration.md), as
  well as the client diagnostics interface comments, are in English. This was
  a prose-only client change; no client build or visual validation is implied.
- Account-registration API responses, validation errors, logs, and adjacent
  tests are in English. JSON field names and HTTP status codes are unchanged;
  focused account and account-API tests cover the translated validation text.
- Native packet-framing and client-integrity diagnostics in `internal/wire`,
  their adjacent comments, and focused test output are in English. Sentinel
  errors and packet bytes are unchanged; the wire package tests pass.
- The architecture map, login/character-selection dispatch guide, shared
  packet-fixture guide, upstream provenance note, and Windows development
  guide have been translated. These are prose-only changes and do not imply
  client, server, or visual validation.
- The water-macro overview, boss configuration guide, and official WYD2 guide
  index and URL map are now in English. Existing source URLs, document paths,
  NPC and skill-data identifiers, and boss example filenames remain unchanged.
  These are documentation-only changes.
- The official quest and event technical summary is now in English. Historical
  values, item names, the `/qst` command, and source URLs were preserved; this
  translation does not establish native 7.48 gameplay parity.
- The guild-war guide now describes the current schedule, collector contract,
  economy, and remaining real-client gate in English. Session-specific build
  and test history was removed from that guide; no gameplay contract changed.
- The opcode and wire-contract catalog is now in English. Opcode values,
  packet sizes, offsets, and contract identifiers were preserved; this
  translation does not add protocol coverage or client validation.
- The web-admin starter and client-asset audit scripts now use English help,
  comments, and diagnostics. Their switches, paths, and behavior are unchanged.
- Ghost-shop and player-trade errors shown to players, diagnostics, and adjacent
  source comments and tests are in English. Focused automated tests cover title
  validation, trade-item rejection, purchase errors, and the existing flows.
  Packet layouts and game-state logic are unchanged; no client visual validation
  is implied.
- Chat-command channel-switch rejection and character-information labels shown
  to players are in English, as are the adjacent server diagnostics, comments,
  and test descriptions. Existing command aliases, packet layouts, and state
  transitions are unchanged; in-client visual validation remains pending.
- The neutral kingdom display name is now `Neutral`; the numeric kingdom IDs,
  cape mappings, and native conversion rules are unchanged. A focused model
  test guards the English label.
- The cargo-slot, character-login use-case, and grid-insertion test descriptions,
  plus the adjacent cargo and login application comments, are in English.
  Release architecture tests pass; this prose-only batch changes no client
  behavior or packet contract.
- The native server entry point and load-test command now use English for
  flag help, startup and failure diagnostics, and adjacent comments. Flag
  names, default values, configuration keys, and server behavior are unchanged.
- The official item-reference guide and the inert demo-scene comments are now
  in English. The shared client/server structure header's introductory comment
  is also in English. This prose-only translation changes no item IDs, wire
  layouts, resource files, or runtime behavior.
- The TMProject target-architecture guide is now in English, including its
  transport boundary, ownership, validation scope, and outstanding send-result
  debt. No source, wire contract, or runtime asset changed in that batch.
- The server operations guide now uses the current English staff-panel labels
  and README anchors. Three client wire-header comments are also in English;
  packet layouts and opcodes are unchanged.
- The 7.48 research-record template, its maintenance reference, validator
  diagnostics, and validator tests now use English. Existing records with
  legacy section names remain readable during migration. The skill-belt
  page-selection record is in English and no longer lists already-handled
  insertion rejection as an open rollback bug; real-client testing remains
  pending. The adjacent grid-insertion and SGrid comments are in English.
- The cube-box visual removal path now releases the detached visual after
  clearing interaction aliases. A source-contract check protects that
  ownership ordering; no client execution is implied.

## Remaining work

1. Audit client-visible strings, resource text, and error paths in
   `tmproject/TMProject748/` and `tmproject/client748/`. Translate prose without
   changing resource IDs, binary formats, or protocol payloads. Validate
   affected UI flows in the actual 7.48 client when client testing is possible.
   `tmproject/client748/SkillData.csv` is a local, Git-ignored audit input,
   not the runtime skill loader input; its labels are not evidence for skill
   semantics or in-client language. Keep numeric IDs and parameters intact.
2. Audit remaining server-facing and administrator-facing messages in
   `wydgo748/`, then update corresponding tests and API documentation together.
3. Translate active guides in `DOCS/`, including their headings, examples, and
   cross-links. Keep established paths stable unless every reference is updated.
4. Translate scripts, diagnostics, and test descriptions in `tools/`,
   `tmproject/`, and `wydgo748/` as each affected workflow is validated.
5. Translate `.agents/research/` and `.agents/handoffs/` where they are still
   used for current decisions; preserve technical evidence and provenance.
6. Translate the remaining Portuguese prose in active skill references. A
   translated skill entry point does not imply that all its supporting
   references have been migrated.
7. Review legacy Portuguese chat-command aliases before an English-only
   release. Removing an established command spelling changes behavior, so
   decide the compatibility policy and update command tests together.

Code identifiers, persisted values, packet bytes, resource keys, and fixture
data require compatibility review before renaming. English prose is the goal;
blind replacement of contract-bearing values is not.

Completion requires a repository-wide review of authored text plus validation
of affected client and server flows. This includes the client resources,
server output, active documentation, scripts, tests, research records, and
handoffs; none of these is exempt as historical migration debt. A keyword
search alone cannot distinguish Portuguese prose from names, examples, and
protocol data. The migration is still incomplete.
Do not declare a global client/server release ready while known Portuguese
player-facing text remains.
