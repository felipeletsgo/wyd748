// Package model define os tipos de dominio persistidos/estaticos do WYD-Go.
// Sao valores puros (sem dependencias externas) usados por wire, store, data e game.
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
	// ChaosFormatVersion marca o modelo assinado adotado pelo emulador. O
	// formato anterior persistia CP=75 como "neutro"; a primeira leitura sem
	// esta marca migra esse valor para CP 0 e grava a marca no próximo save.
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

// Item contem o STRUCT_ITEM nativo (Index+Eff = 8 bytes) e a identidade
// exclusivamente server-side da instancia. UID nunca e serializado no wire:
// lojas/NPCs/templates usam UID vazio como blueprint; itens pertencentes a uma
// conta ou soltos no mundo recebem um UID ao serem materializados.
type Item struct {
	Index uint16  `json:"index"`
	UID   string  `json:"uid,omitempty"`
	Eff   [6]byte `json:"eff,omitempty"`
}

// MarshalJSON omite o campo "eff" quando esta todo zerado -- "omitempty" NAO
// funciona em array em Go, entao um item comum sai enxuto como `{"index":N}`.
func (it Item) MarshalJSON() ([]byte, error) {
	if it.Eff == [6]byte{} {
		return json.Marshal(struct {
			Index uint16 `json:"index"`
			UID   string `json:"uid,omitempty"`
		}{Index: it.Index, UID: it.UID})
	}
	type alias Item
	return json.Marshal(alias(it))
}

// WireEqual compara somente os oito bytes que o client conhece. Snapshots
// recebidos do 7.48 nunca carregam UID; depois de validar esses bytes, o servidor
// deve manter a copia autoritativa que ja possui a identidade.
func (it Item) WireEqual(other Item) bool {
	return it.Index == other.Index && it.Eff == other.Eff
}

// NewItemUID gera 128 bits aleatorios em hexadecimal. Nao depende de contador
// persistente e continua globalmente unico entre Windows, VPS e restauracoes de
// backup. O store ainda verifica colisao/duplicidade antes de persistir.
func NewItemUID() (string, error) {
	var raw [16]byte
	if _, err := rand.Read(raw[:]); err != nil {
		return "", fmt.Errorf("gerar UID de item: %w", err)
	}
	// Variante/versao de UUID v4; armazenamos sem hifens para ocupar menos JSON.
	raw[6] = raw[6]&0x0f | 0x40
	raw[8] = raw[8]&0x3f | 0x80
	return hex.EncodeToString(raw[:]), nil
}

// NormalizeItemUID valida e canonicaliza um UID persistido.
func NormalizeItemUID(uid string) (string, error) {
	if uid == "" {
		return "", nil
	}
	uid = strings.ToLower(uid)
	if len(uid) != ItemUIDHexLength {
		return "", fmt.Errorf("UID %q possui %d caracteres; esperado %d",
			uid, len(uid), ItemUIDHexLength)
	}
	raw, err := hex.DecodeString(uid)
	if err != nil || len(raw) != 16 {
		return "", fmt.Errorf("UID %q invalido", uid)
	}
	return uid, nil
}

// NewCharacterUID cria a identidade persistente server-side de um personagem.
// Nome e slot pertencem ao protocolo/UI e podem se repetir ou mudar; UID nunca
// vai ao wire e continua identificando Mortal, Arch e charstate sem ambiguidade.
func NewCharacterUID() (string, error) {
	return newPersistentUID()
}

// NormalizeCharacterUID valida a representacao hexadecimal persistida.
func NormalizeCharacterUID(uid string) (string, error) {
	return normalizePersistentUID(uid, CharacterUIDHexLength, "UID de personagem")
}

func newPersistentUID() (string, error) {
	var raw [16]byte
	if _, err := rand.Read(raw[:]); err != nil {
		return "", fmt.Errorf("gerar UID: %w", err)
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
		return "", fmt.Errorf("%s %q possui %d caracteres; esperado %d",
			label, uid, len(uid), length)
	}
	raw, err := hex.DecodeString(uid)
	if err != nil || len(raw) != 16 {
		return "", fmt.Errorf("%s %q invalido", label, uid)
	}
	return uid, nil
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

// ValidatePlayerState applies the functional limits of a persisted character.
// NPC source data keeps byte-domain sentinel values (notably SaveMana=255)
// inherited from the native files, so those static definitions use Validate;
// account/character state must pass this stricter boundary.
func (e *ExtendedScore) ValidatePlayerState() error {
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
			return fmt.Errorf("extendedScore.%s=%d excede 100", resistance.name, resistance.value)
		}
	}
	if e.SaveMana > 99 {
		return fmt.Errorf("extendedScore.saveMana=%d excede 99", e.SaveMana)
	}
	if e.RegenHP > 255 || e.RegenMP > 255 {
		return fmt.Errorf("extendedScore regen fora de 0..255: hp=%d mp=%d", e.RegenHP, e.RegenMP)
	}
	if e.CurHP > e.MaxHP {
		return fmt.Errorf("extendedScore.curHP=%d excede maxHP=%d", e.CurHP, e.MaxHP)
	}
	if e.CurMP > e.MaxMP {
		return fmt.Errorf("extendedScore.curMP=%d excede maxMP=%d", e.CurMP, e.MaxMP)
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
	Type       byte   `json:"type,omitempty"`
	ClientType byte   `json:"clientType,omitempty"` // slot visual do client quando difere da semantica server-side
	Value      int    `json:"value,omitempty"`
	Level      int    `json:"level,omitempty"`
	OwnerID    uint16 `json:"ownerId,omitempty"`
	// OwnerCharacterUID is the stable source identity for player-owned
	// hostile affects. OwnerID is a process-local wire/session id and is kept
	// only for compatibility with active legacy sessions.
	OwnerCharacterUID string    `json:"ownerCharacterUid,omitempty"`
	ExpiresAt         time.Time `json:"expiresAt,omitempty"`
	NextTick          time.Time `json:"nextTick,omitempty"`
}

// CelestialForm guarda exclusivamente a forma INATIVA do par
// Celestial/SubCelestial. Equipamentos (exceto o rosto), inventario, gold,
// capa/reino, fama e quests continuam no Char e sao compartilhados. Classe,
// rosto, EXP, atributos, aprendizagens, skills, barra e affects pertencem a
// cada forma, como Persos[0..1] da 7.54.
type CelestialForm struct {
	Evolution             string         `json:"evolution"`
	Class                 byte           `json:"class"`
	Face                  Item           `json:"face"`
	Extended              *ExtendedScore `json:"extendedScore"`
	Exp                   uint32         `json:"exp"`
	LearnedSkill          uint32         `json:"learnedSkill"`
	SecondaryLearnedSkill uint32         `json:"secondaryLearnedSkill,omitempty"`
	ShortSkill            [20]byte       `json:"shortSkill,omitempty"`
	Affects               [16]Affect     `json:"affects,omitempty"`
}

func (f *CelestialForm) Validate() error {
	if f == nil {
		return nil
	}
	evolution := strings.ToLower(strings.TrimSpace(f.Evolution))
	if evolution != "celestial" && evolution != "subcelestial" {
		return fmt.Errorf("forma celestial possui evolucao invalida %q", f.Evolution)
	}
	if f.Class > 3 {
		return fmt.Errorf("forma celestial possui classe invalida %d", f.Class)
	}
	if f.Face.Index == 0 {
		return fmt.Errorf("forma celestial sem rosto")
	}
	if err := f.Extended.ValidatePlayerState(); err != nil {
		return fmt.Errorf("forma celestial: %w", err)
	}
	if f.Extended.Level > 199 {
		return fmt.Errorf("forma celestial possui level interno %d; maximo 199",
			f.Extended.Level)
	}
	return nil
}

// Char = personagem persistido (1 slot da conta).
type Char struct {
	UID   string `json:"uid"`
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
	// CP e o Chaos/PK Point exibido, assinado e autoritativo (-75..+75).
	// CP 0 e o estado neutro de um personagem novo; o byte legado de CreateMob
	// usa CP+75 somente na borda do protocolo.
	CP              int16  `json:"cp"`
	ChaosVersion    uint8  `json:"chaosVersion,omitempty"`
	Gold            uint32 `json:"gold"`
	Exp             uint32 `json:"exp"`
	NextExp         uint32 `json:"-"` // derivado da tabela pelo level
	LearnedSkill    uint32 `json:"learnedSkill"`
	MagicalPillUsed bool   `json:"magicalPillUsed,omitempty"`
	SkillPointBonus uint32 `json:"skillPointBonus,omitempty"`
	// Progressao avancada usada pelas composicoes Ehre/Odin. Evolution vazio e
	// tratado como "mortal"; valores futuros: arch, celestial e subcelestial.
	Evolution string `json:"evolution,omitempty"`
	// ArchMortalUID identifica o Mortal que originou este Arch e
	// ArchMortalLevel guarda o nivel DELE. So tem sentido quando Evolution e
	// "arch"; nos demais personagens ficam vazios/zerados e sao ignorados.
	//
	// O bonus de status do Arch cresce com o nivel do Mortal, e o nativo o
	// recalcula a cada login a partir do personagem de origem -- ou seja, subir o
	// Mortal DEPOIS da ascensao continua fortalecendo o Arch. O vinculo usa UID,
	// nunca slot: excluir/reordenar personagens nao pode apontar o Arch para
	// outra ficha. O nivel fica em cache porque o calculo recebe apenas o Char.
	// ArchCrystals conta os cristais elementais ja consumidos, de 0 a 4. Eles
	// sao feitos EM ORDEM (Elime, Sylphid, Salion, Nohas) e cada um cobra 100
	// milhoes de EXP. Cada um concede 100 pontos ao futuro Celestial; a criacao
	// em si nao exige que os quatro tenham sido concluídos.
	//
	// O nativo guarda isso em QuestInfo.Arch.Cristal -- o campo esta no bloco
	// do ARCH, e e como Arch que a quest e feita.
	ArchCrystals byte `json:"archCrystals,omitempty"`
	// ArchLevel355 e ArchLevel370 sao as travas de nivel do Arch. Enquanto
	// falsas, o personagem PARA de receber EXP ao chegar no nivel interno 354
	// e 369 -- destravar e um craft na Lindy. Sao QuestInfo.Arch.Level355 e
	// .Level370 no nativo.
	ArchLevel355    bool   `json:"archLevel355,omitempty"`
	ArchLevel370    bool   `json:"archLevel370,omitempty"`
	ArchMortalUID   string `json:"archMortalUid,omitempty"`
	ArchMortalLevel uint32 `json:"archMortalLevel,omitempty"`
	// A fama NAO mora aqui: ela e um contador por personagem em
	// CharState.SpecialCoins["fame"], gravado pela identidade UID do personagem.
	// Ver internal/game/counters.go.
	SoulInfo                 uint8 `json:"soulInfo,omitempty"` // 1..10; zero = nenhum
	CelestialLevel40Unlocked bool  `json:"celestialLevel40Unlocked,omitempty"`
	CelestialLevel90Unlocked bool  `json:"celestialLevel90Unlocked,omitempty"`
	// CelestialArchTier registra em qual faixa o Arch virou Celestial:
	// 1=356..369, 2=370..379, 3=380..398, 4=399, 5=400 exibidos.
	// CelestialCytheraTier (0..4) impede repetir os adicionais de 121/151/181/199.
	CelestialArchTier    byte `json:"celestialArchTier,omitempty"`
	CelestialCytheraTier byte `json:"celestialCytheraTier,omitempty"`
	CelestialArcana      bool `json:"celestialArcana,omitempty"`
	// AlternateCelestial e a forma que nao esta materializada no Char. O ativo
	// permanece nos campos normais para todo combate/protocolo continuar usando
	// uma unica fonte autoritativa.
	AlternateCelestial *CelestialForm `json:"alternateCelestial,omitempty"`
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
	// Escritura do Pesadelo (volatile 212): cada uso credita 13 entradas e
	// inicia cooldown de 12 horas. Fica na ficha para item+saldo+cooldown serem
	// confirmados no mesmo SaveAccount.
	NightmareTickets  uint32 `json:"nightmareTickets,omitempty"`
	LastNightmareUnix int64  `json:"lastNightmareUnix,omitempty"`
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

// equalsZero compara todos os demais campos persistidos por valor. Ao adicionar
// estado ao Char, este predicado e seus testes precisam ser atualizados: um slot
// sem nome nunca pode ser serializado como null enquanto ainda contem dominio.
func (c Char) equalsZero() bool {
	return c.UID == "" && c.Name == "" && c.Class == 0 && c.X == 0 && c.Y == 0 &&
		c.Extended == nil && c.ExtendedRuntime == nil &&
		c.Equip == [16]Item{} && c.Inv == [64]Item{} &&
		c.CP == 0 && c.ChaosVersion == 0 && c.Gold == 0 && c.Exp == 0 && c.NextExp == 0 &&
		c.LearnedSkill == 0 && c.SecondaryLearnedSkill == 0 &&
		!c.MagicalPillUsed && c.SkillPointBonus == 0 &&
		c.Evolution == "" && c.ArchCrystals == 0 && !c.ArchLevel355 &&
		!c.ArchLevel370 && c.ArchMortalUID == "" && c.ArchMortalLevel == 0 &&
		c.SoulInfo == 0 && !c.CelestialLevel40Unlocked &&
		!c.CelestialLevel90Unlocked && c.CelestialArchTier == 0 &&
		c.CelestialCytheraTier == 0 && !c.CelestialArcana &&
		c.AlternateCelestial == nil &&
		c.ShortSkill == [20]byte{} && c.Affects == [16]Affect{} &&
		c.GuildID == 0 && c.GuildRank == 0 && c.Citizenship == 0 &&
		c.SavedX == 0 && c.SavedY == 0 && c.NightmareTickets == 0 &&
		c.LastNightmareUnix == 0
}

func (c Char) MarshalJSON() ([]byte, error) {
	if c.IsEmpty() {
		return []byte("null"), nil
	}
	if c.Name == "" {
		return nil, fmt.Errorf("slot de personagem sem nome contem estado")
	}
	type alias Char
	encoded := alias(c)
	// Todo personagem persistido pelo formato atual leva a marca, inclusive
	// quando algum teste/ferramenta construiu Char{CP:75} diretamente. Sem ela
	// a próxima leitura teria de interpretar +75 como o antigo neutro.
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
		return fmt.Errorf("personagem %q possui JSON adicional", aux.Name)
	}
	*c = Char(aux.alias)
	if aux.LegacyChaos != nil {
		if _, hasCP := fields["cp"]; hasCP {
			return fmt.Errorf("personagem %q possui cp e chaos simultaneamente", c.Name)
		}
		// Migracao unica do formato anterior. O campo legado era o byte bruto
		// nativo (1..150), em que 75 e o personagem neutro. O zero usado pelos
		// JSONs antigos significava "sem estado" e tambem deve virar neutro,
		// nunca CP +75. A borda do protocolo converte CP assinado para CP+75.
		if *aux.LegacyChaos == 0 {
			c.CP = 0
		} else {
			c.CP = ClampCP(int(*aux.LegacyChaos) - 75)
		}
	}
	if c.ChaosVersion == 0 {
		// Antes da separação CP/Hold, o template e alguns saves escreviam
		// `cp:75` para representar um personagem neutro. Não há marcador para
		// distinguir esses saves antigos de um +75 recém-obtido; a migração
		// única prioriza preservar o estado neutro e marca o formato novo. A
		// remissão usada depois desta versão permanece +75 porque já carrega a
		// marca.
		if _, hasCP := fields["cp"]; hasCP && aux.LegacyChaos == nil && c.CP == MaxCP {
			c.CP = 0
		}
		c.ChaosVersion = ChaosFormatVersion
	}
	if c.ChaosVersion != 0 && c.ChaosVersion != ChaosFormatVersion {
		return fmt.Errorf("personagem %q possui versao de chaos invalida: %d", c.Name, c.ChaosVersion)
	}
	if c.CP < MinCP || c.CP > MaxCP {
		return fmt.Errorf("personagem %q possui cp fora de -75..75: %d", c.Name, c.CP)
	}
	if err := c.Extended.ValidatePlayerState(); err != nil {
		return fmt.Errorf("personagem %q: %w", c.Name, err)
	}
	if err := c.AlternateCelestial.Validate(); err != nil {
		return fmt.Errorf("personagem %q: %w", c.Name, err)
	}
	if c.ArchCrystals > 4 || c.SoulInfo > 10 ||
		c.CelestialArchTier > 5 || c.CelestialCytheraTier > 4 {
		return fmt.Errorf("personagem %q possui progresso Arch/Celestial invalido", c.Name)
	}
	activeEvolution := strings.ToLower(strings.TrimSpace(c.Evolution))
	if (activeEvolution == "celestial" || activeEvolution == "subcelestial") &&
		c.Extended.Level > 199 {
		return fmt.Errorf("personagem %q possui level Celestial interno %d; maximo 199",
			c.Name, c.Extended.Level)
	}
	if c.AlternateCelestial != nil {
		active := activeEvolution
		alternate := strings.ToLower(strings.TrimSpace(c.AlternateCelestial.Evolution))
		if active != "celestial" && active != "subcelestial" {
			return fmt.Errorf("personagem %q possui forma alterna sem forma celestial ativa", c.Name)
		}
		if active == alternate {
			return fmt.Errorf("personagem %q possui duas formas %q", c.Name, active)
		}
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
	// CelestialCapsules sao selos persistentes que guardam uma ficha Celestial
	// retirada do mundo. O item 3443 no Carry/Cargo referencia ID; o snapshot completo
	// fica aqui para que Postgres/JSON confirmem item, personagem e estado em uma
	// unica gravacao, sem depender de arquivos numerados fora da transacao.
	CelestialCapsules []CelestialCapsule `json:"celestialCapsules,omitempty"`
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
	activeUIDs := make(map[string]struct{}, len(a.Chars))
	for i := range a.Chars {
		character := &a.Chars[i]
		if character.Name == "" {
			if !character.IsEmpty() {
				return fmt.Errorf("conta %q personagem[%d] vazio contem estado", a.Name, i)
			}
			continue
		}
		if err := character.Extended.ValidatePlayerState(); err != nil {
			return fmt.Errorf("conta %q personagem[%d] %q: %w",
				a.Name, i, character.Name, err)
		}
		if err := character.AlternateCelestial.Validate(); err != nil {
			return fmt.Errorf("conta %q personagem[%d] %q: %w",
				a.Name, i, character.Name, err)
		}
		normalized, err := NormalizeCharacterUID(character.UID)
		if err != nil {
			return fmt.Errorf("conta %q personagem[%d] %q: %w",
				a.Name, i, character.Name, err)
		}
		if normalized == "" {
			return fmt.Errorf("conta %q personagem[%d] %q sem UID",
				a.Name, i, character.Name)
		}
		if normalized != character.UID {
			return fmt.Errorf("conta %q personagem[%d] %q possui UID nao canonico",
				a.Name, i, character.Name)
		}
		if _, duplicate := activeUIDs[character.UID]; duplicate {
			return fmt.Errorf("conta %q possui UID de personagem duplicado %q", a.Name, character.UID)
		}
		activeUIDs[character.UID] = struct{}{}
		if character.ArchMortalUID != "" {
			if _, err := NormalizeCharacterUID(character.ArchMortalUID); err != nil {
				return fmt.Errorf("conta %q personagem[%d] %q: %w",
					a.Name, i, character.Name, err)
			}
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
	if len(a.CelestialCapsules) > 65535 {
		return fmt.Errorf("conta %q possui capsulas Celestial demais", a.Name)
	}
	ids := make(map[uint16]struct{}, len(a.CelestialCapsules))
	itemUIDs := make(map[string]struct{}, len(a.CelestialCapsules))
	sourceUIDs := make(map[string]struct{}, len(a.CelestialCapsules))
	for i, capsule := range a.CelestialCapsules {
		if err := capsule.Validate(); err != nil {
			return fmt.Errorf("conta %q capsula[%d]: %w", a.Name, i, err)
		}
		if _, exists := ids[capsule.ID]; exists {
			return fmt.Errorf("conta %q possui ID de capsula duplicado %d", a.Name, capsule.ID)
		}
		if _, exists := itemUIDs[capsule.ItemUID]; exists {
			return fmt.Errorf("conta %q possui item de capsula duplicado %q", a.Name, capsule.ItemUID)
		}
		if _, exists := sourceUIDs[capsule.SourceUID]; exists {
			return fmt.Errorf("conta %q possui personagem de capsula duplicado %q", a.Name, capsule.SourceUID)
		}
		if _, exists := activeUIDs[capsule.SourceUID]; exists {
			return fmt.Errorf("conta %q possui personagem ativo e capsula para o mesmo UID %q", a.Name, capsule.SourceUID)
		}
		ids[capsule.ID] = struct{}{}
		itemUIDs[capsule.ItemUID] = struct{}{}
		sourceUIDs[capsule.SourceUID] = struct{}{}
	}
	// Snapshot e item formam um único agregado. O selo pode circular entre o
	// Carry visível e o Cargo da mesma conta, mas deve existir exatamente uma vez.
	// A validação reversa também recusa selo preenchido sem snapshot.
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
			return fmt.Errorf("conta %q possui selo preenchido sem snapshot em %s", a.Name, location)
		}
		capsule := a.CelestialCapsules[capsuleIndex]
		if item.UID == "" || item.UID != capsule.ItemUID {
			return fmt.Errorf("conta %q capsula %d aponta para item inconsistente em %s", a.Name, id, location)
		}
		sealCounts[item.UID]++
		if sealCounts[item.UID] > 1 {
			return fmt.Errorf("conta %q possui selo duplicado da capsula %d", a.Name, id)
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
				fmt.Sprintf("personagem[%d].inv[%d]", charIndex, slot)); err != nil {
				return err
			}
		}
		for slot := range a.Chars[charIndex].Equip {
			if _, filled := CelestialSealID(a.Chars[charIndex].Equip[slot]); filled {
				return fmt.Errorf("conta %q possui selo preenchido em equipamento[%d]", a.Name, slot)
			}
		}
	}
	for _, capsule := range a.CelestialCapsules {
		if sealCounts[capsule.ItemUID] != 1 {
			return fmt.Errorf("conta %q capsula %d sem selo no Carry/Cargo", a.Name, capsule.ID)
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
		// A montaria ferida continua equipada e persistida, mas o SendEquip
		// nativo a projeta como slot vazio. Sem esta regra, o dono e jogadores
		// que entram depois na visão ainda a desenham viva com HP zero.
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

// SkillListSize e a tabela estrutural do SkillData 7.48: 96 skills das quatro
// classes e os indices especiais 96..103. O wire e LearnedSkill nao possuem
// representacao valida fora dessa faixa.
const SkillListSize = 104

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
