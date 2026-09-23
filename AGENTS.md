# WYD-Go — repository rules

This is the repository's only active rule file. Do not create another
`AGENTS.md`, `CLAUDE.md`, or equivalent inside a source tree. Archived rules
are documentation, not active instructions.

## Project language

English is the sole language of this global WYD project and its global client.
This applies to the entire active repository, not only player-facing UI:
client and server content, the staff panel, documentation, diagnostics, logs,
comments, tests, scripts, rules, skills, research, handoffs, and release
material must be in English. Do not add Portuguese text. Translate existing
Portuguese text in every file being changed; track untouched legacy text as
migration debt and remove it until the repository is fully in English. Preserve
protocol bytes, resource IDs, persisted values, and established identifiers
when translation would break compatibility. Never claim a partial translation
is complete.

English is a delivery requirement, not a localization option. Before closing a
change, inspect all authored text in affected files, including user-visible
text, errors, logs, comments, tests, examples, and documentation. Do not ship
new Portuguese text or use remaining migration debt to justify it. Translate
an existing contract-bearing value only with an explicit compatibility review
and the affected tests. Completion of the language migration requires an
active-repository audit, not merely an English UI or an updated rule.
Do not declare a global client/server release ready while any known Portuguese
player-facing text remains. A migration record is not a release exception.

## Active architecture

- `tmproject/` is the C++ client. `TMProject748/` contains source based on
  TMProject 7.69; `tmproject/client748/` contains the 7.48 runtime and assets.
- `wydgo748/` is the authoritative Go server, with `go.mod` as its module root.
  The client sends intentions and receives validated state; it is never the
  source of truth for game state.
- Build and run the server as a native process from `wydgo748/`. Docker,
  Railway, and external deployment manifests are outside the product
  architecture unless explicitly requested.
- `DOCS/` holds durable documentation. Entry point and inventory:
  [`DOCS/README.md`](DOCS/README.md) and
  [`DOCS/documentation-map.md`](DOCS/documentation-map.md).
- `.agents/skills/` holds working skills. `.agents/research/` and
  `.agents/handoffs/` hold evidence and continuity records, not runtime.
- `references/client748/` preserves historical binaries and tools as read-only
  evidence. `tools/repository/` holds layout validators.

The client/server contract must be explicit, versioned, and testable on both
sides. TMProject 7.69 does not prove that an address, layout, asset, opcode,
or behavior belongs to 7.48.

## Version and evidence

Classify each functional delta once. Documentation, organization, and tooling
without a contract change need no native evidence record:

- `PARIDADE_NATIVA`: behavior proven in the native 7.48 client;
- `MODERNIZACAO_COMPATIVEL`: internal improvement preserving the 7.48 contract;
- `EXTENSAO_COORDENADA`: new contract implemented in client and server.

Before adapting a legacy client boundary or claiming parity, use evidence from
the native 7.48 binary and Ghidra. Reuse proven records when they cover the
current contract and inputs; investigate only the gap. Contract-preserving
internal modernization does not require new native research. A coordinated
extension requires a new contract and study of the legacy boundaries it
crosses, not a native equivalent for the new feature. Internal server,
documentation, and build changes that do not change those boundaries do not
trigger Ghidra work. TMProject 7.69 is a secondary architecture/algorithm
reference. External legacy projects are not parity authority on their own.
Historically sourced data, formulas, or behaviors may count as
`PARIDADE_NATIVA` only after independent, recorded validation in the real 7.48
client/runtime or reproducible tests exercising the 7.48 contract. That 7.48
validation, not historical origin, is the evidence. Do not reopen Ghidra for
identical behavior already validated with unchanged inputs and contract.

The catalog states are `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT`,
`IMPLEMENTED`, and `CLIENT_TESTED`. A cataloged function is not necessarily
understood. `CLIENT_TESTED` requires actual execution in the built client;
builds, static checks, and isolated tests do not substitute for it.

## Implementation

1. Before nontrivial technical work, read the applicable `.agents/skills/`
   skill, check this file, `git status --short`, and `HEAD`, and preserve
   other people's changes.
2. For a legacy client boundary, follow
   `catalog -> call graph -> observable flow -> adaptation -> validation`.
   Do not blindly port 7.69 pseudocode.
3. For cross-project features, define packet/state, authoritative validation,
   failures, and lifecycle before connecting UI or effects.
4. Implement active client changes in source and assets. Historical binaries
   in `references/client748/` are evidence only, never implementation patches.
5. Use small cycles: focused patch, focused test, next patch. Do not refactor
   unrelated code.
6. Do not delete or replace existing code/assets merely because native 7.48
   lacks them. First demonstrate incompatibility in wire, ABI, resource,
   lifecycle, or actual flow.

## Avoid repeated work

- Enter each task once: applicable rules/skills, status + HEAD + scoped diff,
  and handoff only for a continuation. Reuse reads from this session; load
  references only for the current decision.
- After entry, advance with a patch, focused test, or concrete diagnosis. Do
  not restart inventories, research, or audits after every patch, skill switch,
  or context compaction. The current tree overrides a handoff.
- Reuse evidence with known, still-valid inputs, dependencies, environment,
  and coverage. Repeat only what a change, insufficient result, or integration
  gate invalidates. Rehashing an immutable corpus is not a resume ritual;
  a previous failure is never approval.
- Research should close the transition needed for the task, not the entire
  corpus. Implement the cleared batch; record independent gaps without
  blocking safe work.
- After two attempts on the same question without new evidence, change
  hypothesis/method for a concrete reason or report the blockage. Do not
  repeat a failed command without fixing input/environment or identifying a
  transient cause. New evidence is progress; duration alone is not a loop.
- Update an existing record when a decision, validation result, or next step
  changes. Do not create a handoff per patch or copy logs into source.

## File organization

- Do not create `.md`, research scripts, dumps, logs, generated binaries, or
  temporary files inside `tmproject/` or `wydgo748/` unless the product needs
  them. Documentation belongs in `DOCS/`; agent evidence in `.agents/research/`
  or `.agents/handoffs/`.
- Keep only two active source roots: `tmproject/` and `wydgo748/`.
- Keep builds and local artifacts in ignored directories (`build/`, `bin/`,
  `obj/`, or existing equivalents). Do not commit generated executables.
- When moving documentation, fix links and update the central map. Do not
  duplicate a document.

## Skills by scope

| Scope | Skill |
| --- | --- |
| Organization, documentation, rules, skills | `repo-architecture` |
| Server and integrated features | `wydgo748-feature` |
| Legacy client contracts and behavior | `wyd-client748-research`; add `wydgo748-feature` for server/integrated contracts |
| Native census and root selection | `wyd-client748-catalog` |
| Runtime assets and resources | `wyd-client748-assets` |

Skills detail procedures; this file holds global rules.

## Proportional validation

Choose gates by risk and affected consumers, not file count. Pair each logical
patch with a focused test. Run integration validation once per affected batch
without repeating still-valid results. Failure, shared dependencies, or weak
coverage justify a broader gate.

| Change | Required gate |
| --- | --- |
| Documentation/rules/skills only | Check links/layout once per batch, validate changed skills, and run `git diff --check`. No Go, C++, Ghidra, or runtime. |
| Script or build/CI | Test the changed path and a relevant failure. Build the product only when its output/dependency resolution changed. |
| Internal server | Test changed package/flow and consumers. Full Go suite and vet for cross-cutting/integration batches, not every patch. |
| C++ source | Focused test and incremental affected-target build; integrated build for an executable/project batch. Clean rebuild only for dependency/configuration needs. |
| Wire/ABI or coordinated feature | Contracts and tests on both sides, including rejections, sizes, and failures; affected builds and an integrated flow before claiming end-to-end functionality. |
| Persistence/economy/concurrency | Also test applicable rejection, rollback, repetition, and concurrency; use database/race validation when the path needs them. |
| Assets only | Validate identity, format, IDs, and resource loader; visual client flow. Rebuild only when generation, packaging, or build dependencies changed. |

Skipped tests caused by missing fixtures/services do not prove a path. Record
the pending gate; do not promote validation or rerun the suite hoping the
prerequisite appears. Broad audits and publication are not automatic after
ordinary edits.

### Integration commands

From the repository root:

```powershell
Push-Location .\wydgo748
go test -count=1 ./...
Pop-Location
```

To build and install a candidate in the local runtime (overwrites
`tmproject/client748/project.exe`; only when the task includes that validation,
not for documentation checks):

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1
```

The script must produce/verify `tmproject/client748/project.exe` without
altering historical evidence. Report `STATICALLY VERIFIED`, `AUTOMATED TESTED`,
and `CLIENT-TESTED` separately; a build alone never promotes a state.

## Git and delivery

Work directly on `main`; do not create a branch, worktree, or PR to split the
mapping. Do not run `git reset --hard`, `git checkout --`, or broad deletion.
Before deleting anything, resolve exact paths and preserve evidence. At
completion, report changed and removed files, commands run, and pending gates.
