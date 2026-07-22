package model

// VolatileRule descreve uma funcao de item clicavel. Os itens associados a
// cada codigo continuam vindo do itemlist.csv; este tipo configura somente o
// comportamento autoritativo executado pelo servidor.
type VolatileRule struct {
	Action       string                `json:"action"`
	Consume      bool                  `json:"consume"`
	ValueSource  string                `json:"valueSource,omitempty"`
	HP           int                   `json:"hp,omitempty"`
	MP           int                   `json:"mp,omitempty"`
	Gold         uint32                `json:"gold,omitempty"`
	X            uint16                `json:"x,omitempty"`
	Y            uint16                `json:"y,omitempty"`
	Destinations []VolatileDestination `json:"destinations,omitempty"`
	LearnedBit   int                   `json:"learnedBit,omitempty"`
	Summon       *VolatileSummon       `json:"summon,omitempty"`
	// Parametros de affect para a acao "buff" (pocoes, comidas). AffectType
	// escolhe a formula server-side em applyExtendedAffectStats; AffectValue e
	// AffectLevel a alimentam; DurationUnits e o tempo em blocos de 8 s. Sao
	// tunaveis puramente por dado: nenhum numero de balanceamento vive no codigo.
	AffectType    int `json:"affectType,omitempty"`
	AffectValue   int `json:"affectValue,omitempty"`
	AffectLevel   int `json:"affectLevel,omitempty"`
	DurationUnits int `json:"durationUnits,omitempty"`
	// Accumulate: em vez de renovar, SOMA DurationUnits ao tempo restante do
	// affect ate MaxDurationUnits (bau de EXP, comidas). Sem teto o uso e recusado
	// quando saturado. MaxDurationUnits e obrigatorio quando Accumulate.
	Accumulate       bool `json:"accumulate,omitempty"`
	MaxDurationUnits int  `json:"maxDurationUnits,omitempty"`
	// Exp e a quantidade concedida pela acao "grant_exp" (baus/poeiras de XP).
	Exp uint32 `json:"exp,omitempty"`
	// RefineMax e o teto de refino da acao "refine": Ori=6, Lac=9. O servidor
	// recusa refinar um item cujo sanc ja alcancou este teto.
	RefineMax int `json:"refineMax,omitempty"`
	// FaceMesh e o mesh de rosto de monstro da acao "face_transform" (Gremlin=202,
	// Orc=209, Troll=212...). O rosto e sobrescrito no bodyMesh enquanto o affect
	// durar; face_restore o remove.
	FaceMesh int `json:"faceMesh,omitempty"`
	// Mount* configuram as acoes de montaria. MountAction diz o efeito (feed_hp,
	// level, invuln, growth, hatch); Amount e a magnitude (vida/level/dias), por
	// item quando o csv nao carrega o valor (rações/amagos diferem por Index).
	MountAction string `json:"mountAction,omitempty"`
	Amount      int    `json:"amount,omitempty"`
	// Color e o codigo de efeito de cor da acao "tint" (tintura): o slot de sanc
	// do item-alvo (EF_SANC=43) passa a usar este codigo 116-125, mantendo o valor
	// do refino; cada codigo pinta o item de uma cor no client. Ver pRemoveTintura.
	Color int `json:"color,omitempty"`
	// AddPool e o conjunto de "adicionais" sorteaveis da acao "repliction" (Grade
	// A-E): o servidor sorteia UMA entrada e a grava no 2o slot de efeito do manto
	// equipado. Grades melhores tem pools com efeitos/valores maiores. Balanceavel
	// no json sem recompilar (como os buffs). Porta o conceito da composicao de
	// capa do W2PP (_MSG_UseItem: tabela random por sIndex de capa) sem copiar as
	// tabelas fixas daquela versao, que sao presas aos indices de item de la.
	AddPool     []VolatileAdd `json:"addPool,omitempty"`
	Description string        `json:"description,omitempty"`
}

type VolatileDestination struct {
	X uint16 `json:"x"`
	Y uint16 `json:"y"`
}

// VolatileSummon mantem contratos inteiramente configuraveis. Group distingue
// as duas familias/reinos sem antecipar as regras futuras de Castle War.
type VolatileSummon struct {
	Group       string `json:"group"`
	Name        string `json:"name"`
	Face        uint16 `json:"face"`
	Weapon      uint16 `json:"weapon,omitempty"`
	Shield      uint16 `json:"shield,omitempty"`
	Attack      uint32 `json:"attack"`
	Defense     uint32 `json:"defense"`
	HP          uint32 `json:"hp"`
	MoveSpeed   uint8  `json:"moveSpeed"`
	AttackRange int    `json:"attackRange"`
}

// VolatileAdd e um efeito adicional candidato da acao "repliction": Effect e o
// codigo de efeito (EF_*), Value a magnitude. Weight (opcional, default 1) pesa
// o sorteio -- entradas raras/fortes recebem peso menor.
type VolatileAdd struct {
	Effect int `json:"effect"`
	Value  int `json:"value"`
	Weight int `json:"weight,omitempty"`
}

// VolatileCatalog une as regras configuradas aos itens efetivamente
// encontrados no itemlist. ItemCodes contem TODOS os itens com EF_VOLATILE,
// inclusive os que usam apenas o handler generic.
type VolatileCatalog struct {
	Default   VolatileRule
	Rules     map[int]VolatileRule
	Items     map[uint16]VolatileRule
	ItemCodes map[uint16]int
	Codes     map[int]int
}

func (c VolatileCatalog) Rule(itemID uint16) (VolatileRule, int, bool) {
	code, ok := c.ItemCodes[itemID]
	if !ok {
		return VolatileRule{}, 0, false
	}
	if rule, exists := c.Items[itemID]; exists {
		return rule, code, true
	}
	if rule, exists := c.Rules[code]; exists {
		return rule, code, true
	}
	return c.Default, code, true
}
