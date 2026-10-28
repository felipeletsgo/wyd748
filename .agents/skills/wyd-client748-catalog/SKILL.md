---
name: wyd-client748-catalog
description: Update the native WYD 7.48 census or locate and prioritize research roots without an existing seed; do not rerun cataloging for an already located flow.
---

# 7.48 client catalog

Use this skill only when a research root is missing or the census or ranking changes.
With a valid seed or evidence record, continue with wyd-client748-research.

## Rules

- Treat the native 7.48 client as primary evidence for 7.48 parity.
- Treat server 7.59 REA as separate server evidence; do not merge it into client proof.
- Catalog location and symbols do not prove function behavior or runtime execution.
- Treat functions.tsv, exports, scores, and EXACT_MATCH as discovery aids only.
- Reuse the saved corpus, hashes, coverage, and prior queue before new indexing.
- Do not execute native binaries or modify tracked/shared-map evidence during catalog work.

## Procedure

1. Confirm that the requested root is absent or that source coverage changed.
2. Record the exact source, SHA-256, architecture, tool version, and output location.
3. Run triage_catalog.py only when the corpus changes or no valid queue exists.
4. Rank a narrow candidate set by reachable flow, symbols, strings, callers, and saved evidence.
5. Select one smallest root and hand it to research for call graph and observable flow.
6. Update the existing catalog or evidence record; do not create duplicate reports.

Read references/catalog-strategy.md only for schema, ranking, coverage, or source-inventory decisions.
Use UNMAPPED, LOCATED, TRACED, CONTRACT, IMPLEMENTED, or CLIENT_TESTED.
LOCATED means found, not understood; CLIENT_TESTED requires built-client execution.
Run the relevant catalog script or source_inventory.py --check once after the batch, not after every query.
