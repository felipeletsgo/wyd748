// Package game owns the game state: one World runs in a single goroutine
// (actor model, no mutexes). Session commands and game ticks are processed
// sequentially by its loop.
package game

import (
	"fmt"
	"log"
	"runtime/debug"
	"strings"
	"sync/atomic"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// command is a packet received from a session (nil pkt means disconnect).
type command struct {
	kick            *kickRequest
	teleport        *teleportRequest
	bosses          *bossesRequest
	globalDrop      *globalDropRequest
	quiz            *quizRequest
	control         *controlRequest
	accountPresence *accountPresenceRequest
	s               *net.Session
	pkt             []byte
	login           *loginResult
	queuedAt        time.Time
	// A shutdown request runs on the World goroutine like any other command.
	// This lets the final drain observe consistent state without racing a handler.
	shutdown chan error
}

type commandBatchQueue struct {
	head int16
	tail int16
}

const commandBatchEnd = int16(-1)

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

// saveAccount is the world's sole persistence boundary. Current position is
// session state: saves project Char.X/Y to a legacy neutral point in a copy,
// leaving the live session intact. Reentry city comes from the hometown bits
// persisted in Score.Merchant.
func (w *World) saveAccount(account *model.Account) error {
	return w.store.SaveAccount(accountPersistenceSnapshot(account))
}

// asyncSaveStore exposes asynchronous saves for autosave. Stores without this
// interface fall back to synchronous saves, leaving fsync on the game loop.
type asyncSaveStore interface {
	SaveAccountAsync(acc *model.Account) error
}

// saveAccountAsync writes outside the game loop when supported. Only autosave
// uses it; anti-duplication saves remain synchronous via saveAccount.
func (w *World) saveAccountAsync(account *model.Account) error {
	snapshot := accountPersistenceSnapshot(account)
	if as, ok := w.store.(asyncSaveStore); ok {
		return as.SaveAccountAsync(snapshot)
	}
	return w.store.SaveAccount(snapshot)
}

// Player holds a session, account, selected character, and world ID in memory.
type Player struct {
	Session  *net.Session
	Account  *model.Account
	Char     *model.Char
	CharSlot int
	ID       uint16
	InWorld  bool
	// PersistencePoisoned is set only after a handler panic. We cannot know
	// which aggregates it changed; saving on disconnect/autosave could persist
	// partial state and duplicate items. The world enters maintenance mode and
	// discards RAM state, preserving the last commit.
	PersistencePoisoned bool
	X, Y                uint16 // current position tracked from movement packets 0x366
	AirMoveActive       bool
	AirMoveRoute        int
	AirMoveStartedAt    time.Time
	AirMoveSourceX      uint16
	AirMoveSourceY      uint16
	// NPC whose shop is open. The 7.48 buy packet has TargetID=0, so the
	// server uses this authoritative ID rather than trusting the packet.
	ShopNPC  uint16
	ShopTax  uint32
	CraftNPC uint16
	CargoNPC uint16
	// Ephemeral tax/war collector context. Confirmation 0x28F may reuse only
	// the NPC the player actually opened, for a few seconds.
	CityWarNPC          uint16
	CityWarCity         int
	CityWarGuild        uint16
	CityWarUntil        time.Time
	LastCraft           time.Time
	DeadAt              time.Time
	LastPotion          time.Time
	LastPremiumFirework time.Time
	SkillReady          map[int]time.Time
	NextRegen           time.Time
	NextCPRecovery      time.Time
	NextMountTick       time.Time
	// Remaining whole hours live on the egg (EF_INCUDELAY). These fields track
	// only the current online hour for that equipped egg; it resets on unequip,
	// character switch, or disconnect, as in the native client.
	EggIncubationUID      string
	NextEggIncubationTick time.Time
	// Cooldown shared by the /kingdom and /king commands.
	NextKingdomTeleport time.Time
	Visible             map[uint16]struct{} // entities currently materialized for this client
	Party               *Party
	InviteFrom          uint16
	InviteUntil         time.Time
	ChallengeFrom       uint16
	ChallengeMode       uint32
	ChallengeUntil      time.Time
	// Pending guild invitation. Like a party invitation, it stores the inviter
	// and expiry so an old invitation cannot be accepted.
	GuildInviteFrom  uint16
	GuildInviteUntil time.Time
	// Native recruitment cooldown (0x3D5). It is ephemeral, resets on character
	// switch, and is never accepted as client-supplied state.
	NextGuildInvite time.Time
	// Authoritative summon targets: the owner's target first, or their attacker.
	CombatTargetID uint16
	LastAttackerID uint16
	LastAttackAt   time.Time
	LastAttackTick uint32
	// Skills use the native per-skill SkillData.Delay and must not share the
	// physical swing gate with a normal attack. A shared gate made an auto
	// attack arriving just before a spell silently discard the spell packet.
	// Keep a small per-skill tick history for replay/order checks and a global
	// timestamp only as a packet-flood floor.
	LastSkillAt    time.Time
	LastSkillTicks map[int]uint32
	AttackProgress uint16
	// The 7.48 client may repeat Action while walking. These fields retain the
	// last published destination to suppress only repetitions of the same plan;
	// a changed destination retains the validated new origin and Route[24] for
	// observer interpolation.
	MovePublished        bool
	MovePublishedStartX  uint16
	MovePublishedStartY  uint16
	MovePublishedTargetX uint16
	MovePublishedTargetY uint16
	MovePublishedRoute   [maxMovementRouteBytes]byte
	// A published route animates the client immediately, but authoritative
	// position advances only on the World clock. A valid 24-tile intention
	// cannot grant range, pickup, or interaction at its future destination.
	MoveAuthorityRoute        []byte
	MoveAuthorityStep         int
	MoveAuthorityCatchupSteps int
	MoveAuthorityX            uint16
	MoveAuthorityY            uint16
	MoveAuthorityStartedAt    time.Time
	MoveAuthorityStepInterval time.Duration
	Trade                     *TradeState
	GhostShop                 *GhostShop
	// Ghost shop open in this client, used to close affected buyers when the
	// clone disappears.
	BrowsingGhostShopID uint16
	PKMode              bool
	// SpecialCoins are named counters persisted by character UID alongside buffs
	// in the session-state sidecar.
	SpecialCoins map[string]uint32
	// Ephemeral active-character probe. Cleared on success, rejection, timeout,
	// logout, and relogin; it is never part of the persisted aggregate.
	clientIntegrityPending *clientIntegrityPending
}

// Party is World-owned state, never persisted in the account. Members always
// contains the leader first, followed by members in joining order.
type Party struct {
	Members []*Player
}

// Mob is a live NPC/monster instance of an NPCDef in the world.
type Mob struct {
	GuildWarTower    bool
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
	// Rotate is the object's map orientation. Ordinary drops use 0; permanent
	// objects load their rotation from data/init_items.csv.
	Rotate byte
	// Permanent marks world fixtures (gate, door, cannon, tower). They neither
	// expire nor permit pickup. The native server keeps them below g_dwInitItem,
	// a range never swept by decay.
	Permanent bool
	// InstanceID temporarily restricts loot from a runtime to its members.
	// Cleanup may release it to the public world when that run ends, according
	// to the event's rules.
	InstanceID string
	// State is 0 for closed or 1 for open. Only doors use it. The native server
	// changes it with MSG_UpdateItem rather than recreating the object.
	State byte
}

// TMSrv calls this counter "MinuteGenerate", but TIMER_MIN runs every 12000 ms.
// Keeping this tick preserves existing NPCGener.txt values.
const npcGenerMinute = 12 * time.Second
const accountAutoSaveInterval = 3 * time.Second
const accountAutoSaveBuckets = 6
const accountAutoSaveSliceInterval = accountAutoSaveInterval / accountAutoSaveBuckets

// The native server restores one point of negative CP every 450 second cycles
// (Server.cpp/RegenMob.cpp). The session counter resets on world entry; CP
// itself remains persisted on the character.
const chaosRecoveryInterval = 450 * time.Second

// questZoneResetInterval ports the WYD 7.48 quest-zone reset: SecCounter%1200
// with TIMER_SEC=500ms, or ten real minutes.
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

// WithQuests supplies parsed quests.json. NewWorld validates each NPC against
// the quest entries (existence and reserved type) once both sides are present;
// invalid configuration fails startup instead of being silently ignored.
func WithQuests(file model.QuestFile) WorldOption {
	return func(w *World) { w.questFile = file }
}

func WithQuestZones(file model.QuestZoneFile) WorldOption {
	return func(w *World) { w.questZones = file.Zones }
}

// WithUxmal supplies the authoritative Rune Track template. An NPC starts the
// event, so no artificial EF_VOLATILE rule is needed.
func WithUxmal(instance model.VolatileInstance) WorldOption {
	return func(w *World) {
		copy := cfgCopy(instance)
		w.uxmal = &copy
	}
}

// WithInitItems supplies permanent world fixtures (gates, doors, cannons,
// towers). They enter the map at startup and remain there.
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

// WithLoadtestSpawn enables an alternate spawn only for accounts whose login
// starts with the configured prefix. An empty prefix disables it and keeps
// 2100,2100 for real players.
func WithLoadtestSpawn(spawn model.CharacterSpawn, accountPrefix string) WorldOption {
	return func(w *World) {
		w.loadtestSpawn = spawn
		w.loadtestAccountPrefix = strings.ToLower(strings.TrimSpace(accountPrefix))
	}
}

// WithMounts supplies mount stat bonuses by type. Without it, the mount slot
// adds no attributes.
func WithMounts(catalog model.MountCatalog) WorldOption {
	return func(w *World) {
		w.mounts = catalog
	}
}

// World exclusively owns game state.
type World struct {
	bossesPending      atomic.Bool
	kickPending        atomic.Bool
	kickEpoch          string
	kickReceipts       map[string]kickReceipt
	teleportPending    atomic.Bool
	teleportReceipts   map[string]teleportReceipt
	bossesEpoch        string
	bossesReceipts     map[string]bossesReceipt
	globalDropPending  atomic.Bool
	globalDropEpoch    string
	globalDrop         *globalDropState
	globalDropReceipts map[string]globalDropReceipt
	quizPending        atomic.Bool
	quizEpoch          string
	quiz               *quizState
	quizReceipts       map[string]quizReceipt
	controlPending     atomic.Bool
	commands           chan command
	// pendingCommands retains the remainder of a batch when the tick deadline
	// arrives. World remains the sole writer of this queue.
	pendingCommands          []command
	pendingCommandHead       int
	commandBatchScratch      []command
	commandBatchOrderScratch []*net.Session
	commandBatchQueues       map[*net.Session]commandBatchQueue
	commandBatchNext         [worldCommandBatchLimit]int16
	players                  map[*net.Session]*Player
	playersByID              map[uint16]*Player
	playersByCharacterUID    map[string]*Player
	playersByName            map[string]*Player
	accountSessions          map[string]*net.Session
	authClientsByIP          map[string]map[*net.Session]struct{}
	authClientsByNetwork     map[string]map[*net.Session]struct{}
	authPending              map[*net.Session]bool
	authSlots                chan struct{}
	operational              OperationalConfig
	networkAdmission         compiledNetworkAdmission
	networkAdmissionErr      error
	authRateByIP             map[string]*fixedWindowRate
	authRateByAccount        map[string]*fixedWindowRate
	chatRateByAccount        map[string]*fixedWindowRate
	// security aggregates violations per connection at every phase (including
	// before login), without trusting packet-supplied identity fields.
	security map[*net.Session]*securityState
	// charNames is an in-memory lowercase character-name index populated at
	// startup and maintained on create/delete. It avoids scanning all accounts
	// for each 0x20F request (DoS). nil falls back to a store scan.
	charNames             map[string]struct{}
	store                 store.Store
	npcs                  []model.NPCDef
	mobs                  []*Mob
	mobsByID              map[uint16]*Mob
	mobListIndex          map[uint16]int
	mobCells              map[uint32]map[uint16]*Mob
	playerCells           map[uint32]map[uint16]*Player
	mobCell               map[uint16]uint32
	playerCell            map[uint16]uint32
	activeMobs            map[uint16]*Mob
	mobActivationScratch  map[uint16]*Mob
	summons               map[uint16]*Mob
	sephiraObjects        map[uint16]*Mob
	generators            []generState
	generatorByIndex      map[int]int
	nextMobID             uint16
	items                 map[uint16]model.ItemDef
	skills                map[int]model.SkillDef
	groundItems           map[uint16]*GroundItem
	ghostShops            map[uint16]*GhostShop
	groundItemCells       map[uint32]map[uint16]*GroundItem
	groundItemCell        map[uint16]uint32
	groundExpiry          groundItemExpiryHeap
	groundExpiryByID      map[uint16]time.Time
	ghostShopCells        map[uint32]map[uint16]*GhostShop
	ghostShopCell         map[uint16]uint32
	nextItemID            uint16
	nextAutoSave          time.Time
	autoSaveBucket        uint8
	autoSaveScratch       []*Player
	nextTimedItemSweep    time.Time
	dropRates             [model.MaxCarry]int // native drop rate by carry slot
	volatiles             model.VolatileCatalog
	mounts                model.MountCatalog
	charSpawn             model.CharacterSpawn
	loadtestSpawn         model.CharacterSpawn
	loadtestAccountPrefix string
	charTemplates         [4]model.CharacterTemplate
	terrain               model.TerrainMap
	npcGenerLogMode       npcGenerLogMode
	npcGenerLog           npcGenerLogStats
	nextGenerLog          time.Time
	gameplayLogMode       gameplayLogMode
	gameplayLog           gameplayLogStats
	nextGameplayLog       time.Time
	teleports             []model.Teleport
	gameplay              model.GameplayConfig
	clientIntegrity       model.ClientIntegrityFile
	// guilds is the canonical registry loaded from guilds.json. Char.GuildID
	// is only a denormalized copy repaired at login.
	guilds         *model.GuildRegistry
	warConfig      model.GuildWarConfig
	warLocation    *time.Location
	warNextTick    time.Time
	warInitialized bool
	cityWarRunning bool
	cityFighters   map[*Player]cityFighter
	cityWarScores  [4]map[uint16]uint64 // frozen at deadline across persistence retries
	warNotices     []string
	warNoticeNext  time.Time
	// questsByNPC is the quest allowlist; NPCs absent here never offer quests.
	questFile   model.QuestFile
	initItems   []model.InitItem
	questsByNPC map[string]*model.QuestDef
	// questZones are rectangles that return every player inside to town each
	// reset cycle (WYD 7.48 ClearArea). nextQuestZoneReset uses wall-clock time,
	// not tick count, to preserve ten real minutes even after delayed ticks.
	questZones         []model.QuestZone
	nextQuestZoneReset time.Time
	uxmal              *model.VolatileInstance
	// channel is the native ServerIndex+1. Citizenship is per channel; another
	// channel's citizens receive no bonus here. A single instance uses channel 1.
	channel byte
	// Prevents informational packets not yet materialized from flooding logs
	// when clients send them every frame.
	lastProtocolNotice map[uint16]time.Time
	mobTickCounter     uint64
	// clock and rng provide time and randomness (clock.go). Production uses
	// real sources; tests inject controlled versions for deadlines and draws.
	clock Clock
	rng   RNG
	// shuttingDown is true after graceful shutdown (shutdown.go): state is
	// already persisted, so new input is rejected.
	shuttingDown bool
	// shutdownErr preserves the first drain result. Repeated requests cannot
	// turn a real persistence failure into apparent success.
	shutdownErr error
	// bosses holds extra boss behavior indexed by mob ID (boss.go). Every boss
	// also exists in mobs/mobsByID and participates in normal grid, visibility,
	// and combat handling. An empty map costs nothing for non-boss paths.
	bosses map[uint16]*BossRuntime
	// bossCatalog is loaded from data/boss/*.lua; bossSpawns holds live encounter
	// state (current instance and respawn deadline).
	bossCatalog model.BossCatalog
	bossSpawns  []*bossSpawnState
	// itemInstances are temporary rooms created by consumables. One World map
	// replaces per-room tickers and goroutines.
	itemInstances map[string]*ItemInstance
	// playerInstance is the O(1) index of the current private gameplay space.
	// RuntimeIDs are server-only state and never enter the wire protocol. Lookup
	// retains a fallback for fixtures that construct instances directly.
	playerInstance map[uint16]string
	// Stable character identities detached from a private Water instance while
	// their sockets are offline. World IDs are deliberately not persisted.
	pendingInstanceMembers map[string]map[string]struct{}
	pendingInstanceLeaders map[string]string
	nightmarePartyRuns     map[string]int
	// GambleJackpot/Pool are durable global aggregates owned by World so bets,
	// accounts, and pools share one writer and transaction.
	gambleJackpot      uint32
	gamblePool         uint64
	instanceStateDirty bool
}

// firstMobID starts the mob range; lower IDs belong to players.
const firstMobID = uint16(1000)

func isReservedNonPlayerEntityID(id uint16) bool {
	return (id >= 15001 && id <= 15100) ||
		(id > ghostShopIDBase && id < ghostShopIDBase+firstMobID)
}

// allocMobID reserves the next free mob ID.
//
// Two guards prevent previously observed failures:
//
//  1. Clamp before reservation so a zeroed counter never returns ID 0.
//  2. Skip live IDs after uint16 wraparound so respawns cannot overwrite mobs.
//
// Ground objects share the client's entity-ID namespace with mobs. Cannon IDs
// and virtual ghost-shop IDs stay reserved before those entities are spawned.
func (w *World) allocMobID() uint16 {
	if w.nextMobID < firstMobID {
		w.nextMobID = firstMobID
	}
	// Probe the mob range at most once.
	for attempts := 0; attempts <= int(^uint16(0)-firstMobID); attempts++ {
		id := w.nextMobID
		if w.nextMobID == ^uint16(0) {
			w.nextMobID = firstMobID
		} else {
			w.nextMobID++
		}
		if isReservedNonPlayerEntityID(id) {
			continue
		}
		if _, used := w.mobsByID[id]; used {
			continue
		}
		if _, used := w.groundItems[id]; !used {
			return id
		}
	}
	log.Printf("ERROR: mob ID range exhausted (%d live mobs, %d ground objects)", len(w.mobsByID), len(w.groundItems))
	return 0
}

// NewWorld creates the world and materializes static NPCs as live mobs.
// NPC ClientIds start at 1000; player IDs start at 1.
func NewWorld(st store.Store, npcs []model.NPCDef, geners []model.NPCGener, catalog model.Catalog, dropRates [model.MaxCarry]int, volatiles model.VolatileCatalog, characterTemplates model.CharacterTemplateFile, terrain model.TerrainMap, options ...WorldOption) (*World, error) {
	w := &World{
		players:                make(map[*net.Session]*Player),
		playersByID:            make(map[uint16]*Player),
		playersByCharacterUID:  make(map[string]*Player),
		playersByName:          make(map[string]*Player),
		accountSessions:        make(map[string]*net.Session),
		authClientsByIP:        make(map[string]map[*net.Session]struct{}),
		authClientsByNetwork:   make(map[string]map[*net.Session]struct{}),
		authPending:            make(map[*net.Session]bool),
		operational:            DefaultOperationalConfig(),
		authRateByIP:           make(map[string]*fixedWindowRate),
		authRateByAccount:      make(map[string]*fixedWindowRate),
		chatRateByAccount:      make(map[string]*fixedWindowRate),
		security:               make(map[*net.Session]*securityState),
		store:                  st,
		npcs:                   npcs,
		mobsByID:               make(map[uint16]*Mob),
		mobListIndex:           make(map[uint16]int),
		mobCells:               make(map[uint32]map[uint16]*Mob),
		playerCells:            make(map[uint32]map[uint16]*Player),
		mobCell:                make(map[uint16]uint32),
		playerCell:             make(map[uint16]uint32),
		activeMobs:             make(map[uint16]*Mob),
		summons:                make(map[uint16]*Mob),
		sephiraObjects:         make(map[uint16]*Mob),
		generatorByIndex:       make(map[int]int),
		nextMobID:              1000,
		items:                  catalog.Items,
		skills:                 catalog.Skills,
		groundItems:            make(map[uint16]*GroundItem),
		ghostShops:             make(map[uint16]*GhostShop),
		groundItemCells:        make(map[uint32]map[uint16]*GroundItem),
		groundItemCell:         make(map[uint16]uint32),
		groundExpiryByID:       make(map[uint16]time.Time),
		ghostShopCells:         make(map[uint32]map[uint16]*GhostShop),
		ghostShopCell:          make(map[uint16]uint32),
		nextItemID:             10000,
		dropRates:              dropRates,
		volatiles:              volatiles,
		charSpawn:              characterTemplates.Spawn,
		terrain:                terrain,
		npcGenerLogMode:        npcGenerLogSummary,
		gameplayLogMode:        gameplayLogSummary,
		channel:                1, // a single instance uses channel 1
		gameplay:               model.DefaultGameplayConfig(),
		lastProtocolNotice:     make(map[uint16]time.Time),
		clock:                  realClock{},
		rng:                    realRNG{},
		itemInstances:          make(map[string]*ItemInstance),
		playerInstance:         make(map[uint16]string),
		pendingInstanceMembers: make(map[string]map[string]struct{}),
		pendingInstanceLeaders: make(map[string]string),
		nightmarePartyRuns:     make(map[string]int),
	}
	for _, option := range options {
		option(w)
	}
	if w.warConfig.Enabled {
		if err := w.warConfig.Validate(); err != nil {
			return nil, fmt.Errorf("guild wars: %w", err)
		}
		w.warLocation, _ = time.LoadLocation(w.warConfig.Timezone)
	}
	if err := w.operational.Validate(); err != nil {
		return nil, fmt.Errorf("operational configuration: %w", err)
	}
	if w.networkAdmissionErr != nil {
		return nil, w.networkAdmissionErr
	}
	// No producer knows World before NewWorld returns. Instantiate the selected
	// operational capacities here instead of retaining hidden 1024/4 defaults.
	w.commands = make(chan command, w.operational.WorldCommandQueueCapacity)
	w.authSlots = make(chan struct{}, w.operational.AuthHashConcurrency)
	w.channel = w.operational.ChannelID
	// Initialize deadlines after options so an injected test clock starts them
	// at the same instant as the world.
	start := w.now()
	w.nextAutoSave = start.Add(accountAutoSaveSliceInterval)
	w.nextQuestZoneReset = start.Add(questZoneResetInterval)
	w.nextGameplayLog = start.Add(gameplayLogSummaryInterval)
	if err := w.gameplay.Validate(); err != nil {
		return nil, fmt.Errorf("global configuration: %w", err)
	}
	if w.uxmal != nil {
		if err := w.validateUxmalConfig(); err != nil {
			return nil, fmt.Errorf("Uxmal configuration: %w", err)
		}
	}
	// NPC Inventory is a STRUCT_ITEM blueprint. Fill quantities omitted in
	// converted JSON from itemlist before sending the first ShopList, so display
	// and purchase use the same state.
	if err := w.initShopItemDefaults(); err != nil {
		return nil, fmt.Errorf("initialize NPC stock: %w", err)
	}
	// Shop mounts start alive (HP, food, lifespan) so they are not sold dead.
	w.initShopMounts()
	// A store without guild support (or with no file) yields an empty registry.
	// The server starts and guild commands reject with a message.
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	if gs, ok := st.(guildStore); ok {
		registry, err := gs.LoadGuilds()
		if err != nil {
			return nil, fmt.Errorf("load guilds: %w", err)
		}
		w.guilds = registry
	}
	// In-memory character-name index prevents 0x20F scan amplification. A store
	// without index support leaves charNames nil and falls back to a disk scan.
	if namer, ok := st.(interface {
		CharacterNames() (map[string]struct{}, error)
	}); ok {
		names, err := namer.CharacterNames()
		if err != nil {
			return nil, fmt.Errorf("index character names: %w", err)
		}
		w.charNames = names
	}
	// A quest targeting a missing NPC or a type with its own handler fails
	// startup; such a quest could never trigger, so silent misconfiguration is
	// worse than a startup failure.
	questIndex, err := indexQuests(w.questFile, w.npcs)
	if err != nil {
		return nil, fmt.Errorf("quests: %w", err)
	}
	w.questsByNPC = questIndex
	if err := w.spawnInitItems(); err != nil {
		return nil, fmt.Errorf("world objects: %w", err)
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
	if err := w.validateItemInstanceTemplates(); err != nil {
		return nil, err
	}
	for _, g := range geners {
		if !g.Enabled {
			continue
		}
		// Item-activated rooms cannot share the permanent NPCGener population.
		// The WYD 7.48 file also contains Water generators with MinuteGenerate=-1
		// (one spawn at startup); retaining them would spawn monsters before the
		// ticket and duplicate them when the instance opens. Reserve generators
		// from authoritative item configuration, not a decorative NPCGener index.
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
			return nil, fmt.Errorf("NPCGener[%d]: missing template (Leader=%q Follower=%q)", g.Index, g.Leader, g.Follower)
		}
		w.generatorByIndex[g.Index] = len(w.generators)
		w.generators = append(w.generators, generState{def: g, leader: leader, follower: follower})
	}
	now := w.now()
	for i := range w.generators {
		w.spawnGroup(&w.generators[i]) // first call matches GenerateMob at startup
		w.scheduleGenerator(&w.generators[i], now)
	}
	w.flushNPCGenerLog(now, true)
	w.flushGameplayLog(now, true)
	// Bosses do not adopt NPCGener mobs: they spawn from their own catalog
	// (data/boss/*.lua), with separate positions and respawn behavior.
	if err := w.spawnConfiguredBosses(); err != nil {
		return nil, err
	}
	if err := w.restoreInstanceState(); err != nil {
		return nil, fmt.Errorf("restore instance state: %w", err)
	}
	return w, nil
}

func generName(s string) string { return strings.ReplaceAll(s, "_", " ") }

// allocPlayerID returns the lowest free ClientId from 1. Players use IDs below
// 1000; mobs start at 1000. Reusing a slot matches native TMSrv connection
// indexing, allowing a 7.48 relog to retain the same ID. If all 999 slots are
// occupied, it returns false instead of overwriting a visible player.
func (w *World) allocPlayerID() (uint16, bool) {
	var used [1000]bool
	// playersByID is authoritative for combat/visibility lookups. Check both
	// maps so a disconnect record awaiting cleanup cannot cause a collision.
	for id := range w.playersByID {
		if id > 0 && id < uint16(len(used)) {
			used[id] = true
		}
	}
	for _, p := range w.players {
		if p != nil && p.ID > 0 && p.ID < uint16(len(used)) {
			used[p.ID] = true
		}
	}
	for id := uint16(1); id < 1000; id++ {
		if !used[id] {
			return id, true
		}
	}
	return 0, false // world full (player limit)
}

func (w *World) scheduleGenerator(g *generState, now time.Time) {
	if g.def.MinuteGenerate > 0 {
		g.nextGenerate = now.Add(time.Duration(g.def.MinuteGenerate) * npcGenerMinute)
	}
}

// spawnGroup ports GenerateMob: a leader and MinGroup..MaxGroup followers,
// capped by MaxNumMob. StartRange scatters them around Start.
func (w *World) spawnGroup(g *generState) {
	remaining := g.def.MaxNumMob - g.current
	if remaining <= 0 {
		return
	}
	followers := g.def.MinGroup
	if d := g.def.MaxGroup - g.def.MinGroup + 1; d > 1 {
		followers += w.intn(d)
	}
	total := 1 + followers
	if total > remaining {
		total = remaining
	}
	leaderID := uint16(0)
	spawned := 0
	for i := 0; i < total; i++ {
		def := g.leader
		if i > 0 {
			def = g.follower
		}
		pos := g.def.Segments[0]
		requestedX, requestedY := w.scatter(pos.X, pos.Y, pos.Range)
		x, y := requestedX, requestedY
		x, y = w.findFreePosition(x, y, pos.Range)
		if x != requestedX || y != requestedY {
			w.npcGenerLog.relocations++
			if w.npcGenerLogMode == npcGenerLogVerbose {
				log.Printf("NPCGener[%d]: spawn %q relocated (%d,%d)->(%d,%d): blocked/occupied terrain",
					g.def.Index, def.Name, requestedX, requestedY, x, y)
			}
		}
		segments := g.def.Segments
		for si := range segments {
			if segments[si].X != 0 && segments[si].Y != 0 {
				segments[si].X, segments[si].Y = w.scatter(segments[si].X, segments[si].Y, segments[si].Range)
				segments[si].X, segments[si].Y = w.findWalkablePosition(segments[si].X, segments[si].Y, segments[si].Range)
			}
		}
		mobID := w.allocMobID()
		if mobID == 0 {
			log.Printf("NPCGener[%d]: spawn stopped: mob ID range exhausted", g.def.Index)
			break
		}
		m := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: def.Score.MaxHP,
			GenerIndex: g.def.Index, Segments: segments, RouteType: g.def.RouteType,
			WaitUntil: w.now().Add(time.Duration(g.def.Segments[0].Wait) * time.Second)}
		if i == 0 {
			leaderID = m.ID
		} else {
			m.LeaderID = leaderID
		}
		w.appendMobInstance(m)
		g.current++
		spawned++
		w.publishMobSpawn(m)
	}
	if spawned == 0 {
		return
	}
	w.npcGenerLog.groups++
	w.npcGenerLog.mobs += spawned
	if w.npcGenerLogMode == npcGenerLogVerbose {
		log.Printf("NPCGener[%d]: group spawned (%d mobs, alive=%d/%d)",
			g.def.Index, spawned, g.current, g.def.MaxNumMob)
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
		phase := "periodic"
		if initial {
			phase = "initial"
		}
		log.Printf("NPCGener %s summary: generators=%d groups=%d mobs=%d relocated=%d alive=%d",
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
	if m == nil || !w.terrain.RouteHeightCompatible(fromX, fromY, toX, toY) {
		return true
	}
	if w.positionOccupiedInGameplaySpace(toX, toY, mobGameplaySpace(m), m, nil, nil) {
		return true
	}
	// Instances are physically close to other map regions. Without this guard,
	// a mob could pursue an external player and escape a private room.
	if m.InstanceID != "" && !w.instanceMobStepAllowed(m, toX, toY) {
		// If an earlier version left the entity outside the boundary, allow only
		// steps toward the center so it can return; lateral/outward steps cannot
		// be used to pursue another player.
		stage, ok := instanceStageForMob(w.instanceForMob(m))
		if !ok || chebyshev(toX, toY, stage.X, stage.Y) >=
			chebyshev(fromX, fromY, stage.X, stage.Y) {
			return true
		}
	}
	return false
}

func (w *World) positionOccupiedExcept(x, y uint16, exceptMob *Mob, exceptPlayer *Player) bool {
	return w.positionOccupiedExceptPlayers(x, y, exceptMob, exceptPlayer, nil)
}

// positionOccupiedInGameplaySpace is the single collision boundary for all
// dynamic gameplay entities. Terrain and global/static NPCs are shared by all
// spaces; players, hostile mobs and summons reserve tiles only in their own
// GameplaySpace. This is intentionally independent of Water/Cube mechanics.
func (w *World) positionOccupiedInGameplaySpace(x, y uint16, space string,
	exceptMob *Mob, exceptPlayer *Player, ignored map[*Player]struct{},
) bool {
	if !w.terrain.Walkable(x, y) {
		return true
	}
	space = strings.TrimSpace(space)
	key := spatialKey(x, y)
	for _, m := range w.mobCells[key] {
		if m == exceptMob || m.Dead {
			continue
		}
		if m.X != x || m.Y != y {
			continue
		}
		dynamic := m.Def == nil || strings.TrimSpace(m.InstanceID) != "" ||
			m.SummonerID != 0 || m.Def.IsMonster()
		if dynamic && mobGameplaySpace(m) == space {
			return true
		}
		if !dynamic && strings.TrimSpace(m.InstanceID) == "" {
			return true
		}
	}
	for _, p := range w.playerCells[key] {
		if p == exceptPlayer {
			continue
		}
		if _, skip := ignored[p]; skip {
			continue
		}
		if p == nil || !p.InWorld || p.Char == nil || playerCurHP(p.Char) == 0 ||
			p.X != x || p.Y != y {
			continue
		}
		if w.gameplaySpaceForPlayer(p) != space {
			continue
		}
		return true
	}
	for _, shop := range w.ghostShopCells[key] {
		if shop.X == x && shop.Y == y {
			return true
		}
	}
	return false
}

// setPlayerInstanceIndex records the current server-side runtime membership.
// A player belongs to at most one runtime; replacing the value is intentional
// during a committed Water chain transition.
func (w *World) setPlayerInstanceIndex(playerID uint16, runtimeID string) {
	if w == nil || playerID == 0 {
		return
	}
	if w.playerInstance == nil {
		w.playerInstance = make(map[uint16]string)
	}
	if strings.TrimSpace(runtimeID) == "" {
		delete(w.playerInstance, playerID)
		return
	}
	w.playerInstance[playerID] = runtimeID
}

func (w *World) clearPlayerInstanceIndex(playerID uint16, runtimeID string) {
	if w == nil || w.playerInstance == nil || playerID == 0 {
		return
	}
	if runtimeID == "" || w.playerInstance[playerID] == runtimeID {
		delete(w.playerInstance, playerID)
	}
}

// rebuildPlayerInstanceIndex repairs the index after restore and is also used
// by rollback paths. It keeps the map derived from the authoritative instance
// membership rather than trusting process-local player IDs from a snapshot.
func (w *World) rebuildPlayerInstanceIndex() {
	if w == nil {
		return
	}
	if w.playerInstance == nil {
		w.playerInstance = make(map[uint16]string)
	}
	for id := range w.playerInstance {
		delete(w.playerInstance, id)
	}
	now := w.now()
	// Prefer an active runtime if a legacy/failed snapshot temporarily contains
	// a duplicate membership; exit-grace records are only fallback candidates.
	grace := make(map[uint16]string)
	for runtimeID, inst := range w.itemInstances {
		if inst == nil {
			continue
		}
		for _, playerID := range inst.MemberIDs {
			if !itemInstanceInExitGraceAt(inst, now) {
				w.playerInstance[playerID] = runtimeID
			} else if _, active := w.playerInstance[playerID]; !active {
				grace[playerID] = runtimeID
			}
		}
	}
	for playerID, runtimeID := range grace {
		if _, active := w.playerInstance[playerID]; !active {
			w.playerInstance[playerID] = runtimeID
		}
	}
}

// playerRuntimeInstanceID returns the live ownership record for collision
// filtering. The indexed path is O(1); the scan is only a repair path for
// legacy fixtures or a caller that directly restored an ItemInstance.
// Exit-grace membership remains visible to collision allocation until a chain
// transition removes it atomically.
func (w *World) playerRuntimeInstanceID(playerID uint16) string {
	if w == nil || playerID == 0 {
		return ""
	}
	if w.playerInstance != nil {
		if runtimeID := w.playerInstance[playerID]; runtimeID != "" {
			if inst := w.itemInstances[runtimeID]; inst != nil && itemInstanceHasMember(inst, playerID) {
				return runtimeID
			}
			delete(w.playerInstance, playerID)
		}
	}
	var graceRuntime string
	now := w.now()
	for runtimeID, inst := range w.itemInstances {
		if inst == nil || !itemInstanceHasMember(inst, playerID) {
			continue
		}
		if !itemInstanceInExitGraceAt(inst, now) {
			w.setPlayerInstanceIndex(playerID, runtimeID)
			return runtimeID
		}
		if graceRuntime == "" {
			graceRuntime = runtimeID
		}
	}
	if graceRuntime != "" {
		w.setPlayerInstanceIndex(playerID, graceRuntime)
	}
	return graceRuntime
}

// positionOccupiedExceptPlayers handles atomic party movement. All ignored
// players may leave their old tiles together without ignoring nonparticipants.
func (w *World) positionOccupiedExceptPlayers(x, y uint16, exceptMob *Mob,
	exceptPlayer *Player, ignored map[*Player]struct{}) bool {
	return w.positionOccupiedInGameplaySpace(x, y, "", exceptMob, exceptPlayer, ignored)
}

// findFreePlayerPosition tries adjacent tiles first, then expands in rings.
// It ignores the player, who may already occupy the recall tile while reviving.
func (w *World) findFreePlayerPosition(x, y uint16, radius int, player *Player) (uint16, uint16) {
	if !w.positionOccupiedExcept(x, y, nil, player) {
		return x, y
	}
	if radius < 1 {
		radius = 1
	}
	for distance := 1; distance <= radius; distance++ {
		// Prefer cardinal directions, then diagonals and the rest of the ring.
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

// findFreePlayerPositionInInstance allocates a reconnect/recall tile against
// the occupancy of the same private runtime. Players and mobs from another
// phased runtime are not physical blockers, while terrain and global shops
// remain authoritative blockers.
func (w *World) findFreePlayerPositionInInstance(x, y uint16, radius int,
	player *Player, instanceID string) (uint16, uint16) {
	occupied := func(px, py uint16) bool {
		return w.positionOccupiedInGameplaySpace(px, py, instanceID, nil, player, nil)
	}
	if !occupied(x, y) {
		return x, y
	}
	if radius < 1 {
		radius = 1
	}
	for distance := 1; distance <= radius; distance++ {
		offsets := [][2]int{{distance, 0}, {-distance, 0}, {0, distance}, {0, -distance}}
		for dy := -distance; dy <= distance; dy++ {
			for dx := -distance; dx <= distance; dx++ {
				if (dx == 0 && absInt(dy) == distance) ||
					(dy == 0 && absInt(dx) == distance) ||
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
			ux, uy := uint16(nx), uint16(ny)
			if w.terrain.HeightCompatible(x, y, ux, uy) && !occupied(ux, uy) {
				return ux, uy
			}
		}
	}
	return x, y
}

func (w *World) findFreeGameplayPosition(spaceOwner, exceptPlayer *Player,
	x, y uint16, radius uint16) (uint16, uint16) {
	if spaceOwner != nil {
		// Summons and player pulls also occur in shared instances. Use the
		// complete server-side membership here, not only private gameplay
		// spaces, so one shared runtime cannot collide with another runtime.
		if runtimeID := w.playerRuntimeInstanceID(spaceOwner.ID); runtimeID != "" {
			return w.findFreePlayerPositionInInstance(x, y, int(radius), exceptPlayer, runtimeID)
		}
	}
	return w.findFreePositionExcept(x, y, radius, exceptPlayer)
}

// findFreeMobPosition allocates a tile in the mob's gameplay space before the
// new entity is registered in the spatial index.
func (w *World) findFreeMobPosition(instanceID string, x, y uint16, radius uint16) (uint16, uint16) {
	instanceID = strings.TrimSpace(instanceID)
	occupied := func(px, py uint16) bool {
		return w.positionOccupiedInGameplaySpace(px, py, instanceID, nil, nil, nil)
	}
	if !occupied(x, y) {
		return x, y
	}
	r := int(radius)
	if r < 2 {
		r = 2
	}
	for distance := 1; distance <= r+4; distance++ {
		for dy := -distance; dy <= distance; dy++ {
			for dx := -distance; dx <= distance; dx++ {
				if absInt(dx) != distance && absInt(dy) != distance {
					continue
				}
				nx, ny := int(x)+dx, int(y)+dy
				if nx <= 0 || ny <= 0 || nx >= model.TerrainWidth || ny >= model.TerrainHeight {
					continue
				}
				ux, uy := uint16(nx), uint16(ny)
				if w.terrain.HeightCompatible(x, y, ux, uy) && !occupied(ux, uy) {
					return ux, uy
				}
			}
		}
	}
	return x, y
}

// removeMobInstance drops a dead mob from the active list after RemoveMob was
// sent. Retaining it would increase linear scans and leave old groups present
// after many respawns.
func (w *World) removeMobInstance(dead *Mob) {
	if dead == nil {
		return
	}
	if w.mobListIndex == nil {
		w.mobListIndex = make(map[uint16]int)
	}
	index, found := w.mobListIndex[dead.ID]
	if !found || index < 0 || index >= len(w.mobs) || w.mobs[index] != dead {
		// Compatibility with old fixtures/imports that construct the list
		// directly. Live spawns use appendMobInstance and remain O(1).
		for i, mob := range w.mobs {
			if mob == dead {
				index, found = i, true
				break
			}
		}
	}
	w.unregisterMobSpatial(dead)
	delete(w.mobListIndex, dead.ID)
	if !found {
		return
	}
	last := len(w.mobs) - 1
	if index != last {
		moved := w.mobs[last]
		w.mobs[index] = moved
		if moved != nil {
			w.mobListIndex[moved.ID] = index
		}
	}
	w.mobs[last] = nil
	w.mobs = w.mobs[:last]
}

func (w *World) appendMobInstance(mob *Mob) {
	if mob == nil {
		return
	}
	if w.mobListIndex == nil {
		w.mobListIndex = make(map[uint16]int)
	}
	w.mobListIndex[mob.ID] = len(w.mobs)
	w.mobs = append(w.mobs, mob)
}

func (w *World) findFreePosition(x, y, radius uint16) (uint16, uint16) {
	return w.findFreePositionExcept(x, y, radius, nil)
}

func (w *World) findFreePositionExcept(x, y, radius uint16, exceptPlayer *Player) (uint16, uint16) {
	if !w.positionOccupiedExcept(x, y, nil, exceptPlayer) {
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
					!w.positionOccupiedExcept(uint16(nx), uint16(ny), nil, exceptPlayer) {
					return uint16(nx), uint16(ny)
				}
			}
		}
	}
	return x, y
}

// findWalkablePosition corrects NPCGener destinations without occupancy checks:
// multiple mobs may share a segment, but never a blocked terrain cell (127).
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

func (w *World) scatter(x, y, radius uint16) (uint16, uint16) {
	if radius == 0 {
		return x, y
	}
	r := int(radius)
	dx, dy := w.intn(2*r+1)-r, w.intn(2*r+1)-r
	nx, ny := int(x)+dx, int(y)+dy
	if nx < 1 {
		nx = 1
	}
	if ny < 1 {
		ny = 1
	}
	return uint16(nx), uint16(ny)
}

// Enqueue sends a packet (or disconnect) to the loop from a session goroutine.
// It blocks only if the buffer fills, applying backpressure to that client.
func (w *World) Enqueue(s *net.Session, pkt []byte) {
	w.commands <- command{s: s, pkt: pkt, queuedAt: time.Now()}
}

// worldTickInterval is native TIMER_SEC. Heavy AI work is sharded inside tick()
// like TMSrv (combat %4; idle/routes/affects %6), avoiding five full mob scans
// per second.
const worldTickInterval = 500 * time.Millisecond

const (
	// Short batches prevent one session burst from delaying a tick. Round-robin
	// processing advances commands from all clients without dropping valid packets.
	worldCommandBatchLimit = 256
	worldCommandBudget     = 5 * time.Millisecond
)

// Run processes client commands and game ticks sequentially.
func (w *World) Run() {
	ticker := time.NewTicker(worldTickInterval)
	defer ticker.Stop()
	for {
		cmd, ok := w.popPendingCommand()
		if !ok {
			select {
			case cmd = <-w.commands:
			case <-ticker.C:
				w.tick()
				continue
			}
		}
		w.processCommandBatch(cmd, ticker.C)
	}
}

func (w *World) pendingCommandCount() int {
	if w.pendingCommandHead >= len(w.pendingCommands) {
		return 0
	}
	return len(w.pendingCommands) - w.pendingCommandHead
}

func (w *World) commandQueueDepth() int {
	return len(w.commands) + w.pendingCommandCount()
}

func (w *World) popPendingCommand() (command, bool) {
	if w.pendingCommandHead >= len(w.pendingCommands) {
		if len(w.pendingCommands) != 0 || w.pendingCommandHead != 0 {
			clear(w.pendingCommands)
			w.pendingCommands = w.pendingCommands[:0]
			w.pendingCommandHead = 0
		}
		return command{}, false
	}
	idx := w.pendingCommandHead
	cmd := w.pendingCommands[idx]
	w.pendingCommands[idx] = command{}
	w.pendingCommandHead++
	if w.pendingCommandHead == len(w.pendingCommands) {
		w.pendingCommands = w.pendingCommands[:0]
		w.pendingCommandHead = 0
	}
	return cmd, true
}

func (w *World) compactPendingCommands() {
	if w.pendingCommandHead == 0 {
		return
	}
	if w.pendingCommandHead >= len(w.pendingCommands) {
		clear(w.pendingCommands)
		w.pendingCommands = w.pendingCommands[:0]
		w.pendingCommandHead = 0
		return
	}
	live := copy(w.pendingCommands, w.pendingCommands[w.pendingCommandHead:])
	clear(w.pendingCommands[live:])
	w.pendingCommands = w.pendingCommands[:live]
	w.pendingCommandHead = 0
}

func (w *World) reservePendingCommandFront(count int) []command {
	if count <= 0 {
		return nil
	}
	w.compactPendingCommands()
	oldLen := len(w.pendingCommands)
	total := oldLen + count
	if cap(w.pendingCommands) < total {
		next := make([]command, total, total+worldCommandBatchLimit)
		copy(next[count:], w.pendingCommands)
		clear(w.pendingCommands)
		w.pendingCommands = next
	} else {
		w.pendingCommands = w.pendingCommands[:total]
		copy(w.pendingCommands[count:], w.pendingCommands[:oldLen])
	}
	return w.pendingCommands[:count]
}

func (w *World) prependPendingCommands(commands []command) {
	if len(commands) == 0 {
		return
	}
	front := w.reservePendingCommandFront(len(commands))
	copy(front, commands)
}

func (w *World) prepareCommandBatchQueues(batch []command) []*net.Session {
	if w.commandBatchQueues == nil {
		w.commandBatchQueues = make(map[*net.Session]commandBatchQueue)
	} else {
		clear(w.commandBatchQueues)
	}
	order := w.commandBatchOrderScratch[:0]
	for i := range batch {
		idx := int16(i)
		w.commandBatchNext[i] = commandBatchEnd
		queue, exists := w.commandBatchQueues[batch[i].s]
		if !exists {
			order = append(order, batch[i].s)
			queue = commandBatchQueue{head: idx, tail: idx}
		} else {
			w.commandBatchNext[queue.tail] = idx
			queue.tail = idx
		}
		w.commandBatchQueues[batch[i].s] = queue
	}
	w.commandBatchOrderScratch = order
	return order
}

func (w *World) popPreparedCommand(batch []command, session *net.Session) (command, bool) {
	queue, exists := w.commandBatchQueues[session]
	if !exists || queue.head == commandBatchEnd {
		return command{}, false
	}
	idx := queue.head
	queue.head = w.commandBatchNext[idx]
	if queue.head == commandBatchEnd {
		queue.tail = commandBatchEnd
	}
	w.commandBatchQueues[session] = queue
	cmd := batch[idx]
	batch[idx] = command{}
	return cmd, true
}

func (w *World) requeuePreparedCommandBatch(batch []command, order []*net.Session) {
	remaining := 0
	for _, session := range order {
		queue := w.commandBatchQueues[session]
		for idx := queue.head; idx != commandBatchEnd; idx = w.commandBatchNext[idx] {
			remaining++
		}
	}
	if remaining == 0 {
		return
	}
	front := w.reservePendingCommandFront(remaining)
	pos := 0
	for _, session := range order {
		queue := w.commandBatchQueues[session]
		for idx := queue.head; idx != commandBatchEnd; idx = w.commandBatchNext[idx] {
			front[pos] = batch[idx]
			pos++
		}
	}
}

func (w *World) releaseCommandBatchScratch(batch []command, order []*net.Session) {
	clear(batch)
	w.commandBatchScratch = batch[:0]
	for i := range order {
		order[i] = nil
	}
	w.commandBatchOrderScratch = order[:0]
	clear(w.commandBatchQueues)
}

// processCommandBatch drains a small command window in per-session round-robin
// order. One connection cannot monopolize World with movements, and a tick
// can interrupt the batch without losing remaining commands.
func (w *World) processCommandBatch(first command, ticks <-chan time.Time) {
	batch := w.commandBatchScratch[:0]
	batch = append(batch, first)
	deadline := time.Now().Add(worldCommandBudget)
	for len(batch) < worldCommandBatchLimit && time.Now().Before(deadline) {
		// Deferred commands predate those still in the channel. Drain them first;
		// otherwise sustained load advances the old backlog only one command per
		// batch and favors the client still flooding the channel.
		if pending, ok := w.popPendingCommand(); ok {
			batch = append(batch, pending)
			continue
		}
		select {
		case <-ticks:
			w.prependPendingCommands(batch)
			observeCommandBatch(len(batch), true)
			w.releaseCommandBatchScratch(batch, nil)
			w.tick()
			return
		case cmd := <-w.commands:
			batch = append(batch, cmd)
		default:
			// No more ready commands; execute the collected batch.
			goto execute
		}
	}

execute:
	// Each session is an index chain inside the batch. World reuses map/order/next
	// across batches without materializing a separate []command per session.
	order := w.prepareCommandBatchQueues(batch)
	for {
		progress := false
		for _, session := range order {
			select {
			case <-ticks:
				w.requeuePreparedCommandBatch(batch, order)
				observeCommandBatch(len(batch), true)
				w.releaseCommandBatchScratch(batch, order)
				w.tick()
				return
			default:
			}
			cmd, ok := w.popPreparedCommand(batch, session)
			if !ok {
				continue
			}
			w.safeHandle(cmd)
			progress = true
			if time.Now().After(deadline) {
				w.requeuePreparedCommandBatch(batch, order)
				observeCommandBatch(len(batch), true)
				w.releaseCommandBatchScratch(batch, order)
				return
			}
		}
		if !progress {
			break
		}
	}
	observeCommandBatch(len(batch), false)
	w.releaseCommandBatchScratch(batch, order)
}

// commandLabel returns a bounded metric label: the opcode, never a session or
// player identifier that would explode cardinality.
//
// It checks bounds because wire.ParseHeader indexes 12 bytes without a length
// check, and this label is calculated outside safeHandle's recovery. A short
// packet must not crash the game loop's error-containment path.
func commandLabel(cmd command) string {
	if cmd.bosses != nil {
		return "control.bosses"
	}
	if cmd.kick != nil {
		return "control.kick"
	}
	if cmd.teleport != nil {
		return "control.teleport"
	}
	if cmd.quiz != nil {
		return "control.quiz"
	}
	if cmd.globalDrop != nil {
		return "control.global-drop"
	}
	if cmd.control != nil {
		return "control.read"
	}
	if cmd.pkt == nil {
		return "login" // internal authentication-result command
	}
	if len(cmd.pkt) < wire.HeaderSize {
		return "malformed"
	}
	opcode := wire.ParseHeader(cmd.pkt).Type
	if label, ok := inboundCommandMetricLabel(opcode); ok {
		return label
	}
	return "unknown"
}

// safeHandle isolates handler panics. World runs in one goroutine, so an
// unhandled panic would terminate the process. Recovery logs stack/opcode and
// enters fail-closed maintenance; potentially partial state is never saved.
func (w *World) safeHandle(cmd command) {
	label := commandLabel(cmd)
	start := time.Now()
	if !cmd.queuedAt.IsZero() {
		observeCommandQueueAge(start.Sub(cmd.queuedAt))
	}
	defer func() {
		if r := recover(); r != nil {
			var id int64
			if cmd.s != nil {
				id = cmd.s.ID
			}
			metricPanicsTotal.Add(1)
			log.Printf("[#%d] handler PANIC (Type=%s): %v\n%s", id, label, r, debug.Stack())
			w.failClosedAfterHandlerPanic()
		}
		// Measure in defer so a panicking command also appears in duration data.
		observeCommand(label, time.Since(start))
	}()
	w.handle(cmd)
}

// failClosedAfterHandlerPanic protects persistence. A handler can affect
// multiple players (trade, party, PvP), so poisoning only its source session
// is insufficient. All online snapshots become unsavable and sockets close;
// the process remains in maintenance for restart from the last sound commit.
func (w *World) failClosedAfterHandlerPanic() {
	if w == nil {
		return
	}
	w.shuttingDown = true
	log.Printf("CRITICAL: world entered maintenance after panic; RAM snapshots will not be persisted")
	for _, p := range w.players {
		if p == nil {
			continue
		}
		p.PersistencePoisoned = true
		if p.Session != nil {
			p.Session.Close()
		}
	}
}

// poisonAccountsAfterPersistenceFailure prevents a later save from partially
// committing a failed economic operation. The database stays authoritative;
// affected clients reconnect at the last valid commit.
func (w *World) poisonAccountsAfterPersistenceFailure(accounts []*model.Account, operation string, err error) {
	if w == nil || len(accounts) == 0 {
		return
	}
	affected := make(map[*model.Account]struct{}, len(accounts))
	for _, account := range accounts {
		if account != nil {
			affected[account] = struct{}{}
		}
	}
	for _, p := range w.players {
		if p == nil {
			continue
		}
		if _, ok := affected[p.Account]; !ok {
			continue
		}
		p.PersistencePoisoned = true
		if p.Session != nil {
			p.Session.Close()
		}
	}
	log.Printf("CRITICAL: %s was not persisted (%v); %d account(s) isolated", operation, err, len(affected))
}

// tick replenishes NPCGener groups at MinuteGenerate intervals (12-second
// ticks) while MaxNumMob allows. Values -1/0 are startup/manual spawns only.
func (w *World) tick() {
	now := w.now()
	// Instrumentation uses real time to measure execution cost, not game time;
	// a fake test clock must not distort observed duration.
	tickStart := time.Now()
	defer func() {
		observeTick(tickStart, time.Since(tickStart))
		observeWorldGauges(len(w.players), len(w.activeMobs))
		metricCommandQueueDepth.Set(int64(w.commandQueueDepth()))
	}()
	w.mobTickCounter++
	// Server position follows the visual route, but only due steps become
	// authoritative for AI and interactions.
	w.advanceAllPlayerMovement(now)
	w.tickGlobalDrop(now)
	w.tickGuildWars(now)
	// The grid limits this list to mobs near players. An active mob senses a
	// target every second. Pursuit and patrol start new legs every two seconds
	// to avoid chaining animations at maximum speed; execution stays at 500 ms
	// to honor the 1.5-second cooldown.
	if w.mobTickCounter%2 == 0 {
		allowMovement := w.mobTickCounter%4 == 0
		w.tickMobCombat(now, 0, 1, allowMovement)
		if allowMovement {
			w.tickMobRoutes(now, 0, 1)
		}
	}
	w.tickActiveMobActions(now)
	// Boss actions use the world clock, with no per-boss ticker.
	w.tickBossActions(now)
	w.tickBossRespawns(now)
	w.tickSummonCombat(now)
	w.tickSephiraObjects(now)
	// Timed equipment expires before affects/recalculation can consume its
	// bonuses during this logical tick.
	w.tickTimedItems(now)
	w.tickPlayerAffects(now)
	w.tickMobAffects(now, int(w.mobTickCounter%6), 6)
	w.tickPlayerRegen(now)
	w.tickChaosRecovery(now)
	w.tickPlayerMounts(now)
	w.tickGroundItems(now)
	w.tickItemInstances(now)
	w.tickTrades(now)
	w.tickClientIntegrity(now)
	w.tickQuiz(now)
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
	w.flushGameplayLog(now, false)
}

// tickQuestZoneReset ports WYD 7.48 ClearArea: every ten-minute cycle returns
// players in quest zones to town. This is one global deadline, not a full
// ten-minute interval granted separately to each player.
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

// accountAutoSaveBucket assigns accounts to one of six 500 ms slices. Its
// case-insensitive hash allocates nothing and is neither persisted nor sent.
func accountAutoSaveBucket(name string) uint8 {
	const (
		offset32 = uint32(2166136261)
		prime32  = uint32(16777619)
	)
	hash := offset32
	for i := 0; i < len(name); i++ {
		b := name[i]
		if b >= 'A' && b <= 'Z' {
			b += 'a' - 'A'
		}
		hash ^= uint32(b)
		hash *= prime32
	}
	return uint8(hash % uint32(accountAutoSaveBuckets))
}

func (w *World) autoSaveAccounts(now time.Time) {
	bucket := w.autoSaveBucket
	w.autoSaveBucket++
	if w.autoSaveBucket >= uint8(accountAutoSaveBuckets) {
		w.autoSaveBucket = 0
	}
	w.nextAutoSave = now.Add(accountAutoSaveSliceInterval)

	// World reuses this slice after its first growth instead of allocating a
	// new []Player for every slice.
	active := w.autoSaveScratch[:0]
	for _, p := range w.players {
		if !p.InWorld || p.Account == nil || p.Char == nil || p.PersistencePoisoned ||
			accountAutoSaveBucket(p.Account.Name) != bucket {
			continue
		}
		active = append(active, p)
		// Periodic autosave is not a confirmation gate. It uses the asynchronous
		// path to move fsync off the game loop; anti-duplication saves stay sync.
		if err := w.saveAccountAsync(p.Account); err != nil {
			log.Printf("[#%d] ERROR autosaving account %q: %v", p.Session.ID, p.Account.Name, err)
		}
	}
	// Queue character states after accounts in the same slice so the worker
	// consolidates adjacent snapshots without putting all players on one tick.
	for _, p := range active {
		w.saveCharStateAsync(p)
	}
	for i := range active {
		active[i] = nil
	}
	w.autoSaveScratch = active[:0]

	// Global instance state retains its previous three-second cadence.
	if w.autoSaveBucket == 0 {
		w.flushInstanceStateIfDirty()
	}
}

// broadcast sends a packet to each world player. Call the builder per player
// because Send encrypts in place and requires a distinct []byte.
func (w *World) broadcast(build func() []byte) {
	for _, p := range w.players {
		if p.InWorld {
			p.Session.Send(build())
		}
	}
}

// handle dispatches a command by header Type.
func (w *World) handle(cmd command) {
	if cmd.kick != nil {
		w.handleKick(cmd.kick)
		return
	}
	if cmd.teleport != nil {
		w.handleTeleport(cmd.teleport)
		return
	}
	if cmd.bosses != nil {
		w.handleBosses(cmd.bosses)
		return
	}
	if cmd.quiz != nil {
		w.handleQuiz(cmd.quiz)
		return
	}
	if cmd.globalDrop != nil {
		w.handleGlobalDrop(cmd.globalDrop)
		return
	}
	if cmd.control != nil {
		w.handleControl(cmd.control)
		return
	}
	if cmd.accountPresence != nil {
		w.handleAccountPresence(cmd.accountPresence)
		return
	}
	if cmd.shutdown != nil {
		w.runShutdown(cmd.shutdown)
		return
	}
	// No packet may change the world after the final snapshot or a fail-closed
	// panic. A round-robin batch may contain more commands after Shutdown;
	// without this barrier they would run after the final commit and vanish.
	if w.shuttingDown {
		if cmd.pkt == nil && cmd.s != nil {
			w.onDisconnect(cmd.s)
		} else if cmd.s != nil {
			cmd.s.Close()
		}
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
	if p := w.players[cmd.s]; p != nil && p.InWorld && p.Char != nil {
		w.advancePlayerMovement(p, w.now())
	}
	h := wire.ParseHeader(cmd.pkt)
	if p := w.players[cmd.s]; p != nil && p.AirMoveActive {
		if p.Char == nil || playerCurHP(p.Char) == 0 {
			// Death ends travel before any gameplay gate. The client may still
			// emit animation completion after taking damage.
			clearAirMove(p)
		}
	}
	if p := w.players[cmd.s]; p != nil && p.AirMoveActive {
		// Flight remains visual until confirmation 0xAD9/mode=2. No other
		// intention may move or mutate the character in that interval.
		switch h.Type {
		case wire.OpAirMove, wire.OpPing, wire.OpSysQuit, wire.OpCharacterLogout,
			wire.OpClientIntegrityResponse:
		default:
			return
		}
	}
	switch int(h.Type) {
	case wire.OpConnectAccount:
		w.onLogin(cmd.s, cmd.pkt)
	case wire.OpCreateCharacter:
		w.onCreateCharacter(cmd.s, cmd.pkt)
	case wire.OpCharacterLogin:
		w.onEnterWorld(cmd.s, cmd.pkt)
	case wire.OpCharacterLogout:
		w.onCharacterLogout(cmd.s, cmd.pkt)
	case wire.OpClientIntegrityResponse:
		w.onClientIntegrityResponse(cmd.s, cmd.pkt)
	case wire.OpQuizAnswer:
		w.onQuizAnswer(cmd.s, cmd.pkt)
	case wire.OpDeleteCharacter:
		w.onDeleteCharacter(cmd.s, cmd.pkt)
	case wire.OpCharacterTransfer:
		w.onCharacterTransferUnavailable(cmd.s, cmd.pkt)
	case wire.OpSwapItem:
		w.onSwapItem(cmd.s, cmd.pkt)
	case wire.OpDeposit:
		w.onCargoGold(cmd.s, cmd.pkt, true)
	case wire.OpWithdraw:
		w.onCargoGold(cmd.s, cmd.pkt, false)
	case wire.OpUseItem:
		w.onUseItem(cmd.s, cmd.pkt)
	case wire.OpUsePremiumFirework:
		w.onUsePremiumFirework(cmd.s, cmd.pkt)
	case wire.OpCapsuleInfo:
		w.onCapsuleInfo(cmd.s, cmd.pkt)
	case wire.OpPutoutSeal:
		w.onPutoutSeal(cmd.s, cmd.pkt)
	case wire.OpUseNPC, wire.OpReqShopList:
		w.onUseNPC(cmd.s, cmd.pkt)
	case wire.OpBuyItem:
		w.onBuyItem(cmd.s, cmd.pkt)
	case wire.OpBuyToto:
		w.onBuyToto(cmd.s, cmd.pkt)
	case wire.OpDoJackpotBet:
		w.onDoJackpotBet(cmd.s, cmd.pkt)
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
	case wire.OpDeleteItem:
		w.onDeleteItem(cmd.s, cmd.pkt)
	case wire.OpSplitItem:
		w.onSplitItem(cmd.s, cmd.pkt)
	case wire.OpUpdateItem:
		w.onUpdateGroundItem(cmd.s, cmd.pkt)
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
	case wire.OpAirMove:
		w.onAirMove(cmd.s, cmd.pkt)
	case wire.OpPKMode:
		w.onPKMode(cmd.s, cmd.pkt)
	case wire.OpGuildDeprivate:
		w.onGuildDeprivate(cmd.s, cmd.pkt)
	case wire.OpInviteGuild:
		w.onInviteGuild(cmd.s, cmd.pkt)
	case wire.OpGuildAlly:
		w.onGuildAlly(cmd.s, cmd.pkt)
	case wire.OpGuildWar:
		w.onGuildWar(cmd.s, cmd.pkt)
	case wire.OpChallenge, wire.OpChallengeConfirm:
		w.onGuildChallenge(cmd.s, cmd.pkt)
	case wire.OpMoveStop:
		w.onMoveStop(cmd.s, cmd.pkt)
	case wire.OpRestart:
		w.onRestart(cmd.s)
	case wire.OpPing:
		w.onPing(cmd.s, cmd.pkt)
	case wire.OpUpdateScore:
		// The native client may emit 0x336, but WYD 7.48 discards it. Score
		// and affects remain exclusively server-authoritative.
	case wire.OpSysQuit:
		w.onSysQuit(cmd.s)
	case wire.OpAction, wire.OpIllusion:
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
		// 0x39D is 7.48 melee (confirmed in game: repeats about once per second).
		w.onAttack(cmd.s, cmd.pkt)
	case wire.OpPlayerChallenge:
		w.onPlayerChallenge(cmd.s, cmd.pkt)
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
	case wire.OpCombineExtracao:
		w.onCombineExtracao(cmd.s, cmd.pkt)
	case wire.OpCombineAlquimia:
		w.onCombineAlquimia(cmd.s, cmd.pkt)
	default:
		// validateInboundCommand makes this unreachable for network packets.
		// Fail closed for internally constructed/test commands without unbounded
		// per-opcode logging.
		w.recordSecurityViolation(cmd.s, h.Type, "registered opcode has no handler")
	}
}

func (w *World) createGroundDrop(x, y uint16, item model.Item, publish bool) *GroundItem {
	return w.createGroundDropForInstance(x, y, item, publish, "")
}

func (w *World) createGroundDropForInstance(x, y uint16, item model.Item,
	publish bool, instanceID string) *GroundItem {
	itemIndex := item.Index
	item, err := materializeItem(item)
	if err != nil {
		log.Printf("materialize drop item=%d: %v", itemIndex, err)
		return nil
	}
	// The client searches for Cannon (746) exactly under the player and only
	// at IDs 15001..15100. Other drops are scattered around their origin.
	dropX, dropY := x, y
	if item.Index != 746 {
		// Calculate as int: uint16(0)-1 wraps to 65535 and places an unreachable
		// item at the opposite map edge. Valid client tiles are 1..4095; retain
		// the authoritative origin if a scattered point lands on blocked terrain.
		nx := clampInt(int(x)+w.intn(3)-1, 1, model.TerrainWidth-1)
		ny := clampInt(int(y)+w.intn(3)-1, 1, model.TerrainHeight-1)
		candidateX, candidateY := uint16(nx), uint16(ny)
		if w.terrain.Walkable(candidateX, candidateY) {
			dropX, dropY = candidateX, candidateY
		}
	}

	if _, ok := w.items[item.Index]; !ok {
		log.Printf("attempted to drop unknown item: %d", item.Index)
		return nil
	}
	if w.groundItems == nil {
		w.groundItems = make(map[uint16]*GroundItem)
	}

	id, ok := w.allocGroundItemID(item.Index)
	if !ok {
		log.Printf("no free ID for drop item=%d", item.Index)
		return nil
	}

	gItem := &GroundItem{
		ID:         id,
		Item:       item,
		X:          dropX,
		Y:          dropY,
		Expire:     w.now().Add(2 * time.Minute),
		InstanceID: instanceID,
	}
	w.registerGroundItem(gItem)

	if publish {
		w.publishItemSpawn(gItem)
	}
	return gItem
}

// spawnInitItems places map fixtures before the world accepts connections.
// No player is connected yet; AOI publishes each object when a player arrives.
//
// Reuse allocGroundItemID so cannons (746) receive client-required IDs in
// 15001..15100. Permanent objects stay registered and cannot be overwritten
// by later drops.
func (w *World) spawnInitItems() error {
	if len(w.initItems) > 0 && w.terrain.Loaded() {
		w.terrain.Height = append([]byte(nil), w.terrain.Height...)
	}
	for _, obj := range w.initItems {
		id, ok := w.allocGroundItemID(obj.Index)
		if !ok {
			return fmt.Errorf("no free ID for object %d at (%d,%d)",
				obj.Index, obj.X, obj.Y)
		}
		item := &GroundItem{
			ID:        id,
			Item:      model.Item{Index: obj.Index},
			X:         obj.X,
			Y:         obj.Y,
			Rotate:    obj.Rotate,
			Permanent: true,
		}
		w.registerGroundItem(item)
		w.applyGroundItemHeight(item)
	}
	if len(w.initItems) > 0 {
		log.Printf("world objects: %d placed on the map", len(w.initItems))
	}
	return nil
}

func (w *World) allocGroundItemID(itemIndex uint16) (uint16, bool) {
	if itemIndex == 746 {
		for id := uint16(15001); id <= 15100; id++ {
			if _, used := w.groundItems[id]; used {
				continue
			}
			if _, used := w.mobsByID[id]; !used {
				return id, true
			}
		}
		return 0, false
	}
	// IDs below 1000 belong to players; 15001..15100 belong to cannons;
	// 25001..25999 are virtual ghost shops (25000 + player ID).
	// Probe the range at most once so exhaustion cannot spin forever.
	for attempts := 0; attempts < int(^uint16(0)); attempts++ {
		id := w.nextItemID
		if id == 0 {
			id = 1
		}
		if id == ^uint16(0) {
			w.nextItemID = 1
		} else {
			w.nextItemID = id + 1
		}
		if id < firstMobID || isReservedNonPlayerEntityID(id) {
			continue
		}
		if _, used := w.groundItems[id]; used {
			continue
		}
		if _, used := w.mobsByID[id]; !used {
			return id, true
		}
	}
	return 0, false
}

func (w *World) tickGroundItems(now time.Time) {
	w.expireGroundItems(now)
}
