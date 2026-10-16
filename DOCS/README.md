# WYD-Go documentation

This is the entry point for the repository's durable documentation. The code
has two source trees: [`../tmproject/`](../tmproject/) is the client and
[`../wydgo748/`](../wydgo748/) is the authoritative server.

## Map

The inventory by topic and location is in
[`documentation-map.md`](documentation-map.md). It distinguishes active product
documentation, research evidence, and continuity records.

## Where to start

| Goal | Starting document |
| --- | --- |
| Resume work and distinguish local changes from the deployed release | [Current state](current-state.md) and [continuation record](../.agents/handoffs/project-continuation.md) |
| Check implemented website features and remaining product-plan gaps | [Website feature status](server/website-feature-status.md) |
| Enter Hidden Valley using an activated Hidden Ruins map | [Hidden Valley access contract](server/hidden-valley-access.md) |
| Set up Windows development | [Windows environment](windows-development-environment.md) |
| Start the server for the first time | [Root README](../README.md#quick-start-on-windows) |
| Operate the database, accounts, panel, network, and backups | [Server operations](server/operations.md) |
| Publish the public player portal and isolate its VPS services | [Public portal and native VPS deployment](server/public-launch.md) |
| Publish client updates through the launcher | [WYD launcher and client patches](client/launcher.md) |
| Build and validate the server and client | [Build and integration](build-and-integration.md) |
| Understand the attribute contract | [Score](SCORE.md) |
| Check server state | [Server operations](server/operations.md) and code/tests in `wydgo748/` |
| Track nonblocking critical persistence | [Critical persistence plan](server/critical-persistence-plan.md) |
| Check current S2 recovery gaps and their acceptance criteria | [Current recovery queue](server/critical-persistence-plan.md#current-gaps-and-acceptance-criteria) and [October 7 closure audit](server/s2-closure-audit-2026-10-07.md) |
| Handle persistence containment without unsafe replay/unlock | [Critical persistence operation](server/operations.md#critical-persistence-operation) |
| Check stack quantities, Shift-split, merging and CP/name policy | [Inventory stacks and CP contract v1](client/inventory-stack-and-cp-contract.md) |
| Check open 7.48 adaptation work | [Client port](client/port-748.md), [opcode catalog](wire-opcode-catalog.md), and the flow records in `.agents/research/client748/` |
| Adapt the 7.48 client | [Client port](client/port-748.md) |
| Understand C.C. buff scheduling and the supplied TMProject reference | [Combat Control](client/combat-control.md) |
| Plan the TMHuman source separation | [TMHuman separation plan](client/tmhuman-separation-plan.md) |
| Plan the TMFieldScene refactoring | [TMFieldScene refactoring plan](client/tmfieldscene-refactoring-plan.md) |
| Find where client code lives after the source split | [TMProject source organization plan](client/tmproject-source-organization-plan.md) |
| Review the Claude refactoring batch and proposed corrections | [Refactoring batch audit — 2026-10-03](client/refactoring-batch-audit-2026-10-03.md) |
| Review the client and Go server source audit | [Full-source audit — 2026-10-03](client/full-source-audit-2026-10-03.md) |
| Look up known protocols | [Opcode catalog](wire-opcode-catalog.md) |
| Keep the Kefra hall mirror hidden until a persisted boss defeat | [Kefra portal visibility v1](protocol/kefra-portal.md) |
| Maintain documentation organization | [Repository rules](../AGENTS.md) and [inventory](documentation-map.md) |
| Work with Codex or Claude Code | [Agent environments](agent-environments.md) |
| Track the English-only migration | [Language migration](language-migration.md) |
| Reduce agent context/token costs and evaluate Jev | [Agent token efficiency](agent-token-efficiency.md) |

The schedule, rewards, commands, and open guild-war work are documented in
[Guild wars](guild-wars.md). The [WYD Web Platform](WYD-WEB-PLATAFORM.md)
document is a dated product plan whose proposed features need rechecking
against current code. Its current implementation matrix is maintained in
[Website feature status](server/website-feature-status.md). Executable instructions for the integrated panel remain
in [Server operations](server/operations.md); the plan is not a competing
tutorial or a current implementation checklist.

Functional and architectural differences found in a static W2PP comparison
are documented in [W2PP versus WYD-Go](w2pp-go-gap-analysis.md). Recheck its
snapshot against current source before acting on a gap. W2PP is a comparison
reference, not an authority on 7.48 parity.

Historical snapshots and dated plans are not live status reports. Code, tests,
and evidence records take precedence over them;
automated builds are not equivalent to validation in the client.
The [parity handoff](../.agents/handoffs/client748-parity.md) is a historical
snapshot, not the current work queue.

## Organization

- Architecture, status, protocol, client, and server documentation belongs in
  `DOCS/`.
- Technical evidence for agent work belongs in `.agents/research/`; handoffs
  belong in `.agents/handoffs/`.
- Operational skills belong in `.agents/skills/`. The root `CLAUDE.md` and
  `.claude/skills/` only point Claude Code to `AGENTS.md` and those skills.
- `tmproject/` and `wydgo748/` are reserved for source, assets, and data needed
  for execution or builds. Do not place documentation or temporary files there.
- `testdata/protocol/` contains canonical fixtures shared by client and
  server; do not duplicate the same frame in both source trees.
- `tools/client-assets/Audit-ClientAssets.ps1` compares manifests and static
  TMProject literals against materialized assets in `tmproject/client748/`.
  Dependencies explicitly marked unavailable in manifests are known debt and
  remain diagnostic; `-FailOnMissing` fails only for missing required
  references, unclassified source gaps, or casing mismatches.

## Product principles

The target client is WYD 7.48. The available TMProject source is newer (7.69)
and is adapted only when the 7.48 contract or a coordinated extension has
been decided. The Go server validates intentions, maintains authoritative
state, and communicates with the client through a shared packet and state
contract.

Parity decisions must record native 7.48/Ghidra evidence. The documentation
map does not turn a reference into proof: each investigation's actual state is
recorded in its evidence record or corresponding report.
