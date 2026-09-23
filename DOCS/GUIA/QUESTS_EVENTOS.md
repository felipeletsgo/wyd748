# Quests and events — technical summary

Source: official pages at `https://www.wyd2.co.kr/introduction/`.

---

## 1. General quest rules

**Source:** https://www.wyd2.co.kr/introduction/quest.asp

On entering the game, the client displays a quest notification or icon when content is available for the current level. The window can be opened from the notification or with the **X** shortcut.

The page groups quests into three historical categories:

- progression quests through level 256;
- event quests;
- Trans-character quests.

Available quests are highlighted differently from those whose level requirement has not yet been met.

### Types

- **Repeatable:** can be completed again while the requirements are met;
- **Single/one-shot:** can be completed only once.

Items obtained from level-banded quests may be usable only within the same level range. Some quests have time windows; the site warns that server clocks may differ by 1–3 minutes. The minimap (`M`) helps locate NPCs.

### Server-side implication

The server must remain authoritative over:

- eligibility;
- prior completion;
- level range;
- time window;
- item consumption and delivery;
- rewards.

The client notification is presentation only.

---

## 2. Orc Castle and Magical Pill

**Source:** https://www.wyd2.co.kr/introduction/quest_256_03.asp

The quest takes place in Orc Castle. To reach Orc Lord, the group must defeat gate guardians, obtain keys, and open the passages. Keys are single-use, and the gates close again after a period of time.

Orc Lord has the **Magical Pill**, which grants Skill Points without a level-up. The page specifies a bonus of **9 Skill Points**, effective only once per character.

According to the guide, party summon is not permitted inside Orc Castle.

### WYD-Go considerations

- Magical Pill requires a persisted one-shot flag;
- key consumption must be atomic;
- gates must return to the closed state on a server-side timer;
- summons must check the authoritative region.

---

## 3. Advanced Trans-character quests

**Source:** https://www.wyd2.co.kr/introduction/quest_js.asp

A Trans character at **level 355 or above** can open the advanced quest window.

The described flow is:

1. Hunt quest monsters inside Kefra Dungeon;
2. Obtain the required items;
3. Collect the items in any order;
4. Use and complete them sequentially, from the first stage onward;
5. Spend **100,000,000 EXP** at each stage;
6. Gain attributes upon completion;
7. Continue leveling after each stage until the next requirement is met;
8. Complete each of the four stages only once.

The page mentions the Trans 355+ requirement and a high minimum EXP for each stage. It also describes a historical need to reconnect before earning EXP again after the quest. The `/qst` command is listed for checking status.

---

## 4. Seven Trials, Runes, Secret Stones, and Ideal Stone

**Source:** https://www.wyd2.co.kr/introduction/quest_js_02.asp

According to the historical flow, the server opens access to the Unknown Continent after Kefra is defeated.

The **Seven Trials** comprise seven party-oriented zones. The leader must carry a **Clue of Rune** and register the party with Uxmal during the waiting window. The page specifies at least four members for the general flow and a total capacity of **21 registered parties**.

Under the documented rule, a Clue of Rune can be obtained from Odin for **7 Lactolerium Powders**.

### Event cycle

The one-hour period is described as three blocks of:

```text
15 min activity
5 min break
```

Registered parties are distributed among the seven zones. Each trial has its own condition; the prize is a rune item.

### Seven trials

| Rune / trial | Min. parties | Max. parties | Summary |
|---|---:|---:|---|
| Ansuz | 2 | 2 | Area enclosed by moving walls |
| Uraz | 1 | 3 | Three towers |
| Thurisaz | 1 | 3 | Central circular arena |
| Gebo | 1 | 3 | Four gates or portals, five fields |
| Hagalaz | 1 | 3 | Eight boss rooms |
| Isa | 1 | 3 | Maze and enclosed rooms, central boss |
| Jara | 1 | 3 | Warehouse with three entrances |

There are **24 runes**. Seven runes can be combined at Odin for **2,000,000 gold**; their order determines the chance and type of Secret Stone obtained.

Four Secret Stones, Sephirot, and Eternal Stone form the **Ideal Stone** through an interaction with the king.

The page describes the Ideal Stone as part of the transition to Reclass, subject to Mortal 400 and Trans 356+ in the historical flow. The Trans level affects initial stats and the quality of the Circlet received:

- 356–380: Silver Circlet;
- 381–399: Gold Circlet;
- 400: Mystic Circlet.

The page also records rules for disconnection and death during the trials, as well as reward competition among parties.

---

## 5. Enemy kingdom incursion / Token of Courage

**Source:** https://www.wyd2.co.kr/introduction/quest_event_07.asp

The event describes incursions between Acheronia and Hekalotia. Each kingdom has guards and towers that react to enemy knights.

The objective relevant to the quest chain is to kill the **enemy kingdom's king**. The victor receives a **Token of Courage**, which allows the bearer and their party to enter the final room of the Secret Room.

### Implementation

The Token must be awarded by the server-side event associated with the king's death; it must not be generated from client-reported state.

---

## 6. Secret Room

**Source:** https://www.wyd2.co.kr/introduction/quest_event_08.asp

The Secret Room is a four-floor or four-room instance/event. The party enters through the access point in Noatun Castle using an **Invitation**; in the described flow, only the leader needs to carry and use the item.

To reach the final room at certain difficulty levels, the leader must have the **Token of Courage** equipped in the slot specified by the guide.

### Flow

1. The party enters;
2. Monsters spawn in the room;
3. Combat runs for a limited time;
4. Defeating every monster may advance the party early in the first rooms;
5. The party proceeds to the next room;
6. Fragments or rune tablets from each floor form a sequence;
7. If the final sequence spells `W.Y.D`, the final boss may appear;
8. Even if the boss appears, the special drop is probabilistic.

The page documents **220 seconds per floor** and historical populations of 100 mobs on each of the first three floors and 20 plus a boss on the fourth. Monster families vary among Normal, Mystic, and Arcane.

Special Spirit Stones may be obtained from the boss under the appropriate conditions, with Absolute Damage, Damage Absorption, and HP variants.

According to the page, death, recall or warp, and disconnection end participation. In the described model, another party cannot occupy the same Secret Room simultaneously.

---

## 7. Eternal Stone

**Source:** https://www.wyd2.co.kr/introduction/quest_event_09.asp

The quest becomes available at **level 280** from the Oracle of the corresponding kingdom.

The player hunts specific monsters to obtain:

- Phoenix Soul;
- Unicorn Soul.

With both souls and **10 Sapphires**, the Oracle combines the components into an **Eternal Stone**.

The page requires the souls to be placed correctly in the inventory for the historical combination.

The Eternal Stone is a central component of Trans/Sephira progression and is used in the orb slot to continue the transformation chain.

### Relevance

In the emulator, the required inventory layout must be confirmed against the source or protocol before relaxing it. If reproduced, the order and positions must be validated server-side.

---

## 8. Sephira stone / Sephirot

**Source:** https://www.wyd2.co.kr/introduction/quest_event_10.asp

Raw Sephira stones are obtained from specific monsters. Refining a raw stone with **Oriharukon Powder** may convert it into one of the Sephira attributes.

The page presents a per-monster probability matrix for attributes such as:

- Keter;
- Binah;
- Chesed;
- Geburah;
- Tiphereth;
- Netzach;
- Yesod;
- Malkuth.

The documented combination cost at Skill Master is **30,000,000 gold**.

After obtaining Sephirot and Eternal Stone, the player continues the transformation chain at their own kingdom's king.

### Implementation

The probability matrix is content data. If this mechanic is implemented, its values must come from an explicit data source rather than being duplicated in handlers.

---

## 9. Chaos Cube / 25-stage Cube

**Source:** https://www.wyd2.co.kr/introduction/quest_event_11.asp

Entry requires a **Cube Invitation**, sold by Arnold in Azran in these variants:

| Invitation | Listed price | Documented range |
|---|---:|---|
| Normal | 100,000 gold | Trans 1–200 |
| Mystic | 400,000 gold | Trans 1–300 |
| Arcane | 1,000,000 gold | Trans 1–400 |

The entrance is in the construction area at the entrance to Azran; the invitation is used or consumed according to the page's flow.

### Mechanics

The Cube has **25 stages**.

At each stage:

1. Monsters appear;
2. All monsters must be defeated;
3. Participants are shown a quiz;
4. They have approximately **10 seconds** to select the `O` or `X` portal corresponding to the answer;
5. The result determines progression.

At stage 25, the boss **Orc Trooper** is the final objective. The page lists an EXP-granting consumable and random drops such as White Lover Ring or Sapphire as possible rewards.

### Important note for WYD-Go

The official page clearly confirms the O/X mechanic and 25 stages, but the retrieved text does not provide every question, spawn, or state transition needed for a complete reconstruction. The server must not invent the missing details.

---

## 10. Relationships among the quests

The official site supports the following conceptual chain:

```text
Mortal progression
↓
Orc Castle / Magical Pill and other progression quests
↓
Eternal Stone
+ Sephirot
↓
Trans
↓
advanced quests / Kefra
↓
Unknown Continent
↓
Seven Trials
↓
Runes
↓
Secret Stones
↓
Ideal Stone
↓
Reclass
```

Secret Room, Kingdom incursion, and Cube are parallel chains or events, but they reuse items, kingdoms, and rewards that intersect with advanced progression.
