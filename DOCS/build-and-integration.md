# Build and integration

The paths below start at the repository root. Relative server data paths are
resolved from `wydgo748/`.

## Server

Requirements:

- Go at the exact or compatible version specified in `wydgo748/go.mod`;
- Node.js `22.12` or later to build the Astro staff panel;
- PostgreSQL for startup and database integration tests.

The panel is enabled in the versioned `data/server.txt`, but
`web/portal/dist/` is generated and ignored by Git. In a fresh clone, build
the panel before first startup or disable `web_admin_enabled`.

```powershell
pwsh -NoProfile -File tools/web-admin/Start-WYDAdmin.ps1 -NoBrowser

Push-Location wydgo748
New-Item -ItemType Directory -Force bin | Out-Null
go test -count=1 ./...
go vet ./...
go build -o bin/tm.exe ./cmd/server
go build -o bin/account-create.exe ./cmd/account-create
go build -o bin/account-api.exe ./cmd/account-api
Pop-Location
```

| Output | Purpose |
| --- | --- |
| `wydgo748/bin/tm.exe` | game server with integrated staff panel |
| `wydgo748/bin/account-create.exe` | interactive local account registration |
| `wydgo748/bin/account-api.exe` | separate registration API; expose only through an HTTPS proxy |
| `wydgo748/web/portal/dist/` | built assets for the integrated panel |

Start `tm.exe` with `wydgo748/` as the current directory; starting it from the
repository root makes relative `data/...` paths point to the wrong location.
See [server operations](server/operations.md) for database, configuration,
accounts, ports, shutdown, and backups.

PostgreSQL tests use `WYD_TEST_POSTGRES_URL`; without it, conditional tests do
not prove database integration.

## CI and runtime

The versioned workflow checks repository layout, research, assets, the Go
server with PostgreSQL, and the C++ client on Windows. It neither publishes
nor configures an external server. The supported runtime is the native
`bin/tm.exe` process started from `wydgo748/`; the repository has no container
architecture or deployment-provider integration.

The historical `cmd/npcconvert` converter is not part of the runtime. When its
input format is needed, it requires explicit `-in` and `-out` arguments; it
does not fetch external projects or write active data by default. Review
converted results before incorporating them into `data/`.

## Client

Requirements: Windows, Visual Studio with C++ x86 and toolset v145, and the
Windows SDK. DirectX dependencies are in the client tree.

```powershell
pwsh -NoProfile -File tmproject/Build-Client.ps1 -Configuration Release
```

The script builds and runs architecture tests, builds the solution, and copies
`tmproject/build/TMProject748/Release/WYD.exe` to
`tmproject/client748/project.exe`, verifying SHA-256. It neither starts nor
stops the game. The installed executable is a product, not the native
reference.

To validate without replacing the installed executable, add `-NoDeploy`.
In this mode, the artifact and its SHA-256 remain in `tmproject/build/`; no
result should be described as a visual test or `CLIENT-TESTED`. Screen testing
is blocked on the machine used for this work and must not be repeated; record
actual client execution as a pending gate.

## Boundary between projects

The client sends intentions; the server validates them and responds with
authoritative state. For contract changes, record direction, opcode, size,
offsets, packing, rejections, and compatibility before implementing both
ends. The 7.69 architecture may remain where it does not impose incompatible
resources or formats; do not assume equivalence between versions.

Builds and tests do not prove connection, login, world entry, or
logout/relogin. Those flows require integrated validation with the built
client and current server. Each workstream's status is recorded in the
[research records](../.agents/research/client748/README.md) and roadmap.

## Organization

```powershell
pwsh -NoProfile -File tools/repository/Test-RepositoryLayout.ps1
```

The command checks the inventory, local Markdown links, unique repository
rules, and documentation outside source trees. Use `-UpdateMap` only when
documents are added, removed, or moved; content-only changes do not require
regenerating the map.

The `references/client748/` collection is evidence; its patchers are
historical, not active development tools.
