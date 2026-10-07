package game

import (
	"bytes"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Exercise real World consumers with a held worker, not a synthetic fence.
// Completing the worker alone must not release the owner's reservation.
func holdCriticalCombatParticipant(t *testing.T, w *World, p *Player) (func(), *criticalPersistence) {
	t.Helper()
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	// Kill rewards persist every party account in one batch transaction.
	queue := newCriticalTestQueue(t, &criticalBatchTestStore{backend}, 1, 1)
	t.Cleanup(release)
	w.indexPlayerCharacter(p)
	if !w.claimAccountSession(p.Session, p.Account.Name) {
		t.Fatal("fixture account admission failed")
	}
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{p.Account}}, []*Player{p},
		func(criticalPersistenceResult) error { return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	return release, queue
}

// claimCriticalTestPlayers gives fixture players the account ownership and
// character index every production in-world player has, so they can be bound
// as persistence participants.
func claimCriticalTestPlayers(t *testing.T, w *World, players ...*Player) {
	t.Helper()
	for _, p := range players {
		if p.Char != nil && p.Char.UID == "" {
			p.Char.UID = "critical-test-" + p.Char.Name
		}
		w.indexPlayerCharacter(p)
		if !w.claimAccountSession(p.Session, p.Account.Name) {
			t.Fatalf("fixture account %q admission failed", p.Account.Name)
		}
	}
}

func assertCriticalCombatUnchanged(t *testing.T, p *Player, snapshot model.Char, lastAttacker uint16, deadAt time.Time) {
	t.Helper()
	if !reflect.DeepEqual(*p.Char, snapshot) || p.LastAttackerID != lastAttacker || !p.DeadAt.Equal(deadAt) {
		t.Fatal("reserved participant's character or combat lifecycle changed")
	}
	if got := p.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("reserved participant received %d packets", got)
	}
}

func TestCriticalCombatTargetsRemainFrozenUntilOwnerAcknowledgement(t *testing.T) {
	for _, scenario := range []string{"physical-pvp", "magic-pvp", "mob-physical", "boss-magic", "common-mob-damage", "mob-actions"} {
		t.Run(scenario, func(t *testing.T) {
			w, attacker, target, mob := manaControlCombatFixture(t, true)
			if scenario == "magic-pvp" {
				attacker.Char.Class, attacker.Char.LearnedSkill = 1, 1
				w.recalcPlayer(attacker.Char)
			}
			w.skills = map[int]model.SkillDef{24: {Index: 24, InstanceType: 2, InstanceValue: 100, Range: 6, MaxTarget: 1, Aggressive: 1}}
			mob.Awake, mob.TargetID = true, target.ID
			w.activeMobs[mob.ID] = mob
			hit := func() {
				switch scenario {
				case "physical-pvp":
					w.onAttack(attacker.Session, physicalAttackPacket(1000, target.ID, target.X, target.Y))
				case "magic-pvp":
					w.onSkillAttack(attacker, skillCastRequest{Skill: 24, TargetID: target.ID})
				case "mob-physical":
					w.mobAttackPlayer(mob, target, w.now())
				case "boss-magic":
					w.bossCastSkill(&BossRuntime{Profile: &BossProfile{ID: "reservation-test"}}, mob,
						BossActionDef{SkillID: 24, CastRange: 6}, BossEvent{SourceID: target.ID})
				case "common-mob-damage":
					w.applyMobDamageToPlayer(mob, target, 100, w.now(), func(damage uint32) []byte {
						return wire.AttackHit(mob.ID, target.ID, mob.X, mob.Y, target.X, target.Y, damage, playerMaxHP(target.Char), 0, 0)
					})
				case "mob-actions":
					w.tickMobCombat(w.now(), 0, 1, true)
					w.tickActiveMobActions(w.now())
				}
			}
			snapshot := cloneCharacterState(target.Char)
			lastAttacker, deadAt := target.LastAttackerID, target.DeadAt
			beforeMove, beforeAttack := mob.NextMove, mob.NextAttack
			release, queue := holdCriticalCombatParticipant(t, w, target)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				hit()
				assertCriticalCombatUnchanged(t, target, snapshot, lastAttacker, deadAt)
				if mob.TargetID != target.ID || !mob.NextMove.Equal(beforeMove) || !mob.NextAttack.Equal(beforeAttack) {
					t.Fatalf("%s consumed the mob target or deadline", phase)
				}
			}
			w.processCriticalCompletions(1)
			if w.criticalPlayerReserved(target) {
				t.Fatal("owner acknowledgement retained a successful reservation")
			}
			hit()
			if playerCurHP(target.Char) >= playerCurHP(&snapshot) || playerCurMP(target.Char) >= playerCurMP(&snapshot) {
				t.Fatal("real combat and Mana Control did not resume after acknowledgement")
			}
		})
	}
}

func TestCriticalPlayerSkillPreflightsEveryParticipantBeforeCasterCosts(t *testing.T) {
	for _, scenario := range []string{"caster", "first-target", "last-target"} {
		for _, index := range []int{22, 30} {
			t.Run(scenario+"/"+map[int]string{22: "mana-and-movement", 30: "hp-sacrifice"}[index], func(t *testing.T) {
				w, caster, target, _ := manaControlCombatFixture(t, true)
				first, _ := networkedTestPlayer(3, "First", 2101, 2100)
				first.Char.Score.MaxHP, first.Char.Score.CurHP = 100_000, 100_000
				w.recalcPlayer(first.Char)
				w.players[first.Session], w.playersByID[first.ID] = first, first
				w.indexPlayerCharacter(first)
				w.updatePlayerSpatial(first)
				participants := []*Player{caster, first, target}
				reserved := map[string]*Player{"caster": caster, "first-target": first, "last-target": target}[scenario]
				snapshots := make([]model.Char, len(participants))
				for i, p := range participants {
					snapshots[i] = cloneCharacterState(p.Char)
					drainPlayerPackets(p)
				}
				x, y := caster.X, caster.Y
				skill := model.SkillDef{Index: index, InstanceType: 2, InstanceValue: 100, MaxTarget: 2}
				release, queue := holdCriticalCombatParticipant(t, w, reserved)
				for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					w.executePlayerSkill(caster, []*Player{first, target}, skill, 1, 0)
					for i, p := range participants {
						assertCriticalCombatUnchanged(t, p, snapshots[i], 0, time.Time{})
					}
					if caster.X != x || caster.Y != y {
						t.Fatal("rejected skill moved the caster")
					}
				}
				w.processCriticalCompletions(1)
				w.executePlayerSkill(caster, []*Player{first, target}, skill, 1, 0)
				if reflect.DeepEqual(*caster.Char, snapshots[0]) || playerCurHP(target.Char) >= playerCurHP(&snapshots[2]) {
					t.Fatal("caster cost and multi-target damage did not resume")
				}
			})
		}
	}
}

func TestCriticalChaosRecoveryDoesNotConsumeReservedDeadline(t *testing.T) {
	for _, scenario := range []string{"negative-due", "neutral-due", "initial-deadline"} {
		t.Run(scenario, func(t *testing.T) {
			w, _, p, _ := manaControlCombatFixture(t, false)
			st := w.store.(*craftStore)
			p.Char.CP, p.NextCPRecovery = -2, w.now().Add(-time.Second)
			if scenario == "neutral-due" {
				p.Char.CP = 0
			} else if scenario == "initial-deadline" {
				p.NextCPRecovery = time.Time{}
			}
			beforeCP, beforeDeadline := p.Char.CP, p.NextCPRecovery
			release, queue := holdCriticalCombatParticipant(t, w, p)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickChaosRecovery(w.now())
				if p.Char.CP != beforeCP || !p.NextCPRecovery.Equal(beforeDeadline) || st.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatalf("%s consumed reserved CP state or published it", phase)
				}
			}
			w.processCriticalCompletions(1)
			w.tickChaosRecovery(w.now())
			if scenario == "negative-due" {
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
			}
			if !p.NextCPRecovery.Equal(w.now().Add(chaosRecoveryInterval)) {
				t.Fatal("CP deadline did not resume after acknowledgement")
			}
			if scenario == "negative-due" && (p.Char.CP != beforeCP+1 || st.saves != 0) {
				t.Fatal("due negative CP was not recovered and saved")
			}
		})
	}
}

func TestCriticalTimedItemsRetainAllAccountLocationsUntilAcknowledgement(t *testing.T) {
	w, _, p, _ := manaControlCombatFixture(t, false)
	st := w.store.(*craftStore)
	p.Account.Chars = append(p.Account.Chars, model.Char{UID: "inactive-character"})
	p.Char = &p.Account.Chars[0]
	item := model.Item{Index: 1, UID: "expired", ActivatedUnix: w.now().Add(-time.Hour).Unix(), ExpiresUnix: w.now().Add(-time.Second).Unix()}
	p.Char.Equip[0], p.Char.Inv[0], p.Account.Cargo[0] = item, item, item
	p.Account.Chars[1].Inv[0] = item
	// These are separate item instances; their persisted identities must survive.
	p.Char.Equip[0].UID, p.Char.Inv[0].UID = "expired-equipped", "expired-carried"
	p.Account.Cargo[0].UID, p.Account.Chars[1].Inv[0].UID = "expired-cargo", "expired-inactive"
	snapshot := snapshotTimedAccountItems(p.Account)
	release, queue := holdCriticalCombatParticipant(t, w, p)
	for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		if err := w.prepareTimedEquipmentForEnter(p, w.now()); !errors.Is(err, errCriticalPersistenceBusy) {
			t.Fatalf("%s entry preparation error = %v", phase, err)
		}
		w.nextTimedItemSweep = time.Time{}
		w.tickTimedItems(w.now())
		if !reflect.DeepEqual(snapshotTimedAccountItems(p.Account), snapshot) || st.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatalf("%s removed or published reserved timed items", phase)
		}
	}
	w.processCriticalCompletions(1)
	w.tickTimedItems(w.now().Add(timedItemSweepInterval))
	if !reflect.DeepEqual(snapshotTimedAccountItems(p.Account), snapshot) || st.saves != 0 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("eligible expiry applied before its own persistence acknowledgement")
	}
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Equip[0].Index != 0 || p.Char.Inv[0].Index != 0 || p.Account.Cargo[0].Index != 0 || p.Account.Chars[1].Inv[0].Index != 0 || st.saves != 0 || len(queue.pending) != 0 {
		t.Fatal("expired instances were not removed after detached persistence acknowledgement")
	}
	if p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("successful expiry did not publish removals")
	}
}

func TestCriticalTimedEntryRejectsBeforeMetadataProcessing(t *testing.T) {
	for _, scenario := range []string{"empty-account", "valid-live-item", "invalid-live-metadata"} {
		t.Run(scenario, func(t *testing.T) {
			w, _, p, _ := manaControlCombatFixture(t, false)
			w.items = map[uint16]model.ItemDef{4300: premiumTestDef(4300, fairySlot)}
			if scenario != "empty-account" {
				p.Char.Equip[fairySlot] = model.Item{Index: 4300, UID: "live-timed-item",
					ActivatedUnix: w.now().Add(-time.Hour).Unix(), ExpiresUnix: w.now().Add(time.Hour).Unix()}
				if scenario == "invalid-live-metadata" {
					p.Char.Equip[fairySlot].ActivatedUnix = 0
				}
			}
			snapshot := snapshotTimedAccountItems(p.Account)
			release, queue := holdCriticalCombatParticipant(t, w, p)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				if err := w.prepareTimedEquipmentForEnter(p, w.now()); !errors.Is(err, errCriticalPersistenceBusy) {
					t.Fatalf("%s reservation did not precede metadata processing: %v", phase, err)
				}
				if !reflect.DeepEqual(snapshotTimedAccountItems(p.Account), snapshot) {
					t.Fatal("entry preparation changed reserved metadata")
				}
			}
			w.processCriticalCompletions(1)
			err := w.prepareTimedEquipmentForEnter(p, w.now())
			if scenario == "invalid-live-metadata" {
				if err == nil || err.Error() != "invalid timed item metadata" {
					t.Fatalf("metadata validation did not resume: %v", err)
				}
			} else if err != nil {
				t.Fatalf("unchanged entry did not resume: %v", err)
			}
		})
	}
}

func TestCriticalPvPKillsPreflightKillerAndAllVictims(t *testing.T) {
	for _, scenario := range []string{"killer", "last-victim"} {
		t.Run(scenario, func(t *testing.T) {
			killer, _ := networkedTestPlayer(1, "Killer", 2100, 2100)
			first, _ := networkedTestPlayer(2, "First", 2101, 2100)
			last, _ := networkedTestPlayer(3, "Last", 2102, 2100)
			w, st := guildFlowWorld(killer, first, last)
			killer.Char.CP, first.Char.CP, last.Char.CP = 0, -10, -10
			participants := []*Player{killer, first, last}
			snapshots := make([]model.Char, len(participants))
			for i, p := range participants {
				snapshots[i] = cloneCharacterState(p.Char)
			}
			reserved := last
			if scenario == "killer" {
				reserved = killer
			}
			for _, p := range participants {
				if p != reserved {
					claimCriticalTestPlayers(t, w, p)
				}
			}
			release, queue := holdCriticalCombatParticipant(t, w, reserved)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.applyPvPKills(killer, first, last)
				for i, p := range participants {
					assertCriticalCombatUnchanged(t, p, snapshots[i], 0, time.Time{})
				}
				if st.gameSaves != 0 {
					t.Fatal("rejected kill batch persisted")
				}
			}
			w.processCriticalCompletions(1)
			w.applyPvPKills(killer, first, last)
			// The resumed batch is one detached three-account write.
			if killer.Char.CP != 0 || first.Char.CP != -10 {
				t.Fatal("PvP penalties became visible before acknowledgement")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if killer.Char.CP != -6 || first.Char.CP != -9 || last.Char.CP != -9 || st.gameSaves != 0 || len(queue.pending) != 0 {
				t.Fatalf("kill batch did not resume atomically: CP=%d/%d/%d saves=%d", killer.Char.CP, first.Char.CP, last.Char.CP, st.gameSaves)
			}
		})
	}
}

func TestCriticalHiddenCasterRetainsAffectUntilAcknowledgement(t *testing.T) {
	w, caster, target, _ := manaControlCombatFixture(t, false)
	caster.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: w.now().Add(time.Minute)}
	snapshot := cloneCharacterState(caster.Char)
	release, queue := holdCriticalCombatParticipant(t, w, caster)
	for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		if w.canInitiatePvP(caster, target) || w.canInitiatePvP(target, caster) {
			t.Fatal("reserved participant passed PvP authorization")
		}
		w.breakHideOnAttack(caster)
		assertCriticalCombatUnchanged(t, caster, snapshot, 0, time.Time{})
	}
	w.processCriticalCompletions(1)
	w.breakHideOnAttack(caster)
	if hasActiveAffectAt(caster.Char, 28, w.now()) || !w.canInitiatePvP(caster, target) {
		t.Fatal("hide removal or ordinary PK authorization did not resume")
	}
}

func TestCriticalMobChaseRetainsTargetAndMovementDeadline(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Target", 103, 100)
	mob := combatMob(1000, 100, 100)
	mob.TargetID = p.ID
	w := worldWithNetworkedPlayers(p)
	w.store = &craftStore{}
	w.registerMobSpatial(mob)
	w.mobs = append(w.mobs, mob)
	w.activeMobs[mob.ID] = mob
	p.show(mob.ID)
	now := time.Date(2026, 10, 4, 12, 0, 0, 0, time.UTC)
	release, queue := holdCriticalCombatParticipant(t, w, p)
	for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.tickMobCombat(now, 0, 1, true)
		if mob.X != 100 || mob.Y != 100 || !mob.NextMove.IsZero() || mob.TargetID != p.ID || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatalf("%s advanced a reserved target's chase", phase)
		}
	}
	w.processCriticalCompletions(1)
	w.tickMobCombat(now, 0, 1, true)
	if chebyshev(mob.X, mob.Y, p.X, p.Y) > mobAttackRange || mob.NextMove.IsZero() || p.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("real chase and movement publication did not resume")
	}
}

func TestCriticalPlayerSkillRejectsInvalidParticipantsBeforeMutation(t *testing.T) {
	for _, scenario := range []string{"nil-caster", "caster-without-character", "empty-targets", "nil-first-target", "nil-last-target", "target-without-character"} {
		t.Run(scenario, func(t *testing.T) {
			w, caster, target, _ := manaControlCombatFixture(t, false)
			casterSnapshot, targetSnapshot := cloneCharacterState(caster.Char), cloneCharacterState(target.Char)
			skillCaster, targets := caster, []*Player{target}
			switch scenario {
			case "nil-caster":
				skillCaster = nil
			case "caster-without-character":
				skillCaster = &Player{}
			case "empty-targets":
				targets = nil
			case "nil-first-target":
				targets = []*Player{nil, target}
			case "nil-last-target":
				targets = append(targets, nil)
			case "target-without-character":
				targets = append(targets, &Player{})
			}
			w.executePlayerSkill(skillCaster, targets, model.SkillDef{Index: 30, InstanceType: 2, InstanceValue: 100}, 1, 0)
			assertCriticalCombatUnchanged(t, caster, casterSnapshot, 0, time.Time{})
			assertCriticalCombatUnchanged(t, target, targetSnapshot, 0, time.Time{})
		})
	}
}

func TestPlayerSkillDeathNotificationUsesEnglishKillerKind(t *testing.T) {
	w, caster, target, _ := manaControlCombatFixture(t, false)
	setPlayerCurHP(target.Char, 1)
	w.executePlayerSkill(caster, []*Player{target}, model.SkillDef{Index: 24, InstanceType: 2, InstanceValue: 100}, 1, 0)
	if playerCurHP(target.Char) != 0 {
		t.Fatal("fixture did not exercise the lethal PvP skill path")
	}
	found := false
	for {
		packet, ok := target.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid encrypted combat packet")
		}
		found = found || bytes.Contains(packet, []byte("!You were killed by player \"Attacker\"."))
	}
	if !found {
		t.Fatal("lethal PvP skill did not publish the English death notification")
	}
}
