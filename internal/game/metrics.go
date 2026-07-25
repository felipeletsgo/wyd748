package game

import (
	"expvar"
	"sync/atomic"
	"time"
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
	metricCommandDurationMicros = expvar.NewMap("world_command_duration_micros")
	metricCommandsByType        = expvar.NewMap("world_commands_by_type")
	metricActivePlayers         = expvar.NewInt("world_active_players")
	metricActiveMobs            = expvar.NewInt("world_active_mobs")
	// metricPanicsTotal conta panics contidos por safeHandle. Qualquer valor
	// diferente de zero merece investigacao: significa que UM comando deixou
	// estado potencialmente parcial.
	metricPanicsTotal = expvar.NewInt("world_panics_total")
)

// lastTickAt guarda o instante do tick anterior para calcular o lag. E atomico
// porque expvar publica funcoes que podem ser lidas por outra goroutine (o
// handler HTTP), embora a escrita venha sempre do game loop.
var lastTickAt atomic.Int64

// observeTick registra a duracao do tick e o atraso em relacao ao intervalo
// nominal. Chamado pelo proprio tick, com o instante de inicio.
func observeTick(start time.Time, duration time.Duration) {
	metricTickDurationMicros.Set(duration.Microseconds())

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
}

// observeWorldGauges publica os tamanhos que o tick ja conhece.
func observeWorldGauges(players, activeMobs int) {
	metricActivePlayers.Set(int64(players))
	metricActiveMobs.Set(int64(activeMobs))
}
