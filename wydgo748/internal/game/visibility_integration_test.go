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
		Score: model.Score{Version: model.ScoreVersion, MaxHP: 1, CurHP: 1},
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

func TestPrivateWaterPlayersAreIsolatedByRuntimeID(t *testing.T) {
	w, leader, member, _, _ := instanceTestWorld()
	outsider, _ := networkedTestPlayer(3, "Outsider", 2101, 2100)
	w.players[outsider.Session] = outsider
	w.playersByID[outsider.ID] = outsider
	w.updatePlayerSpatial(outsider)

	// Before entering Water the pair is a normal public-world visibility pair.
	w.showPlayerPair(leader, outsider)
	if !leader.hasVisible(outsider.ID) || !outsider.hasVisible(leader.ID) {
		t.Fatal("jogadores do mundo publico nao foram materializados")
	}

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.privateWaterRuntimeIDForPlayer(leader.ID) == "" ||
		w.privateWaterRuntimeIDForPlayer(member.ID) == "" {
		t.Fatal("membros da Water nao receberam RuntimeID privado")
	}
	if !w.playersVisibleTogether(leader, member) {
		t.Fatal("membros da mesma sala Water foram isolados entre si")
	}

	// Put the public player on the same physical tile as the private room. A
	// spatial radius alone would leak this entity; refresh must remove any
	// stale public CreateMob in both directions.
	outsider.X, outsider.Y = leader.X, leader.Y
	outsider.Char.X, outsider.Char.Y = outsider.X, outsider.Y
	w.updatePlayerSpatial(outsider)
	w.refreshPlayerVisibility(leader)
	w.refreshPlayerVisibility(outsider)
	if w.playersVisibleTogether(leader, outsider) ||
		leader.hasVisible(outsider.ID) || outsider.hasVisible(leader.ID) {
		t.Fatal("jogador publico vazou para dentro da Water privada")
	}
	outsiderPackets := outsider.Session.QueuedPacketsForTest()
	memberPackets := member.Session.QueuedPacketsForTest()
	w.syncPlayerVitals(leader)
	if outsider.Session.QueuedPacketsForTest() != outsiderPackets {
		t.Fatal("vitais da Water vazaram para o jogador publico")
	}
	if member.Session.QueuedPacketsForTest() <= memberPackets {
		t.Fatal("membro da mesma Water nao recebeu vitais")
	}

	// A second private room is also isolated even when it shares the same map
	// coordinates and physical visibility radius.
	second, _ := networkedTestPlayer(4, "Second", leader.X, leader.Y)
	second.Char.UID = "44444444444444448444444444444444"
	second.Char.Inv[0] = model.Item{Index: 100}
	w.players[second.Session] = second
	w.playersByID[second.ID] = second
	w.updatePlayerSpatial(second)
	w.onUseItem(second.Session, useItemPacket(0, 0))
	if w.privateWaterRuntimeIDForPlayer(second.ID) == "" ||
		w.playersVisibleTogether(leader, second) {
		t.Fatal("duas instancias Water privadas compartilharam visibilidade")
	}
	w.refreshPlayerVisibility(leader)
	w.refreshPlayerVisibility(second)
	if leader.hasVisible(second.ID) || second.hasVisible(leader.ID) {
		t.Fatal("CreateMob de uma sala Water concorrente foi publicado")
	}
}

func TestPrivateWaterGroundLootIsHiddenUntilRoomCleanup(t *testing.T) {
	member, _ := networkedTestPlayer(1, "Member", 2200, 2200)
	outsider, _ := networkedTestPlayer(2, "Outsider", 2200, 2200)
	w := &World{
		itemInstances: map[string]*ItemInstance{
			"water-normal-1:4": {
				Config:    model.VolatileInstance{ID: "water-normal-1"},
				RuntimeID: "water-normal-1:4",
				MemberIDs: []uint16{member.ID},
			},
		},
	}
	private := &GroundItem{ID: 10001, X: 2200, Y: 2200, InstanceID: "water-normal-1:4"}
	public := &GroundItem{ID: 10002, X: 2200, Y: 2200}
	if !w.groundItemVisibleToPlayer(member, private) ||
		w.groundItemVisibleToPlayer(outsider, private) {
		t.Fatal("loot privado foi visivel fora do RuntimeID")
	}
	if !w.groundItemVisibleToPlayer(outsider, public) {
		t.Fatal("loot publico foi ocultado")
	}
}
