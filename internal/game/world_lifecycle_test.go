package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

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

func TestAllocGroundItemIDSkipsReservedRangeAndWraps(t *testing.T) {
	w := &World{
		nextItemID: 15001,
		groundItems: map[uint16]*GroundItem{
			65535: {ID: 65535},
			1:     {ID: 1},
		},
	}
	id, ok := w.allocGroundItemID(400)
	if !ok || id != 15101 {
		t.Fatalf("ID comum=%d/%v, esperado 15101 apos pular faixa do canhao", id, ok)
	}
	if w.nextItemID != 15102 {
		t.Fatalf("contador de item=%d, esperado 15102", w.nextItemID)
	}
}

func TestAllocGroundItemIDCannonExhaustionIsBounded(t *testing.T) {
	ground := make(map[uint16]*GroundItem, 100)
	for id := uint16(15001); id <= 15100; id++ {
		ground[id] = &GroundItem{ID: id}
	}
	w := &World{groundItems: ground}
	if id, ok := w.allocGroundItemID(746); ok || id != 0 {
		t.Fatalf("canhao deveria falhar sem ID: %d/%v", id, ok)
	}
}

func TestAllocGroundItemIDCommonRangeExhaustionIsBounded(t *testing.T) {
	ground := make(map[uint16]*GroundItem, int(^uint16(0))-100)
	for raw := 1; raw <= int(^uint16(0)); raw++ {
		id := uint16(raw)
		if id >= 15001 && id <= 15100 {
			continue
		}
		ground[id] = &GroundItem{ID: id}
	}
	w := &World{groundItems: ground, nextItemID: ^uint16(0)}
	if id, ok := w.allocGroundItemID(400); ok || id != 0 {
		t.Fatalf("faixa comum esgotada deveria falhar: %d/%v", id, ok)
	}
}

func TestGroundDropAtMapEdgeNeverWraps(t *testing.T) {
	w := &World{
		rng:         fixedRNG{value: 0},
		items:       map[uint16]model.ItemDef{400: {Index: 400}},
		groundItems: make(map[uint16]*GroundItem),
	}
	drop := w.createGroundDrop(1, 1, model.Item{Index: 400}, false)
	if drop == nil {
		t.Fatal("drop nao foi criado")
	}
	if drop.X != 1 || drop.Y != 1 {
		t.Fatalf("drop na borda sofreu wrap/deslocamento invalido: (%d,%d)", drop.X, drop.Y)
	}
}

func TestGroundDropFallsBackWhenScatteredCellIsBlocked(t *testing.T) {
	terrain := loadedFlatTerrain()
	// fixedRNG(0) tenta (99,99); bloqueie apenas a celula de atributo dela.
	terrain.Attribute[(99/4)*model.AttributeWidth+(99/4)] = 0x02
	w := &World{
		rng: fixedRNG{value: 0}, terrain: terrain,
		items:       map[uint16]model.ItemDef{400: {Index: 400}},
		groundItems: make(map[uint16]*GroundItem),
	}
	drop := w.createGroundDrop(100, 100, model.Item{Index: 400}, false)
	if drop == nil || drop.X != 100 || drop.Y != 100 {
		t.Fatalf("drop deveria conservar origem caminhavel: %+v", drop)
	}
}

func TestProcessCommandBatchDrainsOlderPendingCommandsFirst(t *testing.T) {
	s := net.NewTestSession(1, 16)
	w := &World{
		commands:    make(chan command, 8),
		players:     make(map[*net.Session]*Player),
		authPending: make(map[*net.Session]bool),
		security:    make(map[*net.Session]*securityState),
	}
	ping := func() []byte {
		pkt := make([]byte, wire.HeaderSize)
		binary.LittleEndian.PutUint16(pkt[0:2], wire.HeaderSize)
		binary.LittleEndian.PutUint16(pkt[4:6], wire.OpPing)
		return pkt
	}
	w.pendingCommands = []command{{s: s, pkt: ping()}, {s: s, pkt: ping()}}
	w.processCommandBatch(command{s: s, pkt: ping()}, make(chan time.Time))
	if len(w.pendingCommands) != 0 {
		t.Fatalf("backlog antigo nao foi drenado: %d comando(s)", len(w.pendingCommands))
	}
}

func TestHandlerPanicFailsClosedWithoutPersistingPartialAccounts(t *testing.T) {
	first, _ := networkedTestPlayer(1, "First", 2100, 2100)
	second, _ := networkedTestPlayer(2, "Second", 2101, 2100)
	store := &craftStore{}
	w := worldWithNetworkedPlayers(first, second)
	w.store = store

	w.failClosedAfterHandlerPanic()
	if !w.shuttingDown || !first.PersistencePoisoned || !second.PersistencePoisoned {
		t.Fatalf("mundo nao falhou fechado: shutdown=%v first=%v second=%v",
			w.shuttingDown, first.PersistencePoisoned, second.PersistencePoisoned)
	}
	if !first.Session.IsClosed() || !second.Session.IsClosed() {
		t.Fatal("sessoes permaneceram abertas depois do panic")
	}
	w.onDisconnect(first.Session)
	if store.saves != 0 {
		t.Fatalf("estado potencialmente parcial foi persistido: saves=%d", store.saves)
	}
}

func TestWorldRejectsQueuedGameplayAfterFinalShutdownSnapshot(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.InWorld = true
	w.shuttingDown = true

	pkt := wire.Build(wire.OpCharacterLogout, p.ID, 12)
	w.handle(command{s: p.Session, pkt: pkt})

	if !p.InWorld {
		t.Fatal("comando enfileirado depois do shutdown ainda alterou o personagem")
	}
	if st.saves != 0 {
		t.Fatalf("comando rejeitado gerou %d save(s)", st.saves)
	}
}
