---
name: wyd-client748-research
description: Resolve evidence for legacy WYD 7.48 client behavior or contracts during adaptation/parity work; reuse valid records and skip for internal server or documentation maintenance.
---

# 7.48 client research

Use this skill for an affected legacy boundary or one crossed by an extension.
Do not demand a native equivalent for a new contract. The parity pipeline is
a reusable evidence chain, not a command sequence to repeat:

```text
catalog -> call graph -> observable flow -> adaptation -> validation
```

## Authority

For each front, assess and record the native 7.48 binary, Ghidra
project/decompilation, 7.48 assets, current TMProject source, `wydgo748/`,
tests, and other references. Mark each source `USED`, `NOT APPLICABLE`,
or `CONTRADICTORY`, with a reason. The native binary and Ghidra take
precedence; TMProject 7.69, guides, and history are secondary. External
legacy projects may explain a datum's origin but do not establish parity.
If the same behavior/data has independent, recorded validation in the 7.48
runtime/client or a reproducible 7.48 contract test, reuse that evidence as
`PARIDADE_NATIVA` without reopening research.

## Classification and gates

- `PARIDADE_NATIVA`: reproduces legacy behavior. A not-yet-validated
  boundary needs a `TRACED` record, and wire/ABI/loader/resource changes
  need `CONTRACT` before adaptation. Still-valid recorded 7.48 evidence,
  including data and tests already exercised in the real runtime, may be
  reused without repeating Ghidra or exports.
- `MODERNIZACAO_COMPATIVEL`: changes internals while preserving the
  contract; reuse a proven record and limit proof to affected boundaries.
- `EXTENSAO_COORDENADA`: adds a non-native contract; requires format,
  version/capability when needed, collision-free IDs/opcodes, validation,
  rollback/fallback, and tests in both projects. It is not parity.

## What to trace

When opening a new front, read [research-method.md](references/research-method.md).
When writing or updating an evidence record, read
[evidence-record.md](references/evidence-record.md). Consult
[subsystem-map.md](references/subsystem-map.md) for boundary selection only
if no flow/root is defined. Use the catalog skill only when a seed is missing
or the census itself is the task.

A record describes one transition: entry/event, native root, callers/callees,
state and mutations, errors, wire/ABI/resources, source/server, decision, and
validation. Confirm indirect calls, vtables, callbacks, ownership, partial
failure, cleanup, and teardown in Ghidra. For lifecycle, resolve shutdown and
logout/relogin too, or mark them N/A with a reason.

For packets, record direction, opcode, size, offsets, packing, and signedness.
For UI/assets, record materialized resource, binding, input, ownership,
closing, and teardown. Lack of a textual caller does not prove dead code.

## Continuation and exit

Reuse the `AGENTS.md` one-time entry and invalidation policy. With sufficient
evidence and unchanged inputs, proceed to adaptation without a new
export/Ghidra run. If evidence is missing, ask a specific transition question
and choose a query that can answer it. Once the gate closes, implement the
cleared delta; do not research neighboring areas without a demonstrated
dependency. A real gap blocks only its dependent edit and must name the
missing evidence or access. Record evidence in `.agents/research/` and a
durable decision in `DOCS/` without duplication. Run `validate_research.py`
once per changed batch of records/schema before accepting promotion. Changes
only to this skill's instructions do not invalidate records or require
reprocessing the corpus.

`STATICALLY VERIFIED`, `AUTOMATED TESTED`, and `CLIENT-TESTED` are
distinct states. A build or static test never promotes a front to
`CLIENT_TESTED`.
