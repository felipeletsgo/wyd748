package game

import (
	"bytes"
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/wire"
)

func loadRuneCombatSkills(t *testing.T, w *World) {
	t.Helper()
	catalog, err := data.LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.skills = catalog.Skills
}

func TestRuneTrack759AllCasterBarsAndSelectionWindows(t *testing.T) {
	w, _, _, _, clock := runeTrackTestWorld(t)
	loadRuneCombatSkills(t, w)
	// Independent values from the historical event templates, including empty
	// slots. The selection must not reroll an empty slot or use party order.
	bars := map[string][4]byte{
		"RT_Bat": {255, 255, 1, 255}, "RT_BunnyKing": {32, 32, 32, 255},
		"RT_ElfArcher": {255, 255, 79, 255}, "RT_ElfKnight": {255, 255, 1, 255},
		"RT_GoldBunny": {255, 255, 51, 255}, "RT_HellLich": {52, 52, 51, 255},
		"RT_LichKing": {33, 33, 37, 255}, "RT_Mage": {33, 33, 37, 255},
		"RT_MageBoss": {33, 33, 37, 255}, "RT_Ranger": {255, 79, 73, 255},
		"RT_Shadow": {255, 255, 26, 255}, "RT_Shaman": {52, 55, 63, 255},
		"RT_Sorcerer": {36, 34, 39, 255}, "RT_Templar": {255, 255, 2, 255},
	}
	inst := &ItemInstance{RuntimeID: "rune-combat-test", UxmalRoom: 0, State: "combat",
		RuneTrack: &runeTrackRuntime{Start: clock.Now().Add(-time.Minute), Generators: map[uint16]int{1300: 5662}},
		MobIDs:    map[uint16]struct{}{1300: {}}}
	w.itemInstances[inst.RuntimeID] = inst
	casters := 0
	for _, def := range w.npcs {
		if len(def.Name) >= 3 && def.Name[:3] == "RT_" && def.SkillBar != [4]byte{255, 255, 255, 255} {
			casters++
		}
	}
	if casters != len(bars) {
		t.Fatalf("uncovered caster templates: got %d want %d", casters, len(bars))
	}
	for name, bar := range bars {
		t.Run(name, func(t *testing.T) {
			def := w.npcDefByName(name)
			if def == nil || def.SkillBar != bar {
				t.Fatalf("historical skill bar differs: %v", def)
			}
			m := &Mob{ID: 1300, Def: def, InstanceID: inst.RuntimeID, HP: def.Score.MaxHP}
			w.mobsByID[m.ID] = m
			for roll := 0; roll < 100; roll++ {
				w.rng = fixedRNG{value: roll}
				slot := 0
				if roll >= 85 {
					slot = 2
				} else if roll >= 50 {
					slot = 1
				}
				skill, ok := w.runeTrackCombatSkill(m, clock.Now())
				if ok != (bar[slot] != 255) || ok && skill.Index != int(bar[slot]) {
					t.Fatalf("roll %d selected %+v (%v), expected slot %d = %d", roll, skill, ok, slot, bar[slot])
				}
			}
		})
	}
}

func runeCombatFrames(t *testing.T, p *Player, attackerID, targetID uint16) [][]byte {
	t.Helper()
	var frames [][]byte
	for p.Session.QueuedPacketsForTest() > 0 {
		packet, _ := p.Session.DequeuePacketForTest()
		decoded, checksum, err := wire.ReadPacket(bytes.NewReader(packet))
		if err != nil || !checksum {
			t.Fatalf("invalid combat frame: %v", err)
		}
		if len(decoded) >= 48 && binary.LittleEndian.Uint16(decoded[4:]) == wire.OpAttackOne &&
			binary.LittleEndian.Uint16(decoded[12:]) == attackerID && binary.LittleEndian.Uint16(decoded[44:]) == targetID {
			frames = append(frames, decoded)
		}
	}
	return frames
}

func TestRuneTrack759CasterTickVisibilityDeadlineAndPhysicalFallback(t *testing.T) {
	w, p, member, _, clock := runeTrackTestWorld(t)
	loadRuneCombatSkills(t, w)
	inst := enterRuneTrack(t, w, p, clock, 3)
	clock.Advance(31 * time.Second)
	w.tickItemInstances(clock.Now())
	m := runeMob(t, w, inst, 5957)
	if m.Def.Name != "RT_MageBoss" {
		t.Fatalf("unexpected production caster: %s", m.Def.Name)
	}
	w.activeMobs = map[uint16]*Mob{m.ID: m}
	m.TargetID, m.Awake = p.ID, true
	for _, player := range []*Player{p, member} {
		player.X, player.Y = m.X+1, m.Y
		player.Char.X, player.Char.Y = player.X, player.Y
		player.Char.RuntimeScore = nil
		player.Char.Score.MaxHP, player.Char.Score.CurHP = 1_000_000, 1_000_000
		w.updatePlayerSpatial(player)
		player.show(m.ID)
		runeCombatFrames(t, player, m.ID, p.ID)
	}
	outsider, _ := networkedTestPlayer(3, "Outsider", p.X, p.Y)
	w.playersByID[outsider.ID] = outsider
	w.updatePlayerSpatial(outsider)
	outsider.show(m.ID)
	w.rng = fixedRNG{value: 0}
	w.tickActiveMobActions(clock.Now())
	if playerCurHP(p.Char) >= 1_000_000 || p.LastAttackerID != m.ID || m.NextAttack != clock.Now().Add(mobAttackInterval) {
		t.Fatal("production caster did not execute its attack deadline")
	}
	for _, player := range []*Player{p, member} {
		frames := runeCombatFrames(t, player, m.ID, p.ID)
		if len(frames) != 1 || len(frames[0]) != 60 || binary.LittleEndian.Uint16(frames[0][24:]) != 33 ||
			binary.LittleEndian.Uint32(frames[0][56:]) != 1_000_000-playerCurHP(p.Char) {
			t.Fatalf("player %d did not receive the authoritative skill hit: %x", player.ID, frames)
		}
	}
	if frames := runeCombatFrames(t, outsider, m.ID, p.ID); len(frames) != 0 {
		t.Fatal("public observer received an instance skill hit")
	}
	before := playerCurHP(p.Char)
	w.tickActiveMobActions(clock.Now())
	if playerCurHP(p.Char) != before {
		t.Fatal("caster bypassed its attack deadline")
	}
	// Bat's first slot is empty: exercise the real dispatcher, not only selection.
	m.Def = w.npcDefByName("RT_Bat")
	clock.Advance(mobAttackInterval)
	w.tickActiveMobActions(clock.Now())
	frames := runeCombatFrames(t, p, m.ID, p.ID)
	if len(frames) != 1 || len(frames[0]) != 52 {
		t.Fatalf("empty slot did not produce one physical hit: %x", frames)
	}
	for _, state := range []string{"preparation", "settling", "combat"} {
		inst.State = state
		m.NextAttack = time.Time{}
		if state == "combat" {
			clock.Advance(inst.RuneTrack.Start.Add(15 * time.Minute).Sub(clock.Now()))
		}
		before = playerCurHP(p.Char)
		w.tickActiveMobActions(clock.Now())
		if playerCurHP(p.Char) != before || !m.NextAttack.IsZero() {
			t.Fatalf("caster attacked outside the combat window: %s", state)
		}
	}
}

func TestRuneTrack759CasterOwnershipAndMissingSkill(t *testing.T) {
	w, p, _, _, clock := runeTrackTestWorld(t)
	loadRuneCombatSkills(t, w)
	inst := enterRuneTrack(t, w, p, clock, 3)
	clock.Advance(31 * time.Second)
	w.tickItemInstances(clock.Now())
	m := runeMob(t, w, inst, 5957)
	w.rng = fixedRNG{value: 0}
	if _, ok := w.runeTrackCombatSkill(m, clock.Now()); !ok {
		t.Fatal("owned caster rejected")
	}
	for _, mutate := range []func(){
		func() { delete(inst.MobIDs, m.ID) },
		func() { delete(inst.RuneTrack.Generators, m.ID) },
		func() { m.InstanceID = "another-runtime" },
		func() { w.skills = nil },
	} {
		mutate()
		if _, ok := w.runeTrackCombatSkill(m, clock.Now()); ok {
			t.Fatal("unowned or unconfigured caster selected a skill")
		}
		inst.MobIDs[m.ID] = struct{}{}
		inst.RuneTrack.Generators[m.ID] = 5957
		m.InstanceID = inst.RuntimeID
	}
	if _, ok := w.runeTrackCombatSkill(nil, clock.Now()); ok {
		t.Fatal("nil caster selected a skill")
	}
}

func TestRuneTrack759CasterDestroysTowerWithoutHellGateState(t *testing.T) {
	w, p, _, _, clock := runeTrackTestWorld(t)
	loadRuneCombatSkills(t, w)
	inst := enterRuneTrack(t, w, p, clock, 1)
	clock.Advance(31 * time.Second)
	w.tickItemInstances(clock.Now())
	tower := runeMob(t, w, inst, 5715)
	caster := runeMob(t, w, inst, 5718)
	// Production tower invaders have empty bars. Exercise the shared skill path
	// with a configured copy without changing their historical production data.
	configured := *caster.Def
	configured.SkillBar = [4]byte{36, 34, 39, 255}
	caster.Def = &configured
	caster.X, caster.Y = tower.X+1, tower.Y
	p.X, p.Y = tower.X+2, tower.Y
	p.Char.X, p.Char.Y = p.X, p.Y
	w.updatePlayerSpatial(p)
	p.show(caster.ID)
	runeCombatFrames(t, p, caster.ID, tower.ID)
	w.activeMobs = map[uint16]*Mob{caster.ID: caster}
	w.rng = fixedRNG{value: 0}
	w.tickActiveMobActions(clock.Now())
	if tower.HP >= tower.Def.Score.MaxHP || !caster.NextAttack.Equal(clock.Now().Add(mobAttackInterval)) {
		t.Fatal("caster did not attack the tower")
	}
	frames := runeCombatFrames(t, p, caster.ID, tower.ID)
	if len(frames) != 1 || len(frames[0]) != 60 || binary.LittleEndian.Uint16(frames[0][24:]) != 36 ||
		binary.LittleEndian.Uint32(frames[0][56:]) != tower.Def.Score.MaxHP-tower.HP {
		t.Fatalf("tower attack did not publish its authoritative skill hit: %x", frames)
	}
	before := tower.HP
	w.tickActiveMobActions(clock.Now())
	if tower.HP != before {
		t.Fatal("tower spell bypassed its deadline")
	}
	tower.HP = 1
	clock.Advance(mobAttackInterval)
	w.tickActiveMobActions(clock.Now())
	if !tower.Dead || !inst.RuneTrack.Parties[0].TowerLost || inst.HellGateDionysHP != 0 {
		t.Fatal("tower death did not update its objective or contaminated Hell Gate state")
	}
}
