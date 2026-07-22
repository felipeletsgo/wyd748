package model

// StaticEffect e um par nome/valor do itemlist.csv. O nome EF_* e mantido para
// que o servidor possa portar as formulas nativas sem depender do ID usado pelo
// binario do cliente.
type StaticEffect struct {
	Name  string
	Value int
}

// ItemDef e a fonte autoritativa de um item. Aparencia, requisitos, preco,
// posicao e efeitos vem exclusivamente do itemlist/Itemname carregado no boot.
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
}

// SkillDef preserva todas as colunas conhecidas do SkillData.csv. O catalogo
// ja e autoritativo mesmo antes dos handlers de aprender/executar skills.
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
	Unknown           int
	Name              string
}

type Catalog struct {
	Items  map[uint16]ItemDef
	Skills map[int]SkillDef
}
