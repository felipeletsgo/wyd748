package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestOperationalConfigRejectsEveryPublicBoundary(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*OperationalConfig)
	}{
		{"auth ip low", func(c *OperationalConfig) { c.AuthAttemptsPerMinuteIP = 0 }},
		{"auth account high", func(c *OperationalConfig) { c.AuthAttemptsPerMinuteAccount = 1001 }},
		{"chat local low", func(c *OperationalConfig) { c.ChatLocalPer10Seconds = 0 }},
		{"chat whisper high", func(c *OperationalConfig) { c.ChatWhisperPer10Seconds = 1001 }},
		{"chat global low", func(c *OperationalConfig) { c.ChatGlobalPer10Seconds = 0 }},
		{"clients per ip low", func(c *OperationalConfig) { c.MaxAuthenticatedClientsPerIP = 0 }},
		{"clients per ip high", func(c *OperationalConfig) { c.MaxAuthenticatedClientsPerIP = 10001 }},
		{"channel zero", func(c *OperationalConfig) { c.ChannelID = 0 }},
		{"hash concurrency low", func(c *OperationalConfig) { c.AuthHashConcurrency = 0 }},
		{"hash concurrency high", func(c *OperationalConfig) { c.AuthHashConcurrency = 65 }},
		{"world queue low", func(c *OperationalConfig) { c.WorldCommandQueueCapacity = 63 }},
		{"world queue high", func(c *OperationalConfig) { c.WorldCommandQueueCapacity = 65537 }},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			cfg := DefaultOperationalConfig()
			tc.mutate(&cfg)
			if err := cfg.Validate(); err == nil {
				t.Fatal("configuracao operacional invalida foi aceita")
			}
		})
	}

	cfg := DefaultOperationalConfig()
	if err := cfg.Validate(); err != nil {
		t.Fatalf("configuracao padrao invalida: %v", err)
	}
	w := &World{}
	WithOperationalConfig(cfg)(w)
	if w.operational != cfg {
		t.Fatalf("WithOperationalConfig perdeu campos: got=%+v want=%+v", w.operational, cfg)
	}
}

func TestFixedWindowRateCoversResetNotificationAndCardinality(t *testing.T) {
	now := time.Unix(2_000_000_000, 0)
	states := map[string]*fixedWindowRate{}
	if allowed, notify := allowFixedWindow(states, " User ", now, time.Minute, 1, 2); !allowed || notify {
		t.Fatalf("primeira tentativa=(%v,%v)", allowed, notify)
	}
	if _, ok := states["user"]; !ok {
		t.Fatal("chave nao foi normalizada")
	}
	if allowed, notify := allowFixedWindow(states, "USER", now, time.Minute, 1, 2); allowed || !notify {
		t.Fatalf("primeiro excesso=(%v,%v)", allowed, notify)
	}
	if allowed, notify := allowFixedWindow(states, "user", now, time.Minute, 1, 2); allowed || notify {
		t.Fatalf("excesso repetido=(%v,%v)", allowed, notify)
	}
	if allowed, notify := allowFixedWindow(states, "user", now.Add(time.Minute), time.Minute, 1, 2); !allowed || notify {
		t.Fatalf("janela nova=(%v,%v)", allowed, notify)
	}

	full := map[string]*fixedWindowRate{
		"a": {started: now},
		"b": {started: now},
	}
	if allowed, notify := allowFixedWindow(full, "c", now, time.Minute, 1, 2); allowed || notify {
		t.Fatalf("mapa cheio aceitou nova cardinalidade: (%v,%v)", allowed, notify)
	}
	full["a"].started = now.Add(-time.Minute)
	if allowed, notify := allowFixedWindow(full, "c", now, time.Minute, 1, 2); !allowed || notify {
		t.Fatalf("entrada expirada nao foi coletada: (%v,%v)", allowed, notify)
	}

	future := map[string]*fixedWindowRate{"x": {started: now.Add(time.Minute), count: 99}}
	if allowed, notify := allowFixedWindow(future, "x", now, time.Minute, 1, 2); !allowed || notify {
		t.Fatalf("relogio regressivo nao reiniciou janela: (%v,%v)", allowed, notify)
	}
}

func TestLoginAndChatRateLimitersFailClosedAndUseDefaults(t *testing.T) {
	w := &World{}
	now := time.Unix(2_000_000_000, 0)
	if w.allowLoginAttempt("not-an-ip", "account", now) {
		t.Fatal("origem de rede invalida foi aceita")
	}
	if !w.allowLoginAttempt("127.0.0.1", " Account ", now) {
		t.Fatal("primeira tentativa valida foi recusada")
	}
	if w.operational.AuthAttemptsPerMinuteIP != DefaultOperationalConfig().AuthAttemptsPerMinuteIP ||
		w.operational.AuthAttemptsPerMinuteAccount != DefaultOperationalConfig().AuthAttemptsPerMinuteAccount {
		t.Fatal("limites default nao foram materializados")
	}

	if w.allowChat(nil, "local", now) {
		t.Fatal("chat sem jogador foi aceito")
	}
	p := &Player{
		Account: &model.Account{Name: "chat-account"},
		Session: gameNet.NewTestSession(99, 32),
	}
	for i := 0; i < DefaultOperationalConfig().ChatLocalPer10Seconds; i++ {
		if !w.allowChat(p, "local", now) {
			t.Fatalf("mensagem local %d foi limitada cedo", i+1)
		}
	}
	if w.allowChat(p, "local", now) {
		t.Fatal("excesso de chat local foi aceito")
	}
	if p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("primeiro bloqueio de chat nao publicou aviso")
	}
	if !w.allowChat(p, chatChannelGlobal, now.Add(11*time.Second)) {
		t.Fatal("canal global nao abriu nova janela independente")
	}
}

func TestGameplayLoggingModesCountersAndFlushPolicy(t *testing.T) {
	for input, want := range map[string]gameplayLogMode{
		"quiet": gameplayLogQuiet,
		" VERBOSE ": gameplayLogVerbose,
		"invalid": gameplayLogSummary,
	} {
		w := &World{}
		WithGameplayLog(input)(w)
		if w.gameplayLogMode != want {
			t.Fatalf("WithGameplayLog(%q)=%d, esperado=%d", input, w.gameplayLogMode, want)
		}
	}

	w := &World{gameplayLogMode: gameplayLogSummary}
	for _, kind := range []string{"skill", "attack", "kill", "drop", "gold", "batch", "unknown"} {
		w.gameplayLogf(kind, "contract")
	}
	if w.gameplayLog != (gameplayLogStats{Skills: 1, Attacks: 1, Kills: 1, Drops: 1, Gold: 1, Batches: 1}) {
		t.Fatalf("contadores divergiram: %+v", w.gameplayLog)
	}

	now := time.Unix(2_000_000_000, 0)
	w.nextGameplayLog = now.Add(time.Second)
	w.flushGameplayLog(now, false)
	if w.gameplayLog.Skills != 1 {
		t.Fatal("summary foi limpo antes do deadline")
	}
	w.flushGameplayLog(now.Add(2*time.Second), false)
	if w.gameplayLog != (gameplayLogStats{}) || !w.nextGameplayLog.Equal(now.Add(2*time.Second).Add(gameplayLogSummaryInterval)) {
		t.Fatalf("summary vencido nao foi drenado: stats=%+v next=%v", w.gameplayLog, w.nextGameplayLog)
	}

	quiet := &World{gameplayLogMode: gameplayLogQuiet, gameplayLog: gameplayLogStats{Kills: 3}}
	quiet.flushGameplayLog(now, false)
	if quiet.gameplayLog != (gameplayLogStats{}) || !quiet.nextGameplayLog.Equal(now.Add(gameplayLogSummaryInterval)) {
		t.Fatal("quiet nao descartou contadores")
	}

	verbose := &World{gameplayLogMode: gameplayLogVerbose}
	verbose.gameplayLogf("attack", "GAMEPLAY contract verbose")
	verbose.flushGameplayLog(now, true)
	if verbose.gameplayLog.Attacks != 1 {
		t.Fatal("verbose deveria preservar counters ate o fluxo normal consumir")
	}
}

type contractRNG struct{ value int }

func (r contractRNG) Intn(n int) int {
	if n <= 0 {
		return 0
	}
	return r.value % n
}

func TestPercentRollAndClientFacingErrorContracts(t *testing.T) {
	w := &World{rng: contractRNG{value: 0}}
	fail := w.rollPercent(-10)
	if fail.Roll != 1 || fail.Chance != 0 || fail.Success || fail.message() != "Fail: 1/0" ||
		fail.namedMessage("") != fail.message() || fail.namedMessage("Craft") != "Craft Fail: 1/0" {
		t.Fatalf("roll de falha divergiu: %+v %q", fail, fail.namedMessage("Craft"))
	}
	w.rng = contractRNG{value: 99}
	success := w.rollPercent(150)
	if success.Roll != 100 || success.Chance != 100 || !success.Success || success.message() != "Success: 100/100" {
		t.Fatalf("roll de sucesso divergiu: %+v", success)
	}

	err := clientError("Visible error")
	if err.Error() != "Visible error" {
		t.Fatalf("clientFacingError=%q", err.Error())
	}
}
