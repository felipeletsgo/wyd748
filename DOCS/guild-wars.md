# Guild wars: tower, cities, and guild fame

The server implementation is classified as `MODERNIZACAO_COMPATIVEL`. The
calendar and contest rules were compared with the user-specified W2PP
reference (`CWarTower`, timers, and city results), but that provenance does
not establish native 7.48 parity. The active client/server pair uses existing
packet envelopes; no opcode, reward item, or client-supplied economic value
was introduced. The native NPC confirmation flow was verified separately
against the 7.48 client. The server remains authoritative.

Status: statically verified and covered by automated tests; not
`CLIENT_TESTED`. The real-client integration gate remains open.

## Default schedule

The configured timezone is explicitly `America/Sao_Paulo`, independent of
the Windows system timezone.

| Event | Time | Action |
| --- | --- | --- |
| Tower, Monday to Friday | 21:00 and 21:04 | Five-minute and one-minute warnings |
| Tower | 21:05 | Create the tower, clear the arena, announce the start |
| Each capture | During combat | Announce the character/guild and new defender; restore tower HP and remove participants |
| Tower | 21:30 and 21:34 | Five-minute and one-minute closing warnings |
| Tower | 21:35 | Award 100 guild fame to the final owner, announce the result, remove the tower, and clear the arena |
| Cities | Saturday until Sunday 00:00 | Registration, with opening and closing announcements |
| Cities, Sunday | 19:55 and 19:59 | Five-minute and one-minute warnings |
| Cities | 20:00 | Announce the event and its stages |
| Cities | 20:03 | Summon players to four arenas and begin protected preparation |
| Cities | 20:05 | One-minute combat warning |
| Cities | 20:06 | Enable combat for 15 minutes |
| Cities | 20:16 and 20:20 | Five-minute and one-minute closing warnings |
| Cities | 20:21 | Resolve all four cities, persist ownership and defense counts, and clear the arenas |

The server must be running to publish warnings. After a restart, it publishes
only warnings still relevant to the current window; it does not replay old
countdowns or provide a clock synchronized with the client.

Announcements appear in the log and chat. The native announcement panel holds
95 text bytes and replaces a notice after about four seconds. Messages are
split into parts shown five seconds apart, with a volatile queue limited to
64 parts. Chat history receives all parts immediately; a burst can drop only
excess panel presentations. A process crash does not guarantee redelivery.
Economic results do not depend on this queue.

## Tower war

- `/tower` admits a living guild member in Erion's public space during
  combat. Entry is at `(2450,1855)`.
- The arena spans `(2445,1850)..(2546,1920)`, with the dedicated tower near
  `(2495,1885)`. These positions were checked against the installed terrain;
  they are not claimed as exact native-binary positions.
- The installed `Torre` template supplies its appearance, HP, and defense.
  The tower has no merchant behavior, AI, experience, drops, or quest progress.
  Reference generator 1075 is not reused because it names a different monster
  in the active data.
- Only a guild other than the current owner can damage the tower. Periodic
  damage rechecks the attacker, guild, arena, and schedule.
- Capturing persists ownership before announcing it. A failure restores HP
  and the previous owner. The tower regains HP and affects after capture, and
  participants return to Erion.
- The final owner receives **100 guild fame once per round**. No owner means
  no reward; no gold or item is created.
- Restart recovery restores the owner and deadline. It recreates a still-active
  tower at full HP or completes a pending expired-round reward without
  duplication.

## City registration and combat

`/citywar` reports all four cities, owner/challenger IDs, bids, and defense
counts in chat. `/citywar 1`, `2`, `3`, or `4` registers for Armia, Azran,
Erion, or Nippleheim respectively. A numbered command is not a read-only
query.

Registration requires a living canonical guild leader who belongs to the
channel and is in the contested city on Saturday. The guild needs at least
100 fame. Its bid is its fame **before** the charge; the highest bid wins,
with the first registration winning a tie. The 100-fame charge also applies
to losing bids. Repeating a request does not charge again. Each guild may
register once per week; a guild that already owns a city cannot challenge
another.

The native collector flow is `0x28E` (client to server, 16 bytes) -> `0x18D`
(server to client, exact 12-byte `MSG_STANDARD`) -> `0x28F` (client to server,
20 bytes). On `0x28E`, the server validates collector visibility, range,
city, and current owner; it stores a 30-second context for the NPC, city,
and owner guild before opening native selection. A valid collector
click (`onUseNPC`) can also prepare this context. `0x28F` consumes it once.
Replay, expiry, owner change, or interaction with another NPC invalidates
confirmation before charging or registering. An invalid option or an absent,
incompatible, or out-of-range collector also consumes the old request; the
player must interact again. Guild and leadership are re-resolved from the
canonical record at confirmation, not trusted from the packet or context.

Collectors proven in the active data are Balmus -> Armia (`Merchant=6`,
`Level=0`, `2107,2144`), Kara -> Azran (`Level=1`, `2538,1714`), and Empis ->
Erion (`Level=2`, `2466,1986`). No Nippleheim collector exists in the current
data; none was fabricated. `/citywar` remains an equivalent administrative
and playable route subject to the same authoritative validation.

Each participating guild may summon up to 26 living characters from the
city's public space who are not using Ghost Shop. Alphabetical character
name breaks selection ties. Erion permits only mortal characters. Cities
without challengers summon nobody. Summoning cancels trades.

Preparation prevents crossing the arena center and blocks PvP. During
combat, only opposing participants in the same arena may attack. Teleport,
death, disconnect, leaving the arena, or changing guild removes
participation; relogging does not grant another slot. Ordinary CP/experience
death penalties do not apply. Direct, area, and periodic damage all use war
authorization.

The server publishes the opposing guild through `WarInfo 0x3A8/24B` at
combat start and clears the snapshot on exit, death, and completion, reusing
the [native contract](../.agents/research/client748/flows/transport/war-info-contract.md).
Preparation barriers are server-side: the installed data has no gates in
these arenas, so no visual object was invented.

## Results, economy, and persistence

- City points are the sum of valid survivors' levels, plus 399 for each
  non-mortal. A tie preserves the owner; an attack must exceed the defense.
- Conquest changes the owner and clears its defense count. A successful
  defense increments the count up to four. Without a challenger, ownership
  remains unchanged; an unowned city with no scoring attack stays unowned.
- The city reward is **persisted ownership and defense count**. The reference
  result does not automatically award fame, gold, or items, so no such payout
  was introduced.
- A controlled city applies a **10% tax** to ordinary/TOTO purchases and
  NPC sales. NPC sales are final; there is no buyback. One quarter of the tax
  goes to the city treasury. All these transactions use the same
  authoritative economic commit. If credit would exceed `200.000.000.000`,
  the full transaction is rejected without losing gold or items.
- Player mutation, guild/war records, and treasury changes form one logical
  transaction. A persistence failure rolls back all affected state.
- The owning guild's leader can withdraw treasury funds. Below
  `1.000.000.000`, withdrawal gives gold directly, within the character cap.
  From `1.000.000.000`, it gives item `4011` checks worth `1.000.000.000`
  each. Limited inventory permits a partial withdrawal without losing the
  remainder.
- `/guildfame` and the established `/famaguild` alias query guild fame.
  Older records start at zero; tower war supplies the initial fame source.
  Addition rejects overflow, and registration rejects insufficient funds.
- Fame, results, registrations, and completion markers share the guild-record
  transaction in JSON or PostgreSQL. They are not applied twice through the
  reference's redundant paths.
- Failed persistence undoes mutation; rewards are never announced before
  commit. City points freeze during closing persistence attempts so late
  departures cannot change the result.
- A restart during preparation or combat cancels the city round, refunds
  100 fame for each paid registration (including losing bids), and preserves
  owners. Failure to summon before combat also cancels the round; it does not
  start a smaller round with whichever players reconnect first.
- Guild dissolution removes its tower, city, and registration references in
  the existing transaction so a reused guild ID cannot inherit ownership.
- Use the existing single guild record. Running independent `World` instances
  against it is unsupported; multi-server coordination was not added.

## Configuration and implementation

Optional server configuration keys:

```ini
guild_wars_enabled=true
guild_wars_timezone=America/Sao_Paulo
tower_war_hour=21
city_war_hour=20
```

Hours are 0..23. Existing table offsets are unchanged. Configuration is
read at startup without changing the machine clock. Disabling guild wars
turns off scheduling, registration, and war combat/movement gates; it does
not remove existing ownership or disable its taxation and collection.
Re-enabling uses the recovery rules above.

The main implementation is `wydgo748/internal/game/guild_wars.go`, with
models in `wydgo748/internal/model/guild_wars.go`. Relevant tests include
`guild_scheduled_wars_test.go`, `guild_wars_test.go`, persistence and
configuration tests, `WarInfo`, and `guild_city_contract_test.go`. Combat,
movement, teleport, guild, economy, and tick integration reuse the
authoritative `World` without new goroutines.

## Remaining validation

Run the built 7.48 client with opposing guilds to verify warnings and
colors, targeting, arrival and barriers, death, skills and periodic damage,
successive captures, results, fame, collector registration, taxation and
collection, relogin, and restart. A build or simulated clock does not replace
this gate. RvR/Akelonia-Hekalotia and Noatum Castle are outside the tower/city
war scope described here.
