package game

import (
	"encoding/binary"
	"encoding/json"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestPoisonMistFourTicksAtEightSecondIntervals(t *testing.T) {
	w, owner, caster, target := bmSummonPvPWorld(t, 2000)
	clock := newFakeClock(time.Unix(1_800_000_000, 0))
	w.clock = clock
	start := clock.Now()
	target.HP = 1000
	w.applySkillMobEffects(caster, target, model.SkillDef{Index: 40, AffectType: 20, AffectValue: 100, AffectTime: 3}, 20)
	drainPackets(owner)
	drainPackets(caster)
	for _, seconds := range []int{8, 16, 24, 32} {
		clock.current = start.Add(time.Duration(seconds)*time.Second + 250*time.Millisecond)
		before := target.HP
		w.tickMobAffects(clock.Now(), 0, 1)
		if target.HP != before-110 {
			t.Fatalf("tick at %ds: HP=%d, want %d", seconds, target.HP, before-110)
		}
		for _, viewer := range []*Player{owner, caster} {
			found := false
			for pkt, ok := viewer.Session.DequeuePacketForTest(); ok; pkt, ok = viewer.Session.DequeuePacketForTest() {
				if !wire.Decrypt(pkt) {
					t.Fatal("invalid poison packet")
				}
				if wire.ParseHeader(pkt).Type == wire.OpAttackOne && binary.LittleEndian.Uint16(pkt[44:46]) == target.ID && binary.LittleEndian.Uint16(pkt[46:48]) == 110 {
					found = true
				}
			}
			if !found {
				t.Fatalf("missing floating poison hit at %ds for viewer %d", seconds, viewer.ID)
			}
		}
		w.tickMobAffects(clock.Now(), 0, 1)
		if target.HP != before-110 {
			t.Fatal("repeated tick damaged twice")
		}
	}
	if target.Affects[0] != (model.Affect{}) {
		t.Fatal("final tick did not expire poison")
	}
}

func TestTimedItemSweepRefreshesDisplayWithoutAccountWrite(t *testing.T) {
	w, p, st, session := newTimedSwapWorld(t, nil)
	p.Char.Inv[0] = model.Item{}
	p.Char.Equip[fairySlot] = model.Item{Index: 4300, UID: "33333333333343338333333333333333", ExpiresUnix: w.now().Add(7 * 24 * time.Hour).Unix()}
	before := p.Char.Equip[fairySlot]
	for _, at := range []time.Time{w.now(), w.now().Add(time.Minute)} {
		w.tickTimedItems(at)
		pkt, ok := session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpSendItem || binary.LittleEndian.Uint16(pkt[14:16]) != uint16(fairySlot) {
			t.Fatal("timed item display was not refreshed")
		}
		if st.saves != 0 || p.Char.Equip[fairySlot] != before {
			t.Fatal("display refresh mutated or persisted the item")
		}
	}
}

func TestFullHPHealReportsCalculatedAmount(t *testing.T) {
	w, _, caster, _ := bmSummonPvPWorld(t, 2000)
	caster.Char.Class = 1
	setPlayerCurHP(caster.Char, playerMaxHP(caster.Char))
	for _, skill := range []int{27, 29} {
		before := playerCurHP(caster.Char)
		results := w.applySupportSkillToTargets(caster, []*Player{caster}, model.SkillDef{Index: skill, InstanceValue: 100}, 200)
		if len(results) != 1 || results[0].hpDelta != uint32(foemaHealAmount(caster.Char, skill, 200, 100)) || playerCurHP(caster.Char) != before {
			t.Fatalf("heal %d: %#v", skill, results)
		}
	}
}

func TestPoisonMistPlayerTicksSurviveReconnect(t *testing.T) {
	w, target, caster, _ := bmSummonPvPWorld(t, 2000)
	start := time.Unix(1_800_000_000, 0)
	w.clock = newFakeClock(start)
	setOwnedAffectForPlayerAt(target.Char, caster, 20, 1, 0, 4, start)
	for i := range target.Char.Affects {
		if target.Char.Affects[i].Type == 20 {
			target.Char.Affects[i].FinalTickAtExpiry = true
		}
	}
	initialHP := playerCurHP(target.Char)
	w.tickPlayerAffects(start.Add(8250 * time.Millisecond))
	state := buildCharState(target, start.Add(9*time.Second))
	payload, err := json.Marshal(state)
	if err != nil {
		t.Fatal(err)
	}
	state = &model.CharState{}
	if err := json.Unmarshal(payload, state); err != nil {
		t.Fatal(err)
	}
	clear(target.Char.Affects[:])
	w.applyCharState(target, state, start.Add(9*time.Second))
	for _, seconds := range []int{16, 24, 32} {
		w.tickPlayerAffects(start.Add(time.Duration(seconds)*time.Second + 250*time.Millisecond))
	}
	if playerCurHP(target.Char) != initialHP-4 || target.Char.Affects[0].Type != 0 {
		t.Fatalf("poison cadence lost on reconnect: hp=%d want=%d affect=%+v", playerCurHP(target.Char), initialHP-4, target.Char.Affects[0])
	}
}

func TestGuardianCannonHitsAuthorizedPlayerAndRequiresMapCannon(t *testing.T) {
	w, target, caster, _ := bmSummonPvPWorld(t, 2000)
	target.X, target.Y = caster.X+4, caster.Y
	w.updatePlayerSpatial(target)
	target.Char.Score.MaxHP, target.Char.RuntimeScore.MaxHP = 100000, 100000
	setPlayerCurHP(target.Char, 100000)
	bit, ok := specialSkillBit(97)
	if !ok {
		t.Fatal("missing cannon skill bit")
	}
	caster.Char.LearnedSkill |= uint32(1) << bit
	w.skills = map[int]model.SkillDef{97: {Index: 97, Aggressive: 1, Range: 13, MaxTarget: 13, TargetType: 4, InstanceType: 2, InstanceValue: 500}}
	w.groundItems = map[uint16]*GroundItem{15001: {ID: 15001, Item: model.Item{Index: 746}, X: caster.X, Y: caster.Y}}
	req := skillCastRequest{Skill: 97, TargetID: target.ID, TargetX: target.X, TargetY: target.Y}
	w.onSkillAttack(caster, req)
	if playerCurHP(target.Char) >= 100000 {
		t.Fatal("cannon rejected an authorized player target")
	}
	before := playerCurHP(target.Char)
	w.groundItems = nil
	caster.SkillReady[97] = time.Time{}
	w.onSkillAttack(caster, req)
	if playerCurHP(target.Char) != before {
		t.Fatal("cannon fired without a map cannon")
	}
}

func TestReturnScrollPreservesWideMana(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 1
	p.Char.Score.MaxMP = 322670
	p.Char.RuntimeScore.MaxMP = 322670
	setPlayerCurMP(p.Char, 322670)
	w.volatiles = model.VolatileCatalog{Rules: map[int]model.VolatileRule{11: {Action: "recall", Consume: true}}, ItemCodes: map[uint16]int{900: 11}}
	w.items[900] = model.ItemDef{Index: 900}
	p.Char.Inv[0] = model.Item{Index: 900, UID: "return-scroll"}
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if playerCurMP(p.Char) != 322670 {
		t.Fatalf("return scroll truncated MP to %d", playerCurMP(p.Char))
	}
}

func TestIronSpearAcceptsAllEightNeighbours(t *testing.T) {
	for dx := -2; dx <= 2; dx++ {
		for dy := -2; dy <= 2; dy++ {
			want := absInt(dx) <= 1 && absInt(dy) <= 1 && (dx != 0 || dy != 0)
			if got := ironSpearBehindTile(99, 100, 100, 100, uint16(100+dx), uint16(100+dy)); got != want {
				t.Fatalf("offset %d,%d: %t", dx, dy, got)
			}
		}
	}
}
