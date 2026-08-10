package game

import (
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func incubationUsePacket(powderSlot uint32) []byte {
	pkt := useItemPacket(powderSlot, mountSlot)
	binary.LittleEndian.PutUint32(pkt[20:24], placeEquip)
	return pkt
}

func incubationWorld(t *testing.T, critical, rng int) (*World, *Player, *craftStore, *fakeClock) {
	t.Helper()
	w, p, st := useItemWorld(model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9})
	clock := newFakeClock(time.Unix(1_700_000_000, 0))
	w.clock = clock
	w.rng = fixedRNG{value: rng}
	eggID := uint16(model.MountEggBase + 7)
	w.items[eggID] = model.ItemDef{
		Index: eggID, Pos: 1 << mountSlot,
		StaticEffects: []model.StaticEffect{{Name: "EF_INCUBATE", Value: critical}},
	}
	p.Char.Equip[mountSlot] = model.Item{
		Index: eggID,
		UID:   "11111111111141118111111111111111",
	}
	return w, p, st, clock
}

func putIncubationPowder(p *Player) {
	p.Char.Inv[0] = model.Item{Index: 100}
}

func TestEggIncubationCriticalThreeRequiresWaitAndFinalPowder(t *testing.T) {
	w, p, st, clock := incubationWorld(t, 3, 0)

	for wantProgress := 1; wantProgress <= 3; wantProgress++ {
		putIncubationPowder(p)
		w.onUseItem(p.Session, incubationUsePacket(0))
		egg := p.Char.Equip[mountSlot]
		if eggProgress(egg) != wantProgress || eggDelay(egg) != 6 || p.Char.Inv[0].Index != 0 {
			t.Fatalf("etapa %d: progresso=%d delay=%d po=%d",
				wantProgress, eggProgress(egg), eggDelay(egg), p.Char.Inv[0].Index)
		}

		// Nem outra poeira nem o relogio antes de uma hora podem avancar.
		putIncubationPowder(p)
		w.onUseItem(p.Session, incubationUsePacket(0))
		if p.Char.Inv[0].Index != 100 || eggDelay(p.Char.Equip[mountSlot]) != 6 {
			t.Fatalf("etapa %d consumiu poeira durante espera", wantProgress)
		}
		p.Char.Inv[0] = model.Item{}
		clock.Advance(59 * time.Minute)
		w.tickEquippedEggIncubation(p, clock.Now())
		if eggDelay(p.Char.Equip[mountSlot]) != 6 {
			t.Fatalf("etapa %d debitou antes de uma hora", wantProgress)
		}

		clock.Advance(5*time.Hour + time.Minute)
		w.tickEquippedEggIncubation(p, clock.Now())
		if got := eggDelay(p.Char.Equip[mountSlot]); got != 0 {
			t.Fatalf("etapa %d terminou com %dh restantes", wantProgress, got)
		}
	}

	// Ao atingir +3 o ovo ainda espera; somente a poeira seguinte o choca.
	if !model.IsMountEgg(p.Char.Equip[mountSlot].Index) {
		t.Fatal("ovo chocou antes da poeira final")
	}
	putIncubationPowder(p)
	w.onUseItem(p.Session, incubationUsePacket(0))
	baby := p.Char.Equip[mountSlot]
	if !model.IsMountBaby(baby.Index) || baby.MountHP() <= 0 || baby.MountLongev() < 10 ||
		baby.MountLongev() > 30 || baby.MountFood() != 30 || p.Char.Inv[0].Index != 0 {
		t.Fatalf("cria invalida: %+v hp=%d long=%d food=%d po=%d",
			baby, baby.MountHP(), baby.MountLongev(), baby.MountFood(), p.Char.Inv[0].Index)
	}
	if st.saves != 7 { // tres aplicacoes + tres conclusoes de espera + hatch
		t.Fatalf("saves da incubacao=%d, quer 7", st.saves)
	}
}

func TestEggIncubationRunsOnlyOnlineAndEquipped(t *testing.T) {
	w, p, _, clock := incubationWorld(t, 3, 2) // extremo superior: 8 horas
	w.onUseItem(p.Session, incubationUsePacket(0))
	if got := eggDelay(p.Char.Equip[mountSlot]); got != 8 {
		t.Fatalf("delay maximo=%d, quer 8", got)
	}

	// Retirar o ovo pausa e conserva as horas inteiras.
	p.Char.Inv[5], p.Char.Equip[mountSlot] = p.Char.Equip[mountSlot], model.Item{}
	w.resetEggIncubationClock(p)
	clock.Advance(12 * time.Hour)
	w.tickEquippedEggIncubation(p, clock.Now())
	if got := eggDelay(p.Char.Inv[5]); got != 8 {
		t.Fatalf("ovo no inventario perdeu tempo: %d", got)
	}

	// Ao reequipar inicia uma hora nova; 59 minutos nao bastam.
	p.Char.Equip[mountSlot], p.Char.Inv[5] = p.Char.Inv[5], model.Item{}
	w.tickEquippedEggIncubation(p, clock.Now())
	clock.Advance(59 * time.Minute)
	w.tickEquippedEggIncubation(p, clock.Now())
	if got := eggDelay(p.Char.Equip[mountSlot]); got != 8 {
		t.Fatalf("ovo debitou fracao de hora: %d", got)
	}
	clock.Advance(time.Minute)
	w.tickEquippedEggIncubation(p, clock.Now())
	if got := eggDelay(p.Char.Equip[mountSlot]); got != 7 {
		t.Fatalf("ovo equipado nao debitou uma hora: %d", got)
	}

	// Desconectado, nem mesmo o ovo ainda equipado avanca.
	p.InWorld = false
	clock.Advance(8 * time.Hour)
	w.tickEquippedEggIncubation(p, clock.Now())
	if got := eggDelay(p.Char.Equip[mountSlot]); got != 7 {
		t.Fatalf("ovo offline perdeu tempo: %d", got)
	}
}

func TestEggIncubationRollbackOnPersistenceFailure(t *testing.T) {
	t.Run("powder application", func(t *testing.T) {
		w, p, st, _ := incubationWorld(t, 3, 0)
		oldEgg, oldPowder := p.Char.Equip[mountSlot], p.Char.Inv[0]
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, incubationUsePacket(0))
		if p.Char.Equip[mountSlot] != oldEgg || p.Char.Inv[0] != oldPowder {
			t.Fatalf("falha de save alterou ovo/po: ovo=%+v po=%+v",
				p.Char.Equip[mountSlot], p.Char.Inv[0])
		}
	})

	t.Run("elapsed hour", func(t *testing.T) {
		w, p, st, clock := incubationWorld(t, 3, 0)
		w.onUseItem(p.Session, incubationUsePacket(0))
		st.err = errors.New("database unavailable")
		clock.Advance(6 * time.Hour)
		w.tickEquippedEggIncubation(p, clock.Now())
		if got := eggDelay(p.Char.Equip[mountSlot]); got != 6 {
			t.Fatalf("falha de save confirmou countdown: %d", got)
		}
		st.err = nil
		w.tickEquippedEggIncubation(p, clock.Now())
		if got := eggDelay(p.Char.Equip[mountSlot]); got != 0 {
			t.Fatalf("retry duravel nao concluiu countdown: %d", got)
		}
	})
}

func TestEggIncubationRejectsUnequippedEgg(t *testing.T) {
	w, p, st, _ := incubationWorld(t, 3, 0)
	p.Char.Inv[1], p.Char.Equip[mountSlot] = p.Char.Equip[mountSlot], model.Item{}
	w.onUseItem(p.Session, useItemPacket(0, 1))
	if p.Char.Inv[0].Index != 100 || eggProgress(p.Char.Inv[1]) != 0 || st.saves != 0 {
		t.Fatalf("ovo fora do slot foi incubado: po=%d egg=%+v saves=%d",
			p.Char.Inv[0].Index, p.Char.Inv[1], st.saves)
	}
}
