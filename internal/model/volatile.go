package model

// VolatileRule descreve uma funcao de item clicavel. Os itens associados a
// cada codigo continuam vindo do itemlist.csv; este tipo configura somente o
// comportamento autoritativo executado pelo servidor.
type VolatileRule struct {
	Action      string `json:"action"`
	Consume     bool   `json:"consume"`
	ValueSource string `json:"valueSource,omitempty"`
	HP          int    `json:"hp,omitempty"`
	MP          int    `json:"mp,omitempty"`
	Gold        uint32 `json:"gold,omitempty"`
	// As recompensas de quest usam maximo EXCLUSIVO, como o Vol 191 nativo:
	// Level < min || Level >= max. Isso e deliberadamente diferente do
	// MaxLevel inclusivo de QuestRequirements.
	MortalOnly        bool                  `json:"mortalOnly,omitempty"`
	RequiredEvolution string                `json:"requiredEvolution,omitempty"`
	MinLevel          uint32                `json:"minLevel,omitempty"`
	MaxLevelExclusive uint32                `json:"maxLevelExclusive,omitempty"`
	X                 uint16                `json:"x,omitempty"`
	Y                 uint16                `json:"y,omitempty"`
	Destinations      []VolatileDestination `json:"destinations,omitempty"`
	LearnedBit        int                   `json:"learnedBit,omitempty"`
	Summon            *VolatileSummon       `json:"summon,omitempty"`
	Instance          *VolatileInstance     `json:"instance,omitempty"`
	InstanceRef       string                `json:"instanceRef,omitempty"`
	// Parametros de affect para a acao "buff" (pocoes, comidas). AffectType
	// escolhe a formula server-side em applyExtendedAffectStats; AffectValue e
	// AffectLevel a alimentam; DurationUnits e o tempo em blocos de 8 s. Sao
	// tunaveis puramente por dado: nenhum numero de balanceamento vive no codigo.
	AffectType    int `json:"affectType,omitempty"`
	AffectValue   int `json:"affectValue,omitempty"`
	AffectLevel   int `json:"affectLevel,omitempty"`
	DurationUnits int `json:"durationUnits,omitempty"`
	// Affects permite que um unico consumivel aplique o pacote nativo completo
	// em uma unica transacao. Os campos singulares acima continuam sendo a forma
	// compacta para os itens que possuem apenas um affect.
	Affects []VolatileAffect `json:"affects,omitempty"`
	// Accumulate: em vez de renovar, SOMA DurationUnits ao tempo restante do
	// affect ate MaxDurationUnits (bau de EXP, comidas). Sem teto o uso e recusado
	// quando saturado. MaxDurationUnits e obrigatorio quando Accumulate.
	Accumulate       bool `json:"accumulate,omitempty"`
	MaxDurationUnits int  `json:"maxDurationUnits,omitempty"`
	// Exp e a quantidade concedida pela acao "grant_exp" (baus/poeiras de XP).
	Exp uint32 `json:"exp,omitempty"`
	// Counters credita contadores nomeados na acao "grant_counter". No nativo o
	// item 4127 concede 100 entradas de Kefra de uma vez.
	Counters map[string]uint32 `json:"counters,omitempty"`
	// RewardItems e a lista equiprovavel da acao "loot_box". Cada recompensa e
	// materializada com UID; o item consumido e a recompensa entram no mesmo
	// commit antes da confirmacao ao client.
	RewardItems []uint16 `json:"rewardItems,omitempty"`
	// AccessKey/DurationSeconds configuram licencas temporizadas (VIP, Ruinas
	// Escondidas e passes de evento). O vencimento absoluto e persistido no
	// charstate do personagem, sem criar mais uma estrutura paralela.
	AccessKey       string `json:"accessKey,omitempty"`
	DurationSeconds int    `json:"durationSeconds,omitempty"`
	// PartyMode controla tickets: "party" exige um grupo e seu lider; "solo"
	// move apenas o usuario. Vazio preserva o modo legado, que tambem aceita
	// usar a instancia sem grupo.
	PartyMode string `json:"partyMode,omitempty"`
	// RefineMax e o teto de refino da acao "refine": Ori=6, Lac=9. O servidor
	// recusa refinar um item cujo sanc ja alcancou este teto.
	RefineMax int `json:"refineMax,omitempty"`
	// Refino de alvo equipado (Agua das Fadas): slot e indices aceitos tambem
	// ficam nos dados, para o executavel nao inventar equipamentos elegiveis.
	TargetSlot  int      `json:"targetSlot,omitempty"`
	TargetItems []uint16 `json:"targetItems,omitempty"`
	// FaceMesh e o mesh de rosto de monstro da acao "face_transform" (Gremlin=202,
	// Orc=209, Troll=212...). O rosto e sobrescrito no bodyMesh enquanto o affect
	// durar; face_restore o remove.
	FaceMesh int `json:"faceMesh,omitempty"`
	// Mount* configuram as acoes de montaria. MountAction diz o efeito (feed_hp,
	// level, invuln, growth, hatch); Amount e a magnitude (vida/level/dias), por
	// item quando o csv nao carrega o valor (rações/amagos diferem por Index).
	MountAction     string `json:"mountAction,omitempty"`
	Amount          int    `json:"amount,omitempty"`
	CooldownSeconds int    `json:"cooldownSeconds,omitempty"`
	// Color e o codigo de efeito de cor da acao "tint" (tintura): o slot de sanc
	// do item-alvo (EF_SANC=43) passa a usar este codigo 116-125, mantendo o valor
	// do refino; cada codigo pinta o item de uma cor no client. Ver pRemoveTintura.
	Color       int    `json:"color,omitempty"`
	Description string `json:"description,omitempty"`
}

type VolatileAffect struct {
	// SkillID referencia uma entrada autoritativa de SkillData.csv. O SetAffect
	// nativo recebe o indice da skill e deriva dela AffectType/AffectValue; ele
	// nao usa o indice da skill como tipo de affect.
	SkillID       int `json:"skillId,omitempty"`
	Type          int `json:"type"`
	Value         int `json:"value,omitempty"`
	Level         int `json:"level,omitempty"`
	DurationUnits int `json:"durationUnits"`
}

// VolatileInstance descreve uma sala privada ativada por ticket. Conteúdo,
// coordenadas e duração permanecem em data/volatiles.json; o motor apenas
// aplica ocupação, party, spawn, timer e encerramento.
type VolatileInstance struct {
	ID                string                  `json:"id"`
	Name              string                  `json:"name"`
	X                 uint16                  `json:"x"`
	Y                 uint16                  `json:"y"`
	SpawnX            uint16                  `json:"spawnX"`
	SpawnY            uint16                  `json:"spawnY"`
	AreaRadius        int                     `json:"areaRadius"`
	MaxPlayers        int                     `json:"maxPlayers,omitempty"`
	Spawns            []VolatileInstanceSpawn `json:"spawns"`
	RewardItem        uint16                  `json:"rewardItem,omitempty"`
	AllowedEvolutions []string                `json:"allowedEvolutions,omitempty"`
	DurationSeconds   int                     `json:"durationSeconds"`
	// TotalDurationSeconds e um prazo absoluto da instancia, independente das
	// trocas de sala. Hell Gate usa os quatro minutos nativos desta forma.
	TotalDurationSeconds int    `json:"totalDurationSeconds,omitempty"`
	ExitX                uint16 `json:"exitX"`
	ExitY                uint16 `json:"exitY"`
	TransitionSeconds    int    `json:"transitionSeconds,omitempty"`
	// SharedEntry permite que usuarios consumam seus proprios ingressos para
	// entrar na primeira sala de uma execucao ja aberta. Cube usa esse fluxo:
	// ate seis jogadores entram individualmente antes da primeira resposta.
	SharedEntry bool                    `json:"sharedEntry,omitempty"`
	Stages      []VolatileInstanceStage `json:"stages,omitempty"`
}

// VolatileInstanceSpawn permite que uma mesma sala tenha mais de um template.
// As cartas de Nessus usam um chefe e dez auxiliares; salas comuns usam apenas
// uma entrada. O total autoritativo da sala e a soma dos Counts.
type VolatileInstanceSpawn struct {
	NPC   string `json:"npc"`
	Count int    `json:"count"`
	X     uint16 `json:"x,omitempty"`
	Y     uint16 `json:"y,omitempty"`
}

// VolatileInstanceStage permite que uma carta conduza o grupo por varias
// salas/ondas. Water continua usando a forma compacta de uma sala; Magic
// Chamber usa quatro salas e um boss sem hardcode no executavel.
type VolatileInstanceStage struct {
	Name            string                  `json:"name,omitempty"`
	X               uint16                  `json:"x"`
	Y               uint16                  `json:"y"`
	SpawnX          uint16                  `json:"spawnX"`
	SpawnY          uint16                  `json:"spawnY"`
	AreaRadius      int                     `json:"areaRadius"`
	DurationSeconds int                     `json:"durationSeconds,omitempty"`
	Spawns          []VolatileInstanceSpawn `json:"spawns"`
	Quiz            *VolatileInstanceQuiz   `json:"quiz,omitempty"`
}

// VolatileInstanceQuiz reproduz as salas O/X do Cube. Depois de eliminar a
// sala, cada membro escolhe fisicamente O ou X; acertos seguem e recebem EXP,
// erros saem da instancia.
type VolatileInstanceQuiz struct {
	Question        string `json:"question"`
	Answer          bool   `json:"answer"`
	TrueX           uint16 `json:"trueX"`
	TrueY           uint16 `json:"trueY"`
	FalseX          uint16 `json:"falseX"`
	FalseY          uint16 `json:"falseY"`
	DurationSeconds int    `json:"durationSeconds"`
	RewardExp       uint32 `json:"rewardExp,omitempty"`
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

// ReplictionItem relaciona cada consumivel A-E ao nivel do equipamento e ao
// limite de refino aceito. Normal e Premium permanecem dados, nao inferencias.
type ReplictionItem struct {
	ItemLevel int `json:"itemLevel"`
	MaxSanc   int `json:"maxSanc"`
}

// ReplictionBonus e o par indivisivel sorteado pelo SetItemBonus2 nativo.
// Aplicar Repliction sempre substitui os dois adicionais do item.
type ReplictionBonus struct {
	Effect1 int `json:"effect1"`
	Value1  int `json:"value1"`
	Effect2 int `json:"effect2"`
	Value2  int `json:"value2"`
}

type ReplictionCatalog struct {
	Items map[uint16]ReplictionItem
	Pools map[int][]ReplictionBonus
}

// VolatileCatalog une as regras configuradas aos itens efetivamente
// encontrados no itemlist. ItemCodes contem TODOS os itens com EF_VOLATILE,
// inclusive os que pertencem a NPC/comando ou aguardam o sistema Celestial.
type VolatileCatalog struct {
	Default    VolatileRule
	Rules      map[int]VolatileRule
	Items      map[uint16]VolatileRule
	ItemCodes  map[uint16]int
	Codes      map[int]int
	Repliction ReplictionCatalog
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
