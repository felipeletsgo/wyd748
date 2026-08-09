---
name: build-deploy
description: >-
  Build the FunctionsV02.dll plugin and deploy it to the running TMSRV server for
  this WYD 7.54 project. Use whenever you've changed anything under
  `Sourcer Star Micronics/` and need to test in-game, or when a rebuild "isn't
  taking effect." Encodes the exact MSBuild invocation for VS2026 and the
  kill-before-copy dance that prevents the stale/locked-DLL trap that has bitten
  this project repeatedly.
---

# Build & deploy the WYD plugin

This workflow is exclusively for `Sourcer Star Micronics/`. For the Go emulator
under `wyd-go/`, use `$wyd-go-feature` and build `./cmd/server`; never copy a DLL
or stop the native Micronics server as part of a WYD-Go-only change.

Goal: turn edited source under `Sourcer Star Micronics/` into a running change on the server. The #1 recurring failure here is **deploying a stale DLL** — the copy silently fails or the old DLL stays loaded because `TMSRV.exe` was still running (often crash-looping via its `.bat`). Follow these steps exactly.

## 1. Build

Use the VS2026 MSBuild directly (there is no VS2017 on this machine; the project is retargeted to toolset v141 + Windows SDK 10.0.26100.0):

```bash
"/c/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/MSBuild.exe" \
  "Sourcer Star Micronics/FunctionsV02.sln" //p:Configuration=Build //p:Platform=Win32
```

- Expect `0 Error(s)` (there are ~346 pre-existing warnings — ignore them).
- **x86 / Win32 only** — never x64; the DLL loads into a 32-bit process.
- If MSBuild says **"All outputs are up-to-date"** but you changed a file, force it: add `//t:Rebuild`. (Incremental build occasionally misses a change; when in doubt, Rebuild.)
- If link fails with **LNK1201 (can't write FunctionsV02.pdb)**, the PDB is locked — OllyDbg is attached or TMSRV is running. Close them, then rebuild.

Output DLL: `Sourcer Star Micronics/Build/FunctionsV02.dll`.

## 2. Kill TMSRV BEFORE copying (this is the step people skip)

`TMSRV.exe` holds an open handle to `Plugin/FunctionsV02.dll` while running. If it's up (or crash-looping), the copy fails silently and you test the OLD dll. Kill every instance first — check for and close the `.bat` window too, or it will relaunch:

```powershell
Get-Process TMSRV -ErrorAction SilentlyContinue | Stop-Process -Force
```

Confirm none remain before continuing.

## 3. Copy the DLL into the plugin folder

```bash
cp "Sourcer Star Micronics/Build/FunctionsV02.dll" \
   "Server Star Micronics/TMSRV/run/Plugin/FunctionsV02.dll"
```

## 4. VERIFY the deploy actually took (hashes must match)

Do not trust that the copy worked — confirm the deployed bytes equal the freshly built bytes:

```bash
certutil -hashfile "Server Star Micronics/TMSRV/run/Plugin/FunctionsV02.dll" MD5 | sed -n 2p
certutil -hashfile "Sourcer Star Micronics/Build/FunctionsV02.dll" MD5 | sed -n 2p
```

Same MD5 = good. Different = the copy didn't take (TMSRV still had it locked — go back to step 2).

## 5. Restart & test

Start `DBSRV` first, then `TMSRV` (their `.bat` launchers self-restart the exe). Then reproduce the change in-game. There is no test framework — "testing" is observing behavior with the client.

## Note when hand-driving the user

The user runs the actual machine. When a change "doesn't work," the FIRST hypothesis is always a stale/locked DLL, not a code bug — have them do steps 2→4 and confirm the MD5 match before assuming the code is wrong.
