// Package login materializa os contratos de autenticação, seleção de
// personagem e entrada no mundo do client WYD 7.48.
//
// O wire reproduz os layouts comprovados no executável nativo
// (PARIDADE_NATIVA). Os agregados e a máquina de estados são uma
// MODERNIZACAO_COMPATIVEL: não portam classes, globals ou ownership implícito
// do TMProject. O servidor continua sendo a única autoridade de gameplay.
package login

const (
	OpcodeCharacterList            uint16 = 0x10A
	OpcodeEnterWorld               uint16 = 0x114
	OpcodeCharacterLogoutConfirmed uint16 = 0x116
	OpcodeAccountLogin             uint16 = 0x20D
	OpcodeCharacterLogin           uint16 = 0x213
	OpcodeCharacterLogout          uint16 = 0x215
)

const (
	AccountLoginPacketSize                    = 116
	CharacterListPacketSize                   = 2360
	CharacterLoginPacketSize                  = 36
	EnterWorldPacketSize                      = 2104
	CharacterLogoutPacketSize                 = 12
	CharacterLogoutConfirmedPacketSize        = 12
	ScoreSize                                 = 35 * 4
	ItemSize                                  = 8
	SelectionCharacterCount                   = 4
	SelectionCharacterSize                    = 1272
	SelectionEquipmentCount                   = 18
	CargoItemCount                            = 128
	MobSize                                   = 1224
	MobEquipmentCount                         = 18
	MobCarryCount                             = 64
	AffectSize                                = 8
	AffectCount                               = 16
	ShortSkillCount                           = 16
	ClientVersion                      uint32 = 748
)

const (
	accountNameSize   = 16
	passwordSize      = 12
	secretCodeSize    = 16
	characterNameSize = 16
	mobNameSize       = 12
)

// Score preserva as 35 palavras uint32 do STRUCT_SCORE atual. A ordem dos
// campos é parte do contrato wire e coincide com a projeção autoritativa do
// WYD-Go; nenhum valor recebido volta ao servidor como fonte de verdade.
type Score struct {
	Version       uint32
	Level         uint32
	Attack        uint32
	MagicAttack   uint32
	Defense       uint32
	MaxHP         uint32
	MaxMP         uint32
	CurHP         uint32
	CurMP         uint32
	Str           uint32
	Int           uint32
	Dex           uint32
	Con           uint32
	Accuracy      uint32
	Evasion       uint32
	Parry         uint32
	Critical      uint32
	Range         uint32
	ResistFire    uint32
	ResistIce     uint32
	ResistHoly    uint32
	ResistThunder uint32
	SaveMana      uint32
	MagicAmp      uint32
	RegenHP       uint32
	RegenMP       uint32
	StatusPts     uint32
	MasterPts     uint32
	SkillPts      uint32
	Mastery       [4]uint32
	AttackRun     uint32
	Merchant      uint32
}

// Item é a cópia de apresentação dos oito bytes conhecidos pelo client. UID,
// validade econômica e persistência continuam exclusivamente no servidor.
type Item struct {
	Index   uint16
	Effects [6]byte
}

// Affect representa um registro visual de oito bytes da STRUCT_EXT1. Time é o
// número de unidades restantes enviado pelo servidor, não um relógio local
// autoritativo.
type Affect struct {
	Type  byte
	Level byte
	Value int16
	Time  uint32
}

// CharacterSummary contém somente o snapshot exibido na seleção. Nome vazio
// identifica slot livre; a UI não deve inferir ocupação por Score ou item.
type CharacterSummary struct {
	HomeTownX uint16
	HomeTownY uint16
	Name      string
	Score     Score
	Equip     [SelectionEquipmentCount]Item
	Guild     uint16
	Coin      uint32
	Exp       uint64
}

func (c CharacterSummary) Occupied() bool { return c.Name != "" }

// CharacterList é o agregado completo do 0x10A. SecretCode é opaco e só pode
// ser reutilizado na seleção de personagem da mesma sessão autenticada.
type CharacterList struct {
	SecretCode  [secretCodeSize]byte
	Characters  [SelectionCharacterCount]CharacterSummary
	Cargo       [CargoItemCount]Item
	CargoGold   uint32
	AccountName string
	SSN1        uint32
	SSN2        uint32
}

// Mob é o STRUCT_MOB de 1224 bytes entregue no primeiro snapshot de mundo.
// Campos reservados são copiados para manter o parser total sem atribuir
// semântica não comprovada a bytes desconhecidos.
type Mob struct {
	Name                  string
	Chaos                 byte
	NameReserved          [3]byte
	Clan                  byte
	AlignmentReserved     byte
	Guild                 uint16
	Class                 byte
	Reserved              byte
	Quest                 uint16
	Coin                  uint32
	PreExpPadding         [4]byte
	Exp                   uint64
	HomeTownX             uint16
	HomeTownY             uint16
	BaseScore             Score
	CurrentScore          Score
	Equip                 [MobEquipmentCount]Item
	Carry                 [MobCarryCount]Item
	LearnedSkill          uint32
	SecondaryLearnedSkill uint32
	ShortSkill            [4]byte
	GuildLevel            byte
	ReservedTail          [227]byte
	CurrentKill           uint16
	TotalKill             uint16
}

// WorldSnapshot é publicado somente depois de o 0x114 inteiro passar pela
// validação. ExtensionData contém os sete DWORDs posteriores a Hold;
// ExtensionReserved preserva a região ainda sem semântica necessária.
type WorldSnapshot struct {
	PosX              uint16
	PosY              uint16
	Mob               Mob
	Slot              uint16
	ClientID          uint16
	Weather           uint16
	ShortSkill        [ShortSkillCount]byte
	Hold              uint32
	ExtensionData     [7]uint32
	Affects           [AffectCount]Affect
	ExtensionReserved [680]byte
}

// AccountLoginRequest usa []byte para a senha para que o owner possa apagá-la
// depois do envio. O builder não guarda referência para esse slice.
type AccountLoginRequest struct {
	Account  string
	Password []byte
	Adapter  [4]uint32
	ID       uint16
	Tick     uint32
}

// CharacterLoginRequest descreve o 0x213. SecretCode não é interpretado pelo
// client e nunca deve atravessar uma reconexão TCP.
type CharacterLoginRequest struct {
	Slot       int32
	Force      int32
	SecretCode [secretCodeSize]byte
	ID         uint16
	Tick       uint32
}
