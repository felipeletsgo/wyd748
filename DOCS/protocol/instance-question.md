# Cube questions and Big Cube entry v1 (0x7F12)

Classification: `EXTENSAO_COORDENADA`, version 1, 2026-10-21. The timed
question display and ticket confirmation are project extensions, not a claim
that opcode `0x7F12` or this confirmation exists in native 7.48.

## Entry and return

The owner specifies the inclusive Entrance Ticket (item 1737) entry rectangle:
X **1308 through 1375**, Y **1444 through 1743**. The server validates the
player's authoritative position; a client-provided position cannot authorize
entry. Rejection preserves the ticket. The existing admission transaction
consumes the ticket and persists entry before publishing the instance.

Right-clicking the ticket opens a confirmation before sending the existing
36-byte `0x373` use intention. Cancel sends nothing. Confirm re-resolves the
native inventory slot and requires the same item snapshot before using it;
moving or replacing the ticket invalidates the pending confirmation.

The portal at **(1342,1466)** returns to **Noatum (1058,1725)**, the reverse
endpoint of the existing Noatum-to-Big-Cube route. It does not consume a
ticket or create an instance. The return configuration uses the existing
route's 700-gold tariff; this is project configuration, not native evidence.
The server chooses and validates the destination and charge.

The native portal prompt reads bit `0x10` in a 4x4 world block, so this portal
covers X1340..1343 / Y1464..1467. Attribute offset 375119 changes from `0x04`
to `0x14`, preserving the safe-zone bit. Both active maps are synchronized:
`tmproject/client748/Env/AttributeMap.dat` (1,048,580 bytes, trailer preserved)
and `wydgo748/data/maps/AttributeMap.dat` (1,048,576 bytes). No historical
reference asset is modified. The generic portal prompt sends the existing
16-byte `0x290`; no new client destination table is necessary.

Staff-run global OX mode remains a separate existing ticket destination.
The timed panel covers Big Cube and the regular, Mystic and Arcane Cube
instances, not the four-choice quiz extension (`0x7F10`/`0x7F11`).

## Display-only wire contract

All fields are little-endian inside the existing encrypted transport.
Exactly 116 bytes are required; no client answer packet is introduced.

| Offset | Size | Meaning |
| --- | --- | --- |
| 0 | 12 | Standard header; Size=116, Type=`0x7F12`, ID=recipient player |
| 12 | 2 | Version=1 |
| 14 | 2 | Reserved=0 |
| 16 | 4 | Remaining display duration in milliseconds, 0..10000 |
| 20 | 96 | NUL-terminated question, at most 95 content bytes |

The client rejects wrong sizes, versions, reserved bits, excessive duration,
unterminated text and empty active questions. The field scene also requires
the recipient ID to match its own character. Duration zero closes the panel.
This packet is sent privately to admitted members, never to bystanders.

## Lifecycle and authority

### Big Cube admission

1. Confirm the ticket and validate/persist admission. Asynchronous admission
   remains in `big_cube_waiting` without a question deadline until commit.
2. Start the question on the world loop after admission. Refresh any stale
   pre-commit timestamp so persistence latency does not shorten the window.
3. Send the ten-second HUD time and the question. Render the latter in its
   own noninteractive panel, independent of confirmation dialogs and the
   shared four-second notice panel.
4. Keep the question visible until its local monotonic duration expires or
   the server closes it. At the authoritative ten-second deadline the server
   evaluates the player's position on O/X, not client text or UI state.
5. Close the question on resolution and clear it when instance time is
   cleared or the field scene is destroyed. Existing result, persistence,
   reward and exit logic remains authoritative.

### Regular, Mystic and Arcane Cube rooms

Each variant inherits the same 24 questions preceding its final combat room.
After the last monster in a question room is cleared, the server refreshes
any stale kill timestamp and opens the same private ten-second question
panel. Both the configured duration and authoritative deadline are ten
seconds; the previous shared notice and fourteen-second answer window are
replaced. EXP messages, actions and ordinary notices do not own this panel
and cannot reset or hide its question.

At the deadline, the existing atomic answer transaction persists the correct
players' EXP, wrong-answer departures and next-room state. Only after commit
does the server close every participant's question, before publishing any
party updates, EXP or departure packets. A failed commit restores the quiz
and publishes neither a question close nor speculative results. The local
display still expires at ten seconds rather than extending answer time while
persistence is unavailable. Answers, rewards and room progression otherwise
retain their existing authoritative rules.

Transport latency is not a client-side extension of the authoritative answer
deadline. Client and server must be delivered together: old clients do not
implement this panel, and there is no legacy notice fallback guaranteeing
ten seconds.

## Reused native boundaries

- [Volatile item routing](../../.agents/research/client748/flows/ui/volatile-item-use-routing.md):
  native use intention and inventory slot contract; item 1737 confirmation
  is a project choice, not an inferred native confirmation.
- [Message panel text](../../.agents/research/client748/flows/ui/message-panel-text.md):
  ordinary notices cannot supply an independent ten-second question.
- [Teleport request](../../.agents/research/client748/flows/transport/req-teleport.md):
  portal attribute, 4x4 normalization, generic prompt and server-owned route.

## Validation and delivery boundary

Go tests cover the entire inclusive rectangle and immediately excluded
border, ticket consumption/rejection, portal destination/charge, private
question packets, delayed persistence and a complete ten-second deadline.
Regular Cube tests additionally cover both participants' private packets,
the 9,999/10,000 ms resolution boundary, stale kill timestamps, wrong-answer
departure, correct-answer EXP, close-before-result ordering and persistence
rollback. Configuration tests resolve all three active ticket variants and
check all 24 inherited questions use ten seconds.
C++ architecture tests cover packet rejection, timer expiry/wrap, explicit
close and both shipped portal attributes. Full server and client build gates
are recorded in [current state](../current-state.md).

These are automated checks, not `CLIENT_TESTED`. In-game acceptance still
requires the exact candidate executable and changed Env asset, confirmation
cancel/accept, question visibility during movement and notices, O/X resolution,
and the return portal. No installation, publication or VPS restart is part of
this correction.
