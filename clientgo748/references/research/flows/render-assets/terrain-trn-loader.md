---
id: terrain-trn-loader
title: Native 7.48 TRN terrain loader
subsystem: render-assets
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-09
---

# Native 7.48 TRN terrain loader

## Classification

- Mode: `PARIDADE_NATIVA`
- Status: `CONTRACT`
- Scope: file framing and the one cell field directly consumed by the native
  terrain mesh builder. This does not claim collision, texture selection, or
  final rendering parity.

## Native evidence

The analyzed executable is `references/ghidra/input/WYD.exe` with SHA-256
`8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.

- `FUN_00533DD7` opens the path, reads a one-byte name length, copies that
  many bytes, reads two one-byte dimensions, then reads `0x1000` records of
  `0x0c` bytes into the terrain object.
- The same function validates a checksum over all `0xc000` payload bytes and
  builds border/normal data from the grid.
- `FUN_00534EBE`, called while building the mesh, reads the first byte of each
  record through `char` and converts it to a signed height sample. This is the
  only cell-field semantic promoted here.
- `FUN_00532390` constructs the terrain object; callers such as
  `FUN_0049F0E7`, `FUN_004A8F14`, and `FUN_00432181` select `.trn` resources for
  character and field scenes.

## Go contract

`internal/assets.ParseTerrain` validates the length-prefixed name, two header
bytes, a square payload of 12-byte records, and bounded allocation. Each cell
owns its bytes and exposes `Height int8`, copied from `Raw[0]`. The other 11
bytes remain available through `Raw` and are intentionally opaque.

The parser does not use `Height` for collision or movement yet. A future
promotion of tile/material fields requires separate native xrefs and a focused
golden test.

## Source matrix

- Native 7.48 binary/Ghidra: `UTILIZADA`; primary evidence for framing and
  signed height access.
- Official 7.48 assets: `UTILIZADA`; `Env/Character.trn` validates the 64x64,
  4096-record instance.
- Current Go source/tests: `UTILIZADA`; implementation and regression tests.
- TMProject748: `UTILIZADA`; secondary comparison only for the 12-byte record
  shape, not for addresses or ABI.
- Guides and W2PP/Secrets/Micronics: `NÃO APLICÁVEL`/excluded; not used to
  establish this contract.

## Open questions

The meanings of bytes 1..11, the two header bytes, checksum table selection,
and the relationship between height and gameplay collision remain open. Do not
promote them from `Raw` without native evidence.
