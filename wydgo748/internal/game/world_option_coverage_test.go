package game

import (
	"testing"

	"wydgo/internal/lang"
	"wydgo/internal/model"
)

func TestWorldOptionsInstallOptionalRuntimeDependencies(t *testing.T) {
	w := &World{}
	spawn := model.CharacterSpawn{X: 1200, Y: 1300}
	WithLoadtestSpawn(spawn, "  TEST-")(w)
	if w.loadtestSpawn != spawn || w.loadtestAccountPrefix != "test-" {
		t.Fatalf("load-test spawn option = %+v, prefix=%q", w.loadtestSpawn, w.loadtestAccountPrefix)
	}

	cluster := &ChannelCluster{}
	outbox := &ChannelOutbox{}
	WithChannelCluster(cluster)(w)
	WithChannelOutbox(outbox)(w)
	WithShutdownRequest(func(string) {})(w)
	WithPanicRestart()(w)
	WithAsyncEntryReads()(w)
	WithShutdownSaveWorkers(3)(w)
	authorizer := func(string, string) bool { return true }
	WithCommandAuthorizer(authorizer)(w)
	catalog := lang.Default()
	WithMessages(catalog)(w)
	WithLanguage(lang.Portuguese)(w)
	WithGameplayLog(" quiet ")(w)
	if w.cluster != cluster || w.outbox != outbox || w.staffOps.request == nil || !w.panicRestart ||
		!w.asyncEntryReads || w.shutdownSaveWorkers != 3 || w.commandAuthorizer == nil ||
		w.messages != catalog || w.language != lang.Portuguese || w.gameplayLogMode != gameplayLogQuiet {
		t.Fatal("optional world runtime dependencies were not installed")
	}
	WithGameplayLog("verbose")(w)
	if w.gameplayLogMode != gameplayLogVerbose {
		t.Fatal("verbose gameplay logging option was not installed")
	}
	WithGameplayLog("invalid")(w)
	if w.gameplayLogMode != gameplayLogSummary {
		t.Fatal("invalid gameplay logging option did not select the safe summary mode")
	}
}
