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

// TestAllocMobIDNeverReturnsPlayerRange protege uma regressao encontrada ao
// implementar os bosses: com o clamp DEPOIS da reserva, um World de ID zerado
// devolvia 0 (que significa "sem ID") e logo em seguida 1000 -- colidindo com um
// mob ja existente e sobrescrevendo a entrada dele em mobsByID.
func TestAllocMobIDNeverReturnsPlayerRange(t *testing.T) {
	w := &World{} // nextMobID zerado, como um World construido direto
	seen := make(map[uint16]struct{}, 8)
	for i := 0; i < 8; i++ {
		id := w.allocMobID()
		if id < 1000 {
			t.Fatalf("ID %d invade a faixa reservada aos jogadores", id)
		}
		if _, repeated := seen[id]; repeated {
			t.Fatalf("ID %d alocado duas vezes", id)
		}
		seen[id] = struct{}{}
	}
}

func TestAllocMobIDPreservesSequenceFromInitializedWorld(t *testing.T) {
	w := &World{nextMobID: 1000} // como NewWorld deixa
	if got := w.allocMobID(); got != 1000 {
		t.Fatalf("primeiro ID=%d, quer 1000 (comportamento de producao inalterado)", got)
	}
	if got := w.allocMobID(); got != 1001 {
		t.Fatalf("segundo ID=%d, quer 1001", got)
	}
}

// TestAllocMobIDSkipsLiveMobsOnWraparound protege contra corrupcao silenciosa
// em servidor de longa duracao: cada respawn consome um ID, entao o contador
// inevitavelmente da a volta em 65535. Se ele reusasse o ID de um mob VIVO, a
// entrada dele em mobsByID seria sobrescrita e alvo/visibilidade/affects
// passariam a apontar para a criatura errada.
func TestAllocMobIDSkipsLiveMobsOnWraparound(t *testing.T) {
	live := &Mob{ID: 1000}
	w := &World{
		nextMobID: ^uint16(0), // proximo passo da a volta
		mobsByID:  map[uint16]*Mob{1000: live, 1001: {ID: 1001}},
	}

	if got := w.allocMobID(); got != 65535 {
		t.Fatalf("primeiro ID=%d, quer 65535", got)
	}
	// Ao dar a volta, 1000 e 1001 estao ocupados: precisa pular para 1002.
	got := w.allocMobID()
	if got == 1000 || got == 1001 {
		t.Fatalf("ID %d reusado de um mob vivo", got)
	}
	if got != 1002 {
		t.Fatalf("ID=%d, quer 1002 (primeiro livre apos a volta)", got)
	}
	if w.mobsByID[1000] != live {
		t.Fatal("o mob vivo foi deslocado do registro")
	}
}
