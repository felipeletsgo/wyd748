# WYD 7.48 — client and server

WYD-Go 7.48 monorepo: an authoritative Go server and a C++ client adapted for
the 7.48 runtime. Some flows still require in-game validation; a successful
build is not equivalent to testing them in the client.

| Location | Contents |
| --- | --- |
| `wydgo748/` | Go server, configuration, and authoritative data |
| `tmproject/TMProject748/` | C++ client source and tests |
| `tmproject/client748/` | 7.48 client runtime and assets |
| `DOCS/` | Architecture, operations, and contract documentation |
| `references/client748/` | Read-only historical evidence |

## Quick start on Windows

This procedure starts PostgreSQL, builds the web panel and server, creates an
account, and leaves the game server listening on TCP port `8281`.

### 1. Prerequisites

- PowerShell 7;
- the Go version specified in [`wydgo748/go.mod`](wydgo748/go.mod) (currently `1.26.5`);
- Node.js `22.12` or later, with `npm`;
- PostgreSQL 16 or later, with `psql` available in the terminal.

Clone the repository and open PowerShell at its root. Confirm that the
PostgreSQL service is running; on Windows, its name includes the installed
version:

```powershell
Get-Service -Name 'postgresql*'
```

If it is stopped, start it from an administrative PowerShell session with
`Get-Service -Name 'postgresql*' | Start-Service`. Create a dedicated role and
database in the SQL Shell (`psql`):

```powershell
psql -U postgres -d postgres
```

```sql
CREATE ROLE wydgo LOGIN PASSWORD 'wydgo-local';
CREATE DATABASE wydgo OWNER wydgo;
\q
```

Do not repeat these commands if the role and database already exist. Use a
different password in production, and expose port `5432` only to the private
or local network.

### 2. Configure the database and build

Set the environment variable in the same terminal that will start the server.
The PostgreSQL schema is installed and validated automatically on first boot.

```powershell
$env:WYD_DATABASE_URL = 'postgres://wydgo:wydgo-local@127.0.0.1:5432/wydgo?sslmode=disable'

pwsh -NoProfile -File .\tools\web-admin\Start-WYDAdmin.ps1 -NoBrowser

Push-Location .\wydgo748
New-Item -ItemType Directory -Force .\bin | Out-Null
go build -o .\bin\tm.exe .\cmd\server
go build -o .\bin\account-create.exe .\cmd\account-create
Pop-Location
```

The web-panel script only prepares web files; it does not start or stop the
server. To work without the panel, set `web_admin_enabled=false` in
[`wydgo748/data/server.txt`](wydgo748/data/server.txt) and skip that script.

### 3. Start the server

Always run it from `wydgo748/` because `data/server.txt` uses paths relative
to that directory.

```powershell
Push-Location .\wydgo748
.\bin\tm.exe
```

A successful boot reports authoritative PostgreSQL and finishes with the game
listener at `0.0.0.0:8281`. If the panel is enabled, open
`http://127.0.0.1:8082/admin/`. Change the example `admin_access_pin` before
using the panel outside a local environment. The administrative account must
also be authorized in `wydgo748/data/staff.json`.

Do not forcibly terminate the process. Use `Ctrl+C` so the server can drain
persistence and shut down. For subsequent starts, confirm PostgreSQL is
running and start the server again:

```powershell
Get-Service -Name 'postgresql*' | Start-Service
Push-Location .\wydgo748
.\bin\tm.exe
```

### 4. Create an account

In the integrated panel, open `http://127.0.0.1:8082/admin/#accounts` and
enter the username, password, and `admin_access_pin`. Registration creates a
regular account without administrative permissions.

Alternatively, run the local utility in another PowerShell session from the
repository root:

```powershell
$env:WYD_DATABASE_URL = 'postgres://wydgo:wydgo-local@127.0.0.1:5432/wydgo?sslmode=disable'
Push-Location .\wydgo748
.\bin\account-create.exe
Pop-Location
```

The utility prompts for a username and password. Do not pass the password on
the command line.

### 5. Connect the client

Point the client to the server host's IP address or domain and TCP port `8281`.
For local testing, use `127.0.0.1`; from another computer, allow only
`8281/TCP` through the firewall and use the reachable host address. Keep the
PostgreSQL endpoint and administrative ports private.

To rebuild and install the client into the local runtime:

```powershell
pwsh -NoProfile -File .\tmproject\Build-Client.ps1 -Configuration Release
```

The script installs the result as `tmproject/client748/project.exe`. Generate
the endpoint configuration with the external `serverlist editor.exe` and save
its output as `tmproject/client748/serverlist.bin`; do not edit that binary as
text. You do not need to preserve an old copy. See the
[Windows setup guide](DOCS/windows-development-environment.md) for the full
procedure.

## Quick troubleshooting

- PostgreSQL is configured but `WYD_DATABASE_URL` is empty: set the variable
  in the same process or terminal that starts the server.
- `connection refused` on `5432`: start PostgreSQL and check the host, port,
  username, and database in the URL.
- The game does not connect: confirm that the server announced
  `0.0.0.0:8281`, the client uses the correct address, and `8281/TCP` is open.
- The panel did not start: generate `wydgo748/web/portal/dist` with
  `tools/web-admin/Start-WYDAdmin.ps1` or disable the panel in the configuration.
- `data/...` paths cannot be found: the process was started outside
  `wydgo748/`.

The [server operations guide](DOCS/server/operations.md) covers the database,
accounts, panel, security, backup, and shutdown. Build and validation gates
are documented in [Build and integration](DOCS/build-and-integration.md).

## Documentation

- [Task-based index](DOCS/README.md)
- [Windows environment](DOCS/windows-development-environment.md)
- [Complete documentation map](DOCS/documentation-map.md)
- [Server operations](DOCS/server/operations.md)
- [Open 7.48 adaptation work](.agents/handoffs/client748-parity.md)
- [Repository rules](AGENTS.md)

The server validates intentions and remains the authority for game state.
Later architecture is reused only when compatible with the 7.48 contract or
through an explicit extension tested on both sides. Native evidence and tests
must support every parity claim.
