# WYD launcher and client patches

`tmproject/client748/WYDLauncher.exe` is the OnGame 7.47 launcher adapted to
this client. It downloads incremental patches from the VPS, keeps
`config.txt` and `guilds.txt` current and starts `WYD.exe`. The full client
is distributed outside the VPS (Google Drive) so that its outbound traffic
stays limited to small patches.

Classification: `PARIDADE_NATIVA` for the patch protocol (traced in the
native 7.47 launcher, compatible with the 7.48 `config.txt`, `Music.txt` and
`sound.txt` formats); the host, translations and resolution table are a
`MODERNIZACAO_COMPATIVEL` adaptation. Game wire, assets and `WYD.exe` are
unchanged.

## Building the launcher

```powershell
python -I tools/client-assets/build_wyd_launcher.py
```

The script reads the local-only reference `references/OnGame/WYD/WYDLauncher.exe`
(SHA-256 `6cd8b16f…44da75`), refuses any other input, and writes the adapted
executable plus the neutral `Launcher.bmp`/`Launcher.hat` artwork to
`tmproject/client748/`. The reference is never modified. Every patch checks
the original bytes first:

| Change | Detail |
| --- | --- |
| Update host | `update.htm` and file base URL become `http://wyd.forjago.com/patch/` |
| FTP fallback | A failed HTTP download fell back to FTP at the third-party `80.77.12.45`; now `127.0.0.1` (fails closed) |
| Language | Every Portuguese message, button, dialog label and the window title (`WYD 7.48`) is English |
| Resolutions | The settings radios list this client's 11-entry `[RES]` table (`NewApp.cpp` `stResList`) |
| Classic UI | The loader forced the classic UI for RES 1 and 6 (both 640x480 in the OnGame table); only RES 1 now |
| DirectX help | The Korean help URL opens `https://wyd.forjago.com/download/` |

Known residue: the MFC framework string tables (internal error messages)
remain Korean; they are not Portuguese and appear only on MFC failures.

## Traced protocol (Ghidra, native 7.47)

1. On start, the launcher reads `config.txt` (`[VERSION]` must be non-zero),
   `Music.txt` and `sound.txt`, normalizes values and rewrites them on exit.
2. It reads `update.htm` with one 8 KiB `InternetReadFile`, stores it as
   `update.dat` and requires the first line `[WYD]`.
3. `guilds.txt` is always downloaded first. The 7.48 client reads
   `./Guilds.txt` to show guild names; no packet carries them.
4. Each line is `<version> <file> [<file>...]` (`NOTIC w h url` shows a
   notice). The first line's version must not exceed the local version, or
   the launcher asks for a new full client. Files of every line whose version
   is greater than `[VERSION]` are queued; versions below 1000 are multiplied
   by ten.
5. If the newest version is in another ten-thousand range than the local one
   (`version / 10000` differs), the launcher asks for a new full client.
6. Each file is fetched over plain HTTP on port 80 (`InternetConnect`,
   no TLS) from the base URL. ZIPs are extracted over the client folder,
   creating folders, then deleted; `[VERSION]` advances after each patch.
7. **Start** copies `serverlist.<N>` over `serverlist.bin` if it exists (N is
   the server combo index, both entries are `WYD-Go`; no such file ships) and
   runs `WYD.EXE` from the launcher folder.

The launcher cannot delete files, replace itself while running, or verify
downloads beyond HTTP and ZIP CRCs. Patches travel over plain HTTP.

## Server feed

Caddy serves `/patch/` on both `http://` (no redirect, for the launcher) and
`https://wyd.forjago.com`:

- `/patch/update.htm` and `/patch/patch-<version>.zip` from `/srv/wyd-go/patch`;
- `/patch/guilds.txt` from the game server's `Guilds.txt` export in
  `/var/lib/wyd-go-public` (`guilds_txt` in `server.txt`).

The client base version is `[VERSION] 10000` in the distributed
`config.txt`. The initial list is `tools/deployment/vps/update.htm`.

## Publishing a patch

A patch is the set of client files added or changed between two commits:

```powershell
pwsh -NoProfile -File tools/deployment/Build-ClientPatch.ps1 -From <released commit> -To HEAD -Version 10001
pwsh -NoProfile -File tools/deployment/Publish-ClientPatch.ps1 -Version 10001 -SshTarget <user@host> -IdentityFile <key>
```

`Build-ClientPatch.ps1` takes bytes from Git with `core.autocrlf=true`
pinned (a fresh Windows clone), skips `config.txt` and `ClientSettings.txt`,
refuses `WYDLauncher.exe` (it cannot replace itself) and reports deletions,
which players keep. `Publish-ClientPatch.ps1` appends `<version>
patch-<version>.zip` to the live list; `vps/publish-patch.sh` tests the ZIP,
installs it, keeps the previous list as `.update.htm.previous` and replaces
`update.htm` last. A launcher change requires a new full client.

## Validation state

- Patcher: every original string, code byte and dialog verified against the
  7.47 image; output dialogs contain no non-ASCII text; code and data
  sections are byte-identical apart from the listed patches (automated).
- Feed: `http://wyd.forjago.com/patch/update.htm` and `/patch/guilds.txt`
  return 200 without redirect; other HTTP paths redirect to HTTPS.
- Patch builder: a dry run produced a valid ZIP whose entries match the
  client tree.
- Not yet `CLIENT_TESTED`: the launcher window, settings dialog, a real
  patch download and **Start** need a person to run them.

## Published patch 10001 (2026-10-11)

The production feed now lists `10001 patch-10001.zip` after the `10000`
base entry. This patch contains only the tested `WYD.exe`, including the
female bow body-reflection correction. It does not replace the distributed
launcher, player settings, or runtime assets. The source launcher remains
incompatible with this legacy feed and was not deployed.

The executable is an ignored build artifact, so this publication used an
explicit single-file ZIP from the validated local runtime rather than the
Git-revision patch builder. `Publish-ClientPatch.ps1 -Version 10001` uploaded
the archive and replaced the manifest last; the previous manifest remains
at `/srv/wyd-go/patch/.update.htm.previous`. The game service was not restarted.

- ZIP size: 810,500 bytes.
- ZIP SHA-256: `DC9F3CC1E2F68110923CFE79A4AFE7BE8D442D78711E6A4F15FBDF0897FD821C`.
- `WYD.exe` SHA-256: `81E824FC6392645B68D0B81E3B3FA17CD62AED40933878D9F87A296F7E34DCB8`.
- Public HTTP manifest returned 200 without a redirect and contained the new
  entry. A fresh HTTP download matched both hashes and contained only `WYD.exe`.
- Validation: `BUILD_AND_DEPLOY_VERIFIED` locally; patch publication and
  public download verified. Launcher extraction and in-game orientation remain
  pending human verification, not `CLIENT_TESTED`.

Players must close the game before opening `WYDLauncher.exe`. A successful
update advances `[VERSION]` in `config.txt` to `10001`; do not edit this value
manually to bypass downloading the patch. Future patches must use a higher
version. The legacy HTTP/ZIP-CRC transport limitations above still apply.
This client patch does not resolve the separately investigated server-side
critical-persistence session isolation incident.

## Published patch 10002 (2026-10-12)

The feed lists `10002 patch-10002.zip`. Like 10001 it contains only
`WYD.exe`, built from the working tree with Sephira special skills, slot-13
familiars, the costume-slot extension, costume expiry tooltips and the
Inventory X cascade ([port record](port-748.md#wyd-go-extensions-costume-slot-expiry-display-timed-horses-2026-10-12)).
It requires server release `20261012T024132Z`, deployed first.

- ZIP size: 811,012 bytes.
- ZIP SHA-256: `5FDB89F86EE9CBD943C02A59C28252A36EE570259136B255EE3D994AD36808FE`.
- `WYD.exe` SHA-256: `F84E27ED9CA0C4E6A0C01CCB877214F98E14C592C367574A34394451EC591FE0`.
- The public manifest returned 200 without a redirect; a fresh HTTP download
  matched the ZIP hash and contained only `WYD.exe`.
- Pending: launcher extraction and in-game checks by a person
  (not `CLIENT_TESTED`).

The earlier source launcher in `tmproject/WYDLauncher/` (content-addressed
HTTPS updates) is superseded by this adaptation.

## Published patch 10003 (2026-10-12)

The production feed now appends `10003 patch-10003.zip`. Server release
`20261012T041400Z` was installed first. The patch includes the audited client
executable with the remaining-time fairy tooltip and restored countdown HUD.

All 20 tracked runtime assets changed from `aa4d0855` to `ce411059` were
included, together with the ignored, validated `WYD.exe` build: 13 music files,
server/group lists, launcher resources/configuration, the corrected sound
table and its new `mguardatt.wav` dependency. Matching the local full bundle
does not prove that older player installations contain these assets; the
incremental patch therefore delivers them too. All 21 entries match the
audited source/build bytes. No settings or launcher executable are included.

- ZIP size: 26,989,810 bytes.
- ZIP SHA-256: `D54C7A40486FBB0F3A7E76D6518FDC45E462CCD1C10E02F60950E946F9CC825E`.
- `WYD.exe` SHA-256: `2A4869B828AD2BAC25657E7A160C574121D761C9380E8BE0333C8134C95519F1`.
- `sound/soundlist.txt` SHA-256: `274AB767F2E978F34E0E50306D7B232379740015DD1A7B0DA4912D3951D4E795`.
- The public HTTP manifest returned 200 without redirect and contained 10003.
  A fresh public download matched the complete ZIP hash and all 21 entries;
  the executable, table and new sound dependency were separately hashed.
- Publication uses a serialized, append-only manifest update after ZIP
  validation/installation. The previous manifest remains recoverable. This
  prevents concurrent publishers from losing an entry; it does not add TLS,
  signatures or authenticity to the legacy transport.
- Validation: `AUTOMATED TESTED`; built artifact and public publication
  verified. Local runtime installation was deliberately not performed.
  Launcher extraction and gameplay remain pending human validation, not
  `CLIENT_TESTED`.

Close the game and run `WYDLauncher.exe` to receive 10003. The launcher cannot
remove the obsolete `project.exe` from older installations; launch through
`WYDLauncher.exe`/`WYD.exe`, not that stale executable. The full client remains
hosted outside the VPS; this feed serves incremental updates only.
