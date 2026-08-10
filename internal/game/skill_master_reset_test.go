package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func abilityResetTestWorld(t *testing.T) (*World, *Player, *Mob, *craftStore) {
	t.Helper()
	w, p, store := handlerTestWorld(t)
	master := &Mob{
		ID: 1810, X: p.X + 1, Y: p.Y,
		Def: &model.NPCDef{
			Name: "Skill_Master", Tipo: model.TipoNPC,
			Extended: &model.ExtendedScore{Merchant: abilityResetMasterMerchant},
		},
	}
	w.registerMobSpatial(master)
	p.show(master.ID)
	p.Char.Class = 0
	p.Char.Extended.Level = 100
	p.Char.Extended.Str = 158 // base 8 + 150: somente 100 retornam
	p.Char.Extended.Int = 54  // base 4 + 50: todos retornam
	p.Char.Extended.Dex = 17  // base 7 + 10: todos retornam
	p.Char.Extended.Con = 6
	w.recalcPlayer(p.Char)
	return w, p, master, store
}

func confirmedAbilityResetPacket(id uint16) []byte {
	pkt := useNPCPacket(id)
	binary.LittleEndian.PutUint32(pkt[16:20], 1)
	return pkt
}

func TestSkillMasterUsesPremiumFirstAndResetsAtMostOneHundredPerStat(t *testing.T) {
	w, p, master, store := abilityResetTestWorld(t)
	p.Char.Inv[0] = model.Item{Index: abilityResetPremiumItem}
	p.Char.Inv[1] = model.Item{Index: model.SapphirePack}
	p.Char.Inv[2] = model.Item{Index: model.SapphirePack}
	p.Char.Inv[3] = model.Item{Index: model.SapphirePack}

	w.onUseNPC(p.Session, confirmedAbilityResetPacket(master.ID))

	e := p.Char.Extended
	if store.saves != 1 || p.Char.Inv[0].Index != 0 ||
		p.Char.Inv[1].Index != model.SapphirePack || p.Char.Inv[2].Index != model.SapphirePack ||
		e.Str != 58 || e.Int != 4 || e.Dex != 7 || e.Con != 6 || e.StatusPts != 450 {
		t.Fatalf("reset premium incorreto: saves=%d inv=%v stats=%+v",
			store.saves, p.Char.Inv[:4], e)
	}
}

func TestSkillMasterConsumesThirtySapphiresWithoutPremium(t *testing.T) {
	w, p, master, store := abilityResetTestWorld(t)
	for slot := 0; slot < 3; slot++ {
		p.Char.Inv[slot] = model.Item{Index: model.SapphirePack}
	}

	w.onUseNPC(p.Session, confirmedAbilityResetPacket(master.ID))

	if store.saves != 1 {
		t.Fatalf("reset por safiras salvou %d vez(es)", store.saves)
	}
	for slot := 0; slot < 3; slot++ {
		if p.Char.Inv[slot].Index != 0 {
			t.Fatalf("pacote de safiras no slot %d nao foi consumido", slot)
		}
	}
}

func TestSkillMasterRejectsEquipmentAndMissingPayment(t *testing.T) {
	for _, test := range []struct {
		name  string
		setup func(*model.Char)
	}{
		{name: "equipped", setup: func(ch *model.Char) {
			ch.Equip[1] = model.Item{Index: 100}
			ch.Inv[0] = model.Item{Index: abilityResetPremiumItem}
		}},
		{name: "missing payment", setup: func(*model.Char) {}},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, master, store := abilityResetTestWorld(t)
			test.setup(p.Char)
			before := cloneCharacterState(p.Char)

			w.onUseNPC(p.Session, confirmedAbilityResetPacket(master.ID))

			if store.saves != 0 || p.Char.Inv != before.Inv || p.Char.Equip != before.Equip ||
				p.Char.Extended.Str != before.Extended.Str {
				t.Fatalf("recusa alterou estado: saves=%d char=%+v", store.saves, p.Char)
			}
		})
	}
}

func TestSkillMasterRollsBackMaterialAndStatsOnSaveFailure(t *testing.T) {
	w, p, master, store := abilityResetTestWorld(t)
	p.Char.Inv[0] = model.Item{Index: abilityResetPremiumItem}
	before := cloneCharacterState(p.Char)
	store.err = errors.New("database unavailable")

	w.onUseNPC(p.Session, confirmedAbilityResetPacket(master.ID))

	if store.saves != 1 || p.Char.Inv != before.Inv ||
		p.Char.Extended.Str != before.Extended.Str || p.Char.Extended.Int != before.Extended.Int ||
		p.Char.Extended.StatusPts != before.Extended.StatusPts {
		t.Fatalf("rollback incompleto: saves=%d inv=%v stats=%+v", store.saves, p.Char.Inv[0], p.Char.Extended)
	}
}

func TestSkillMasterDoesNotExecuteBeforeNativeConfirmation(t *testing.T) {
	w, p, master, store := abilityResetTestWorld(t)
	p.Char.Inv[0] = model.Item{Index: abilityResetPremiumItem}
	before := cloneCharacterState(p.Char)

	w.onUseNPC(p.Session, useNPCPacket(master.ID))

	if store.saves != 0 || p.Char.Inv != before.Inv || p.Char.Extended.Str != before.Extended.Str {
		t.Fatal("clique sem confirmacao executou o reset")
	}
}
