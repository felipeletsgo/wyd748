package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func prepareWarMedalPlayer(w *World, p *Player) {
	p.Char.UID = fmt.Sprintf("%032x", p.ID)
	w.indexPlayerCharacter(p)
	p.X, p.Y, p.Char.X, p.Char.Y = 2500, 1870, 2500, 1870
	w.updatePlayerSpatial(p)
}

func TestWarMedalEqualChoiceEligibilityAndPendingReplay(t *testing.T) {
	for roll, expected := range []uint16{model.WarMedalGreen, model.WarMedalGold, model.WarMedalRed} {
		t.Run(fmt.Sprint(roll), func(t *testing.T) {
			w, _, a, b, _ := scheduledWarWorld(t, "2026-10-19 21:15")
			w.guilds.FindByID(1).Kingdom, w.guilds.FindByID(2).Kingdom = 7, 8
			prepareWarMedalPlayer(w, a)
			prepareWarMedalPlayer(w, b)
			w.rng = fixedRNG{value: roll}
			w.guilds.Wars.Tower = model.TowerWarState{Day: "2026-10-19", Kingdom: 7, Owner: 1, Captures: 1}
			w.addTowerMedals(w.guilds, "capture")
			w.addTowerMedals(w.guilds, "capture")
			if len(w.guilds.Wars.Medals) != 1 || w.guilds.Wars.Medals[0].Item != expected || w.guilds.Wars.Medals[0].CharacterUID != a.Char.UID {
				t.Fatal("equal roll mapping, realm eligibility or pending replay failed")
			}
			// A separate finish event is a second native reward, not a replay.
			w.addTowerMedals(w.guilds, "finish")
			if len(w.guilds.Wars.Medals) != 2 {
				t.Fatal("finish reward lost")
			}
		})
	}
}

func TestWarMedalDeliverySurvivesOfflineFullInventoryAndSaveFailure(t *testing.T) {
	w, store, a, _, _ := scheduledWarWorld(t, "2026-10-19 22:00")
	prepareWarMedalPlayer(w, a)
	claim := model.WarMedalClaim{Event: "tower/2026-10-19/finish/1", Account: a.Account.Name, CharacterUID: a.Char.UID, Item: model.WarMedalGreen}
	w.guilds.Wars.Medals = []model.WarMedalClaim{claim}
	a.InWorld = false
	if err := w.deliverWarMedal(); err != nil || store.gameSaves != 0 || len(w.guilds.Wars.Medals) != 1 {
		t.Fatal("offline claim was consumed", err)
	}
	a.InWorld = true
	for i := 0; i < model.PlayerCarrySlots; i++ {
		a.Char.Inv[i] = model.Item{Index: 400}
	}
	if err := w.deliverWarMedal(); err != nil || store.gameSaves != 0 || len(w.guilds.Wars.Medals) != 1 {
		t.Fatal("full inventory consumed claim", err)
	}
	a.Char.Inv[5] = model.Item{}
	before := accountStateSnapshot(a.Account)
	store.err = errors.New("save failed")
	if err := w.deliverWarMedal(); err == nil || !reflect.DeepEqual(before, accountStateSnapshot(a.Account)) || len(w.guilds.Wars.Medals) != 1 || a.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("failed delivery changed inventory, consumed claim or published success")
	}
	// Reload the durable pending queue; the rolled color does not change.
	w.guilds.Wars = model.GuildWarState{Medals: []model.WarMedalClaim{claim}}.Clone()
	store.err = nil
	if err := w.deliverWarMedal(); err != nil || len(w.guilds.Wars.Medals) != 0 || a.Char.Inv[5].Index != claim.Item || a.Char.Inv[5].UID == "" {
		t.Fatal("pending reward was not delivered with a unique item identity", err)
	}
	saves := store.gameSaves
	if err := w.deliverWarMedal(); err != nil || store.gameSaves != saves || len(store.durable.Wars.Medals) != 0 {
		t.Fatal("delivered reward replayed", err)
	}
}

func TestCriticalWarMedalAccountAndClaimCommitTogether(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		t.Run(fmt.Sprint(rejected), func(t *testing.T) {
			w, p, _, fallback, backend, queue, release := criticalTreasuryFixture(t, 0)
			prepareWarMedalPlayer(w, p)
			if rejected {
				backend.failure = criticalCertifiedRejection(t)
			}
			w.guilds.Wars.Medals = []model.WarMedalClaim{{Event: "tower/2026-10-19/capture/1", Account: p.Account.Name, CharacterUID: p.Char.UID, Item: model.WarMedalRed}}
			before, wars := accountStateSnapshot(p.Account), w.guilds.Wars.Clone()
			if err := w.deliverWarMedal(); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, backend.entered)
			if proposal.guilds == nil || len(proposal.guilds.Wars.Medals) != 0 || len(proposal.accounts) != 1 || proposal.accounts[0].Chars[p.CharSlot].Inv[0].Index != model.WarMedalRed {
				t.Fatal("claim removal and inventory addition were not one transaction")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				_ = w.deliverWarMedal()
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || !reflect.DeepEqual(wars, w.guilds.Wars) || p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 {
					t.Fatal("delivery published before actor acknowledgement or was duplicated")
				}
			}
			w.processCriticalCompletions(1)
			if fallback.gameSaves != 0 || fallback.saves != 0 {
				t.Fatal("critical delivery used synchronous fallback")
			}
			if rejected {
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || !reflect.DeepEqual(wars, w.guilds.Wars) || w.criticalGuildReserved() {
					t.Fatal("rejected delivery lost pending reward")
				}
			} else if p.Char.Inv[0].Index != model.WarMedalRed || len(w.guilds.Wars.Medals) != 0 || w.criticalGuildReserved() {
				t.Fatal("acknowledgement did not apply delivered reward")
			}
		})
	}
}

func TestCriticalWeeklyFameResetRetainsBidsUntilAcknowledgement(t *testing.T) {
	w, p, _, fallback, backend, queue, release := criticalTreasuryFixture(t, 0)
	w.guilds.Guilds[0].Fame = 321
	w.guilds.Wars.FameResetDay = "2026-10-18"
	w.guilds.Wars.Cities = model.CityWarState{FameBids: true, Phase: "registration", Day: "2026-10-25", Entries: []model.CityWarEntry{{Guild: 1, City: 1, Bid: 42}}}
	w.guilds.Wars.Cities.Territories[1] = model.CityTerritory{Challenger: 1, Bid: 42}
	before := w.criticalGuildSnapshot()
	local := time.Date(2026, 10, 25, 0, 0, 0, 0, time.UTC)
	if pending, err := w.tickGuildWarEconomy(local); !pending || err != nil {
		t.Fatal("reset was not queued", err)
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	if len(proposal.accounts) != 0 || proposal.guilds.Guilds[0].Fame != 0 || proposal.guilds.Wars.Cities.Phase != "waiting" || proposal.guilds.Wars.Cities.Territories[1].Bid != 42 {
		t.Fatal("reset lost accepted bid or did not close registration")
	}
	if !reflect.DeepEqual(before, w.criticalGuildSnapshot()) || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("reset changed live balances before commit")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if w.guilds.Guilds[0].Fame != 0 || w.guilds.Wars.FameResetDay != "2026-10-25" || fallback.gameSaves != 0 {
		t.Fatal("reset acknowledgement failed")
	}
	w.guilds.Guilds[0].Fame = 5
	if pending, err := w.tickGuildWarEconomy(local.Add(time.Hour)); pending || err != nil || w.guilds.Guilds[0].Fame != 5 {
		t.Fatal("reset replayed in same week", err)
	}
}

func TestFameWarGoldEconomyDisabledBeforeFirstTick(t *testing.T) {
	w, _, a, _, _ := scheduledWarWorld(t, "2026-10-19 12:00")
	w.guilds.Wars.FameResetDay = ""
	w.guilds.Wars.Cities.Territories[0] = model.CityTerritory{Owner: 1, Treasury: 100}
	shop := &Mob{X: 2100, Y: 2100, Def: &model.NPCDef{Score: &model.Score{Merchant: 1}}}
	if _, _, taxed := w.cityShopTax(shop); taxed {
		t.Fatal("first-tick gap enabled gold taxation")
	}
	if w.creditCityTreasury(0, 1) == nil || w.withdrawCityTreasury(a, 0) == nil || w.guilds.Wars.Cities.Territories[0].Treasury != 100 {
		t.Fatal("fame wars still allow gold funding or collection")
	}
}

func TestCriticalTowerRoundUsesCastleRealmAfterAcknowledgement(t *testing.T) {
	w, _, _, fallback, backend, queue, release := criticalTreasuryFixture(t, 0)
	w.warConfig = model.DefaultGuildWarConfig()
	w.guilds.Guilds[0].Kingdom = 7
	w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Kingdom: 8})
	w.guilds.Wars.Castle.Owner = 1
	w.guilds.Wars.Tower = model.TowerWarState{Day: "2026-10-18", Owner: 2, Kingdom: 8, Finished: true, DefenderCaptures: 3}
	before := w.guilds.Wars.Clone()
	start := time.Date(2026, 10, 19, 20, 55, 0, 0, time.UTC)
	if err := w.tickCriticalTowerWar(start); err != nil {
		t.Fatal(err)
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	s := proposal.guilds.Wars.Tower
	if s.Owner != 0 || s.Kingdom != 7 || s.InitialKingdom != 7 || s.DefenderCaptures != 3 || !s.EndsAt.Equal(start.Add(65*time.Minute)) {
		t.Fatal("critical start used previous tower winner or lost native divisor", s)
	}
	if !reflect.DeepEqual(before, w.guilds.Wars) {
		t.Fatal("critical calendar changed live state before acknowledgement")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if !reflect.DeepEqual(s, w.guilds.Wars.Tower) || fallback.gameSaves != 0 {
		t.Fatal("critical calendar did not publish committed proposal")
	}
}

func TestCriticalTowerFinishCommitsMedalsAndFameOnce(t *testing.T) {
	w, p, _, fallback, backend, queue, release := criticalTreasuryFixture(t, 0)
	prepareWarMedalPlayer(w, p)
	w.guilds.Guilds[0].Kingdom = 8
	w.guilds.Wars.Tower = model.TowerWarState{Day: "2026-10-19", Owner: 1, Kingdom: 8, InitialKingdom: 7, Captures: 1, DefenderCaptures: 1}
	w.rng = fixedRNG{value: 1}
	before := w.criticalGuildSnapshot()
	if err := w.queueTowerWarFinish(1); err != nil {
		t.Fatal(err)
	}
	proposal := awaitCriticalWrite(t, backend.entered)
	if !proposal.guilds.Wars.Tower.Finished || proposal.guilds.Guilds[0].Fame != 100 || len(proposal.guilds.Wars.Medals) != 1 || proposal.guilds.Wars.Medals[0].Item != model.WarMedalGold {
		t.Fatal("finish marker, guild fame and medal claim were not one proposal")
	}
	if !reflect.DeepEqual(before, w.criticalGuildSnapshot()) || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("finish published unacknowledged rewards")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if !w.guilds.Wars.Tower.Finished || w.guilds.Guilds[0].Fame != 100 || len(w.guilds.Wars.Medals) != 1 || fallback.gameSaves != 0 {
		t.Fatal("finish acknowledgement failed")
	}
	// Even after delivery consumed the pending claim, finish cannot create it again.
	w.guilds.Wars.Medals = nil
	if err := w.queueTowerWarFinish(1); err != nil || len(queue.pending) != 0 || len(w.guilds.Wars.Medals) != 0 || w.guilds.Guilds[0].Fame != 100 {
		t.Fatal("settled tower replayed a consumed medal or fame", err)
	}
}
