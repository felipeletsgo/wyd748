// Package game e o nucleo: um World e dono EXCLUSIVO do estado e roda numa unica
// goroutine (modelo de ator, zero mutex). Comandos das sessoes e ticks do jogo
// sao processados LINEARMENTE pelo loop.
package game

import (
	"fmt"
	"log"
	"math/rand"
	"runtime/debug"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// command = um pacote recebido de uma sessao (pkt nil = desconexao).
type command struct {
	s     *net.Session
	pkt   []byte
	login *loginResult
	// shutdown, quando presente, pede o desligamento controlado. E um comando
	// como qualquer outro justamente para rodar NA goroutine do World: assim o
	// drain final enxerga o estado consistente, sem concorrer com um handler.
	shutdown chan struct{}
}

type loginResult struct {
	accountName string
	account     *model.Account
	err         error
}

const (
	playerEntryX = uint16(2100)
	playerEntryY = uint16(2100)
)

func pinAccountEntryPositions(account *model.Account) {
	if account == nil {
		return
	}
	for i := range account.Chars {
		if account.Chars[i].Name != "" {
			account.Chars[i].X = playerEntryX
			account.Chars[i].Y = playerEntryY
		}
	}
}

// saveAccount e a unica fronteira de persistencia do mundo. Posicao atual e
// estado de sessao: todos os saves gravam somente o ponto fixo de reentrada.
func (w *World) saveAccount(account *model.Account) error {
	pinAccountEntryPositions(account)
	return w.store.SaveAccount(account)
}

// asyncSaveStore expoe os saves assincronos usados pelo autosave. Store sem
// suporte cai no save sincrono -- o autosave continua funcionando, so nao tira o
// fsync do game-loop.
type asyncSaveStore interface {
	SaveAccountAsync(acc *model.Account) error
}

// saveAccountAsync grava a conta FORA do game-loop quando o store suporta. Usado
// so pelo autosave: os saves anti-dupe continuam sincronos via saveAccount.
func (w *World) saveAccountAsync(account *model.Account) error {
	pinAccountEntryPositions(account)
	if as, ok := w.store.(asyncSaveStore); ok {
		return as.SaveAccountAsync(account)
	}
	return w.store.SaveAccount(account)
}

// Player = jogador em RAM: sessao + conta + char selecionado + id de mundo.
type Player struct {
	Session  *net.Session
	Account  *model.Account
	Char     *model.Char
	CharSlot int
	ID       uint16
	InWorld  bool
	X, Y     uint16 // posicao atual (rastreada dos pacotes de movimento 0x366)
	// NPC cuja loja esta aberta. O buy do 7.48 vem com TargetID=0, portanto o
	// servidor usa este ID autoritativo em vez de confiar no pacote.
	ShopNPC       uint16
	CraftNPC      uint16
	LastCraft     time.Time
	DeadAt        time.Time
	LastPotion    time.Time
	SkillReady    map[int]time.Time
	NextRegen     time.Time
	NextMountTick time.Time
	// Tickets de dungeon usam um prazo server-side unico; ao vencer, o jogador
	// e retirado pelo fluxo normal de teleporte, sem ticker por instancia.
	DungeonExitAt time.Time
	DungeonExitX  uint16
	DungeonExitY  uint16
	// Cooldown compartilhado pelos comandos /kingdom e /king.
	NextKingdomTeleport time.Time
	Visible             map[uint16]struct{} // entidades atualmente materializadas neste client
	Party               *Party
	InviteFrom          uint16
	InviteUntil         time.Time
	// Convite de guild pendente. Mesmo padrao do convite de grupo: guarda quem
	// convidou e ate quando vale, para nao aceitar convite esquecido.
	GuildInviteFrom  uint16
	GuildInviteUntil time.Time
	// Alvos autoritativos usados pelas evocacoes: primeiro quem o dono atacou;
	// na ausencia dele, quem atacou o dono.
	CombatTargetID uint16
	LastAttackerID uint16
	LastAttackAt   time.Time
	LastAttackTick uint32
	AttackProgress uint16
	// O client 7.48 envia Action continuamente durante uma caminhada. Estes
	// campos guardam o ultimo destino ja anunciado aos observadores, para que
	// uma atualizacao intermediaria nao reinicie a interpolacao remota.
	MovePublished        bool
	MovePublishedTargetX uint16
	MovePublishedTargetY uint16
	// Orcamento server-side de deslocamento. Impede que rotas individualmente
	// validas sejam reenviadas em alta frequencia para produzir speedhack.
	MoveBudget   float64
	MoveBudgetAt time.Time
	Trade        *TradeState
	GhostShop    *GhostShop
	// Loja fantasma cuja janela este client abriu. Permite fechar somente os
	// compradores afetados quando o clone desaparece.
	BrowsingGhostShopID uint16
	PKMode              bool
	// SpecialCoins sao contadores nomeados, persistidos pelo UID do personagem
	// no sidecar de estado de sessao junto com os buffs.
	SpecialCoins map[string]uint32
}

// Party e estado exclusivo do mundo e nunca e persistido na conta. Members
// sempre inclui o lider na posicao zero, seguido pela ordem de entrada.
type Party struct {
	Members []*Player
}

// Mob = NPC/monstro vivo no mundo (instancia de uma NPCDef).
type Mob struct {
	ID               uint16
	Def              *model.NPCDef
	X, Y             uint16
	HP               uint32
	Dead             bool
	GenerIndex       int
	LeaderID         uint16
	Segments         [model.MaxGenerSegments]model.GenerSegment
	RouteType        int
	SegmentProgress  int
	SegmentDirection int
	WaitUntil        time.Time
	NextMove         time.Time
	TargetID         uint16
	NextAttack       time.Time
	Affects          [16]model.Affect
	AffectOwners     [16]uint16
	SummonerID       uint16
	SummonKind       byte
	SummonRange      int
	ExpiresAt        time.Time
	Awake            bool
	InstanceID       string
}

const (
	summonKindBM byte = iota + 1
	summonKindContract
	summonKindMount
	summonKindThornWall
)

type generState struct {
	def          model.NPCGener
	leader       *model.NPCDef
	follower     *model.NPCDef
	current      int
	nextGenerate time.Time
}

type GroundItem struct {
	ID     uint16
	Item   model.Item
	X, Y   uint16
	Expire time.Time
	// Rotate e a orientacao do objeto no mapa. Drop comum usa 0; objeto
	// permanente carrega a rotacao do data/init_items.csv.
	Rotate byte
	// Permanent marca objeto de mundo (portao, porta, canhao, torre): nao
	// expira e nao pode ser recolhido. O nativo consegue o mesmo efeito
	// mantendo-os abaixo de g_dwInitItem, faixa que o decay nunca varre.
	Permanent bool
	// State e o estado do objeto: 0 fechado, 1 aberto. So porta usa. O nativo
	// troca isso e emite MSG_UpdateItem em vez de recriar o item.
	State byte
}

// O TMSrv chama o contador "MinuteGenerate", mas seu TIMER_MIN roda a cada
// 12000 ms. Manter esse tick conserva os valores dos NPCGener.txt existentes.
const npcGenerMinute = 12 * time.Second
const accountAutoSaveInterval = 3 * time.Second

// questZoneResetInterval porta o reset de area de quest do W2PP: la e um
// SecCounter%1200 com TIMER_SEC=500ms, ou seja 10 minutos reais.
const questZoneResetInterval = 10 * time.Minute
const npcGenerSummaryInterval = time.Minute

type npcGenerLogMode byte

const (
	npcGenerLogQuiet npcGenerLogMode = iota
	npcGenerLogSummary
	npcGenerLogVerbose
)

type npcGenerLogStats struct {
	groups      int
	mobs        int
	relocations int
}

type WorldOption func(*World)

// WithNPCGenerLog seleciona quiet, summary ou verbose. Configuracao invalida
// cai em summary; LoadServerConfig normalmente a rejeita antes daqui.
// WithQuests entrega o quests.json ja parseado. O cruzamento com os NPCs
// (existe? nao e tipo reservado?) acontece no NewWorld, que e onde os dois
// lados estao disponiveis -- e falha o boot em vez de ignorar a configuracao.
func WithQuests(file model.QuestFile) WorldOption {
	return func(w *World) { w.questFile = file }
}

func WithQuestZones(file model.QuestZoneFile) WorldOption {
	return func(w *World) { w.questZones = file.Zones }
}

// WithInitItems entrega os objetos permanentes do mundo (portoes, portas,
// canhoes, torres). Eles entram no mapa no boot e nunca saem.
func WithInitItems(objetos []model.InitItem) WorldOption {
	return func(w *World) { w.initItems = objetos }
}

func WithNPCGenerLog(mode string) WorldOption {
	return func(w *World) {
		switch strings.ToLower(strings.TrimSpace(mode)) {
		case "quiet":
			w.npcGenerLogMode = npcGenerLogQuiet
		case "verbose":
			w.npcGenerLogMode = npcGenerLogVerbose
		default:
			w.npcGenerLogMode = npcGenerLogSummary
		}
	}
}

func WithTeleports(teleports []model.Teleport) WorldOption {
	return func(w *World) {
		w.teleports = append([]model.Teleport(nil), teleports...)
	}
}

func WithGameplayConfig(config model.GameplayConfig) WorldOption {
	return func(w *World) {
		w.gameplay = config
	}
}

// WithMounts injeta o catalogo de montarias (bonus de stat por tipo). Ausente,
// o slot de montaria nao adiciona atributos.
func WithMounts(catalog model.MountCatalog) WorldOption {
	return func(w *World) {
		w.mounts = catalog
	}
}

// World e o dono do estado do jogo.
type World struct {
	commands        chan command
	players         map[*net.Session]*Player
	playersByID     map[uint16]*Player
	accountSessions map[string]*net.Session
	authPending     map[*net.Session]bool
	authSlots       chan struct{}
	// security agrega violacoes por conexao em qualquer fase (inclusive antes
	// do login), sem confiar em campos de identidade enviados no pacote.
	security map[*net.Session]*securityState
	// charNames e o indice em memoria de nomes de personagem (minusculos),
	// populado no boot e mantido em criar/deletar. Evita varrer todas as contas do
	// disco a cada 0x20F (DoS). nil = indice indisponivel -> cai no scan do store.
	charNames       map[string]struct{}
	store           store.Store
	npcs            []model.NPCDef
	mobs            []*Mob
	mobsByID        map[uint16]*Mob
	mobCells        map[uint32]map[uint16]*Mob
	playerCells     map[uint32]map[uint16]*Player
	mobCell         map[uint16]uint32
	playerCell      map[uint16]uint32
	activeMobs      map[uint16]*Mob
	summons         map[uint16]*Mob
	sephiraObjects  map[uint16]*Mob
	generators      []generState
	nextMobID       uint16
	items           map[uint16]model.ItemDef
	skills          map[int]model.SkillDef
	groundItems     map[uint16]*GroundItem
	ghostShops      map[uint16]*GhostShop
	nextItemID      uint16
	nextAutoSave    time.Time
	dropRates       [model.MaxCarry]int // taxa de drop por slot do carry (nativa)
	volatiles       model.VolatileCatalog
	mounts          model.MountCatalog
	charSpawn       model.CharacterSpawn
	charTemplates   [4]model.CharacterTemplate
	terrain         model.TerrainMap
	npcGenerLogMode npcGenerLogMode
	npcGenerLog     npcGenerLogStats
	nextGenerLog    time.Time
	teleports       []model.Teleport
	gameplay        model.GameplayConfig
	// guilds e o registro canonico carregado do guilds.json. Char.GuildID e
	// apenas uma copia desnormalizada reparada no login.
	guilds *model.GuildRegistry
	// questsByNPC e a allowlist de quest: NPC ausente daqui nunca vira quest.
	questFile   model.QuestFile
	initItems   []model.InitItem
	questsByNPC map[string]*model.QuestDef
	// questZones sao retangulos que expulsam todo jogador para a cidade a cada
	// ciclo de reset (mecanismo ClearArea do W2PP). nextQuestZoneReset e o
	// deadline do proximo reset -- baseado em relogio, nao no contador de tick,
	// para dar 10 minutos reais mesmo se algum tick atrasar.
	questZones         []model.QuestZone
	nextQuestZoneReset time.Time
	// channel e o numero deste canal (o ServerIndex+1 do nativo). A cidadania
	// e por canal: ser cidadao de outro canal nao rende o bonus aqui.
	// Instancia unica = canal 1.
	channel byte
	// Evita que pacotes informativos ainda nao materializados contaminem o log
	// em clientes que os enviam a cada frame.
	lastProtocolNotice map[uint16]time.Time
	mobTickCounter     uint64
	// clock e rng sao as fontes de tempo/aleatoriedade (clock.go). Em producao
	// sao o relogio e o RNG reais; os testes injetam versoes controladas para
	// exercitar deadline e sorteio sem time.Sleep.
	clock Clock
	rng   RNG
	// shuttingDown fica true depois do desligamento controlado (shutdown.go):
	// o estado ja foi persistido, entao nenhuma entrada nova e aceita.
	shuttingDown bool
	// bosses guarda o comportamento extra dos bosses, indexado pelo ID do mob
	// (boss.go). Todo boss TAMBEM esta em mobs/mobsByID e se comporta como um
	// mob comum em grid, visibilidade e combate. Mapa vazio = custo zero para
	// quem nao usa boss.
	bosses map[uint16]*BossRuntime
	// bossCatalog e a configuracao lida de data/boss/*.lua; bossSpawns guarda o
	// estado vivo de cada encontro (instancia atual e deadline de respawn).
	bossCatalog model.BossCatalog
	bossSpawns  []*bossSpawnState
	// itemInstances são salas temporárias criadas por consumíveis. Um mapa no
	// ator World substitui tickers/goroutines por sala.
	itemInstances map[string]*ItemInstance
}

// firstMobID e o inicio da faixa de mobs; abaixo dela ficam os jogadores.
const firstMobID = uint16(1000)

// allocMobID reserva o proximo ID de mob LIVRE.
//
// Dois cuidados, ambos vindos de bugs reais:
//
//  1. O clamp vem ANTES de reservar. Com ele depois, um World de contador
//     zerado devolvia 0 (que significa "sem ID") e em seguida 1000.
//  2. O contador DA A VOLTA em 65535. Como cada respawn consome um ID novo, um
//     servidor de longa duracao inevitavelmente retorna ao inicio da faixa; sem
//     verificar ocupacao, o ID de um mob VIVO seria reusado e a entrada dele em
//     mobsByID, sobrescrita -- corrompendo alvo, visibilidade e affects em
//     silencio. mobsByID e o registro dos mobs vivos (removeMobInstance limpa),
//     entao basta pular o que ja esta la.
func (w *World) allocMobID() uint16 {
	if w.nextMobID < firstMobID {
		w.nextMobID = firstMobID
	}
	// No maximo uma volta completa na faixa.
	for attempts := 0; attempts <= int(^uint16(0)-firstMobID); attempts++ {
		id := w.nextMobID
		if w.nextMobID == ^uint16(0) {
			w.nextMobID = firstMobID
		} else {
			w.nextMobID++
		}
		if _, used := w.mobsByID[id]; !used {
			return id
		}
	}
	// Inalcancavel na pratica: exigiria mais de 64 mil mobs vivos ao mesmo
	// tempo. Registrado alto porque o retorno 0 produziria um mob invalido.
	log.Printf("ERRO: faixa de IDs de mob esgotada (%d mobs vivos)", len(w.mobsByID))
	return 0
}

// NewWorld cria o mundo, materializando os NPCs estaticos em mobs vivos.
// ClientId dos NPCs comeca em 1000 (players usam ID baixo a partir de 1).
func NewWorld(st store.Store, npcs []model.NPCDef, geners []model.NPCGener, catalog model.Catalog, dropRates [model.MaxCarry]int, volatiles model.VolatileCatalog, characterTemplates model.CharacterTemplateFile, terrain model.TerrainMap, options ...WorldOption) (*World, error) {
	w := &World{
		commands:           make(chan command, 1024),
		players:            make(map[*net.Session]*Player),
		playersByID:        make(map[uint16]*Player),
		accountSessions:    make(map[string]*net.Session),
		authPending:        make(map[*net.Session]bool),
		authSlots:          make(chan struct{}, 4),
		security:           make(map[*net.Session]*securityState),
		store:              st,
		npcs:               npcs,
		mobsByID:           make(map[uint16]*Mob),
		mobCells:           make(map[uint32]map[uint16]*Mob),
		playerCells:        make(map[uint32]map[uint16]*Player),
		mobCell:            make(map[uint16]uint32),
		playerCell:         make(map[uint16]uint32),
		activeMobs:         make(map[uint16]*Mob),
		summons:            make(map[uint16]*Mob),
		sephiraObjects:     make(map[uint16]*Mob),
		nextMobID:          1000,
		items:              catalog.Items,
		skills:             catalog.Skills,
		groundItems:        make(map[uint16]*GroundItem),
		ghostShops:         make(map[uint16]*GhostShop),
		nextItemID:         10000,
		dropRates:          dropRates,
		volatiles:          volatiles,
		charSpawn:          characterTemplates.Spawn,
		terrain:            terrain,
		npcGenerLogMode:    npcGenerLogSummary,
		channel:            1, // instancia unica: somos o canal 1
		gameplay:           model.DefaultGameplayConfig(),
		lastProtocolNotice: make(map[uint16]time.Time),
		clock:              realClock{},
		rng:                realRNG{},
		itemInstances:      make(map[string]*ItemInstance),
	}
	for _, option := range options {
		option(w)
	}
	// Os deadlines nascem DEPOIS das options: WithClock precisa estar aplicado
	// para que um teste com relogio falso parta do mesmo instante que o mundo.
	start := w.now()
	w.nextAutoSave = start.Add(accountAutoSaveInterval)
	w.nextQuestZoneReset = start.Add(questZoneResetInterval)
	if err := w.gameplay.Validate(); err != nil {
		return nil, fmt.Errorf("configuracao global: %w", err)
	}
	// Montarias a venda nascem vivas (HP/comida/longevidade), senao a loja as
	// exibiria e venderia mortas.
	w.initShopMounts()
	// Registro de guild. Store sem suporte (ou arquivo ausente) resulta em
	// registro vazio: o servidor sobe e os comandos de guild recusam com
	// mensagem, em vez de derrubar o boot.
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	if gs, ok := st.(guildStore); ok {
		registry, err := gs.LoadGuilds()
		if err != nil {
			return nil, fmt.Errorf("carregar guilds: %w", err)
		}
		w.guilds = registry
	}
	// Indice de nomes de personagem em memoria (anti-DoS do 0x20F). Store sem
	// suporte deixa charNames nil e a checagem cai no scan do disco.
	if namer, ok := st.(interface {
		CharacterNames() (map[string]struct{}, error)
	}); ok {
		names, err := namer.CharacterNames()
		if err != nil {
			return nil, fmt.Errorf("indexar nomes de personagem: %w", err)
		}
		w.charNames = names
	}
	// Allowlist de quest. Quest apontando para NPC inexistente ou para um tipo
	// que ja tem handler proprio derruba o boot: seria uma quest que nunca
	// dispararia, e erro silencioso de configuracao e pior que servidor parado.
	questIndex, err := indexQuests(w.questFile, w.npcs)
	if err != nil {
		return nil, fmt.Errorf("quests: %w", err)
	}
	w.questsByNPC = questIndex
	if err := w.spawnInitItems(); err != nil {
		return nil, fmt.Errorf("objetos de mundo: %w", err)
	}
	for _, template := range characterTemplates.Classes {
		if template.Class < 4 {
			w.charTemplates[template.Class] = template
		}
	}
	templates := make(map[string]*model.NPCDef, len(w.npcs)*2)
	for i := range w.npcs {
		n := &w.npcs[i]
		templates[n.Name] = n
		templates[generName(n.Name)] = n
	}
	for itemID, rule := range w.volatiles.Items {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			continue
		}
		spawns := rule.Instance.Spawns
		if len(rule.Instance.Stages) > 0 {
			spawns = nil
			for _, stage := range rule.Instance.Stages {
				spawns = append(spawns, stage.Spawns...)
			}
		}
		for _, spawn := range spawns {
			if templates[spawn.NPC] == nil {
				return nil, fmt.Errorf("volatile item %d: template de instancia %q ausente",
					itemID, spawn.NPC)
			}
		}
	}
	for _, g := range geners {
		if !g.Enabled {
			continue
		}
		// Salas ativadas por item nao podem compartilhar a populacao permanente
		// do NPCGener. O arquivo Micronics contem os mesmos geradores Water com
		// MinuteGenerate=-1 (spawn unico no boot); mantê-los criaria monstros
		// antes do ticket e duplicaria a sala quando a instancia fosse aberta.
		// A reserva nasce da configuracao autoritativa do item, sem depender do
		// numero decorativo da secao no NPCGener.
		if w.generatorReservedForItemInstance(g) {
			continue
		}
		leader := templates[g.Leader]
		if leader == nil {
			leader = templates[generName(g.Leader)]
		}
		follower := templates[g.Follower]
		if follower == nil {
			follower = templates[generName(g.Follower)]
		}
		if leader == nil || follower == nil {
			return nil, fmt.Errorf("NPCGener[%d]: template ausente (Leader=%q Follower=%q)", g.Index, g.Leader, g.Follower)
		}
		w.generators = append(w.generators, generState{def: g, leader: leader, follower: follower})
	}
	now := w.now()
	for i := range w.generators {
		w.spawnGroup(&w.generators[i]) // a primeira chamada equivale ao GenerateMob no boot
		w.scheduleGenerator(&w.generators[i], now)
	}
	w.flushNPCGenerLog(now, true)
	// Bosses NAO adotam mobs do NPCGener: eles nascem do proprio catalogo
	// (data/boss/*.lua), com posicao e respawn proprios.
	if err := w.spawnConfiguredBosses(); err != nil {
		return nil, err
	}
	return w, nil
}

func generName(s string) string { return strings.ReplaceAll(s, "_", " ") }

// allocPlayerID devolve o MENOR ClientId livre a partir de 1 (players ficam
// abaixo de 1000; mobs comecam em 1000). Reusar o slot e o comportamento do
// TMSrv nativo (id = indice da conexao): um relog no mesmo client 7.48 volta a
// receber o MESMO id, e todo estado que o client guarda por id continua valido.
func (w *World) allocPlayerID() uint16 {
	used := make(map[uint16]struct{}, len(w.players))
	for _, p := range w.players {
		if p.ID != 0 {
			used[p.ID] = struct{}{}
		}
	}
	for id := uint16(1); id < 1000; id++ {
		if _, taken := used[id]; !taken {
			return id
		}
	}
	return 999 // mundo cheio (limite de players); inalcancavel hoje
}

func (w *World) scheduleGenerator(g *generState, now time.Time) {
	if g.def.MinuteGenerate > 0 {
		g.nextGenerate = now.Add(time.Duration(g.def.MinuteGenerate) * npcGenerMinute)
	}
}

// spawnGroup porta GenerateMob: cria um lider e MinGroup..MaxGroup seguidores,
// sem ultrapassar MaxNumMob. O StartRange dispersa cada membro ao redor do Start.
func (w *World) spawnGroup(g *generState) {
	remaining := g.def.MaxNumMob - g.current
	if remaining <= 0 {
		return
	}
	followers := g.def.MinGroup
	if d := g.def.MaxGroup - g.def.MinGroup + 1; d > 1 {
		followers += rand.Intn(d)
	}
	total := 1 + followers
	if total > remaining {
		total = remaining
	}
	leaderID := uint16(0)
	for i := 0; i < total; i++ {
		def := g.leader
		if i > 0 {
			def = g.follower
		}
		pos := g.def.Segments[0]
		requestedX, requestedY := scatter(pos.X, pos.Y, pos.Range)
		x, y := requestedX, requestedY
		x, y = w.findFreePosition(x, y, pos.Range)
		if x != requestedX || y != requestedY {
			w.npcGenerLog.relocations++
			if w.npcGenerLogMode == npcGenerLogVerbose {
				log.Printf("NPCGener[%d]: spawn %q reposicionado (%d,%d)->(%d,%d): terreno bloqueado/ocupado",
					g.def.Index, def.Name, requestedX, requestedY, x, y)
			}
		}
		segments := g.def.Segments
		for si := range segments {
			if segments[si].X != 0 && segments[si].Y != 0 {
				segments[si].X, segments[si].Y = scatter(segments[si].X, segments[si].Y, segments[si].Range)
				segments[si].X, segments[si].Y = w.findWalkablePosition(segments[si].X, segments[si].Y, segments[si].Range)
			}
		}
		m := &Mob{ID: w.allocMobID(), Def: def, X: x, Y: y, HP: def.Extended.MaxHP,
			GenerIndex: g.def.Index, Segments: segments, RouteType: g.def.RouteType,
			WaitUntil: w.now().Add(time.Duration(g.def.Segments[0].Wait) * time.Second)}
		if i == 0 {
			leaderID = m.ID
		} else {
			m.LeaderID = leaderID
		}
		w.mobs = append(w.mobs, m)
		g.current++
		w.publishMobSpawn(m)
	}
	w.npcGenerLog.groups++
	w.npcGenerLog.mobs += total
	if w.npcGenerLogMode == npcGenerLogVerbose {
		log.Printf("NPCGener[%d]: grupo gerado (%d mobs, vivos=%d/%d)",
			g.def.Index, total, g.current, g.def.MaxNumMob)
	}
}

func (w *World) flushNPCGenerLog(now time.Time, initial bool) {
	if w.npcGenerLogMode != npcGenerLogSummary {
		w.npcGenerLog = npcGenerLogStats{}
		w.nextGenerLog = now.Add(npcGenerSummaryInterval)
		return
	}
	if !initial && now.Before(w.nextGenerLog) {
		return
	}
	if initial || w.npcGenerLog.groups != 0 || w.npcGenerLog.relocations != 0 {
		phase := "periodico"
		if initial {
			phase = "inicial"
		}
		log.Printf("NPCGener resumo %s: geradores=%d grupos=%d mobs=%d reposicionados=%d vivos=%d",
			phase, len(w.generators), w.npcGenerLog.groups, w.npcGenerLog.mobs,
			w.npcGenerLog.relocations, len(w.mobs))
	}
	w.npcGenerLog = npcGenerLogStats{}
	w.nextGenerLog = now.Add(npcGenerSummaryInterval)
}

func (w *World) positionOccupied(x, y uint16, except *Mob) bool {
	return w.positionOccupiedExcept(x, y, except, nil)
}

func (w *World) mobStepBlockedFrom(m *Mob, fromX, fromY, toX, toY uint16) bool {
	return m == nil || !w.terrain.RouteHeightCompatible(fromX, fromY, toX, toY) ||
		w.positionOccupied(toX, toY, m)
}

func (w *World) positionOccupiedExcept(x, y uint16, exceptMob *Mob, exceptPlayer *Player) bool {
	if !w.terrain.Walkable(x, y) {
		return true
	}
	key := spatialKey(x, y)
	for _, m := range w.mobCells[key] {
		if m != exceptMob && !m.Dead && m.X == x && m.Y == y {
			return true
		}
	}
	for _, p := range w.playerCells[key] {
		if p != exceptPlayer && p.InWorld && p.Char != nil && playerCurHP(p.Char) > 0 && p.X == x && p.Y == y {
			return true
		}
	}
	for _, shop := range w.ghostShops {
		if shop.X == x && shop.Y == y {
			return true
		}
	}
	return false
}

// findFreePlayerPosition escolhe primeiro um dos oito tiles adjacentes e depois
// expande em aneis. O proprio jogador e ignorado, importante quando ele ja esta
// sobre a coordenada de recall e apenas esta renascendo.
func (w *World) findFreePlayerPosition(x, y uint16, radius int, player *Player) (uint16, uint16) {
	if !w.positionOccupiedExcept(x, y, nil, player) {
		return x, y
	}
	if radius < 1 {
		radius = 1
	}
	for distance := 1; distance <= radius; distance++ {
		// Prioriza empurrar para os lados/cardinais; diagonais e restante do anel
		// sao tentados logo depois.
		offsets := [][2]int{{distance, 0}, {-distance, 0}, {0, distance}, {0, -distance}}
		for dy := -distance; dy <= distance; dy++ {
			for dx := -distance; dx <= distance; dx++ {
				if (dx == 0 && absInt(dy) == distance) || (dy == 0 && absInt(dx) == distance) ||
					(absInt(dx) != distance && absInt(dy) != distance) {
					continue
				}
				offsets = append(offsets, [2]int{dx, dy})
			}
		}
		for _, offset := range offsets {
			nx, ny := int(x)+offset[0], int(y)+offset[1]
			if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
				continue
			}
			if w.terrain.HeightCompatible(x, y, uint16(nx), uint16(ny)) &&
				!w.positionOccupiedExcept(uint16(nx), uint16(ny), nil, player) {
				return uint16(nx), uint16(ny)
			}
		}
	}
	return x, y
}

// removeMobInstance elimina a instancia morta da lista ativa. O client ja
// recebeu RemoveMob; conservar o ponteiro aqui so aumenta a busca linear e,
// depois de muitos respawns, faz parecer que os grupos antigos ainda existem.
func (w *World) removeMobInstance(dead *Mob) {
	w.unregisterMobSpatial(dead)
	for i, m := range w.mobs {
		if m == dead {
			copy(w.mobs[i:], w.mobs[i+1:])
			w.mobs[len(w.mobs)-1] = nil
			w.mobs = w.mobs[:len(w.mobs)-1]
			return
		}
	}
}

func (w *World) findFreePosition(x, y, radius uint16) (uint16, uint16) {
	if !w.positionOccupied(x, y, nil) {
		return x, y
	}
	r := int(radius)
	if r < 2 {
		r = 2
	}
	for d := 1; d <= r+4; d++ {
		for dy := -d; dy <= d; dy++ {
			for dx := -d; dx <= d; dx++ {
				if absInt(dx) != d && absInt(dy) != d {
					continue
				}
				nx, ny := int(x)+dx, int(y)+dy
				if nx > 0 && ny > 0 && nx <= 65535 && ny <= 65535 &&
					w.terrain.HeightCompatible(x, y, uint16(nx), uint16(ny)) &&
					!w.positionOccupied(uint16(nx), uint16(ny), nil) {
					return uint16(nx), uint16(ny)
				}
			}
		}
	}
	return x, y
}

// findWalkablePosition corrige destinos do NPCGener sem considerar ocupacao:
// varios mobs podem compartilhar o mesmo segmento, mas nunca uma celula 127.
func (w *World) findWalkablePosition(x, y, radius uint16) (uint16, uint16) {
	if w.terrain.Walkable(x, y) {
		return x, y
	}
	r := int(radius)
	if r < 2 {
		r = 2
	}
	for d := 1; d <= r+8; d++ {
		for dy := -d; dy <= d; dy++ {
			for dx := -d; dx <= d; dx++ {
				if absInt(dx) != d && absInt(dy) != d {
					continue
				}
				nx, ny := int(x)+dx, int(y)+dy
				if nx > 0 && ny > 0 && nx < model.TerrainWidth && ny < model.TerrainHeight &&
					w.terrain.Walkable(uint16(nx), uint16(ny)) {
					return uint16(nx), uint16(ny)
				}
			}
		}
	}
	return x, y
}

func absInt(v int) int {
	if v < 0 {
		return -v
	}
	return v
}

func scatter(x, y, radius uint16) (uint16, uint16) {
	if radius == 0 {
		return x, y
	}
	r := int(radius)
	dx, dy := rand.Intn(2*r+1)-r, rand.Intn(2*r+1)-r
	nx, ny := int(x)+dx, int(y)+dy
	if nx < 1 {
		nx = 1
	}
	if ny < 1 {
		ny = 1
	}
	return uint16(nx), uint16(ny)
}

// Enqueue empurra um pacote (ou desconexao) pro loop. Chamado pelas goroutines de
// sessao. Bloqueia so se o buffer encher -- backpressure daquele client, sem
// travar os outros.
func (w *World) Enqueue(s *net.Session, pkt []byte) {
	w.commands <- command{s: s, pkt: pkt}
}

// worldTickInterval e o TIMER_SEC nativo. A IA pesada e sharded dentro de
// tick(), como o TMSrv (combate %4; paz/rota/affects %6), em vez de varrer
// milhares de mobs cinco vezes por segundo.
const worldTickInterval = 500 * time.Millisecond

// Run e o game loop: comandos dos clients + tick do jogo, processados linearmente.
func (w *World) Run() {
	ticker := time.NewTicker(worldTickInterval)
	defer ticker.Stop()
	for {
		select {
		case cmd := <-w.commands:
			w.safeHandle(cmd)
		case <-ticker.C:
			w.tick()
		}
	}
}

// commandLabel devolve o rotulo de metrica de um comando: o opcode, um conjunto
// fechado e pequeno (nunca sessao/jogador, que explodiriam a cardinalidade).
//
// E deliberadamente bounds-safe: wire.ParseHeader indexa 12 bytes sem checar o
// tamanho, e este rotulo e calculado FORA do recover de safeHandle. Um pacote
// truncado nao pode derrubar o game loop justamente na funcao que existe para
// conte-lo.
func commandLabel(cmd command) string {
	if cmd.pkt == nil {
		return "login" // comando interno (resultado de autenticacao)
	}
	if len(cmd.pkt) < wire.HeaderSize {
		return "malformed"
	}
	return fmt.Sprintf("0x%X", wire.ParseHeader(cmd.pkt).Type)
}

// safeHandle isola um panic de handler. O World roda numa UNICA goroutine, entao
// um panic nao tratado derrubaria TODOS os jogadores (o client ve "nao
// conectado"). O recover loga o stack e o pacote culpado e mantem o servidor
// vivo; no pior caso o estado de UM comando fica parcial, o que e muito melhor
// que a queda total. Tambem serve de diagnostico: o Type do pacote sai no log.
func (w *World) safeHandle(cmd command) {
	label := commandLabel(cmd)
	start := time.Now()
	defer func() {
		if r := recover(); r != nil {
			var id int64
			if cmd.s != nil {
				id = cmd.s.ID
			}
			metricPanicsTotal.Add(1)
			log.Printf("[#%d] PANIC no handler (Type=%s): %v\n%s", id, label, r, debug.Stack())
		}
		// Medido no defer para que um comando que panicou tambem apareca na
		// duracao -- normalmente e justo o que interessa investigar.
		observeCommand(label, time.Since(start))
	}()
	w.handle(cmd)
}

// tick: o NPCGener repoe grupos no intervalo MinuteGenerate (ticks de 12 s)
// enquanto houver
// espaco em MaxNumMob. Entradas -1/0 sao apenas de geracao inicial/manual.
func (w *World) tick() {
	now := w.now()
	// Instrumentacao usa o relogio REAL: mede custo de execucao, nao tempo de
	// jogo. Um clock falso de teste nao deve falsear a duracao observada.
	tickStart := time.Now()
	defer func() {
		observeTick(tickStart, time.Since(tickStart))
		observeWorldGauges(len(w.players), len(w.activeMobs))
		metricCommandQueueDepth.Set(int64(len(w.commands)))
	}()
	w.mobTickCounter++
	// O grid ja reduziu a lista aos mobs com jogador proximo. Uma vez acordado,
	// o mob percebe alvo a cada 1 s. Perseguicao e patrulha so iniciam um novo
	// trecho a cada 2 s, evitando emendar animacoes na velocidade maxima. O
	// executor permanece em 500 ms para cumprir o cooldown de 1,5 s.
	if w.mobTickCounter%2 == 0 {
		allowMovement := w.mobTickCounter%4 == 0
		w.tickMobCombat(now, 0, 1, allowMovement)
		if allowMovement {
			w.tickMobRoutes(now, 0, 1)
		}
	}
	w.tickActiveMobActions(now)
	// Acoes de boss vencem pelo relogio do mundo; nao ha ticker por boss.
	w.tickBossActions(now)
	w.tickBossRespawns(now)
	w.tickSummonCombat(now)
	w.tickSephiraObjects(now)
	w.tickPlayerAffects(now)
	w.tickMobAffects(now, int(w.mobTickCounter%6), 6)
	w.tickPlayerRegen(now)
	w.tickPlayerMounts(now)
	w.tickGroundItems(now)
	w.tickItemInstances(now)
	w.tickDungeonTeleports(now)
	w.tickTrades(now)
	if !now.Before(w.nextQuestZoneReset) {
		w.tickQuestZoneReset(now)
		w.nextQuestZoneReset = now.Add(questZoneResetInterval)
	}
	if !now.Before(w.nextAutoSave) {
		w.autoSaveAccounts(now)
	}
	for i := range w.generators {
		g := &w.generators[i]
		if !g.nextGenerate.IsZero() && !now.Before(g.nextGenerate) {
			w.spawnGroup(g)
			w.scheduleGenerator(g, now)
		}
	}
	w.flushNPCGenerLog(now, false)
}

// tickQuestZoneReset porta o ClearArea do W2PP: a cada ciclo (10 min), todo
// jogador dentro de uma zona de quest e recolhido para a cidade. E global por
// deadline, nao per-player -- reproduz o comportamento nativo (o jogador nao
// "ganha" 10 min cheios; o relogio do servidor e que decide).
func (w *World) tickQuestZoneReset(now time.Time) {
	if len(w.questZones) == 0 {
		return
	}
	for _, p := range w.players {
		if !p.InWorld || p.Char == nil {
			continue
		}
		for i := range w.questZones {
			if w.questZones[i].Contains(p.X, p.Y) {
				w.recallPlayer(p, "reset "+w.questZones[i].Name)
				break
			}
		}
	}
}

func (w *World) autoSaveAccounts(now time.Time) {
	w.nextAutoSave = now.Add(accountAutoSaveInterval)
	active := make([]*Player, 0, len(w.players))
	for _, p := range w.players {
		if !p.InWorld || p.Account == nil || p.Char == nil {
			continue
		}
		active = append(active, p)
		// Autosave e periodico, nao gate de confirmacao: usa o caminho ASSINCRONO
		// para tirar o fsync do game-loop. Os saves anti-dupe seguem sincronos.
		if err := w.saveAccountAsync(p.Account); err != nil {
			log.Printf("[#%d] ERRO no autosave da conta %q: %v", p.Session.ID, p.Account.Name, err)
		}
	}
	// Enfileira charstates depois das contas. No Postgres isso deixa os snapshots
	// de conta contiguos para o worker consolida-los em poucos commits.
	for _, p := range active {
		// Buffs e moedas vivem no sidecar de sessao, gravado junto do autosave.
		w.saveCharStateAsync(p)
	}
}

// broadcast manda um pacote pra todos os players no mundo. O builder e chamado
// UMA VEZ POR player (Send criptografa in-place, entao cada um precisa do seu []byte).
func (w *World) broadcast(build func() []byte) {
	for _, p := range w.players {
		if p.InWorld {
			p.Session.Send(build())
		}
	}
}

// handle despacha um comando pelo Type do header.
func (w *World) handle(cmd command) {
	if cmd.shutdown != nil {
		w.runShutdown(cmd.shutdown)
		return
	}
	if cmd.login != nil {
		w.onLoginResult(cmd.s, cmd.login)
		return
	}
	if cmd.pkt == nil {
		w.onDisconnect(cmd.s)
		return
	}
	if !w.validateInboundCommand(cmd.s, cmd.pkt) {
		return
	}
	h := wire.ParseHeader(cmd.pkt)
	switch int(h.Type) {
	case wire.OpConnectAccount:
		w.onLogin(cmd.s, cmd.pkt)
	case wire.OpCreateCharacter:
		w.onCreateCharacter(cmd.s, cmd.pkt)
	case wire.OpCharacterLogin:
		w.onEnterWorld(cmd.s, cmd.pkt)
	case wire.OpCharacterLogout:
		w.onCharacterLogout(cmd.s, cmd.pkt)
	case wire.OpDeleteCharacter:
		w.onDeleteCharacter(cmd.s, cmd.pkt)
	case wire.OpSwapItem:
		w.onSwapItem(cmd.s, cmd.pkt)
	case wire.OpDeposit:
		w.onCargoGold(cmd.s, cmd.pkt, true)
	case wire.OpWithdraw:
		w.onCargoGold(cmd.s, cmd.pkt, false)
	case wire.OpUseItem:
		w.onUseItem(cmd.s, cmd.pkt)
	case wire.OpUseNPC, wire.OpReqShopList:
		w.onUseNPC(cmd.s, cmd.pkt)
	case wire.OpBuyItem:
		w.onBuyItem(cmd.s, cmd.pkt)
	case wire.OpSellItem:
		w.onSellItem(cmd.s, cmd.pkt)
	case wire.OpApplyBonus:
		w.onApplyBonus(cmd.s, cmd.pkt)
	case wire.OpPartyRequest:
		w.onPartyRequest(cmd.s, cmd.pkt)
	case wire.OpPartyAccept:
		w.onPartyAccept(cmd.s, cmd.pkt)
	case wire.OpPartyRemove:
		w.onPartyRemove(cmd.s, cmd.pkt)
	case wire.OpTrade:
		w.onTrade(cmd.s, cmd.pkt)
	case wire.OpCloseTrade:
		w.onCloseTrade(cmd.s)
	case wire.OpAutoTrade:
		w.onAutoTrade(cmd.s, cmd.pkt)
	case wire.OpReqTradeList:
		w.onReqTradeList(cmd.s, cmd.pkt)
	case wire.OpReqBuyAutoTrade:
		w.onReqBuyAutoTrade(cmd.s, cmd.pkt)
	case wire.OpDropItem:
		w.onDropItem(cmd.s, cmd.pkt)
	case wire.OpGetItem:
		w.onGetItem(cmd.s, cmd.pkt)
	case wire.OpMessageChat:
		w.onMessageChat(cmd.s, cmd.pkt)
	case wire.OpMessageWhisper:
		w.onMessageWhisper(cmd.s, cmd.pkt)
	case wire.OpSetShortSkill:
		w.onSetShortSkill(cmd.s, cmd.pkt)
	case wire.OpChangeCity:
		w.onChangeCity(cmd.s, cmd.pkt)
	case wire.OpReqTeleport:
		w.onReqTeleport(cmd.s, cmd.pkt)
	case wire.OpPKMode:
		w.onPKMode(cmd.s, cmd.pkt)
	case wire.OpGuildDeprivate:
		w.onGuildDeprivate(cmd.s, cmd.pkt)
	case wire.OpGuildAlly:
		w.onGuildAlly(cmd.s, cmd.pkt)
	case wire.OpGuildWar:
		// Reconhecido para nao poluir o log. Guerra de guild esta fora do
		// escopo deste marco: exige zonas, torre e estado de cerco.
		log.Printf("[#%d] 0xE0E guerra de guild ignorada (sistema nao implementado)", cmd.s.ID)
	case wire.OpChallenge, wire.OpChallengeConfirm:
		w.onGuildChallenge(cmd.s, cmd.pkt)
	case wire.OpMoveStop:
		w.onMoveStop(cmd.s, cmd.pkt)
	case wire.OpRestart:
		w.onRestart(cmd.s)
	case wire.OpPing:
		// Keepalive: receber o pacote ja prova que a sessao esta viva.
	case wire.OpSysQuit:
		w.onSysQuit(cmd.s)
	case wire.OpAction:
		w.onMove(cmd.s, cmd.pkt)
	case wire.OpActionStop:
		w.onActionStop(cmd.s, cmd.pkt)
	case wire.OpREQMobByID:
		w.onREQMobByID(cmd.s, cmd.pkt)
	case wire.OpMotion:
		w.onMotion(cmd.s, cmd.pkt)
	case wire.OpClientUnknown2BC:
		w.onClientUnknown2BC(cmd.s, cmd.pkt)
	case wire.OpAttackOne, wire.OpAttackMulti, wire.OpAttackTwo:
		// 0x39D e o melee do 7.48 (confirmado in-game: repete ~1x/s ao atacar).
		w.onAttack(cmd.s, cmd.pkt)
	case wire.OpCombineTiny:
		w.onCombineTiny(cmd.s, cmd.pkt)
	case wire.OpCombineLindy:
		w.onCombineLindy(cmd.s, cmd.pkt)
	case wire.OpCombineCompositor:
		w.onCombineCompositor(cmd.s, cmd.pkt)
	case wire.OpCombineAgatha:
		w.onCombineAgatha(cmd.s, cmd.pkt)
	case wire.OpCombineAylin:
		w.onCombineAylin(cmd.s, cmd.pkt)
	case wire.OpCombineEhre:
		w.onCombineEhre(cmd.s, cmd.pkt)
	case wire.OpCombineOdin:
		w.onCombineOdin(cmd.s, cmd.pkt)
	default:
		log.Printf("[#%d] sem handler: Type=0x%X Size=%d", cmd.s.ID, h.Type, h.Size)
	}
}

// spawnDrop poe um item no CHAO (com efeitos preservados -- item jogado pelo
// player carrega refino/adds). Usado como fallback do loot (inventario cheio) e
// pelo jogar-item (0x272).
func (w *World) spawnDrop(x, y uint16, item model.Item) *GroundItem {
	return w.createGroundDrop(x, y, item, true)
}

func (w *World) createGroundDrop(x, y uint16, item model.Item, publish bool) *GroundItem {
	itemIndex := item.Index
	item, err := materializeItem(item)
	if err != nil {
		log.Printf("materializar drop item=%d: %v", itemIndex, err)
		return nil
	}
	// O client procura Canhao (746) exatamente sob o jogador e somente nos IDs
	// 15001..15100. Demais drops continuam espalhados ao redor da origem.
	dropX, dropY := x, y
	if item.Index != 746 {
		dropX = x + uint16(rand.Intn(3)) - 1
		dropY = y + uint16(rand.Intn(3)) - 1
	}

	if _, ok := w.items[item.Index]; !ok {
		log.Printf("Tentou dropar item inexistente: %d", item.Index)
		return nil
	}

	id := w.allocGroundItemID(item.Index)
	if id == 0 {
		return nil
	}

	gItem := &GroundItem{
		ID:     id,
		Item:   item,
		X:      dropX,
		Y:      dropY,
		Expire: w.now().Add(2 * time.Minute),
	}
	w.groundItems[id] = gItem

	if publish {
		w.publishItemSpawn(gItem)
	}
	return gItem
}

// spawnInitItems poe a mobilia do mapa no chao antes do servidor abrir. Roda no
// NewWorld, entao ninguem esta conectado: nao ha o que publicar, o AOI entrega
// cada objeto quando um jogador chega perto.
//
// Reusa allocGroundItemID de proposito -- ele ja poe o Canhao (746) na faixa
// 15001..15100 que o client exige, e a checagem de ocupacao impede que um drop
// futuro receba o ID de um objeto permanente, que fica em groundItems para
// sempre.
func (w *World) spawnInitItems() error {
	for _, obj := range w.initItems {
		id := w.allocGroundItemID(obj.Index)
		if id == 0 {
			return fmt.Errorf("sem ID livre para o objeto %d em (%d,%d)",
				obj.Index, obj.X, obj.Y)
		}
		w.groundItems[id] = &GroundItem{
			ID:        id,
			Item:      model.Item{Index: obj.Index},
			X:         obj.X,
			Y:         obj.Y,
			Rotate:    obj.Rotate,
			Permanent: true,
		}
	}
	if len(w.initItems) > 0 {
		log.Printf("objetos de mundo: %d postos no mapa", len(w.initItems))
	}
	return nil
}

func (w *World) allocGroundItemID(itemIndex uint16) uint16 {
	if itemIndex == 746 {
		for id := uint16(15001); id <= 15100; id++ {
			if _, used := w.groundItems[id]; !used {
				return id
			}
		}
		return 0
	}
	for {
		id := w.nextItemID
		w.nextItemID++
		if id >= 15001 && id <= 15100 {
			continue
		}
		if _, used := w.groundItems[id]; !used {
			return id
		}
	}
}

func (w *World) tickGroundItems(now time.Time) {
	for id, item := range w.groundItems {
		if item.Permanent {
			continue // portao/porta/canhao: mobilia do mapa, nao drop
		}
		if now.After(item.Expire) {
			w.publishItemRemove(item)
			delete(w.groundItems, id)
		}
	}
}
