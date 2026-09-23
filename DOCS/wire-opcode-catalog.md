# Opcode and contract catalog

Sources: `internal/core/Basedef.h` and the dedicated `internal/wire/` headers
re-exported through the facade. This catalog is an inventory, not
authorization to change the protocol.

## Identified families

| Family | Examples |
| --- | --- |
| Bootstrap/login | `MSG_CNFAccountLogin_Opcode`, `MSG_CNFCharacterLogin` |
| Movement | `MSG_Action_Opcode`, `MSG_Action_Stop_Opcode`, `MSG_Motion_Opcode`, `MSG_AirMove_Start_Opcode` |
| Inventory | `MSG_DeleteItem_Opcode`, `MSG_SplitItem_Opcode`, `MSG_UseItem_Opcode`, `MSG_UpdateEquip` |
| Trade/bank | `MSG_Trade_Opcode`, `MSG_CloseTrade_Opcode`, `MSG_Withdraw_Opcode`, `MSG_Deposit_Opcode` |
| Combination | `MSG_CombineItem_Opcode`, `MSG_CombineItemAylin_Opcode`, `MSG_CombineItemAgatha_Opcode`, `MSG_CombineItemTiny_Opcode` |
| Quest/mission | `MSG_Quest_Opcode`, `MSG_Mission` |
| Events | `MSG_Ping_Opcode`, `MSG_DelayStart_Opcode`, `MSG_SysQuit_Opcode` |

## Current dispatch

Network entry point: `platform/windows/CPSock.cpp`. Packets are framed as
`MSG_STANDARD` and forwarded to the `ObjectManager`/scene. The global
`ReceivedPacketDispatch.h` gate checks actual size, declared size, and opcode
for extracted contracts before legacy callbacks. `ExpectedSize`, rather
than this summary, lists the opcodes currently covered.

The first isolated receive contract was `MSG_ReqTransper_Opcode` (`0xFAA`),
52 bytes, in `CharacterTransferPacket.h`. `ReceivedPacketDispatch.h` checks
actual/declared length and opcode before traversal in `ObjectManager`.
Unregistered opcodes retain the previous fallback. The transfer evidence
record distinguishes native claims from local hardening. WYD-Go accepts
`0xFAA` only during selection, at exactly 52 bytes, and replies with
`Result=4` (the client's generic error) without changing the account or
slot. This ends the client's wait but does not implement transfer to the
Integrated server.

Other contracts at this entry point include `0x182` (SendItem, 24 bytes),
`0x101` (MessagePanel, 108 bytes), `0x102/0x104` (opaque messages, 116/152
bytes), and `0x333` (local chat, 108 bytes). Dedicated `internal/wire`
headers hold their structs and assertions and are re-exported by Basedef.
Validation uses the view's actual length and the header's Size and Type;
callbacks retain the original borrowed buffer. Other opcodes still depend
on their consumers.

The coordinated extensions `0x105/0x106` also require 108 bytes here.
`IndexedMessageContract.h` declares their opcodes and refers to the chat
envelope without duplicating the struct. ID, selector, index, and CSV remain
in the scene parser; these extensions are not native-message parity.

`0x52A` (server/channel migration, 80 bytes) uses
`ServerMigrationPacket.h`. The network entry point validates the frame
before the scene copies its image for replay. Local replay remains direct
in the handler with its existing state and window; this gate does not
validate ticket text or the server index. The reconnection handler separately
validates the TID's numeric prefix with `ParseMigrationServer` (bounded to
52 bytes, with overflow and capacity checks) and the local group before
indexing `g_pServerList`. The ticket suffix remains opaque.

`0x334` (whisper/channel/mail, 128 bytes) uses
`WhisperMessagePacket.h`, with the name at offset 12, text at 28, and color
at 124. `ReceivedPacketDispatch` validates the envelope before the existing
handlers; prefixes and filters remain in those handlers. Extracting this
contract does not add a gate for shout opcode `0xD1D`.

`0x3AE` uses `DelayStartPacket.h` for the shared 16-byte ABI. SysQuit names
`Parm=0` in the System/close paths; DelayStart retains values `1/2` for
portal/teleport transitions and return. The native gate does not enumerate
this opcode, so the 16-byte server-to-client validation documents the
coordinated WYD-Go response and preserves the existing callback without
expanding the native claim.

`0x194` uses `BillingNoticePacket.h`: a 16-byte server-to-client frame with
four opaque payload bytes. The client marks billing and displays localized
message 132; WYD-Go has no emitter or charging rule for it.

`0x37D` uses `PartyAddPacket.h`: a 40-byte server-to-client frame with `PARTY`
at `+12` and a final reserved field at `+38`. The gate preserves the existing
Party-panel insertion callback; invitation, removal, and confirmation are
separate contracts.

`0x37E` uses `PartyRemovePacket.h`: a 16-byte bidirectional frame with `Parm`
at `+12`. Zero clears/dissolves the list; another value identifies the
removed member. The gate preserves the callback without changing server
authority over the party.

`0x37F` uses `PartyRequestPacket.h`: a 44-byte bidirectional frame with the
leader's `PARTY` snapshot at `+12` and int32 TargetID at `+40`. Existing
client and server handlers continue to revalidate invitation, destination,
range, and party state.

`0x3AB` uses `PartyAcceptPacket.h`: a 32-byte client-to-server intention,
with short `LeaderID` at `+12`, `LeaderName[16]` at `+14`, and a reserved WORD
at `+30`. Both client senders preserve the existing flow; there is no
server-to-client route for this opcode.

`0x36A` uses `MotionPacket.h`: a 20-byte bidirectional frame with short
`Motion` at `+12`, short `Parm` at `+14`, and float `Direction` at `+16`.
The server reconstructs player emotes using the authoritative ID and keeps
special effects in the server-to-client direction.

`0x369` uses `MissingEntityRequestPacket.h`: a 16-byte client-to-server
intention with short `MobID` at `+12` and a reserved WORD at `+14`. Action
and Attack use the same type; the server limits responses to visible,
nearby, authorized entities.

`0x289` uses `RestartRecallPacket.h`: a 12-byte client-to-server request
consisting only of `MSG_STANDARD`. Death and recall retain their timers;
the server still decides HP, position, score, and cooldown.

`0x3A0` uses `KeepalivePingPacket.h`: a 12-byte client-to-server keepalive
consisting only of `MSG_STANDARD`. Field sends the local ID, SelectChar uses
zero, and the server records activity without replying.

`0x291` uses `ChangeCityPacket.h`: a 16-byte client-to-server intention with
the village index at `+12`. Both `TMFieldScene` paths retain the native
`Village < 4` condition, update HomeTown in the same flow, and send no
reply; position and city rules remain authoritative on the server.

`0x290` uses `ReqTeleportPacket.h`: a 16-byte client-to-server intention with
the reserved DWORD at `+12` set to zero. `case 16` of portal confirmation
retains the `0x10` attribute gate; the server still decides destination,
price, gold, and persistence.

`0xAD9` uses `AirMoveContract.h` and `MSG_STANDARDPARM2`: a 20-byte
client-to-server intention with the character's `Header.ID`, route `0..4`
in `Parm1/+12`, and start mode `1` or end mode `2` in `Parm2/+16`. It carries
neither NPC ID nor destination. The server validates the visible NPC and
origin chunk, stores the flight, and publishes only the native route's final
point after the corresponding end. See
[the air-move contract](../.agents/research/client748/flows/transport/airmove-contract.md).

`0x28B` uses `UseNPCPacket.h`: a 20-byte client-to-server intention with
`TargetID` at `+12` and `ClickOk` at `+16`. Clicks and confirmations retain
values `0/1`; the server still validates the NPC and context before opening
any feature.

`0x28C` uses `GuildDeprivatePacket.h`: a 16-byte client-to-server intention
with the member's `TargetID` at `+12`. The expulsion confirmation case keeps
the contract; the server remains authoritative for rank, target, and
persistence.

`0xE0E/0xE12` use `GuildRelationPacket.h`: 20-byte client-to-server
intentions with the local guild at `+12` and target guild at `+16`.
Confirmation cases retain native values; war, alliance, leadership, and
persistence remain server-authoritative.

`0xED7/0xED8` use `ServerWarLetterContract.h`: 16-byte client-to-server
intentions with the integer target channel in `Parm/+12`. In the native
client, items 4030/4031 first open the modal in modes 9/10; the packet is
sent only after confirmation. WYD-Go has no handler or authoritative
cross-instance coordination yet, so the client restores the contract
without claiming server support for inter-channel war.

`0x28F` uses `ChallengeConfirmPacket.h`: a 20-byte client-to-server
confirmation with `Parm1` at `+12` and `Parm2` at `+16`. Native confirmation
retains `Parm1=m_dwTID` and `Parm2=0`; the server still decides contest rules.

`0x116` (character logout confirmation, 12 bytes) uses
`CharacterLogoutConfirmPacket.h`. The contract is header-only; the scene
validates the character ID and then copies Score/Equip before transitioning
to SelectChar. The receive gate does not change that authority or transition.

Client-to-server request `0x215` uses `CharacterLogoutRequestPacket.h` and
the same 12-byte envelope, but it lies outside `ReceivedPacketDispatch`
because it is outbound. The FieldScene five-second timer fills in the local
ID and sends the request; the server decides persistence before emitting
`0x116`.

`0x114` (CharacterLogin confirmation, 2,104 bytes) uses
`CharacterLoginConfirmContract.h`. The gate validates only the envelope;
selection still interprets Pos/MOB/Slot/ClientID/Weather/ShortSkill and
sidecars before rebuilding FieldScene.

Integrity probe `0x1C1`/`0x2C2` uses `ClientIntegrityArrayContract.h`:
24 bytes, Category at `+12`, ByteOffset at `+16`, and Value at `+20`.
Reception validates only `0x1C1`; the handler builds a `0x2C2` response,
preserving the received category and offset and promoting the signed byte
to int32.

`0x3CE` (TOTO purchase, 36 bytes) uses `TotoPurchasePacket.h`. The client
validates local fields and closes the panel after sending; price, shop,
item 4147, gold, UID, effects, and rollback are exclusively WYD-Go's
responsibility.

`0x277` (ApplyBonus, 20 bytes) uses `ApplyBonusPacket.h`. `BonusType=2`,
`Detail`, and `TargetID` express purchase intent; the server decides skill,
points, prerequisites, cost, and persistence.

`0x373` (UseItem, 36 bytes) uses `UseItemPacket.h`, with source at
`+12/+16`, destination at `+20/+24`, cell at `+28/+30`, and ItemID at `+32`.
The client may consume the item visually before the response; the server
reconciles authoritative state through SendItem.

The F hotkey also uses `UseItemPacket.h`/`0x373/36`: SourType=1, normalized
position, zeroed destinations, current GridX/GridY, and ItemID=0. Selecting
the consumable compatible with Equip[12] and enforcing cooldown belong to
Field. This shares the wire format with the E potion; it does not get a
second struct.

`0x399` (PK Mode, 16 bytes) uses `PKModePacket.h`: `Parm` at `+12`, valid
domain `0/1`. The server applies the state before PvP and confirms it via
`MessagePanel` (`0x101`); the 7.48 client does not dispatch a `0x166`
response. Absence of an optional control does not change the wire format or
interrupt the toggle.

`0x378` (SetShortSkill, bidirectional, 32 bytes) uses
`ShortSkillSnapshotContract.h`: twenty shortcuts occupy `[12:32]`. The
client sends the entire array; the server removes unlearned skills and
returns the authoritative snapshot. Before `memcpy` and rebuilding both
pages, the receive gate requires actual/declared sizes and both opcodes.

`0x366/0x367/0x368` share `ActionFrameContract.h` and a 52-byte envelope:
PosXY at `+12`, Speed at `+16`, Effect at `+20`, TargetXY at `+24`, and
Route[24] at `+28`. The receive gate requires the complete frame before
`TMHuman` chooses movement, stop, or Illusion; semantics and authority
remain with WYD-Go.

`0x39D/0x39E/0x36C` use `AttackFrameContract.h`: native prefixes of
48/52/96 bytes for one, two, and thirteen targets, with the damage list at
`+44`. The active pair also accepts coordinated wide-damage extensions:
physical `0x39D/52`, `0x39D/60`, `0x39E/64|68`, and `0x36C/108..156` in
steps of four with `DMGX`. The gate requires actual size, `Header.Size`,
and the opcode's allowed size set to match before `OnPacketAttack`;
calculation and publication remain authoritative in WYD-Go.

`0x376/20` and `0x379/24` use `InventoryTransactionContract.h`. SwapItem
carries source/destination at `+12..+15` and TargetID at `+16`; Buy carries
the merchant at `+12`, sparse shop cell at `+14`, Carry at `+16`, and Coin at
`+20`. Beyond envelope validation, the client applies the same domains as
WYD-Go: Equip `0..15` except 9, Carry `0..62`, Cargo `0..119`, and shop cells
`0..8`, `27..35`, or `54..62`. A successful swap is completed by `0x376`;
`0x182` resynchronizes cells after rejection. A successful purchase is
confirmed by `0x379` itself.

`0x387/0x388` (Cargo gold withdrawal/deposit, bidirectional, 16 bytes) use
`CargoGoldTransferContract.h`: the `uint32` amount occupies `+12`. The gate
requires the full envelope before `OnPacketWithdraw`/`OnPacketDeposit` casts.
WYD-Go validates and persists the transfer, returns the same opcode/amount,
then reconciles balances with authoritative `0x339` and `0x337` frames.

`0x39F` (PlayerChallenge, bidirectional, 20 bytes) uses
`PlayerChallengeContract.h`: the other player occupies `Parm1/+12`, and
the mode occupies `Parm2/+16`. The same envelope carries initial intent
(`0..3`), the invitation sent to the target, and acceptance (`4`). The
gate requires the full frame before `TMHuman::OnPacketReqRanking`; WYD-Go
remains authoritative for lifecycle, range, expiration, and one-time use.

`0x397` (AutoTrade, bidirectional, 196 bytes) uses `AutoTradeContract.h`:
description at `+12`, twelve items at `+36`, positions at `+132`, prices at
`+144`, fee at `+192`, and target at `+194`. The gate requires the exact
envelope before Field terminates the description, copies the snapshot, and
materializes offers. Validation and persistence remain with WYD-Go.

`0x2CD` (16-byte client-to-server query) and `0xDC3` (52-byte
server-to-client CapsuleInfo) form the Celestial Capsule round trip.
`CapsuleInfoContract.h` fixes CIndex at `+12`, class/level at `+16/+18`,
attributes at `+20..+26`, two masteries at `+28`, nine skills at `+32`, and
quest at `+50`. The old 7.69 facade had four masteries and 56 bytes; the
ABI and gate now match WYD-Go's 7.48 builder, which checks account and seal
ownership before replying.

`0x1BF` (Gamble result, 36 bytes, server to client) and `0x2BE` (wager,
20 bytes, client to server) use `GamblePacket.h`. Result, prize, and jackpot
are copied to the UI; balance, pool, and RNG remain server-authoritative.

`0x338` (CNFMobKill, 24 bytes, server to client) uses
`MobKillConfirmPacket.h`: Hold/FakeExp at `+12`, victim at `+16`, killer at
`+18`, and uint32 EXP at `+20`. The gate validates the frame before Field
applies EXP/Hold and visual death.

`0x337` (UpdateEtc, 36 bytes, server to client) uses
`UpdateEtcPacket.h`: Hold at `+12`, EXP at `+16`, LearnedSkill at `+20`,
WORD points at `+24..30`, and gold at `+32`. The full Score remains in
`0x336`; CP/Chaos does not occupy this compact snapshot.

`0x3CA` (Premium Firework, 36 bytes, server to client) uses
`PremiumFireworkPacket.h`: eight reserved bytes at `+12` and a 10x10
LSB-first bitmap occupying 16 bytes at `+20`. The server publishes only
after validating and persisting consumption; the scene and effect container
own the visual effect.

Request `0x3C9` uses `PremiumFireworkUsePacket.h`: 52 bytes,
source/destination equivalent to UseItem, ItemID at `+32`, and bitmap at
`+34`. The server validates the final two bytes, bits outside the grid,
item, position, and cooldown.

## Migration rules

- Re-export an opcode through the facade while legacy consumers remain.
- Validate size before reinterpreting a buffer.
- Do not infer semantics from the opcode name alone.
- Record the caller, consumer, and fallback for each migrated entry.
- Add a truncated-packet test before moving a handler.
