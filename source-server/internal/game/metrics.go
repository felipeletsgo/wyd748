package game

import (
	"expvar"
	"sync/atomic"
	"time"

	"wydgo/internal/wire"
)

// metrics.go -- instrumentacao do game loop via expvar (stdlib). Nao usamos
// Prometheus de proposito: o modulo nao tem NENHUMA dependencia externa e isso
// e um ativo do projeto. Os contadores ficam publicados em /debug/vars, que o
// cmd/server sobe apenas em 127.0.0.1.
//
// Custo: os contadores sao atomicos e escritos uma vez por comando/tick. Nada
// aqui varre jogadores nem mobs -- os gauges sao atualizados pelo proprio tick,
// que ja tem os tamanhos em maos.
//
// Cardinalidade: o rotulo de comando e o opcode em hexadecimal, um conjunto
// fechado e pequeno. NUNCA indexar por sessao, jogador ou mob.

var (
	// metricLoopLagMicros e o atraso do ultimo tick: quanto o intervalo real
	// passou dos 500 ms nominais. Cresce quando o loop esta congestionado --
	// e o sinal mais direto de que o servidor nao esta acompanhando.
	metricLoopLagMicros = expvar.NewInt("world_loop_lag_micros")
	// metricLoopLagMaxMicros retem o pior atraso observado desde o boot.
	metricLoopLagMaxMicros = expvar.NewInt("world_loop_lag_max_micros")
	// metricTickDurationMicros e quanto o ultimo tick levou para executar.
	metricTickDurationMicros = expvar.NewInt("world_tick_duration_micros")
	// metricCommandQueueDepth e o tamanho da fila de entrada no ultimo tick.
	metricCommandQueueDepth = expvar.NewInt("world_command_queue_depth")
	metricCommandsTotal     = expvar.NewInt("world_commands_total")
	// metricCommandDurationMicros acumula o tempo gasto por opcode. Dividido
	// por world_commands_by_type da a media por comando.
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
	// metricPanicsTotal conta panics contidos por safeHandle. Qualquer valor
	// diferente de zero merece investigacao: significa que UM comando deixou
	// estado potencialmente parcial.
	metricPanicsTotal = expvar.NewInt("world_panics_total")
)

// lastTickAt guarda o instante do tick anterior para calcular o lag. E atomico
// porque expvar publica funcoes que podem ser lidas por outra goroutine (o
// handler HTTP), embora a escrita venha sempre do game loop.
var inboundCommandMetricLabels = map[uint16]string{
	wire.OpConnectAccount:          "0x20D",
	wire.OpCreateCharacter:         "0x20F",
	wire.OpDeleteCharacter:         "0x211",
	wire.OpCharacterLogin:          "0x213",
	wire.OpCharacterLogout:         "0x215",
	wire.OpClientIntegrityResponse: "0x2C2",
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
	wire.OpRebuy:                   "0x3E8",
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

// observeTick registra a duracao do tick e o atraso em relacao ao intervalo
// nominal. Chamado pelo proprio tick, com o instante de inicio.
func observeTick(start time.Time, duration time.Duration) {
	metricTickDurationMicros.Set(duration.Microseconds())
	observeBucket(metricTickDurationBuckets, duration)

	previous := lastTickAt.Swap(start.UnixNano())
	if previous == 0 {
		return // primeiro tick: nao existe intervalo anterior para comparar
	}
	// Lag = o quanto o intervalo real excedeu os 500 ms nominais. Negativo
	// (tick adiantado) vira zero: so o atraso interessa.
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

// observeCommand registra a duracao de UM comando, rotulado pelo opcode.
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

// observeBucket publica distribuicoes fixas. O operador consegue derivar p50,
// p95 e p99 no intervalo de coleta sem criar uma serie por jogador/opcode.
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

// observeWorldGauges publica os tamanhos que o tick ja conhece.
func observeWorldGauges(players, activeMobs int) {
	metricActivePlayers.Set(int64(players))
	metricActiveMobs.Set(int64(activeMobs))
}
