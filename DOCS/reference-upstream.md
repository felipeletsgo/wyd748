# Upstream provenance

This tree was imported from:

- Repository: <https://github.com/lorransouza/TMProject-Free>
- Branch: `main`
- Commit: `14937eda00d462c06dfd3a1ac5905f910d70a804`
- Import date: 2026-08-20
- Imported scope: `Source cliente/` (original upstream path)

The external repository's Git history, releases, prebuilt client, and server
source were not imported.

Upstream declares its code to be under GNU GPL v3 in its README. WYD-Go is
also distributed under GNU GPL v3; the root `LICENSE` applies to this modified
copy. Preserve upstream attribution when distributing this source.

This copy is WYD-Go's only source-built WYD 7.48 client. Review future upstream
updates field by field, and port only semantics proven for 7.48 in each commit.
Never bulk-import ABI definitions, packets, controls, loaders, or 7.59/7.69
branches, or maintain a second target version. This directory is not a
submodule and must not receive its own `.git` directory.

Historical executables and patchers do not participate in upstream sync,
builds, or validation. Every active change belongs in this source or in assets
consumed directly by `tmproject/client748/project.exe`.
