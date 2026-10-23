# Login and character-selection dispatch

## Client build admission — contract v1, 2026-10-22

Classification: `EXTENSAO_COORDENADA`, not native build-version parity.
The native login boundary is reused from
`account-login-credential-bounds.md` (`FUN_004AC985`) and
`field-scene-rebuild-after-server-move.md` (`FUN_00484D44`) under
`.agents/research/client748/flows/`: native SHA-256
`8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`,
opcode `0x20D`, 116 bytes, account/password/protocol at 12/28/40. Those records
and their Ghidra evidence are USED for the unchanged envelope and migration
ticket. Current source and Go/tests are USED for the new policy. Native assets
and later-version algorithms are NOT APPLICABLE; no resource or opcode is added.
Native evidence does not establish this project's new build policy.

The little-endian unsigned word at byte 44, formerly the zero/obsolete
`DBNeedSave` selector, is now `ClientBuild`. Its upper 16 bits must be `0x5747`
and its lower 16 bits are a monotonically increasing admission revision.
The compiled client sends `0x57470001` (revision 1) in initial login and
migration reconnect. The server requires protocol 748 and revision >= 1.
Reserved migration bytes 48..99 and adapter words 100..115 remain untouched.
Do not reuse byte 44 as an ABI selector. Zero, old selectors, unknown prefixes
and insufficient revisions fail closed; the 116-byte framing remains exact.

The gate runs before password verification, auth-slot acquisition and account/IP
reservations. Rejection sends the existing `0x101/108` message with ID 0:
`Client update required. Close the game and run the launcher to update.`
It permanently marks that socket as rejected, queues the message once and
closes after a bounded 300 ms delivery grace period. Retrying a different build
on the same socket cannot authenticate; a fresh connection is required.
Late authentication results cannot admit a rejected socket. Existing network,
maintenance, sanctions, duplicate-account and credential policies remain active.
No account or character data is mutated by the version rejection. Normal
disconnect cleanup releases the transport; no per-world rejection map is kept.
The existing client message panel remains visible across the disconnect callback
instead of being replaced by its generic connection-lost message. This source
behavior still requires actual game validation on both an old and new executable.

`ClientBuildContract.h` and `internal/wire/client_build.go` own their respective
constants; cross-project tests prevent drift. This is a compiled executable
identity, independent of the launcher's mutable `config.txt` patch counter.
It is not cryptographic attestation: a modified client can forge the claim,
and the existing asset-integrity probes remain separate. No hot update or
automatic download is attempted inside the game.

For each future mandatory client update, increment `Revision` in the client
and `MinimumClientBuildRevision` in the server together, preserving the prefix
and protocol; do not reset or wrap revisions. Revision 3 (patch 10045)
covers the inventory ankh stage-state fix. Revision 2 (patch 10044)
covers the ankh inventory tint, Growth auto-buff and CON-free camera zoom.
Revision 1 covers the local
2026-10-22 fixes (Water/fairy/mount HUD/hand slots/monster scaling). Release
ordering is mandatory: build and publish the compatible client through the
established launcher feed first, verify that update, then activate the server
gate. Otherwise every currently installed old executable will be rejected.
Rollback requires a matching client/server policy; never silently disable the
gate. Existing authenticated sessions are not forcibly disconnected by this
implementation. Shutdown/restart and publication are operational actions and
are not authorized by this implementation request.

Implemented with automated tests covering all 65,536 revision values, legacy
selectors, unknown prefixes, protocol mismatch and every truncated length;
client/server constants are cross-checked. Handler tests verify rejection
before authentication or account/IP reservations, a single update message,
same-socket retry rejection, late-result rejection, normal admission for current
and newer revisions, and wrong-password behavior. A real local socket test
verifies encrypted message delivery and EOF after the grace period. Full
`go test ./... -count=1`, `go vet ./...` and `go build ./...` passed locally.
Client build validation is recorded in `DOCS/current-state.md`. Manual acceptance
must still cover old-client rejection/message/close, current-client normal login,
wrong credentials, reconnect, and migration when a real coordinator is available.
No `CLIENT_TESTED` claim is made; no client installation or server activation
has occurred in this batch.

## Current map

- `CPSock::ReadPacketView` validates framing and preserves the opcode,
  pointer, and actual length in `PacketView`.
- `ObjectManager::OnPacketView` runs `received_packet::Dispatch` before any
  legacy callback. Known frames proceed only when `Size`, `Type`, and actual
  length match the exact contract.
- `TMScene::OnPacketEvent` performs common handling and forwards to the scene
  after that gate.
- `TMFieldScene::OnPacketEvent` handles `0x10A` (`MSG_CNFAccountLogin`) and
  `0x114` (`MSG_CNFCharacterLogin`) through its own methods.
- `TMSelectServerScene` also handles `MSG_CNFAccountLogin_Opcode` during server
  selection.
- `TMSelectCharScene` handles `MSG_CNFCharacterLogin_Opcode` during character
  selection.

Length is no longer discarded at the `NewApp` boundary. The legacy virtual
API still receives `(dwCode, buf)` to preserve existing synthetic callbacks,
but only after the global length-aware validation.

## Migration decision

Scene methods were not extracted because they change state, controls, and
object ownership. The current facade remains responsible for lifecycle and
scene transitions. Memory protection is isolated in the global gate without
duplicating parsing or changing callback order.

## Character deletion in the 7.48 release candidate

`SelCharScene2.bin` contains modal panel 626 and password edit control 627.
After confirming deletion button 4615, control 1024 sends `0x211/44` with
slot, name, and password; control 921 cancels without sending. These IDs come
from the native 7.48 client, not TMProject 7.69. The server remains
authoritative: it validates the slot, name, and account password before
persistence and responds with `0x112/1288`. The rebuilt candidate binds these
controls and clears the password edit on confirmation or cancellation. The
complete flow still needs a manual client test.

## Extracted contract

`internal/wire/LoginPacketContract.h` classifies login/selection envelopes
sent by the server:

- `0x10A` (`MSG_CNFAccountLogin`): 2360 bytes;
- `0x110` and `0x112` (`MSG_CNFNewCharacter` and
  `MSG_CNFDeleteCharacter`): 1288 bytes;
- `0x114` (`MSG_CNFCharacterLogin`): 2104 bytes;
- `0x11A` and `0x11C` (creation failure and account already connected):
  12 bytes.

`received_packet::Dispatch` requires exact lengths and rejects truncated
prefixes, extra trailing bytes, null buffers, and mismatches among opcode,
`Header.Type`, `Header.Size`, and actual length before scene casts.

`internal/application/ports/PacketView.h` provides a non-owning view: opcode,
pointer, and framed length travel together without changing wire or taking
ownership. This guard is classified as `MODERNIZACAO_COMPATIVEL`.

## Extracted character-login request — 2026-09-05

`TMSelectCharScene` delegates construction and transmission of `0x213` to
`RequestCharacterLogin(ICharacterLoginSender&, slot)`. The use case validates
slots 0..3 and calls the semantic port once, without importing wire or
platform concerns. `CharacterLoginSender` encodes one zero-initialized
`MSG_CharacterLogin` via `ITransport`; `SocketTransport<CPSock>` borrows the
socket and forwards once to `CPSock::SendPacket`. The scene still owns
character-existence checks, debounce, timestamps, and button disabling,
including when sending fails. No receive handler was extracted.

Classification: `MODERNIZACAO_COMPATIVEL`; local extraction from existing
source. Reused evidence: `scene-transition.md` in
`.agents/research/client748/flows/lifecycle/`, section covering character
selection and packet `0x213`: `FUN_004A32DD`, call site `0x004A3422`, 36
zero-initialized bytes, opcode at +4, and slot at +12. This does not promote
the evidence record's maturity or change the server contract.

`MessageHeader.h` and `CharacterLoginPacket.h` are the sole owners of the two
extracted structs. SharedStructs/Basedef re-export them for compatibility.
Assertions preserve sizes, offsets, and underlying WORD/BYTE/DWORD types;
tests compare all 36 bytes before transport for all four slots. The standalone
header still requires the Windows integer model (32-bit `long`); this is not
a platform migration.

## Pending manual validation

Before moving another handler, test accepted and rejected login, valid and
rejected character selection, and return to the previous screen. A build does
not replace those flow tests.
