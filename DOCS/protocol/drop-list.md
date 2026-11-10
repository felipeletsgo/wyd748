# In-game drop search

Classification: `EXTENSAO_COORDENADA`, contract version 1. This feature has no claimed native 7.48 equivalent.

## Ownership and UI

Press D outside editable controls to toggle the search window. D replaces its previous alternate C.C. shortcut.
Reuse FieldScene2 background 12848, texture set 444, and existing text, edit and button widgets.
The Field scene owns every control. Close and Escape hide the panel and release its edit focus.
While the panel is visible, suppress overhead character names, shop signs, health bars and ground-item labels; these world controls otherwise draw above the modal.
Scene destruction owns cleanup. No thread, HTTP request or persistent state belongs to this feature.

Search by monster name or item name/ID. The client resolves localized item names into a bounded item-ID mask.
The server owns catalog relationships, probabilities, bonus calculations and paging. Results never grant items.
Only templates referenced by enabled spawn sources appear; files that remain under `data/npcs` without a spawn reference stay hidden.
Bosses use their boss catalog; their base template is not shown as an ordinary monster unless another source also spawns it.
Names and input use Windows-1252 on the wire; UI captions use EN/PT locale rows with English fallback.
The mode tabs show the active search and a matching prompt. The result area labels base chance and chance with the player's bonus.
The player's bonus remains visible when the search has no matches. Previous and Next are disabled outside available pages.

## Wire v1

All integers use little endian. Requests use opcode `0x7F41`; responses use `0x7F42`.
Both opcodes are reserved for this feature and must not serve other directions.

Request size: 904 bytes. Header occupies bytes 0–11.

| Offset | Type | Value |
| --- | --- | --- |
| 12 | uint16 | Version 1 |
| 14 | uint8 | Mode: 0 monster, 1 item |
| 15 | uint8 | Reserved zero |
| 16 | uint32 | Nonzero request ID |
| 20 | uint32 | Result offset, multiple of 10 |
| 24 | char[64] | NUL-terminated query, zero padding |
| 88 | uint8[816] | Item mask: bit N selects catalog item N; bits 6500–6527 remain zero |

Response size: 716 bytes. Header ID is Field scene 0x7530.

| Offset | Type | Value |
| --- | --- | --- |
| 12 | uint16 | Version 1 |
| 14 | uint8 | Echo mode |
| 15 | uint8 | Status: 0 success, 1 throttled |
| 16 | uint32 | Echo request ID |
| 20 | uint32 | Effective result offset |
| 24 | uint32 | Total matching relationships |
| 28 | int32 | Current drop bonus percent |
| 32 | uint16 | Row count, 0–10 |
| 34 | uint16 | Reserved zero |
| 36 | Row[10] | Unused rows are zero |

Each 68-byte row has item ID uint16 at 0, origin flags uint16 at 2,
reserved uint32 at 4, base chance uint32 at 8, current chance uint32 at 12,
reserved uint32 at 16, and NUL-terminated monster name char[48] at 20.
Chance units range from 0 to 1,000,000,000. Origin flags: 1 carry, 2 boss, 4 active global event, 8 shared boss area reward.

## Rules and failures

Read-only queries require an admitted in-world character. Accept at most one query per character every 250 ms.
Malformed frames, versions, padding, masks and offsets are rejected before searching.
The client ignores responses for hidden panels, old request IDs or another mode. A seven-second timeout permits retry.
Queries have no economic mutation, persistence write or gameplay-space traversal.

For each monster/item pair, combine independent slot rolls as `1 - product(1 - probability)`.
Apply `model.DropRateForSlot` and the exact `applyDropBonus` function before combining current chances.
The bonus changes ordinary one-in-N rates with `floor(rate * 100 / (100 + bonus))`; it is not a percentage-point addition or a direct multiplier of the final chance.
The displayed bonus is the character's total citizenship, equipped-gem and applicable skill bonus at query time.
Boss rows use their own configured table, without inheriting ordinary template carry.
Boss configured and global-event fixed chances retain their configured percentages. The personal bonus changes ordinary rate-based rolls only.
If a row combines ordinary and fixed chances, only the ordinary portion changes with the bonus.
Templates need an enabled spawn source. A referenced monster remains searchable while dead or waiting for its next spawn.
EXP-only instances suppress loot; this table does not promise eligibility.
The table describes configured item chances, not refinement options, quest rewards or direct mob gold. Shared area rewards have a separate conditional row.
Build the relationship index before the World starts. Live NPC definition changes invalidate it.
Cache event relationships separately. Each query allocates only one result page.
Event state and character bonuses are read for every query. Search again after changing equipment or bonuses.
Monster searches accept the visible label and the configured template name. Item searches accept localized names and numeric IDs.

## Delivery and validation

Release 7.48.101 was published as mandatory patch 48101 and activated on both game channels on 2026-11-08. The full-client baseline remains 7.48.100 on Google Drive; the site advertises the launcher update. See the [activation receipt](../../.agents/research/release-748101-20261108.json).
Validate both parsers, paging, stale replies, duplicate-slot probability, live reload, bonus changes and read-only state.
Client execution, EN/PT layout, focus, Escape, movement blocking and relogin acceptance remain pending until observed.

## Implementation and check results

- [Server search and probability index](../../wydgo748/internal/game/drop_list.go).
- [Server wire contract](../../wydgo748/internal/wire/drop_list.go).
- [Client window and lifecycle](../../tmproject/TMProject748/internal/app/scenes/TMFieldSceneDropList.cpp).
- [Client parser and pending-request state](../../tmproject/TMProject748/internal/wire/DropListPacket.h).
- EN/PT captions occupy UIString rows 267–289. Row 231 remains reserved.
- The Help page now advertises D for drop search; P remains the party shortcut.

`STATICALLY VERIFIED`: existing quest-window skin, unique control IDs, exact wire sizes,
server-derived chances, bounded pages, cache invalidation and source-only ownership.

`AUTOMATED TESTED`: full Go suite and vet; focused race tests; probability, paging,
localized item masks, visible monster names, boss-specific tables, fixed rewards,
active-event expiry, per-channel cache invalidation and character-runtime reset.
The client gate passed 3,202,296 architecture checks, 295 socket checks and 148 crash-capture checks.
All 22 locale tests passed. The final boss-coverage check is recorded in `build/drop-list-focused.log`.

Client patch 48101 is published at `/patch/patch-48101.zip`; its live SHA-256 is recorded in the [release receipt](../../.agents/research/release-748101-20261108.json). The active server binary and Linux release hashes are recorded in that receipt. Logs remain in ignored `build/` and `tmproject/build/` task outputs.

## Published client and search-index correction 7.48.102

The supplied classic WYD screenshot shows a monster list, selected-monster details and an item grid. This remains a visual reference. The published search view keeps reverse item lookup and explicit chance columns, using the existing FieldScene skin.
The review makes the selected mode explicit, explains the search term for each mode, shows the personal drop bonus separately from page counts, and disables unavailable page controls.
Search results now use actual spawn references from NPCGener, boss summons, event and instance configurations. Orphaned NPC files no longer appear as ordinary monsters or global-event sources.
The table describes each item's chance of at least one drop per eligible kill. Fixed boss and event chances are shown separately from the player's bonus semantics.
The 7.48.102 Release client build passed with SHA-256 `187598EEA5C6FCE89EFC8CBD45237601574FFC6C19BB7F68660A1E21C4329630`. The full Go suite, `go vet ./...`, and 22 EN/PT locale tests passed. Patch 48102 is published, and release `20261108T123541Z` enforces minimum revision 102. Both channels restarted. Public archive, payload, running-executable and admission checks passed. The owner's later screenshots rejected label occlusion, panel size and footer placement. See the [activation receipt](../../.agents/research/release-748102-20261108.json).

`CLIENT-TESTED`: pending. Test D with chat and search focus, monster/item modes,
EN/PT names with accents, numeric item IDs, multiple pages, empty results, close/Escape,
late replies, relogin and movement while the modal is visible. Compare a common,
rare and guaranteed drop with the server configuration. Check conditional boss/event rows.
The patch is published and the server contract is active. The owner observed the remaining presentation defects below.

## November 8 follow-up — published 7.48.103

Classification: `MODERNIZACAO_COMPATIVEL`. Wire v1, resource IDs, drop rates and rewards remain unchanged.
The server minimum and compiled client revision advanced together to 103. Production now runs release `20261108T150126Z`; patch 48103 is published.

| Area | Evidence and correction |
| --- | --- |
| Human and NPC labels | Revision 102 guarded only `LabelPosition2`. Animated actors used `LabelPosition`. Both render branches now use one visibility gate. |
| First open | UI geometry is queued before entity rendering. Hide existing human and item labels when opening, before that queue pass. |
| Panel bounds | Replace the 700x460 frame with 620x360. Read actual status, minimap and lower-HUD bounds before positioning. |
| Unsupported scale | Reject a frame that cannot fit. Show localized guidance rather than place it over the HUD. |
| Footer and Close | Texture 444 crops CommonBox to 492x420. Align Close with artwork `(214,394)-(283,414)`. Keep notes above the content border. |
| Text bounds | Opt drop-search captions and its edit into measured ellipsis. Preserve source names and query bytes. |
| Active mode | Use independent child captions and a persistent background color. Avoid the legacy button caption offset. |
| Page availability | Dim disabled Previous and Next captions. Their controls continue to reject clicks. |
| Empty and failure states | Put loading, no-result, timeout and throttling feedback inside the empty table. Keep counts on the navigation row. |
| Request identity | Use a monotonic sequence across close/reset. A same-tick reopen cannot accept its predecessor's reply. |
| Search matching | Collapse and trim whitespace in client and server keys, including localized item-mask searches. |
| Close lifecycle | Explicitly clear edit focus in both keyboard modes; the generic focus setter rejects null in the alternate mode. |
| Protocol rejection | Existing size, version, mode, item, chance, row, padding, terminator and stale-response checks remain enforced. |
| Spawn eligibility | Existing index uses enabled spawn references, explicit boss tables and instance/event sources. Orphan template files stay excluded. |
| Probability semantics | Reuse authoritative slot and bonus rules. Combine independent rolls; fixed rewards retain their configured chance. |
| Event cache | Expiry and exhausted limits are checked per query. A new event replaces its identity; NPC edits invalidate relationships. |
| Ownership and economy | Controls belong to the scene. Queries change no inventory, gold, position or persistent state. |

Native evidence reused: [UI scale and anchors](../../.agents/research/client748/flows/ui/field-ui-scale-and-anchors.md)
and [Quest panel lifecycle](../../.agents/research/client748/flows/ui/quest-panel-layout-lifecycle.md).
These records support the existing layout and skin boundary. They do not establish native parity for the new drop list.

`STATICALLY VERIFIED`: the table above identifies code paths and unchanged contracts.
`AUTOMATED TESTED`: the incremental Release build with `-NoDeploy` passed, including architecture, socket and isolated crash-capture tests.
Client SHA-256: `23B40806A698BB143051FAF547FE9E68731372B2DB2AB68D109F5C46F561A8FD`.
GDI tests measured complete EN/PT headings, bonus and notes at UI scales 1, 1.25, 1.5 and 2.
They also checked ellipsis for a 63-byte query. Placement tests cover default scales at 800x600, 1024x768, 1280x720 and 1920x1080.
`go test ./internal/game -count=1`, `go test ./internal/wire -count=1`, and `go vet ./internal/game ./internal/wire` passed.
All 22 locale tests and the repository layout/link check passed. No database or economy contract changed; dedicated database/race tests were not rerun.
Logs: `tmproject/build/drop-list-103/client-build.log`, `tmproject/build/drop-list-103/layout-check.log`, and `build/drop-list-103-game-test.log`.
The Windows server build also passed. The saved client EXE/PDB pair and file hashes are in `tmproject/build/drop-list-103/`.
`validation.json` lists affected source, locale and documentation files. No files were removed.
`CLIENT-TESTED`: owner approved the installed local candidate before publication. No per-scenario capture was supplied; post-activation gameplay remains unobserved.
At audit completion, no game was launched, installed executable replaced, or follow-up release published.
The owner then authorized local installation. On November 8, `tmproject/client748/WYD.exe` was replaced with the verified candidate above.
The previous executable and its verified hash are preserved; `tmproject/build/drop-list-103/local-install.json` records both identities and the backup path.
EN/PT locale files match validated inputs. The agent did not launch the game. The owner then approved the local candidate and requested publication.

Patch 48103 was downloaded over HTTPS and all four entry hashes matched. The HTTP and HTTPS feeds agree.
The Linux server package verified 1,076 payload files; the site package verified 6,661 files. Portal checks, build and 116 tests passed.
Both game channels restarted under existing owner authority. API and log-reader process IDs, private configuration and schema v20 were preserved.
A fresh database backup passed checksum and archive-list checks. Revision 102 is rejected; revision 103 reaches credential validation on both channels.
The site manifest, download page and changelog advertise 7.48.103. Full clients remain externally hosted; news remains a draft.
See the [activation receipt](../../.agents/research/release-748103-20261108.json). Release 7.48.102 remains the rollback target.

Check 800x600, 1024x768 and a widescreen resolution in EN/PT. Test crowded shops, ground labels, first open,
Close/Escape, both keyboard modes, long names, a 63-byte query, whitespace, mode changes, multiple pages,
no results, timeout, rapid close/reopen, scene changes and relogin. Confirm labels return after close.
