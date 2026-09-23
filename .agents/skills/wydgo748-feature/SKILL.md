---
name: wydgo748-feature
description: Implement or review WYD-Go 7.48 server behavior and integrated TMProject client contracts; skip for documentation or organization without functional changes.
---

# WYD-Go 7.48 feature

Use this skill for Go server changes or a feature connecting `wydgo748/`
and `tmproject/`.

## Authority model

The client sends intentions and displays state. The server validates identity,
permissions, rules, resources, cooldowns, collisions, and transitions;
persists authoritative state; and sends snapshots/deltas. Never accept a
client-provided position, item, damage, skill, or balance as truth without
server-side validation.

Before editing, classify the delta as `PARIDADE_NATIVA`,
`MODERNIZACAO_COMPATIVEL`, or `EXTENSAO_COORDENADA`. If it touches legacy
behavior, packet, ABI, UI, input, rendering, asset, or lifecycle, use
`wyd-client748-research` before this skill. Do not blindly port TMProject
7.69 to 7.48.

## Procedure

1. Reuse the one-time `AGENTS.md` entry and current contract. An internal
   server change that does not cross a legacy boundary needs no client
   research.
2. Locate the live flow with `rg` and follow the intention from packet
   through state, persistence, and response. Define relevant rejection,
   replay/idempotence, concurrency, and partial failure before connecting UI;
   do not audit domains the change cannot reach.
3. For a coordinated extension, document format, version/capability,
   opcode/IDs, validation, fallback, and compatibility on both sides.
4. Make a small patch, run a focused test, then continue. Do not refactor
   unrelated code.

## Validation

Apply the `AGENTS.md` matrix. In `wydgo748/`, select the package and tests
exercising the change (`go test ./internal/game -run <TestName>`, for
example; replace with the real target). The filter must run tests, not just
return success with zero cases. Include rejection cases and affected
consumers; reserve full suite/vet for a cross-cutting batch.

If both projects change, test both. A server-side test does not validate the
client parser/UI. Build the affected C++ target and record real execution as
pending when unavailable; never install a candidate as a hidden side effect
of a check. Validation states are defined in `AGENTS.md`.

Active client changes belong in source/assets. `references/client748/`
contains historical evidence and must not be overwritten.

## References by topic

- Server contracts: [emulator-contracts.md](references/emulator-contracts.md).
- Cross-project contracts: find the subsystem in
  [repository-contracts.md](references/repository-contracts.md) and read the
  relevant section.
- Legacy UI: [client-ui-748.md](references/client-ui-748.md).
- Native evidence access: [ghidra-client748.md](references/ghidra-client748.md).
- Requested audits: [audit.md](references/audit.md).
- Continuation: [session-continuity.md](references/session-continuity.md).
