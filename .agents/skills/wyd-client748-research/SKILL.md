---
name: wyd-client748-research
description: Resolve evidence for legacy WYD 7.48 client behavior or contracts during adaptation/parity work; reuse valid records and skip for internal server or documentation maintenance.
---

# 7.48 client research

Use this skill for an affected legacy boundary or a boundary crossed by an extension.
Skip it for internal server, asset-preserving, documentation, or rules-only changes.

## Authority and gates

- Native 7.48 binary, Ghidra evidence, assets, current source, server, tests, and references are the evidence set.
- Native binary and Ghidra take precedence; 7.69, W2PP, guides, and history are secondary.
- External projects explain possible origins but do not prove 7.48 parity.
- Mark each relevant source USED, NOT APPLICABLE, or CONTRADICTORY with a reason.
- Reuse independently validated records for unchanged behavior; self-tests alone do not prove native origin.
- Classify the change once as PARIDADE_NATIVA, MODERNIZACAO_COMPATIVEL, or EXTENSAO_COORDENADA.
- A new parity boundary needs TRACED; wire, ABI, loader, or resource changes need CONTRACT before adaptation.
- A coordinated extension needs format, capability/version, collision-free IDs, validation, fallback, rollback, and both-project tests.

## Minimal route

1. Define the one transition or contract claim.
2. Reuse the record, hashes, exports, and tests already covering unchanged inputs.
3. If evidence is missing, choose the smallest bounded query that can answer the claim.
4. Trace entry, callers, state, mutation, errors, wire/ABI/resources, ownership, teardown, and validation.
5. Adapt only the cleared boundary, then record the decision and focused checks.

Read research-method.md only when opening a new front.
Read evidence-record.md when writing or updating a record.
Read subsystem-map.md only when no root or flow is defined.
Use wyd-client748-catalog only when the seed or census is missing.

## Evidence and exit

- Record one transition with entry/event, root, callers/callees, state, errors, resources, decision, and validation.
- For packets, record direction, opcode, size, offsets, packing, and signedness.
- For UI or assets, record materialization, binding, input, ownership, close, and teardown.
- Resolve indirect calls, callbacks, vtables, partial failure, cleanup, shutdown, and relogin, or mark each N/A with a reason.
- Treat provider, execution, timeout, resource, EPERM, EACCES, and missing-facet errors as tooling limits, not absence proof.
- After two attempts without new evidence, change the method or report the dependent gap.
- Store evidence in .agents/research/ and durable decisions in DOCS/ without duplication.
- Run python -B .agents/skills/wyd-client748-research/scripts/validate_research.py --repo . once per changed record/schema batch.
- Keep STATICALLY VERIFIED, AUTOMATED TESTED, and CLIENT-TESTED distinct.
