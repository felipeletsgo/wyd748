# Cube questions and Big Cube entry v1 (0x7F12)

Classification: `EXTENSAO_COORDENADA`, version 1, 2026-10-21. The timed
question display and ticket confirmation are project extensions, not a claim
that opcode `0x7F12` or this confirmation exists in native 7.48.
The automatic public OX lifecycle was integrated on 2026-10-22 without
changing the version 1 question packet or client panel.

## Entry and return

The owner specifies the inclusive Entrance Ticket (item 1737) entry rectangle:
X **1308 through 1375**, Y **1444 through 1743**. The server validates the
player's authoritative position; a client-provided position cannot authorize
entry. Rejection preserves the ticket. Item 1737 enters the public OX event
only during enrollment; inactive events, closed enrollment, duplicate entry
and entry from another private instance reject without consuming it.
Admission persists ticket consumption before publishing the teleport and
session-bound membership. Durable login coordinates retain the existing
neutral-position policy; they do not restore a paid arena admission.

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

The staff panel starts the automatic public OX event using the existing
`cube-native` question and EXP configuration. The timed panel covers public
OX, private Big Cube and the regular, Mystic and Arcane Cube
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

### Automatic public OX

1. Staff `start` or `mode_on` opens the six gates and announces **60 seconds
   of enrollment** globally. `mode_off` stops the cycle and evacuates members
   through the world-loop arena guard. A repeated start does not reset a
   running event. Missing gates or questions reject the start.
2. Confirm the ticket and validate/persist admission. Admitted members receive
   the enrollment HUD countdown. An admitted write may finish after the
   enrollment deadline, but questions wait for all pending admissions; late
   requests are rejected. Staff mutations are blocked during pending writes.
3. On the world loop, send each admitted member a **ten-second** HUD time and
   question. The independent noninteractive panel is not owned by confirmation
   dialogs, EXP messages, actions or the shared four-second notice panel.
   Bystanders receive no question. Membership is bound to session, character
   identity and slot, and cannot survive character replacement or relogin.
4. Evaluate authoritative position at the ten-second deadline. O is west,
   X1296..1340; X is east, X1342..1387; both use Y1481..1518. Column X1341
   wins neither answer. Dead players lose. Walking is capped at the deadline;
   late teleports and skill jumps cannot change the answer. Staff `result_o`
   and `result_x` select the losing side but never resolve before ten seconds.
5. Atomically persist configured EXP for correct players and departures for
   wrong players. Winners remain; losers receive no question reward and leave
   near (1341,1465), with dead losers revived to 1 HP. Reserve unique free exit
   tiles outside the arena until acknowledgement. No available exit means no
   partial round or partial payment. Public movement, teleports, skill jumps
   and the world-loop guard reject or evacuate unregistered occupants without
   applying public OX admission rules to private instances.
6. After commit, close the question, publish rewards/departures, open the
   gates and send winners a **50-second next-question HUD countdown**. The next
   question gets a fresh ten seconds. Normally this is one question per minute;
   persistence delay extends the cycle rather than shortening either window.
   Certified rejection restores state without rewards or departures; unknown
   outcomes retain persistence fences and trigger existing containment.
7. Repeat all 24 configured questions, or stop when no members remain. Server
   restart is inactive; membership is intentionally not persisted or resumed.
   The protocol's close packet, cleared instance time and field-scene teardown
   clear the question panel; local monotonic expiry still ends it at ten
   seconds even if result persistence is delayed.

### Internal private Big Cube lifecycle

The private instance engine retains its `big_cube_waiting` admission state and
starts a fresh ten-second question after admission commit. Public item 1737
does not fall through to that engine when OX is inactive. Existing private
instance result, persistence and exit rules remain authoritative.

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
Public OX tests additionally cover the complete 60/10/50 cycle, every configured
question and EXP reward, both manual losing-side overrides and replay checks,
dead-player departures, crowded exits, character replacement, unauthorized
walking/teleports/skill jumps and private-instance isolation. Critical-write
tests cover delayed ticket admission, atomic multi-account rewards/departures,
distinct reserved exits, certified rollback and unknown-outcome containment.
Portal tests cover the updated phase display and result controls. Full Go
tests, `go vet`, portal tests, type checks and production build are separate
automated gates, not gameplay evidence.
C++ architecture tests cover packet rejection, timer expiry/wrap, explicit
close and both shipped portal attributes. Full server and client build gates
are recorded in [current state](../current-state.md).

These are automated checks, not `CLIENT_TESTED`. In-game acceptance still
requires the exact candidate executable and changed Env asset, confirmation
cancel/accept, question visibility during movement and notices, O/X resolution,
and the return portal. No installation, publication or VPS restart is part of
this correction.
