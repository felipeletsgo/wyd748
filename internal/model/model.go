// Package model define os tipos de dominio persistidos/estaticos do WYD-Go.
// Sao valores puros (sem dependencias externas) usados por wire, store, data e game.
package model

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"time"
)

// Item = STRUCT_ITEM do wire (8 bytes): sIndex + 3 pares (efeito, valor). Em
// JSON, {index, eff:[6]} -- ex.: uma pilha de 120 e eff:[61,120,0,0,0,0]
// (EF_AMOUNT=61, valor 120).
type Item struct {
	Index uint16  `json:"index"`
	Eff   [6]byte `json:"eff,omitempty"`
}

// MarshalJSON omite o campo "eff" quando esta todo zerado -- "omitempty" NAO
// funciona em array em Go, entao um item comum sai enxuto como `{"index":N}`.
func (it Item) MarshalJSON() ([]byte, error) {
	if it.Eff == [6]byte{} {
		return json.Marshal(struct {
			Index uint16 `json:"index"`
		}{it.Index})
	}
	type alias Item
	return json.Marshal(alias(it))
}

// WireScore = STRUCT_SCORE 7.48 (28 bytes no wire). Merchant/AttackRun sao bytes
// CHEIOS (Merchant: low=tipo|high=direcao; AttackRun: Att*16+Run) -- NAO nibbles
// separados. Este tipo nunca e persistido nem usado em calculos.
type WireScore struct {
	Level     uint16  `json:"-"`
	Defense   uint16  `json:"-"`
	Attack    uint16  `json:"-"`
	Merchant  byte    `json:"-"`
	AttackRun byte    `json:"-"`
	MaxHP     uint16  `json:"-"`
	MaxMP     uint16  `json:"-"`
	CurHP     uint16  `json:"-"`
	CurMP     uint16  `json:"-"`
	Str       uint16  `json:"-"`
	Int       uint16  `json:"-"`
	Dex       uint16  `json:"-"`
	Con       uint16  `json:"-"`
	Mastery   [4]byte `json:"-"`
}

const (
	ExtendedScoreVersion  uint32 = 2
	MaxExtendedScoreValue uint32 = 2_000_000_000
)

// ExtendedScore e o score autoritativo de 32 bits usado por personagens e mobs.
// WireScore continua sendo apenas a projecao de ABI de 28 bytes; nenhuma rotina
// server-side de combate deve usa-lo como fonte de verdade.
//
// Os calculos intermediarios do game usam int64 e os resultados persistidos
// ficam limitados a 2.000.000.000, evitando overflow nos campos signed do client.
type ExtendedScore struct {
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
	AttackRun     byte      `json:"attackRun,omitempty"`
	Merchant      byte      `json:"merchant,omitempty"`
}

// Validate garante que todo estado persistido usa exatamente o contrato atual
// e que nenhum valor ultrapassa os inteiros signed usados nos calculos.
func (e *ExtendedScore) Validate() error {
	if e == nil {
		return fmt.Errorf("extendedScore ausente")
	}
	if e.Version != ExtendedScoreVersion {
		return fmt.Errorf("extendedScore v%d; esperado v%d", e.Version, ExtendedScoreVersion)
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
		if field.value > MaxExtendedScoreValue {
			return fmt.Errorf("extendedScore.%s=%d excede %d",
				field.name, field.value, MaxExtendedScoreValue)
		}
	}
	for i, value := range e.Mastery {
		if value > MaxExtendedScoreValue {
			return fmt.Errorf("extendedScore.mastery[%d]=%d excede %d",
				i, value, MaxExtendedScoreValue)
		}
	}
	return nil
}

const (
	compatibilityMaximum = uint32(30_000)
	compatibilityDerived = uint32(1_000)
)

// CompatibilityHPScale e o divisor que leva o HP real para a faixa que o
// STRUCT_SCORE nativo comporta. Vale 1 enquanto o MaxHP couber direto.
//
// Existe exportado porque NAO basta projetar o HP: o dano dos pacotes de
// ataque viaja num WORD que o client subtrai do CurHP JA PROJETADO. Mandar o
// dano cru fazia a barra cair `scale` vezes mais rapido que o correto, e
// depois saltar de volta quando chegava o 0x181.
func CompatibilityHPScale(maximum uint32) uint32 {
	if maximum <= compatibilityMaximum {
		return 1
	}
	return (maximum + compatibilityMaximum - 1) / compatibilityMaximum
}

// ProjectHPDelta converte uma variacao de HP real (dano, cura) para a escala
// do prefixo nativo. Arredonda para CIMA: com MaxHP alto, truncar deixaria
// todo golpe menor que a escala invisivel na barra.
func ProjectHPDelta(delta, maximum uint32) uint16 {
	if delta == 0 {
		return 0
	}
	scale := CompatibilityHPScale(maximum)
	projected := (delta + scale - 1) / scale
	// O teto e a BARRA CHEIA do alvo, nao a constante de 30.000: com MaxHP de
	// 5.000.000 a barra projetada vale 29.941, e um overkill que mandasse
	// 30.000 ainda passaria dela. Alvo sem MaxHP conhecido cai na constante.
	teto := compatibilityMaximum
	if maximum > 0 {
		teto = (maximum + scale - 1) / scale
	}
	if projected > teto {
		projected = teto
	}
	return uint16(projected)
}

func projectCompatibilityPair(current, maximum uint32) (uint16, uint16) {
	if current > maximum {
		current = maximum
	}
	if maximum <= compatibilityMaximum {
		return uint16(current), uint16(maximum)
	}
	scale := CompatibilityHPScale(maximum)
	project := func(value uint32) uint16 {
		if value == 0 {
			return 0
		}
		value = (value + scale - 1) / scale
		if value > compatibilityMaximum {
			value = compatibilityMaximum
		}
		return uint16(value)
	}
	return project(current), project(maximum)
}

func minScoreValue(value, maximum uint32) uint32 {
	if value < maximum {
		return value
	}
	return maximum
}

// CompatibilityScore gera a projecao signed-safe consumida pelo motor 7.48.
// Os valores reais continuam exclusivamente neste ExtendedScore.
func (e *ExtendedScore) CompatibilityScore() WireScore {
	if e == nil {
		return WireScore{}
	}
	score := WireScore{
		Level:     uint16(minScoreValue(e.Level, 65_535)),
		Defense:   uint16(minScoreValue(e.Defense, compatibilityDerived)),
		Attack:    uint16(minScoreValue(e.Attack, compatibilityDerived)),
		Merchant:  e.Merchant,
		AttackRun: e.AttackRun,
		Str:       uint16(minScoreValue(e.Str, compatibilityDerived)),
		Int:       uint16(minScoreValue(e.Int, compatibilityDerived)),
		Dex:       uint16(minScoreValue(e.Dex, compatibilityDerived)),
		Con:       uint16(minScoreValue(e.Con, compatibilityDerived)),
	}
	score.CurHP, score.MaxHP = projectCompatibilityPair(e.CurHP, e.MaxHP)
	score.CurMP, score.MaxMP = projectCompatibilityPair(e.CurMP, e.MaxMP)
	for i := range score.Mastery {
		score.Mastery[i] = byte(minScoreValue(e.Mastery[i], 255))
	}
	return score
}

// ElementalResists segue a ordem funcional da 7.59: Resist[0..3] =
// Fogo, Gelo, Sagrado e Trovao. Fica fora do STRUCT_SCORE 7.48 e pertence ao
// estado autoritativo do servidor.
type ElementalResists struct {
	Fire    uint32 `json:"fire"`
	Ice     uint32 `json:"ice"`
	Sacred  uint32 `json:"sacred"`
	Thunder uint32 `json:"thunder"`
}

// Affect e o estado temporario autoritativo de buff/debuff. O client recebe
// apenas Type e o tempo restante; Value/Level alimentam as formulas server-side.
type Affect struct {
	Type       byte      `json:"type,omitempty"`
	ClientType byte      `json:"clientType,omitempty"` // slot visual do client quando difere da semantica server-side
	Value      int       `json:"value,omitempty"`
	Level      int       `json:"level,omitempty"`
	OwnerID    uint16    `json:"ownerId,omitempty"`
	ExpiresAt  time.Time `json:"expiresAt,omitempty"`
	NextTick   time.Time `json:"nextTick,omitempty"`
}

// Char = personagem persistido (1 slot da conta).
type Char struct {
	Name  string `json:"name"`
	Class byte   `json:"class"`
	X     uint16 `json:"x"`
	Y     uint16 `json:"y"`
	// Jogadores nao armazenam STRUCT_SCORE. Ele e projetado na borda.
	Extended *ExtendedScore `json:"extendedScore"`
	// ExtendedRuntime contem o score efetivo depois de buffs/debuffs. Extended
	// permanece a base persistida; separar os dois impede que cada recálculo
	// acumule Possuido/transformacoes e grave o bonus temporario no JSON.
	ExtendedRuntime *ExtendedScore `json:"-"`
	Equip           [16]Item       `json:"equip"` // slots de equipamento
	Inv             [64]Item       `json:"inv"`   // wire/persistencia tem 64; UI usa 0..62
	Chaos           uint32         `json:"chaos"`
	Gold            uint32         `json:"gold"`
	Exp             uint32         `json:"exp"`
	NextExp         uint32         `json:"-"` // derivado da tabela pelo level
	LearnedSkill    uint32         `json:"learnedSkill"`
	MagicalPillUsed bool           `json:"magicalPillUsed,omitempty"`
	SkillPointBonus uint32         `json:"skillPointBonus,omitempty"`
	// Progressao avancada usada pelas composicoes Ehre/Odin. Evolution vazio e
	// tratado como "mortal"; valores futuros: arch, celestial e subcelestial.
	Evolution string `json:"evolution,omitempty"`
	// ArchMortalSlot e o slot do Mortal que originou este Arch e
	// ArchMortalLevel guarda o nivel DELE. So tem sentido quando Evolution e
	// "arch"; nos demais personagens ficam zerados e sao ignorados.
	//
	// O bonus de status do Arch cresce com o nivel do Mortal, e o nativo o
	// recalcula a cada login a partir do personagem no slot -- ou seja, subir o
	// Mortal DEPOIS da ascensao continua fortalecendo o Arch. Guardamos o slot
	// para reproduzir isso e o nivel em cache porque o calculo de pontos recebe
	// apenas o Char, sem acesso a conta.
	// ArchCrystals conta os cristais elementais ja consumidos, de 0 a 4. Eles
	// sao feitos EM ORDEM (Elime, Sylphid, Salion, Nohas) e cada um cobra 100
	// milhoes de EXP. Concluir os quatro e requisito para o Celestial.
	//
	// O nativo guarda isso em QuestInfo.Arch.Cristal -- o campo esta no bloco
	// do ARCH, e e como Arch que a quest e feita.
	ArchCrystals             byte   `json:"archCrystals,omitempty"`
	// ArchLevel355 e ArchLevel370 sao as travas de nivel do Arch. Enquanto
	// falsas, o personagem PARA de receber EXP ao chegar no nivel interno 354
	// e 369 -- destravar e um craft na Lindy. Sao QuestInfo.Arch.Level355 e
	// .Level370 no nativo.
	ArchLevel355             bool   `json:"archLevel355,omitempty"`
	ArchLevel370             bool   `json:"archLevel370,omitempty"`
	ArchMortalSlot           int    `json:"archMortalSlot,omitempty"`
	ArchMortalLevel          uint32 `json:"archMortalLevel,omitempty"`
	// A fama NAO mora aqui: ela e um contador por personagem em
	// CharState.SpecialCoins["fame"], gravado em data/charstate/<nome>.json.
	// Ver internal/game/counters.go.
	SoulInfo                 uint8 `json:"soulInfo,omitempty"` // 1..10; zero = nenhum
	CelestialLevel40Unlocked bool   `json:"celestialLevel40Unlocked,omitempty"`
	// Habilidades da evolucao; separadas dos 24 bits Mortais como na W2PP.
	SecondaryLearnedSkill uint32     `json:"secondaryLearnedSkill,omitempty"`
	ShortSkill            [20]byte   `json:"shortSkill,omitempty"`
	Affects               [16]Affect `json:"affects,omitempty"`
	// GuildID/GuildRank sao copias DESNORMALIZADAS do registro canonico em
	// guilds.json, mantidas aqui so para o wire e consulta rapida. O login
	// repara divergencias contra o registro; a guild e sempre a fonte da
	// verdade. Ausentes no JSON antigo carregam como zero = sem guild.
	GuildID   uint16 `json:"guildId,omitempty"`
	GuildRank byte   `json:"guildRank,omitempty"`
	// QuestsDone guarda os IDs de quest ja concluidas. Lista em vez de mascara
	// de bits: mascara limitaria o projeto a 64 quests e amarraria o id ao bit.
	QuestsDone []int32 `json:"questsDone,omitempty"`
	// Citizenship e o CANAL do qual o personagem e cidadao (0 = nenhum). No
	// nativo o valor e ServerIndex+1, ou seja, "Cidadania 1" = canal 1. Ele
	// so rende bonus no proprio canal -- ver CitizenDropBonus.
	Citizenship byte `json:"citizenship,omitempty"`
	// SavedX/SavedY sao a coordenada gravada pela Gema Estelar (volatile 12) e
	// usada pelo Warp (volatile 13). Persistem na ficha; 0/0 = nenhuma salva.
	SavedX uint16 `json:"savedX,omitempty"`
	SavedY uint16 `json:"savedY,omitempty"`
}

// charEquip = os 16 slots de equip do char como OBJETO NOMEADO, mantendo a riqueza do tipo Item.
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

// MarshalJSON grava o equip como OBJETO nomeado e o inventario com TODOS os 64
// slots (sem cortar) -- salvar e carregar viram simetricos e todo slot fica visivel.
// O alias embutido evita duplicar os campos escalares (o equip/inv de fora, na
// profundidade 0, vencem os do alias, profundidade 1, por terem a mesma tag json).
// IsEmpty diz se o slot de personagem esta realmente vazio. Substitui a
// comparacao direta com Char{}, impossivel desde que QuestsDone virou slice
// (struct com slice nao e comparavel em Go).
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

// equalsZero compara o restante dos campos por valor. Fica separado para que a
// adicao de um campo nao-comparavel no futuro falhe na compilacao aqui, e nao
// silenciosamente em outro lugar.
func (c Char) equalsZero() bool {
	return c.Name == "" && c.Class == 0 && c.X == 0 && c.Y == 0 &&
		c.Extended == nil && c.ExtendedRuntime == nil &&
		c.Equip == [16]Item{} && c.Inv == [64]Item{} &&
		c.Chaos == 0 && c.Gold == 0 && c.Exp == 0 && c.NextExp == 0 &&
		c.LearnedSkill == 0 && c.SecondaryLearnedSkill == 0 &&
		!c.MagicalPillUsed && c.SkillPointBonus == 0 &&
		c.ShortSkill == [20]byte{} && c.Affects == [16]Affect{} &&
		c.GuildID == 0 && c.GuildRank == 0
}

func (c Char) MarshalJSON() ([]byte, error) {
	if c.IsEmpty() {
		return []byte("null"), nil
	}
	if c.Name == "" {
		return nil, fmt.Errorf("slot de personagem sem nome contem estado")
	}
	type alias Char

	return json.Marshal(struct {
		alias
		Equip charEquip `json:"equip"`
		Inv   []Item    `json:"inv"`
	}{
		alias: alias(c),
		Equip: toCharEquip(c.Equip),
		Inv:   c.Inv[:], // todos os 64 slots
	})
}

// UnmarshalJSON le exclusivamente o formato autoritativo atual: extendedScore
// v2, equipamento nomeado e inventario posicional completo.
func (c *Char) UnmarshalJSON(b []byte) error {
	if bytes.Equal(bytes.TrimSpace(b), []byte("null")) {
		*c = Char{}
		return nil
	}
	type alias Char
	aux := struct {
		alias
		Equip charEquip `json:"equip"`
		Inv   []Item    `json:"inv"`
	}{
		alias: alias(*c),
	}
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&aux); err != nil {
		return err
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		return fmt.Errorf("personagem %q possui JSON adicional", aux.Name)
	}
	*c = Char(aux.alias)
	if err := c.Extended.Validate(); err != nil {
		return fmt.Errorf("personagem %q: %w", c.Name, err)
	}
	if len(aux.Inv) != len(c.Inv) {
		return fmt.Errorf("personagem %q possui %d slots de inventario; esperado %d",
			c.Name, len(aux.Inv), len(c.Inv))
	}
	c.Equip = aux.Equip.array()
	for i := range c.Inv {
		c.Inv[i] = aux.Inv[i]
	}
	return nil
}

// Account = conta persistida: login + senha + ate 4 chars.
type Account struct {
	Name         string    `json:"name"`
	PasswordHash string    `json:"passwordHash"`
	Chars        []Char    `json:"chars"`
	Cargo        [128]Item `json:"cargo"`               // bau compartilhado pelos quatro personagens
	CargoGold    uint32    `json:"cargoGold,omitempty"` // gold guardado/vendas da Loja Fantasma
}

// UnmarshalJSON exige o cargo estrutural completo. Os oito slots sem UI
// continuam no arquivo para preservar a ABI, mas Account.Validate exige zeros.
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
		return fmt.Errorf("conta %q possui JSON adicional", aux.Name)
	}
	if len(aux.Cargo) != MaxCargo {
		return fmt.Errorf("conta %q possui %d slots de cargo; esperado %d",
			aux.Name, len(aux.Cargo), MaxCargo)
	}
	*a = Account(aux.alias)
	copy(a.Cargo[:], aux.Cargo)
	return nil
}

func (a *Account) Validate() error {
	if a == nil {
		return fmt.Errorf("conta ausente")
	}
	if a.Name == "" {
		return fmt.Errorf("conta sem nome")
	}
	if a.PasswordHash == "" {
		return fmt.Errorf("conta %q sem passwordHash", a.Name)
	}
	if len(a.Chars) > 4 {
		return fmt.Errorf("conta %q possui %d personagens; maximo 4", a.Name, len(a.Chars))
	}
	for i := range a.Chars {
		character := &a.Chars[i]
		if character.Name == "" {
			if !character.IsEmpty() {
				return fmt.Errorf("conta %q personagem[%d] vazio contem estado", a.Name, i)
			}
			continue
		}
		if err := character.Extended.Validate(); err != nil {
			return fmt.Errorf("conta %q personagem[%d] %q: %w",
				a.Name, i, character.Name, err)
		}
		if character.Inv[PlayerCarrySlots].Index != 0 {
			return fmt.Errorf("conta %q personagem[%d] %q ocupa slot de inventario reservado %d",
				a.Name, i, character.Name, PlayerCarrySlots)
		}
	}
	for i := PlayerCargoSlots; i < MaxCargo; i++ {
		if a.Cargo[i].Index != 0 {
			return fmt.Errorf("conta %q ocupa slot de cargo reservado %d", a.Name, i)
		}
	}
	return nil
}

// Equip preserva os STRUCT_ITEM completos dos 16 slots do mob. Os efeitos sao
// necessarios para o codigo visual de refino/cor/montaria. Na ABI 7.54,
// montaria e slot 14 e capa e slot 15.
type Equip struct {
	Rosto      Item `json:"rosto"`      // 0  rosto/corpo base
	Elmo       Item `json:"elmo"`       // 1  capacete
	Armadura   Item `json:"armadura"`   // 2  torso
	Calca      Item `json:"calca"`      // 3
	Luva       Item `json:"luva"`       // 4
	Bota       Item `json:"bota"`       // 5
	Arma       Item `json:"arma"`       // 6  mao direita
	Escudo     Item `json:"escudo"`     // 7  mao esquerda / 2a arma
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

// VisualItemCode porta GetItemIDEF do PacketProtocolV754.
func VisualItemCode(item Item, mount bool) uint16 {
	if item.Index == 0 {
		return 0
	}
	if mount {
		if item.Index >= 2360 && item.Index < 2390 && item.Eff[1] > 127 {
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

// AncientCode porta GetAnctCode do PacketProtocolV754.
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

// Tipos de NPC (campo Tipo). Define o papel do mob no SERVIDOR (o Merchant define
// a UI do lado do CLIENT). So "monstro" e atacavel/hostil no combate.
const (
	TipoNPC     = "npc"     // padrao: parado, nao atacavel (mercador, quest, guarda)
	TipoMonstro = "monstro" // hostil: pode ser atacado, morre, renasce
)

// NPCDef = definicao estatica de um NPC (carregada de JSON legivel).
type NPCDef struct {
	Name      string `json:"name"`
	Tipo      string `json:"tipo,omitempty"` // "monstro" ou "npc" (default npc)
	Equip     Equip  `json:"equip"`          // aparencia (16 slots nomeados)
	ClassInfo byte   `json:"classInfo,omitempty"`
	ExpReward uint32 `json:"expReward"`
	// Extended e a unica fonte de atributos do NPC. Assim como nos personagens,
	// STRUCT_SCORE WORD e somente uma projecao criada na borda do protocolo.
	Extended *ExtendedScore `json:"extendedScore"`
	// Carry = o inventario de DROP do monstro (mecanica nativa do WYD): a POSICAO
	// no array e o slot (0..63) e define a chance via a tabela de drop rates
	// (data/droprate.json, portada do g_pDropRate[64] do W2PP). Na morte, cada slot
	// ocupado rola rand()%rate==0; o item dropado vai DIRETO pro inventario do killer.
	Carry []Item `json:"carry,omitempty"`
	// Gold = o "Coin" do mob nativo; na morte rola o drop de gold (formula do
	// MobKilled) direto pro gold do killer.
	Gold uint32 `json:"gold,omitempty"`
	// Direction preserva somente o nibble alto de Status.Merchant. No wire, o
	// client recebe Direction | (Extended.Merchant & 0x0F). O valor completo de
	// Merchant continua em Extended para a logica server-side (19, 43 etc.).
	Direction    byte    `json:"direction,omitempty"`
	LearnedSkill uint32  `json:"learnedSkill,omitempty"`
	SkillBar     [4]byte `json:"skillBar,omitempty"`
	// Vende = itens a venda de um NPC de comercio (o Inventory[] do mob; o client
	// compra mandando o sellSlot). So mercadores usam.
	Vende []Item `json:"vende,omitempty"`
}

// MaxCarry e o tamanho ESTRUTURAL do Carry no wire/JSON e do carry dos mobs.
// O client 7.48 transmite 64 STRUCT_ITEM, mas o indice 63 nao possui celula na
// UI do inventario do jogador. Nunca reduzir o array: isso quebraria o ABI.
const MaxCarry = 64

// PlayerCarrySlots e a capacidade realmente acessivel na UI: indices 0..62.
const PlayerCarrySlots = MaxCarry - 1

// ItemListSize e a quantidade estrutural de entradas do ItemList 7.48/7.54.
// Indices validos vao de 0 a 6499; valores maiores em arquivos de NPC sao
// memoria residual/corrupcao e nunca podem chegar ao client.
const ItemListSize = 6500

// MaxCargo e o tamanho estrutural transmitido no char-list 7.48. O cliente
// possui tres paginas de 40 celulas; os oito slots finais existem no protocolo,
// mas nao possuem UI e nunca podem receber movimentacoes do jogador.
const MaxCargo = 128
const PlayerCargoSlots = 120

// MaxGenerSegments e o numero de pontos de rota do NPCGener original:
// Start, Segment1, Segment2, Segment3 e Dest.
const MaxGenerSegments = 5

// GenerSegment representa um ponto da rota de um gerador. Action fica preservado
// para a futura IA executar os scripts do arquivo padrao.
type GenerSegment struct {
	X, Y   uint16
	Range  uint16
	Wait   int
	Action string
}

// NPCGener e uma entrada do NPCGener.txt padrao do WYD. NPCDef continua sendo o
// template de atributos/aparencia; esta struct define geracao, grupo e rota.
type NPCGener struct {
	Index          int
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

// Teleport representa uma entrada server-side de data/teleports.ini.
// O portal e acionado pelo 0x290 quando o jogador esta no quadrado 4x4 cuja
// origem e SourceX/SourceY.
type Teleport struct {
	SourceX, SourceY uint16
	DestX, DestY     uint16
	Price            uint32
	Name             string
}

// IsMonster diz se o mob e hostil/atacavel (tipo "monstro"). Merchant diferente
// de zero sempre identifica uma funcao de NPC no protocolo; essa segunda trava
// impede um template convertido incorretamente de transformar loja/artesao em mob.
func (n *NPCDef) IsMonster() bool {
	return n != nil && n.Tipo == TipoMonstro && (n.Extended == nil || n.Extended.Merchant == 0)
}

// Mesh devolve os 16 slots de aparencia (ItemEff@34 do CreateMob) como slice.
func (n *NPCDef) Mesh() []uint16 {
	s := n.Equip.Slots()
	return s[:]
}

// MakeExtendedScore clona os atributos autoritativos e injeta os recursos atuais.
func (n *NPCDef) MakeExtendedScore(currentHP uint32) *ExtendedScore {
	if n == nil || n.Extended == nil {
		return &ExtendedScore{Version: ExtendedScoreVersion}
	}
	score := *n.Extended
	score.Version = ExtendedScoreVersion
	direction := n.Direction & 0xF0
	score.Merchant = direction | (score.Merchant & 0x0F)
	score.CurHP = minScoreValue(currentHP, score.MaxHP)
	score.CurMP = score.MaxMP
	return &score
}
