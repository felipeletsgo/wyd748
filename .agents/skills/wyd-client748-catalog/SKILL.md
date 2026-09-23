---
name: wyd-client748-catalog
description: Update the native WYD 7.48 census or locate and prioritize research roots without an existing seed; do not rerun cataloging for an already located flow.
---

# 7.48 client catalog

Use this skill when a root is missing or the task changes the census/ranking.
With a valid seed or evidence record, proceed to `wyd-client748-research`.
The census prioritizes roots; a `functions.tsv` row does not mean the
function is understood. Cataloging is not a repeated prerequisite for every
adaptation.

## Rules

- Start from the live feature/function in `tmproject/` and propagate to
  native candidates by seed, string, fingerprint, vtable, and call graph.
- The native binary at `references/client748/WYD.exe`, the Ghidra project,
  and studied decompilation are primary references for 7.48 claims.
- `functions.tsv`, exports, scores, and `EXACT_MATCH` are discovery aids.
  Callbacks, thunks, vtables, `bodyrefs`, and `rawrelatives` require Ghidra
  confirmation.
- Reuse hashes, seeds, triage, and confirmed correlations when inputs have
  not changed. Do not repeat a global scan on continuation.
- A coordinated extension may be planned without a native equivalent if
  declared as such with a separate client/server contract.

## Procedure

1. Reuse the one-time `AGENTS.md` entry. Consult the existing seed/census;
   verify identity only when necessary evidence is missing or has changed.
2. Run the triage tool only when the corpus/catalog changes or no valid queue
   exists for the new root:

   ```powershell
   python .agents/skills/wyd-client748-catalog/scripts/triage_catalog.py --repo . --format summary
   ```

3. Choose a small batch tied to an observable entry point; prioritize
   `bootstrap -> login -> scenes -> dispatcher -> world -> logout/relogin`.
4. Record root, rationale, candidates, evidence, and next symbol in
   `.agents/research/`. Once the root is located, hand candidates and gaps
   to research; full call-graph understanding is not required to end cataloging.

### Reproducible source inventory

When the task requires TMProject748 source coverage, install pinned
dependencies from `scripts/requirements-source-inventory.txt` in an isolated
environment and run `scripts/source_inventory.py --repo <root> --write`.
Use `--check --fail-on-parse-errors` at the gate to detect stale inventories
or remaining parser errors. Generated TSVs record file, project, line, and
symbol facts only; do not store human compatibility decisions or promote
research state in them.

## States

Use only `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`, `IMPLEMENTED`,
or `CLIENT_TESTED`. `LOCATED` means found, not approved for a parity
claim; `CLIENT_TESTED` requires execution of the built client.

Read `references/catalog-strategy.md` only when changing schema, ranking,
or coverage criteria.
