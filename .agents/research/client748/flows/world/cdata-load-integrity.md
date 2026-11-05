---
id: cdata-load-integrity
title: Validate cdata during the first ground load
subsystem: world
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-11-02
---

# Ground checksum validation

## Question

Which checksum accepts the distributed 8,192-byte `cdata.bin`?

## Evidence boundary

USED: native `FUN_00532390.c` in the cached 7.48 decompilation corpus.
USED: current `TMGround.cpp`, `CDataIntegrity.h`, and the distributed asset.
Asset SHA-256: `71F506D92E7A74F83CE023E8BB6977F1C9D2B3728E41B7723EE0FFC0F15D07F8`.
NOT APPLICABLE: TMSRVIN and Core.dll do not define this client-local file check.
The incidental Core `CData` name matches are not evidence for this format.

## Native 7.48 flow

### Callers

Direct callers include `FUN_00432181`, `FUN_00499f0e`, `FUN_0049add2`,
`FUN_0049f0e7`, and `FUN_004a8f14` in the saved corpus.
The current ground constructor performs this first-load transition.

### Callees

`FUN_00532390` opens the file through `FUN_0058f716`, reads `0x2000` bytes
through `FUN_00590d68`, and closes through `FUN_0058f62c`.
Its nested loops visit 64 rows and 32 columns. Each cell contributes
`8 * row + 16 * column + 4 * data[row][column]` with 32-bit wrap.
Expected checksum: `0x231122b4`. The native mismatch branch checks scene code 30000.

## State and lifecycle

`TMGround::m_bFirst` remains set until a complete valid first read.
A missing, short, or mismatched file marks a critical scene error.
The first valid read sets the flag to zero. Later ground instances reuse the table.
The adapted check also runs in selection scenes, where the first load can occur.
The file handle closes on success and failure. No asynchronous owner exists.

## Wire, ABI, and resources

Client-local resource: `tmproject/client748/cdata.bin`, 64 by 32 little-endian
32-bit integers. File size remains 8,192 bytes. No packet or resource ID changes.
The legacy checksum is an accidental-corruption check, not a signature.

## Current mapping

`CDataIntegrity.h` uses unsigned 32-bit arithmetic. `TMGround` requires a
complete read and compares the native checksum. The asset remains unchanged.
This replaces diagonal indexing and an incompatible expected constant.

## Delta matrix

| Claim | Evidence | Decision |
| --- | --- | --- |
| Native sum visits every cell | `FUN_00532390` nested loops | Use row and column indices |
| Distributed data matches the native sum | Asset size, hash, and checksum probe | Preserve data and expected constant |
| Selection can load first | Current constructor lifecycle | Validate every first load |

## Decisions

Classification: `MODERNIZACAO_COMPATIVEL`.
Reuse native arithmetic and validate the actual first-load boundary.
Do not replace the checksum with a new data format or claim cryptographic integrity.

## Gaps

Real client: pending. Check selection, field entry, and scene changes.
Malformed-input checks run through fixtures without replacing runtime assets.
No native executable was launched.

## Validation

`CoreDataSafetyTests` checks zero data, an off-diagonal mutation, and signed wrap.
The asset probe confirms the expected native checksum on the distributed bytes.
Build receipt: `build/client-native-corrections-20261102/REPORT.md`.
