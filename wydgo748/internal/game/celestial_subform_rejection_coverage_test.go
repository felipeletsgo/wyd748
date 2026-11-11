package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func subCelestialRejectionWorld(t *testing.T) (*World, *Player, *craftStore) {
	t.Helper()
	w, p, st := newCelestialWorld(t, 399)
	w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
	reenterCelestialTestCharacter(t, w, p)
	p.Char.Score.Level = model.SubCelestialMinimumLevel
	p.Char.Equip[sefirotSlot] = model.Item{Index: 1763}
	p.Char.Inv[0] = model.Item{Index: idealStoneItem}
	p.SpecialCoins = map[string]uint32{fameCounter: subCelestialFameCost}
	return w, p, st
}

func TestSubCelestialRejectsInvalidStateAndRestoresInventory(t *testing.T) {
	tests := []struct {
		name  string
		setup func(*Player)
	}{
		{name: "alternate form already exists", setup: func(p *Player) {
			p.Char.AlternateCelestial = &model.CelestialForm{Evolution: "subcelestial"}
		}},
		{name: "character below required level", setup: func(p *Player) {
			p.Char.Score.Level = model.SubCelestialMinimumLevel - 1
		}},
		{name: "invalid class sephirot", setup: func(p *Player) {
			p.Char.Equip[sefirotSlot] = model.Item{}
		}},
		{name: "invalid character face", setup: func(p *Player) {
			p.Char.Equip[0] = model.Item{}
		}},
		{name: "inventory has no room for mysterious stones", setup: func(p *Player) {
			for i := range p.Char.Inv {
				p.Char.Inv[i] = model.Item{Index: 5000 + uint16(i)}
			}
			p.Char.Inv[0] = model.Item{Index: idealStoneItem}
			setItemAmount(&p.Char.Inv[0], 2)
		}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st := subCelestialRejectionWorld(t)
			tc.setup(p)
			beforeInventory, beforeEquip := p.Char.Inv, p.Char.Equip
			beforeFame := counterBalance(p, fameCounter)
			beforeAlternate := p.Char.AlternateCelestial

			w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)

			if p.Char.Inv != beforeInventory || p.Char.Equip != beforeEquip ||
				counterBalance(p, fameCounter) != beforeFame || p.Char.AlternateCelestial != beforeAlternate || st.saves != 1 {
				t.Fatalf("rejected SubCelestial creation changed state: inv=%+v fame=%d alternate=%+v saves=%d",
					p.Char.Inv[0], counterBalance(p, fameCounter), p.Char.AlternateCelestial, st.saves)
			}
		})
	}
}

func TestMysteriousStoneSaveFailureRestoresBothCelestialForms(t *testing.T) {
	w, p, st := subCelestialRejectionWorld(t)
	p.Char.AlternateCelestial = &model.CelestialForm{
		Evolution: "subcelestial", Class: 3,
		Face: model.Item{Index: 18}, Score: newCelestialScore(3, p.Char.Score),
		LearnedSkill: celestialSoulBit,
	}
	p.Char.Inv[0] = model.Item{Index: mysteriousStoneItem}
	setItemAmount(&p.Char.Inv[0], 2)
	before := cloneCharacterState(p.Char)
	st.err = errors.New("database unavailable")

	w.useMysteriousStone(p.Session, p, &p.Char.Inv[0], 0)

	if !reflect.DeepEqual(p.Char, &before) || st.saves != 2 {
		t.Fatalf("failed form switch did not restore both forms: evolution=%s item=%+v saves=%d",
			p.Char.Evolution, p.Char.Inv[0], st.saves)
	}
}
