package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

// This exercises domain settlement through the existing persistence fence.
// It does not enable a castle scheduler, runtime seal sampler, or client flow.
func TestCastleDomainCriticalAcknowledgement(t *testing.T) {
	for _, reject := range []bool{false, true} {
		t.Run(fmt.Sprintf("reject-%t", reject), func(t *testing.T) {
			w, _, _, fallback, base, q, release := criticalTreasuryFixture(t, 0)
			w.guilds.FindByID(1).Fame = 900
			w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Attacker", Fame: 100,
				Members: []model.GuildMember{{Character: "CastleLeader", Rank: model.GuildRankLeader}}})
			start := time.Date(2026, 10, 25, 21, 0, 0, 0, time.UTC)
			if err := w.guilds.BeginCastleWar(start); err != nil {
				t.Fatal(err)
			}
			w.guilds.Wars.Castle.Owner, w.guilds.Wars.Castle.Phase = 1, model.CastleSeal
			w.guilds.Wars.Cities.Territories[0].Owner = 2
			now := start.Add(20 * time.Minute)
			var progress model.CastleSealProgress
			for range model.CastleSealSteps {
				progress.Observe(w.guilds, "CastleLeader", 1046, 1690, true, now)
			}
			published := 0
			settle := func() error {
				return w.queueScheduledGuildChange(func(r *model.GuildRegistry) error {
					return r.CaptureCastle(progress, now)
				}, func() { published++ })
			}
			before := w.criticalGuildSnapshot()
			if reject {
				base.failure = criticalCertifiedRejection(t)
			}
			if err := settle(); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.guilds == nil || len(proposal.accounts) != 0 || proposal.guilds.FindByID(1).Fame != 250 || proposal.guilds.FindByID(2).Fame != 750 || proposal.guilds.Wars.Castle.Owner != 2 || !proposal.guilds.Wars.Castle.Captured {
				t.Fatal("castle settlement was not one detached registry proposal")
			}
			assertRetained := func() {
				t.Helper()
				if !reflect.DeepEqual(w.criticalGuildSnapshot(), before) || published != 0 || fallback.gameSaves != 0 {
					t.Fatal("unacknowledged castle settlement escaped to live state")
				}
			}
			assertRetained()
			if err := settle(); !errors.Is(err, errCriticalPersistenceBusy) {
				t.Fatal("pending castle settlement did not fence duplicates", err)
			}
			release()
			result := awaitCriticalResult(t, q)
			assertRetained()
			w.applyCriticalCompletion(result)
			if reject {
				assertRetained()
				if w.criticalGuildReserved() || w.shuttingDown {
					t.Fatal("certified rejection did not release the castle fence")
				}
				base.failure = nil
				if err := settle(); err != nil {
					t.Fatal(err)
				}
				awaitCriticalWrite(t, base.entered)
				w.applyCriticalCompletion(awaitCriticalResult(t, q))
			}
			if w.shuttingDown || w.criticalGuildReserved() || published != 1 || len(q.pending) != 0 || !reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) {
				t.Fatal("castle acknowledgement did not publish exactly the committed registry")
			}
			w.applyCriticalCompletion(result)
			if published != 1 || w.guilds.FindByID(2).Fame != 750 {
				t.Fatal("duplicate acknowledgement repeated castle settlement")
			}
		})
	}
}
