package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func chatPacket(message string) []byte {
	pkt := make([]byte, 12+len(message)+1)
	copy(pkt[12:], message)
	return pkt
}

func whisperPacket(target, message string) []byte {
	pkt := make([]byte, 128)
	copy(pkt[12:28], target)
	copy(pkt[28:124], message)
	return pkt
}

func TestChatHandlersRouteLocalWhisperAndChannels(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	recipient, _ := networkedTestPlayer(2, "Recipient", 2101, 2100)
	third, _ := networkedTestPlayer(3, "Third", 2102, 2100)
	w := worldWithNetworkedPlayers(sender, recipient, third)
	w.store = &craftStore{}
	sender.show(recipient.ID)
	recipient.show(sender.ID)
	third.show(sender.ID)

	beforeRecipient := recipient.Session.QueuedPacketsForTest()
	beforeThird := third.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket("hello local"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 ||
		third.Session.QueuedPacketsForTest() != beforeThird+1 {
		t.Fatal("chat local nao chegou a todos os observadores")
	}

	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket(`/Recipient private`))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatal("whisper digitado no chat comum nao foi roteado")
	}

	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("Recipient", "direct"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatal("whisper 0x334 nao chegou ao destinatario")
	}

	party := &Party{Members: []*Player{sender, recipient}}
	sender.Party, recipient.Party = party, party
	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "=party"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatal("chat de party nao chegou ao membro")
	}

	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	beforeThird = third.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "--global"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 ||
		third.Session.QueuedPacketsForTest() != beforeThird+1 {
		t.Fatal("chat global nao foi difundido")
	}
}

func TestChatCommandsClearInventoryAndShout(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	recipient, _ := networkedTestPlayer(2, "Recipient", 2101, 2100)
	w := worldWithNetworkedPlayers(sender, recipient)
	st := &craftStore{}
	w.store = st
	sender.Char.Inv[0] = model.Item{Index: 100}

	w.onMessageChat(sender.Session, chatPacket("/limparinv"))
	if sender.Char.Inv[0].Index != 0 || st.saves != 1 {
		t.Fatalf("/limparinv nao foi despachado: item=%d saves=%d", sender.Char.Inv[0].Index, st.saves)
	}

	sender.Char.Inv[1] = model.Item{Index: magicTrumpet}
	beforeRecipient := recipient.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket("/spk server announcement"))
	if sender.Char.Inv[1].Index != 0 || st.saves != 2 ||
		recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatalf("/spk incorreto: item=%d saves=%d recipient=%d/%d",
			sender.Char.Inv[1].Index, st.saves, recipient.Session.QueuedPacketsForTest(), beforeRecipient)
	}

	beforeSender := sender.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket("/spk no item"))
	if sender.Session.QueuedPacketsForTest() != beforeSender+1 {
		t.Fatal("/spk sem item nao informou o jogador")
	}
}

func TestWhisperHandlerReportsOfflineAndCharacterInfo(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	recipient, _ := networkedTestPlayer(2, "Recipient", 2101, 2100)
	w := worldWithNetworkedPlayers(sender, recipient)

	before := sender.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("Offline", "hello"))
	if sender.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("whisper offline nao retornou aviso")
	}

	before = sender.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("Recipient", ""))
	if sender.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("/nick sem texto nao retornou informacoes")
	}
}

func TestWhisperDayRequestReturnsHiddenCalendarSync(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	w := worldWithNetworkedPlayers(sender)
	before := sender.Session.QueuedPacketsForTest()

	w.onMessageWhisper(sender.Session, whisperPacket("day", ""))

	if got := sender.Session.QueuedPacketsForTest(); got != before+1 {
		t.Fatalf("day nao gerou sincronismo: fila %d -> %d", before, got)
	}
}

func TestSephiraCannonAndThornWallLifecycle(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Sephira", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.groundItems = map[uint16]*GroundItem{
		15001: {ID: 15001, Item: model.Item{Index: 746}, X: 2100, Y: 2100},
		10001: {ID: 10001, Item: model.Item{Index: 746}, X: 2100, Y: 2100},
	}
	w.ghostShops = make(map[uint16]*GhostShop)
	if cannon := w.groundCannonAt(2100, 2100); cannon == nil || cannon.ID != 15001 {
		t.Fatalf("canhao nativo nao encontrado: %+v", cannon)
	}
	if w.groundCannonAt(2101, 2100) != nil {
		t.Fatal("canhao encontrado em coordenada incorreta")
	}

	vine := model.NPCDef{
		Name: "Vinha", Tipo: model.TipoNPC,
		Score: &model.Score{
			Version: model.ScoreVersion, MaxHP: 500, CurHP: 100,
		},
	}
	w.npcs = []model.NPCDef{vine}
	w.nextMobID = firstMobID
	skill := model.SkillDef{Index: 98, Range: 6}
	req := skillCastRequest{TargetX: 2102, TargetY: 2100}
	if !w.canCastThornWall(p, req, skill) {
		t.Fatal("posicao livre e em alcance foi recusada")
	}
	if !w.castThornWall(p, req, skill, 40, 1) {
		t.Fatal("Vinha nao foi evocada")
	}
	if len(w.sephiraObjects) != 1 || len(w.mobsByID) != 1 {
		t.Fatalf("Vinha nao foi registrada: sephira=%d mobs=%d", len(w.sephiraObjects), len(w.mobsByID))
	}

	var wall *Mob
	for _, mob := range w.sephiraObjects {
		wall = mob
	}
	if wall.HP != 500 || wall.Def.Score.CurHP != 500 || wall.SummonerID != p.ID {
		t.Fatalf("estado da Vinha incorreto: %+v", wall)
	}
	if w.canCastThornWall(p, req, skill) {
		t.Fatal("segunda Vinha aceitou coordenada ocupada")
	}

	w.tickSephiraObjects(time.Now().Add(thornWallLifetime + time.Second))
	if len(w.sephiraObjects) != 0 || len(w.mobsByID) != 0 || !wall.Dead {
		t.Fatalf("Vinha expirada permaneceu: sephira=%d mobs=%d dead=%v",
			len(w.sephiraObjects), len(w.mobsByID), wall.Dead)
	}
}

func TestThornWallRejectsInvalidConfiguration(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Sephira", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.ghostShops = make(map[uint16]*GhostShop)
	skill := model.SkillDef{Index: 98, Range: 2}
	if w.canCastThornWall(p, skillCastRequest{TargetX: 2110, TargetY: 2100}, skill) {
		t.Fatal("Vinha fora de alcance foi aceita")
	}
	if w.castThornWall(p, skillCastRequest{TargetX: 2101, TargetY: 2100}, skill, 0, 0) {
		t.Fatal("Vinha sem template foi criada")
	}
}
