# WYD-Go repository rules

This is the only active repository rule file.
Keep the root `CLAUDE.md` as an import of this file.
Do not add rule files inside source trees.
Keep procedures and examples in linked documents or skills.
Treat archived instructions as historical documentation.
Use [agent environments](DOCS/agent-environments.md) for environment guidance.

## Fast task routing

Read this file once. Read each selected skill entrypoint once per turn.
Read a reference only when the selected route requires its decision or schema.
Reopen a file only after a relevant edit, failure, or context compaction.

| Task | Read | Skip |
| --- | --- | --- |
| Answer or inspection | Requested files | Skills and product builds |
| Rules, docs, or skills | `repo-architecture`, changed references | Product code and runtime checks |
| Go or integrated feature | `wydgo748-feature`, affected topic references | Full research corpus |
| Legacy client contract | `wyd-client748-research`, required evidence references | Catalog when a valid seed exists |
| Native census or root selection | `wyd-client748-catalog`, catalog strategy when needed | Full adaptation procedure |
| Client assets | `wyd-client748-assets`, research only for an affected legacy boundary | Unrelated asset or UI references |
| Publication or deployment | `wyd-release`, affected DOCS sections, used scripts | Feature and research skills unless required |

Use the smallest route that preserves the required evidence and validation.
Do not turn a one-off failure into a permanent procedure.

## Language and writing

- Write identifiers and engineering prose in English.
- Use ASD-STE100 vocabulary, active voice, and imperative instructions.
- Keep instructions under 20 words and descriptions under 25 words when practical.
- Support English (`EN`) and Brazilian Portuguese (`PT`) in changed product content.
- Use locale resources and English fallback; never derive channel or citizenship from language.
- Translate only changed sections of legacy engineering prose.
- Preserve protocol bytes, resource IDs, persisted values, and established identifiers.
- Check locale keys, encoding, placeholders, layout, fallback, selection, and actual coverage.

## Layout and ownership

- Build the native server as an executable and run it from its wydgo748 working directory.
- Do not patch or deliver historical binaries.
- Keep active source roots limited to `tmproject/` and `wydgo748/`.
- Keep the adapted C++ client and runtime assets under `tmproject/`.
- Keep authoritative game state and tests under `wydgo748/`.
- Validate client intentions on the server with versioned, testable contracts.
- Keep durable documentation in `DOCS/`, indexed by `DOCS/README.md` and its map.
- Keep skills in `.agents/skills/`, evidence in `.agents/research/`, and handoffs in `.agents/handoffs/`.
- For native server work, read `references/decompilacao server/dlls/rea-analysis/README.md` first.
- Keep `references/` local-only and preserve original binaries and third-party sources.
- Keep evidence scope under `references/`; separate static evidence, proven behavior, and client 7.48 compatibility.
- Use Docker, Railway, or external deployment manifests only when requested.

## Compatibility and evidence

- Classify each functional change once: `PARIDADE_NATIVA`, `MODERNIZACAO_COMPATIVEL`, or `EXTENSAO_COORDENADA`.
- Use native 7.48 evidence before claiming legacy parity; reuse valid records for unchanged contracts.
- Do not require native equivalents for new coordinated contracts or internal compatible changes.
- Keep catalog entries distinct from `CLIENT_TESTED` execution evidence.
- Treat `RuntimeID` as a channel boundary for entities and instances.
- Keep client 7.48 and server 7.59 evidence separate.
- Document account, guild, and persistence coordination as shared only where the contract defines it.
- Distinguish static facts, runtime observations, automated tests, and client acceptance.
- Distinguish source, built, installed, published, active, and gameplay evidence.
- Do not invent missing recipes, probabilities, timers, rewards, or descriptive text.

## Work and authorization

- Before nontrivial work, check `git status --short`, `HEAD`, and the scoped diff once.
- Define packets, state, validation, failures, lifecycle, and ownership before cross-project UI or effects.
- Make focused changes and run focused checks; preserve unrelated dirty files.
- Reuse evidence and results; rerun only for changed inputs, failures, coverage gaps, or integration needs.
- After two attempts without new evidence, change the method or report the blocker.
- Preserve bounded failure diagnostics with exit status and causal error.
- Treat diagnosis and review as inspection-only.
- A correction does not authorize publication, restart, commit, push, or security changes.
- Use `wyd-release` for deployment, SSH, publication, and authorized restart diagnosis.
- Never guess credentials, disable host verification, or change firewalls to bypass refusal.
- Do not launch the game or replace an installed executable without authorization.

## Client admission

- Treat every published client patch as mandatory.
- Raise `ClientBuildContract.h` revision and `client_build.go` minimum together.
- Set both to the last published revision plus one in the client change commit.
- Publish and verify the patch before activating a server release that requires it.
- Require explicit restart or reload authority for activation.
- Never lower or reuse a revision; reset only with an owner-requested full-client reset and new prefix.

## Files, outputs, and cleanup

- Put generated task output in ignored root `build/<task>/`.
- Put client builds, tests, tools, and native research output in `tmproject/build/`.
- Put Go executables in root `build/` or system temporary storage with absolute paths.
- Never use `wydgo748/build/`, `wydgo748/bin/`, or arbitrary `bin/` and `obj/` output.
- Build clients through `tmproject/Build-Client.ps1` or the solution; never `tmproject/TMProject748/build/`.
- Keep scratch files in agent temporary storage; keep product crash evidence in ignored `tmproject/client748/dumps/`.
- Run Python with `-B` or `PYTHONDONTWRITEBYTECODE=1`.
- Do not commit generated executables or duplicate documentation.
- Never delete `references/`, `tmproject/build/tools/`, or `tmproject/build/native-research/`.
- Resolve exact cleanup paths and stop when policy blocks cleanup.
- Add legitimate output locations to `.gitignore` and the layout validator allowlist.

## Validation and delivery

Select checks by risk and affected consumers, then run each affected batch once.

| Change | Minimum checks |
| --- | --- |
| Rules, docs, or skills | Links, layout, changed-skill validation, `git diff --check` |
| Scripts or build | Changed path and relevant failure; targeted build when output changes |
| Internal server | Changed flow, package, and consumers; full Go suite and vet for shared integration |
| C++ source | Focused tests and incremental affected-target build |
| Wire, ABI, or coordinated feature | Both contracts, rejection/failure tests, sizes, affected builds, integrated flow |
| Persistence, economy, or concurrency | Rejection, rollback, repetition, concurrency, database, and race checks |
| Assets | Identity, format, IDs, loader, fallback, and client flow |

Use PowerShell 7 and `-NoDeploy` for client build-only checks.
Use clean rebuilds only for dependency or configuration needs.
Record skipped checks and missing prerequisites as pending.
For replicated state, cover owners, observers, creation, updates, removal, scene changes, and relogin.
Report `STATICALLY VERIFIED`, `AUTOMATED TESTED`, and `CLIENT-TESTED` separately.
Report changed and removed files, checks, and pending requirements.

## Git

Work directly on `main`.
Do not create branches, worktrees, or PRs for ordinary repository work.
Do not use `git reset --hard`, `git checkout --`, or broad deletion.
When the user asks for a commit, commit the whole pending working tree on `main`.
Do not limit a commit to the files of the current task.
Never commit secrets, credentials, environment files, private runtime state, or prohibited generated outputs.
Scan the staged diff for these files before each commit.
Use handoffs only for continuation or an independent blocker.
