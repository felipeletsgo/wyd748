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
| Skill UI metadata | `.agents/skills/<name>/agents/openai.yaml` for every canonical skill | Not used |
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
`git reset --hard`, `git checkout --`, forced pushes, `git clean`, new
branches, worktrees, and pull requests, client launches through
`Start-Process` or `Invoke-Item`, and edits under the read-only `references/`
evidence collection (`Edit` rules also cover file writes). It sets
`PYTHONDONTWRITEBYTECODE=1` for every command, as the Python rule requires.
It does not pin a model or effort level; choose those in the client. Deny
rules are a safety net, not a substitute for the rules: shell patterns cannot
catch every equivalent command. Codex has no tracked equivalent; its local
configuration stays untracked.

## Claude Code tool mapping

The rules name generic actions. Use these Claude Code tools for them:

| Rule or skill wording | Claude Code tool |
| --- | --- |
| Search with `rg` | `Grep` tool (ripgrep); use `rg` in a shell only for pipelines |
| PowerShell 7 commands | `PowerShell` tool (`pwsh` 7); the `Bash` tool is Git Bash |
| Agent temporary storage | The session scratchpad directory |
| Bounded reads of large files | `Read` with `offset` and `limit` after a `Grep` hit |
| Broad corpus or REA search | `Explore` subagent; keep only its conclusion in context |
| Skill authoring and validation | `skill-creator` skill, then the layout validator |
| Task progress | The app task list; handoffs remain the only durable continuity |
| Work on `main` | Never enter a worktree; `EnterWorktree` is denied |

Claude Code auto memory is personal to the user and machine. It records user
preferences, not repository rules. Move a durable repository rule into
`AGENTS.md` or a canonical skill, then delete the duplicate memory.

## Working conventions for either agent

- Reuse the selected checkout and preserve its dirty changes. This document
  does not impose a branch policy absent from `AGENTS.md`. A fresh worktree
  may lack ignored runtime inputs such as `tmproject/client748/WYD.exe`;
  inspect prerequisites before treating fixture failures as product regressions.
- Keep scratch output in the agent's temporary directory or ignored build
  directories, never in `tmproject/` or `wydgo748/`. Recorded paths such as
  `%TEMP%\codex-wyd748-lifecycle-149205b7` in research records and handoffs are
  historical evidence locations, not a required naming scheme.
  Root `build/<task>/` holds generated staging, audit reports, logs, and temporary databases.
  The client may write `client-debug.log` beside its executable and crash
  dumps to `tmproject/client748/dumps/`. Older clients may leave
  `crash-monitor-*.log` files there. Retain these files until the incident
  closes, or for 30 days when no incident exists. Never commit or package them.
  Retain `tmproject/client748.zip` only while its matching build or receipt is
  under review. Remove it after publication verification.
  Cleanup examples include staging, verification downloads, stopped temporary
  clusters, and unused candidate executables created by the current task.
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

## Writing guidance

The writing requirement is in [AGENTS.md](../AGENTS.md).
It covers comments, diagnostics, logs, tests, scripts, rules, skills, documentation, research, and handoffs.
Identifiers also remain in English, subject to compatibility requirements.
Product content keeps its declared locale in the client, launcher, server messages, website, and releases.
ASD-STE100 does not replace the product's EN/PT language selection.

The [official ASD-STE100 source](https://www.asd-ste100.org/about_STE.html)
describes writing rules and a controlled dictionary.
Use the official standard when checking vocabulary and permitted technical terms.
This page is a working guide, not a replacement for that standard.

During review:

1. Separate instructions from descriptions.
2. Use imperative verbs for instructions.
3. Put each instruction in a separate sentence.
4. Check the 20-word instruction and 25-word description limits.
5. Replace ambiguous references with specific names.
6. Use the same technical term for the same concept.
7. Check word meanings against the controlled dictionary.
8. Preserve identifiers, commands, protocol values, and paths.
9. Check that shorter text retains scope, conditions, exceptions, and required evidence.

Sentence counts alone do not establish ASD-STE100 compliance.
Record dictionary or terminology checks that remain incomplete.
Review changed sections first; unrelated legacy text does not require a full-file rewrite.

For example, replace "A successful build does not prove the launcher or player used that build"
with "Verify the installed executable separately from the build."
Keep the executable path and hash requirements in the relevant rule.

## Integration command examples

These examples support the validation requirements in `AGENTS.md`.
They do not authorize a build, runtime replacement, or game launch.
Run them from the repository root only when the affected scope requires them.

For a Go integration batch:

```powershell
Push-Location .\wydgo748
try {
    go test -count=1 ./...
} finally {
    Pop-Location
}
```

For a client build without runtime replacement, use PowerShell 7:

```powershell
pwsh -NoProfile -File .\tmproject\Build-Client.ps1 -NoDeploy
```

When runtime replacement is authorized, omit `-NoDeploy`.
The script then installs and verifies `tmproject/client748/WYD.exe`.
Running the built client is a separate validation step.
Record its path, SHA-256, dependencies, scenario, and observed result.
See [build and integration](build-and-integration.md) for additional build details.

## Evidence terminology

Catalog states are `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`, `IMPLEMENTED`, and `CLIENT_TESTED`.
They distinguish discovery, contract knowledge, implementation, and actual client execution.
Builds, static checks, and isolated tests do not establish `CLIENT_TESTED`.
The execution requirement remains in `AGENTS.md`.

Server, documentation, organization, tooling, and build changes can preserve all legacy client boundaries.
Those internal changes do not require new native research under the root evidence rules.
Changed boundaries still require the evidence specified there.

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
