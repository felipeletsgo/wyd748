package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func kibitaTestWorld(level uint32) (*World, *Player, *Mob, *model.QuestDef) {
	w := newZoneTestWorld()
	w.channel = 3
	w.clock = newFakeClock(time.Date(2026, 10, 23, 21, 0, 0, 0, time.FixedZone("server", -3*3600)))
	p := addZonePlayer(w, 1, 2100, 2100, 500)
	p.Char.UID = "kibita-player"
	p.Char.Citizenship = 3
	p.Char.Score.Level = level
	p.Char.Score.Str, p.Char.Score.Int = 151, 253
	p.Char.Score.Dex, p.Char.Score.Con = 157, 159
	p.Char.Inv[0] = model.Item{Index: 420}
	setItemAmount(&p.Char.Inv[0], 2)
	p.Char.Inv[1] = model.Item{Index: 5334}
	setItemAmount(&p.Char.Inv[1], 2)
	m := &Mob{Def: &model.NPCDef{Name: "Kibita"}}
	q := &model.QuestDef{ID: 89, NPC: "Kibita", Repeatable: true}
	q.Rewards.Citizenship = true
	w.recalcPlayer(p.Char)
	return w, p, m, q
}

func TestKibitaLacWindow(t *testing.T) {
	start := time.Date(2026, 10, 19, 21, 0, 0, 0, time.FixedZone("server", -3*3600))
	for day := 0; day < 7; day++ {
		for _, offset := range []time.Duration{-time.Second, 0, time.Hour - time.Second, time.Hour} {
			now := start.AddDate(0, 0, day).Add(offset)
			want := day < 5 && offset >= 0 && offset < time.Hour
			if got := kibitaLacWindow(now); got != want {
				t.Errorf("window at %s: got %v, want %v", now, got, want)
			}
		}
	}
}

func TestKibitaSecretStoneClassesAndReplay(t *testing.T) {
	for class, stone := range []uint16{5334, 5336, 5335, 5337} {
		t.Run(fmt.Sprint(class), func(t *testing.T) {
			w, p, m, q := kibitaTestWorld(370)
			p.Char.Class = uint8(class)
			p.Char.Inv[1].Index = stone
			p.Char.Equip[model.CapeSlot] = model.Item{Index: 3191}
			w.handleKibitaSoul(p.Session, p, m, q)
			if !specialSkillLearned(p.Char, 102) || itemStackAmount(p.Char.Inv[1]) != 1 ||
				p.Char.Equip[model.CapeSlot].Index != 3194 || activePlayerAffectAt(p.Char, 29, w.now()) != nil {
				t.Fatalf("invalid Soul unlock: learned=%x cape=%d inventory=%+v", p.Char.LearnedSkill,
					p.Char.Equip[model.CapeSlot].Index, p.Char.Inv[1])
			}
			w.handleKibitaSoul(p.Session, p, m, q)
			if itemStackAmount(p.Char.Inv[1]) != 1 || w.store.(*craftStore).saves != 1 {
				t.Fatal("repeated interaction consumed a second stone")
			}
		})
	}
}

func TestKibitaSoulRejectsWithoutPayment(t *testing.T) {
	cases := []struct {
		name      string
		level     uint32
		change    func(*World, *Player, *Mob, *model.QuestDef)
		delegated bool
	}{
		{"citizenship purchase", 370, func(_ *World, p *Player, _ *Mob, _ *model.QuestDef) { p.Char.Citizenship = 0 }, true},
		{"other channel", 370, func(_ *World, p *Player, _ *Mob, _ *model.QuestDef) { p.Char.Citizenship = 2 }, false},
		{"arch", 370, func(_ *World, p *Player, _ *Mob, _ *model.QuestDef) { p.Char.Evolution = "arch" }, false},
		{"celestial", 370, func(_ *World, p *Player, _ *Mob, _ *model.QuestDef) { p.Char.Evolution = "celestial" }, false},
		{"official level boundary", 369, func(_ *World, _ *Player, _ *Mob, _ *model.QuestDef) {}, false},
		{"wrong stone", 370, func(_ *World, p *Player, _ *Mob, _ *model.QuestDef) { p.Char.Inv[1].Index = 5335 }, false},
		{"invalid class", 370, func(_ *World, p *Player, _ *Mob, _ *model.QuestDef) { p.Char.Class = 4 }, false},
		{"other NPC", 370, func(_ *World, _ *Player, m *Mob, _ *model.QuestDef) { m.Def.Name = "Other" }, true},
		{"outside schedule", 368, func(w *World, _ *Player, _ *Mob, _ *model.QuestDef) { w.clock.(*fakeClock).Advance(time.Hour) }, false},
		{"missing Lac", 368, func(_ *World, p *Player, _ *Mob, _ *model.QuestDef) { p.Char.Inv[0] = model.Item{} }, false},
		{"affect capacity", 368, func(w *World, p *Player, _ *Mob, _ *model.QuestDef) {
			for i := range p.Char.Affects {
				p.Char.Affects[i] = model.Affect{Type: 11, ExpiresAt: w.now().Add(time.Hour)}
			}
		}, false},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			w, p, m, q := kibitaTestWorld(tc.level)
			tc.change(w, p, m, q)
			before := cloneCharacterState(p.Char)
			if got := w.handleKibitaSoul(p.Session, p, m, q); got == tc.delegated {
				t.Fatal("incorrect routing decision")
			}
			if !reflect.DeepEqual(*p.Char, before) || w.store.(*craftStore).saves != 0 {
				t.Fatal("rejected exchange changed character state or charged payment")
			}
		})
	}
}

func TestKibitaLacExpiresAndReloads(t *testing.T) {
	w, p, m, q := kibitaTestWorld(368)
	st := &atomicCharStateMemoryStore{}
	w.store = st
	observer := addZonePlayer(w, 2, 2101, 2100, 500)
	observer.show(p.ID)
	base := *p.Char.Score
	w.handleKibitaSoul(p.Session, p, m, q)
	a := activePlayerAffectAt(p.Char, 29, w.now())
	if a == nil || !a.ExpiresAt.Equal(w.now().Add(time.Hour)) || itemStackAmount(p.Char.Inv[0]) != 1 ||
		p.Char.LearnedSkill&celestialSoulBit != 0 || st.atomicSaves != 1 || st.state == nil || len(st.state.Affects) != 1 {
		t.Fatalf("Lac transaction is incomplete: affect=%+v saves=%d state=%+v", a, st.atomicSaves, st.state)
	}
	if e := effectiveScore(p.Char); e.Str != 2000 || e.Int != 2000 || e.MaxHP != 10000 || *p.Char.Score != base {
		t.Fatalf("fixed Soul changed canonical attributes or missed the limits: %+v", e)
	}
	w.handleKibitaSoul(p.Session, p, m, q)
	if st.atomicSaves != 1 || itemStackAmount(p.Char.Inv[0]) != 1 {
		t.Fatal("active Soul allowed a second payment")
	}
	if observer.Session.QueuedPacketsForTest() == 0 {
		t.Fatal("observer received no affect update")
	}
	state := st.state
	fresh := &Player{Char: &model.Char{Score: &model.Score{Str: 151, Int: 253}}}
	w.applyCharState(fresh, state, w.now().Add(30*time.Minute))
	if restored := activePlayerAffectAt(fresh.Char, 29, w.now().Add(30*time.Minute)); restored == nil || !restored.ExpiresAt.Equal(a.ExpiresAt) {
		t.Fatal("relogin lost or extended the absolute expiry")
	}
	w.clock.(*fakeClock).Advance(time.Hour)
	w.tickPlayerAffects(w.now())
	if activePlayerAffectAt(p.Char, 29, w.now()) != nil || effectiveScore(p.Char).Str != base.Str || effectiveScore(p.Char).Int != base.Int {
		t.Fatal("expiration did not restore the normal attributes")
	}
	fresh.Char.Affects = [16]model.Affect{}
	w.applyCharState(fresh, state, w.now())
	if activePlayerAffectAt(fresh.Char, 29, w.now()) != nil {
		t.Fatal("expired sidecar resurrected Soul")
	}
}

func TestMortalSoulOfficialConfigurations(t *testing.T) {
	// Expected STR/INT/DEX/CON from official sub_4BB4C0, multiplier 1.
	want := [][4]uint32{{211, 253, 157, 206}, {151, 354, 157, 206}, {151, 253, 219, 206},
		{211, 253, 204, 159}, {151, 354, 204, 159}, {151, 253, 204, 222},
		{241, 253, 157, 159}, {151, 404, 157, 159}, {151, 253, 157, 254}, {151, 253, 251, 159}}
	for config := 0; config <= 11; config++ {
		w, p, _, _ := kibitaTestWorld(370)
		p.Char.LearnedSkill |= celestialSoulBit
		p.Char.SoulInfo = uint8(config)
		// Runtime includes equipment: prove it does not read raw Score here.
		applyScore(p.Char)
		p.Char.RuntimeScore.Str += 100
		p.Char.Score.Str = 51
		p.Char.Affects[0] = model.Affect{Type: 29, ExpiresAt: w.now().Add(time.Minute)}
		w.applyExtendedAffectStats(p.Char)
		idx := config - 1
		if idx < 0 || idx >= len(want) {
			idx = 0
		}
		e := effectiveScore(p.Char)
		// Runtime STR is 251 after the synthetic equipment bonus.
		str := uint32(251)
		if idx == 0 || idx == 3 {
			str = 351
		} else if idx == 6 {
			str = 401
		}
		got := [4]uint32{e.Str, e.Int, e.Dex, e.Con}
		expected := want[idx]
		expected[0] = str
		if got != expected {
			t.Errorf("SoulInfo %d: got %v, want %v", config, got, expected)
		}
	}
}

func TestMortalFixedSoulOverridesOtherEffects(t *testing.T) {
	for _, initial := range []uint32{151, 5001} {
		w, p, _, _ := kibitaTestWorld(368)
		p.Char.Score.Str, p.Char.Score.Int = initial, initial
		p.Char.Affects[0] = model.Affect{Type: 29, ExpiresAt: w.now().Add(time.Hour)}
		p.Char.Affects[1] = model.Affect{Type: 35, Value: 50, ExpiresAt: w.now().Add(time.Hour)}
		applyScore(p.Char)
		w.applyExtendedAffectStats(p.Char)
		e := effectiveScore(p.Char)
		if e.Str != 2000 || e.Int != 2000 || e.MaxHP != 10000 || p.Char.Score.Str != initial || p.Char.Score.Int != initial {
			t.Fatalf("fixed Soul did not lock the effective score: %+v", e)
		}
	}
}

func TestMortalSoulCapsPrimaryAttributes(t *testing.T) {
	w, p, _, _ := kibitaTestWorld(370)
	p.Char.LearnedSkill |= celestialSoulBit
	p.Char.SoulInfo = 1
	p.Char.Affects[0] = model.Affect{Type: 29, ExpiresAt: w.now().Add(time.Hour)}
	p.Char.RuntimeScore.Str, p.Char.RuntimeScore.Int = 50000, 70000
	p.Char.RuntimeScore.Dex, p.Char.RuntimeScore.Con = 70000, 50000
	w.applyExtendedAffectStats(p.Char)
	e := effectiveScore(p.Char)
	if e.Str != 60000 || e.Int != 60000 || e.Dex != 60000 || e.Con != 60000 {
		t.Fatalf("official Soul caps were not applied: %+v", e)
	}
}

func TestMortalFixedSoulKeepsHPWithHeldExperience(t *testing.T) {
	w, p, _, _ := kibitaTestWorld(368)
	p.Char.Hold = heldExperienceLimit(p.Char)
	if !heldExperiencePenaltyActive(p.Char) {
		t.Fatal("fixture did not enable the held-experience penalty")
	}
	normalHP := effectiveScore(p.Char).MaxHP
	p.Char.Affects[0] = model.Affect{Type: 29, ExpiresAt: w.now().Add(time.Hour)}
	w.recalcPlayer(p.Char)
	if got := effectiveScore(p.Char).MaxHP; got != 10000 {
		t.Fatalf("fixed Soul HP with debt: got %d, want 10000", got)
	}
	w.clock.(*fakeClock).Advance(time.Hour)
	w.tickPlayerAffects(w.now())
	if got := effectiveScore(p.Char).MaxHP; got != normalHP/2 {
		t.Fatalf("expired Soul did not restore debt penalty: got %d, want %d", got, normalHP/2)
	}
}

func TestKibitaSoulUsesValidatedNPCRoute(t *testing.T) {
	for _, allowed := range []bool{false, true} {
		w, p, m, q := kibitaTestWorld(370)
		m.ID, m.X, m.Y = 1000, 2101, 2100
		m.Def.Score = &model.Score{Merchant: 74}
		w.mobsByID[m.ID] = m
		w.questsByNPC = map[string]*model.QuestDef{"kibita": q}
		if allowed {
			p.show(m.ID)
		}
		packet := make([]byte, 20)
		binary.LittleEndian.PutUint16(packet[12:14], m.ID)
		w.onUseNPC(p.Session, packet)
		if got := specialSkillLearned(p.Char, 102); got != allowed {
			t.Fatalf("NPC visibility=%v: Soul learned=%v", allowed, got)
		}
		if !allowed && itemStackAmount(p.Char.Inv[1]) != 2 {
			t.Fatal("invisible NPC consumed a stone")
		}
	}
}

func TestMortalLearnedSoulCanBeCast(t *testing.T) {
	w, p, m, q := kibitaTestWorld(370)
	w.handleKibitaSoul(p.Session, p, m, q)
	results := w.applySupportSkill(p, skillCastRequest{}, model.SkillDef{Index: 102, AffectType: 29}, 0)
	a := activePlayerAffectAt(p.Char, 29, w.now())
	if len(results) != 1 || a == nil || !a.ExpiresAt.Equal(w.now().Add(304*time.Second)) {
		t.Fatalf("learned Mortal Soul did not use the existing cast contract: results=%+v affect=%+v", results, a)
	}
	w.recalcPlayer(p.Char)
	if effectiveScore(p.Char).Str == 2000 || effectiveScore(p.Char).Str <= p.Char.Score.Str {
		t.Fatal("learned Soul used the fixed Lac branch")
	}
}

func TestKibitaSoulSaveFailureRestoresPaymentAndReward(t *testing.T) {
	for _, level := range []uint32{368, 370} {
		w, p, m, q := kibitaTestWorld(level)
		w.store = &atomicCharStateMemoryStore{atomicErr: errors.New("save unavailable")}
		before := cloneCharacterState(p.Char)
		w.handleKibitaSoul(p.Session, p, m, q)
		if !reflect.DeepEqual(*p.Char, before) {
			t.Fatalf("failed exchange at level %d changed character", level)
		}
	}
}

func TestKibitaSoulPublishesOnlyAfterCriticalCommit(t *testing.T) {
	for _, level := range []uint32{368, 370} {
		for _, rejected := range []bool{false, true} {
			t.Run(fmt.Sprintf("level%d/rejected%v", level, rejected), func(t *testing.T) {
				w, p, m, q := kibitaTestWorld(level)
				observer := addZonePlayer(w, 2, 2101, 2100, 500)
				observer.show(p.ID)
				claimCriticalTestPlayers(t, w, p)
				gate, release := criticalGate(t)
				backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
				if rejected {
					backend.failure = criticalCertifiedRejection(t)
				}
				queue := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{backend}}, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				before := cloneCharacterState(p.Char)
				w.handleKibitaSoul(p.Session, p, m, q)
				write := awaitCriticalWrite(t, backend.entered)
				proposed := &write.accounts[0].Chars[0]
				if write.character == nil || write.character.state == nil {
					t.Fatal("missing atomic sidecar")
				}
				if level == 368 {
					if itemStackAmount(proposed.Inv[0]) != 1 || len(write.character.state.Affects) != 1 || write.character.state.Affects[0].Type != 29 {
						t.Fatal("Lac payment and effect were not in the same transaction")
					}
				} else if itemStackAmount(proposed.Inv[1]) != 1 || proposed.LearnedSkill&celestialSoulBit == 0 || proposed.Equip[model.CapeSlot].Index != 3196 {
					t.Fatal("stone, skill and cape were not in the same transaction")
				}
				w.handleKibitaSoul(p.Session, p, m, q)
				if !reflect.DeepEqual(*p.Char, before) || p.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("pending exchange mutated or published state")
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				if !reflect.DeepEqual(*p.Char, before) {
					t.Fatal("worker published before actor acknowledgement")
				}
				w.processCriticalCompletions(1)
				if rejected {
					if !reflect.DeepEqual(*p.Char, before) || observer.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("rejected transaction published reward")
					}
				} else {
					if reflect.DeepEqual(*p.Char, before) || observer.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("committed transaction was not installed and published")
					}
					found := false
					for {
						packet, ok := p.Session.DequeuePacketForTest()
						if !ok {
							break
						}
						if !wire.Decrypt(packet) {
							t.Fatal("invalid packet checksum")
						}
						if wire.ParseHeader(packet).Type == wire.OpUpdateEtc {
							found = true
						}
					}
					if !found {
						t.Fatal("owner did not receive the skill snapshot")
					}
				}
			})
		}
	}
}
