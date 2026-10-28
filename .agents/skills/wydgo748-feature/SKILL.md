---
name: wydgo748-feature
description: Implement or review WYD-Go 7.48 server behavior and integrated TMProject client contracts; skip for documentation or organization without functional changes.
---

# WYD-Go 7.48 feature

Use this skill for Go server behavior or a feature connecting wydgo748/ and tmproject/.
Skip it for documentation, organization, and isolated asset changes.

## Authority

- For native server logic, read references/decompilacao server/dlls/rea-analysis/README.md first.
- Reuse its maps, coverage, and saved evidence before new REA queries.
- Treat supplied server decompilation as primary server evidence and W2PP as secondary.
- Treat TMProject 7.69 as secondary evidence, not 7.48 contract proof.
- Use wyd-client748-research only for an affected legacy client boundary or parity claim.
- Never port later TMProject behavior to 7.48 without evidence.

The client sends intentions and displays state.
The server validates identity, permissions, rules, resources, cooldowns, collisions, and transitions.
The server persists authoritative state and sends snapshots or deltas.
Never trust client-provided position, item, damage, skill, or balance values.

## Route

1. Classify the delta as PARIDADE_NATIVA, MODERNIZACAO_COMPATIVEL, or EXTENSAO_COORDENADA.
2. Locate the live flow with rg from packet or input through state, persistence, and response.
3. Define rejection, replay/idempotence, concurrency, partial failure, and lifecycle for the affected path.
4. For extensions, define format, version/capability, IDs, validation, fallback, and both contracts.
5. Make the smallest patch; do not refactor unrelated code.

Trace conversions where representations can diverge, including canonical/wire IDs, stats, items, visuals, coordinates, and RuntimeID.
Read only the relevant section of the topic references.

## Validation

- Select the package, consumers, and focused tests from AGENTS.md.
- Ensure filtered tests execute cases; include rejection and affected-consumer checks.
- Use full Go suite and vet only for shared or cross-cutting changes.
- If both projects change, validate both; server tests do not validate client parsing or UI.
- Cover the failure-relevant lifecycle: owner/observer state, removal, teardown, visibility, rollback, and repetition.
- Record unavailable client execution as pending; never install a candidate as a hidden test side effect.
- Keep active changes in source/assets; do not overwrite references/client748/.
- Use wyd-release separately for packaging, publication, activation, or deployment.

## References

- Server contracts: references/emulator-contracts.md
- Cross-project contracts: relevant sections of references/repository-contracts.md
- Legacy UI: references/client-ui-748.md
- Native client evidence: references/ghidra-client748.md
- Audits: references/audit.md
- Continuation: references/session-continuity.md
