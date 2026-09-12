package model

// StaticEffect e um par nome/valor do itemlist.csv. O nome EF_* e mantido para
// que o servidor possa portar as formulas nativas sem depender do ID usado pelo
// binario do cliente.
type StaticEffect struct {
	Name  string
	Value int
}

// ItemDef e a fonte autoritativa de um item. Aparencia, requisitos, preco,
// posicao e efeitos vem exclusivamente dos arquivos carregados no boot.
type ItemDef struct {
	Index         uint16
	Name          string
	Mesh          int
	Texture       int
	ReqLevel      int
	ReqStr        int
	ReqInt        int
	ReqDex        int
	ReqCon        int
	Unique        int
	Price         uint32
	Pos           int
	Extra         int
	Grade         int
	StaticEffects []StaticEffect
	// DynamicEffectNames traduz os IDs dos efeitos gravados na instancia do
	// item. O mapa compartilhado vem de ItemEffect.h; nao deve ser recriado no
	// codigo do servidor.
	DynamicEffectNames map[byte]string
}

// SkillDef preserva as 23 colunas autoritativas do SkillData.csv (ID, 21
// parametros, duas sequencias de acao e nome). Nao existe uma coluna
// "Unknown": o layout server-side precisa permanecer alinhado ao arquivo de
// dados, sem inventar campos para preencher structs de outra versao.
type SkillDef struct {
	Index             int
	SkillPoint        int
	TargetType        int
	ManaSpent         int
	Delay             int
	Range             int
	InstanceType      int
	InstanceValue     int
	TickType          int
	TickValue         int
	AffectType        int
	AffectValue       int
	AffectTime        int
	Act               [8]byte
	ActAlt            [8]byte
	InstanceAttribute int
	TickAttribute     int
	Aggressive        int
	MaxTarget         int
	Party             int
	AffectResist      int
	Passive           int
	Name              string
}

type Catalog struct {
	Items       map[uint16]ItemDef
	Skills      map[int]SkillDef
	ItemEffects map[byte]string
}
