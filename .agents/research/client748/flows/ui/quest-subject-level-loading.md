---
id: quest-subject-level-loading
title: Quest subject prefixes select entries in the field level table
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-23
---

# Quest subject prefixes select entries in the field level table

## Question

Which resource indices and bounds must localized quest subject loading preserve?

## Evidence boundary

- USED: native `references/client748/WYD.exe`, SHA-256 in front matter;
  saved Ghidra project `tmproject/build/native-research/WYD748.gpr` and
  decompilations `FUN_0049ec64.c` and `FUN_00441823.c` in its `decompiled/`
  corpus. Both exports declare the same native hash.
- USED: current `TMSceneResources.cpp::LoadMsgLevel`,
  `TMFieldSceneUI.cpp` initialization and `QuestSubjectLevel.h`.
- USED: canonical, EN and PT `UI/QuestSubjects*.txt` and `UI/QuestMessage.txt`;
  resource identities are recorded by `build_client_locales.py --check` in
  `build/client-locales/resource-audit.json`.
- NOT APPLICABLE: official 7.59 server and other historical server sources;
  this is a client file-to-table transition with no server operation.
- NOT APPLICABLE: later TMProject as native authority. Current source is used
  only to locate and adapt the corresponding boundary.

## Native 7.48 flow

### Callers

`FUN_00441823` clears 500 bytes at scene offset `0x276c0`, then invokes
`FUN_0049ec64` for QuestSubjects, QuestSubjects2, QuestSubjects3,
QuestSubjects4 and QuestMessage, in that order, with types 97, 98, 99, 101
and 100 respectively.

### Main function

`FUN_0049ec64` returns zero if opening fails. It reads at most 100 chunks
of 256 bytes, copies the first ten bytes into a zeroed prefix and parses
`%d %x`. It writes the supplied type into destination `[level - 1]`.
The native function does not validate successful conversion or table bounds.
Later rows and later files overwrite an earlier value at the same index.

### Callees

The saved export identifies `FUN_0058f716` as the file-opening boundary,
`FUN_00590a06` as the bounded line read, `FUN_005909d2` as the formatted
conversion and `FUN_0058f62c` as the close boundary, alongside `_memset`
and `_strncpy`. The table write is direct, not a callback or network send.

## State and lifecycle

The field scene owns the 500-byte table. Initialization clears it before all
five loads. The parser borrows its destination and owns no persistent state.
The file is closed after the bounded read loop. Reinitialization repeats
loading from cleared state. Owner/observer replication, logout persistence
and server teardown are N/A: this table is local UI metadata.

## Wire, ABI, and resources

No packet, persisted identifier or native object layout changes. The source
helper accepts the destination capacity explicitly. Valid levels 1 through
500 retain their one-based interpretation and existing type bytes. Only the
ten-byte numeric prefix is examined; localized title bytes remain untouched.
Missing optional files still return zero without changing the destination.

## Current mapping

`TMFieldSceneUI.cpp` passes `sizeof(m_pLevelQuest)` for every call.
`TMSceneResources.cpp::LoadMsgLevel` retains the native file order, chunk
size and 100-read limit. `quest_subject::Apply` validates both numbers,
numeric overflow, delimiters and the level range before writing. A malformed
prefix is skipped without changing any table entry.

## Delta matrix

| Claim | Evidence | Classification | Limit |
| --- | --- | --- | --- |
| Preserve valid one-based indices, type bytes and overwrite order | Native initializer and loader above | MODERNIZACAO_COMPATIVEL | Native malformed-input writes are intentionally rejected |
| Reject malformed metadata and levels outside 1..capacity | New bounded parser and guard tests | MODERNIZACAO_COMPATIVEL | Safety improvement, not native parity |

## Decisions

Preserve the recovered valid resource contract and remove the unchecked
write. Do not alter quest availability, content, rewards or server rules.
No new REA export is needed for the unchanged native binary and saved roots.

## Gaps

Actual quest-panel rendering, selection, scene changes and reopening in the
built candidate remain pending. Skipping malformed rows is a modern safety
policy and does not reproduce native behavior on invalid input.

## Validation

`LocaleBoundaryTests` covers levels 1 and 500, duplicate overwrite, comments,
blank rows, missing/invalid numbers, numeric overflow, levels 0, -1 and 501,
null/empty destinations and unchanged guard bytes around the table.
The complete NoDeploy build passed on 2026-10-23. The existing resource
tests preserve quest keys, levels and page boundaries across EN and PT.
Real client: pending. Candidate identity and delivery scope are recorded in
[the multilingual client document](../../../../../DOCS/client/multilingual-client.md).
