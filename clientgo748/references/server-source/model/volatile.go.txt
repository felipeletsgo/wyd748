package model

import "time"

// VolatileRule descreve uma funcao de item clicavel. Os itens associados a
// cada codigo continuam vindo do itemlist.csv; este tipo configura somente o
// comportamento autoritativo executado pelo servidor.
type VolatileRule struct {
	Action  string `json:"action"`
	Consume bool   `json:"consume"`
	// CustomPattern distingue o Premium FireCracker dos fogos comuns que
	// compartilham EF_VOLATILE 19. O primeiro usa o desenho 10x10 enviado pelo
	// protocolo 0x3C9; os demais continuam usando o Motion 100 aleatorio.
	CustomPattern bool   `json:"customPattern,omitempty"`
	ValueSource   string `json:"valueSource,omitempty"`
	HP            int    `json:"hp,omitempty"`
	MP            int    `json:"mp,omitempty"`
	Gold          uint32 `json:"gold,omitempty"`
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
	// OnceQuestID identifica uma trava permanente server-side para acoes de
	// quest de uso unico (por exemplo o Molar do Gargula). IDs negativos ficam
	// reservados para flags internas e nao colidem com quests configuradas.
	OnceQuestID int32 `json:"onceQuestId,omitempty"`
	// Refino de alvo equipado (Agua das Fadas): slot e indices aceitos tambem
	// ficam nos dados, para o executavel nao inventar equipamentos elegiveis.
	TargetSlot  int      `json:"targetSlot,omitempty"`
	TargetItems []uint16 `json:"targetItems,omitempty"`
	// FaceMesh e o mesh de rosto de monstro da acao "face_transform" (Gremlin=202,
	// Orc=209, Troll=212...). O rosto e sobrescrito no bodyMesh enquanto o affect
	// durar; face_restore o remove.
	FaceMesh int `json:"faceMesh,omitempty"`
	// Mount* configuram as acoes de montaria. MountAction diz o efeito (feed,
	// essence, longevity_restore, level_set, growth, hatch); Amount e a
	// magnitude/level-alvo. MountMinLevel permite descrever catalisadores sem
	// confundir o requisito da montaria com MinLevel, que pertence ao personagem.
	MountAction     string `json:"mountAction,omitempty"`
	MountMinLevel   int    `json:"mountMinLevel,omitempty"`
	Amount          int    `json:"amount,omitempty"`
	CooldownSeconds int    `json:"cooldownSeconds,omitempty"`
	// Color e o codigo de efeito de cor da acao "tint" (tintura): o slot de sanc
	// do item-alvo (EF_SANC=43) passa a usar este codigo 116-125, mantendo o valor
	// do refino; cada codigo pinta o item de uma cor no client. Ver pRemoveTintura.
	Color       int    `json:"color,omitempty"`
	Description string `json:"description,omitempty"`
	// Variant seleciona uma opcao dentro de familias nativas que compartilham
	// o mesmo motor (Gemas 0..3 e minérios 0..3). SuccessPercent permanece dado
	// de balanceamento, nunca confiado ao pacote do client.
	Variant        int `json:"variant,omitempty"`
	SuccessPercent int `json:"successPercent,omitempty"`
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
// coordenadas e duração permanecem em data/instances.json; o motor apenas
// aplica ocupação, party, spawn, timer e encerramento.
type VolatileInstance struct {
	ID string `json:"id"`
	// BaseRef permite declarar variantes (Cube Mystic/Arcane) sem duplicar a
	// tabela inteira. O loader materializa uma copia independente antes de
	// validar e executar a regra.
	BaseRef    string                  `json:"baseRef,omitempty"`
	Name       string                  `json:"name"`
	X          uint16                  `json:"x"`
	Y          uint16                  `json:"y"`
	SpawnX     uint16                  `json:"spawnX"`
	SpawnY     uint16                  `json:"spawnY"`
	AreaRadius int                     `json:"areaRadius"`
	MaxPlayers int                     `json:"maxPlayers,omitempty"`
	Spawns     []VolatileInstanceSpawn `json:"spawns"`
	RewardItem uint16                  `json:"rewardItem,omitempty"`
	// ChainNextItem is the item accepted during exit grace to open the next
	// independent instance. It is intentionally separate from RewardItem:
	// the final Water boss accepts the Room 1 ticket but does not grant it.
	ChainNextItem uint16 `json:"chainNextItem,omitempty"`
	// ChainStart marks the only ticket that may open a Water tier without a
	// completed predecessor. Keeping this edge in data prevents a player from
	// bypassing rooms merely by reaching a later platform on the map.
	ChainStart        bool     `json:"chainStart,omitempty"`
	AllowedEvolutions []string `json:"allowedEvolutions,omitempty"`
	DurationSeconds   int      `json:"durationSeconds"`
	// TotalDurationSeconds e um prazo absoluto da instancia, independente das
	// trocas de sala. Hell Gate usa os quatro minutos nativos desta forma.
	TotalDurationSeconds int `json:"totalDurationSeconds,omitempty"`
	// ActiveDurationSeconds is the event lifetime after a scheduled entry
	// window. Hell Gate admits players for four minutes and then remains active
	// through its native combat window; the two deadlines must not be conflated.
	ActiveDurationSeconds int    `json:"activeDurationSeconds,omitempty"`
	ExitX                 uint16 `json:"exitX"`
	ExitY                 uint16 `json:"exitY"`
	TransitionSeconds     int    `json:"transitionSeconds,omitempty"`
	// SharedEntry permite que usuarios consumam seus proprios ingressos para
	// entrar na primeira sala de uma execucao ja aberta. Cube usa esse fluxo:
	// ate seis jogadores entram individualmente antes da primeira resposta.
	SharedEntry bool `json:"sharedEntry,omitempty"`
	// Mode define a fronteira de execucao. Os valores usados pelo servidor sao
	// private_chain, private_shared_entry, shared_timed_zone e state_machine.
	// Vazio preserva o comportamento privado legado.
	Mode string `json:"mode,omitempty"`
	// SharedGroup e a chave runtime de uma zona fisica compartilhada. IDs de
	// regras diferentes (por exemplo ticket pessoal e de party) podem apontar
	// para a mesma execucao sem sobrescreverem outra sala.
	SharedGroup string `json:"sharedGroup,omitempty"`
	// ExclusiveGroup implementa locks logicos independentes da ocupacao fisica.
	// Magic Chamber usa um unico lock para Normal/Mystic/Arcane.
	ExclusiveGroup string `json:"exclusiveGroup,omitempty"`
	// FinishPolicy evita que o motor tenha de inferir o encerramento pela
	// quantidade de mobs: finish_on_clear, finish_on_timeout,
	// advance_on_clear, respawn_until_timeout ou state_machine.
	FinishPolicy string `json:"finishPolicy,omitempty"`
	// NoCombatTimeout is used by the native Cube: a room advances only after
	// its mobs are cleared and the O/X question is resolved.  It must not be
	// confused with an event-wide deadline or with a transition timeout.
	NoCombatTimeout bool `json:"noCombatTimeout,omitempty"`
	// StateMachine identifica uma progressao que nao pode ser descrita apenas
	// por salas lineares. Os controladores leem somente estes dados; nenhuma
	// coordenada de Hell Gate/Big Cube fica hardcoded no gameplay.
	StateMachine string            `json:"stateMachine,omitempty"`
	HellGate     *VolatileHellGate `json:"hellGate,omitempty"`
	// Uxmal descreve a Pista de Runas iniciada pelo NPC, nao por um item
	// volatile. O template continua dentro de instances.json para que salas,
	// tickets e recompensas sejam auditaveis sem recompilar o servidor.
	Uxmal *VolatileUxmal `json:"uxmal,omitempty"`
	// AllowChainDuringExitGrace explicita a janela Water de dez segundos. O
	// fallback historico por RewardItem continua somente para dados antigos.
	AllowChainDuringExitGrace bool `json:"allowChainDuringExitGrace,omitempty"`
	// EntryAreas restringe o TargetXY do 0x373 a uma ou mais areas nativas de
	// entrada. Lista vazia preserva instancias que podem ser abertas em
	// qualquer mapa (por exemplo, contratos de teste).
	EntryAreas []VolatileInstanceEntryArea `json:"entryAreas,omitempty"`
	// EntryAreaSet referencia uma tabela reutilizavel do arquivo de dados. O
	// loader expande a tabela para EntryAreas; assim as 27 cartas Water usam
	// exatamente as mesmas caixas nativas sem espalhar coordenadas duplicadas.
	EntryAreaSet string `json:"entryAreaSet,omitempty"`
	// Schedule descreve janelas repetidas dentro de cada hora. Quando existe,
	// o ticket so pode ser usado durante uma janela e a instancia termina no
	// fim dela, como Nightmare e Hell Gate no TMSrv.
	Schedule []VolatileInstanceWindow `json:"schedule,omitempty"`
	// PartyRunLimit limita quantas execucoes compartilhadas podem ser abertas
	// na mesma janela horaria. Nightmare nativo usa tres por tier; zero deixa
	// o template sem limite global.
	PartyRunLimit int `json:"partyRunLimit,omitempty"`
	// NightmareTier ativa as regras nativas de entrada do Pesadelo. Os valores
	// validos sao normal, mystic e arcane; vazio mantem o template generico.
	// A camada de jogo usa este campo para aplicar os bloqueios de nivel dos
	// Celestiais e debitar uma entrada NT por Celestial no Arcano.
	NightmareTier string                  `json:"nightmareTier,omitempty"`
	Stages        []VolatileInstanceStage `json:"stages,omitempty"`
}

// VolatileUxmal e a tabela autoritativa da Pista de Runas/Uxmal. RoomPositions
// usa uma lista por sala e tres posicoes por sala (o primeiro nivel nativo
// aceita apenas duas parties; MaxParties expressa essa excecao). Runes sao
// sorteadas por sala e TicketNextSanc e a sancao do proximo Clue of Runes.
type VolatileUxmal struct {
	NPC            string                      `json:"npc"`
	TicketItem     uint16                      `json:"ticketItem"`
	EntryAreas     []VolatileInstanceEntryArea `json:"entryAreas"`
	RoomPositions  [][]VolatileDestination     `json:"roomPositions"`
	MaxParties     []int                       `json:"maxParties"`
	Runes          [][]uint16                  `json:"runes"`
	TicketNextSanc []int                       `json:"ticketNextSanc"`
}

// InstanceRuntimeState is the restart-safe portion of an event instance.
// Entity IDs, player IDs and mob HP are deliberately excluded: they are
// process-local and are rebuilt from the authoritative data after a restart.
// Stable Character UIDs are retained for resumable event reattachment.
type InstanceRuntimeState struct {
	RuntimeID   string `json:"runtimeId"`
	ConfigID    string `json:"configId"`
	SharedGroup string `json:"sharedGroup,omitempty"`
	State       string `json:"state,omitempty"`
	// Character UIDs are the stable membership identity for private Water
	// rooms. Client/session IDs are process-local and must never be persisted.
	// Keeping the leader separately preserves reward ownership across a restart.
	MemberCharacterUIDs   []string  `json:"memberCharacterUids,omitempty"`
	LeaderCharacterUID    string    `json:"leaderCharacterUid,omitempty"`
	CurrentStage          int       `json:"currentStage"`
	ScheduleEnd           time.Time `json:"scheduleEnd,omitempty"`
	HardDeadline          time.Time `json:"hardDeadline,omitempty"`
	CombatDeadline        time.Time `json:"combatDeadline,omitempty"`
	TransitionAt          time.Time `json:"transitionAt,omitempty"`
	QuizAt                time.Time `json:"quizAt,omitempty"`
	ExitAt                time.Time `json:"exitAt,omitempty"`
	RewardGranted         bool      `json:"rewardGranted,omitempty"`
	HellGateVariant       int       `json:"hellGateVariant,omitempty"`
	HellGateClearedMask   uint8     `json:"hellGateClearedMask,omitempty"`
	HellGateLichSpawned   uint8     `json:"hellGateLichSpawnedMask,omitempty"`
	HellGateValidLichMask uint8     `json:"hellGateValidLichMask,omitempty"`
	HellGateWrongLich     bool      `json:"hellGateWrongLich,omitempty"`
}

// InstanceStateSnapshot is the single durable aggregate for shared event
// state. Keeping the Nightmare window counters beside the instances makes a
// restart unable to reset the three-party limit.
type InstanceStateSnapshot struct {
	Version            int                    `json:"version"`
	Instances          []InstanceRuntimeState `json:"instances,omitempty"`
	NightmarePartyRuns map[string]int         `json:"nightmarePartyRuns,omitempty"`
	GambleJackpot      uint32                 `json:"gambleJackpot,omitempty"`
	GamblePool         uint64                 `json:"gamblePool,omitempty"`
}

const InstanceStateVersion = 1

type VolatileInstanceEntryArea struct {
	MinX          uint16 `json:"minX"`
	MinY          uint16 `json:"minY"`
	MaxX          uint16 `json:"maxX"`
	MaxY          uint16 `json:"maxY"`
	RequiresChain bool   `json:"requiresChain,omitempty"`
}

type VolatileInstanceWindow struct {
	StartMinute     int `json:"startMinute"`
	DurationSeconds int `json:"durationSeconds"`
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
	// CompletionSpawns nasce na mesma sala depois que Spawns forem eliminados.
	// Magic Chamber usa esse campo para o chefe da quarta sala sem criar uma
	// quinta sala nem renovar o prazo de combate.
	CompletionSpawns []VolatileInstanceSpawn `json:"completionSpawns,omitempty"`
	Quiz             *VolatileInstanceQuiz   `json:"quiz,omitempty"`
}

// VolatileHellGate descreve a maquina de estados do Porto Infernal. O grupo
// inicial e a Tarantula controladora; depois dela os quatro quadrantes nascem
// juntos e somente o par de Lichs sorteado abre a fase final.
type VolatileHellGate struct {
	ControllerNPC string                     `json:"controllerNPC"`
	Quadrants     []VolatileHellGateQuadrant `json:"quadrants"`
	FinalSpawns   []VolatileInstanceSpawn    `json:"finalSpawns,omitempty"`
	FinalNPCs     []VolatileInstanceSpawn    `json:"finalNPCs,omitempty"`
}

type VolatileHellGateQuadrant struct {
	ID         int                     `json:"id"`
	X          uint16                  `json:"x"`
	Y          uint16                  `json:"y"`
	SpawnX     uint16                  `json:"spawnX"`
	SpawnY     uint16                  `json:"spawnY"`
	AreaRadius int                     `json:"areaRadius"`
	Spawns     []VolatileInstanceSpawn `json:"spawns"`
	Lich       VolatileInstanceSpawn   `json:"lich"`
}

// VolatileInstanceQuiz reproduz as salas O/X do Cube. Depois de eliminar a
// sala, cada membro escolhe fisicamente O ou X; acertos seguem e recebem EXP,
// erros saem da instancia. TrueX/TrueY representam a plataforma O (Sim) e
// FalseX/FalseY a plataforma X (Nao); os nomes indicam a verdade da afirmacao,
// nao o desenho da plataforma.
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
	Default VolatileRule
	Rules   map[int]VolatileRule
	Items   map[uint16]VolatileRule
	// Instances preserva os templates nomeados do arquivo de dados mesmo
	// quando nenhum item volatile aponta para eles. Eventos iniciados por NPC
	// (como a Pista de Runas/Uxmal) usam o mesmo catalogo autoritativo das
	// instancias iniciadas por consumiveis.
	Instances  map[string]VolatileInstance
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
