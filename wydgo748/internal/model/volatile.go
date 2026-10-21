package model

import "time"

// VolatileRule describes a clickable item's authoritative server behavior.
// Item associations still come from itemlist.csv.
type VolatileRule struct {
	Action  string `json:"action"`
	Consume bool   `json:"consume"`
	// CustomPattern separates Premium FireCracker from ordinary EF_VOLATILE 19
	// fireworks: a 10x10 pattern in 0x3C9 versus random Motion 100.
	CustomPattern bool   `json:"customPattern,omitempty"`
	ValueSource   string `json:"valueSource,omitempty"`
	HP            int    `json:"hp,omitempty"`
	MP            int    `json:"mp,omitempty"`
	Gold          uint32 `json:"gold,omitempty"`
	// Quest rewards use an EXCLUSIVE maximum, as native Vol 191 does:
	// Level < min || Level >= max, unlike inclusive QuestRequirements.MaxLevel.
	MortalOnly        bool                  `json:"mortalOnly,omitempty"`
	RequiredEvolution string                `json:"requiredEvolution,omitempty"`
	MinLevel          uint32                `json:"minLevel,omitempty"`
	MaxLevelExclusive uint32                `json:"maxLevelExclusive,omitempty"`
	X                 uint16                `json:"x,omitempty"`
	Y                 uint16                `json:"y,omitempty"`
	Destinations      []VolatileDestination `json:"destinations,omitempty"`
	// QuizArena is where the ticket teleports while staff runs the 7.59 OX
	// quiz mode; outside it the rule's own action applies.
	QuizArena   *VolatileDestination `json:"quizArena,omitempty"`
	LearnedBit  int                  `json:"learnedBit,omitempty"`
	Summon      *VolatileSummon      `json:"summon,omitempty"`
	Instance    *VolatileInstance    `json:"instance,omitempty"`
	InstanceRef string               `json:"instanceRef,omitempty"`
	// Parameters for the "buff" action (potions and food). AffectType selects
	// applyExtendedAffectStats; AffectValue and AffectLevel feed that formula.
	// DurationUnits uses eight-second blocks. Balance remains data-driven.
	AffectType    int `json:"affectType,omitempty"`
	AffectValue   int `json:"affectValue,omitempty"`
	AffectLevel   int `json:"affectLevel,omitempty"`
	DurationUnits int `json:"durationUnits,omitempty"`
	// Affects applies a consumable's complete effect bundle atomically.
	// The singular fields above remain the compact single-effect form.
	Affects []VolatileAffect `json:"affects,omitempty"`
	// Accumulate adds DurationUnits to the remaining affect duration, capped
	// at MaxDurationUnits. Saturated use is rejected; the cap is required.
	Accumulate       bool `json:"accumulate,omitempty"`
	MaxDurationUnits int  `json:"maxDurationUnits,omitempty"`
	// Exp is the amount granted by "grant_exp" (EXP chests and powders).
	Exp uint32 `json:"exp,omitempty"`
	// Counters credits named counters through "grant_counter".
	// The native item 4127 grants 100 Kefra entries at once.
	Counters map[string]uint32 `json:"counters,omitempty"`
	// RewardItems is the uniform "loot_box" pool. Each reward receives a UID;
	// consumption and reward commit together before client confirmation.
	RewardItems []uint16 `json:"rewardItems,omitempty"`
	// AccessKey/DurationSeconds configure timed licenses (VIP, Hidden Ruins,
	// and event passes). Absolute expiry is persisted in character state.
	AccessKey       string `json:"accessKey,omitempty"`
	DurationSeconds int    `json:"durationSeconds,omitempty"`
	// PartyMode controls tickets: "party" requires a party and its leader;
	// "solo" moves only the user. Empty retains legacy ungrouped admission.
	PartyMode string `json:"partyMode,omitempty"`
	// RefineMax caps the "refine" action: Ori=6, Lac=9.
	// The server rejects an item whose refinement has reached that cap.
	RefineMax int `json:"refineMax,omitempty"`
	// RefineTenChance is the percent chance of refining a +10 item to +11 with
	// a "refine" powder; zero disables it. It never opens +9 -> +10, which only
	// the Aylin composition produces, nor any level above +11.
	RefineTenChance int `json:"refineTenChance,omitempty"`
	// OnceQuestID is a permanent server-side latch for single-use quests,
	// such as Gargoyle Tooth. Negative IDs are reserved for internal flags
	// and cannot collide with configured quests.
	OnceQuestID int32 `json:"onceQuestId,omitempty"`
	// Equipped-target refinement (Fairy Water): eligible slots and item IDs
	// remain in data rather than being invented by the executable.
	TargetSlot  int      `json:"targetSlot,omitempty"`
	TargetItems []uint16 `json:"targetItems,omitempty"`
	// FaceMesh is the monster face mesh for "face_transform" (Gremlin=202,
	// Orc=209, Troll=212). It overrides bodyMesh while the affect lasts;
	// face_restore removes it.
	FaceMesh int `json:"faceMesh,omitempty"`
	// Mount* configure feed, essence, longevity_restore, level_set, growth,
	// and hatch. Amount is the magnitude or target level. MountMinLevel
	// expresses mount requirements independently of character MinLevel.
	MountAction     string `json:"mountAction,omitempty"`
	MountMinLevel   int    `json:"mountMinLevel,omitempty"`
	Amount          int    `json:"amount,omitempty"`
	CooldownSeconds int    `json:"cooldownSeconds,omitempty"`
	// Color is the "tint" effect code: the target item's EF_SANC=43 slot
	// uses code 116-125 while preserving refinement. Each code selects a
	// client color; see pRemoveTintura.
	Color       int    `json:"color,omitempty"`
	Description string `json:"description,omitempty"`
	// Variant selects within shared native engines (gems 0..3 and ores 0..3).
	// SuccessPercent remains authoritative balance data, never client input.
	Variant        int `json:"variant,omitempty"`
	SuccessPercent int `json:"successPercent,omitempty"`
}

type VolatileAffect struct {
	// SkillID references authoritative SkillData.csv. Native SetAffect
	// receives a skill index and derives AffectType/AffectValue from it;
	// the skill index itself is not an affect type.
	SkillID       int `json:"skillId,omitempty"`
	Type          int `json:"type"`
	Value         int `json:"value,omitempty"`
	Level         int `json:"level,omitempty"`
	DurationUnits int `json:"durationUnits"`
}

// VolatileInstance describes a ticket-activated room. Content, coordinates
// and durations remain in data/instances.json; the engine enforces occupancy,
// party admission, spawning, clocks and closure.
type VolatileInstance struct {
	ID string `json:"id"`
	// BaseRef declares variants (Cube Mystic/Arcane) without duplicating the
	// whole table. The loader materializes an independent copy before validation.
	BaseRef    string                  `json:"baseRef,omitempty"`
	Name       string                  `json:"name"`
	X          uint16                  `json:"x"`
	Y          uint16                  `json:"y"`
	SpawnX     uint16                  `json:"spawnX"`
	SpawnY     uint16                  `json:"spawnY"`
	AreaRadius int                     `json:"areaRadius"`
	MaxPlayers int                     `json:"maxPlayers,omitempty"`
	Spawns     []VolatileInstanceSpawn `json:"spawns"`
	// Generator names a generator file under the data directory (for example
	// "NPCGener/NightmareN.txt"). The loader turns every block into a spawn at
	// its native start point, so Spawns must stay empty when it is set.
	Generator  string `json:"generator,omitempty"`
	RewardItem uint16 `json:"rewardItem,omitempty"`
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
	// TotalDurationSeconds is an absolute instance deadline, independent of
	// room transitions; an admission window is not an event lifetime.
	TotalDurationSeconds int `json:"totalDurationSeconds,omitempty"`
	// ActiveDurationSeconds is the event lifetime after a scheduled entry
	// window. Admission and encounter deadlines must not be conflated.
	ActiveDurationSeconds int    `json:"activeDurationSeconds,omitempty"`
	ExitX                 uint16 `json:"exitX"`
	ExitY                 uint16 `json:"exitY"`
	TransitionSeconds     int    `json:"transitionSeconds,omitempty"`
	// SharedEntry lets users spend individual tickets to join an existing
	// first room. Cube admits up to six players before the first answer.
	SharedEntry bool `json:"sharedEntry,omitempty"`
	// Mode defines the execution boundary: private_chain,
	// private_shared_entry, shared_timed_zone or state_machine.
	// Empty preserves the legacy private behavior.
	Mode string `json:"mode,omitempty"`
	// SharedGroup is the runtime key for a shared physical zone. Different
	// rules (solo and party tickets) may share an execution without replacing
	// another room.
	SharedGroup string `json:"sharedGroup,omitempty"`
	// ExclusiveGroup implements logical locks independent of physical occupancy.
	// Magic Chamber shares one lock across Normal/Mystic/Arcane.
	ExclusiveGroup string `json:"exclusiveGroup,omitempty"`
	// FinishPolicy avoids inferring closure from monster counts:
	// finish_on_clear, finish_on_timeout, advance_on_clear,
	// respawn_until_timeout or state_machine.
	FinishPolicy string `json:"finishPolicy,omitempty"`
	// NoCombatTimeout is used by the native Cube: a room advances only after
	// its mobs are cleared and the O/X question is resolved.  It must not be
	// confused with an event-wide deadline or with a transition timeout.
	NoCombatTimeout bool `json:"noCombatTimeout,omitempty"`
	// StateMachine identifies progression beyond linear rooms. Controllers
	// read these data; Hell Gate/Big Cube coordinates are not gameplay constants.
	StateMachine string            `json:"stateMachine,omitempty"`
	HellGate     *VolatileHellGate `json:"hellGate,omitempty"`
	// Uxmal describes the NPC-started Clue of Runes, not a volatile item.
	// Its template stays in instances.json so rooms, tickets and rewards
	// can be audited without recompiling the server.
	Uxmal *VolatileUxmal `json:"uxmal,omitempty"`
	// AllowChainDuringExitGrace makes Water's ten-second window explicit.
	// The historical RewardItem fallback remains only for old data.
	AllowChainDuringExitGrace bool `json:"allowChainDuringExitGrace,omitempty"`
	// EntryAreas restricts 0x373 TargetXY to one or more native admission
	// areas. Empty preserves tickets usable anywhere, including test fixtures.
	EntryAreas []VolatileInstanceEntryArea `json:"entryAreas,omitempty"`
	// EntryAreaSet references a reusable data table expanded into EntryAreas.
	// All 27 Water tickets therefore share the same native bounding boxes.
	EntryAreaSet string `json:"entryAreaSet,omitempty"`
	// Schedule defines admission windows repeated each hour. Tickets are
	// accepted only within a window; combat may continue after it closes
	// according to the event's separate lifetime.
	Schedule []VolatileInstanceWindow `json:"schedule,omitempty"`
	// PartyRunLimit caps shared executions in the same hourly window.
	// Nightmare uses three per tier; zero leaves the template uncapped.
	PartyRunLimit int `json:"partyRunLimit,omitempty"`
	// NightmareTier selects normal, mystic or arcane admission rules; empty
	// keeps a generic template. Gameplay applies Celestial level gates and
	// debits one NT entry per Celestial in Arcane.
	NightmareTier string `json:"nightmareTier,omitempty"`
	// NightmareProtectors are friendly, mortal NPCs; they never count as kills.
	NightmareProtectors []VolatileInstanceSpawn `json:"nightmareProtectors,omitempty"`
	// NightmareBoss is a one-shot encounter, independent of ordinary respawns.
	NightmareBoss *VolatileNightmareBoss  `json:"nightmareBoss,omitempty"`
	Stages        []VolatileInstanceStage `json:"stages,omitempty"`
}

type VolatileNightmareBoss struct {
	Spawn            VolatileInstanceSpawn `json:"spawn"`
	SecondsRemaining int                   `json:"secondsRemaining"`
}

// VolatileUxmal is the authoritative Rune Track table. Version 759 selects
// historical room objectives and event-only generators. Room +0 allows two
// parties, +4 allows four, and other rooms allow three. Rewards use each
// party's historical global slot; TicketNextSanc selects the next refinement.
type VolatileUxmal struct {
	Version        int                         `json:"version,omitempty"`
	Generator      string                      `json:"generator,omitempty"`
	Generators     []NPCGener                  `json:"-"`
	NPC            string                      `json:"npc"`
	TicketItem     uint16                      `json:"ticketItem"`
	EntryAreas     []VolatileInstanceEntryArea `json:"entryAreas"`
	RoomPositions  [][]VolatileDestination     `json:"roomPositions"`
	MaxParties     []int                       `json:"maxParties"`
	Runes          [][]uint16                  `json:"runes"`
	TicketNextSanc []int                       `json:"ticketNextSanc"`
}

// InstanceRuntimeState is the restart-safe portion of an event instance.
// Entity IDs, player IDs and ordinary mob HP are deliberately excluded: they are
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
	HellGateVersion       int       `json:"hellGateVersion,omitempty"`
	HellGateVariant       int       `json:"hellGateVariant,omitempty"`
	HellGateClearedMask   uint8     `json:"hellGateClearedMask,omitempty"`
	HellGateLichSpawned   uint8     `json:"hellGateLichSpawnedMask,omitempty"`
	HellGateValidLichMask uint8     `json:"hellGateValidLichMask,omitempty"`
	HellGateWrongLich     bool      `json:"hellGateWrongLich,omitempty"`
	HellGateDionysHP      uint32    `json:"hellGateDionysHp,omitempty"`
	NightmareLostMask     uint8     `json:"nightmareLostMask,omitempty"`
	NightmareBossSpawned  bool      `json:"nightmareBossSpawned,omitempty"`
	NightmareBossKilled   bool      `json:"nightmareBossKilled,omitempty"`
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

// VolatileInstanceSpawn supports multiple templates in a room. Nessus cards
// use one boss and ten helpers; ordinary rooms may use one entry.
// The authoritative hostile population is the sum of Counts.
type VolatileInstanceSpawn struct {
	NPC   string `json:"npc"`
	Count int    `json:"count"`
	X     uint16 `json:"x,omitempty"`
	Y     uint16 `json:"y,omitempty"`
	// Range limits placement around X/Y (the generator StartRange). Zero keeps
	// the stage area radius.
	Range int `json:"range,omitempty"`
	// Formation759 selects the historical formation-zero offsets: 1 includes
	// the leader, 2 contains followers only. Zero retains ordinary placement.
	Formation759 int `json:"formation759,omitempty"`
	// GeneratorID selects one block from the enclosing Hell Gate generator.
	// The loader resolves it before validating the encounter. Runtime code
	// consumes the resulting NPC/count/position, not the selector.
	GeneratorID int `json:"generatorId,omitempty"`
}

// VolatileInstanceStage lets a ticket lead a party through several rooms.
// Water retains its compact room form; Magic Chamber configures four rooms
// and a boss without hardcoding them in the executable.
type VolatileInstanceStage struct {
	Name       string `json:"name,omitempty"`
	X          uint16 `json:"x"`
	Y          uint16 `json:"y"`
	SpawnX     uint16 `json:"spawnX"`
	SpawnY     uint16 `json:"spawnY"`
	AreaRadius int    `json:"areaRadius"`
	// AreaX/AreaY separate room bounds from the entrance. Each zero coordinate
	// inherits X/Y, preserving the bounds of existing templates.
	AreaX           uint16                  `json:"areaX,omitempty"`
	AreaY           uint16                  `json:"areaY,omitempty"`
	DurationSeconds int                     `json:"durationSeconds,omitempty"`
	Spawns          []VolatileInstanceSpawn `json:"spawns"`
	// CompletionSpawns appears in the same room after Spawns is cleared.
	// Magic Chamber uses it for the fourth room's boss without adding a fifth
	// room or renewing the combat deadline.
	CompletionSpawns []VolatileInstanceSpawn `json:"completionSpawns,omitempty"`
	// RuneBonusSpawns appears after the completion boss when the run's Magic
	// Chamber runes include colors 1, 2 and 3 (7.59 TMSRV sub_561F90).
	RuneBonusSpawns []VolatileInstanceSpawn `json:"runeBonusSpawns,omitempty"`
	Quiz            *VolatileInstanceQuiz   `json:"quiz,omitempty"`
	// Generator, GeneratorIDs and CompletionGeneratorIDs select blocks (#[n])
	// of a generator file under the data directory for this room's wave and
	// completion boss. The loader fills Spawns/CompletionSpawns from them.
	Generator              string `json:"generator,omitempty"`
	GeneratorIDs           []int  `json:"generatorIds,omitempty"`
	CompletionGeneratorIDs []int  `json:"completionGeneratorIds,omitempty"`
}

// VolatileHellGate describes the controller, corridor bosses, four rooms,
// their Liches and the protected final actor. Version zero retains the old
// pair-of-Liches contract; version two selects one correct boss and Lich.
// Version three restores the historically documented random pair and portal gate.
type VolatileHellGate struct {
	// Versions 2 and 3 share timed protection, but retain independent puzzle rules.
	Version   int                     `json:"version,omitempty"`
	RewardEXP uint32                  `json:"rewardExp,omitempty"`
	Corridors []VolatileInstanceSpawn `json:"corridors,omitempty"`
	Portals   []Teleport              `json:"portals,omitempty"`
	// Generator contains instance-only NPCGener blocks for all encounter waves.
	Generator     string                     `json:"generator,omitempty"`
	ControllerNPC string                     `json:"controllerNPC"`
	Quadrants     []VolatileHellGateQuadrant `json:"quadrants"`
	FinalSpawns   []VolatileInstanceSpawn    `json:"finalSpawns,omitempty"`
	FinalNPCs     []VolatileInstanceSpawn    `json:"finalNPCs,omitempty"`
	// Historical population is used only by version 3. The original fields
	// remain available for already persisted version-2 runs.
	Population759 *VolatileHellGatePopulation `json:"population759,omitempty"`
}

type VolatileHellGatePopulation struct {
	CorridorEscorts []VolatileInstanceSpawn   `json:"corridorEscorts"`
	Rooms           [][]VolatileInstanceSpawn `json:"rooms"`
	LichEscorts     [][]VolatileInstanceSpawn `json:"lichEscorts"`
	FinalSpawns     []VolatileInstanceSpawn   `json:"finalSpawns"`
}

func (p *VolatileHellGatePopulation) Spawns() []VolatileInstanceSpawn {
	if p == nil {
		return nil
	}
	spawns := append([]VolatileInstanceSpawn(nil), p.CorridorEscorts...)
	spawns = append(spawns, p.FinalSpawns...)
	for _, rooms := range [][][]VolatileInstanceSpawn{p.Rooms, p.LichEscorts} {
		for _, room := range rooms {
			spawns = append(spawns, room...)
		}
	}
	return spawns
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

// VolatileInstanceQuiz models Cube O/X rooms. After clearing a room, members
// physically choose O or X: correct answers advance and grant EXP; incorrect
// answers exit. TrueX/TrueY identify O (Yes), FalseX/FalseY identify X (No).
// The names refer to statement truth, not platform artwork.
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

// VolatileSummon keeps contracts data-driven. Group distinguishes two
// families/kingdoms without anticipating future Castle War rules.
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

// ReplictionItem maps consumables A-E to equipment level and refinement cap.
// Normal and Premium remain data, not inferred behavior.
type ReplictionItem struct {
	ItemLevel int `json:"itemLevel"`
	MaxSanc   int `json:"maxSanc"`
}

// ReplictionBonus is the indivisible pair selected by native SetItemBonus2.
// Applying Repliction always replaces both item bonuses.
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

// VolatileCatalog joins configured rules to items present in itemlist.
// ItemCodes contains ALL EF_VOLATILE items, including NPC/command items
// and entries awaiting the Celestial system.
type VolatileCatalog struct {
	Default VolatileRule
	Rules   map[int]VolatileRule
	Items   map[uint16]VolatileRule
	// Instances preserves named data templates even without a volatile item.
	// NPC-started events such as Clue of Runes/Uxmal use the same authoritative
	// catalog as consumable-started instances.
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
