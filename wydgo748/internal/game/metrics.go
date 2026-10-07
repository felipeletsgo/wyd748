package game

import (
	"expvar"
	"sync/atomic"
	"time"

	"wydgo/internal/wire"
)

// metrics.go -- game-loop instrumentation through expvar (stdlib). Prometheus
// is deliberately not used: the module has NO external dependency, which is a
// project asset. The counters are published at /debug/vars, which cmd/server
// only serves on 127.0.0.1.
//
// Cost: the counters are atomic and written once per command/tick. Nothing
// here scans players or mobs -- the gauges are updated by the tick itself,
// which already has the sizes at hand.
//
// Cardinality: the command label is the opcode in hexadecimal, a small closed
// set. NEVER index by session, player or mob.

var (
	// metricLoopLagMicros is the delay of the last tick: how much the real
	// interval exceeded the nominal 500 ms. It grows when the loop is congested
	// -- the most direct sign that the server is not keeping up.
	metricLoopLagMicros = expvar.NewInt("world_loop_lag_micros")
	// metricLoopLagMaxMicros keeps the worst delay observed since boot.
	metricLoopLagMaxMicros = expvar.NewInt("world_loop_lag_max_micros")
	// metricTickDurationMicros is how long the last tick took to run.
	metricTickDurationMicros = expvar.NewInt("world_tick_duration_micros")
	// metricCommandQueueDepth is the size of the input queue at the last tick.
	metricCommandQueueDepth = expvar.NewInt("world_command_queue_depth")
	// Packets retained behind their own player persistence reservations.
	metricCriticalDeferredCommands = expvar.NewInt("world_critical_deferred_commands")
	// Legacy inline writes refused because the coordinator owns persistence.
	metricSyncPersistenceRejected = expvar.NewInt("world_sync_persistence_rejected")
	// Account-only operations isolated after an unknown or unverifiable outcome.
	metricCriticalQuarantined = expvar.NewInt("world_critical_quarantined")
	metricCommandsTotal     = expvar.NewInt("world_commands_total")
	// metricCommandDurationMicros accumulates the time spent per opcode.
	// Divided by world_commands_by_type it gives the mean per command.
	metricCommandDurationMicros  = expvar.NewMap("world_command_duration_micros")
	metricCommandsByType         = expvar.NewMap("world_commands_by_type")
	metricCommandQueueAgeMicros  = expvar.NewInt("world_command_queue_age_micros")
	metricCommandBatchSize       = expvar.NewInt("world_command_batch_size")
	metricCommandBatchesTotal    = expvar.NewInt("world_command_batches_total")
	metricCommandBudgetExceeded  = expvar.NewInt("world_command_budget_exceeded_total")
	metricTickDurationBuckets    = expvar.NewMap("world_tick_duration_buckets")
	metricCommandDurationBuckets = expvar.NewMap("world_command_duration_buckets")
	metricCommandAgeBuckets      = expvar.NewMap("world_command_age_buckets")
	metricActivePlayers          = expvar.NewInt("world_active_players")
	metricActiveMobs             = expvar.NewInt("world_active_mobs")
	// Attack/skill admission counters make a missing hit diagnosable without
	// enabling a synchronous log line for every packet.  A spike in rejected
	// skills points at client timing; a normal counter with no damage points at
	// target/cooldown/mana validation inside the skill handler.
	metricPhysicalAttackRejected = expvar.NewInt("world_physical_attack_rejected_total")
	metricSkillPacketRejected    = expvar.NewInt("world_skill_packet_rejected_total")
	metricSkillPacketAccepted    = expvar.NewInt("world_skill_packet_accepted_total")
	// metricPanicsTotal counts panics contained by safeHandle. Any value other
	// than zero deserves investigation: it means ONE command left potentially
	// partial state.
	metricPanicsTotal = expvar.NewInt("world_panics_total")
)

// lastTickAt keeps the time of the previous tick to compute the lag. It is
// atomic because expvar publishes functions that another goroutine (the HTTP
// handler) can read, although the write always comes from the game loop.
var inboundCommandMetricLabels = map[uint16]string{
	wire.OpConnectAccount:          "0x20D",
	wire.OpCreateCharacter:         "0x20F",
	wire.OpDeleteCharacter:         "0x211",
	wire.OpCharacterTransfer:       "0xFAA",
	wire.OpDeclareServerWar:        "0xED7",
	wire.OpRefuseServerWar:         "0xED8",
	wire.OpBuyLotteryTicket:        "0x3B8",
	wire.OpHellStoreBuy:            "0x2BD",
	wire.OpStoreGoldRequest:        "0x3D1",
	wire.OpCharacterLogin:          "0x213",
	wire.OpCharacterLogout:         "0x215",
	wire.OpClientIntegrityResponse: "0x2C2",
	wire.OpQuizAnswer:              "0x7F11",
	wire.OpSwapItem:                "0x376",
	wire.OpDeposit:                 "0x388",
	wire.OpWithdraw:                "0x387",
	wire.OpUseItem:                 "0x373",
	wire.OpUsePremiumFirework:      "0x3C9",
	wire.OpCapsuleInfo:             "0x2CD",
	wire.OpPutoutSeal:              "0x3CC",
	wire.OpBuyToto:                 "0x3CE",
	wire.OpDoJackpotBet:            "0x2BE",
	wire.OpUseNPC:                  "0x28B",
	wire.OpReqShopList:             "0x27B",
	wire.OpBuyItem:                 "0x379",
	wire.OpSellItem:                "0x37A",
	wire.OpApplyBonus:              "0x277",
	wire.OpPartyRequest:            "0x37F",
	wire.OpPartyAccept:             "0x3AB",
	wire.OpPartyRemove:             "0x37E",
	wire.OpTrade:                   "0x383",
	wire.OpCloseTrade:              "0x384",
	wire.OpAutoTrade:               "0x397",
	wire.OpReqTradeList:            "0x39A",
	wire.OpReqBuyAutoTrade:         "0x398",
	wire.OpDropItem:                "0x272",
	wire.OpGetItem:                 "0x270",
	wire.OpDeleteItem:              "0x2E4",
	wire.OpUpdateItem:              "0x374",
	wire.OpSplitItem:               "0x2E5",
	wire.OpSetShortSkill:           "0x378",
	wire.OpMessageChat:             "0x333",
	wire.OpMessageWhisper:          "0x334",
	wire.OpChangeCity:              "0x291",
	wire.OpReqTeleport:             "0x290",
	wire.OpAirMove:                 "0xAD9",
	wire.OpPKMode:                  "0x399",
	wire.OpMoveStop:                "0x2CB",
	wire.OpUpdateScore:             "0x336",
	wire.OpRestart:                 "0x289",
	wire.OpPing:                    "0x3A0",
	wire.OpSysQuit:                 "0x3AE",
	wire.OpAction:                  "0x366",
	wire.OpActionStop:              "0x367",
	wire.OpIllusion:                "0x368",
	wire.OpREQMobByID:              "0x369",
	wire.OpGuildDeprivate:          "0x28C",
	wire.OpInviteGuild:             "0x3D5",
	wire.OpGuildAlly:               "0xE12",
	wire.OpGuildWar:                "0xE0E",
	wire.OpChallenge:               "0x28E",
	wire.OpChallengeConfirm:        "0x28F",
	wire.OpMotion:                  "0x36A",
	wire.OpClientUnknown2BC:        "0x2BC",
	wire.OpAttackOne:               "0x39D",
	wire.OpAttackTwo:               "0x39E",
	wire.OpAttackMulti:             "0x36C",
	wire.OpPlayerChallenge:         "0x39F",
	wire.OpCombineTiny:             "0x3C0",
	wire.OpCombineLindy:            "0x2C3",
	wire.OpCombineCompositor:       "0x3A6",
	wire.OpCombineAgatha:           "0x3BA",
	wire.OpCombineAylin:            "0x3B5",
	wire.OpCombineEhre:             "0x2D3",
	wire.OpCombineOdin:             "0x2D2",
	wire.OpCombineExtracao:         "0x2D4",
	wire.OpCombineAlquimia:         "0x2E1",
}

var metricDurationBucketLimits = [...]time.Duration{
	time.Millisecond,
	5 * time.Millisecond,
	10 * time.Millisecond,
	25 * time.Millisecond,
	50 * time.Millisecond,
	100 * time.Millisecond,
	250 * time.Millisecond,
	time.Second,
}

var metricDurationBucketLabels = [...]string{
	"<=1ms",
	"<=5ms",
	"<=10ms",
	"<=25ms",
	"<=50ms",
	"<=100ms",
	"<=250ms",
	"<=1s",
}

func inboundCommandMetricLabel(opcode uint16) (string, bool) {
	label, ok := inboundCommandMetricLabels[opcode]
	return label, ok
}

func durationBucketLabel(duration time.Duration) string {
	for i, limit := range metricDurationBucketLimits {
		if duration <= limit {
			return metricDurationBucketLabels[i]
		}
	}
	return "+Inf"
}

var lastTickAt atomic.Int64

// observeTick records the tick duration and the delay against the nominal
// interval. Called by the tick itself, with its start time.
func observeTick(start time.Time, duration time.Duration) {
	metricTickDurationMicros.Set(duration.Microseconds())
	observeBucket(metricTickDurationBuckets, duration)

	previous := lastTickAt.Swap(start.UnixNano())
	if previous == 0 {
		return // first tick: there is no previous interval to compare
	}
	// Lag = how much the real interval exceeded the nominal 500 ms. A negative
	// value (early tick) becomes zero: only the delay matters.
	lag := start.Sub(time.Unix(0, previous)) - worldTickInterval
	if lag < 0 {
		lag = 0
	}
	micros := lag.Microseconds()
	metricLoopLagMicros.Set(micros)
	if micros > metricLoopLagMaxMicros.Value() {
		metricLoopLagMaxMicros.Set(micros)
	}
}

// observeCommand records the duration of ONE command, labeled by opcode.
func observeCommand(label string, duration time.Duration) {
	metricCommandsTotal.Add(1)
	metricCommandsByType.Add(label, 1)
	metricCommandDurationMicros.Add(label, duration.Microseconds())
	observeBucket(metricCommandDurationBuckets, duration)
}

func observeCommandQueueAge(age time.Duration) {
	if age < 0 {
		age = 0
	}
	metricCommandQueueAgeMicros.Set(age.Microseconds())
	observeBucket(metricCommandAgeBuckets, age)
}

// observeBucket publishes fixed distributions. The operator can derive p50,
// p95 and p99 over the collection interval without a series per player/opcode.
func observeBucket(m *expvar.Map, duration time.Duration) {
	m.Add(durationBucketLabel(duration), 1)
}

func observeCommandBatch(size int, budgetExceeded bool) {
	metricCommandBatchSize.Set(int64(size))
	metricCommandBatchesTotal.Add(1)
	if budgetExceeded {
		metricCommandBudgetExceeded.Add(1)
	}
}

// observeWorldGauges publishes the sizes the tick already knows.
func observeWorldGauges(players, activeMobs int) {
	metricActivePlayers.Set(int64(players))
	metricActiveMobs.Set(int64(activeMobs))
}
