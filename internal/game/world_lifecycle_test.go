package game

import "testing"

func TestRemoveMobInstanceRemovesOnlyRequestedMob(t *testing.T) {
	first := &Mob{ID: 1000}
	dead := &Mob{ID: 1001, Dead: true}
	last := &Mob{ID: 1002}
	w := &World{mobs: []*Mob{first, dead, last}}

	w.removeMobInstance(dead)

	if len(w.mobs) != 2 {
		t.Fatalf("mobs ativos=%d, esperado 2", len(w.mobs))
	}
	if w.mobs[0] != first || w.mobs[1] != last {
		t.Fatalf("instancias restantes incorretas: %#v", w.mobs)
	}
}

func TestRemoveMobInstanceIgnoresUnknownMob(t *testing.T) {
	only := &Mob{ID: 1000}
	w := &World{mobs: []*Mob{only}}

	w.removeMobInstance(&Mob{ID: 2000})

	if len(w.mobs) != 1 || w.mobs[0] != only {
		t.Fatal("mob desconhecido alterou a lista ativa")
	}
}
