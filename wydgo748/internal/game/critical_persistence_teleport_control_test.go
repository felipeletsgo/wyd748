package game

import (
	"context"
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/control"
)

func TestCriticalStaffTeleportRetainsPendingAndTerminalReceipts(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection", "unknown", "stale-uid", "closed", "timeout"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
			w.warConfig.Enabled = false
			// Staff commands accept 1..12 lowercase account characters.
			w.releaseAccountSession(p.Session, p.Account)
			p.Account.Name = "staffacct"
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("staff fixture account admission failed")
			}
			p.ShopNPC, p.CargoNPC = 77, 88
			p.MoveAuthorityRoute = []byte{'1'}
			before := accountStateSnapshot(p.Account)
			if outcome == "rejection" {
				base.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				base.failure = errors.New("staff teleport acknowledgement lost")
			}
			c := control.TeleportCommand{Version: 1, OperationID: "critical-teleport-0001", Epoch: w.moderationEpoch(),
				UID: p.Char.UID, Account: accountSessionKey(p.Account.Name), ExpectedSessionID: p.Session.ID,
				Reason: "Recover a stuck character", Destination: "azran"}
			ctx, cancel := context.WithCancel(context.Background())
			defer cancel()
			request := func() *teleportRequest {
				return &teleportRequest{ctx: context.Background(), actor: "operator", command: c,
					authorize: func() bool { return true }, reply: make(chan control.TeleportResult, 1)}
			}
			req := request()
			req.ctx = ctx
			w.handleTeleport(req)
			proposal := awaitCriticalWrite(t, base.entered)
			if outcome == "timeout" {
				cancel()
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, q, 1)
				}
				retry := request()
				w.handleTeleport(retry)
				result := <-retry.reply
				if result.Code != "operation_pending" || !result.Replayed || len(req.reply) != 0 ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.ShopNPC != 77 || p.CargoNPC != 88 ||
					len(p.MoveAuthorityRoute) == 0 || p.Session.QueuedPacketsForTest() != 0 || len(base.entered) != 0 {
					t.Fatalf("%s staff teleport published or replayed the write", phase)
				}
			}
			if outcome == "closed" {
				p.Session.Close()
			} else if outcome == "stale-uid" {
				p.Char.UID = "replacement-staff-target"
				before = accountStateSnapshot(p.Account)
			}
			w.processCriticalCompletions(1)
			retry := request()
			w.handleTeleport(retry)
			result := <-retry.reply
			switch outcome {
			case "commit", "closed", "timeout":
				if result.Code != "ok" || !result.Replayed || len(req.reply) != 1 || result.X != p.X || result.Y != p.Y ||
					!reflect.DeepEqual(accountPersistenceSnapshot(p.Account), proposal.accounts[0]) ||
					p.ShopNPC != 0 || p.CargoNPC != 0 || len(p.MoveAuthorityRoute) != 0 {
					t.Fatal("staff teleport did not settle the committed receipt")
				}
			case "rejection":
				if result.Code != "persistence_failed" || !result.Replayed || len(req.reply) != 1 || w.shuttingDown ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || w.criticalPlayerReserved(p) {
					t.Fatal("staff teleport did not retain its certified rejection receipt")
				}
			default:
				if result.Code != "operation_pending" || !result.Replayed || len(req.reply) != 0 || !w.shuttingDown ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !w.criticalPlayerReserved(p) {
					t.Fatal("unverified staff teleport reported a terminal outcome")
				}
			}
			if len(base.entered) != 0 || fallback.saves != 0 || fallback.gameSaves != 0 {
				t.Fatal("staff receipt replay submitted a write or used actor storage")
			}
		})
	}
}
