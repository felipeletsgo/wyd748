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

func TestChatHandlersRouteCitizenshipAndChannelChat(t *testing.T) {
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

	// '@' is the channel chat: every character in this world receives it,
	// regardless of citizenship or kingdom cape.
	sender.Char.Equip[model.CapeSlot].Index = 545
	outsider.Char.Equip[model.CapeSlot].Index = 548
	beforeCitizen = citizen.Session.QueuedPacketsForTest()
	beforeOutsider = outsider.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "@channel"))
	if citizen.Session.QueuedPacketsForTest() != beforeCitizen+1 ||
		outsider.Session.QueuedPacketsForTest() != beforeOutsider+1 {
		t.Fatal("channel chat did not reach every character on the channel")
	}

	// Neutral citizenship is an independent audience. The client echoes the
	// sender locally, so the server must deliver only to the other recipients.
	sender.Char.Citizenship = 0
	outsider.Char.Citizenship = 0
	beforeSender := sender.Session.QueuedPacketsForTest()
	beforeCitizen = citizen.Session.QueuedPacketsForTest()
	beforeOutsider = outsider.Session.QueuedPacketsForTest()
	w.onMessageWhisper(sender.Session, whisperPacket("", "@@neutral"))
	if sender.Session.QueuedPacketsForTest() != beforeSender ||
		outsider.Session.QueuedPacketsForTest() != beforeOutsider+1 ||
		citizen.Session.QueuedPacketsForTest() != beforeCitizen {
		t.Fatal("neutral citizenship chat did not stay within its audience")
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

func TestServerSwitchRejectsUnavailableDestinationWithoutMigration(t *testing.T) {
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
			!bytes.Contains(pkt[12:107], []byte("Use srv 1 or srv 2")) {
			t.Fatalf("srv request %q did not return a destination rejection", channel)
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

func thornWallTestWorld(t *testing.T) (*World, *Player, *Mob) {
	t.Helper()
	p, _ := networkedTestPlayer(1, "Sephira", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.terrain = loadedFlatTerrain()
	w.ghostShops = make(map[uint16]*GhostShop)
	w.npcs = []model.NPCDef{{
		Name: "Vinha", Tipo: model.TipoMonstro,
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 500, CurHP: 500},
	}}
	w.nextMobID = firstMobID
	skill := model.SkillDef{Index: 98, Range: 6}
	if !w.castThornWall(p, skillCastRequest{TargetX: 2102, TargetY: 2100}, skill, 40, 1) {
		t.Fatal("Thorn Wall vine was not summoned")
	}
	for _, mob := range w.sephiraObjects {
		return w, p, mob
	}
	t.Fatal("Thorn Wall vine was not registered")
	return nil, nil, nil
}

// Native Vinha is an ordinary mob: players damage and destroy it, it grants no
// reward, and it never targets players.
func TestThornWallIsAttackableWithoutReward(t *testing.T) {
	w, p, wall := thornWallTestWorld(t)
	if !w.playerCanInteractWithMob(p, wall) {
		t.Fatal("player cannot attack the Thorn Wall vine")
	}
	if w.mobCanTargetPlayer(wall, p) {
		t.Fatal("Thorn Wall vine can target a player")
	}
	if w.hostileMob(wall) {
		t.Fatal("Thorn Wall vine became a hostile mob for guards and summons")
	}
	exp, gold := p.Char.Exp, p.Char.Gold
	wall.HP = 0
	if plan := w.planMobKill(p, wall, 500, 500); plan != nil {
		t.Fatalf("Thorn Wall kill produced a reward plan: %+v", plan)
	}
	if !wall.Dead || len(w.sephiraObjects) != 0 || len(w.mobsByID) != 0 || len(w.summons) != 0 {
		t.Fatalf("destroyed Thorn Wall vine remained: dead=%v sephira=%d mobs=%d summons=%d",
			wall.Dead, len(w.sephiraObjects), len(w.mobsByID), len(w.summons))
	}
	if p.Char.Exp != exp || p.Char.Gold != gold {
		t.Fatalf("Thorn Wall kill granted a reward: exp %d->%d gold %d->%d", exp, p.Char.Exp, gold, p.Char.Gold)
	}
}

// The native countdown runs only in peace mode: damage holds the lifetime.
func TestThornWallLifetimePausesWhileDamaged(t *testing.T) {
	w, _, wall := thornWallTestWorld(t)
	start := wall.ExpiresAt.Add(-thornWallLifetime)
	w.tickSephiraObjects(start)
	wall.HP -= 10
	w.tickSephiraObjects(start.Add(5 * time.Second))
	w.tickSephiraObjects(start.Add(10 * time.Second))
	if want := start.Add(thornWallLifetime + 10*time.Second); !wall.ExpiresAt.Equal(want) {
		t.Fatalf("damaged wall expires at %v, want %v", wall.ExpiresAt, want)
	}
	w.tickSephiraObjects(start.Add(thornWallLifetime + 5*time.Second))
	if wall.Dead {
		t.Fatal("damaged wall expired before its held lifetime")
	}
	w.tickSephiraObjects(start.Add(thornWallLifetime + 10*time.Second))
	if !wall.Dead || len(w.sephiraObjects) != 0 {
		t.Fatal("wall did not expire after combat ended")
	}
}

// Player routes may cross occupied tiles, but a thorn wall stops them.
func TestThornWallBlocksPlayerRoute(t *testing.T) {
	w, p, wall := thornWallTestWorld(t)
	now := w.now()
	w.beginPlayerMovement(p, 2100, 2100, 2103, 2100, []byte("666"), []byte("666"), 0, now)
	w.advancePlayerMovement(p, now.Add(10*time.Second))
	if p.X != 2101 || p.Y != 2100 {
		t.Fatalf("player crossed the Thorn Wall vine: position=(%d,%d)", p.X, p.Y)
	}

	w.destroyPlayerSummon(wall, p.ID)
	now = w.now()
	w.beginPlayerMovement(p, 2101, 2100, 2103, 2100, []byte("66"), []byte("66"), 0, now)
	w.advancePlayerMovement(p, now.Add(10*time.Second))
	if p.X != 2103 {
		t.Fatalf("route stayed blocked after the wall was destroyed: x=%d", p.X)
	}
}
