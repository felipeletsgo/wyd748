package game

import (
	"bytes"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
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
	// Chat local is spatial/gameplay-space publication. Visible is only the
	// entity-materialization cache and may still be empty during a visibility
	// refresh; it must not silently drop a valid nearby message.
	w.itemInstances = make(map[string]*ItemInstance)
	w.itemInstances["chat-isolated-runtime"] = &ItemInstance{
		RuntimeID: "chat-isolated-runtime",
		MemberIDs: []uint16{third.ID},
	}
	w.setPlayerInstanceIndex(third.ID, "chat-isolated-runtime")

	beforeRecipient := recipient.Session.QueuedPacketsForTest()
	beforeThird := third.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket("hello local"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatal("local chat incorrectly depended on the Visible cache")
	}
	if third.Session.QueuedPacketsForTest() != beforeThird {
		t.Fatal("local chat leaked into another gameplay space")
	}

	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket(`/Recipient private`))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatal("whisper typed in normal chat was not routed")
	}

	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("Recipient", "direct"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatal("0x334 whisper did not reach its recipient")
	}

	party := &Party{Members: []*Player{sender, recipient}}
	sender.Party, recipient.Party = party, party
	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "=party"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatal("party chat did not reach the member")
	}

	beforeRecipient = recipient.Session.QueuedPacketsForTest()
	beforeThird = third.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "--global"))
	if recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 ||
		third.Session.QueuedPacketsForTest() != beforeThird+1 {
		t.Fatal("global chat was not broadcast")
	}
}

func TestChatHandlersRouteCitizenshipAndKingdomChannels(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	citizen, _ := networkedTestPlayer(2, "Citizen", 2101, 2100)
	outsider, _ := networkedTestPlayer(3, "Outsider", 2102, 2100)
	w := worldWithNetworkedPlayers(sender, citizen, outsider)
	sender.Char.Citizenship = 1
	citizen.Char.Citizenship = 1
	outsider.Char.Citizenship = 2

	beforeCitizen := citizen.Session.QueuedPacketsForTest()
	beforeOutsider := outsider.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "@@citizen"))
	if citizen.Session.QueuedPacketsForTest() != beforeCitizen+1 ||
		outsider.Session.QueuedPacketsForTest() != beforeOutsider {
		t.Fatal("citizenship chat ignored the character's channel")
	}

	// Kingdom is derived from the cape in the same way as the live server.
	sender.Char.Equip[model.CapeSlot].Index = 545
	citizen.Char.Equip[model.CapeSlot].Index = 545
	outsider.Char.Equip[model.CapeSlot].Index = 548
	beforeCitizen = citizen.Session.QueuedPacketsForTest()
	beforeOutsider = outsider.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "@kingdom"))
	if citizen.Session.QueuedPacketsForTest() != beforeCitizen+1 ||
		outsider.Session.QueuedPacketsForTest() != beforeOutsider {
		t.Fatal("kingdom chat ignored the character's kingdom")
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
		t.Fatalf("/limparinv was not dispatched: item=%d saves=%d", sender.Char.Inv[0].Index, st.saves)
	}

	sender.Char.Inv[1] = model.Item{Index: magicTrumpet}
	beforeRecipient := recipient.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket("/spk server announcement"))
	if sender.Char.Inv[1].Index != 0 || st.saves != 2 ||
		recipient.Session.QueuedPacketsForTest() != beforeRecipient+1 {
		t.Fatalf("/spk failed: item=%d saves=%d recipient=%d/%d",
			sender.Char.Inv[1].Index, st.saves, recipient.Session.QueuedPacketsForTest(), beforeRecipient)
	}

	beforeSender := sender.Session.QueuedPacketsForTest()
	w.onMessageChat(sender.Session, chatPacket("/spk no item"))
	if sender.Session.QueuedPacketsForTest() != beforeSender+1 {
		t.Fatal("/spk did not notify the player when the item was missing")
	}
}

func TestWhisperHandlerReportsOfflineAndCharacterInfo(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	recipient, _ := networkedTestPlayer(2, "Recipient", 2101, 2100)
	w := worldWithNetworkedPlayers(sender, recipient)

	before := sender.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("Offline", "hello"))
	if sender.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("offline whisper did not return a notice")
	}

	before = sender.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("Recipient", ""))
	if sender.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("/nick without text did not return character information")
	}
}

func TestWhisperDayRequestReturnsHiddenCalendarSync(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	w := worldWithNetworkedPlayers(sender)
	before := sender.Session.QueuedPacketsForTest()

	w.onMessageWhisper(sender.Session, whisperPacket("day", ""))

	if got := sender.Session.QueuedPacketsForTest(); got != before+1 {
		t.Fatalf("day did not produce sync: queue %d -> %d", before, got)
	}
}

func TestServerSwitchRequestReportsUnavailableWithoutMigration(t *testing.T) {
	sender, _ := networkedTestPlayer(1, "Sender", 2100, 2100)
	w := worldWithNetworkedPlayers(sender)
	oldX, oldY := sender.X, sender.Y

	for _, channel := range []string{"1", "0", "not-a-channel"} {
		w.onMessageWhisper(sender.Session, whisperPacket("srv", channel))
		if sender.Session.QueuedPacketsForTest() != 1 {
			t.Fatalf("srv request %q did not receive exactly one rejection", channel)
		}
		pkt, ok := sender.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel ||
			!bytes.Contains(pkt[12:107], []byte("Channel switching is unavailable")) {
			t.Fatalf("srv request %q did not return an unavailable notice", channel)
		}
		if sender.X != oldX || sender.Y != oldY || !sender.InWorld || w.players[sender.Session] != sender {
			t.Fatalf("srv request %q changed state without migration", channel)
		}
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
		t.Fatalf("native cannon not found: %+v", cannon)
	}
	if w.groundCannonAt(2101, 2100) != nil {
		t.Fatal("cannon found at the wrong coordinates")
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
		t.Fatal("free position within range was rejected")
	}
	if !w.castThornWall(p, req, skill, 40, 1) {
		t.Fatal("Thorn Wall vine was not summoned")
	}
	if len(w.sephiraObjects) != 1 || len(w.mobsByID) != 1 {
		t.Fatalf("Thorn Wall vine was not registered: sephira=%d mobs=%d", len(w.sephiraObjects), len(w.mobsByID))
	}

	var wall *Mob
	for _, mob := range w.sephiraObjects {
		wall = mob
	}
	if wall.HP != 500 || wall.Def.Score.CurHP != 500 || wall.SummonerID != p.ID {
		t.Fatalf("Thorn Wall vine has incorrect state: %+v", wall)
	}
	if w.canCastThornWall(p, req, skill) {
		t.Fatal("second Thorn Wall vine accepted an occupied coordinate")
	}

	w.tickSephiraObjects(time.Now().Add(thornWallLifetime + time.Second))
	if len(w.sephiraObjects) != 0 || len(w.mobsByID) != 0 || !wall.Dead {
		t.Fatalf("expired Thorn Wall vine remained: sephira=%d mobs=%d dead=%v",
			len(w.sephiraObjects), len(w.mobsByID), wall.Dead)
	}
}

func TestThornWallRejectsInvalidConfiguration(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Sephira", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.ghostShops = make(map[uint16]*GhostShop)
	skill := model.SkillDef{Index: 98, Range: 2}
	if w.canCastThornWall(p, skillCastRequest{TargetX: 2110, TargetY: 2100}, skill) {
		t.Fatal("out-of-range Thorn Wall vine was accepted")
	}
	if w.castThornWall(p, skillCastRequest{TargetX: 2101, TargetY: 2100}, skill, 0, 0) {
		t.Fatal("Thorn Wall vine was created without a template")
	}
}
