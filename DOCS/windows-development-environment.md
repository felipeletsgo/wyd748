# Windows development environment for WYD-Go

This guide covers setup of the current checkout on Windows. For dependency
versions, consult `wydgo748/go.mod`, `wydgo748/web/portal/package.json`, and
the `.vcxproj` projects first. Do not reuse paths, executables, or credentials
from earlier installations.

## Tools

- Git and PowerShell 7 (`pwsh`);
- the Go version declared in `wydgo748/go.mod`;
- Node.js compatible with `wydgo748/web/portal/package.json`, and `npm`;
- Visual Studio with MSBuild, x86 C++ tools, and the toolset specified in
  `tmproject/TMProject748/TMProject748.vcxproj`;
- PostgreSQL and `psql` for server persistence;
- Python and Java only when native/Ghidra research requires them.

The client uses the DirectX dependencies already present in
`tmproject/TMProject748/Dependencies/Directx`. Docker, CMake, and Ninja are
not requirements of the current architecture. Keep the Ghidra project and
its data outside the repository; `references/client748/` contains read-only
historical evidence.

## Prepare the checkout

Run from the repository root:

```powershell
git status --short
git rev-parse HEAD
pwsh -NoProfile -File .\tools\repository\Test-RepositoryLayout.ps1
go version
node --version
npm --version
psql --version
```

Preserve local changes before updating or moving the checkout. A fresh clone
does not replace untracked or ignored files, Ghidra projects, or secrets.
Keep credentials out of Git. See [Server operations](server/operations.md) for
`WYD_DATABASE_URL`, database setup, and backups.

## Build and validate

The procedure and integration gates are in
[Build and integration](build-and-integration.md). To compile the client
without replacing the runtime executable:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 -NoDeploy
```

Without `-NoDeploy`, the script installs the candidate as
`tmproject/client748/project.exe`; use that mode only when the task includes
replacing the runtime. Builds and automated tests do not amount to visual
client validation (`CLIENT-TESTED`).

Run the server with `wydgo748/` as its working directory. To connect the
client, generate `tmproject/client748/serverlist.bin` with the external
server-list editor and the server's reachable address. Run the client from
`tmproject/client748/`, because it resolves assets relative to the runtime
directory. The connection contract and operating steps are in
[Build and integration](build-and-integration.md) and
[Server operations](server/operations.md).
