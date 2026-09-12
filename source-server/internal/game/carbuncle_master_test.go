package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func carbMasterTestWorld(t *testing.T, name string) (*World, *Player, *Mob, *craftStore) {
	t.Helper()
	w, p, st := handlerTestWorld(t)
	p.Char.Evolution = ""
	p.Char.Score = testScore(model.Score{
		Level: 0, Attack: 100, Defense: 100, MaxHP: 100, CurHP: 100,
		MaxMP: 100, CurMP: 100, AttackRun: 2,
	})
	w.skills = map[int]model.SkillDef{
		41: {Index: 41, AffectType: 2, AffectValue: 1, AffectTime: 150},
		43: {Index: 43, AffectType: 11, AffectValue: 15, AffectTime: 150},
		44: {Index: 44, AffectType: 9, AffectValue: 5, AffectTime: 150},
		45: {Index: 45, AffectType: 15, AffectValue: 7, AffectTime: 150},
	}
	master := &Mob{ID: 1900, X: p.X + 1, Y: p.Y, Def: &model.NPCDef{
		Name: name, Tipo: model.TipoNPC,
		Score: &model.Score{Merchant: 43},
	}}
	w.registerMobSpatial(master)
	p.show(master.ID)
	return w, p, master, st
}

func TestCarbMasterRecognizesOnlyConfiguredFamily(t *testing.T) {
	for _, name := range []string{"Carb_Mstr1", "Carb_Mstr2", "Carb_Mstr3", "Carb_Mstr4", "Carb_Mstr5", "Carb_Mstr6", "Carb Mstr6"} {
		if !isCarbMasterNPC(&model.NPCDef{Name: name, Tipo: model.TipoNPC}) {
			t.Errorf("%q deveria ser mestre de buffs", name)
		}
	}
	for _, name := range []string{"Carbunkle_Anct", "Carb_Mstr7", "Carb_Mstr", "Arch"} {
		if isCarbMasterNPC(&model.NPCDef{Name: name, Tipo: model.TipoNPC}) {
			t.Errorf("%q nao deveria ser mestre de buffs", name)
		}
	}
}

func TestCarbMasterClickAppliesAuthoritativeSkillDataPackage(t *testing.T) {
	w, p, master, st := carbMasterTestWorld(t, "Carb_Mstr4")
	started := time.Now()

	w.onUseNPC(p.Session, useNPCPacket(master.ID))

	if st.saves != 1 {
		t.Fatalf("saves=%d, esperado 1", st.saves)
	}
	for _, want := range []struct {
		typeID byte
		value  int
	}{
		{2, 1}, {11, 15}, {9, 5}, {15, 7},
	} {
		affect := activePlayerAffect(p.Char, want.typeID)
		if affect == nil || affect.Value != want.value || affect.Level != carbMasterPower {
			t.Fatalf("affect %d incorreto: %+v", want.typeID, affect)
		}
		remaining := affect.ExpiresAt.Sub(started)
		if remaining < 1199*time.Second || remaining > 1201*time.Second {
			t.Fatalf("duracao do affect %d=%s, esperava cerca de 1200s", want.typeID, remaining)
		}
	}
	if effectiveScore(p.Char).Defense <= 100 || effectiveScore(p.Char).Attack <= 100 {
		t.Fatalf("buffs nao alteraram score efetivo: %+v", effectiveScore(p.Char))
	}
}

func TestCarbMasterRejectsAdvancedAndOverleveledCharacters(t *testing.T) {
	for _, test := range []struct {
		name      string
		evolution string
		level     uint32
	}{
		{name: "arch", evolution: "arch"},
		{name: "level 117 displayed", level: carbMasterLevelLimit},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, master, st := carbMasterTestWorld(t, "Carb_Mstr1")
			p.Char.Evolution = test.evolution
			p.Char.Score.Level = test.level
			p.Char.RuntimeScore = nil

			w.onUseNPC(p.Session, useNPCPacket(master.ID))

			if st.saves != 0 {
				t.Fatalf("recusa persistiu estado: saves=%d", st.saves)
			}
			for _, skill := range carbMasterSkills {
				typeID, _, _ := skillAffect(w.skills[skill])
				if activePlayerAffect(p.Char, typeID) != nil {
					t.Fatalf("recusa aplicou affect %d", typeID)
				}
			}
		})
	}
}

func TestCarbMasterRollsBackWholePackageWhenSaveFails(t *testing.T) {
	w, p, master, st := carbMasterTestWorld(t, "Carb_Mstr6")
	before := cloneCharacterState(p.Char)
	st.err = errors.New("database unavailable")

	w.onUseNPC(p.Session, useNPCPacket(master.ID))

	if st.saves != 1 || p.Char.Affects != before.Affects {
		t.Fatalf("rollback incompleto: saves=%d affects=%+v", st.saves, p.Char.Affects)
	}
}
