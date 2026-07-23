package model

// Sistema de montaria fiel ao WYD/W2PP. O estado de cada montaria vive nos 3
// slots de efeito do proprio item (Equip[14]), exatamente como o nativo:
//
//	Eff[0..1] (uint16 LE) = HP da montaria      (stEffect[0].sValue, ate ~30000)
//	Eff[2]                = level               (stEffect[1].cEffect, 0..120)
//	Eff[3]                = longevidade/vidas    (stEffect[1].cValue, 0..60)
//	Eff[4]                = comida               (stEffect[2].cEffect, 0..100)
//	Eff[5]                = flag interno          (stEffect[2].cValue)
//
// O sIndex codifica tipo+estagio: ovo 2300-2329, cria 2330-2358, adulta
// 2360-2388. Tipo = (sIndex-base)%30; adulta = cria+30. As faixas batem 1:1 com
// o nosso itemlist (mesmo client 7.48 do Micronics).
const (
	MountEggBase   = 2300 // primeiro ovo (Pig's_Egg)
	MountBabyBase  = 2330 // primeira cria (Pig's_Baby)
	MountAdultBase = 2360 // primeira adulta (Pig)
	MountTypeCount = 30

	MountMaxLevel     = 120
	MountMaxLongevity = 60
	MountMaxFood      = 100
	MountMaxHP        = 30000
	MountEssenceHP    = 20000 // HP que o amago restaura
)

// MountStats sao os atributos base de um tipo de montaria (g_pMountBonus do
// W2PP): [AttackAdd, MagicAdd, Evasion, Resistence, Speed]. Dano e magia escalam
// com o level da montaria; evasao/resist/velocidade sao flat.
type MountStats struct {
	Attack  int    `json:"attack"`
	Magic   int    `json:"magic"`
	Evasion int    `json:"evasion"`
	Resist  int    `json:"resist"`
	Speed   int    `json:"speed"`
	MaxHP   int    `json:"maxHP,omitempty"`
	Name    string `json:"name,omitempty"`
}

// MountCatalog resolve os atributos de um tipo de montaria (0..29). Vazio, o
// slot de montaria nao adiciona bonus.
type MountCatalog struct {
	Types map[int]MountStats
}

// Stats devolve os atributos do tipo de montaria (0..29).
func (c MountCatalog) Stats(mountType int) (MountStats, bool) {
	if c.Types == nil {
		return MountStats{}, false
	}
	s, ok := c.Types[mountType]
	return s, ok
}

// MountType extrai o indice de tipo (0..29) de um sIndex de ovo/cria/adulta.
// Devolve -1 se o sIndex nao for de montaria.
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

// IsMountEgg / IsMountBaby / IsMountAdult classificam o estagio pelo sIndex.
func IsMountEgg(sIndex uint16) bool {
	return sIndex >= MountEggBase && sIndex < MountEggBase+MountTypeCount
}

func IsMountBaby(sIndex uint16) bool {
	return sIndex >= MountBabyBase && sIndex < MountBabyBase+MountTypeCount
}

func IsMountAdult(sIndex uint16) bool {
	return sIndex >= MountAdultBase && sIndex < MountAdultBase+MountTypeCount
}

// IsMount diz se o item e uma montaria (cria ou adulta -- ovo ainda nao conta,
// fica no inventario chocando). So estas usam os slots de efeito como estado.
func IsMount(sIndex uint16) bool {
	return IsMountBaby(sIndex) || IsMountAdult(sIndex)
}

// --- Acessores do estado guardado nos slots de efeito do item ---

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
