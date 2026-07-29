package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestVisibilityShowHidePlayerAndGhostShopPairs(t *testing.T) {
	a, aSession := networkedTestPlayer(1, "Alpha", 2100, 2100)
	b, bSession := networkedTestPlayer(2, "Beta", 2101, 2100)
	w := worldWithNetworkedPlayers(a, b)

	w.showPlayerPair(a, b)
	if !a.hasVisible(b.ID) || !b.hasVisible(a.ID) {
		t.Fatal("jogadores proximos nao foram materializados mutuamente")
	}
	aPackets, bPackets := aSession.QueuedPacketsForTest(), bSession.QueuedPacketsForTest()
	w.showPlayerPair(a, b)
	if aSession.QueuedPacketsForTest() != aPackets || bSession.QueuedPacketsForTest() != bPackets {
		t.Fatal("showPlayerPair duplicou CreateMob de entidade ja visivel")
	}
	w.hidePlayerPair(a, b)
	if a.hasVisible(b.ID) || b.hasVisible(a.ID) {
		t.Fatal("hidePlayerPair conservou entidade visivel")
	}

	shop := &GhostShop{
		ID: 25001, Name: "Carbunkle", Title: "Test Shop", X: 2100, Y: 2100,
		Extended: model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1, CurHP: 1},
	}
	w.showGhostShop(a, shop)
	if !a.hasVisible(shop.ID) {
		t.Fatal("loja fantasma nao apareceu")
	}
	w.hideGhostShop(a, shop)
	if a.hasVisible(shop.ID) {
		t.Fatal("loja fantasma nao foi removida")
	}
}

func TestRevivedPlayerIsRematerializedAliveForObservers(t *testing.T) {
	subject, subjectSession := networkedTestPlayer(1, "Revived", 2100, 2100)
	observer, observerSession := networkedTestPlayer(2, "Observer", 2101, 2100)
	w := worldWithNetworkedPlayers(subject, observer)
	w.showPlayerPair(subject, observer)
	setPlayerCurHP(subject.Char, 0)

	before := observerSession.QueuedPacketsForTest()
	w.rematerializePlayerAfterRevive(subject)
	if observerSession.QueuedPacketsForTest() != before {
		t.Fatal("personagem ainda morto foi rematerializado")
	}

	setPlayerCurHP(subject.Char, playerMaxHP(subject.Char))
	w.rematerializePlayerAfterRevive(subject)
	if !observer.hasVisible(subject.ID) {
		t.Fatal("observador nao recebeu novamente o personagem revivido")
	}
	if observerSession.QueuedPacketsForTest() <= before || subjectSession.QueuedPacketsForTest() == 0 {
		t.Fatal("revive nao publicou remocao/criacao/vitais")
	}
}

func TestVisibilityGuardsIgnoreInvalidEntities(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Player", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.showMob(nil, nil)
	w.hideMob(nil, nil, 0)
	w.showGhostShop(nil, nil)
	w.hideGhostShop(nil, nil)
	w.showPlayerPair(nil, p)
	w.hidePlayerPair(nil, p)
	w.rematerializePlayerAfterRevive(nil)
	if packets := playerEnterViewPackets(nil); packets != nil {
		t.Fatal("entidade nil gerou pacotes de entrada")
	}
}
