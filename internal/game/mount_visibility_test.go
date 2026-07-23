package game

import (
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

// O backstop de montaria roda periodicamente. Uma cria que ja existe deve ser
// reconciliada por identidade, nunca rematerializada a cada tick.
func TestSyncCriaPetDoesNotRecreateExistingPet(t *testing.T) {
	session := gameNet.NewTestSession(1, 8)
	ch := &model.Char{Extended: testExtended(model.ExtendedScore{
		Level: 10, MaxHP: 100, CurHP: 100,
	})}
	ch.Equip[mountSlot] = model.Item{Index: model.MountBabyBase}
	ch.Equip[mountSlot].SetMountHP(50)
	p := &Player{ID: 1, X: 2100, Y: 2100, InWorld: true, Char: ch, Session: session}
	w := testSpatialWorld(nil, p)
	w.nextMobID = 1000

	w.syncCriaPet(p)
	pet := w.findCriaPet(p.ID)
	if pet == nil || len(w.mobs) != 1 {
		t.Fatalf("primeiro sync nao criou exatamente uma cria: pet=%v mobs=%d", pet, len(w.mobs))
	}
	if got := session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("spawn da cria enviou %d pacotes, esperado um CreateMob", got)
	}

	w.syncCriaPet(p)
	if current := w.findCriaPet(p.ID); current != pet || len(w.mobs) != 1 {
		t.Fatalf("segundo sync recriou a cria: antiga=%p atual=%p mobs=%d", pet, current, len(w.mobs))
	}
	if got := session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("cria existente gerou %d pacotes; nenhum pacote novo era esperado", got)
	}
}
