package model

// Mount system faithful to WYD 7.48. Each mount's state lives in the three
// effect pairs of its own item (Equip[14]), exactly as in the native client:
//
//	Eff[0..1] (uint16 LE) = mount HP           (stEffect[0].sValue, up to ~30000)
//	Eff[2]                = level              (stEffect[1].cEffect, 0..120)
//	Eff[3]                = longevity/lives    (stEffect[1].cValue, 0..60)
//	Eff[4]                = food               (stEffect[2].cEffect, 0..100)
//	Eff[5]                = internal flag      (stEffect[2].cValue)
//
// sIndex encodes type and stage: egg 2300-2329, baby 2330-2358, adult
// 2360-2388. Type = (sIndex-base)%30; adult = baby+30. The ranges match the
// 7.48 item list one to one.
const (
	MountEggBase   = 2300 // first egg (Pig's_Egg)
	MountBabyBase  = 2330 // first baby (Pig's_Baby)
	MountAdultBase = 2360 // first adult (Pig)
	MountTypeCount = 30

	MountMaxLevel     = 120
	MountMaxLongevity = 60
	MountMaxFood      = 100
	MountMaxHP        = 30000
	MountEssenceHP    = 20000 // HP restored by an essence
)

// MountStats are the base attributes of a mount type (native g_pMountBonus):
// [AttackAdd, MagicAdd, Evasion, Resistance, Speed, RiderDamagePct]. Damage and
// magic scale with the mount level; evasion, resistance and speed are flat.
// RiderDamagePct is the share of damage kept by the rider when the mount
// absorbs damage.
type MountStats struct {
	Attack         int    `json:"attack"`
	Magic          int    `json:"magic"`
	Evasion        int    `json:"evasion"`
	Resist         int    `json:"resist"`
	Speed          int    `json:"speed"`
	RiderDamagePct int    `json:"riderDamagePct,omitempty"`
	MaxHP          int    `json:"maxHP,omitempty"`
	Name           string `json:"name,omitempty"`
}

// PremiumMountStats describe a timed horse 3980..3999. These items are not
// baby/adult mounts and their item-list rows carry no stats or lifetime; the
// client draws them from its g_pMountBonus2 table, which this data mirrors
// server-side: flat attack, magic attack percentage, flat evasion and
// resistance, the final mounted speed and the lifetime in days.
type PremiumMountStats struct {
	Name         string `json:"name"`
	Attack       int    `json:"attack"`
	MagicPercent int    `json:"magicPercent"`
	Evasion      int    `json:"evasion"`
	Resist       int    `json:"resist"`
	Speed        int    `json:"speed"`
	DurationDays int    `json:"durationDays"`
}

// MountCatalog resolves mount type attributes (0..29) and timed premium
// horses by item index. An empty catalog adds no mount bonus.
type MountCatalog struct {
	Types   map[int]MountStats
	Premium map[uint16]PremiumMountStats
}

// Stats returns the attributes of a mount type (0..29).
func (c MountCatalog) Stats(mountType int) (MountStats, bool) {
	if c.Types == nil {
		return MountStats{}, false
	}
	s, ok := c.Types[mountType]
	return s, ok
}

// PremiumStats returns the attributes of a timed premium horse.
func (c MountCatalog) PremiumStats(index uint16) (PremiumMountStats, bool) {
	s, ok := c.Premium[index]
	return s, ok
}

// MountType extracts the type index (0..29) of an egg/baby/adult sIndex. It
// returns -1 when sIndex is not a mount.
func MountType(sIndex uint16) int {
	switch {
	case sIndex >= MountEggBase && sIndex < MountEggBase+MountTypeCount:
		return int(sIndex - MountEggBase)
	case sIndex >= MountBabyBase && sIndex < MountBabyBase+MountTypeCount:
		return int(sIndex - MountBabyBase)
	case sIndex >= MountAdultBase && sIndex < MountAdultBase+MountTypeCount:
		return int(sIndex - MountAdultBase)
	}
	return -1
}

// IsMountEgg, IsMountBaby and IsMountAdult classify the stage by sIndex.
func IsMountEgg(sIndex uint16) bool {
	return sIndex >= MountEggBase && sIndex < MountEggBase+MountTypeCount
}

func IsMountBaby(sIndex uint16) bool {
	return sIndex >= MountBabyBase && sIndex < MountBabyBase+MountTypeCount
}

func IsMountAdult(sIndex uint16) bool {
	return sIndex >= MountAdultBase && sIndex < MountAdultBase+MountTypeCount
}

// IsMount reports whether the item is a mount (baby or adult; an egg still
// hatches in the inventory). Only these use the effect pairs as state.
func IsMount(sIndex uint16) bool {
	return IsMountBaby(sIndex) || IsMountAdult(sIndex)
}

// --- Accessors for the state stored in the item effect pairs ---

func (it Item) MountHP() int     { return int(it.Eff[0]) | int(it.Eff[1])<<8 }
func (it Item) MountLevel() int  { return int(it.Eff[2]) }
func (it Item) MountLongev() int { return int(it.Eff[3]) }
func (it Item) MountFood() int   { return int(it.Eff[4]) }

func (it *Item) SetMountHP(hp int) {
	if hp < 0 {
		hp = 0
	}
	if hp > 65535 {
		hp = 65535
	}
	it.Eff[0] = byte(hp & 0xFF)
	it.Eff[1] = byte(hp >> 8 & 0xFF)
}

func (it *Item) SetMountLevel(v int) {
	it.Eff[2] = byte(clampByte(v))
}

func (it *Item) SetMountLongev(v int) {
	it.Eff[3] = byte(clampByte(v))
}

func (it *Item) SetMountFood(v int) {
	it.Eff[4] = byte(clampByte(v))
}

func clampByte(v int) int {
	if v < 0 {
		return 0
	}
	if v > 255 {
		return 255
	}
	return v
}
