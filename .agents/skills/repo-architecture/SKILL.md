---
name: repo-architecture
description: Organize the WYD-Go repository architecture, documentation, rules, and skills without scattering files into source trees.
---

# Repository architecture

Use this skill for structural, documentation, or skill-maintenance tasks.

## Canonical layout

- `tmproject/`: only the adapted 7.48 C++ client, runtime, and required assets;
- `wydgo748/`: only the authoritative Go server and its data/tests;
- `DOCS/`: durable documentation, indexed by `README.md` and
  `documentation-map.md`;
- `.agents/skills/`: active skills;
- `.agents/research/` and `.agents/handoffs/`: evidence and continuity records.

Do not create `AGENTS.md`, `CLAUDE.md`, a project README, research script,
dump, log, or temporary file inside the source trees. Keep only the two active
source roots above. Binary evidence belongs in `references/client748/`.
Follow the repository-wide English-only language rule in `AGENTS.md`.

## Safe procedure

1. Use the one-time entry and validation gates in `AGENTS.md`; do not repeat
   inspection when entering this skill. Identify the affected files and users.
2. Before moving or removing files, search for references with `rg`. Use a
   global inventory only for global reorganization, not a paragraph edit.
3. Move documentation into one `DOCS/` category and fix links. Keep technical
   evidence in `.agents/`, without duplicate reports.
4. Remove only verified duplicates, obsolete rules, and generated artifacts,
   after confirming exact paths. Do not delete source, assets, or native
   evidence for convenience.
5. At batch close, run `tools/repository/Test-RepositoryLayout.ps1`. Use
   `-UpdateMap` only when inventory paths, entries, or categories change; that
   invocation already validates the result. Also run `git diff --check` and
   validators for changed resources.

## Skills

Each active skill has one short, specific `SKILL.md`. Avoid duplicating global
rules in every skill. References and scripts need a clear purpose, current
paths, and reproducible tests.

When creating or revising skills, use the session's `skill-creator` skill.
Validate frontmatter and review triggering, evidence reuse, and blocking
scenarios. Valid YAML alone does not prove that the workflow avoids loops. Do
not invoke gameplay skills merely because documentation mentions a client or
server.
