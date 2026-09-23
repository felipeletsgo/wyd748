---
name: wyd-client748-assets
description: Audit and adapt 7.48 client assets while preserving identity, paths, dependencies, and server compatibility.
---

# 7.48 client assets

Use this skill for assets, visual resources, UI, audio, or runtime files under
`tmproject/client748/`.

- Identify the asset, loader, and consumers. For a changed legacy boundary,
  use `wyd-client748-research`, reusing valid evidence about resource/ID and,
  for UI, binding, input, ownership, closing, and teardown. Replacing a
  texture while preserving format and ID does not require retracing the UI.
- Preserve existing names, formats, and files. Absence from native 7.48 does
  not authorize removal of a manually created asset; demonstrate runtime,
  contract, or build incompatibility first.
- Distinguish assets actually used by the build from historical/reference
  assets. Record origin, hash when identity matters, and materialized
  destination in `DOCS/` or `.agents/research/`, never a source-tree dump.
- `references/client748/` is historical evidence only; do not replace its
  binaries with adaptation output.
- When an asset participates in a coordinated extension, document its ID,
  version/capability, fallback, and expected effect in `wydgo748/`.
- Use ignored, removable temporary directories for builds, conversion, and
  inspection. Do not scatter scripts, logs, or generated artifacts.

Validate format, references, and loading of changed resources, including
applicable fallback. Follow the `AGENTS.md` matrix: an asset without a
compilation dependency needs no rebuild; a coordinated contract requires
tests on both sides. Visual/audio confirmation requires the real client flow;
a hash or build cannot substitute for it. Do not rescan the entire archive for
one asset.
