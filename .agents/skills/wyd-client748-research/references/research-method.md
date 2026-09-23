# 7.48 client research method

## Sources and limits

To understand work already implemented, first read the current source and its
diff; they reveal what actually needs a decision. For native parity claims,
consult these sources in order:

1. The historical `WYD.exe` with a confirmed hash and its Ghidra project;
2. The 7.48 assets actually consumed;
3. The current WYD-Go protocol and behavior;
4. The current buildable source, treated as a candidate implementation;
5. TMProject and guides as secondary semantic references. External legacy
   projects provide historical provenance, never parity authority. When data
   or behavior has already been independently validated in the real 7.48
   runtime, that 7.48 validation is reusable evidence.

The exported `.c` corpus supports bulk search but loses important information:
data xrefs, indirect calls, types, stack details, structures, and part of the
call graph. A missing textual occurrence does not prove there is no caller.
Resolve the question in the Ghidra project before promoting a record to
`TRACED`.

## Decision modes

- `PARIDADE_NATIVA`: the native client determines behavior; the record measures
  how well that behavior is understood.
- `MODERNIZACAO_COMPATIVEL`: the source may use a better internal structure;
  investigate only the native boundaries that must remain equivalent.
- `EXTENSAO_COORDENADA`: client and server define a new contract. Ghidra helps
  locate integration points and collisions, not prove an absent feature.

A `LOCATED` record blocks only the native claim that depends on it. Do not use
native research maturity to approve or reject an extension. Preserve existing
code and assets until concrete incompatibility is demonstrated.

## Flow boundary

Start with a transition that has observable input and output. Useful examples:

- Received bytes -> validated packet -> handler -> updated HUD;
- Slot click -> selection/drag -> sent intent -> received rollback;
- Login packet -> created scene -> bound resources -> active world;
- Key press -> toggled UI root -> changed focus -> destroyed window;
- Received affect -> local state -> rendered effect -> expiration/removal.

Avoid titles based on a file (`TMFieldScene.cpp`) or a broad feature
(`inventory`). A file contains many flows, and a flow spans files.

## Native evidence collection

Reuse records and exports that already support the decision with a valid
identity. The queries below address concrete gaps; they are not a checklist
to repeat every export mode in every investigation. Record the question and
stop collecting when the transition reaches the required adaptation gate.

For each entry point not yet proven:

1. Record the address, current name, inferred signature, and calling convention;
2. Record direct callers, relevant indirect xrefs, and why the entry is reached;
3. Record callees that mutate state, send network data, load resources, or alter
   lifecycle;
4. Identify globals read or written and pointer ownership;
5. Describe preconditions, success branches, rejection, teardown, and retry;
6. Record constants, strings, IDs, opcodes, and layouts only with context;
7. If an export times out, retry a focused export with a longer timeout. If it
   still fails, inspect the function in the project or record the gap. Never
   infer the missing block or repeat the same timeout unchanged.

When using `ExportWydFlow.java`, pass only the functions, vtable bases, and
slots that answer the record's question. The data window provides structural
context, but a slot's xrefs count as evidence only when the exact address was
also requested. Do not version a broad sweep merely to preserve future search
possibilities.

To search for computed virtual dispatch, use `virtualslot:<offset>` with a
hexadecimal offset, with or without the `0x` prefix. The report records each
matching `CALL [reg+offset]` and ends with `virtual_slot_search`, including
hits, candidates, and scanned instructions. The same offset occurs in vtables
of different classes: the instruction owner is not necessarily the receiver
owner. Before naming a class, prove the receiver register's origin, the
observed vptr, the vtable, and the lifecycle transition that makes the instance
reachable.

Headless Ghidra can catch a script exception and still exit with code `0`.
Validate every run using all three conditions: no `SCRIPT ERROR` in the log,
the expected SHA-256 on the `program` line, and the requested mode summary in
the TSV. Invalid arguments must produce `SCRIPT ERROR` and no accepted summary;
an exit code alone is not a test.

### Batch differential correlation

`ExportWydFingerprints.java` extracts byte, mnemonic, normalized-operand,
p-code, and CFG hashes for each function, along with strings, imports,
constants, and direct neighbors. Run it in separate read-only Ghidra projects
for the 7.48 binary and the exact `project.exe` being compared. The export
must contain the program SHA-256; the correlator must also validate the
binaries when they are available.

`correlate_fingerprints.py` uses uniqueness, reciprocal best matches, margin,
and limited call-graph support. Interpret its classes as follows:

- `EXACT_MATCH`: a unique structural fingerprint or unique bytes, still
  subject to flow review;
- `STRONG_MATCH`: a unique candidate with stable signals and sufficient margin;
- `CANDIDATE`: useful for prioritizing manual analysis, not a behavior decision;
- `NO_MATCH`: no useful correlation; does not prove absence from the source.

Correlation narrows the search but does not prove semantics, ABI, receiver
type, ownership, reachability, or lifecycle. Review the root and relevant
neighbors in Ghidra before recording a name or promoting a record. Complete
exports are regenerable caches outside Git; version only focused excerpts
supporting a record's claims.

When a function has no typed xref, use `pointers:<entry>` to find its raw
32-bit VA throughout loaded memory. Record the number of hits and classify
each slot in the project. Zero hits rule out only that literal representation;
they do not rule out a computed call, thunk, slot constructed at runtime, or
reference not yet materialized by analysis.

To distinguish the absence of a literal pointer from the absence of a direct
call, also use `relatives:<entry>`. This mode scans the decoded listing and
resolves only five-byte `CALL E8` and `JMP E9` instructions by their signed
`rel32` displacement. Validate the scanner with at least one known target in
the same binary. Zero hits rule out only those decoded branches; indirect
calls, unanalyzed regions, and runtime-computed targets remain open.

When an entry point has no xref, use `bodyrefs:<entry>` to find external xrefs
to any instruction in the function body, not just its entry. The report
separates `FLOW`, `DATA`, other types, and recognized thunks, and records the
immediately preceding instruction and its fallthrough. An external `FLOW`
xref or thunk may reveal another static entry; `DATA` xrefs forming internal
jump tables describe the switch CFG and are not callers. No `FLOW` or thunk
still does not rule out indirect calls or runtime-computed targets.

Use `rawrelatives:<entry>` to complete the listing scan: it examines every
byte of initialized executable blocks and resolves `E8/E9 rel32` candidates,
including in regions not yet decoded. Each hit is classified as `instruction`,
`data`, or `undefined` and requires Ghidra review. Zero hits rule out only
this encoding in the recorded blocks and bytes, not indirect calls, derived
targets, runtime-built tables, or memory absent from the analyzed program.

Run `bodyrefs` and `rawrelatives` against functions with known direct callers
in the same binary as positive controls. Record hits, candidates, bytes,
blocks, and errors. If the controls do not recover the expected entries, a
zero result for the target is invalid and cannot support a conclusion.

An address without a module and hash is not stable. For a crash, record the
module, loaded base, RVA, preferred VA, and invalid pointer.

## Wire and ABI

A wire record must contain:

- Direction (`C->S`, `S->C`, or local);
- Opcode and total size;
- Header, offsets, widths, signedness, and packing;
- Native constructor/emitter and consumer;
- Any larger final packet embedding the struct;
- The `internal/wire` equivalent and server input validation;
- A byte-for-byte corpus or test that makes the conclusion falsifiable.

A `static_assert` table in the candidate source protects the current
implementation but does not, by itself, prove a layout came from 7.48.

## UI, input, and rendering

Record together:

- Resource file, materialized root, and child IDs;
- Construction function and initial state;
- Binding in the candidate source;
- Mouse/key input and hit testing;
- Open/close callback and visibility condition;
- Ownership, focus, teardown, and access while closed;
- Resolution and scale used for comparison;
- Assets, texture, mesh, blend/depth, and draw order when visual.

A missing modern control may be `nullptr`; prove its callers and preserve the
main transition. Do not fabricate an ID merely to satisfy 7.59 topology. The
control can be materialized as a deliberate extension when its resource,
binding, input, ownership, and teardown are implemented and tested.

## Delta matrix

Compare by claim, not by entire file:

| Claim | Native 7.48 | Current source | TMProject | WYD-Go | Decision |
| --- | --- | --- | --- | --- | --- |
| Behavior/layout | Evidence | Implementation | Origin/risk | Contract | Port/keep/modernize/extend/remove/unconfirmed |

Use these classes:

- `CONFIRMED`: supported by reproducible primary evidence;
- `PROBABLE`: several consistent clues, but one link is still missing;
- `HYPOTHESIS`: a search candidate, never a contract;
- `REJECTED`: contradicted by stronger evidence.

## Review and promotion

Before promoting a native record:

1. Run `validate_research.py` on the changed record batch before accepting
   promotion; one final run may cover several records;
2. Confirm that relevant entries, callers, and callees have enough Ghidra
   evidence for the claim. Reopen only links not yet proven or invalidated;
   do not repeat full inspections already completed in this session;
3. Confirm that the recorded fingerprint still matches. Recalculate the hash
   only if path, size, or mtime changed, or if the record cannot otherwise
   identify the binary;
4. Ensure gaps and unconfirmed claims were not used to make the decision;
5. Record reproducible tests and the actual validation state.

`CLIENT_TESTED` requires running the flow in `tmproject/client748/project.exe`;
a build or isolated startup is insufficient.

For lifecycle records at `TRACED`, `CONTRACT`, or `CLIENT_TESTED`, the validator
requires verifiable content for observable entry, transition matrix,
vtables/vptrs/receivers, ownership, partial failure, cleanup/teardown,
shutdown, and logout/relogin. When an item truly does not apply, record
`N/A:` with a reason; an empty marker does not close the gate.
