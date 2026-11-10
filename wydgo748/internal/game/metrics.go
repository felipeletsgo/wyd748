package game

import (
	"errors"
	"expvar"
	"fmt"
	"strconv"
	"sync"
	"sync/atomic"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/store"
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
	metricNPCDefinitionPollFailures     = expvar.NewInt("npc_definition_poll_failures_total")
	metricNPCDefinitionQueueDrops       = expvar.NewInt("npc_definition_queue_drops_total")
	metricNPCDefinitionApplyDelayMicros = expvar.NewMap("npc_definition_apply_delay_micros")
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
	metricCommandsTotal       = expvar.NewInt("world_commands_total")
	// metricCommandDurationMicros accumulates the time spent per opcode.
	// Divided by world_commands_by_type it gives the mean per command.
	metricCommandDurationMicros = expvar.NewMap("world_command_duration_micros")
	metricCommandsByType        = expvar.NewMap("world_commands_by_type")
	metricCommandQueueAgeMicros = expvar.NewInt("world_command_queue_age_micros")
	metricCommandBatchSize      = expvar.NewInt("world_command_batch_size")
	metricCommandBatchesTotal   = expvar.NewInt("world_command_batches_total")
	metricCommandBudgetExceeded = expvar.NewInt("world_command_budget_exceeded_total")
	metricTickDurationBuckets   = expvar.NewMap("world_tick_duration_buckets")
	// Fixed lag distribution: p95/p99 between two readings, not only last/max.
	metricLoopLagBuckets         = expvar.NewMap("world_loop_lag_buckets")
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
	// Critical persistence state (W0). The actor tick samples the queue gauges;
	// workers update running, outcomes and execution time atomically.
	metricCriticalPending         = expvar.NewInt("world_critical_pending")
	metricCriticalQueuedJobs      = expvar.NewInt("world_critical_queued_jobs")
	metricCriticalRunning         = expvar.NewInt("world_critical_running")
	metricCriticalResultsWaiting  = expvar.NewInt("world_critical_results_waiting")
	metricCriticalQuarantinedNow  = expvar.NewInt("world_critical_quarantined_now")
	metricCriticalOldestMicros    = expvar.NewInt("world_critical_oldest_pending_micros")
	metricCriticalOutcomes        = expvar.NewMap("world_critical_outcomes_total")
	metricCriticalAdmission       = expvar.NewMap("world_critical_admission_rejected_total")
	metricCriticalDurationBuckets = expvar.NewMap("world_critical_duration_buckets")
	// Views whose optional creations waited for output queue room (scene.go).
	metricSceneDeferred = expvar.NewInt("world_scene_deferred_total")
	// Account-only quarantines released after their exact outcome was final.
	metricCriticalQuarantineReleased = expvar.NewInt("world_critical_quarantine_released_total")
	// Writes parked behind a background autosave instead of holding input (W7).
	metricCriticalParked = expvar.NewInt("world_critical_parked_total")
	// Ticks that skipped instance lifecycle work behind an instance write.
	metricInstanceTickSkipped = expvar.NewInt("world_instance_tick_skipped_total")
)

// Closed label sets for critical persistence outcomes and admission refusals.
var (
	criticalOutcomeLabels   = control.CriticalOutcomeLabels
	criticalAdmissionLabels = control.CriticalAdmissionLabels
)

func init() {
	for _, label := range criticalOutcomeLabels {
		metricCriticalOutcomes.Add(label, 0)
	}
	for _, label := range criticalAdmissionLabels {
		metricCriticalAdmission.Add(label, 0)
	}
	expvar.Publish(control.ChannelMetricsName, expvar.Func(func() any { return readChannelMetrics(time.Now()) }))
}

// observeCriticalOutcome classifies one worker result before actor delivery.
func observeCriticalOutcome(err error, duration time.Duration) {
	label := criticalOutcomeLabels[0]
	switch {
	case err == nil:
	case store.IsWriteRejected(err) && !errors.Is(err, errCriticalPersistenceIndeterminate):
		label = criticalOutcomeLabels[1]
	default:
		label = criticalOutcomeLabels[2]
	}
	metricCriticalOutcomes.Add(label, 1)
	observeBucket(metricCriticalDurationBuckets, duration)
}

// observeCriticalAdmission counts a submission refused before queue admission.
func observeCriticalAdmission(err error) {
	label := criticalAdmissionLabels[4]
	switch {
	case errors.Is(err, errCriticalPersistenceBusy):
		label = criticalAdmissionLabels[0]
	case errors.Is(err, errCriticalPersistenceFull):
		label = criticalAdmissionLabels[1]
	case errors.Is(err, errCriticalPersistenceStopping):
		label = criticalAdmissionLabels[2]
	case errors.Is(err, errCriticalPersistenceUnsupported):
		label = criticalAdmissionLabels[3]
	}
	metricCriticalAdmission.Add(label, 1)
}

// observeCriticalQueue samples the actor-owned queue state. The pending map is
// bounded by capacity plus retained quarantine, so one scan per tick is cheap.
func observeCriticalQueue(q *criticalPersistence, now time.Time) {
	if q == nil {
		return
	}
	var oldest time.Time
	for _, pending := range q.pending {
		if pending.quarantined || pending.submittedAt.IsZero() {
			continue
		}
		if oldest.IsZero() || pending.submittedAt.Before(oldest) {
			oldest = pending.submittedAt
		}
	}
	age := time.Duration(0)
	if !oldest.IsZero() {
		age = max(now.Sub(oldest), 0)
	}
	metricCriticalPending.Set(int64(len(q.pending) - q.quarantined))
	metricCriticalQueuedJobs.Set(int64(len(q.jobs)))
	metricCriticalResultsWaiting.Set(int64(len(q.results)))
	metricCriticalQuarantinedNow.Set(int64(q.quarantined))
	metricCriticalOldestMicros.Set(age.Microseconds())
}

// channelMetrics holds one channel's actor readings. The cluster runs every
// channel on one goroutine, so process-wide gauges are sums across channels.
// Labels are channel numbers only; nothing is indexed by player or session.
type channelMetrics struct {
	players, mobs, queueDepth, deferred atomic.Int64
	// heartbeat is the UnixNano end of the last tick or command batch.
	heartbeat atomic.Int64
	// maintenance mirrors World.shuttingDown for readiness (W5/D5).
	maintenance                      atomic.Bool
	loopLag, tick, command, queueAge durationBuckets
}

type durationBuckets [len(control.DurationBucketLabels)]atomic.Int64

func (b *durationBuckets) observe(duration time.Duration) { b[durationBucketIndex(duration)].Add(1) }

func (b *durationBuckets) read() (out [len(control.DurationBucketLabels)]int64) {
	for i := range b {
		out[i] = b[i].Load()
	}
	return out
}

var channelMetricsRegistry = struct {
	sync.Mutex
	channels map[byte]*channelMetrics
}{channels: make(map[byte]*channelMetrics)}

func channelMetricsFor(channel byte) *channelMetrics {
	channelMetricsRegistry.Lock()
	defer channelMetricsRegistry.Unlock()
	m := channelMetricsRegistry.channels[channel]
	if m == nil {
		m = &channelMetrics{}
		channelMetricsRegistry.channels[channel] = m
	}
	return m
}

// channelMetrics resolves after NewWorld assigns the configured channel.
func (w *World) channelMetrics() *channelMetrics {
	if w.metrics == nil {
		w.metrics = channelMetricsFor(w.channel)
	}
	return w.metrics
}

func readChannelMetrics(now time.Time) map[string]control.ChannelReading {
	channelMetricsRegistry.Lock()
	defer channelMetricsRegistry.Unlock()
	out := make(map[string]control.ChannelReading, len(channelMetricsRegistry.channels))
	for channel, m := range channelMetricsRegistry.channels {
		reading := control.ChannelReading{
			Players: m.players.Load(), Mobs: m.mobs.Load(), QueueDepth: m.queueDepth.Load(),
			DeferredCommands: m.deferred.Load(), LoopLag: m.loopLag.read(), Tick: m.tick.read(),
			Command: m.command.read(), QueueAge: m.queueAge.read(),
		}
		if beat := m.heartbeat.Load(); beat != 0 {
			reading.HeartbeatAgeMicros = max(now.Sub(time.Unix(0, beat)), 0).Microseconds()
		}
		out[strconv.Itoa(int(channel))] = reading
	}
	return out
}

// publishChannelSums keeps the established process-wide gauges as totals.
func publishChannelSums() {
	channelMetricsRegistry.Lock()
	defer channelMetricsRegistry.Unlock()
	var players, mobs, queue, deferred int64
	for _, m := range channelMetricsRegistry.channels {
		players += m.players.Load()
		mobs += m.mobs.Load()
		queue += m.queueDepth.Load()
		deferred += m.deferred.Load()
	}
	metricActivePlayers.Set(players)
	metricActiveMobs.Set(mobs)
	metricCommandQueueDepth.Set(queue)
	metricCriticalDeferredCommands.Set(deferred)
}

func (w *World) observeDeferredCommands() {
	w.channelMetrics().deferred.Store(int64(w.criticalDeferredCount))
	publishChannelSums()
}

func (w *World) observeHeartbeat(at time.Time) {
	m := w.channelMetrics()
	m.heartbeat.Store(at.UnixNano())
	m.maintenance.Store(w.shuttingDown)
}

// ChannelsReady reports whether every running channel actor is responsive and
// open for play. It reads only atomics and never waits for World. A channel
// in maintenance, shutdown or with a heartbeat older than maxAge is not ready.
func ChannelsReady(now time.Time, maxAge time.Duration) error {
	channelMetricsRegistry.Lock()
	defer channelMetricsRegistry.Unlock()
	for channel, m := range channelMetricsRegistry.channels {
		beat := m.heartbeat.Load()
		if beat == 0 {
			continue // never ran: not a live channel of this process
		}
		if m.maintenance.Load() {
			return fmt.Errorf("channel %d is in maintenance", channel)
		}
		if age := now.Sub(time.Unix(0, beat)); age > maxAge {
			return fmt.Errorf("channel %d actor heartbeat is %s old", channel, age.Round(time.Millisecond))
		}
	}
	return nil
}

// Observe committed revisions only; channel labels are bounded to uint8.
func observeNPCDefinitionApply(channel uint8, updatedAt, now time.Time) {
	if updatedAt.IsZero() {
		return
	}
	delay := now.Sub(updatedAt)
	if delay < 0 {
		delay = 0
	}
	value := new(expvar.Int)
	value.Set(delay.Microseconds())
	metricNPCDefinitionApplyDelayMicros.Set(strconv.Itoa(int(channel)), value)
}

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
	wire.OpPremiumBuy:              "0x7F21",
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
	wire.OpQuotedSellItem:          "0x7F40",
	wire.OpDropListQuery:           "0x7F41",
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

// durationBucketIndex returns the fixed bucket; the last index is +Inf.
func durationBucketIndex(duration time.Duration) int {
	for i, limit := range metricDurationBucketLimits {
		if duration <= limit {
			return i
		}
	}
	return len(metricDurationBucketLimits)
}

func durationBucketLabel(duration time.Duration) string {
	if i := durationBucketIndex(duration); i < len(metricDurationBucketLabels) {
		return metricDurationBucketLabels[i]
	}
	return "+Inf"
}

// observeTick records the tick duration and the delay against the nominal
// interval. Each World keeps its own previous start: the cluster ticks both
// channels back to back, and a shared value would hide the real lag.
func (w *World) observeTick(start time.Time, duration time.Duration) {
	m := w.channelMetrics()
	metricTickDurationMicros.Set(duration.Microseconds())
	observeBucket(metricTickDurationBuckets, duration)
	m.tick.observe(duration)
	m.heartbeat.Store(start.Add(duration).UnixNano())
	m.maintenance.Store(w.shuttingDown)

	previous := w.lastTickStart
	w.lastTickStart = start
	if previous.IsZero() {
		return // first tick: there is no previous interval to compare
	}
	// Lag = how much the real interval exceeded the nominal 500 ms. A negative
	// value (early tick) becomes zero: only the delay matters.
	lag := start.Sub(previous) - worldTickInterval
	if lag < 0 {
		lag = 0
	}
	micros := lag.Microseconds()
	metricLoopLagMicros.Set(micros)
	observeBucket(metricLoopLagBuckets, lag)
	m.loopLag.observe(lag)
	if micros > metricLoopLagMaxMicros.Value() {
		metricLoopLagMaxMicros.Set(micros)
	}
}

// observeCommand records the duration of ONE command, labeled by opcode.
func (w *World) observeCommand(label string, duration time.Duration) {
	metricCommandsTotal.Add(1)
	metricCommandsByType.Add(label, 1)
	metricCommandDurationMicros.Add(label, duration.Microseconds())
	observeBucket(metricCommandDurationBuckets, duration)
	w.channelMetrics().command.observe(duration)
}

func (w *World) observeCommandQueueAge(age time.Duration) {
	if age < 0 {
		age = 0
	}
	metricCommandQueueAgeMicros.Set(age.Microseconds())
	observeBucket(metricCommandAgeBuckets, age)
	w.channelMetrics().queueAge.observe(age)
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
func (w *World) observeWorldGauges(players, activeMobs, queueDepth int) {
	m := w.channelMetrics()
	m.players.Store(int64(players))
	m.mobs.Store(int64(activeMobs))
	m.queueDepth.Store(int64(queueDepth))
	publishChannelSums()
}
