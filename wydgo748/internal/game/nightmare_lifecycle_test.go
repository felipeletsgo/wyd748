package game

import (
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func scheduledNightmareTestRule(tier string, startMinute int) model.VolatileRule {
	rule := nightmareTestRule(tier)
	rule.PartyMode = "party"
	rule.Instance.Mode = "shared_timed_zone"
	rule.Instance.FinishPolicy = "respawn_until_timeout"
	rule.Instance.DurationSeconds = 240
	rule.Instance.Schedule = []model.VolatileInstanceWindow{{StartMinute: startMinute, DurationSeconds: 240}}
	return rule
}

func assertNightmareHUD(t *testing.T, p *Player, seconds uint32, survivors ...uint32) {
	t.Helper()
	wantCount := uint32(0)
	if len(survivors) != 0 {
		wantCount = survivors[0]
	}
	var timerSeen, countSeen bool
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid outbound checksum")
		}
		op := wire.ParseHeader(packet).Type
		if op != wire.OpInstanceTime && op != wire.OpInstanceMobs {
			continue
		}
		value := binary.LittleEndian.Uint32(packet[12:16])
		if op == wire.OpInstanceTime {
			timerSeen = true
			if value != seconds {
				t.Fatalf("timer=%d, want=%d", value, seconds)
			}
		} else {
			countSeen = true
			if value != wantCount {
				t.Fatalf("Nightmare survivors=%d, want=%d", value, wantCount)
			}
		}
	}
	if !timerSeen || !countSeen {
		t.Fatalf("missing Nightmare HUD reset: timer=%v count=%v", timerSeen, countSeen)
	}
}

func nightmareTestProtectors(w *World, rule *model.VolatileRule) {
	w.npcs = append(w.npcs, model.NPCDef{Name: "Protected", Tipo: model.TipoNPC,
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 10, CurHP: 10}})
	for i := 0; i < 8; i++ {
		rule.Instance.NightmareProtectors = append(rule.Instance.NightmareProtectors,
			model.VolatileInstanceSpawn{NPC: "Protected", Count: 1, X: uint16(2203 + i%4), Y: uint16(2204 + i/4)})
	}
}

func assertNightmareLevelingBalance(t *testing.T, mob *Mob, tier string) {
	t.Helper()
	want := map[string][2]uint32{
		"normal": {2000, 200000}, "mystic": {5000, 400000}, "arcane": {10000, 600000},
	}[tier]
	if mob == nil || mob.Def == nil || mob.Def.Score == nil {
		t.Fatal("missing Nightmare monster")
	}
	score := mob.Def.Score
	if mob.HP != want[0] || score.MaxHP != want[0] || score.CurHP != want[0] ||
		score.Attack != 1 || score.MagicAttack != 1 || score.Defense != 1 || mob.Def.ExpReward != want[1] {
		t.Fatalf("%s balance: HP=%d score=%+v EXP=%d, want HP=%d attack/defense=1 EXP=%d",
			tier, mob.HP, score, mob.Def.ExpReward, want[0], want[1])
	}
}

func TestLevelingInstanceKillsGrantExperienceWithoutAnyLoot(t *testing.T) {
	cases := []string{"normal", "mystic", "arcane", "outside", "cube"}
	for _, tier := range []string{"normal", "mystic", "arcane"} {
		cases = append(cases, "magic-chamber-"+tier)
		for _, room := range []string{"1", "2", "3", "4", "5", "6", "7", "8", "boss"} {
			cases = append(cases, "water-"+tier+"-"+room)
		}
	}
	for _, tier := range cases {
		for _, full := range []bool{false, true} {
			t.Run(fmt.Sprintf("%s/full=%v", tier, full), func(t *testing.T) {
				w, player, mob, _ := killRewardTestWorld(t, nil)
				nightmare := tier == "normal" || tier == "mystic" || tier == "arcane"
				if tier != "outside" {
					rule := instanceTestRule()
					if nightmare {
						rule = scheduledNightmareTestRule(tier, 0)
					} else {
						rule.Instance.ID = tier
					}
					w.itemInstances = map[string]*ItemInstance{rule.Instance.ID: {Config: *rule.Instance}}
					mob.InstanceID = rule.Instance.ID
				}
				if full {
					for slot := range player.Char.Inv {
						player.Char.Inv[slot] = model.Item{Index: 500}
					}
				}
				// All three item sources and gold would be guaranteed without the exclusion.
				state := &bossSpawnState{mobID: mob.ID, config: model.BossConfig{
					ID: "loot-fixture", Drops: []model.BossDrop{{Item: 500, ChancePercent: 100, Amount: 1}},
				}}
				w.bossSpawns = []*bossSpawnState{state}
				if result := executeEvent(w, eventCommand(w, "nightmare-loot-test"), true); result.Code != "ok" {
					t.Fatalf("global event setup: %s", result.Code)
				}
				beforeInventory, beforeGold := player.Char.Inv, player.Char.Gold
				plan := w.planMobKill(player, mob, 500, 100)
				if plan == nil || plan.killerReward == 0 || plan.bossState != state {
					t.Fatal("loot exclusion removed EXP or boss death bookkeeping")
				}
				if tier != "outside" && tier != "cube" {
					if len(plan.drops) != 0 || plan.gold != 0 || player.Char.Inv != beforeInventory ||
						player.Char.Gold != beforeGold || w.globalDrop.reserved != 0 || w.globalDrop.status.Dropped != 0 {
						t.Fatal("leveling-instance kill planned loot, changed inventory/gold or reserved a global drop")
					}
				} else {
					if len(plan.drops) != 3 || plan.gold == 0 || w.globalDrop.reserved != 1 {
						t.Fatal("loot outside leveling instances was changed or fixture sources were not exercised")
					}
					for _, drop := range plan.drops {
						if (drop.inventoryPos < 0) != full {
							t.Fatal("inventory/ground fallback did not match inventory capacity")
						}
					}
				}
			})
		}
	}
}

func TestLevelingInstanceDamageCapsPhysicalAndSpellResults(t *testing.T) {
	for _, id := range []string{"nightmare", "water-normal-1", "water-mystic-8", "water-arcane-boss",
		"magic-chamber-normal", "magic-chamber-mystic", "magic-chamber-arcane", "cube", "outside"} {
		t.Run(id, func(t *testing.T) {
			w, leader, _, _, clock := instanceTestWorld()
			rule := instanceTestRule()
			rule.Instance.ID = id
			if id == "nightmare" {
				rule = scheduledNightmareTestRule("arcane", 0)
			}
			w.itemInstances[rule.Instance.ID] = &ItemInstance{Config: *rule.Instance}
			mob := &Mob{ID: 2000, Def: &w.npcs[0], InstanceID: rule.Instance.ID}
			if id == "outside" {
				mob.InstanceID = ""
			}
			// Both physical attacks and spells use this final authoritative boundary.
			for _, damage := range []uint32{0, 1, 20} {
				setPlayerCurHP(leader.Char, 100)
				before := playerCurHP(leader.Char)
				w.applyMobDamageToPlayer(mob, leader, damage, clock.Now(), func(uint32) []byte { return nil })
				want := damage
				if id != "cube" && id != "outside" {
					want = min(damage, 1)
				}
				if got := before - playerCurHP(leader.Char); got != want {
					t.Fatalf("incoming damage=%d applied=%d, want=%d", damage, got, want)
				}
			}
			if id != "cube" && id != "outside" && w.levelingInstanceDamage(mob, 999999) != 1 {
				t.Fatal("large physical or spell damage escaped the cap")
			}
		})
	}
}

func TestNightmareProtectorsAreMortalAndScaleOriginalExperience(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC)
	rule := scheduledNightmareTestRule("arcane", 0)
	nightmareTestProtectors(w, &rule)
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil || len(inst.NPCIDs) != 8 || len(inst.MobIDs) != 0 {
		t.Fatal("admission did not publish exactly eight friendly protectors")
	}
	for _, p := range []*Player{leader, member} {
		assertNightmareHUD(t, p, 60, 8)
		for id := range inst.NPCIDs {
			if !p.hasVisible(id) || w.mobsByID[id].Def.IsMonster() {
				t.Fatal("protected NPC was invisible or hostile to a participant")
			}
		}
	}
	clock.Advance(time.Minute)
	w.tickItemInstances(clock.Now())
	var monster *Mob
	for id := range inst.MobIDs {
		monster = w.mobsByID[id]
	}
	if monster == nil {
		t.Fatal("combat did not spawn a hostile monster")
	}
	for _, p := range []*Player{leader, member} {
		assertNightmareHUD(t, p, 900, 8)
		// Prove that protected NPCs, not nearby living players, keep AI active.
		setPlayerCurHP(p.Char, 0)
	}
	for lost := 0; lost < 8; lost++ {
		var guardian *Mob
		for id := range inst.NPCIDs {
			guardian = w.mobsByID[id]
			break
		}
		oldX, oldY := monster.X, monster.Y
		monster.X, monster.Y = guardian.X-1, guardian.Y
		monster.TargetID = 0
		monster.NextMove, monster.NextAttack = time.Time{}, time.Time{}
		w.moveMobSpatial(monster, oldX, oldY)
		w.tickMobCombat(clock.Now(), 0, 1, false)
		if _, protected := inst.NightmareProtectors[monster.TargetID]; !protected || !monster.Awake {
			t.Fatalf("monster did not acquire a protected NPC without a living player: target=%d awake=%v", monster.TargetID, monster.Awake)
		}
		guardian = w.mobsByID[monster.TargetID]
		// Exercise a lethal one-damage hit without bypassing leveling balance.
		guardian.HP = 1
		w.tickActiveMobActions(clock.Now())
		if !guardian.Dead || w.mobsByID[guardian.ID] != nil || len(inst.NPCIDs) != 7-lost || inst.Remaining != 1 {
			t.Fatal("protected death respawned, changed the hostile quota or retained the NPC")
		}
		if got := w.nightmareExperience(monster, 1000); got != uint32(900-lost*100) {
			t.Fatalf("lost=%d: EXP=%d, want=%d", lost+1, got, 900-lost*100)
		}
		w.sendItemInstanceStageStatus(inst)
		assertNightmareHUD(t, leader, 900, uint32(7-lost))
	}
	if w.nightmareExperience(monster, ^uint32(0)) != uint32(uint64(^uint32(0))*20/100) ||
		w.nightmareExperience(&Mob{}, 1000) != 1000 {
		t.Fatal("EXP scaling overflowed or affected a monster outside Nightmare")
	}
	clone := cloneCriticalInstanceMembership(w).itemInstances[inst.Config.ID]
	clone.NightmareProtectors[99] = 0
	delete(clone.NightmareSpawns, monster.ID)
	if len(inst.NightmareProtectors) != 0 || len(inst.NightmareSpawns) != 1 {
		t.Fatal("detached instance membership borrowed live Nightmare maps")
	}

	restored, _, _, _, restoredClock := instanceTestWorld()
	restoredClock.current = clock.Now()
	nightmareRule := scheduledNightmareTestRule("arcane", 0)
	nightmareTestProtectors(restored, &nightmareRule)
	restored.volatiles.Items[100] = nightmareRule
	restored.store = &gambleStore{load: w.instanceStateSnapshot()}
	if err := restored.restoreInstanceState(); err != nil {
		t.Fatal(err)
	}
	resumed := restored.itemInstances[inst.Config.ID]
	if resumed == nil || resumed.NightmareLostMask != 255 || len(resumed.NPCIDs) != 0 ||
		instanceDisplayedMobCount(resumed) != 0 || !resumed.HardDeadline.Equal(inst.HardDeadline) {
		t.Fatal("restoration resurrected protected NPCs or reset their loss/deadline")
	}
}

func TestNightmareScheduledAdmissionThenFifteenMinuteCombat(t *testing.T) {
	for tier, starts := range map[string][]int{
		"normal": {0, 20, 40}, "mystic": {5, 25, 45}, "arcane": {10, 30, 50},
	} {
		for _, start := range starts {
			for _, elapsed := range []int{0, 3} {
				t.Run(fmt.Sprintf("%s/%02d/elapsed%d", tier, start, elapsed), func(t *testing.T) {
					w, leader, member, _, clock := instanceTestWorld()
					clock.current = time.Date(2026, 10, 16, 12, start+elapsed, 0, 0, time.UTC)
					rule := scheduledNightmareTestRule(tier, start)
					w.volatiles.Items[100] = rule
					w.onUseItem(leader.Session, useItemPacket(0, 0))
					inst := w.itemInstances[rule.Instance.ID]
					if inst == nil || !nightmareWaiting(inst) || len(inst.MobIDs) != 0 || inst.Remaining != 0 {
						t.Fatalf("admission spawned monsters or failed: %+v", inst)
					}
					wait := time.Duration(4-elapsed) * time.Minute
					end := clock.Now().Add(wait + 15*time.Minute)
					if !inst.HardDeadline.Equal(end) || leader.Char.Inv[0].Index != 0 {
						t.Fatalf("wrong absolute deadline or ticket charge: %+v", inst)
					}
					for _, p := range []*Player{leader, member} {
						assertNightmareHUD(t, p, uint32(wait/time.Second))
					}
					clock.Advance(wait - time.Second)
					w.tickItemInstances(clock.Now())
					if !nightmareWaiting(inst) || len(inst.MobIDs) != 0 {
						t.Fatal("combat started before admission closed")
					}
					clock.Advance(time.Second)
					w.tickItemInstances(clock.Now())
					if inst.State != "combat" || inst.Remaining != 1 || len(inst.MobIDs) != 1 || !inst.CombatDeadline.Equal(end) {
						t.Fatalf("combat did not start for fifteen minutes: %+v", inst)
					}
					for _, p := range []*Player{leader, member} {
						assertNightmareHUD(t, p, 900)
						for id := range inst.MobIDs {
							if !p.hasVisible(id) || !w.mobsByID[id].Awake {
								t.Fatal("combat spawn was not visible and awake without player movement")
							}
						}
					}
					clock.Advance(14*time.Minute + 59*time.Second)
					w.tickItemInstances(clock.Now())
					if w.itemInstances[rule.Instance.ID] == nil {
						t.Fatal("Nightmare expired before the combat deadline")
					}
					clock.Advance(time.Second)
					w.tickItemInstances(clock.Now())
					if w.itemInstances[rule.Instance.ID] != nil || len(w.mobsByID) != 0 {
						t.Fatal("Nightmare survived its absolute combat deadline")
					}
					for _, p := range []*Player{leader, member} {
						assertNightmareHUD(t, p, 0)
					}
				})
			}
		}
	}
}

func TestNightmareRespawnPreservesAbsoluteCombatDeadline(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC)
	rule := scheduledNightmareTestRule("normal", 0)
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil {
		t.Fatal("admission failed")
	}
	clock.Advance(time.Minute)
	w.tickItemInstances(clock.Now())
	for _, p := range []*Player{leader, member} {
		for _, seconds := range questTimerPackets(t, p) {
			if seconds != 60 && seconds != 900 {
				t.Fatalf("unexpected countdown: %d", seconds)
			}
		}
	}
	clock.Advance(10 * time.Minute)
	for id := range inst.MobIDs {
		mob := w.mobsByID[id]
		w.onItemInstanceMobKilled(mob, clock.Now())
		w.removeMobInstance(mob)
		break
	}
	if inst.Remaining != 1 || len(inst.MobIDs) != 1 || !inst.CombatDeadline.Equal(inst.HardDeadline) || inst.RewardGranted {
		t.Fatalf("respawn changed the combat contract: %+v", inst)
	}
	// A death does not resend the countdown; an explicit status refresh must
	// still report the original absolute end and no monster quota.
	w.sendItemInstanceStageStatus(inst)
	for _, p := range []*Player{leader, member} {
		assertNightmareHUD(t, p, 300)
	}
}

func TestNightmareIndividualMonstersRespawnUntilTimerEnds(t *testing.T) {
	for _, tier := range []string{"normal", "mystic", "arcane"} {
		t.Run(tier, func(t *testing.T) {
			w, leader, member, _, clock := instanceTestWorld()
			clock.current = time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC)
			rule := scheduledNightmareTestRule(tier, 0)
			rule.Instance.Spawns[0].Count = 2
			w.volatiles.Items[100] = rule
			w.onUseItem(leader.Session, useItemPacket(0, 0))
			inst := w.itemInstances[rule.Instance.ID]
			if inst == nil {
				t.Fatal("admission failed")
			}
			for _, p := range []*Player{leader, member} {
				assertNightmareHUD(t, p, 60)
			}
			clock.Advance(time.Minute)
			w.tickItemInstances(clock.Now())
			for _, p := range []*Player{leader, member} {
				assertNightmareHUD(t, p, 900)
			}
			var survivor *Mob
			for id := range inst.MobIDs {
				survivor = w.mobsByID[id]
				break
			}
			deadline := inst.HardDeadline
			for deaths := 0; deaths < 12; deaths++ {
				var victim *Mob
				for id := range inst.MobIDs {
					if id != survivor.ID {
						victim = w.mobsByID[id]
						break
					}
				}
				if victim == nil {
					t.Fatal("population disappeared before the timer ended")
				}
				spawn := inst.NightmareSpawns[victim.ID]
				clock.Advance(time.Minute)
				if deaths%2 == 0 {
					w.killMobState(leader, victim, victim.HP, victim.HP)
				} else {
					w.killMobWithoutPlayer(victim)
				}
				if len(inst.MobIDs) != 2 || len(w.mobsByID) != 2 || len(w.mobs) != 2 ||
					inst.Remaining != 2 || !inst.HardDeadline.Equal(deadline) ||
					!inst.CombatDeadline.Equal(deadline) || inst.RewardGranted || !inst.ExitAt.IsZero() {
					t.Fatalf("individual respawn changed population or lifecycle: %+v", inst)
				}
				if w.mobsByID[survivor.ID] != survivor || survivor.Dead || w.mobsByID[victim.ID] != nil {
					t.Fatal("respawn replaced the survivor or retained the dead monster")
				}
				for id := range inst.MobIDs {
					mob := w.mobsByID[id]
					if id != survivor.ID && (mob.Def != victim.Def || mob.HP != victim.Def.Score.MaxHP || inst.NightmareSpawns[id] != spawn) {
						t.Fatal("replacement lost its monster type, health or spawn origin")
					}
					for _, p := range []*Player{leader, member} {
						if !p.hasVisible(id) || !mob.Awake {
							t.Fatal("respawn did not reach stationary participants")
						}
					}
				}
				for _, p := range []*Player{leader, member} {
					if timers := questTimerPackets(t, p); len(timers) != 0 {
						t.Fatalf("monster death restarted or changed the timer: %v", timers)
					}
				}
			}
			clock.Advance(3 * time.Minute)
			w.killMobWithoutPlayer(survivor)
			if len(inst.MobIDs) != 2 || len(w.mobsByID) != 1 {
				// The expired encounter retains its membership until the same
				// tick's cleanup, but must not replace the final dead monster.
				t.Fatal("a monster respawned at or after the combat deadline")
			}
			w.tickItemInstances(clock.Now())
			if len(w.itemInstances) != 0 || len(w.mobsByID) != 0 {
				t.Fatal("timeout did not remove the remaining population")
			}
		})
	}
}

func TestNightmareRestorePreservesScheduledPhase(t *testing.T) {
	for _, elapsed := range []time.Duration{3 * time.Minute, 4 * time.Minute, 10 * time.Minute, 19 * time.Minute} {
		t.Run(elapsed.String(), func(t *testing.T) {
			w, _, _, _, clock := instanceTestWorld()
			start := time.Date(2026, 10, 16, 12, 0, 0, 0, time.UTC)
			clock.current = start.Add(elapsed)
			rule := scheduledNightmareTestRule("normal", 0)
			w.volatiles.Items[100] = rule
			// Old releases stored entry with no hard deadline and a combat
			// deadline equal to admission close. Restore must migrate that too.
			w.store = &gambleStore{load: &model.InstanceStateSnapshot{
				Version: model.InstanceStateVersion,
				Instances: []model.InstanceRuntimeState{{
					RuntimeID: rule.Instance.ID, ConfigID: rule.Instance.ID, State: "entry",
					ScheduleEnd: start.Add(4 * time.Minute), CombatDeadline: start.Add(4 * time.Minute),
				}},
			}}
			if err := w.restoreInstanceState(); err != nil {
				t.Fatal(err)
			}
			inst := w.itemInstances[rule.Instance.ID]
			if elapsed >= 19*time.Minute {
				if inst != nil || len(w.mobsByID) != 0 {
					t.Fatal("restart resurrected an expired Nightmare")
				}
				return
			}
			if inst == nil || !inst.HardDeadline.Equal(start.Add(19*time.Minute)) {
				t.Fatalf("restart changed the fixed end: %+v", inst)
			}
			if elapsed < 4*time.Minute {
				if !nightmareWaiting(inst) || len(inst.MobIDs) != 0 || !inst.CombatDeadline.Equal(inst.ScheduleEnd) {
					t.Fatalf("restart spawned monsters during admission: %+v", inst)
				}
			} else if inst.State != "combat" || len(inst.MobIDs) != 1 || !inst.CombatDeadline.Equal(inst.HardDeadline) {
				t.Fatalf("restart did not resume the remaining combat time: %+v", inst)
			}
			if elapsed >= 4*time.Minute {
				for id := range inst.MobIDs {
					w.killMobWithoutPlayer(w.mobsByID[id])
					break
				}
				if len(inst.MobIDs) != 1 || len(inst.NightmareSpawns) != 1 || !inst.HardDeadline.Equal(start.Add(19*time.Minute)) {
					t.Fatal("restoration lost individual respawn origins or extended combat")
				}
			}
		})
	}
}

func TestNightmareLateSoloJoinDoesNotResetSchedule(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 16, 12, 0, 0, 0, time.UTC)
	rule := scheduledNightmareTestRule("normal", 0)
	rule.PartyMode = "solo"
	leader.Party, member.Party = nil, nil
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil {
		t.Fatal("first solo admission failed")
	}
	end := inst.HardDeadline
	clock.Advance(3 * time.Minute)
	member.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(member.Session, useItemPacket(0, 0))
	if !itemInstanceHasMember(inst, member.ID) || !inst.HardDeadline.Equal(end) || len(inst.MobIDs) != 0 {
		t.Fatalf("late entry reset or started the encounter: %+v", inst)
	}
	assertNightmareHUD(t, member, 60)
	clock.Advance(time.Minute)
	w.tickItemInstances(clock.Now())
	// A fresh ticket cannot admit another visit after the entry window closes.
	leader.Char.Inv[1] = model.Item{Index: 100}
	w.onUseItem(leader.Session, useItemPacket(0, 1))
	if leader.Char.Inv[1].Index != 100 || !inst.HardDeadline.Equal(end) {
		t.Fatal("closed admission consumed a ticket or extended the encounter")
	}
}

func TestNightmareCriticalAdmissionCommitsOrRejectsWithoutEarlyMobs(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection"} {
		t.Run(outcome, func(t *testing.T) {
			w, leader, _, peer, _, base, q, _, release := criticalInstanceEntryFixture(t, "water", outcome)
			leader.Trade, peer.Trade = nil, nil
			clock := newFakeClock(time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC))
			w.clock = clock
			rule := scheduledNightmareTestRule("normal", 0)
			nightmareTestProtectors(w, &rule)
			w.volatiles.Items[100] = rule
			if err := w.queueCriticalInstanceTicket(leader, 0, rule, ""); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.instances == nil || len(proposal.instances.Instances) != 1 ||
				proposal.instances.Instances[0].State != "entry" ||
				!proposal.instances.Instances[0].HardDeadline.Equal(clock.Now().Add(16*time.Minute)) {
				t.Fatalf("proposal omitted the waiting phase or combat deadline: %+v", proposal.instances)
			}
			if len(w.itemInstances) != 0 || len(w.mobsByID) != 0 || leader.Char.Inv[0].Index != 100 {
				t.Fatal("unacknowledged admission published state, monsters or a ticket charge")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			if len(w.criticalTeleportTiles) != 0 || len(w.criticalSummonIDs) != 0 {
				t.Fatal("admission left ID or tile reservations")
			}
			if outcome == "rejection" {
				if len(w.itemInstances) != 0 || len(w.mobsByID) != 0 || leader.Char.Inv[0].Index != 100 {
					t.Fatal("rejected admission consumed the ticket or installed an instance")
				}
				return
			}
			inst := w.itemInstances[rule.Instance.ID]
			if !nightmareWaiting(inst) || len(inst.NPCIDs) != 8 || len(w.mobsByID) != 8 || leader.Char.Inv[0].Index != 0 {
				t.Fatalf("committed admission did not install waiting membership: %+v", inst)
			}
			assertNightmareHUD(t, leader, 60, 8)
			for id := range inst.NPCIDs {
				if !leader.hasVisible(id) {
					t.Fatal("committed protected NPC was not published to the admitted player")
				}
			}
			// A delayed tick starts only the remaining combat time, not a fresh
			// fifteen minutes measured from scheduler recovery.
			clock.Advance(2 * time.Minute)
			w.tickItemInstances(clock.Now())
			if inst.State != "combat" || len(inst.MobIDs) != 1 {
				t.Fatalf("committed event did not start combat: %+v", inst)
			}
			assertNightmareHUD(t, leader, 840, 8)
			peer.Char.Inv[0] = model.Item{Index: 100}
			if err := w.queueCriticalInstanceTicket(peer, 0, rule, ""); err == nil || err.Error() != "This instance is not open right now." || peer.Char.Inv[0].Index != 100 {
				t.Fatal("closed admission accepted a new player or consumed a ticket")
			}
		})
	}
}

func nightmareBossTestRule(w *World) model.VolatileRule {
	rule := scheduledNightmareTestRule("arcane", 0)
	nightmareTestProtectors(w, &rule)
	w.npcs = append(w.npcs, model.NPCDef{Name: "Lugefer", Tipo: model.TipoMonstro,
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100}})
	rule.Instance.NightmareBoss = &model.VolatileNightmareBoss{
		Spawn:            model.VolatileInstanceSpawn{NPC: "Lugefer", Count: 1, X: 2208, Y: 2208, Range: 1},
		SecondsRemaining: 300,
	}
	return rule
}

func TestNightmareBossIsDelayedOneShotAndDoesNotStopOrdinaryRespawns(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC)
	rule := nightmareBossTestRule(w)
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil || len(inst.MobIDs) != 0 || inst.NightmareBossID != 0 {
		t.Fatal("admission spawned the boss or failed")
	}
	clock.Advance(time.Minute)
	w.tickItemInstances(clock.Now())
	clock.Advance(10*time.Minute - time.Second)
	w.tickItemInstances(clock.Now())
	if len(inst.MobIDs) != 1 || inst.NightmareBossSpawned {
		t.Fatal("boss spawned before five minutes remaining")
	}
	clock.Advance(time.Second)
	w.tickItemInstances(clock.Now())
	boss := w.mobsByID[inst.NightmareBossID]
	if boss == nil || !inst.NightmareBossSpawned || len(inst.MobIDs) != 2 ||
		len(inst.NightmareSpawns) != 1 || remainingInstanceSeconds(inst.HardDeadline, clock.Now()) != 300 {
		t.Fatalf("boss was not spawned separately at five minutes remaining: %+v", inst)
	}
	for _, p := range []*Player{leader, member} {
		if !p.hasVisible(boss.ID) || !boss.Awake {
			t.Fatal("stationary participant did not receive the boss")
		}
	}
	w.tickItemInstances(clock.Now())
	if len(inst.MobIDs) != 2 {
		t.Fatal("repeated scheduler tick duplicated the boss")
	}
	w.killMobWithoutPlayer(boss)
	if !inst.NightmareBossKilled || inst.NightmareBossID != 0 || len(inst.MobIDs) != 1 {
		t.Fatal("boss death entered the ordinary respawn path")
	}
	// The owner's explicit rule keeps common monsters respawning even with
	// zero protected survivors. Their loss affects EXP, not the population.
	for id := range inst.NightmareProtectors {
		w.killMobWithoutPlayer(w.mobsByID[id])
	}
	for id := range inst.MobIDs {
		w.killMobWithoutPlayer(w.mobsByID[id])
		break
	}
	w.tickItemInstances(clock.Now())
	if len(inst.MobIDs) != 1 || len(inst.NightmareSpawns) != 1 || instanceDisplayedMobCount(inst) != 0 {
		t.Fatal("protector loss stopped ordinary respawns or resurrected the boss")
	}
	if saved := w.instanceStateSnapshot().Instances; len(saved) != 1 || !saved[0].NightmareBossKilled {
		t.Fatal("boss defeat was missing from the durable snapshot")
	}
}

func TestNightmareBossRestoreKeepsOriginalClockAndDefeat(t *testing.T) {
	for _, scenario := range []string{"before-due", "delayed-tick", "alive", "dead", "all-protectors-dead", "alive-after-protector-loss"} {
		t.Run(scenario, func(t *testing.T) {
			w, _, _, _, clock := instanceTestWorld()
			start := time.Date(2026, 10, 16, 12, 0, 0, 0, time.UTC)
			clock.current = start.Add(15 * time.Minute)
			rule := nightmareBossTestRule(w)
			w.volatiles.Items[100] = rule
			saved := model.InstanceRuntimeState{RuntimeID: rule.Instance.ID, ConfigID: rule.Instance.ID,
				State: "combat", ScheduleEnd: start.Add(4 * time.Minute), HardDeadline: start.Add(19 * time.Minute)}
			wantBoss := true
			switch scenario {
			case "before-due":
				clock.current = start.Add(13 * time.Minute)
				wantBoss = false
			case "alive":
				saved.NightmareBossSpawned = true
			case "dead":
				saved.NightmareBossSpawned, saved.NightmareBossKilled = true, true
				wantBoss = false
			case "all-protectors-dead":
				saved.NightmareLostMask = 255
				wantBoss = false
			case "alive-after-protector-loss":
				saved.NightmareBossSpawned, saved.NightmareLostMask = true, 255
			}
			w.store = &gambleStore{load: &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
				Instances: []model.InstanceRuntimeState{saved}}}
			if err := w.restoreInstanceState(); err != nil {
				t.Fatal(err)
			}
			inst := w.itemInstances[rule.Instance.ID]
			if inst == nil || !inst.HardDeadline.Equal(saved.HardDeadline) || (inst.NightmareBossID != 0) != wantBoss {
				t.Fatalf("restoration changed the deadline or boss state: %+v", inst)
			}
			want := 1
			if wantBoss {
				want++
			}
			if len(inst.MobIDs) != want || len(inst.NightmareSpawns) != 1 {
				t.Fatal("restoration mixed the boss into the ordinary population")
			}
		})
	}
}

func TestNightmareInvalidBossRejectsBeforeTicketConsumption(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC)
	rule := nightmareBossTestRule(w)
	rule.Instance.NightmareBoss.Spawn.NPC = "MissingBoss"
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || len(w.itemInstances) != 0 || len(w.mobsByID) != 0 {
		t.Fatal("invalid delayed boss consumed a ticket or published an encounter")
	}
}

func TestNightmareBossDefeatCommitsWithRewards(t *testing.T) {
	for _, rejected := range []bool{false, true} {
		t.Run(fmt.Sprintf("rejected=%t", rejected), func(t *testing.T) {
			w, leader, member, _, clock := instanceTestWorld()
			clock.current = time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC)
			rule := nightmareBossTestRule(w)
			w.volatiles.Items[100] = rule
			w.onUseItem(leader.Session, useItemPacket(0, 0))
			inst := w.itemInstances[rule.Instance.ID]
			if inst == nil {
				t.Fatal("admission failed")
			}
			clock.Advance(11 * time.Minute)
			w.tickItemInstances(clock.Now())
			boss := w.mobsByID[inst.NightmareBossID]
			if boss == nil {
				t.Fatal("delayed tick did not spawn the boss")
			}
			boss.Def.ExpReward = 1000
			boss.Def.Score.Level = 300
			w.gameplay = model.DefaultGameplayConfig()
			beforeEXP := leader.Char.Exp
			claimCriticalTestPlayers(t, w, leader, member)
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 1), release: gate}
			if rejected {
				base.failure = criticalCertifiedRejection(t)
			}
			q := newCriticalTestQueue(t, &criticalCertifiedInstanceTestStore{base}, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			w.killMobState(leader, boss, boss.HP, boss.HP)
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.instances == nil || len(proposal.instances.Instances) != 1 ||
				!proposal.instances.Instances[0].NightmareBossKilled || len(proposal.accounts) == 0 {
				t.Fatal("boss defeat was not committed atomically with reward accounts")
			}
			if inst.NightmareBossKilled || leader.Char.Exp != beforeEXP || !w.criticalInstanceReserved() {
				t.Fatal("boss defeat or rewards became visible before acknowledgement")
			}
			release()
			awaitReadyCriticalResults(t, q, 1)
			w.processCriticalCompletions(1)
			if rejected {
				if inst.NightmareBossKilled || boss.Dead || boss.HP == 0 || leader.Char.Exp != beforeEXP {
					t.Fatal("rejected boss defeat changed rewards or the encounter")
				}
			} else if !inst.NightmareBossKilled || inst.NightmareBossID != 0 || leader.Char.Exp <= beforeEXP {
				t.Fatalf("committed boss defeat: killed=%t id=%d EXP=%d before=%d",
					inst.NightmareBossKilled, inst.NightmareBossID, leader.Char.Exp, beforeEXP)
			}
		})
	}
}

func TestNightmareCombatSpawnFailureClosesWithoutReward(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	clock.current = time.Date(2026, 10, 16, 12, 3, 0, 0, time.UTC)
	rule := scheduledNightmareTestRule("normal", 0)
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.itemInstances[rule.Instance.ID] == nil {
		t.Fatal("admission failed")
	}
	w.npcs = nil
	clock.Advance(time.Minute)
	w.tickItemInstances(clock.Now())
	if len(w.itemInstances) != 0 || len(w.mobsByID) != 0 || leader.Char.Inv[0].Index == 3174 {
		t.Fatal("failed combat spawn left an empty encounter or granted a clear reward")
	}
}
