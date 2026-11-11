package game

import (
	"reflect"
	"testing"
)

func TestAdvancedCompositionAdmissionReportsDomainAndPersistenceFailures(t *testing.T) {
	testCase := advancedCraftCases()[0]
	t.Run("invalid recipe returns the combine failure", func(t *testing.T) {
		w, p, _, backend, queue, _, _, _ := criticalAdvancedFixture(t, testCase)
		before := accountStateSnapshot(p.Account)
		packetsBefore := p.Session.QueuedPacketsForTest()

		w.admitAdvancedComposition(p, combineRequest{}, testCase.artisan)

		if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 0 ||
			len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() <= packetsBefore {
			t.Fatal("invalid recipe mutated state, started persistence, or omitted the combine failure")
		}
	})

	t.Run("stopping persistence returns unavailable", func(t *testing.T) {
		w, p, _, backend, queue, req, _, _ := criticalAdvancedFixture(t, testCase)
		before := accountStateSnapshot(p.Account)
		packetsBefore := p.Session.QueuedPacketsForTest()
		w.shuttingDown = true

		w.admitAdvancedComposition(p, req, testCase.artisan)

		if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || len(queue.pending) != 0 ||
			len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() <= packetsBefore {
			t.Fatal("persistence rejection mutated state, started a write, or omitted unavailable feedback")
		}
	})
}
