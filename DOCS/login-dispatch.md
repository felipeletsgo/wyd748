# Login and character-selection dispatch

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
