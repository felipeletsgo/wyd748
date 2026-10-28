---
name: wyd-release
description: Package, publish, verify, or diagnose WYD client patches and VPS releases, including SSH access and authorized service restarts. Use for deployment work, not ordinary implementation or documentation review.
---

# WYD release delivery

Use existing deployment scripts and receipts.
Read only the affected sections of DOCS/client/launcher.md, DOCS/build-and-integration.md, and used tools/deployment scripts.
Historical receipts inform provenance but do not prove current host, feed, account, or active version.

## Scope and authority

- Recover the requested issue, components, artifact, destination, and prior checks before acting.
- A review or SSH diagnostic does not authorize upload.
- Publishing does not authorize restart, disconnect, commit, push, credential reset, firewall change, or reboot.
- Respect client-only and no-restart requests; record missing authority as pending.
- Preserve unrelated dirty files, private runtime state, account state, and database state.
- Do not infer the tested executable from its name or timestamp.
- Separate local testing, packaging, feed publication, server activation, and gameplay acceptance.

## SSH

- Resolve account, host, port, identity, and host key from current configuration and the latest receipt.
- Use noninteractive authentication, strict host-key verification, explicit configured identity, and bounded timeouts.
- Never print secrets or cycle through accounts, keys, or passwords.
- Distinguish timeout, refusal, banner closure, key rejection, host-key mismatch, and remote permission errors.
- After interrupted transfer or command, inspect remote staging and live state before retrying.
- After two unchanged failures, change the diagnostic method or report the blocker.

## Client admission

The server minimum must match every published client patch that raises the revision.

1. Read the last published revision from the latest launcher receipt.
2. Set ClientBuildContract.h Revision and internal/wire/client_build.go MinimumClientBuildRevision to that value plus one.
3. Keep the prefix and protocol; never lower, reuse, or wrap a revision.
4. Run go test ./internal/wire and the client admission gate.
5. For committed packaging, build the candidate and patch from the exact same client To commit.
6. Publish and verify the patch before activating the matching server release.
7. Activate only with explicit restart or reload authority.
8. Verify current-client admission and record revision, active hashes, and rollback pair.

## Package inputs

- Standard packaging uses Build-ClientPatch.ps1 with explicit committed From and To revisions.
- It must not package arbitrary working-tree bytes or ignored executables.
- An explicitly authorized working-tree package may select dirty or ignored files.
- For that path, record HEAD, dirty scope, entry paths, sizes, SHA-256, build inputs, dependencies, and provenance.
- Mark it working-tree, inspect the archive, and do not pass it to the Git-revision builder.
- Do not stage or commit silently or include unrelated dirty files.
- Inspect dependency closure: executable, descriptions, tables, meshes, textures, UI, shaders, and loaders.
- Preserve path casing and launcher names; do not assume an executable-only ZIP includes required assets.
- Exclude credentials, account state, local settings, unrelated files, launcher self-updates, and update.dat.
- Preserve config.txt and ClientSettings.txt; deletions need an explicit migration strategy.
- Read the live feed before allocating a version; use a new maximum and never overwrite a published ZIP.
- After an ambiguous timeout, verify the intended version and hash before creating another version.
- Build affected portal distributions before packaging them.
- Build-VpsRelease.ps1 obtains runtime data through Git; inspect its actual archive.
- Keep site news, manifest, version, and the advertised ZIP consistent.
- If a client-only scope excludes the website receipt, report that step pending.
- Do not publish an incompatible client while required server activation is deferred.

## Publish and activate

1. Prepare hashes, archive inspection, rollback reference, and required backups.
2. Publish artifacts before feed, site, or manifest references.
3. Preserve old feed entries and confirm the advertised ZIP exists.
4. Verify the patch before server activation when the minimum revision changes.
5. Activate only the matching server release through authorized restart or reload.
6. Determine the active version before retry or rollback after failure.
7. Stop when recovery requires new authority.

Feed publication delivers client bytes; it does not activate the server.
A copied server binary is not active until its process uses it.
Do not report success from exit code, empty tests, staging-file existence, or SSH upload alone.

## Verify, record, and clean

- Download the advertised ZIP and verify feed, URL, archive, and entry hashes.
- Check applicable HTTP/HTTPS metadata and readiness.
- For activated services, verify logs and the running release identity and hash.
- Update the existing receipt with provenance, hashes, public verification, active components, restart result, rollback, and pending gates.
- Report source, STATICALLY VERIFIED, AUTOMATED TESTED, local runtime installation, publication, active server, and CLIENT-TESTED separately.
- Remove only this task's verified temporary staging and downloads.
- Preserve rollback artifacts and incremental outputs; report exact leftovers when cleanup is blocked.

## Current script limits

- Both feed publishers currently parse non-header lines as numeric, while the native launcher accepts NOTIC directives.
- If valid non-version directives exist, stop for a scoped publisher correction; never strip them.
- Respect the 8 KiB feed limit.
- Publish-ClientPatch.ps1 may report an old feed after a remote error and may suppress cleanup errors.
- Independently verify live state and staging paths.
