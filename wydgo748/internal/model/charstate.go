package model

// CharState stores persistent character session state separately from the
// account aggregate. Production uses character_states keyed by Char.UID;
// the JSON adapter uses charstate/<UID>.json. Active buffs and counters can
// therefore survive reconnects.
//
// A sidecar preserves the strict account JSON contract: affects have absolute
// expiry times and are transient state rather than part of the account record.
type CharState struct {
	Version int `json:"version"`
	// Affects contains active buffs/debuffs with absolute expiry in Unix seconds.
	// Loading discards expired entries and restores the remaining lifetime.
	Affects []PersistedAffect `json:"affects,omitempty"`
	// SpecialCoins stores named character counters, including fame and special
	// currencies. Names are persisted identifiers, not localized display text.
	SpecialCoins map[string]uint32 `json:"specialCoins,omitempty"`
}

// PersistedAffect is the serializable form of an Affect: domain fields plus
// its absolute expiry time.
type PersistedAffect struct {
	Type              byte   `json:"type"`
	ClientType        byte   `json:"clientType,omitempty"`
	Value             int    `json:"value,omitempty"`
	Level             int    `json:"level,omitempty"`
	OwnerID           uint16 `json:"ownerId,omitempty"`
	OwnerCharacterUID string `json:"ownerCharacterUid,omitempty"`
	// Economic provenance remains in the PostgreSQL payload for the buff's
	// entire lifetime, even after the consumed unit leaves the inventory.
	SourceItemUID   string `json:"sourceItemUid,omitempty"`
	SourceItemIndex uint16 `json:"sourceItemIndex,omitempty"`
	ExpiresUnix     int64  `json:"expiresUnix"`
	// Poison Mist retains its four scheduled ticks across reconnects.
	FinalTickAtExpiry bool  `json:"finalTickAtExpiry,omitempty"`
	NextTickUnix      int64 `json:"nextTickUnix,omitempty"`
}

// CharStateVersion identifies the sidecar contract version. Additive optional
// fields (such as OwnerCharacterUID) retain this version so older currency and
// buff records remain readable; new required fields need a migration.
const CharStateVersion = 1
