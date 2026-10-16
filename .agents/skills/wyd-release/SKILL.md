---
name: wyd-release
description: Package, publish, verify, or diagnose WYD client patches and VPS releases, including SSH access and authorized service restarts. Use for deployment work, not ordinary implementation or documentation review.
---

# WYD release delivery

Use existing deployment scripts and receipts; do not invent a parallel
publisher. Read the affected sections of [launcher delivery](../../../DOCS/client/launcher.md)
and [build and integration](../../../DOCS/build-and-integration.md), plus the
scripts actually used under `tools/deployment/`. Resolve current paths and
parameters before running them. Historical receipts are evidence, not live
host, account, feed-version, or deployment-state authority.

## Scope and prerequisites

1. Recover the requested issue set, affected components, and prior validation.
   A review or SSH diagnostic does not authorize upload. Publishing does not
   authorize a restart, disconnect, commit, push, password reset, or firewall
   change. Respect client-only and no-restart requests. Record missing authority
   instead of expanding scope.
2. Identify source revision and relevant dirty changes, candidate path/hash,
   runtime assets, packaging inputs, and destination. Preserve unrelated work
   and private runtime/account state. Never infer which executable was tested
   from its filename or build timestamp alone.
3. Reuse valid focused checks. Block on failed required gates, unknown artifact
   provenance, incompatible client/server versions, or known Portuguese
   player-facing release content. Instruction-only review needs no build or
   production access. Building/installing a local candidate and running it are
   separate actions; a blocked visual test remains pending.

## SSH access without guessing

- Resolve the known deployment account, host, port and identity from existing
  configuration and the latest relevant receipt; verify against current
  configuration before use. Use noninteractive authentication, explicit
  identity selection where configured, strict host-key verification, and a
  bounded connection timeout. Do not print private keys or secret settings.
- Distinguish TCP timeout/refusal, SSH banner or preauthentication closure,
  public-key rejection, host-key mismatch, and remote sudo/path permissions.
  A refusal before authentication is not evidence that a key changed or that
  an IP ban exists. Record the observed phase and error, not a guessed cause.
- Do not cycle through usernames, keys, or passwords. Do not disable host-key
  checking or reset credentials/security policy to make deployment work.
  A trusted host-key mismatch needs independent verification. Use an available
  console only within authorized diagnosis.
- After an interrupted transfer or command, inspect remote staging and live
  state before retrying: it may already have succeeded. Retry with a corrected
  prerequisite or evidence of a transient condition. After two unchanged
  failures, change the diagnostic method or report the blocker.

## Package the actual inputs

- `Build-ClientPatch.ps1` packages explicit committed `-From`/`-To` revisions,
  not arbitrary working-tree bytes or ignored executables. Do not label such
  a package as containing an uncommitted candidate.
- For an established explicit working-tree artifact allowlist, record HEAD,
  relevant dirty scope, exact entry paths/hashes, and candidate build inputs.
  Inspect the archive itself. Do not stage or commit silently to satisfy a
  script. Use the path that represents the authorized source and dependencies.
  Establish the allowlist from the requested fixes, build output and required
  runtime dependencies, using a prior receipt only as a starting point. This
  is normal packaging within authorized publication, not permission to include
  unrelated dirty files. If provenance or component scope remains ambiguous,
  resolve that gap before packaging.
- Inspect dependency closure: executable, descriptions, item tables, meshes,
  textures, UI, shaders and other changed loaders as applicable. Compare with
  supported earlier patches. Do not assume an executable-only ZIP fixes missing
  assets or archive the whole runtime. Preserve path casing and launcher names.
- Exclude credentials, account state, local settings and unrelated files.
  Preserve `config.txt` and `ClientSettings.txt`; do not include launcher
  self-updates or `update.dat` in ordinary patches. Deletions need an explicit
  migration strategy; adding a ZIP entry does not remove old files.
- Read the live feed before allocating a version. Use a new version greater
  than its maximum, preserve existing entries, and never overwrite a published
  ZIP. A receipt's version is not a permanent default.
  After an ambiguous timeout, first check whether the intended version/hash
  is already published; if so, verify that delivery instead of creating a
  duplicate version. Preserve concurrent additions and rebuild stale feed input.
- `Build-VpsRelease.ps1` obtains its runtime-data list through Git; inspect the
  actual archive rather than assuming untracked data is included. Preserve
  private VPS configuration. Build any affected portal before packaging its
  distribution; public metadata must describe the actual patch.
- State coordinated server/client activation dependencies. Do not publish an
  incompatible client while required server activation is deferred by a
  no-restart restriction; request the missing decision.

## Publish and activate

1. Prepare checksummed artifacts and a rollback reference before downtime.
   Verify uploads and archive contents before activation. Preserve the prior
   patch/release, account state, runtime settings and database; use the
   established backup/atomic activation procedure.
2. Publish artifacts before references advertising them. Use the existing feed
   publisher so old entries survive and readers cannot see a new entry with
   an absent ZIP. Keep site news, manifest and version consistent with delivery.
   Check the launcher's website receipt requirement; if an explicit client-only
   scope excludes that component, report the outstanding site step rather than
   silently deploying it or claiming the whole release is complete.
3. Restart only authorized services, after artifacts are ready. A client patch
   normally needs launcher delivery, not a game-server restart. A copied server
   binary is not active until its process uses it. A restart request does not
   authorize unrelated host-service restarts or a VPS reboot.
4. On failure, determine the active version before retry or rollback. Use the
   established rollback path within scope; stop if recovery needs new authority.
   Never report success from exit code alone, an empty test result, or staging
   file existence.

## Verify, record, and clean

- Check the public feed and download its new ZIP through the advertised URL;
  verify ZIP and entry hashes against the candidate/allowlist. Verify applicable
  HTTP/HTTPS endpoints and site metadata. SSH upload alone is not publication.
- For activated server/API/site components, check readiness, relevant logs and
  actual running release identity/hash. Separate existing/nonblocking warnings
  from new failures. HTTP 200 alone does not identify the running game binary.
- Update the existing receipt with source/input provenance, artifact hashes,
  public verification, active components, restart outcome, rollback reference,
  and per-issue pending gates. Deployment does not prove untested gameplay or
  close unrelated work.
- Remove only this task's verified temporary staging/downloads after preserving
  durable facts. Keep current incremental outputs and rollback artifacts. If
  cleanup is denied by policy, do not try another shell/API/path workaround;
  report exact leftovers. Do not retry earlier blocked cleanup on resume.

Report source changes, `STATICALLY VERIFIED`, `AUTOMATED TESTED`, local runtime
installation, publication, active server version, and `CLIENT-TESTED` separately.

## Known script limits to check before use

At this review, both feed publishers parse every non-header line as a numeric
version, while the native launcher also accepts `NOTIC w h url`. If a live feed
contains non-version directives, stop for a scoped publisher correction; never
strip valid directives to satisfy the script. The 8 KiB feed limit also applies.

`Publish-ClientPatch.ps1` can report that the previous feed remains live after
a remote error, but a lost connection after activation does not prove that.
Its cleanup suppresses removal errors. Independently verify live state and
remaining staging paths; a success/failure message cannot replace these checks.
These are limitations to recheck against current script contents, not proof
that a particular production run failed.
