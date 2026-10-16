# Ghidra: native WYD 7.48 client

Use this reference for parity claims and affected legacy boundaries in
`tmproject/`, not as a gate for unrelated internal modernization. Reuse proven
records; investigate only missing or invalidated evidence.

## Artifact identity

| Artifact | Role |
| --- | --- |
| `references/client748/WYDoriginal.exe` | Immutable historical stock client |
| `references/client748/WYD.exe` | Historical Ghidra reference |
| Build output identified by `tmproject/Build-Client.ps1` | Recompiled candidate; build-only uses `-NoDeploy` |
| `tmproject/client748/WYD.exe` | Normal installed runtime; verify actual launched path/hash |

The 2026-08-23 historical snapshot recorded:

```text
WYDoriginal.exe B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C
WYD.exe         8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
```

These hashes identify historical evidence, not the current candidate. Old
records mentioning `project.exe` do not make that file the current product.
Do not run/edit historical patchers or copy a historical executable as fallback.
Active changes belong in source/assets. Do not install a binary as a side effect
of consulting Ghidra; installation and client execution require task scope.

Discover local analysis outside Git. The following finds candidates, not proof
that the newest project/export belongs to the required binary:

```powershell
$ghidraProject = Get-ChildItem "$env:USERPROFILE\Tools\GhidraProjects" `
    -Filter 'WYD748Native_*.gpr' | Sort-Object LastWriteTime -Descending | `
    Select-Object -First 1
$decompRoot = Get-ChildItem "$env:USERPROFILE\Tools\GhidraAnalysis" `
    -Directory | Sort-Object Name -Descending | ForEach-Object {
        Join-Path $_.FullName 'decompiled'
    } | Where-Object { Test-Path $_ } | Select-Object -First 1
```

Verify the analyzed program identity against the record before using addresses.
`functions.tsv` and per-function `.c` files support search; the Ghidra project
resolves xrefs, types, stack, calling conventions and call graphs.

## New or invalidated evidence

1. Read the existing transition record first. Reuse a confirmed fingerprint
   when recorded path, size, mtime and corpus are unchanged; hash again when
   these inputs change or identity is insufficient. A mismatch requires
   remapping, never silent reuse of addresses.
2. Locate the behavior by string, opcode, size, constant, import or symbol.
   Resolve native callers/callees and indirect reachability, not just a plausible
   decompiled block.
3. Map initial state, mutations, effects, ownership, rejection and lifecycle.
   For wire/ABI, record size, offsets, signedness, packing and C->S/S->C direction;
   tie byte tests/assertions to independent 7.48 evidence.
4. Compare current source and secondary TMProject. Preserve compatible manual
   improvements and explicit coordinated extensions. Port readable behavior,
   not temporary decompiler names, virtual addresses or unstructured pseudocode.
   Comment non-obvious compatibility/ownership decisions.
5. Apply proportional gates from [AGENTS.md](../../../../AGENTS.md). Research
   alone ends with a validated record; source adaptation adds focused tests/build.
   Use `-NoDeploy` for build-only work. When actual client testing is allowed,
   identify the installed candidate/hash, loaded resources and affected scenario.
   A blocked visual test stays pending, not `CLIENT-TESTED`.

## Minimum evidence by boundary

- Packets: native opcode/size validator, constructor/parser, direction,
  enabling state, expected bytes and corresponding assertion/test.
- UI/input: materialized resource IDs, creation/initial visibility,
  input/callback, open/close conditions, pointer ownership and destruction order.
  An inherited modern control may legitimately be null. Protect all access
  paths; do not fabricate a widget merely to satisfy a newer member.
- Character/items/render: consumed struct, mesh/face/item selector, actual
  loaded resources/indices, renderer/fallback and create/update/destroy lifecycle.
  Include owner and observer paths when synchronization affects appearance.

## Known seeds

These addresses belong to the recorded historical identity, not a stable API.
Reopen only a changed or unproven dependent claim:

```text
FUN_0055890a  Native opcode/size validator
FUN_00524bbb  C->S movement 0x366 construction
FUN_00520216  Local route construction/validation
FUN_0055f2dd  Send/throttle
FUN_00492e7d  Field input/dispatch
FUN_00489a3e  Received packet dispatch
FUN_00441823  Main field UI creation/initialization
```

## Crash triage, minidumps and ASLR

```text
RVA = fault_address - loaded_module_base
preferred_VA = preferred_ImageBase + RVA
```

Record dump, exception, module, loaded base, RVA, preferred image base and exact
executable hash. Use matching symbols/PDB or disassembly for that binary.
An RVA from a recompiled candidate does not map directly into a different
historical binary's Ghidra project. Identify the invalid register/pointer and
source access; the address alone is not a diagnosis. Preserve crash artifacts
before rotating logs or replacing the candidate.

Historical examples (not current addresses):

- 2026-08-23: fault `0x009A1F46`, loaded base `0x008B0000`, RVA `0x000F1F46`,
  preferred VA `0x004F1F46`; virtual call with `ecx == 0` at direct
  `pFScene->m_pMHPBar` access during character materialization.
- 2026-08-26: `tmproject/client748/client-crash-20260826-154302.dmp`,
  `SGridControl::TradeItem`, RVA `0x000718AC`, dereferenced
  `m_pChatSelectPanel`. Native AutoTrade uses panel 626, edit 627, caption 630
  and button 667, without the inherited 7.59 chat selector. Inspect all accesses,
  including error, reentry, input and closure; guarding only the first frame
  leaves equivalent crashes reachable.

The recorded 7.48 UI lacks modern `T_CURRENT_MHP=65618`, `T_MAX_MHP=65619`,
`P_MHP_PROGRESS=65625` and `P_MHP_PROGRESS_TR=65626`, including truncated IDs
`82/83/89/90`. Their bindings may be null by contract. Score/equipment/mount
updates must tolerate that without blocking world entry. Do not truncate IDs
or fabricate replacements. A deliberate new control needs its own coordinated
resource, binding, input and lifecycle contract.

## Review boundaries and result

Do not infer parity from compilation or a similar screenshot, import newer
packing/IDs without proof, remove an extension merely because stock lacks it,
or accept a wrong modern packet to conceal a client constructor mismatch.
Coordinate fixes must not conceal incorrect control identity/lifecycle.

Before adaptation, record a concise claim matrix:

```text
Mode | Behavior | 7.48 function/boundary | Current source | Server contract | Decision
```

Report `IMPLEMENTED`, `STATICALLY VERIFIED`, `AUTOMATED TESTED` and
`CLIENT-TESTED` distinctly. The last requires actual execution of the affected
flow in the identified candidate; historical evidence is never a product fallback.
