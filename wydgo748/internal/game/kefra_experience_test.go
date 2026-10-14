package game

import (
	"bytes"
	"errors"
	"math"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestKefraTemporaryOneHPDatasetDiesInOneSuccessfulAttack(t *testing.T) {
	defs, err := data.LoadNPCs("../../data/npcs/Kefra.json")
	if err != nil {
		t.Fatal(err)
	}
	def := &defs[0]
	if def.Name != kefraBossName || def.Score.MaxHP != 1 || def.Score.CurHP != 1 {
		t.Fatalf("temporary Kefra test profile must have exactly 1 HP: %+v", def.Score)
	}
	w, _, killer, _, _, _ := kefraTestWorld(t)
	w.rng = fixedRNG{value: 0} // A successful hit, not an evasion.
	killer.Char.Score.Attack = 100_000
	killer.Char.Score.Accuracy = 1000
	applyScore(killer.Char)
	g := &generState{def: model.NPCGener{Index: 3856, MaxNumMob: 1,
		Segments: [model.MaxGenerSegments]model.GenerSegment{{X: killer.X + 1, Y: killer.Y}}},
		leader: def, follower: def}
	w.spawnGroup(g)
	var boss *Mob
	for _, mob := range w.mobs {
		if mob.generator == g {
			boss = mob
		}
	}
	if boss == nil || boss.HP != 1 {
		t.Fatal("generator did not spawn the temporary one-HP boss")
	}
	killer.show(boss.ID)
	w.onAttack(killer.Session, physicalAttackPacket(1000, boss.ID, boss.X, boss.Y))
	if !boss.Dead || boss.HP != 0 || !w.kefraDefeated() {
		t.Fatalf("one successful attack did not commit the defeat: HP=%d dead=%v", boss.HP, boss.Dead)
	}
}

func TestKefraExperienceBonusFollowsDurableDefeatAndWeeklyReset(t *testing.T) {
	w, st, killer, outsider, clock, boss := kefraTestWorld(t)
	outsider.X, outsider.Y = 100, 100 // No proximity, party or winning-guild requirement.
	w.items = make(map[uint16]model.ItemDef)
	st.durable = cloneGuildRegistry(w.guilds)
	check := func(want uint32) {
		t.Helper()
		for _, receiver := range []*Player{killer, outsider} {
			if got := w.mobKillExperienceForReceiver(receiver.Char, receiver.Char, 1000); got != want {
				t.Fatalf("%s EXP=%d, want %d", receiver.Char.Name, got, want)
			}
		}
	}
	check(1000)
	st.err = errors.New("injected defeat save failure")
	w.onKefraKilled(killer, boss)
	check(1000)
	restartGuilds(t, w, st)
	st.err = nil
	w.onKefraKilled(killer, boss)
	check(2000)
	w.onKefraKilled(killer, boss) // Duplicate notification must not stack the bonus.
	check(2000)
	restartGuilds(t, w, st)
	check(2000)
	clock.Advance(14*time.Hour + 59*time.Minute)
	w.tickKefra(clock.Now())
	check(2000)
	clock.Advance(time.Minute)
	st.err = errors.New("injected weekly reset save failure")
	w.tickKefra(clock.Now())
	check(2000)
	st.err = nil
	w.tickKefra(clock.Now())
	check(1000)
	restartGuilds(t, w, st)
	check(1000)
}

func TestKefraExperienceBonusStacksWithItemsAndReachesEveryPartyReceiver(t *testing.T) {
	w, killer, member := experiencePartyKillWorld(t)
	equipCombatExperienceBonuses(t, w, killer)
	w.guilds = &model.GuildRegistry{Wars: model.GuildWarState{
		Kefra: model.KefraState{Defeated: true, DefeatedAt: w.now()},
	}}
	killExperienceTestMob(t, w, killer, 1100)
	const want = uint32(49_088) // 10,400 party share * 1.18 items * 2 box * 2 Kefra.
	if killer.Char.Exp != want || member.Char.Exp != want {
		t.Fatalf("killer=%d member=%d, want %d each", killer.Char.Exp, member.Char.Exp, want)
	}
}

func TestKefraExperienceBonusSaturatesAndPreservesEvolutionReduction(t *testing.T) {
	w, _, killer, member, clock, _ := kefraTestWorld(t)
	w.items = make(map[uint16]model.ItemDef)
	w.guilds.Wars.Kefra = model.KefraState{Defeated: true, DefeatedAt: clock.Now()}
	for _, reward := range []uint32{0, 1, math.MaxUint32 / 2, math.MaxUint32/2 + 1, math.MaxUint32} {
		want := uint64(reward) * 2
		if want > math.MaxUint32 {
			want = math.MaxUint32
		}
		if got := w.mobKillExperienceForReceiver(killer.Char, killer.Char, reward); got != uint32(want) {
			t.Fatalf("reward=%d got=%d want=%d", reward, got, want)
		}
	}
	member.Char.Evolution, member.Char.Score.Level = "arch", 300
	if got := w.mobKillExperienceForReceiver(killer.Char, member.Char, 8000); got != 2000 {
		t.Fatalf("Arch reduction lost: got %d, want 2000", got)
	}
}

func TestKefraDefeatNoticeNamesWinningGuildForDistantPlayer(t *testing.T) {
	w, _, killer, outsider, _, boss := kefraTestWorld(t)
	outsider.X, outsider.Y = 100, 100
	drainPlayerPackets(outsider)
	w.onKefraKilled(killer, boss)
	want := []byte("Guild " + w.guilds.FindByID(1).Name + " has defeated Kefra.")
	found := false
	for {
		packet, ok := outsider.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid encrypted notice")
		}
		if wire.ParseHeader(packet).Type == wire.OpMessagePanel && bytes.Contains(packet[12:], want) {
			found = true
		}
	}
	if !found {
		t.Fatalf("distant player did not receive guild notice %q", want)
	}
}
