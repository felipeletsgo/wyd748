package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCharacterLogoutReturnsToSelectionAndResetsRuntime(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Logout", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	w := worldWithNetworkedPlayers(p, observer)
	st := &craftStore{}
	w.store = st
	w.ghostShops = make(map[uint16]*GhostShop)
	observer.show(p.ID)
	p.ShopNPC = 1000
	p.CraftNPC = 1001
	p.PKMode = true
	p.SpecialCoins = map[string]uint32{"ticket": 5}

	w.onCharacterLogout(p.Session, make([]byte, 12))
	if p.InWorld || p.Char != nil || p.ID != 0 || p.ShopNPC != 0 || p.CraftNPC != 0 ||
		p.PKMode || p.SpecialCoins != nil {
		t.Fatalf("runtime nao foi resetado: %+v", p)
	}
	if w.playersByID[1] != nil || observer.hasVisible(1) || st.saves != 1 {
		t.Fatalf("logout deixou referencias: byID=%v visible=%v saves=%d",
			w.playersByID[1], observer.hasVisible(1), st.saves)
	}
	if w.players[p.Session] != p || p.Account == nil {
		t.Fatal("logout de personagem encerrou a sessao autenticada")
	}
}

func TestREQMobByIDRecoversMobAndPlayer(t *testing.T) {
	requester, _ := networkedTestPlayer(1, "Requester", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(requester, target)
	w.ghostShops = make(map[uint16]*GhostShop)
	mob := &Mob{ID: 1000, X: 2102, Y: 2100, Def: testNPCDef(model.Score{
		MaxHP: 100, CurHP: 100,
	})}
	w.registerMobSpatial(mob)

	pkt := make([]byte, 16)
	binary.LittleEndian.PutUint16(pkt[12:14], mob.ID)
	before := requester.Session.QueuedPacketsForTest()
	w.onREQMobByID(requester.Session, pkt)
	if !requester.hasVisible(mob.ID) || requester.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("REQMobByID nao recuperou o mob")
	}

	// Repetir uma entidade ja visivel ainda precisa reenviar o CreateMob para
	// recuperar a perda local do objeto no client.
	before = requester.Session.QueuedPacketsForTest()
	w.onREQMobByID(requester.Session, pkt)
	if requester.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("REQMobByID nao rematerializou mob ja marcado como visivel")
	}

	requester.hide(target.ID)
	binary.LittleEndian.PutUint16(pkt[12:14], target.ID)
	before = requester.Session.QueuedPacketsForTest()
	w.onREQMobByID(requester.Session, pkt)
	if !requester.hasVisible(target.ID) || requester.Session.QueuedPacketsForTest() != before+3 {
		t.Fatal("REQMobByID nao recuperou o jogador")
	}

	binary.LittleEndian.PutUint16(pkt[12:14], requester.ID)
	before = requester.Session.QueuedPacketsForTest()
	w.onREQMobByID(requester.Session, pkt)
	if requester.Session.QueuedPacketsForTest() != before {
		t.Fatal("REQMobByID respondeu para o proprio jogador")
	}
}

func TestREQMobByIDDoesNotLeakPrivateInstanceMob(t *testing.T) {
	member, _ := networkedTestPlayer(1, "Member", 2200, 2200)
	outsider, _ := networkedTestPlayer(2, "Outsider", 2200, 2200)
	w := worldWithNetworkedPlayers(member, outsider)
	inst := &ItemInstance{
		Config: model.VolatileInstance{ID: "private-req", Stages: []model.VolatileInstanceStage{{
			X: 2200, Y: 2200, AreaRadius: 8,
		}}},
		MemberIDs: []uint16{member.ID}, CurrentStage: 0,
	}
	w.itemInstances = map[string]*ItemInstance{inst.Config.ID: inst}
	mob := &Mob{ID: 1500, X: 2200, Y: 2200, HP: 100, InstanceID: inst.Config.ID,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w.mobs = []*Mob{mob}
	w.registerMobSpatial(mob)
	pkt := make([]byte, 16)
	binary.LittleEndian.PutUint16(pkt[12:14], mob.ID)
	before := outsider.Session.QueuedPacketsForTest()
	w.onREQMobByID(outsider.Session, pkt)
	if outsider.hasVisible(mob.ID) || outsider.Session.QueuedPacketsForTest() != before {
		t.Fatal("REQMobByID vazou mob de instancia para observador externo")
	}
	before = member.Session.QueuedPacketsForTest()
	w.onREQMobByID(member.Session, pkt)
	if !member.hasVisible(mob.ID) || member.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("REQMobByID nao recuperou mob para membro autorizado")
	}
}

func TestProtocolTelemetryHandlersAreRateLimited(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Protocol", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.lastProtocolNotice = make(map[uint16]time.Time)

	w.onMotion(p.Session, make([]byte, 19))
	w.onMotion(p.Session, make([]byte, 19))
	w.onClientUnknown2BC(p.Session, make([]byte, 107))
	if len(w.lastProtocolNotice) != 2 {
		t.Fatalf("avisos de protocolo nao foram limitados por opcode: %d", len(w.lastProtocolNotice))
	}
	// Tamanhos nativos sao reconhecidos silenciosamente.
	w.onMotion(p.Session, make([]byte, 20))
	w.onClientUnknown2BC(p.Session, make([]byte, 108))
	if len(w.lastProtocolNotice) != 2 {
		t.Fatal("pacote valido criou aviso de protocolo")
	}
}

func TestGuaranteedMobDropUsesInventoryAndGroundFallback(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Drops", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.items = map[uint16]model.ItemDef{500: {Index: 500}}
	w.groundItems = make(map[uint16]*GroundItem)
	w.ghostShops = make(map[uint16]*GhostShop)
	w.nextItemID = 10000
	carry := make([]model.Item, 12)
	carry[11] = model.Item{Index: 500, Eff: [6]byte{1, 2, 3, 4, 5, 6}}
	mob := &Mob{ID: 1000, X: 2100, Y: 2100, Def: &model.NPCDef{
		Name: "Dropper", Tipo: model.TipoMonstro, Carry: carry,
		Score: &model.Score{Version: model.ScoreVersion, Level: 10},
	}}

	w.rollMobDrops(p, mob)
	if p.Char.Inv[0].Index != 500 || p.Char.Inv[0].Eff[5] != 6 {
		t.Fatalf("drop garantido nao entrou completo no inventario: %+v", p.Char.Inv[0])
	}

	for i := range p.Char.Inv {
		p.Char.Inv[i] = model.Item{Index: 999}
	}
	w.rollMobDrops(p, mob)
	if len(w.groundItems) != 1 {
		t.Fatalf("inventario cheio nao enviou drop ao chao: %d", len(w.groundItems))
	}
}
