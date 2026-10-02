# Agent token efficiency and Jev integration

Audit date: 2026-10-01. Repository snapshot: `77e8dcc7f1af929be1c6418d08b97c7dd8412713`.
This is a development-tooling proposal, not a gameplay change or a measured
claim of Jev savings in WYD-Go. No Jev integration has been installed or tested.

## Decision

Use deterministic filtering first, then trial Jev for ambiguous evidence
selection **before** large results reach Codex. Keep Codex responsible for
implementation, integration reasoning, and interpreting validated evidence.
Do not add an AI dependency to the client, server, or staff panel.

Jev returns typed decisions rather than code or prose; it is not a drop-in
replacement for a coding agent's main model. Schema guarantees do not establish
that a decision is correct. [TypeSafe coding-agent guidance](https://docs.typesafe.ai/introduction/coding-agents).
The published harness pattern uses a classifier for routing alongside an LLM,
not instead of all LLM work. [LangChain harness guide](https://www.langchain.com/blog/building-a-harness-with-jev).

## Observed bottlenecks

Physical sizes include blank lines; bytes are not tokenizer measurements.

| Input | Bytes | Lines | Lower-cost access |
| --- | ---: | ---: | --- |
| `tmproject/TMProject748/internal/game/entities/TMHuman.cpp` | 702,727 | 18,661 | Search symbols, then read the affected function and callers |
| `DOCS/client/port-748.md` | 69,112 | 1,081 | Read the relevant section, not the entire status history |
| `.agents/research/client748/flows/transport/legacy-sale-confirmation-envelope.md` | 62,320 | 911 | Select the contract, evidence, and unresolved transition |
| `AGENTS.md` | 10,636 | 196 | Read once on entry; retain mandatory rules |

Active skills inspected were approximately 1.8-3.8 KB each. Removing mandatory
skill instructions is a weaker and riskier target than bounding large source,
research, command-output, and repeated continuity reads. Existing repository
rules already require evidence reuse and prohibit repeated entry inventories.

The current thread's local journal was approximately 3.06 GB. A bounded tail
sample recorded cumulative usage of 184,117,257 input tokens, including
175,612,416 cached input tokens (95.38%), and 724,180 output tokens. Uncached
input was 8,504,841 tokens. Reasoning-output counters are a subset of output,
not another quantity to add. These are a dated journal snapshot, not an account
bill, current context size, or per-task consumption. Journal disk size itself
does not prove token usage; deleting it would not demonstrate savings.

A broad plugin/MCP CLI inventory in this audit reported approximately 240,791
tokens before truncation. That is evidence of excessive generated output, not
proof that all those tokens entered the model. Capture and filter inventories
locally; request concise fields rather than wide tables or entire catalogs.

The local, untracked `.codex/config.toml` requests a Luna model with maximum
reasoning, while recent journal metadata records `gpt-6.1-sol` with high effort.
Do not assume a configuration file controls an already-running desktop task.
Likewise, configured compaction/output limits are not proof of actual savings.
Changing the user's selected model is outside this proposal.

## Integration choices

| Approach | Assessment for this repository |
| --- | --- |
| Local `rg`, bounded reads, deduplication, concise test summaries | First layer; no provider cost or external disclosure |
| [Jev Codex Token Saver](https://github.com/jcressler/jev-codex-token-saver) | Best pilot candidate: local MCP evidence collection and selected exact excerpts before Codex ingestion; experimental community integration |
| [jev-code](https://github.com/FrancoisChastel/jev-code) | Optional later layer for batches of classification/ranking questions; asking after Codex reads everything does not save that ingestion |
| [jev-gateway](https://github.com/vinilana/jev-gateway) | Deferred: proxies a wrapped CLI session and steers tool choices; introduces an authentication/request boundary and does not establish transparent desktop integration |

Codex supports packaged plugins containing MCP tools and skills.
[OpenAI plugin documentation](https://developers.openai.com/plugins/build/plugins).
That support does not make these community integrations OpenAI-maintained.
Installing an API-teaching skill alone does not delegate the existing agent loop.

The token-saver candidate offers `search_workspace_evidence`,
`read_large_text_evidence`, and `read_selected_evidence`. It does not intercept
native tools or replace compaction. Real Jev selection reports `mode: "jev"`
with `metrics.jevRequests: 1`; `bypass` and `local-fallback` are not Jev savings.
Its exclusions are not complete secret redaction.
[Candidate behavior and boundaries](https://github.com/jcressler/jev-codex-token-saver).

TypeSafe's 2026-09-15 announcement advertised $0.042 per million input tokens
and unmetered output. Verify current terms before activation. Low Jev API cost
does not prove lower Codex cost: include extra tool calls, recovery reads,
cache behavior, and the applicable subscription/API accounting model.
[Dated TypeSafe announcement](https://typesafe.ai/blog/introducing-system-one-models-and-jev).

## Project-wide application

| Scope | Useful delegated question | Non-delegable safeguard |
| --- | --- | --- |
| C++ / 7.48 research | Which candidate functions or evidence sections address this transition? | Native authority, ABI/wire evidence, and required skill reads |
| Go server / opcodes | Which handlers, serializers, and tests belong to the requested flow? | Authoritative validation, packet sizes, rollback, and rejection tests |
| Assets | Which manifest entries and loader references match the issue? | Identity, format, resource IDs, and visual-validation requirements |
| Documentation | Which records need reading for this task? | Current source precedence and mandatory repository rules |
| Build/test logs | Which intact error blocks identify the failing dependency? | Exit status, failing tests, causal stack, and missing fixtures |

Known paths and small outputs should bypass Jev. First use scoped searches and
local deduplication. Submit only bounded ambiguous candidates, return exact
paths/line ranges, and recover omitted facts through bounded follow-up reads.
Retain conflicting evidence rather than letting a relevance score hide it.
Never let a classification promote a build to `CLIENT_TESTED`, select destructive
actions, waive a gate, or turn an incomplete protocol mapping into parity.

If caching selections, key them by question, input hashes, policy, and selector
version. Invalidate changed inputs; do not rescan immutable evidence merely to
resume work. Treat repository text and provider responses as untrusted data.

## Activation prerequisites and pilot

Node.js `v24.19.0` was available. `TYPESAFE_API_KEY` was absent from both the
process and persistent Windows user environment. No Jev tool was available in
this task; a plugin-catalog search returned no matching entry. This does not
exclude installing a community integration from its repository.

Before installation, review and pin its code/dependencies, verify project-root
containment and access to `.agents/research/`, and test secret exclusions and
failure behavior. Limit external transfer to explicitly authorized source/docs
excerpts. Exclude credentials, account data, local sessions, database exports,
native binary collections, dumps, and unrestricted logs. Keep the key outside
Git and prompts; do not paste it into chat. Provider retention terms also need
review. Installing a proxy that handles Codex authentication is not required.

Run a read-only pilot against fixed fixtures: one Go handler lookup, one C++
flow investigation, one documentation lookup, and one noisy build-log diagnosis.
Compare three arms: current workflow, deterministic selection, and the same
selection with Jev. Use three repetitions per task/arm, equivalent model and
effort, recorded cache policy, and identical quality gates. This is a proposed
benchmark, not a reason to repeat product builds or finished investigations.

Measure input, cached and uncached input, output, Jev usage, tool calls, recovery
reads, and elapsed time. Use actual usage counters; character counts are only
payload-size indicators. Report deterministic savings separately from Jev's
incremental benefit. Require all seeded critical facts and no weakened gates;
retain deterministic mode if Jev fails that test or adds cost without benefit.

Pending: authorization of excerpt transfer, provider-key setup, pinned-code
security review, actual MCP discovery, API/fallback tests, and paired savings
measurement. No project-specific savings percentage is established yet.
