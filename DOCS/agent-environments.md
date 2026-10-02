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

- Work on `main` in the main checkout. Do not use agent worktree isolation:
  `AGENTS.md` forbids splitting work into branches or worktrees, and a fresh
  worktree lacks ignored runtime inputs such as
  `tmproject/client748/Mounts-KR.json`, so `go test ./...` fails there.
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
- `tmproject/Build-Client.ps1` installs `tmproject/client748/project.exe`
  by default. Pass `-NoDeploy` unless the task includes replacing the runtime.
  `CLIENT-TESTED` always requires running the built client.

## Validation

After changing rules, skills, pointers, or agent settings:

```powershell
pwsh -NoProfile -File .\tools\repository\Test-RepositoryLayout.ps1
git diff --check
```

Add `-UpdateMap` only when the documentation inventory changes, as for adding
or removing a skill.
