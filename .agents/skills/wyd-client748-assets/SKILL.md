---
name: wyd-client748-assets
description: Audit and adapt 7.48 client assets while preserving identity, paths, dependencies, and server compatibility.
---

# 7.48 client assets

Use this skill for visual, audio, UI, runtime, or asset changes under tmproject/client748/.

## Route

- Identify the asset, resource ID, loader, format, path, and direct consumers.
- Reuse valid evidence; do not rerun the catalog for an already located asset flow.
- Invoke wyd-client748-research only when the change crosses an unproven legacy boundary.
- Preserve client identity, protocol values, paths, dependencies, and fallback behavior.
- Keep runtime assets and active source changes under tmproject/.

## Decisions

- A replacement with the same ID, format, path, and loader needs only focused validation.
- A loader, binding, input, ownership, close, teardown, or protocol change needs contract research.
- A replicated visual state needs owner and observer checks, including removal and scene changes.
- A new coordinated asset contract needs client and server definitions before packaging.

## Validation

- Check identity, format, dimensions, encoding, IDs, path casing, loader behavior, and fallback.
- Run the smallest affected test or client target; record unavailable visual execution as pending.
- Do not claim gameplay or visual acceptance from file existence or automated checks.
- Keep generated builds and research output in approved build paths.
- Use wyd-release only for package, feed, activation, or deployment work.
