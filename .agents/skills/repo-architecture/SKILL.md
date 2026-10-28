---
name: repo-architecture
description: Organize the WYD-Go repository architecture, documentation, rules, and skills without scattering files into source trees.
---

# Repository architecture

Use this skill for repository layout, durable documentation, rules, or skill maintenance.
Do not invoke gameplay, asset, research, or release skills for organization-only work.

## Route

- Read the requested rule, document, or skill and its direct references.
- Inspect AGENTS.md, git status --short, HEAD, and the scoped diff once.
- Search with rg; do not rescan unrelated source or research trees.
- Correct the source of a contradiction, not a duplicate or historical record.

## Invariants

- Keep active source roots in tmproject/ and wydgo748/.
- Keep documentation in DOCS/, skills in .agents/skills/, evidence in .agents/research/, and handoffs in .agents/handoffs/.
- Keep native server REA under references/decompilacao server/dlls/rea-analysis/.
- Keep references/ local-only and separate static evidence, proven behavior, and client 7.48 compatibility.
- Keep generated outputs in approved ignored paths; never add source-tree rule files or generated executables.

## Skill maintenance

- Keep one short canonical entrypoint per skill.
- Put detailed schemas, procedures, and examples in named references.
- Preserve frontmatter, IDs, evidence, and existing pointer names.
- Keep matching .claude/skills/<name>/SKILL.md pointers and agents/openai.yaml metadata synchronized.
- Use skill-creator guidance and its validator when changing a skill.
- Keep automatic invocation enabled unless the user explicitly requests another mode.

## Closeout

- Update DOCS/README.md and DOCS/documentation-map.md only when inventory changes.
- Run tools/repository/Test-RepositoryLayout.ps1 once for the changed batch.
- Validate each changed skill entrypoint, scoped links, and git diff --check.
- Do not run product builds, Ghidra, or runtime checks for documentation-only work.
- Report skipped checks and unrelated dirty files without modifying them.
