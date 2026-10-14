# Kefra portal visibility v1 (0x7F30)

Classification: `EXTENSAO_COORDENADA`, version 1, 2026-10-14. This is a new
server-to-client world snapshot, not a claim of a native 7.48 opcode.

## Mechanic and authority

The [official Dungeon Kefra guide](https://feedback.kersef.com/pt-br/knowledge-bases/21/articles/21703-dungeon-kefra)
describes the passage to the Unknown Continent, where Odin and Uxmal are
located, becoming available after Kefra is defeated. The owner also reported
that the mirror in the hall must not appear while the boss lives. The current
guide is evidence for this gameplay rule, not proof of 7.48 channel, ticket,
reward or schedule values. Its 100-entry seal allowance is not imported:
the owner explicitly requires 50 entries per Locked Seal in this project.

The server already rejects use of the hall passage while
`GuildRegistry.Wars.Kefra.Defeated` is false. The defect was independent
presentation: the static map model and its two billboards were always drawn.
Visibility now follows that same durable server flag, never whether the boss
is currently in view or whether a local death animation played.

The existing server lifecycle remains unchanged: a successful non-instance
Kefra kill opens the passage to Uxmal; the next Tuesday at 12:00 in the war
timezone resets it. Existing cost, destination, guild fame, guards and kill
reward transactions are unchanged. See [operations](../server/operations.md).

Hall admission is a separate server-side repair, not part of `0x7F30`.
Survivor already exchanges the seal and performs a paid-in-entries visit.
The raw `0x290` entrance around `(2366,3893)` now validates and atomically
spends the same character counter; carrying an unexchanged seal cannot bypass
an exhausted allowance. The boss-controlled Uxmal passage is a different
portal and does not spend another hall entry.

## Wire contract

All integers are little-endian. The packet is exactly 16 bytes, inside the
existing encrypted session transport.

| Offset | Type | Meaning |
| --- | --- | --- |
| 0 | u16 | Size = 16, filled by transport |
| 2 | u8 | Transport keyword |
| 3 | u8 | Transport checksum |
| 4 | u16 | Type = `0x7F30` |
| 6 | u16 | ID = field scene `0x7530` |
| 8 | u32 | Transport tick |
| 12 | u16 | Contract version = 1 |
| 14 | u16 | Kefra defeated: 0 = hidden, 1 = visible |

There is no client request or client-supplied defeat state. The server's
teleport admission still decides movement, even if a modified client draws
the portal early. The snapshot carries no currency, reward or teleport action.

The client rejects null, short, oversized, mismatched opcode/metadata, wrong
scene ID, unknown version and non-boolean state before the legacy scene cast.
Parsing supports unaligned transport buffers. Rejection leaves the current
valid state unchanged; before any valid snapshot the default is hidden.

## State and lifecycle

| Event | Server publication | Client result |
| --- | --- | --- |
| Field-scene construction or reuse | None yet | Reset to hidden |
| Successful character entry, including relogin/restart | Current durable flag, before initial visibility snapshot | Reconstruct the correct state without observing the boss |
| Successfully persisted defeat | Broadcast true to all in-world sessions | Show model and both effects |
| Failed defeat save | No snapshot | No false opening |
| Successfully persisted weekly reset | Broadcast false to all in-world sessions | Hide all three visuals without reloading the map |
| Failed reset save | No snapshot | Preserve the last durable state |
| Critical write pending or worker result not acknowledged | No snapshot | No speculative change |
| Unknown write outcome | Existing maintenance containment; no proposed snapshot | No speculative opening or closure |
| Map object streams out and back in | No extra request | New objects query the current scene state |

Only model 2035 at global map position `(2366.5, 3926.5)` and its billboards
423 and 424 are gated. Other instances of model 2035 retain their rendering.
The wrapper gates both visibility and render calls, clearing stale visibility
from earlier frames. It owns no scene/object pointer; normal map ownership,
resource loading, collision masks and open-portal culling remain unchanged.

## Evidence and implementation

The [legacy loader record](../../.agents/research/client748/flows/gameplay/kefra-portal-visuals.md)
records the native 7.48 model/effect boundary and its remaining gaps. Neither
the new packet nor the server flag is mislabelled as native parity.

- Go: `internal/wire/kefra_portal.go`; `internal/game/kefra.go` broadcasts
  only after durable defeat/reset publication. `publishCharacterEntry`
  synchronizes every successful entry, including the critical-write path.
- C++: `KefraPortalPacket.h`, `ReceivedPacketDispatch.h`, the field-scene
  state/handler and `TMObjectContainer.cpp`. `KefraPortalVisual.h` adapts the
  actual mesh and billboard visibility without retaining streaming pointers.
- Canonical frames live once in `testdata/protocol/` and are consumed by Go
  and C++ tests. Go tests exercise real encrypted session output, entry,
  restoration, duplicates, out-of-view players, failed saves and both critical
  transitions before/after acknowledgement, rejection and unknown outcomes.
  C++ tests exercise malformed frames, reset/relogin, streaming and the actual
  three-visual adapter with a device-free test double.

Both updated client and server are required. An old client does not implement
this visibility gate. A new client connected to a server without the snapshot
fails closed visually, even after a kill; this mixed-version limitation is not
end-to-end compatibility. Deploy the pair in one coordinated release and have
players update their client. No publication is implied by local test success.

## Validation limits

Automated tests and an incremental Release build are recorded in
[current state](../current-state.md#published-combat-cape-and-kefra-batch--2026-10-14).
No visual game run was performed. Manual acceptance still needs a living boss
with no mirror/effects, a committed kill revealing them, use of the passage,
relogin/region return after defeat, and a reset hiding the passage again.

## Post-kill activation repair — 2026-10-14

Classification: `MODERNIZACAO_COMPATIVEL`, separate from the visibility
extension. The owner reported that the mirror appeared after the kill but
could not be used. The client log showed `index=-1`: the prompt table lacked
the aligned block `(2364,3924)`, although both attribute maps already mark it
as a portal. The added Uxmal row covers `(2364..2367,3924..3927)` and carries
the existing 700 gold route price. The server still resolves the destination
`(3250,1703)` from its route at `(2366,3925)` and rejects use before defeat.
No packet, map asset, admission rule or server implementation is changed.

The regression checks the production table, both attribute maps and the real
route file; it reproduces the failure before the fix. Existing server tests
cover closed/open/reset admission and charging. The client build is published
as patch 10020, paired with release `20261014T184657Z`, but actual passage use
has not yet been tested in game; see the
[repair receipt](../current-state.md#published-kefra-passage-activation-repair--2026-10-14).
