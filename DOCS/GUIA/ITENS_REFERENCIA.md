# Items and references — technical summary

Source: official pages at `https://www.wyd2.co.kr/introduction/`.

---

## 1. One-handed swords

**Source:** https://www.wyd2.co.kr/introduction/item_weapon01.asp

The official catalog lists one-handed weapons in Normal and Ancient variants, with requirements, damage, and grade. The family includes Dagger, Short Sword, Stiletto, Rapier, Long Sword, and Scimitar.

The guide describes their basic role as occupying one hand while leaving the other available for another weapon or a shield.

### Relevance

Individual item values must continue to come from `data/itemlist.csv`. This page is a cross-check for names, weapon classes, requirements, and the player's visual expectations.

---

## 2. Staffs and dual wielding

**Source:** https://www.wyd2.co.kr/introduction/item_weapon04.asp

The catalog includes one- and two-handed staffs and describes the role of magic weapons.

The combat rule of interest is **physical dual wielding** as recorded in this reference family:

- the higher-damage weapon contributes its full damage;
- the lower-damage weapon contributes only part of its damage;
- when both weapons belong to a compatible family or type, the secondary contribution increases;
- options other than primary damage can still be considered from both weapons under the system rules.

The historical site summary describes 100% of the primary weapon and 30% of the secondary weapon, rising to 50% for an equivalent pairing.

### Relevance to code

This page helps validate `equipmentDamage` and passive-skill modifiers. Final values must be reconciled with `BASE_GetMobAbility` and the source, especially when a class has a passive skill that changes how much of the second weapon's damage applies.

---

## 3. Ultimate Weapons

**Source:** https://www.wyd2.co.kr/introduction/item_weapon08.asp

The page catalogs the first family of **Ultimate Weapons**, described as weapons originally used by monsters and given to players through events.

The guide notes that some weapons in this family have no level requirement, requiring only the relevant stats, and deal more damage than comparable normal weapons.

Examples shown include Cutlass and Doom Axe, among others in the category.

### Relevance

The absence of a level requirement must not be implemented as a name-based hard-coded exception. The server must obtain requirements and effects exclusively from the loaded `itemlist.csv` content.

---

## 4. Trans equipment

**Source:** https://www.wyd2.co.kr/introduction/item_js01.asp

According to the page, equipment exclusive to Trans characters follows the same general option and application rules as ordinary items.

The key armor rule concerns **body/base class**:

> Compatible armor is determined by the character's original Mortal body class, not necessarily by the Trans character's current skill class.

The site's example is a Mortal Trans Knight who becomes a Trans Hunter but continues to use armor corresponding to the Trans Knight body.

The page lists sets such as Initiate Armor and their effects, including HP, regeneration, critical chance, resistance, and movement speed.

### Relevance to WYD-Go

This supports the existing architectural separation between:

```text
class governing skills
versus
body class governing armor
```

This distinction must remain server-side in `canEquip`.

---

## 5. Consumables and utility items

**Source:** https://www.wyd2.co.kr/introduction/item_consumer.asp

The page is a broad catalog of consumables and materials.

### HP and MP

It includes herbs, Healing Potions, Mana Potions, and boxes in different quantities.

### Combat potions

It describes three stages or types of temporary potions that increase physical or magical attack, with progressively longer durations and stronger effects.

### Courage

- **Courage Elixir:** increases damage against monsters;
- **Courage Potion:** a stronger version, also intended for PvE.

The guide specifies a bonus against monsters, not a generic PvP bonus.

### Blood Jewel

A life-drain effect, compared by the site with the Live Drain effect.

### Shield Jewel

Increases maximum HP in relation to defense and level, according to the corresponding mechanic.

### Angels

The page describes Silver Angel and Gold Angel as versions derived from Red Angel. They grant drop and EXP bonuses and can rapidly convert inventory items into gold through a specific client interaction.

The current material describes Gold Angel as granting stronger bonuses than Silver or Red Angel.

### Silver Wydon

Items representing large amounts of gold, such as 100 million or 1 billion, work around legacy inventory and cargo balance limits. Using one converts its value back into gold.

### EXP Box

According to the page, using this item doubles EXP from the last hit for **two hours**. Additional uses extend the duration.

### Revival Horse

Revives a dead mount without visiting the Mount Master, reproducing the NPC's effect.

### Warrior Seal Book

An item associated with increased Fame and other documented progression and cape conditions.

### Spirit Seal

An item that encapsulates an advanced character for trade. Further details are in `GUIAS_JOGABILIDADE.md`.

### Nightmare Book

The page describes **24 Nightmare entries** for Reclass and a **20-hour cooldown** from the first use before reuse.

---

## 6. Refining materials

**Source:** https://www.wyd2.co.kr/introduction/item_consumer.asp

### Oriharukon Scrap / Powder

- An alchemist NPC can convert 10 scraps into powder;
- the page cites a cost of **1,000,000 gold**;
- powder is used for refinement up to +6;
- the first levels have guaranteed success according to the historical documentation.

### Lactolerium Scrap / Powder

- used for higher-level refinement, up to +9;
- also used in advanced combinations, including higher transitions;
- the page records a longer guaranteed-success range than Oriharukon before the chance decreases.

### Legendary stones

The catalog lists raw stones such as Spinel, Beryl, Tectite, and Adamantite for transforming specific armor families into Legendary equipment, with a chance of failure.

### Architectural rule

Item-to-effect-to-use relationships should come from loaders and configuration whenever they represent content. Handlers must not duplicate lists of IDs or values already present in `data/`.

---

## 7. Gems

The refining and Ancient pages document four principal gems:

| Gem | Reference effect from the site |
| --- | --- |
| Diamond | Drop +8% |
| Emerald | Absolute Damage +40 |
| Coral | EXP +2% |
| Garnet | Damage Absorption +40 |

These values are useful for validating loaded content but must not become a second hard-coded table if already represented by `itemlist.csv` or item effects.

---

## 8. Relationship to the server

For each item on the official site, the order of authority within WYD-Go remains:

```text
itemlist.csv
+ Itemname.csv for names only
+ ItemEffect.h for ID ↔ EF_*
+ persisted instance effects
→ server-side calculation
```

Use the official catalog to detect content discrepancies, not to add post-load corrections.
