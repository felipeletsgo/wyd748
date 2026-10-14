# Hidden Valley access contract v1

Status: local implementation and automated tests, 2026-10-13. Not deployed by
this workstream; integrated client execution remains pending.

## Scope and authority

Hidden Valley is the destination labelled `Abaddon Dungeon` by the installed
client. Access is exclusively through an activated Hidden Ruins map. There is
no Valley Fairy NPC, equipment requirement, or fairy bypass in this contract.
Carrying a map is insufficient: the player must use it first.

Classification: `MODERNIZACAO_COMPATIVEL`. This is a server-owned eligibility
policy using existing consumable, movement, teleport and recall contracts.
It is not a claim that ticket eligibility, durations or the area boundary have
been independently proven as native 7.48 parity. No packet, client asset or
historical binary is changed.

The existing [native teleport evidence](../../.agents/research/client748/flows/transport/req-teleport.md)
covers the unchanged confirmation and `0x290` request. The client sends a
16-byte request, not a destination selected by the player. Its portal table
already contains source `(2548,1740)` and the 1,000 Gold price. Historical
read-only W2PP `TMSrv/GetFunc.cpp` suggests destination `(2281,3688)`, but its
equipment rule is not imported. The installed terrain independently verifies
that destination and its connected area. Current online guides for other WYD
editions do not establish the missing PC 7.48 ticket rules.

## Activation and entry

| Item ID | Configured access duration | Note |
| --- | --- | --- |
| `3909` | 24 hours | Existing `volatiles.json` timed-access rule |
| `3910` | 7 days | Existing rule; its item name mentions 24 hours, an unresolved data discrepancy |
| `3974` | 8 hours | Existing timed-access rule |

This change preserves those configured durations; it does not silently resolve
the `3910` naming discrepancy. Activation consumes one map and stores an
absolute Unix expiry in the character sidecar counter
`access_expires:hidden_ruins`. Duplicate activation while access is active
does not consume another map or extend the deadline. Access belongs to the
character UID, not every character on the account. Logout does not pause time.

Use the existing Azran portal near `(2548,1740)` after activation. The server
resolves the source's native four-by-four block, checks active access, then
uses the configured route to `(2281,3688)` for 1,000 Gold. The existing free-tile
resolver still selects a safe destination when necessary. Failed eligibility
does not consume Gold or submit a persistence write. Map activation does not
itself teleport the player.

## Lifecycle and failure handling

- The protected public-map rectangle is `X=2191..2292`, `Y=3600..3695`, inclusive.
  It encloses all 6,200 reachable terrain cells from the installed entry using
  the server's walkability/height checks. It is a server policy boundary, not a
  native map-region constant.
- All player teleports check eligibility before and after free-tile resolution.
  Authoritative walking steps also check it; an alternate route cannot bypass
  the ticket gate.
- At the exact expiry second, access is no longer active. The per-second sweep
  recalls occupants to their home city through the existing persistence path.
  A pending critical write settles first; the sweep retries afterward. Expiry
  crossing during a confirmed write does not become an identity failure or
  permanent containment.
- S2 activation saves consumption and sidecar expiry atomically and only
  publishes on acknowledgement. Certified rejection preserves the live map and
  access. Existing unknown-outcome containment is not weakened.
- Paid entry uses the existing save-before-confirm S2 teleport flow. Legacy
  synchronous save failure rolls back the fee and leaves the source position.
- Relogin restores the deadline without extending it. Normal world entry uses
  the existing home-city spawn policy, not persisted Valley coordinates.

## Content and validation boundaries

The installed terrain is present and walkable, but `NPCGener.txt` currently
has **zero enabled generators whose starting position lies inside the Valley**.
This work opens access; it does not populate the dungeon, invent monsters,
bosses, drops, experience rates, schedules or rewards. Those require a separate
data/content contract and validated 7.48 evidence where parity is claimed.

Automated coverage is in `internal/game/hidden_valley_test.go` and the shipped
route checks in `internal/data/teleports_test.go`. It covers absent/carried-only/
fairy-only/expired access, exact deadline, consumption, duplicate use, relogin,
alternate teleports, walking, recall, failed saves, S2 rejection and a deadline
crossed during asynchronous portal settlement. The installed terrain fixture
is required, not skipped.

Passing gates: full `go test -count=1 ./...`, `go vet ./...`, focused game tests
under `go test -race`, and native server compilation. No client source changed,
so no new C++ build was required. These results are `AUTOMATED TESTED`, not
`CLIENT_TESTED`. Manual gates: activate each map, confirm the Azran portal,
check the rejection message/fee, relogin, expire inside, and verify home recall
against the built client after deployment.
