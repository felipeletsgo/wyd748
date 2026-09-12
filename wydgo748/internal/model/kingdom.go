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

// KingdomCapeTier representa as cinco familias paralelas de capa existentes
// no client 7.48. O tier nao e inferido pelo valor numerico do item: as tres
// variantes (Hekalotia, Akelonia e neutra) sao tabeladas pela source nativa.
type KingdomCapeTier byte

const (
	CapeTierBasic  KingdomCapeTier = iota // 545/546/548: Medal of Kingdom, nivel 220
	CapeTierKnight                        // 543/544/549: Crusader/Shiner, nivel 256
	CapeTierElite                         // 3191/3192/3193: Arch nivel 355
	CapeTierHero                          // 3194/3195/3196: Soul
	CapeTierMaster                        // 3197/3198/3199: Celestial
)

type kingdomCapeFamily struct {
	Hekalotia uint16
	Akelonia  uint16
	Neutral   uint16
}

// Ordem confirmada em KingDomCape[2][5] e CapeBroker[5] da 7.54 Secrets.
var kingdomCapeFamilies = [...]kingdomCapeFamily{
	{Hekalotia: 545, Akelonia: 546, Neutral: 548},
	{Hekalotia: 543, Akelonia: 544, Neutral: 549},
	{Hekalotia: 3191, Akelonia: 3192, Neutral: 3193},
	{Hekalotia: 3194, Akelonia: 3195, Neutral: 3196},
	{Hekalotia: 3197, Akelonia: 3198, Neutral: 3199},
}

// KingdomFromCape segue BASE_GetCapeInfo/GetCurScore_CapeInfo: o reino nao e
// um segundo estado persistido no personagem; ele e sempre derivado da capa.
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
		return "Neutro"
	}
}

// KingdomCapeTierOf identifica inclusive a variante neutra. Capas especiais
// fora da matriz nativa continuam definindo reino em KingdomFromCape, mas nao
// podem ser promovidas nem neutralizadas como se fossem uma destas familias.
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

// KingdomCape converte a capa neutra para o equivalente do reino. A capa
// vazia e a Wanderer entram pela medalha basica; tiers superiores preservam a
// categoria, como no case KING do W2PP.
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

// NeutralCape e a conversao inversa usada pelo Kingdom Broker.
func NeutralCape(index uint16) (uint16, bool) {
	tier, ok := KingdomCapeTierOf(index)
	if ok && KingdomFromCape(index) != KingdomNeutral {
		return KingdomCapeAtTier(tier, KingdomNeutral)
	}
	// Estas capas especiais Celestiais usam o broker Master na W2PP.
	if index == 1767 || index == 1770 {
		return KingdomCapeAtTier(CapeTierMaster, KingdomNeutral)
	}
	return 0, false
}
