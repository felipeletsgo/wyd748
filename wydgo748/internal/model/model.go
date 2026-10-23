// Package model defines the persisted and static domain types of WYD-Go.
// They are plain values (no external dependencies) used by wire, store, data and game.
package model

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"strings"
	"time"
)

const (
	ItemUIDHexLength      = 32
	CharacterUIDHexLength = 32
	MinCP                 = -75
	MaxCP                 = 75
	// ChaosFormatVersion marks the signed model adopted by the emulator. The
	// previous format stored CP=75 as "neutral"; the first read without this
	// mark migrates that value to CP 0 and writes the mark on the next save.
	ChaosFormatVersion uint8 = 1
)

func ClampCP(value int) int16 {
	if value < MinCP {
		return MinCP
	}
	if value > MaxCP {
		return MaxCP
	}
	return int16(value)
}

func clampCP(value int16) int16 { return ClampCP(int(value)) }

// Item holds the native STRUCT_ITEM (Index+Eff = 8 bytes), the instance
// identity, optional expiry and fairy pause timestamp. UID and dates exist
// only on the server and are never serialized on the wire: shops/NPCs/templates use an
// empty UID as a blueprint; materialized items keep the same identity and
// expiry through inventory, Cargo, trade and relogin.
type Item struct {
	Index         uint16  `json:"index"`
	UID           string  `json:"uid,omitempty"`
	Eff           [6]byte `json:"eff,omitempty"`
	ActivatedUnix int64   `json:"activatedUnix,omitempty"`
	ExpiresUnix   int64   `json:"expiresUnix,omitempty"`
	PausedUnix    int64   `json:"pausedUnix,omitempty"`
}

// MarshalJSON omits "eff" when it is all zero -- "omitempty" does NOT work on
// Go arrays, so a plain item is written compactly as `{"index":N}`.
func (it Item) MarshalJSON() ([]byte, error) {
	if it.Eff == [6]byte{} {
		return json.Marshal(struct {
			Index         uint16 `json:"index"`
			UID           string `json:"uid,omitempty"`
			ActivatedUnix int64  `json:"activatedUnix,omitempty"`
			ExpiresUnix   int64  `json:"expiresUnix,omitempty"`
			PausedUnix    int64  `json:"pausedUnix,omitempty"`
		}{Index: it.Index, UID: it.UID, ActivatedUnix: it.ActivatedUnix, ExpiresUnix: it.ExpiresUnix, PausedUnix: it.PausedUnix})
	}
	type alias Item
	return json.Marshal(alias(it))
}

// WireEqual compares only the eight bytes the client knows. Snapshots received
// from 7.48 never carry a UID; after validating those bytes the server must keep
// the authoritative copy that already has the identity.
func (it Item) WireEqual(other Item) bool {
	// A client echoes the bytes it received, which may include the presentation
	// expiry of a timed item (ExpiryDisplayEffects). That projection changes over
	// time, so an empty pair on the timed side matches any projected expiry
	// field in the echo; every other byte must still be identical.
	if it.Index != other.Index {
		return false
	}
	for pair := 0; pair < 3; pair++ {
		a := [2]byte{it.Eff[pair*2], it.Eff[pair*2+1]}
		b := [2]byte{other.Eff[pair*2], other.Eff[pair*2+1]}
		if a == b {
			continue
		}
		if a[0] == 0 && it.ExpiresUnix > 0 && isExpiryDisplayEffect(b[0]) ||
			b[0] == 0 && other.ExpiresUnix > 0 && isExpiryDisplayEffect(a[0]) {
			continue
		}
		return false
	}
	return true
}

// NewItemUID generates 128 random bits in hexadecimal. It needs no persistent
// counter and stays globally unique across Windows, VPS and backup restores.
// The store still checks for collisions/duplicates before persisting.
func NewItemUID() (string, error) {
	var raw [16]byte
	if _, err := rand.Read(raw[:]); err != nil {
		return "", fmt.Errorf("generate item UID: %w", err)
	}
	// UUID v4 variant/version; stored without hyphens to keep the JSON short.
	raw[6] = raw[6]&0x0f | 0x40
	raw[8] = raw[8]&0x3f | 0x80
	return hex.EncodeToString(raw[:]), nil
}

// NormalizeItemUID validates and canonicalizes a persisted UID.
func NormalizeItemUID(uid string) (string, error) {
	if uid == "" {
		return "", nil
	}
	uid = strings.ToLower(uid)
	if len(uid) != ItemUIDHexLength {
		return "", fmt.Errorf("UID %q has %d characters; expected %d",
			uid, len(uid), ItemUIDHexLength)
	}
	raw, err := hex.DecodeString(uid)
	if err != nil || len(raw) != 16 {
		return "", fmt.Errorf("invalid UID %q", uid)
	}
	return uid, nil
}

// NewCharacterUID creates the persistent server-side identity of a character.
// Name and slot belong to the protocol/UI and may repeat or change; the UID
// never reaches the wire and identifies Mortal, Arch and charstate unambiguously.
func NewCharacterUID() (string, error) {
	return newPersistentUID()
}

// NormalizeCharacterUID validates the persisted hexadecimal representation.
func NormalizeCharacterUID(uid string) (string, error) {
	return normalizePersistentUID(uid, CharacterUIDHexLength, "character UID")
}

func newPersistentUID() (string, error) {
	var raw [16]byte
	if _, err := rand.Read(raw[:]); err != nil {
		return "", fmt.Errorf("generate UID: %w", err)
	}
	raw[6] = raw[6]&0x0f | 0x40
	raw[8] = raw[8]&0x3f | 0x80
	return hex.EncodeToString(raw[:]), nil
}

func normalizePersistentUID(uid string, length int, label string) (string, error) {
	if uid == "" {
		return "", nil
	}
	uid = strings.ToLower(uid)
	if len(uid) != length {
		return "", fmt.Errorf("%s %q has %d characters; expected %d",
			label, uid, len(uid), length)
	}
	raw, err := hex.DecodeString(uid)
	if err != nil || len(raw) != 16 {
		return "", fmt.Errorf("invalid %s %q", label, uid)
	}
	return uid, nil
}

const (
	ScoreVersion  uint32 = 2
	MaxScoreValue uint32 = 2_000_000_000
	// MaxDonateBalance caps Account.Donate (premium shop balance).
	MaxDonateBalance uint32 = 2_000_000_000
)

// Score is the only attribute representation of characters and mobs and the
// binary contract shared with the TMProject748 client. Every field is uint32;
// there is no legacy score, sidecar or parallel projection.
//
// Intermediate game calculations use int64 and persisted results are capped
// at 2,000,000,000, avoiding overflow in the client's signed fields.
type Score struct {
	Version       uint32    `json:"version"`
	Level         uint32    `json:"level"`
	Attack        uint32    `json:"attack"`
	MagicAttack   uint32    `json:"magicAttack"`
	Defense       uint32    `json:"defense"`
	MaxHP         uint32    `json:"maxHP"`
	MaxMP         uint32    `json:"maxMP"`
	CurHP         uint32    `json:"curHP"`
	CurMP         uint32    `json:"curMP"`
	Str           uint32    `json:"str"`
	Int           uint32    `json:"int"`
	Dex           uint32    `json:"dex"`
	Con           uint32    `json:"con"`
	Accuracy      uint32    `json:"accuracy,omitempty"`
	Evasion       uint32    `json:"evasion,omitempty"`
	Parry         uint32    `json:"parry,omitempty"`
	Critical      uint32    `json:"critical,omitempty"`
	Range         uint32    `json:"range,omitempty"`
	ResistFire    uint32    `json:"resistFire,omitempty"`
	ResistIce     uint32    `json:"resistIce,omitempty"`
	ResistHoly    uint32    `json:"resistHoly,omitempty"`
	ResistThunder uint32    `json:"resistThunder,omitempty"`
	SaveMana      uint32    `json:"saveMana,omitempty"`
	MagicAmp      uint32    `json:"magicAmp,omitempty"`
	RegenHP       uint32    `json:"regenHP,omitempty"`
	RegenMP       uint32    `json:"regenMP,omitempty"`
	StatusPts     uint32    `json:"statusPoints,omitempty"`
	MasterPts     uint32    `json:"masteryPoints,omitempty"`
	SkillPts      uint32    `json:"skillPoints,omitempty"`
	Mastery       [4]uint32 `json:"mastery,omitempty"`
	AttackRun     uint32    `json:"attackRun,omitempty"`
	Merchant      uint32    `json:"merchant,omitempty"`
}

// Validate ensures persisted state uses exactly the current contract and that
// no value exceeds the signed integers used in calculations.
func (e *Score) Validate() error {
	if e == nil {
		return fmt.Errorf("missing score")
	}
	if e.Version != ScoreVersion {
		return fmt.Errorf("score v%d; expected v%d", e.Version, ScoreVersion)
	}
	fields := []struct {
		name  string
		value uint32
	}{
		{"level", e.Level}, {"attack", e.Attack}, {"magicAttack", e.MagicAttack},
		{"defense", e.Defense}, {"maxHP", e.MaxHP}, {"maxMP", e.MaxMP},
		{"curHP", e.CurHP}, {"curMP", e.CurMP}, {"str", e.Str}, {"int", e.Int},
		{"dex", e.Dex}, {"con", e.Con}, {"accuracy", e.Accuracy},
		{"evasion", e.Evasion}, {"parry", e.Parry}, {"critical", e.Critical},
		{"range", e.Range}, {"resistFire", e.ResistFire}, {"resistIce", e.ResistIce},
		{"resistHoly", e.ResistHoly}, {"resistThunder", e.ResistThunder},
		{"saveMana", e.SaveMana}, {"magicAmp", e.MagicAmp},
		{"regenHP", e.RegenHP}, {"regenMP", e.RegenMP},
		{"statusPoints", e.StatusPts}, {"masteryPoints", e.MasterPts},
		{"skillPoints", e.SkillPts},
	}
	for _, field := range fields {
		if field.value > MaxScoreValue {
			return fmt.Errorf("score.%s=%d exceeds %d",
				field.name, field.value, MaxScoreValue)
		}
	}
	for i, value := range e.Mastery {
		if value > MaxScoreValue {
			return fmt.Errorf("score.mastery[%d]=%d exceeds %d",
				i, value, MaxScoreValue)
		}
	}
	return nil
}

// ValidatePlayerState applies the functional limits of a persisted character.
// NPC source data keeps byte-domain sentinel values (notably SaveMana=255)
// inherited from the native files, so those static definitions use Validate;
// account/character state must pass this stricter boundary.
func (e *Score) ValidatePlayerState() error {
	if err := e.Validate(); err != nil {
		return err
	}
	for _, resistance := range []struct {
		name  string
		value uint32
	}{
		{"resistFire", e.ResistFire}, {"resistIce", e.ResistIce},
		{"resistHoly", e.ResistHoly}, {"resistThunder", e.ResistThunder},
	} {
		if resistance.value > 100 {
			return fmt.Errorf("score.%s=%d exceeds 100", resistance.name, resistance.value)
		}
	}
	if e.SaveMana > 99 {
		return fmt.Errorf("score.saveMana=%d exceeds 99", e.SaveMana)
	}
	if e.RegenHP > 255 || e.RegenMP > 255 {
		return fmt.Errorf("score regen outside 0..255: hp=%d mp=%d", e.RegenHP, e.RegenMP)
	}
	if e.CurHP > e.MaxHP {
		return fmt.Errorf("score.curHP=%d exceeds maxHP=%d", e.CurHP, e.MaxHP)
	}
	if e.CurMP > e.MaxMP {
		return fmt.Errorf("score.curMP=%d exceeds maxMP=%d", e.CurMP, e.MaxMP)
	}
	return nil
}

// ElementalResists follows the 7.59 functional order: Resist[0..3] = Fire,
// Ice, Holy and Thunder. It lives outside the 7.48 STRUCT_SCORE and belongs to
// the server's authoritative state.
type ElementalResists struct {
	Fire    uint32 `json:"fire"`
	Ice     uint32 `json:"ice"`
	Sacred  uint32 `json:"sacred"`
	Thunder uint32 `json:"thunder"`
}

// Affect is the authoritative temporary buff/debuff state. The client receives
// only Type and the remaining time; Value/Level feed the server-side formulas.
type Affect struct {
	Type       byte   `json:"type,omitempty"`
	ClientType byte   `json:"clientType,omitempty"` // client visual slot when it differs from the server-side meaning
	Value      int    `json:"value,omitempty"`
	Level      int    `json:"level,omitempty"`
	OwnerID    uint16 `json:"ownerId,omitempty"`
	// OwnerCharacterUID is the stable source identity for player-owned
	// hostile affects. OwnerID is a process-local wire/session id and is kept
	// only for compatibility with active legacy sessions.
	OwnerCharacterUID string `json:"ownerCharacterUid,omitempty"`
	// SourceItem identifies the economic instance that created or extended the
	// buff; the index helps human audits without replacing the authoritative UID.
	SourceItemUID   string    `json:"sourceItemUid,omitempty"`
	SourceItemIndex uint16    `json:"sourceItemIndex,omitempty"`
	ExpiresAt       time.Time `json:"expiresAt,omitempty"`
	NextTick        time.Time `json:"nextTick,omitempty"`
}

// CelestialForm holds only the INACTIVE form of the Celestial/SubCelestial
// pair. Equipment (except the face), inventory, gold, cape/kingdom, fame and
// quests stay in Char and are shared. Class, face, EXP, attributes, learned
// skills, skill bar and affects belong to each form, like 7.54 Persos[0..1].
type CelestialForm struct {
	Evolution             string     `json:"evolution"`
	Class                 byte       `json:"class"`
	Face                  Item       `json:"face"`
	Score                 *Score     `json:"score"`
	Exp                   uint32     `json:"exp"`
	LearnedSkill          uint32     `json:"learnedSkill"`
	SecondaryLearnedSkill uint32     `json:"secondaryLearnedSkill,omitempty"`
	ShortSkill            [20]byte   `json:"shortSkill,omitempty"`
	Affects               [16]Affect `json:"affects,omitempty"`
}

func (f *CelestialForm) Validate() error {
	if f == nil {
		return nil
	}
	evolution := strings.ToLower(strings.TrimSpace(f.Evolution))
	if evolution != "celestial" && evolution != "subcelestial" {
		return fmt.Errorf("celestial form has invalid evolution %q", f.Evolution)
	}
	if f.Class > 3 {
		return fmt.Errorf("celestial form has invalid class %d", f.Class)
	}
	if f.Face.Index == 0 {
		return fmt.Errorf("celestial form without a face")
	}
	if err := f.Score.ValidatePlayerState(); err != nil {
		return fmt.Errorf("celestial form: %w", err)
	}
	if f.Score.Level > 199 {
		return fmt.Errorf("celestial form has internal level %d; maximum 199",
			f.Score.Level)
	}
	return nil
}

// Char is a persisted character (one account slot).
type Char struct {
	UID   string `json:"uid"`
	Name  string `json:"name"`
	Class byte   `json:"class"`
	X     uint16 `json:"x"`
	Y     uint16 `json:"y"`
	// Score is persisted directly and shared with the TMProject748 client.
	Score *Score `json:"score"`
	// RuntimeScore holds the effective copy after buffs/debuffs. Score stays
	// the persisted base; this copy defines no other score format. Keeping them
	// apart stops each recalculation from stacking Possessed/transformations and
	// writing the temporary bonus to the JSON.
	RuntimeScore *Score `json:"-"`
	// RuntimeAttackSpeedPercent preserves the precision lost by AttackRun's
	// ten-point bands. It is derived on recalculation, never persisted or sent.
	RuntimeAttackSpeedPercent int      `json:"-"`
	Equip                     [16]Item `json:"equip"` // equipment slots
	Inv                       [64]Item `json:"inv"`   // wire/persistence has 64; the UI uses 0..62
	// CP is the displayed, signed and authoritative Chaos/PK Point (-75..+75).
	// CP 0 is the neutral state of a new character; the legacy CreateMob byte
	// uses CP+75 only at the protocol boundary.
	CP           int16  `json:"cp"`
	ChaosVersion uint8  `json:"chaosVersion,omitempty"`
	Gold         uint32 `json:"gold"`
	Exp          uint32 `json:"exp"`
	// Hold is the persistent EXP debt created by a PvP death. EXP earned in
	// combat pays this debt before Exp grows again.
	Hold            uint32 `json:"hold"`
	NextExp         uint32 `json:"-"` // derived from the table by level
	LearnedSkill    uint32 `json:"learnedSkill"`
	MagicalPillUsed bool   `json:"magicalPillUsed,omitempty"`
	SkillPointBonus uint32 `json:"skillPointBonus,omitempty"`
	// Advanced progression used by the Ehre/Odin compositions. An empty
	// Evolution means "mortal"; other values: arch, celestial and subcelestial.
	Evolution string `json:"evolution,omitempty"`
	// ArchMortalUID identifies the Mortal that originated this Arch and
	// ArchMortalLevel caches THAT character's level. They only matter when
	// Evolution is "arch"; for other characters they stay empty and are ignored.
	//
	// The Arch status bonus grows with the Mortal's level, and native code
	// recalculates it on every login from the source character -- so leveling
	// the Mortal AFTER the ascension still strengthens the Arch. The link uses
	// the UID, never the slot: deleting or reordering characters must not point
	// the Arch at another character. The level is cached because the calculation
	// only receives the Char.
	// ArchCrystals counts the elemental crystals already consumed, 0 to 4. They
	// are made IN ORDER (Elime, Sylphid, Salion, Nohas) and each costs 100
	// million EXP. Each grants 100 points to the future Celestial; creating it
	// does not require all four.
	//
	// Native code keeps this in QuestInfo.Arch.Cristal -- the field lives in the
	// ARCH block, and the quest is done as an Arch.
	ArchCrystals byte `json:"archCrystals,omitempty"`
	// ArchLevel355 and ArchLevel370 are the Arch level locks. While false, the
	// character STOPS earning EXP at internal level 354 and 369 -- unlocking is a
	// craft at Lindy. Native names: QuestInfo.Arch.Level355 and .Level370.
	ArchLevel355    bool   `json:"archLevel355,omitempty"`
	ArchLevel370    bool   `json:"archLevel370,omitempty"`
	ArchMortalUID   string `json:"archMortalUid,omitempty"`
	ArchMortalLevel uint32 `json:"archMortalLevel,omitempty"`
	// Fame does NOT live here: it is a per-character counter in
	// CharState.SpecialCoins["fame"], saved under the character UID.
	// See internal/game/counters.go.
	SoulInfo                 uint8 `json:"soulInfo,omitempty"` // 1..10; zero = none
	CelestialLevel40Unlocked bool  `json:"celestialLevel40Unlocked,omitempty"`
	CelestialLevel90Unlocked bool  `json:"celestialLevel90Unlocked,omitempty"`
	// CelestialArchTier records the band in which the Arch became Celestial:
	// 1=356..369, 2=370..379, 3=380..398, 4=399, 5=400 displayed.
	// CelestialCytheraTier (0..4) prevents repeating the 121/151/181/199 bonuses.
	CelestialArchTier    byte `json:"celestialArchTier,omitempty"`
	CelestialCytheraTier byte `json:"celestialCytheraTier,omitempty"`
	CelestialArcana      bool `json:"celestialArcana,omitempty"`
	// AlternateCelestial is the form not materialized in Char. The active one
	// stays in the normal fields so all combat/protocol code keeps a single
	// authoritative source.
	AlternateCelestial *CelestialForm `json:"alternateCelestial,omitempty"`
	// Evolution skills, kept apart from the 24 Mortal bits as in WYD 7.48.
	SecondaryLearnedSkill uint32     `json:"secondaryLearnedSkill,omitempty"`
	ShortSkill            [20]byte   `json:"shortSkill,omitempty"`
	Affects               [16]Affect `json:"affects,omitempty"`
	// GuildID/GuildRank are DENORMALIZED copies of the canonical record in
	// guilds.json, kept only for the wire and quick lookups. Login repairs
	// differences against the record; the guild is always the source of truth.
	// Missing in old JSON they load as zero = no guild.
	GuildID   uint16 `json:"guildId,omitempty"`
	GuildRank byte   `json:"guildRank,omitempty"`
	// QuestsDone keeps the IDs of completed quests. A list instead of a bit mask:
	// a mask would limit the project to 64 quests and tie the ID to the bit.
	QuestsDone []int32 `json:"questsDone,omitempty"`
	// Citizenship is the CHANNEL the character is a citizen of (0 = none). In
	// native code the value is ServerIndex+1, so "Citizenship 1" = channel 1. It
	// only gives a bonus on that channel -- see CitizenDropBonus.
	Citizenship byte `json:"citizenship,omitempty"`
	// CapeEmblem is a cosmetic variant (1..10), independent of channel bonuses,
	// chat and wars. Zero denotes a legacy character awaiting its first grant
	// or the entry migration. Promotions and kingdom changes retain the symbol.
	CapeEmblem byte `json:"capeEmblem,omitempty"`
	// SavedX/SavedY are the coordinate saved by the Star Gem (volatile 12) and
	// used by Warp (volatile 13). Persisted per character; 0/0 = none saved.
	SavedX uint16 `json:"savedX,omitempty"`
	SavedY uint16 `json:"savedY,omitempty"`
	// Nightmare Scroll (volatile 212): each use credits 13 entries and starts a
	// 12-hour cooldown. Kept on the character so item, balance and cooldown are
	// committed in the same SaveAccount.
	NightmareTickets  uint32 `json:"nightmareTickets,omitempty"`
	LastNightmareUnix int64  `json:"lastNightmareUnix,omitempty"`
}

// charEquip is the character's 16 equipment slots as a NAMED OBJECT, keeping the full Item type.
type charEquip struct {
	Rosto      Item `json:"rosto"`      // 0
	Elmo       Item `json:"elmo"`       // 1
	Armadura   Item `json:"armadura"`   // 2
	Calca      Item `json:"calca"`      // 3
	Luva       Item `json:"luva"`       // 4
	Bota       Item `json:"bota"`       // 5
	Arma       Item `json:"arma"`       // 6
	Escudo     Item `json:"escudo"`     // 7
	Acessorio1 Item `json:"acessorio1"` // 8
	Acessorio2 Item `json:"acessorio2"` // 9
	Acessorio3 Item `json:"acessorio3"` // 10
	Acessorio4 Item `json:"acessorio4"` // 11
	Acessorio5 Item `json:"acessorio5"` // 12
	Acessorio6 Item `json:"acessorio6"` // 13
	Montaria   Item `json:"montaria"`   // 14
	Capa       Item `json:"capa"`       // 15
}

func (e charEquip) array() [16]Item {
	return [16]Item{
		e.Rosto, e.Elmo, e.Armadura, e.Calca, e.Luva, e.Bota,
		e.Arma, e.Escudo,
		e.Acessorio1, e.Acessorio2, e.Acessorio3, e.Acessorio4,
		e.Acessorio5, e.Acessorio6, e.Montaria, e.Capa,
	}
}

func toCharEquip(inv [16]Item) charEquip {
	return charEquip{
		Rosto:      inv[0],
		Elmo:       inv[1],
		Armadura:   inv[2],
		Calca:      inv[3],
		Luva:       inv[4],
		Bota:       inv[5],
		Arma:       inv[6],
		Escudo:     inv[7],
		Acessorio1: inv[8],
		Acessorio2: inv[9],
		Acessorio3: inv[10],
		Acessorio4: inv[11],
		Acessorio5: inv[12],
		Acessorio6: inv[13],
		Montaria:   inv[14],
		Capa:       inv[15],
	}
}

// MarshalJSON writes the equipment as a NAMED object and the inventory with ALL
// 64 slots (untrimmed) -- save and load are symmetric and every slot is visible.
// The embedded alias avoids duplicating scalar fields (the outer equip/inv at
// depth 0 win over the alias ones at depth 1 because they share the json tag).
// IsEmpty reports whether a character slot is really empty. It replaces a direct
// comparison with Char{}, impossible since QuestsDone became a slice (a struct
// with a slice is not comparable in Go).
func (c *Char) IsEmpty() bool {
	if c == nil {
		return true
	}
	if len(c.QuestsDone) != 0 {
		return false
	}
	clone := *c
	clone.QuestsDone = nil
	var zero Char
	zero.QuestsDone = nil
	return clone.equalsZero()
}

// equalsZero compares every other persisted field by value. When state is added
// to Char, this predicate and its tests must be updated: a nameless slot must
// never be serialized as null while it still holds domain state.
func (c Char) equalsZero() bool {
	return c.UID == "" && c.Name == "" && c.Class == 0 && c.X == 0 && c.Y == 0 &&
		c.Score == nil && c.RuntimeScore == nil &&
		c.Equip == [16]Item{} && c.Inv == [64]Item{} &&
		c.CP == 0 && c.ChaosVersion == 0 && c.Gold == 0 && c.Exp == 0 && c.Hold == 0 && c.NextExp == 0 &&
		c.LearnedSkill == 0 && c.SecondaryLearnedSkill == 0 &&
		!c.MagicalPillUsed && c.SkillPointBonus == 0 &&
		c.Evolution == "" && c.ArchCrystals == 0 && !c.ArchLevel355 &&
		!c.ArchLevel370 && c.ArchMortalUID == "" && c.ArchMortalLevel == 0 &&
		c.SoulInfo == 0 && !c.CelestialLevel40Unlocked &&
		!c.CelestialLevel90Unlocked && c.CelestialArchTier == 0 &&
		c.CelestialCytheraTier == 0 && !c.CelestialArcana &&
		c.AlternateCelestial == nil &&
		c.ShortSkill == [20]byte{} && c.Affects == [16]Affect{} &&
		c.GuildID == 0 && c.GuildRank == 0 && c.Citizenship == 0 && c.CapeEmblem == 0 &&
		c.SavedX == 0 && c.SavedY == 0 && c.NightmareTickets == 0 &&
		c.LastNightmareUnix == 0
}

func (c Char) MarshalJSON() ([]byte, error) {
	if c.IsEmpty() {
		return []byte("null"), nil
	}
	if c.Name == "" {
		return nil, fmt.Errorf("nameless character slot holds state")
	}
	type alias Char
	encoded := alias(c)
	// Every character persisted in the current format carries the mark, even
	// when a test/tool built Char{CP:75} directly. Without it the next read
	// would have to interpret +75 as the old neutral value.
	if encoded.ChaosVersion == 0 {
		encoded.ChaosVersion = ChaosFormatVersion
	}

	return json.Marshal(struct {
		alias
		Equip charEquip `json:"equip"`
		Inv   []Item    `json:"inv"`
	}{
		alias: encoded,
		Equip: toCharEquip(c.Equip),
		Inv:   c.Inv[:], // all 64 slots
	})
}

// UnmarshalJSON reads only the current authoritative format: score v2, named
// equipment and the complete positional inventory.
func (c *Char) UnmarshalJSON(b []byte) error {
	if bytes.Equal(bytes.TrimSpace(b), []byte("null")) {
		*c = Char{}
		return nil
	}
	type alias Char
	var fields map[string]json.RawMessage
	if err := json.Unmarshal(b, &fields); err != nil {
		return err
	}
	aux := struct {
		alias
		Equip       charEquip `json:"equip"`
		Inv         []Item    `json:"inv"`
		LegacyChaos *uint32   `json:"chaos,omitempty"`
	}{
		alias: alias(*c),
	}
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&aux); err != nil {
		return err
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		return fmt.Errorf("character %q has trailing JSON", aux.Name)
	}
	*c = Char(aux.alias)
	if aux.LegacyChaos != nil {
		if _, hasCP := fields["cp"]; hasCP {
			return fmt.Errorf("character %q has both cp and chaos", c.Name)
		}
		// One-time migration of the previous format. The legacy field was the raw
		// native byte (1..150), where 75 is a neutral character. The zero used by
		// old JSON meant "no state" and must also become neutral, never CP +75.
		// The protocol boundary converts the signed CP to CP+75.
		if *aux.LegacyChaos == 0 {
			c.CP = 0
		} else {
			c.CP = ClampCP(int(*aux.LegacyChaos) - 75)
		}
	}
	if c.ChaosVersion == 0 {
		// Before the CP/Hold split, the template and some saves wrote `cp:75` for
		// a neutral character. Nothing distinguishes those old saves from a newly
		// earned +75; the one-time migration keeps the neutral state and marks the
		// new format. A remission used after this version stays +75 because it
		// already carries the mark.
		if _, hasCP := fields["cp"]; hasCP && aux.LegacyChaos == nil && c.CP == MaxCP {
			c.CP = 0
		}
		c.ChaosVersion = ChaosFormatVersion
	}
	if c.ChaosVersion != 0 && c.ChaosVersion != ChaosFormatVersion {
		return fmt.Errorf("character %q has an invalid chaos version: %d", c.Name, c.ChaosVersion)
	}
	if c.CP < MinCP || c.CP > MaxCP {
		return fmt.Errorf("character %q has cp outside -75..75: %d", c.Name, c.CP)
	}
	if err := c.Score.ValidatePlayerState(); err != nil {
		return fmt.Errorf("character %q: %w", c.Name, err)
	}
	if err := c.AlternateCelestial.Validate(); err != nil {
		return fmt.Errorf("character %q: %w", c.Name, err)
	}
	if c.ArchCrystals > 4 || c.SoulInfo > 10 ||
		c.CelestialArchTier > 5 || c.CelestialCytheraTier > 4 {
		return fmt.Errorf("character %q has invalid Arch/Celestial progress", c.Name)
	}
	activeEvolution := strings.ToLower(strings.TrimSpace(c.Evolution))
	if (activeEvolution == "celestial" || activeEvolution == "subcelestial") &&
		c.Score.Level > 199 {
		return fmt.Errorf("character %q has internal Celestial level %d; maximum 199",
			c.Name, c.Score.Level)
	}
	if c.AlternateCelestial != nil {
		active := activeEvolution
		alternate := strings.ToLower(strings.TrimSpace(c.AlternateCelestial.Evolution))
		if active != "celestial" && active != "subcelestial" {
			return fmt.Errorf("character %q has an alternate form without an active celestial form", c.Name)
		}
		if active == alternate {
			return fmt.Errorf("character %q has two %q forms", c.Name, active)
		}
	}
	if len(aux.Inv) != len(c.Inv) {
		return fmt.Errorf("character %q has %d inventory slots; expected %d",
			c.Name, len(aux.Inv), len(c.Inv))
	}
	c.Equip = aux.Equip.array()
	for i := range c.Inv {
		c.Inv[i] = aux.Inv[i]
	}
	return nil
}

// Account is a persisted account: login, password and up to 4 characters.
type Account struct {
	Name         string    `json:"name"`
	PasswordHash string    `json:"passwordHash"`
	Chars        []Char    `json:"chars"`
	Cargo        [128]Item `json:"cargo"`               // storage shared by the four characters
	CargoGold    uint32    `json:"cargoGold,omitempty"` // stored gold / Phantom Shop sales
	// CelestialCapsules are persistent seals holding a Celestial character taken
	// out of the world. Item 3443 in Carry/Cargo references the ID; the complete
	// snapshot lives here so Postgres/JSON commit item, character and state in a
	// single write, without numbered files outside the transaction.
	CelestialCapsules []CelestialCapsule `json:"celestialCapsules,omitempty"`
	// Donate is the account-wide premium balance (DOCS/protocol/premium-shop.md).
	// It is saved with the account, so a purchase and its item commit together.
	Donate uint32 `json:"donate,omitempty"`
}

// UnmarshalJSON requires the complete structural cargo. The eight slots without
// UI stay in the file to preserve the ABI, but Account.Validate requires zeros.
func (a *Account) UnmarshalJSON(b []byte) error {
	type alias Account
	aux := struct {
		alias
		Cargo []Item `json:"cargo"`
	}{
		alias: alias(*a),
	}
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&aux); err != nil {
		return err
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		return fmt.Errorf("account %q has trailing JSON", aux.Name)
	}
	if len(aux.Cargo) != MaxCargo {
		return fmt.Errorf("account %q has %d cargo slots; expected %d",
			aux.Name, len(aux.Cargo), MaxCargo)
	}
	*a = Account(aux.alias)
	copy(a.Cargo[:], aux.Cargo)
	return nil
}

func (a *Account) Validate() error {
	if a == nil {
		return fmt.Errorf("missing account")
	}
	if a.Name == "" {
		return fmt.Errorf("account without a name")
	}
	if a.PasswordHash == "" {
		return fmt.Errorf("account %q without passwordHash", a.Name)
	}
	if a.Donate > MaxDonateBalance {
		return fmt.Errorf("account %q donate balance %d exceeds %d", a.Name, a.Donate, MaxDonateBalance)
	}
	if len(a.Chars) > 4 {
		return fmt.Errorf("account %q has %d characters; maximum 4", a.Name, len(a.Chars))
	}
	activeUIDs := make(map[string]struct{}, len(a.Chars))
	validateItemTime := func(item Item, location string) error {
		if item.ActivatedUnix < 0 || item.ExpiresUnix < 0 ||
			(item.ActivatedUnix == 0) != (item.ExpiresUnix == 0) ||
			item.ExpiresUnix != 0 && item.ExpiresUnix < item.ActivatedUnix ||
			item.PausedUnix < 0 || item.PausedUnix != 0 &&
			(item.ActivatedUnix == 0 || item.PausedUnix < item.ActivatedUnix || item.PausedUnix >= item.ExpiresUnix ||
				item.Index < 3900 || item.Index >= 3980) {
			return fmt.Errorf("account %q has an invalid item expiry at %s", a.Name, location)
		}
		if item.Index == 0 && (item.UID != "" || item.ActivatedUnix != 0 || item.ExpiresUnix != 0 || item.PausedUnix != 0) {
			return fmt.Errorf("account %q has an empty item with identity/expiry at %s", a.Name, location)
		}
		if item.ActivatedUnix != 0 {
			uid, err := NormalizeItemUID(item.UID)
			if err != nil || uid == "" || uid != item.UID {
				return fmt.Errorf("account %q has a timed item without a canonical UID at %s", a.Name, location)
			}
		}
		return nil
	}
	for i := range a.Chars {
		character := &a.Chars[i]
		if character.Name == "" {
			if !character.IsEmpty() {
				return fmt.Errorf("account %q empty character[%d] holds state", a.Name, i)
			}
			continue
		}
		if err := character.Score.ValidatePlayerState(); err != nil {
			return fmt.Errorf("account %q character[%d] %q: %w",
				a.Name, i, character.Name, err)
		}
		if err := character.AlternateCelestial.Validate(); err != nil {
			return fmt.Errorf("account %q character[%d] %q: %w",
				a.Name, i, character.Name, err)
		}
		normalized, err := NormalizeCharacterUID(character.UID)
		if err != nil {
			return fmt.Errorf("account %q character[%d] %q: %w",
				a.Name, i, character.Name, err)
		}
		if normalized == "" {
			return fmt.Errorf("account %q character[%d] %q without a UID",
				a.Name, i, character.Name)
		}
		if normalized != character.UID {
			return fmt.Errorf("account %q character[%d] %q has a non-canonical UID",
				a.Name, i, character.Name)
		}
		if _, duplicate := activeUIDs[character.UID]; duplicate {
			return fmt.Errorf("account %q has a duplicate character UID %q", a.Name, character.UID)
		}
		activeUIDs[character.UID] = struct{}{}
		if character.ArchMortalUID != "" {
			if _, err := NormalizeCharacterUID(character.ArchMortalUID); err != nil {
				return fmt.Errorf("account %q character[%d] %q: %w",
					a.Name, i, character.Name, err)
			}
		}
		if character.Inv[PlayerCarrySlots].Index != 0 {
			return fmt.Errorf("account %q character[%d] %q uses reserved inventory slot %d",
				a.Name, i, character.Name, PlayerCarrySlots)
		}
		for slot := range character.Equip {
			if err := validateItemTime(character.Equip[slot], fmt.Sprintf("character[%d].equip[%d]", i, slot)); err != nil {
				return err
			}
		}
		for slot := range character.Inv {
			if err := validateItemTime(character.Inv[slot], fmt.Sprintf("character[%d].inv[%d]", i, slot)); err != nil {
				return err
			}
		}
	}
	for i := range a.Cargo {
		if err := validateItemTime(a.Cargo[i], fmt.Sprintf("cargo[%d]", i)); err != nil {
			return err
		}
	}
	for i := PlayerCargoSlots; i < MaxCargo; i++ {
		if a.Cargo[i].Index != 0 {
			return fmt.Errorf("account %q uses reserved cargo slot %d", a.Name, i)
		}
	}
	if len(a.CelestialCapsules) > 65535 {
		return fmt.Errorf("account %q has too many Celestial capsules", a.Name)
	}
	ids := make(map[uint16]struct{}, len(a.CelestialCapsules))
	itemUIDs := make(map[string]struct{}, len(a.CelestialCapsules))
	sourceUIDs := make(map[string]struct{}, len(a.CelestialCapsules))
	for i, capsule := range a.CelestialCapsules {
		if err := capsule.Validate(); err != nil {
			return fmt.Errorf("account %q capsule[%d]: %w", a.Name, i, err)
		}
		if _, exists := ids[capsule.ID]; exists {
			return fmt.Errorf("account %q has a duplicate capsule ID %d", a.Name, capsule.ID)
		}
		if _, exists := itemUIDs[capsule.ItemUID]; exists {
			return fmt.Errorf("account %q has a duplicate capsule item %q", a.Name, capsule.ItemUID)
		}
		if _, exists := sourceUIDs[capsule.SourceUID]; exists {
			return fmt.Errorf("account %q has a duplicate capsule character %q", a.Name, capsule.SourceUID)
		}
		if _, exists := activeUIDs[capsule.SourceUID]; exists {
			return fmt.Errorf("account %q has an active character and a capsule for the same UID %q", a.Name, capsule.SourceUID)
		}
		ids[capsule.ID] = struct{}{}
		itemUIDs[capsule.ItemUID] = struct{}{}
		sourceUIDs[capsule.SourceUID] = struct{}{}
	}
	// Snapshot and item form a single aggregate. The seal may move between the
	// visible Carry and the same account's Cargo, but must exist exactly once.
	// The reverse check also refuses a filled seal without a snapshot.
	sealCounts := make(map[string]int, len(a.CelestialCapsules))
	validateSeal := func(item Item, location string) error {
		id, filled := CelestialSealID(item)
		if !filled {
			return nil
		}
		capsuleIndex := -1
		for i := range a.CelestialCapsules {
			if a.CelestialCapsules[i].ID == id {
				capsuleIndex = i
				break
			}
		}
		if capsuleIndex < 0 {
			return fmt.Errorf("account %q has a filled seal without a snapshot at %s", a.Name, location)
		}
		capsule := a.CelestialCapsules[capsuleIndex]
		if item.UID == "" || item.UID != capsule.ItemUID {
			return fmt.Errorf("account %q capsule %d points to an inconsistent item at %s", a.Name, id, location)
		}
		sealCounts[item.UID]++
		if sealCounts[item.UID] > 1 {
			return fmt.Errorf("account %q has a duplicate seal for capsule %d", a.Name, id)
		}
		return nil
	}
	for i := 0; i < PlayerCargoSlots; i++ {
		if err := validateSeal(a.Cargo[i], fmt.Sprintf("cargo[%d]", i)); err != nil {
			return err
		}
	}
	for charIndex := range a.Chars {
		if a.Chars[charIndex].Name == "" {
			continue
		}
		for slot := 0; slot < PlayerCarrySlots; slot++ {
			if err := validateSeal(a.Chars[charIndex].Inv[slot],
				fmt.Sprintf("character[%d].inv[%d]", charIndex, slot)); err != nil {
				return err
			}
		}
		for slot := range a.Chars[charIndex].Equip {
			if _, filled := CelestialSealID(a.Chars[charIndex].Equip[slot]); filled {
				return fmt.Errorf("account %q has a filled seal in equipment[%d]", a.Name, slot)
			}
		}
	}
	for _, capsule := range a.CelestialCapsules {
		if sealCounts[capsule.ItemUID] != 1 {
			return fmt.Errorf("account %q capsule %d has no seal in Carry/Cargo", a.Name, capsule.ID)
		}
	}
	return nil
}

// Equip keeps the complete STRUCT_ITEM of the mob's 16 slots. The effects are
// needed for the refine/color/mount visual code. In the 7.54 ABI the mount is
// slot 14 and the cape slot 15.
type Equip struct {
	Rosto      Item `json:"rosto"`      // 0  face / base body
	Elmo       Item `json:"elmo"`       // 1  helmet
	Armadura   Item `json:"armadura"`   // 2  armor
	Calca      Item `json:"calca"`      // 3
	Luva       Item `json:"luva"`       // 4
	Bota       Item `json:"bota"`       // 5
	Arma       Item `json:"arma"`       // 6  right hand
	Escudo     Item `json:"escudo"`     // 7  left hand / second weapon
	Acessorio1 Item `json:"acessorio1"` // 8
	Acessorio2 Item `json:"acessorio2"` // 9
	Acessorio3 Item `json:"acessorio3"` // 10
	Acessorio4 Item `json:"acessorio4"` // 11
	Acessorio5 Item `json:"acessorio5"` // 12
	Acessorio6 Item `json:"acessorio6"` // 13
	Montaria   Item `json:"montaria"`   // 14
	Capa       Item `json:"capa"`       // 15
}

func (e Equip) Items() [16]Item {
	return [16]Item{
		e.Rosto, e.Elmo, e.Armadura, e.Calca, e.Luva, e.Bota,
		e.Arma, e.Escudo, e.Acessorio1, e.Acessorio2,
		e.Acessorio3, e.Acessorio4, e.Acessorio5, e.Acessorio6,
		e.Montaria, e.Capa,
	}
}

// VisualItemCode ports GetItemIDEF from PacketProtocolV754.
func VisualItemCode(item Item, mount bool) uint16 {
	if item.Index == 0 {
		return 0
	}
	if mount {
		// A wounded mount stays equipped and persisted, but native SendEquip
		// projects it as an empty slot. Without this rule the owner and players
		// entering view later still draw it alive with zero HP.
		if item.MountHP() <= 0 || item.Index >= 2360 && item.Index < 2390 && item.Eff[1] > 127 {
			return 0
		}
		return uint16(uint32(item.Index) + uint32(item.Eff[2]/10)*0x1000)
	}

	value, colored := 0, false
	for i := 0; i < 3; i++ {
		effect, effectValue := item.Eff[i*2], item.Eff[i*2+1]
		if effect >= 116 && effect <= 125 {
			value, colored = int(effectValue), true
			break
		}
	}
	if !colored {
		for i := 0; i < 3; i++ {
			if item.Eff[i*2] == 43 {
				value = int(item.Eff[i*2+1])
				break
			}
		}
	}
	if value == 0 {
		return item.Index
	}
	if colored {
		if value > 9 {
			value = 9
		}
	} else if value < 230 {
		value %= 10
	} else {
		value = (value-230)/4 + 10
		if value > 16 {
			value = 16
		}
	}
	return uint16(uint32(item.Index) + uint32(value)*0x1000)
}

// AncientCode ports GetAnctCode from PacketProtocolV754.
func AncientCode(item Item) byte {
	value := 0
	for i := 0; i < 3; i++ {
		effect, effectValue := item.Eff[i*2], item.Eff[i*2+1]
		if effect >= 116 && effect <= 125 {
			return effect
		}
		if effect == 43 {
			value = int(effectValue)
		}
	}
	if value == 0 {
		return 0
	}
	if value < 230 {
		return 43
	}
	switch value % 4 {
	case 0:
		return 0x30
	case 1:
		return 0x40
	case 2:
		return 0x10
	default:
		return 0x20
	}
}

func (e Equip) Slots() [16]uint16 {
	items := e.Items()
	var out [16]uint16
	for i, item := range items {
		out[i] = VisualItemCode(item, i == 14)
	}
	return out
}

func (e Equip) AncientCodes() [16]byte {
	items := e.Items()
	var out [16]byte
	for i, item := range items {
		out[i] = AncientCode(item)
	}
	return out
}

// NPC kinds (Tipo field; persisted values stay in Portuguese for data
// compatibility). They define the mob's role on the SERVER (Merchant defines the
// CLIENT UI). Only "monstro" (monster) is attackable/hostile in combat.
const (
	TipoNPC     = "npc"     // default: stationary, not attackable (merchant, quest, guard)
	TipoMonstro = "monstro" // hostile: can be attacked, dies, respawns
)

// NPCDef is the static definition of an NPC (loaded from readable JSON).
type NPCDef struct {
	CombatRole string `json:"combatRole,omitempty"`
	Name       string `json:"name"`
	Tipo       string `json:"tipo,omitempty"` // "monstro" or "npc" (default npc)
	Equip      Equip  `json:"equip"`          // appearance (16 named slots)
	ClassInfo  byte   `json:"classInfo,omitempty"`
	ExpReward  uint32 `json:"expReward"`
	// Score is the NPC's only attribute source. As for characters, the WORD
	// STRUCT_SCORE is only a projection created at the protocol boundary.
	Score *Score `json:"score"`
	// Carry is the monster's DROP inventory (native WYD mechanic): the POSITION
	// in the array is the slot (0..63) and sets the chance through the drop rate
	// table (data/droprate.json, ported from WYD 7.48 g_pDropRate[64]). On death
	// each occupied slot rolls rand()%rate==0; the drop goes STRAIGHT to the killer's inventory.
	Carry []Item `json:"carry,omitempty"`
	// Gold is the native mob "Coin"; on death it rolls the gold drop (MobKilled
	// formula) straight into the killer's gold.
	Gold uint32 `json:"gold,omitempty"`
	// Direction keeps only the high nibble of Status.Merchant. On the wire the
	// client receives Direction | (Score.Merchant & 0x0F). The full Merchant
	// value stays in Score for server-side logic (19, 43 etc.).
	Direction    byte    `json:"direction,omitempty"`
	LearnedSkill uint32  `json:"learnedSkill,omitempty"`
	SkillBar     [4]byte `json:"skillBar,omitempty"`
	// Vende lists the goods of a trading NPC (the mob's Inventory[]; the client
	// buys by sending the sellSlot). Only merchants use it.
	Vende []Item `json:"vende,omitempty"`
}

// IsCityGuard is an explicit server-side role, never inferred from a name or face.
// Guards remain friendly NPCs at the player interaction boundary.
func (n *NPCDef) IsCityGuard() bool {
	return n != nil && n.Tipo == TipoNPC && n.CombatRole == "city_guard"
}

// MaxCarry is the STRUCTURAL Carry size on the wire/JSON and of mob carries.
// The 7.48 client sends 64 STRUCT_ITEM, but index 63 has no cell in the
// player's inventory UI. Never shrink the array: that would break the ABI.
const MaxCarry = 64

// PlayerCarrySlots is the capacity actually reachable in the UI: indices 0..62.
const PlayerCarrySlots = MaxCarry - 1

// ItemListSize is the structural entry count of the 7.48/7.54 ItemList. Valid
// indices are 0 to 6499; larger values in NPC files are residual memory or
// corruption and must never reach the client.
const ItemListSize = 6500

// SkillListSize is the structural 7.48 SkillData table: 96 skills of the four
// classes and special indices 96..103. The wire and LearnedSkill have no valid
// representation outside that range.
const SkillListSize = 104

// MaxCargo is the structural size sent in the 7.48 character list. The client
// has three pages of 40 cells; the last eight slots exist in the protocol but
// have no UI and must never receive player moves.
const MaxCargo = 128
const PlayerCargoSlots = 120

// MaxGenerSegments is the number of route points of the original NPCGener:
// Start, Segment1, Segment2, Segment3 and Dest.
const MaxGenerSegments = 5

// GenerSegment is one route point of a generator. Action is preserved so the
// AI can run the standard file's scripts.
type GenerSegment struct {
	X, Y   uint16
	Range  uint16
	Wait   int
	Action string
}

// NPCGener is one entry of the standard WYD NPCGener.txt. NPCDef stays the
// attribute/appearance template; this struct defines spawning, group and route.
type NPCGener struct {
	Index int
	// Label is the number of the "#[n]" header; instance configs select
	// generator blocks by it.
	Label          int
	Enabled        bool
	MinuteGenerate int
	MaxNumMob      int
	MinGroup       int
	MaxGroup       int
	Leader         string
	Follower       string
	RouteType      int
	Formation      int
	Segments       [MaxGenerSegments]GenerSegment
}

// Teleport is one server-side entry of data/teleports.ini. The portal fires on
// 0x290 when the player stands in the 4x4 square whose origin is SourceX/SourceY.
type Teleport struct {
	SourceX, SourceY uint16
	DestX, DestY     uint16
	Price            uint32
	Name             string
}

// IsMonster reports whether the mob is hostile/attackable (tipo "monstro"). A
// nonzero Merchant always identifies an NPC role in the protocol; this second
// lock stops a badly converted template from turning a shop/crafter into a mob.
func (n *NPCDef) IsMonster() bool {
	return n != nil && n.Tipo == TipoMonstro && (n.Score == nil || n.Score.Merchant == 0)
}

// Mesh returns the 16 appearance slots (CreateMob ItemEff@34) as a slice.
func (n *NPCDef) Mesh() []uint16 {
	s := n.Equip.Slots()
	return s[:]
}

// MakeScore clones the authoritative attributes and injects current resources.
func (n *NPCDef) MakeScore(currentHP uint32) *Score {
	if n == nil || n.Score == nil {
		return &Score{Version: ScoreVersion}
	}
	score := *n.Score
	score.Version = ScoreVersion
	direction := uint32(n.Direction & 0xF0)
	score.Merchant = direction | (score.Merchant & 0x0F)
	score.CurHP = currentHP
	if score.CurHP > score.MaxHP {
		score.CurHP = score.MaxHP
	}
	score.CurMP = score.MaxMP
	return &score
}
