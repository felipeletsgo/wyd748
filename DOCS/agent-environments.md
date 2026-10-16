# Agent environments: Codex and Claude Code

The repository was first developed with OpenAI Codex and is also worked on with
Claude Code. Both agents must read the same rules and skills. This page maps
each agent's entry points to the single sources of truth; it adds no rules.
Rules remain in [`AGENTS.md`](../AGENTS.md) and procedures in
`.agents/skills/`.

## Entry points

| Concern | Codex | Claude Code |
| --- | --- | --- |
| Repository rules | Loads `AGENTS.md` | Loads the root `CLAUDE.md`, which only imports `@AGENTS.md` |
| Skill discovery | `.agents/skills/<name>/SKILL.md` | `.claude/skills/<name>/SKILL.md` pointer to the canonical skill |
| Skill invocation | `$<name>` or automatic selection | `/<name>` or automatic selection by description |
| Skill UI metadata | `agents/openai.yaml` | Not used |
| Shared project settings | None tracked | `.claude/settings.json` (tracked guardrails) |
| Local, personal settings | `.codex/config.toml` (ignored) | `.claude/settings.local.json` (ignored) |
| Skill authoring helper | Codex `skill-creator` | Claude Code `skill-creator` |

The root `CLAUDE.md` must contain only the import shim, and each Claude Code
skill pointer must keep the canonical `name` and `description` and link its
`.agents/skills/<name>/SKILL.md`. Procedure lives only in the canonical
skill; its `references/` and `scripts/` paths resolve from that directory.
`tools/repository/Test-RepositoryLayout.ps1` enforces the shim, pointer
parity, missing pointers, and orphan pointers.

## Shared guardrails

`.claude/settings.json` denies actions that `AGENTS.md` already forbids:
`git reset --hard`, `git checkout --`, forced pushes, `git clean`, and edits
under the read-only `references/` evidence collection (`Edit` rules also cover
file writes). It does not pin a model or effort level; choose those in the
client. Deny rules are skipped in Claude Code's bypass-permissions mode, so
they are a safety net, not a substitute for the rules. Codex has no tracked
equivalent; its local configuration stays untracked.

## Working conventions for either agent

- Reuse the selected checkout and preserve its dirty changes. This document
  does not impose a branch policy absent from `AGENTS.md`. A fresh worktree
  may lack ignored runtime inputs such as `tmproject/client748/Mounts-KR.json`;
  inspect prerequisites before treating fixture failures as product regressions.
- Keep scratch output in the agent's temporary directory or ignored build
  directories, never in `tmproject/` or `wydgo748/`. Recorded paths such as
  `%TEMP%\codex-wyd748-lifecycle-149205b7` in research records and handoffs are
  historical evidence locations, not a required naming scheme.
- Model or reasoning-effort notes in historical handoffs (for example
  `gpt-5.6-sol/xhigh`) are provenance, not policy. Commands that call
  `%USERPROFILE%/.codex/skills/.system/skill-creator/scripts/quick_validate.py`
  are Codex-specific; under Claude Code validate skills with its
  `skill-creator` skill and the layout validator.
- Research scripts named without a path in the research references live in
  `.agents/skills/wyd-client748-research/scripts/`.
- `tmproject/Build-Client.ps1` installs `tmproject/client748/WYD.exe`
  by default. Pass `-NoDeploy` unless the task includes replacing the runtime.
  `CLIENT-TESTED` always requires running the built client.

## Validation

The October 2026 rules/skills review clarified the shared routing:

- `repo-architecture` owns instruction maintenance; references are reviewed
  alongside entrypoints, Claude pointers and discovery metadata.
- `wydgo748-feature` owns authoritative behavior and integrated contracts;
  `wyd-client748-research` is added for an affected legacy boundary, not for
  every internal server edit. External source is not native 7.48 proof alone.
- `wyd-client748-assets` separates asset identity, render state and lifecycle;
  `wyd-client748-catalog` is for census/root selection, not repeated research
  of an already located flow.
- `wyd-release` owns authorized packaging, SSH diagnosis, publication and
  activation. It separates source, built/installed artifacts, public downloads,
  running server identity and actual gameplay. It does not broaden authority.

Release-specific versions, hashes, hosts and outcomes remain in existing
release receipts. They are not permanent defaults in shared instructions.

The review covers all canonical skill entrypoints, their instruction references,
Claude pointers and existing discovery metadata. It does not certify research
helper algorithms or whole-repository language migration. Legacy Portuguese
remains in helper scripts such as `inventory_corpus.py`,
`build_research_queue.py`, `correlate_fingerprints.py`, `ExportWydFlow.java`,
`ExportWydFingerprints.java` and `ExportWydFunctionCatalog.java` under
`.agents/skills/wyd-client748-research/scripts/`; translate each when that helper
is changed and validate its affected output/consumer contract. No helper was
modified or corpus regenerated by this instruction-only review.

After changing rules, skills, pointers, or agent settings:

```powershell
pwsh -NoProfile -File .\tools\repository\Test-RepositoryLayout.ps1
git diff --check
```

Add `-UpdateMap` only when the documentation inventory changes, as for adding
or removing a skill.
