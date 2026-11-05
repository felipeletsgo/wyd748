# Build and integration

The paths below start at the repository root. Relative server data paths are
resolved from `wydgo748/`.

## Server

Requirements:

- Go at the exact or compatible version specified in `wydgo748/go.mod`;
- Node.js `22.12` or later to build the Astro staff panel;
- PostgreSQL for startup and database integration tests.

The panel is enabled in the versioned `data/config/server.txt`, but
`web/portal/dist/` is generated and ignored by Git. In a fresh clone, build
the panel before first startup or disable `web_admin_enabled`.

```powershell
pwsh -NoProfile -File tools/web-admin/Start-WYDAdmin.ps1 -NoBrowser

$repoRootPath = (Get-Location).Path
$serverOutput = Join-Path $repoRootPath 'build/server'
New-Item -ItemType Directory -Force $serverOutput | Out-Null
Push-Location wydgo748
try {
    go test -count=1 ./...
    go vet ./...
    go build -o (Join-Path $serverOutput 'tm.exe') ./cmd/server
    go build -o (Join-Path $serverOutput 'account-create.exe') ./cmd/account-create
    go build -o (Join-Path $serverOutput 'player-api.exe') ./cmd/player-api
} finally {
    Pop-Location
}
```

| Output | Purpose |
| --- | --- |
| `build/server/tm.exe` | game server with integrated staff panel |
| `build/server/account-create.exe` | interactive local account registration |
| `build/server/player-api.exe` | public account, player, ranking and guild API; keep the listener private behind HTTPS |
| `wydgo748/web/portal/dist/` | built assets for the integrated panel |

Start `build/server/tm.exe` with `wydgo748/` as the current directory; starting
it from the repository root makes relative `data/...` paths point to the wrong
location.

```powershell
Push-Location wydgo748
try {
    & ..\build\server\tm.exe
} finally {
    Pop-Location
}
```
See [server operations](server/operations.md) for database, configuration,
accounts, ports, shutdown, and backups.

The deployed player API is separate from the game process and normally listens
on loopback port `8083`. `cmd/account-api` remains a legacy registration utility,
not the current public deployment entry point. The staff listener on `8082`
must also remain private. See [current deployment state](current-state.md).

### Database and race-test gates

Use a disposable local PostgreSQL database for integration tests, never the
operational database. `WYD_TEST_POSTGRES_URL` enables the store/S2 fixture tests;
they can create schema and write test data. A green package with skipped tests
does not prove those paths. Keep fixture URLs in the current shell, not in
versioned files, and record leaf skips as well as package results.

For an integration batch with supported CGO/C compiler tooling, run from the
repository root after setting `WYD_TEST_POSTGRES_URL` to the isolated fixture:

```powershell
$env:CGO_ENABLED = '1'
Push-Location wydgo748
go test -race -count=1 -timeout=300s ./...
go vet ./...
Pop-Location
```

| Conditional gate | Prerequisite |
| --- | --- |
| Store/S2 PostgreSQL tests | `WYD_TEST_POSTGRES_URL` points to a disposable writable database |
| Player API PostgreSQL tests | `WYD_TEST_PLAYER_OWNER_URL`, `WYD_TEST_PLAYER_DATABASE_URL` and `WYD_TEST_PLAYER_PRIVILEGES` identify an isolated fixture and its role grants |
| `TestStartWebAdminPostgres` in `cmd/server` | Separate `WYD_WEB_TEST_DATABASE_URL`; an initialized compatible database accessible through the read-only pool. This test does not migrate or seed it |
| `TestProjectAccountsUseCurrentSchema` in `internal/store` | Optional local `wydgo748/data/accounts/*.json` fixtures; absent/empty local accounts produce a skip. Do not publish private account snapshots to satisfy it |

The October 7 [S2 audit](server/s2-closure-audit-2026-10-07.md)
records a passing full race suite with its S2 PostgreSQL fixtures enabled, but
the two other conditional gates above skipped. That result is reusable for
unchanged code/inputs; documentation-only corrections need links/layout and
whitespace validation, not another product build or full suite. Actual
transport-loss/restart recovery remains distinct from injected worker outcomes.

## CI and runtime

The versioned workflow checks repository layout, research, assets, the Go
server with PostgreSQL, and the C++ client on Windows. It neither publishes
nor configures an external server. The supported runtime is the native
`build/server/tm.exe` process started from `wydgo748/`; the repository has no
container architecture or deployment-provider integration.

The historical `cmd/npcconvert` converter is not part of the runtime. When its
input format is needed, it requires explicit `-in` and `-out` arguments; it
does not fetch external projects or write active data by default. Review
converted results before incorporating them into `data/`.

## Client

Requirements: Windows, Visual Studio with C++ x86 and toolset v145, and the
Windows SDK. DirectX dependencies are in the client tree.

```powershell
pwsh -NoProfile -File tmproject/Build-Client.ps1 -Configuration Release -NoDeploy
```

The script builds and runs architecture, socket and crash-capture tests, builds
the game and crash reporter, and copies
`tmproject/build/TMProject748/Release/WYD.exe` to
`tmproject/client748/WYD.exe`, the executable players run through the
launcher, verifying SHA-256. It neither starts nor
stops the game. The installed executable is a product, not the native
reference.

The adapted native launcher remains separate from the game solution. Validate
its language extension with `Build-NativeLauncherLanguage.ps1 -NoDeploy`.
Its source is in `tmproject/NativeLauncher/`. The alternative launcher is removed.
Use the [native packaging route](client/launcher.md#native-full-client-packaging-2026-11-02)
for a full installation with the native launcher and signed runtime installer.
Keep the resulting full-client ZIP on the owner's external file host, not the VPS.

To validate without replacing the installed executable, add `-NoDeploy`.
In this mode, the artifact and its SHA-256 remain in `tmproject/build/`; no
result should be described as a visual test or `CLIENT-TESTED`. When client
execution is unavailable, record the runtime gate as pending rather than
inferring it from a build or static test.

## Boundary between projects

The client sends intentions; the server validates them and responds with
authoritative state. For contract changes, record direction, opcode, size,
offsets, packing, rejections, and compatibility before implementing both
ends. The 7.69 architecture may remain where it does not impose incompatible
resources or formats; do not assume equivalence between versions.

Builds and tests do not prove connection, login, world entry, or
logout/relogin. Those flows require integrated validation with the built
client and current server. Each workstream's status is recorded in the
[research records](../.agents/research/client748/README.md); the
[client port](client/port-748.md) and [opcode catalog](wire-opcode-catalog.md)
identify known gaps. A historical handoff is not a live roadmap.

## Organization

```powershell
pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1
```

The command checks the inventory, local Markdown links, unique repository
rules, and authored documentation outside source trees. Generated client
build artifacts and installed frontend dependencies are excluded. Use
`-UpdateMap` only when inventory paths, entries, or categories change;
content-only changes do not require regenerating the map.

The `references/client748/` collection is evidence; its patchers are
historical, not active development tools.

<a id="windows-setup"></a>
## Windows checkout setup

<a id="windows-setup-windows-development-environment-for-wyd-go"></a>

This guide covers setup of the current checkout on Windows. For dependency
versions, consult `wydgo748/go.mod`, `wydgo748/web/portal/package.json`, and
the `.vcxproj` projects first. Do not reuse paths, executables, or credentials
from earlier installations.

<a id="windows-setup-tools"></a>
### Tools

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

<a id="windows-setup-prepare-the-checkout"></a>
### Prepare the checkout

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

<a id="windows-setup-build-and-validate"></a>
### Local client connection

Use the [client build procedure](#client) and [integration gates](#boundary-between-projects) above.

Run the server with `wydgo748/` as its working directory. To connect the
client locally, run `tools/client-assets/New-LocalServerList.ps1` from the
repository root. It writes the ignored `serverlist.local.bin` loopback override
without modifying the shipped public `serverlist.bin`. Move the override out
of the runtime before connecting to the VPS; launcher patches do not remove
local overrides. Run `WYD.exe` or `WYDLauncher.exe` from
`tmproject/client748/`, because it resolves assets relative to the runtime
directory. See [server operations](server/operations.md) for operating steps.
