---
id: player-shout-top-notice
title: Native player trumpet chat and separate server top notices
subsystem: chat
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-10-14
---

# Native player trumpet chat and separate server top notices

## Question

How should a paid player trumpet differ from a server announcement?

## Evidence boundary

- USED: native 7.48 FUN_0049889a, opcode 0xD1D, body offset +12,
  trailing bytes +106/+107, and green color 0xFF00CD00.
- USED: TMScene::OnShoutMessage, dispatch validation, server wire/shout.go,
  executeShout and existing server notice opcode 0x101.
- CONTRADICTORY: the previous adaptation used a custom colored top notice
  for trumpet text. The user requires native green chat with player identity.
- NOT APPLICABLE: later-client shout overlays do not establish this contract.

## Native 7.48 flow

### Callers

Received packet dispatch selects the fixed 0xD1D message and invokes the
scene shout handler. The native handler is FUN_0049889a.

### Callees

The handler terminates its 96-byte text storage at offsets 106/107 and
inserts green text into chat. It does not publish a top announcement.
The existing general-chat filter remains in force.

## State and lifecycle

The server validates the player, nonempty text and trumpet item 3330.
It consumes and persists the item before broadcast. Save rejection rolls back
and sends no public text. Critical persistence publishes only after its commit
acknowledgement; repeated settlement cannot charge or broadcast twice.
The server builds the player prefix; client text cannot supply SERVER identity.
Received chat follows existing chat ownership, filtering and scene teardown.
No new timer, message-panel state or persistent client state is introduced.

## Wire, ABI, and resources

Player trumpet: opcode 0xD1D, 108 bytes, 12-byte standard header and 96-byte
String. At most 94 text bytes including the server-built "[name]> " prefix;
the last two bytes remain zero. Header ID is zero, not a trusted player ID.

Server notice: existing opcode 0x101 and its 95-byte top-panel payload.
No color extension, custom 334-byte shout or new message-panel ABI remains.

## Current mapping

Trumpet -> executeShout -> persisted item consumption -> wire.PlayerShout ->
TMScene::OnShoutMessage -> green chat.

Server notices, including Arch/Celestial results -> existing MessagePanel ->
top panel. No player trumpet is routed there.

Classification: PARIDADE_NATIVA for the 0xD1D receiver layout/color;
MODERNIZACAO_COMPATIVEL for server notice routing and authoritative payment.

## Delta matrix

| Claim | Previous adaptation | Current decision |
| --- | --- | --- |
| Trumpet identity | Server-style notice | Server-built player name |
| Trumpet presentation | Custom top notice | Native green chat |
| Trumpet wire | Custom envelope | 0xD1D, 108 bytes |
| Server results | Gray SERVER whisper in affected paths | Existing top notice |
| Payment failure | Must not publish | Tested rollback and no broadcast |

## Decisions

Keep player communication and server announcements separate.
Retain established payment/persistence semantics and the native text bound.
The historical record filename is retained to avoid breaking references.

## Gaps

Actual client color, filtering, top notice replacement and multi-player
visibility require in-game verification. Published in server release
`20261014T234237Z` and patch 10024; no CLIENT_TESTED claim. See
[current state](../../../../../DOCS/current-state.md) for receipts and acceptance.

## Validation

Go wire and encrypted game tests cover exact length, prefix, truncation,
payment rejection, empty text, missing item and critical settlement replay.
The full Go suite and vet passed. ArchitectureTests passed 1,209,006 checks,
including dispatch size, green chat and no trumpet top-panel call.
Release/Win32 -NoDeploy build passed; no runtime overwrite.
