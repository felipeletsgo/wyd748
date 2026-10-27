# WYD-Go repository rules

This is the only active repository rule file.
Keep the root `CLAUDE.md` as an import of this file.
Do not add rule files inside source trees.
Treat archived instructions as historical documentation.
Keep procedures and examples in linked documents or skills.
See [agent environments](DOCS/agent-environments.md) for examples and writing guidance.

## Language and writing

- Write identifiers and engineering prose in English.
- Use ASD-STE100 writing rules and approved vocabulary for English engineering prose.
  Use consistent software technical names and verbs.
  Use active voice and imperative verbs for instructions.
  Write one instruction per sentence.
  Limit instruction sentences to 20 words and descriptive sentences to 25 words.
  Use one term for each meaning.
  Check changed prose. Record unreviewed legacy sections.
- Support English (`EN`) and Brazilian Portuguese (`PT`) in changed product content.
  Preserve the multilingual client and shared assets.
  Do not treat supported Portuguese as migration debt or a release blocker.
- Use locale resources and the existing language selection mechanism.
  Use English when a translation is unavailable.
  Do not select a channel or citizenship from the language preference.
- Translate legacy engineering prose only in changed sections.
  Do not translate an entire file for a localized change.
- Preserve protocol bytes, resource IDs, persisted values, and established identifiers.
  Review compatibility before translating contract values.
  Test affected contracts.
- Check changed locale keys, encoding, placeholders, layout, fallback, and language selection.
  Report actual locale coverage. Do not claim a partial translation is complete.

## Architecture and ownership

- Keep only two active source roots: `tmproject/` and `wydgo748/`.
- Keep the C++ client in `tmproject/` and runtime assets in `tmproject/client748/`.
  Treat TMProject 7.69 as secondary evidence, not 7.48 contract proof.
- Keep authoritative game state in the Go server under `wydgo748/`.
  Validate client intentions on the server.
  Define versioned, testable contracts on both sides.
- Build the server as a native executable.
  Run it from `wydgo748/`.
  Use Docker, Railway, or external deployment manifests only when the user requests them.
- Keep durable documentation in `DOCS/`.
  Use [the documentation entry point](DOCS/README.md) and [inventory](DOCS/documentation-map.md).
- Keep skills in `.agents/skills/`, evidence in `.agents/research/`, and continuity records in `.agents/handoffs/`.
- Keep native server REA work in `references/decompilacao server/dlls/rea-analysis/`.
  Read its `README.md` first for native server functions or logic.
  Reuse matching evidence before new queries.
- Keep `references/` local-only.
  Preserve original binaries and third-party sources.
  Update derived research within the authorized scope.
  Do not deliver or patch historical binaries.

## Evidence and compatibility

- Classify each functional change once:
  - `PARIDADE_NATIVA`: behavior proven through the native 7.48 client contract.
  - `MODERNIZACAO_COMPATIVEL`: internal improvement that preserves the 7.48 contract.
  - `EXTENSAO_COORDENADA`: a new contract implemented in client and server.
- Use native 7.48 binary and Ghidra evidence before adapting legacy client boundaries or claiming parity.
  Reuse records for unchanged inputs and contracts.
- Validate historical data, formulas, and behavior independently through the 7.48 contract.
  Record runtime observations or reproducible tests.
  External legacy projects alone do not prove parity.
- Study the legacy boundaries crossed by a coordinated extension.
  Do not require native equivalents for new behavior.
- Do not require new research for internal changes that preserve legacy client boundaries.
- Keep catalog states distinct.
  Catalog entries do not prove understanding.
  Require actual execution in the built client for `CLIENT_TESTED`.

## Work process

1. Read the applicable skill before nontrivial work.
   Check these rules, `git status --short`, `HEAD`, and the scoped diff once.
   Preserve existing changes.
   Read handoffs only for continuations. The current tree takes precedence.
2. For a legacy client boundary, use `catalog -> call graph -> observable flow -> adaptation -> validation`.
   Do not port later-version pseudocode without evidence.
3. Define packets, state, server validation, failures, and lifecycle before connecting UI or effects across projects.
4. Implement active client changes in source and assets.
   Make focused changes. Run focused tests.
   Do not refactor unrelated code.
5. Prove wire, ABI, resource, lifecycle, or flow incompatibility before removing existing code or assets for parity.
6. Separate proven behavior, secondary algorithms, and product choices.
   Do not invent missing recipes, probabilities, timers, rewards, or descriptive text.
   Ask only for a missing choice that changes the outcome.
7. Reuse valid reads, evidence, and tests across patches and context compaction.
   Repeat only for changed inputs, failures, insufficient coverage, or integration requirements.
   Do not rehash unchanged evidence at task entry.
8. After two attempts without new evidence, change the method or report the blockage.
   Correct the input or environment before repeating a failed command.
   Do not treat a previous failure as approval.
9. Complete supported work. Record independent gaps without blocking unrelated safe changes.
   Update existing records for changed decisions or results.
   Do not create a handoff for each patch.
   Do not copy logs into source.
10. Preserve full failure diagnostics in permitted output locations.
    Report bounded excerpts, exit status, and the causal error.
    Do not infer success from exit status, empty test selections, or file existence alone.

## Scope and delivery

- Treat diagnosis and review requests as inspection only.
  A correction request does not authorize publication, disconnection, restart, commit, push, or security changes.
  Obey client-only and no-restart restrictions.
- Distinguish source, built candidates, installed runtimes, published patches, active VPS releases, and gameplay evidence.
  Record executable paths, SHA-256 values, and resource or server dependencies.
  A build does not identify the player's executable.
- Verify artifact, version, and original reproduction before repeating a fix.
- Check replicated state for owners and observers where applicable.
  Cover creation, incremental updates, removal, scene changes, and relogin.
- Report each issue as implemented, validated, published, or pending.
  Limit user validation to the observed scenario.
  Do not close unrelated gaps after a successful deployment.
- Use `wyd-release` for publication, restart, and SSH deployment diagnosis.
  Recover the account, host, and key from established configuration or receipts.
  Do not guess accounts.
  Do not replace credentials to bypass a refusal.
  Do not disable host verification.
  Do not change firewalls to bypass a refusal.

## Mandatory client updates

- Treat every published client patch as mandatory.
- Raise `client_build::Revision` in `tmproject/TMProject748/internal/wire/ClientBuildContract.h` with `MinimumClientBuildRevision` in `wydgo748/internal/wire/client_build.go`.
  Set both to the last published revision plus one in the client change commit.
- Publish and verify the patch before activating the server release that requires it.
  Follow [client build admission](DOCS/login-dispatch.md).
  Never lower, reuse, or disable the gate.
- Reset numbering only for an owner-requested full-client reset.
  Use a new build prefix that rejects all previous published identities.

## Files, outputs, and cleanup

- Keep documentation, research scripts, dumps, logs, and temporary files outside source trees unless required by the product.
  Fix links after moving documentation.
  Update the central inventory.
  Do not duplicate documents.
  Do not commit generated executables.
- Use ignored root `build/<task>/` for generated task output.
  Use `tmproject/build/` for client builds, tests, Ghidra tools, and native research output.
- Build clients through `tmproject/Build-Client.ps1` or the solution.
  Do not build into `tmproject/TMProject748/build/`.
- Build Go executables into root `build/` or system temporary storage, with absolute output paths.
  Do not use `wydgo748/build/`, `wydgo748/bin/`, or arbitrary `bin/` and `obj/` directories.
- Keep scratch scripts, dumps, screenshots, and downloads in agent temporary storage.
  Product crash evidence may remain under `tmproject/client748/dumps/`; keep it ignored and local.
  Run Python with `-B` or `PYTHONDONTWRITEBYTECODE=1`.
- Remove task outputs when no longer needed, including remote `/tmp` staging.
  Keep hashes, sizes, and results in existing records.
- Never delete `references/`, `tmproject/build/tools/`, or `tmproject/build/native-research/`.
  Preserve incremental outputs, tracked files, accounts, character state, runtime settings, and needed evidence.
  Resolve exact paths before deletion.
  Check each resolved path.
- Stop cleanup when execution policy blocks it.
  Do not bypass the block through another shell, API, path, or script.
  Report exact leftovers separately from delivery results.
- Add legitimate output locations to `.gitignore` and the layout validator allowlist.
  Do not leave unexplained exceptions.

## Skills by scope

Use canonical skills under `.agents/skills/` for procedures.
Keep `.claude/skills/` as pointers with matching names and descriptions.
Do not add procedures to those pointers.

| Scope | Skill |
| --- | --- |
| Organization, documentation, rules, skills | `repo-architecture` |
| Server and integrated features | `wydgo748-feature` |
| Legacy client contracts | `wyd-client748-research`; add `wydgo748-feature` for integrated contracts |
| Native census and root selection | `wyd-client748-catalog` |
| Runtime assets and resources | `wyd-client748-assets` |
| Publication, restart, deployment SSH diagnosis | `wyd-release` |

## Validation

Select checks by risk and affected consumers.
Use focused tests for functional changes.
Run integration checks once per affected batch.
Broaden checks for failures, shared dependencies, or insufficient coverage.

| Change | Required checks |
| --- | --- |
| Documentation, rules, skills | Links, layout, changed skills, `git diff --check`. No product build, Ghidra, or runtime. |
| Scripts, build, CI | Changed path and a relevant failure. Build only for changed output or dependency resolution. |
| Internal server | Changed flow, package, consumers. Full Go suite and vet for integration or shared components. |
| C++ source | Focused tests and incremental affected-target build. Integrated build per executable or project batch. |
| Wire, ABI, coordinated feature | Both contracts, rejection and failure tests, sizes, affected builds, and integrated flow. |
| Persistence, economy, concurrency | Applicable rejection, rollback, repetition, concurrency, database, and race checks. |
| Assets | Identity, format, IDs, loader, and visual client flow. Rebuild only for changed generation, packaging, or build dependencies. |

- Use clean rebuilds only for dependency or configuration needs.
- Record skipped checks and missing prerequisites as pending.
  Do not repeat unchanged checks while prerequisites remain missing.
- Do not start broad audits or publication after ordinary edits without authorization.
- Use PowerShell 7 for client build scripts.
  Pass `-NoDeploy` for build-only validation.
  Replace `tmproject/client748/WYD.exe` or launch the game only when authorized.
  Verify the installed executable when deployment is enabled.
- Report `STATICALLY VERIFIED`, `AUTOMATED TESTED`, and `CLIENT-TESTED` separately.

## Git and completion

Work directly on `main`.
Do not create a branch, worktree, or PR to split the mapping.
Do not use `git reset --hard`, `git checkout --`, or broad deletion.
Report changed and removed files, checks run, and pending requirements.
