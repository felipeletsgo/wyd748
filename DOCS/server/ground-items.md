# Ground items

Rules for items lying on the map: placement, lifetime and pickup rights.
Implementation: `wydgo748/internal/game/ground_rules.go`, `world.go`
(`createGroundDropCell`), `handlers.go` (`onDropItem`, `onGetItem`).

## Evidence

| Behavior | Source | Classification |
| --- | --- | --- |
| A player drop sends the character cell at `GridX@24`/`GridY@26` of 0x272 | native client `FUN_0046247E` | `PARIDADE_NATIVA` |
| The client draws an item at cell + 0.5, height × 0.1 + 0.1, rotation × π/2 | native client `FUN_004856C3` | `PARIDADE_NATIVA` (client already matched) |
| The client ignores the `Owner` field of 0x26E | native client `FUN_004856C3` | server-only rules need no client change |
| Free-cell search: the origin, else the first free cell of the 3x3 ring (rows top to bottom, left to right); no stacking | legacy server `GetEmptyItemGrid` (W2PP), not native evidence on its own | historical rule, validated by the owner's client test |
| Monster loot goes to the killer's inventory and falls on the ground only when it is full | legacy `MobKilled` and existing server | unchanged |

## Rules

- **Player drop:** the item lands on the cell the client reports when that
  cell is walkable and within 2 cells of the server position (the server
  position trails the visual route); otherwise on the server position.
- **Placement:** every ground item (player drop, monster loot with a full
  inventory, instance and Uxmal rewards) uses the free-cell search above. When
  all nine cells hold items, the drop is refused ("There is no room to drop
  that item here.") instead of stacking. Cannon 746 keeps its exact cell. An
  item in another gameplay space (instance) does not occupy the cell.
- **Lifetime:** 2 minutes, then the item disappears for everyone. Permanent
  map objects never expire.
- **Protection (owner decision, 2026-10-07):** for 60 seconds only the owner
  and the members of the owner's party may pick the item up; other players
  receive "This item belongs to another player for now.". The owner is the
  player who dropped the item, the killer for monster loot, the player for an
  Uxmal reward and the leader for an instance reward. A player who leaves the
  party loses the right; one who joins it gains it. After 60 seconds the item
  is public until it disappears.
- **Instances:** items created inside an instance are visible and collectable
  only inside that instance, as before.

## Validation

- Go: `ground_rules_test.go` (owner, party, leaving the party, public after
  protection, free-cell order, other gameplay space, full 3x3),
  `TestPlayerDropLandsOnTheReportedCharacterCell`, and the critical drop test
  (another player refused during protection, accepted afterwards); full suite
  and `-race` for ground, drop and pickup tests passed (2026-10-07).
- Client test by the owner: pending.
