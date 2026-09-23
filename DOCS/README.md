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
| Set up Windows development | [Windows environment](windows-development-environment.md) |
| Start the server for the first time | [Root README](../README.md#quick-start-on-windows) |
| Operate the database, accounts, panel, network, and backups | [Server operations](server/operations.md) |
| Build and validate the server and client | [Build and integration](build-and-integration.md) |
| Understand the attribute contract | [Score](SCORE.md) |
| Check server state | [Server operations](server/operations.md) and code/tests in `wydgo748/` |
| Check open 7.48 adaptation work | [Parity handoff](../.agents/handoffs/client748-parity.md) and records in `.agents/research/client748/` |
| Adapt the 7.48 client | [Client port](client/port-748.md) |
| Look up known protocols | [Opcode catalog](wire-opcode-catalog.md) |
| Maintain documentation organization | [Repository rules](../AGENTS.md) and [inventory](documentation-map.md) |
| Track the English-only migration | [Language migration](language-migration.md) |

The schedule, rewards, commands, and open guild-war work are documented in
[Guild wars](guild-wars.md). The panel and platform plan is in
[WYD Web Platform](WYD-WEB-PLATAFORM.md); executable instructions for the
integrated panel remain in [Server operations](server/operations.md), so the
product plan does not become a competing tutorial.

Functional and architectural differences found in the static W2PP audit
against the current server are documented in
[W2PP versus WYD-Go](w2pp-go-gap-analysis.md). W2PP is a comparison reference,
not an authority on 7.48 parity.

Old status and planning snapshots have been removed from active documentation.
Code, tests, and evidence records take precedence over dated records;
automated builds are not equivalent to validation in the client.

## Organization

- Architecture, status, protocol, client, and server documentation belongs in
  `DOCS/`.
- Technical evidence for agent work belongs in `.agents/research/`; handoffs
  belong in `.agents/handoffs/`.
- Operational skills belong in `.agents/skills/`.
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
