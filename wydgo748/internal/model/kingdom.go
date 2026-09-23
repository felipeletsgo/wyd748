package model

const (
	KingdomNeutral   byte = 0
	KingdomHekalotia byte = 7
	KingdomAkelonia  byte = 8

	CapeSlot      = 15
	SapphireItem  = 697
	SapphirePack  = 4131
	KingdomEmblem = 4081
)

// KingdomCapeTier represents the five parallel cape families in the 7.48
// client. The tier is not inferred from the numeric item value: all three
// variants (Hekalotia, Akelonia, and neutral) are listed by native source.
type KingdomCapeTier byte

const (
	CapeTierBasic  KingdomCapeTier = iota // 545/546/548: Medal of Kingdom, level 220
	CapeTierKnight                        // 543/544/549: Crusader/Shiner, level 256
	CapeTierElite                         // 3191/3192/3193: Arch level 355
	CapeTierHero                          // 3194/3195/3196: Soul
	CapeTierMaster                        // 3197/3198/3199: Celestial
)

type kingdomCapeFamily struct {
	Hekalotia uint16
	Akelonia  uint16
	Neutral   uint16
}

// Order confirmed in KingDomCape[2][5] and CapeBroker[5] from 7.54 WYD 7.48.
var kingdomCapeFamilies = [...]kingdomCapeFamily{
	{Hekalotia: 545, Akelonia: 546, Neutral: 548},
	{Hekalotia: 543, Akelonia: 544, Neutral: 549},
	{Hekalotia: 3191, Akelonia: 3192, Neutral: 3193},
	{Hekalotia: 3194, Akelonia: 3195, Neutral: 3196},
	{Hekalotia: 3197, Akelonia: 3198, Neutral: 3199},
}

// KingdomFromCape follows BASE_GetCapeInfo/GetCurScore_CapeInfo: kingdom is
// not separate persisted character state; it is always derived from the cape.
func KingdomFromCape(index uint16) byte {
	switch index {
	case 543, 545, 734, 736, 1767, 3191, 3194, 3197, 3300, 3303, 3306:
		return KingdomHekalotia
	case 544, 546, 735, 737, 1770, 3192, 3195, 3198, 3301, 3304, 3307:
		return KingdomAkelonia
	default:
		return KingdomNeutral
	}
}

func KingdomName(kingdom byte) string {
	switch kingdom {
	case KingdomHekalotia:
		return "Hekalotia"
	case KingdomAkelonia:
		return "Akelonia"
	default:
		return "Neutral"
	}
}

// KingdomCapeTierOf also identifies the neutral variant. Special capes outside
// the native matrix still define a kingdom in KingdomFromCape, but they cannot
// be promoted or neutralized as if they belonged to one of these families.
func KingdomCapeTierOf(index uint16) (KingdomCapeTier, bool) {
	for tier, family := range kingdomCapeFamilies {
		if index == family.Hekalotia || index == family.Akelonia || index == family.Neutral {
			return KingdomCapeTier(tier), true
		}
	}
	return 0, false
}

func KingdomCapeAtTier(tier KingdomCapeTier, kingdom byte) (uint16, bool) {
	if int(tier) >= len(kingdomCapeFamilies) {
		return 0, false
	}
	family := kingdomCapeFamilies[tier]
	switch kingdom {
	case KingdomHekalotia:
		return family.Hekalotia, true
	case KingdomAkelonia:
		return family.Akelonia, true
	case KingdomNeutral:
		return family.Neutral, true
	default:
		return 0, false
	}
}

// KingdomCape converts a neutral cape to its kingdom equivalent. An empty cape
// and Wanderer use the basic medal; higher tiers preserve their category,
// as in the WYD 7.48 KING case.
func KingdomCape(index uint16, kingdom byte) (uint16, bool) {
	if index == 0 {
		return KingdomCapeAtTier(CapeTierBasic, kingdom)
	}
	tier, ok := KingdomCapeTierOf(index)
	if !ok || KingdomFromCape(index) != KingdomNeutral {
		return 0, false
	}
	return KingdomCapeAtTier(tier, kingdom)
}

// NeutralCape is the inverse conversion used by the Kingdom Broker.
func NeutralCape(index uint16) (uint16, bool) {
	tier, ok := KingdomCapeTierOf(index)
	if ok && KingdomFromCape(index) != KingdomNeutral {
		return KingdomCapeAtTier(tier, KingdomNeutral)
	}
	// These special Celestial capes use the Master broker in WYD 7.48.
	if index == 1767 || index == 1770 {
		return KingdomCapeAtTier(CapeTierMaster, KingdomNeutral)
	}
	return 0, false
}
