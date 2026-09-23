# Official WYD2 guide — index

Primary source: https://www.wyd2.co.kr/

This mapping of the official Korean WYD/With Your Destiny site was prepared on
August 12, 2026.

## Scope and editorial policy

This directory organizes technical and gameplay information found on official
pages. For copyright reasons, Korean text is neither reproduced nor translated
in full. Each entry is a detailed technical summary that preserves requirements,
numbers, rules, items, NPCs, and relationships between systems where the page
provides them.

The legacy site uses ASP pages and has incomplete public indexing. Each URL
therefore has one of these states:

- **Confirmed/documented** — the page was located and its content recovered.
- **Confirmed/partial** — the URL exists or appears in navigation or indexes,
  but not all of its content was recoverable.
- **Unconfirmed** — only a numbering pattern suggests the page; it is not
  treated as a source until it is found.

Do not fill gaps in the site with general knowledge of the game. If a rule
does not appear in the recovered source, mark it as undetermined.

## Documents

- [MAPA_DE_URLS.md](MAPA_DE_URLS.md) — URL inventory and collection status.
- [GUIAS_JOGABILIDADE.md](GUIAS_JOGABILIDADE.md) — getting started,
  intermediate and advanced systems, and PvP.
- [QUESTS_EVENTOS.md](QUESTS_EVENTOS.md) — quests, progression, Cube, Secret
  Room, Seven Trials, and related systems.
- [ITENS_REFERENCIA.md](ITENS_REFERENCIA.md) — items, weapons, Trans equipment,
  and consumables.

## Official page families identified

### `/guide/firstguide*`

Basic client usage and fundamental systems: shortcuts, parties, guilds,
communication, and C.C/auto-farm.

### `/guide/midguide*`

Intermediate systems: refinement, combination, mounts, dyeing, and secondary
characters.

### `/guide/highguide*`

Advanced systems: Ancient, Trans equipment options, Spirit Seal, and Reclass
equipment combination.

### `/guide/pvpguide*`

PvP, C.Point, sieges, and Kingdom War.

### `/guide/tip*`

Practical character-progression guides.

### `/introduction/quest*`

Quest and event documentation. This family includes mechanics relevant to the
server, not just introductory text.

### `/introduction/item*`

Catalogs and rules for items, weapons, Trans equipment, and consumables.

### `/premium/*`

Cash/Premium guides. These are peripheral to the emulator core and were
cataloged separately in the URL map where found.

## Use in WYD-Go

These pages are secondary references for documented behavior and may describe
later versions. They do not replace native 7.48 evidence, explicit contracts,
or tests of the current client and server. The source policy is in
[AGENTS.md](../../AGENTS.md).

The official site is particularly useful for:

- requirements and costs shown to players;
- access rules for quests and events;
- rewards and limits;
- NPC interaction flows;
- expected C.C/auto-farm behavior;
- party and guild composition;
- combination and refinement rules;
- event schedules and capacity;
- item and effect descriptions.

When the site differs from code or C++ source, record and investigate the
difference; do not silently change behavior.
