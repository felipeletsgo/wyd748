package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func TestCriticalGuildMembershipSettlement(t *testing.T) {
	for _, operation := range []string{"promotion", "expulsion", "succession", "alliance", "break-alliance", "war", "cancel-war"} {
		for _, outcome := range []string{"commit", "rejection", "unknown", "stale"} {
			t.Run(operation+"/"+outcome, func(t *testing.T) {
				f := criticalGuildJoinFixture(t, false)
				w, actor, target := f.w, f.actor, f.target
				target.Char.GuildID, target.Char.GuildRank = 1, model.GuildRankMember
				w.guilds.Guilds[0].Members = append(w.guilds.Guilds[0].Members, model.GuildMember{
					Character: target.Char.Name, Account: target.Account.Name, Rank: model.GuildRankMember,
				})
				w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Other"})
				if operation == "break-alliance" {
					w.guilds.Guilds[0].Ally = 2
				}
				if operation == "cancel-war" {
					w.guilds.Guilds[0].WarTarget, w.guilds.Guilds[1].WarTarget = 2, 1
				}
				if outcome == "rejection" {
					f.base.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					f.base.failure = errors.New("commit acknowledgement lost")
				}
				registry := w.guilds
				beforeGuilds := w.criticalGuildSnapshot()
				actorBefore, targetBefore := accountStateSnapshot(actor.Account), accountStateSnapshot(target.Account)
				request := func() error {
					guild := w.guilds.FindByID(1)
					switch operation {
					case "promotion":
						return w.queueGuildPromotion(actor, target, guild, model.GuildRankLeader-1)
					case "expulsion":
						return w.queueGuildDeparture(actor, target, guild, false)
					case "succession":
						return w.queueGuildDeparture(actor, actor, guild, true)
					case "alliance":
						return w.queueGuildAlliance(actor, guild, 2)
					case "break-alliance":
						return w.queueGuildAlliance(actor, guild, 0)
					case "war":
						return w.queueGuildWarDeclaration(actor, guild, 2)
					default:
						return w.queueGuildWarDeclaration(actor, guild, 0)
					}
				}
				if err := request(); err != nil {
					t.Fatal(err)
				}
				proposal := awaitCriticalWrite(t, f.base.entered)
				if proposal.guilds == nil || proposal.guilds == registry || len(proposal.accounts) == 0 {
					t.Fatal("membership did not prepare a detached combined transaction")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						f.release()
						awaitReadyCriticalResults(t, f.q, 1)
					}
					if !errors.Is(request(), errCriticalPersistenceBusy) || len(f.base.entered) != 0 ||
						!reflect.DeepEqual(w.criticalGuildSnapshot(), beforeGuilds) ||
						!reflect.DeepEqual(accountStateSnapshot(actor.Account), actorBefore) ||
						!reflect.DeepEqual(accountStateSnapshot(target.Account), targetBefore) ||
						actor.Session.QueuedPacketsForTest() != 0 || target.Session.QueuedPacketsForTest() != 0 {
						t.Fatalf("%s membership mutated, replayed or published before acknowledgement", phase)
					}
				}
				if outcome == "stale" {
					w.guilds.Guilds[0].Name = "Replacement"
					beforeGuilds = w.criticalGuildSnapshot()
				}
				w.processCriticalCompletions(1)
				if outcome == "commit" {
					if w.shuttingDown || w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) ||
						len(f.q.pending) != 0 || w.criticalGuildReserved() || actor.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("membership did not settle the committed registry exactly once")
					}
					for _, account := range proposal.accounts {
						p := actor
						if account.Name == target.Account.Name {
							p = target
						}
						if !reflect.DeepEqual(accountPersistenceSnapshot(p.Account), account) {
							t.Fatal("membership account differs from its committed proposal")
						}
					}
				} else {
					if !reflect.DeepEqual(w.criticalGuildSnapshot(), beforeGuilds) ||
						!reflect.DeepEqual(accountStateSnapshot(actor.Account), actorBefore) ||
						!reflect.DeepEqual(accountStateSnapshot(target.Account), targetBefore) {
						t.Fatal("failed membership installed an unverified proposal")
					}
					if outcome == "rejection" {
						if w.shuttingDown || len(f.q.pending) != 0 || w.criticalGuildReserved() || actor.Session.QueuedPacketsForTest() != 1 {
							t.Fatal("certified rejection did not release without success publication")
						}
					} else if !w.shuttingDown || len(f.q.pending) != 1 || !w.criticalGuildReserved() || actor.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("unverified membership released fences or published success")
					}
				}
				packets := actor.Session.QueuedPacketsForTest()
				w.processCriticalCompletions(1)
				if actor.Session.QueuedPacketsForTest() != packets || f.fallback.saves != 0 || f.fallback.gameSaves != 0 || len(f.base.entered) != 0 {
					t.Fatal("membership completion replayed or used inline persistence")
				}
			})
		}
	}
}

func TestCriticalTaxedMerchantSettlement(t *testing.T) {
	for _, operation := range []string{"buy", "ticket", "sale"} {
		for _, outcome := range []string{"commit", "rejection", "unknown", "stale-city"} {
			t.Run(operation+"/"+outcome, func(t *testing.T) {
				w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
				p.Char.Gold = 10000
				p.Char.Inv[5] = model.Item{Index: 400, UID: "merchant-source"}
				if outcome == "rejection" {
					base.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					base.failure = errors.New("merchant acknowledgement lost")
				}
				before, guildsBefore := accountStateSnapshot(p.Account), w.criticalGuildSnapshot()
				request := func() error {
					switch operation {
					case "buy":
						return w.queueShopPurchase(p, model.Item{Index: 400}, 1100, 25, 0, 3200, 0)
					case "ticket":
						return w.queueTicketPurchase(p, model.Item{Index: 4147, UID: "merchant-ticket"}, 6, 3300, 75, 0)
					default:
						return w.queueShopSale(p, 5, 225, 6, 0)
					}
				}
				if err := request(); err != nil {
					t.Fatal(err)
				}
				proposal := awaitCriticalWrite(t, base.entered)
				release()
				awaitReadyCriticalResults(t, q, 1)
				if !errors.Is(request(), errCriticalPersistenceBusy) || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
					!reflect.DeepEqual(w.criticalGuildSnapshot(), guildsBefore) || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("taxed merchant published or mutated before acknowledgement")
				}
				if outcome == "stale-city" {
					w.guilds.Wars.Cities.Territories[0].Owner = 2
					guildsBefore = w.criticalGuildSnapshot()
				}
				w.processCriticalCompletions(1)
				if outcome == "commit" {
					if w.shuttingDown || len(q.pending) != 0 || !reflect.DeepEqual(accountPersistenceSnapshot(p.Account), proposal.accounts[0]) ||
						!reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) || p.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("tax and account did not apply one committed transaction")
					}
				} else {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), guildsBefore) {
						t.Fatal("failed merchant installed an unverified debit or treasury change")
					}
					if outcome == "rejection" {
						if w.shuttingDown || len(q.pending) != 0 || p.Session.QueuedPacketsForTest() != 1 {
							t.Fatal("certified merchant rejection did not settle without success")
						}
					} else if !w.shuttingDown || len(q.pending) != 1 || !w.criticalGuildReserved() || p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("unknown merchant released the shared city fence")
					}
				}
				if fallback.saves != 0 || fallback.gameSaves != 0 || len(base.entered) != 0 {
					t.Fatal("merchant bypassed the detached adapter or automatically retried")
				}
			})
		}
	}
}
