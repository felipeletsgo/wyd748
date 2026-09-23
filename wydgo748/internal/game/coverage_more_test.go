package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math/rand"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestPreviouslyUncoveredAuthoritativeEntryPoints(t *testing.T) {
	t.Run("real rng and world enqueue", func(t *testing.T) {
		if got := (realRNG{}).Intn(1); got != 0 {
			t.Fatalf("Intn(1)=%d", got)
		}
		w := &World{commands: make(chan command, 1)}
		session := gameNet.NewTestSession(7, 1)
		packet := []byte{1, 2, 3}
		w.Enqueue(session, packet)
		queued := <-w.commands
		if queued.s != session || len(queued.pkt) != len(packet) {
			t.Fatal("Enqueue lost a session or packet")
		}
	})

	t.Run("cargo access binds packet to nearby banker", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Cargo", 100, 100)
		banker := &Mob{ID: 1000, X: 102, Y: 100, Def: &model.NPCDef{
			Name: "Cargo", Score: &model.Score{
				Version: model.ScoreVersion, Merchant: 2,
			},
		}}
		w := testSpatialWorld([]*Mob{banker}, p)
		p.show(banker.ID)
		pkt := make([]byte, 20)
		binary.LittleEndian.PutUint32(pkt[16:20], uint32(banker.ID))
		if !w.validCargoAccess(p, pkt) {
			t.Fatal("visible banker identified by MSG_SwapItem was rejected")
		}
		if p.CargoNPC != banker.ID {
			t.Fatal("valid MSG_SwapItem did not bind the native Cargo context")
		}
		banker.X = p.X + npcInteractionRange + 1
		w.moveMobSpatial(banker, 102, 100)
		if w.validCargoAccess(p, pkt) {
			t.Fatal("cargo used visual range instead of the NPC boundary")
		}
		banker.X = 102
		w.moveMobSpatial(banker, p.X+npcInteractionRange+1, p.Y)
		binary.LittleEndian.PutUint32(pkt[16:20], 0x1_0000)
		if w.validCargoAccess(p, pkt) || w.validCargoAccess(nil, pkt) ||
			w.validCargoAccess(p, pkt[:19]) {
			t.Fatal("cargo accepted a DWORD ID, nil player, or truncated packet")
		}

		binary.LittleEndian.PutUint32(pkt[16:20], uint32(banker.ID))
		p.CargoNPC = 0
		if !w.nearCargoNPC(p) || p.CargoNPC != banker.ID {
			t.Fatal("Cargo operation without TargetID did not resolve the nearest visible banker")
		}
		p.hide(banker.ID)
		p.CargoNPC = 0
		if w.nearCargoNPC(p) || p.CargoNPC != 0 {
			t.Fatal("cargo accepted a banker not materialized for the client")
		}
	})

	t.Run("illusion uses skill data mana and destination", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Huntress", 100, 100)
		p.Char.Class = 3
		p.Char.LearnedSkill = 1 << 1
		p.Char.Score.Mastery[1] = 40
		setPlayerCurMP(p.Char, 800)
		w := worldWithNetworkedPlayers(p)
		w.skills = map[int]model.SkillDef{
			73: {Index: 73, ManaSpent: 10, Delay: 1, Range: 8},
		}
		pkt := make([]byte, 28)
		binary.LittleEndian.PutUint16(pkt[24:26], 105)
		binary.LittleEndian.PutUint16(pkt[26:28], 100)
		beforeMP := playerCurMP(p.Char)
		w.onIllusionMove(p, pkt)
		if p.X != 105 || p.Y != 100 || playerCurMP(p.Char) >= beforeMP ||
			p.SkillReady[73].IsZero() {
			t.Fatalf("Illusion: pos=(%d,%d) mp=%d/%d cooldown=%v",
				p.X, p.Y, playerCurMP(p.Char), beforeMP, p.SkillReady[73])
		}
	})

	t.Run("quest interaction always revalidates server-side", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Quest", 100, 100)
		w := worldWithNetworkedPlayers(p)
		quest := &model.QuestDef{
			ID: 999, Requires: model.QuestRequirements{MinLevel: 99},
		}
		mob := &Mob{ID: 1000, X: 101, Y: 100, Def: &model.NPCDef{Name: "QuestNPC"}}
		before := p.Session.QueuedPacketsForTest()
		w.onQuestInteraction(p.Session, p, mob, quest, 1)
		if p.Session.QueuedPacketsForTest() <= before {
			t.Fatal("blocked quest did not report validation failure")
		}
	})

	t.Run("summon movement respects cooldown", func(t *testing.T) {
		summon := &Mob{ID: 1000, X: 100, Y: 100, Def: testNPCDef(
			model.Score{AttackRun: 4, MaxHP: 100, CurHP: 100})}
		w := testSpatialWorld([]*Mob{summon})
		now := time.Unix(2_000_000_000, 0)
		w.moveSummonToward(summon, 110, 100, 1, now)
		movedX := summon.X
		if movedX <= 100 || summon.NextMove.IsZero() {
			t.Fatalf("summon did not move: x=%d next=%v", summon.X, summon.NextMove)
		}
		w.moveSummonToward(summon, 110, 100, 1, now)
		if summon.X != movedX {
			t.Fatal("summon ignored movement cooldown")
		}
	})

	t.Run("combine with charstate is one transaction and rolls back", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "AtomicCraft", 100, 100)
		w := worldWithNetworkedPlayers(p)
		st := &atomicCharStateMemoryStore{}
		w.store = st
		oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
		p.Char.Inv[0] = model.Item{Index: 777}
		p.SpecialCoins = map[string]uint32{"fame": 1}
		if !w.commitCombineWithPlayerState(p, oldInv, oldEquip, oldGold,
			map[int]struct{}{0: {}}, nil, 1) || st.atomicSaves != 1 {
			t.Fatal("valid atomic craft was not confirmed")
		}

		oldInv, oldEquip, oldGold = p.Char.Inv, p.Char.Equip, p.Char.Gold
		p.Char.Inv[0] = model.Item{Index: 888}
		st.atomicErr = errors.New("database unavailable")
		if w.commitCombineWithPlayerState(p, oldInv, oldEquip, oldGold,
			map[int]struct{}{0: {}}, nil, 1) || p.Char.Inv[0].Index != 777 {
			t.Fatal("atomic craft failure did not restore inventory")
		}
	})
}

func TestSummonCombatCoversAttackFollowPassiveAndImmobileKinds(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "BeastMaster", 100, 100)
	target := &Mob{ID: 1500, X: 102, Y: 100, HP: 1000,
		Def: testNPCDef(model.Score{
			Defense: 0, MaxHP: 1000, CurHP: 1000,
		})}
	attacker := &Mob{ID: 1600, X: 101, Y: 100, HP: 100,
		SummonerID: owner.ID, SummonRange: 2,
		Def: testNPCDef(model.Score{
			Attack: 500, AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	follower := &Mob{ID: 1601, X: 50, Y: 50, HP: 100,
		SummonerID: owner.ID,
		Def: testNPCDef(model.Score{
			Attack: 100, AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	pet := &Mob{ID: 1602, X: 85, Y: 85, HP: 100,
		SummonerID: owner.ID, SummonKind: summonKindMount,
		Def: testNPCDef(model.Score{
			AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	wall := &Mob{ID: 1603, X: 80, Y: 80, HP: 100,
		SummonerID: owner.ID, SummonKind: summonKindThornWall,
		Def: testNPCDef(model.Score{
			AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	w := testSpatialWorld([]*Mob{target, attacker, follower, pet, wall}, owner)
	w.rng = fixedRNG{value: 0}
	for _, summon := range []*Mob{attacker, follower, pet, wall} {
		w.summons[summon.ID] = summon
	}
	owner.CombatTargetID = target.ID
	now := time.Unix(2_000_000_000, 0)
	w.tickSummonCombat(now)
	if target.HP != 500 || attacker.TargetID != target.ID ||
		attacker.NextAttack.IsZero() {
		t.Fatalf("summon attack: hp=%d target=%d next=%v",
			target.HP, attacker.TargetID, attacker.NextAttack)
	}
	// Remove the order: ordinary summons and pets should only follow.
	owner.CombatTargetID = 0
	oldFollowerX, oldPetX, oldWallX := follower.X, pet.X, wall.X
	w.tickSummonCombat(now.Add(3 * time.Second))
	if follower.X == oldFollowerX || pet.X == oldPetX {
		t.Fatalf("followers did not keep up: normal=%d/%d pet=%d/%d",
			oldFollowerX, follower.X, oldPetX, pet.X)
	}
	if wall.X != oldWallX {
		t.Fatal("Thorn Wall moved")
	}
}

func networkedTestPlayer(id uint16, name string, x, y uint16) (*Player, *gameNet.Session) {
	session := gameNet.NewTestSession(int64(id), 128)
	ch := &model.Char{
		Name: name, UID: fmt.Sprintf("test-character-%d-%s", id, name), X: x, Y: y,
		Score: testScore(model.Score{
			Level: 10, Attack: 500, MagicAttack: 600, Defense: 100,
			Str: 100, Int: 100, Dex: 100, Con: 100,
			MaxHP: 1000, CurHP: 1000, MaxMP: 800, CurMP: 800,
			SkillPts: 9, AttackRun: 4,
		}),
	}
	acc := &model.Account{Name: name + "Account", PasswordHash: "hash", Chars: []model.Char{*ch}}
	p := &Player{
		ID: id, Session: session, Account: acc, Char: &acc.Chars[0], CharSlot: 0,
		InWorld: true, X: x, Y: y, Visible: make(map[uint16]struct{}),
	}
	applyScore(p.Char)
	return p, session
}

func worldWithNetworkedPlayers(players ...*Player) *World {
	w := testSpatialWorld(nil)
	w.players = make(map[*gameNet.Session]*Player, len(players))
	w.playersByID = make(map[uint16]*Player, len(players))
	w.playersByCharacterUID = make(map[string]*Player, len(players))
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	for _, p := range players {
		w.players[p.Session] = p
		w.playersByID[p.ID] = p
		w.indexPlayerCharacter(p)
		w.updatePlayerSpatial(p)
	}
	return w
}

func TestPartyInviteAcceptAndLeaveLifecycle(t *testing.T) {
	leader, leaderSession := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, memberSession := networkedTestPlayer(2, "Member", 2101, 2100)
	w := worldWithNetworkedPlayers(leader, member)

	request := make([]byte, 44)
	binary.LittleEndian.PutUint32(request[40:44], uint32(member.ID))
	w.onPartyRequest(leaderSession, request)
	if member.InviteFrom != leader.ID || member.InviteUntil.IsZero() {
		t.Fatalf("invite not registered: from=%d until=%v", member.InviteFrom, member.InviteUntil)
	}
	if memberSession.QueuedPacketsForTest() != 2 {
		t.Fatalf("invite sent %d packets", memberSession.QueuedPacketsForTest())
	}

	accept := make([]byte, 30)
	binary.LittleEndian.PutUint16(accept[12:14], leader.ID)
	copy(accept[14:30], leader.Char.Name)
	w.onPartyAccept(memberSession, accept)
	if leader.Party == nil || member.Party != leader.Party || len(leader.Party.Members) != 2 ||
		leader.Party.leader() != leader {
		t.Fatalf("party was not formed: leader=%+v memberParty=%p", leader.Party, member.Party)
	}

	remove := make([]byte, 16) // Target zero means the member leaves.
	w.onPartyRemove(memberSession, remove)
	if leader.Party != nil || member.Party != nil {
		t.Fatalf("two-member party was not dissolved: leader=%p member=%p", leader.Party, member.Party)
	}
}

func TestPartyRejectsExpiredInviteAndNonLeaderRemoval(t *testing.T) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, memberSession := networkedTestPlayer(2, "Member", 2101, 2100)
	other, otherSession := networkedTestPlayer(3, "Other", 2102, 2100)
	w := worldWithNetworkedPlayers(leader, member, other)

	member.InviteFrom = leader.ID
	member.InviteUntil = time.Now().Add(-time.Second)
	accept := make([]byte, 30)
	binary.LittleEndian.PutUint16(accept[12:14], leader.ID)
	copy(accept[14:30], leader.Char.Name)
	w.onPartyAccept(memberSession, accept)
	if member.Party != nil || member.InviteFrom != 0 {
		t.Fatal("expired invite was accepted or not cleared")
	}

	party := &Party{Members: []*Player{leader, member, other}}
	leader.Party, member.Party, other.Party = party, party, party
	remove := make([]byte, 16)
	binary.LittleEndian.PutUint32(remove[12:16], uint32(other.ID))
	w.onPartyRemove(memberSession, remove)
	if other.Party != party {
		t.Fatal("ordinary member expelled another member")
	}
	if memberSession.QueuedPacketsForTest() == 0 || otherSession.QueuedPacketsForTest() != 0 {
		t.Fatal("expulsion rejection was not sent only to the requester")
	}
}

func TestWhisperMailPartyChatAndCharacterInfo(t *testing.T) {
	sender, senderSession := networkedTestPlayer(1, "Sender", 2100, 2100)
	recipient, recipientSession := networkedTestPlayer(2, "Recipient", 2101, 2100)
	w := worldWithNetworkedPlayers(sender, recipient)
	sender.Visible[recipient.ID] = struct{}{}
	recipient.Visible[sender.ID] = struct{}{}

	w.deliverWhisper(senderSession, sender, "Recipient", "hello")
	w.deliverWhisper(senderSession, sender, "Recipient", "!mail")
	if recipientSession.QueuedPacketsForTest() != 2 {
		t.Fatalf("whisper+letter sent %d packets", recipientSession.QueuedPacketsForTest())
	}
	w.deliverWhisper(senderSession, sender, "Offline", "hello")
	if senderSession.QueuedPacketsForTest() != 1 {
		t.Fatalf("offline lookup did not produce a notice: %d", senderSession.QueuedPacketsForTest())
	}

	party := &Party{Members: []*Player{sender, recipient}}
	sender.Party, recipient.Party = party, party
	w.sendPartyChat(sender, "party")
	if recipientSession.QueuedPacketsForTest() != 3 {
		t.Fatalf("party chat did not arrive: %d", recipientSession.QueuedPacketsForTest())
	}
	if got := w.playerByCharacterName("recipient"); got != recipient {
		t.Fatalf("name lookup=%p, want %p", got, recipient)
	}

	recipient.Char.Citizenship = 2
	w.sendCharacterInfo(senderSession, sender, "Recipient")
	if senderSession.QueuedPacketsForTest() != 2 {
		t.Fatalf("character lookup did not respond: %d", senderSession.QueuedPacketsForTest())
	}
	if displayLevel(recipient.Char) != 11 || characterCitizenship(recipient.Char) != "Channel 2" {
		t.Fatalf("incorrect info: level=%d citizenship=%q",
			displayLevel(recipient.Char), characterCitizenship(recipient.Char))
	}
	if info := w.characterInfoLine(recipient.Char); !strings.Contains(info, "No guild") ||
		!strings.Contains(info, "Citizenship: Channel 2") {
		t.Fatalf("character information is not in English: %q", info)
	}
	if displayLevel(nil) != 1 || characterCitizenship(nil) != "" {
		t.Fatal("incorrect nil-info fallback")
	}
}

func TestNPCInteractionResolutionAndMessages(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Player", 100, 100)
	npc := &Mob{ID: 1000, X: 102, Y: 100, Def: &model.NPCDef{Name: "Merchant"}}
	w := testSpatialWorld([]*Mob{npc}, p)

	if _, err := w.resolveNPCInteraction(nil, npc.ID); err != errNPCNotFound {
		t.Fatalf("player nil: %v", err)
	}
	if _, err := w.resolveNPCInteraction(p, 9999); err != errNPCNotFound {
		t.Fatalf("missing NPC: %v", err)
	}
	if _, err := w.resolveNPCInteraction(p, npc.ID); err != errNPCNotVisible {
		t.Fatalf("invisible NPC: %v", err)
	}
	p.Visible[npc.ID] = struct{}{}
	if got, err := w.resolveNPCInteraction(p, npc.ID); err != nil || got != npc {
		t.Fatalf("valid NPC: got=%p err=%v", got, err)
	}
	p.X = 200
	if _, err := w.resolveNPCInteraction(p, npc.ID); err == nil {
		t.Fatal("distant NPC was accepted")
	} else if npcInteractionMessage(err) != "Move closer to interact." {
		t.Fatalf("distance message=%q", npcInteractionMessage(err))
	}
	if npcInteractionMessage(errNPCNotVisible) != "That character is not visible." ||
		npcInteractionMessage(errNPCNotFound) != "That character is not available." {
		t.Fatal("incorrect interaction messages")
	}
}

func TestTeleportPKAndGuildChallengeHandlers(t *testing.T) {
	p, session := networkedTestPlayer(1, "Traveler", 2118, 2102)
	st := &craftStore{}
	w := worldWithNetworkedPlayers(p)
	w.store = st
	w.teleports = []model.Teleport{{
		SourceX: 2118, SourceY: 2102, DestX: 2200, DestY: 2201, Price: 100, Name: "Destination",
	}}
	p.Char.Gold = 500
	w.onReqTeleport(session, make([]byte, 16))
	if p.X != 2200 || p.Y != 2201 || p.Char.Gold != 400 || st.saves != 1 {
		t.Fatalf("teleport: pos=(%d,%d) gold=%d saves=%d", p.X, p.Y, p.Char.Gold, st.saves)
	}

	pk := make([]byte, 16)
	binary.LittleEndian.PutUint32(pk[12:16], 1)
	beforePKPackets := session.QueuedPacketsForTest()
	w.onPKMode(session, pk)
	if !p.PKMode {
		t.Fatal("PK mode was not enabled")
	}
	if got := session.QueuedPacketsForTest(); got != beforePKPackets+1 {
		t.Fatalf("active PK mode published a packet without a 7.48 consumer: got=%d want=%d", got, beforePKPackets+1)
	}
	binary.LittleEndian.PutUint32(pk[12:16], 0)
	beforePKPackets = session.QueuedPacketsForTest()
	w.onPKMode(session, pk)
	if p.PKMode {
		t.Fatal("PK mode was not disabled")
	}
	if got := session.QueuedPacketsForTest(); got != beforePKPackets+1 {
		t.Fatalf("inactive PK mode published a packet without a 7.48 consumer: got=%d want=%d", got, beforePKPackets+1)
	}
	binary.LittleEndian.PutUint32(pk[12:16], 2)
	w.onPKMode(session, pk)
	if p.PKMode {
		t.Fatal("PK mode accepted a value outside the 0/1 contract")
	}

	challenge := make([]byte, 16)
	binary.LittleEndian.PutUint32(challenge[12:16], 77)
	before := session.QueuedPacketsForTest()
	w.onGuildChallenge(session, challenge)
	if session.QueuedPacketsForTest() != before+1 {
		t.Fatal("guild challenge did not return a safe notice")
	}
}

func TestCombatPathsUseAuthoritativeExtendedStats(t *testing.T) {
	for _, tc := range []struct{ dam, ac, combat int }{
		{500, 100, 0}, {10, 1000, 0}, {100, 200, 20},
	} {
		if got := hitDamage(tc.dam, tc.ac, tc.combat); got < 1 {
			t.Fatalf("hitDamage(%d,%d,%d)=%d", tc.dam, tc.ac, tc.combat, got)
		}
	}
	if double, critical := rollPhysicalHitFlags(&model.Char{}, nil, func(int) int { return 0 }); double || critical {
		t.Fatal("physical flags accepted a nil server progression")
	}

	attacker, _ := networkedTestPlayer(1, "Attacker", 100, 100)
	target, _ := networkedTestPlayer(2, "Target", 101, 100)
	attacker.Char.Score.Attack = 2000
	attacker.Char.Score.Dex = 1000
	attacker.Char.Score.Accuracy = 1000
	target.Char.Score.Dex = 0
	target.Char.Score.Defense = 10
	applyScore(attacker.Char)
	applyScore(target.Char)
	mob := &Mob{
		ID: 1000, HP: 1000,
		Def: testNPCDef(model.Score{
			Level: 1, Attack: 1000, Defense: 10, Str: 100, Dex: 0, MaxHP: 1000, CurHP: 1000,
		}),
	}
	zero := func(int) int { return 0 }
	if damage := playerHitsMobAt(attacker, mob, zero, time.Now()); damage == 0 {
		t.Fatal("authoritative attack against a mob returned zero")
	}
	if damage := playerHitsPlayerWithRNG(attacker, target, zero); damage == 0 {
		t.Fatal("authoritative PvP attack returned zero")
	}
	if damage := mobHitsPlayerAt(mob, target.Char, zero, time.Now()); damage == 0 {
		t.Fatal("authoritative mob attack returned zero")
	}
	if playerHitsMob(nil, mob) != 0 || playerHitsPlayer(nil, target) != 0 || mobHitsPlayer(nil, target.Char) != 0 {
		t.Fatal("nil combat should return zero")
	}
	if playerSkillPoints(attacker.Char) != 9 || playerAttackRun(attacker.Char) != 4 {
		t.Fatal("Score accessors returned incorrect values")
	}
}

func TestMobCombatAppliesDamageAndLethalState(t *testing.T) {
	target, session := networkedTestPlayer(1, "Target", 101, 100)
	mob := &Mob{
		ID: 1000, X: 100, Y: 100, HP: 1000, Awake: true, TargetID: target.ID,
		Def: testNPCDef(model.Score{
			Attack: 1000, Str: 100, Dex: 0, MaxHP: 1000, CurHP: 1000,
		}),
	}
	mob.Def.Name = "Gremlin"
	w := worldWithNetworkedPlayers(target)
	w.mobs = []*Mob{mob}
	w.registerMobSpatial(mob)
	w.activeMobs = map[uint16]*Mob{mob.ID: mob}
	target.Visible[mob.ID] = struct{}{}

	now := time.Now()
	w.applyMobDamageToPlayer(mob, target, 100, now, func(applied uint32) []byte {
		return make([]byte, 12)
	})
	if playerCurHP(target.Char) != 900 || target.LastAttackerID != mob.ID {
		t.Fatalf("nonlethal damage: hp=%d attacker=%d", playerCurHP(target.Char), target.LastAttackerID)
	}
	w.applyMobDamageToPlayer(mob, target, 5000, now.Add(time.Second), func(applied uint32) []byte {
		return make([]byte, 12)
	})
	if playerCurHP(target.Char) != 0 || target.DeadAt.IsZero() || mob.TargetID != 0 {
		t.Fatalf("death was not applied: hp=%d dead=%v target=%d",
			playerCurHP(target.Char), target.DeadAt, mob.TargetID)
	}
	if session.QueuedPacketsForTest() == 0 {
		t.Fatal("combat did not publish packets")
	}
}

func TestAdvancedCraftHelpersAndDeterministicRecipes(t *testing.T) {
	if advancedEvolution(nil, "celestial") {
		t.Fatal("nil evolution was accepted")
	}
	ch := &model.Char{Evolution: "Celestial"}
	if !advancedEvolution(ch, "mortal", "celestial") || advancedEvolution(ch, "arch") {
		t.Fatal("advancedEvolution did not ignore storage/list")
	}
	var req combineRequest
	req.Items[0].Index, req.Items[1].Index = 1, 2
	if !exactRecipe(req, []uint16{1, 2}) || exactRecipe(req, []uint16{1}) {
		t.Fatal("incorrect exactRecipe")
	}
	for _, index := range []uint16{540, 551, 595, 663, 1738} {
		if !odinTargetBlocked(index) {
			t.Errorf("Odin target %d should be blocked", index)
		}
	}
	if odinTargetBlocked(500) || odinRefineBonus(11) != 5 || odinRefineBonus(14) != 1 ||
		odinRefineBonus(10) != 0 {
		t.Fatal("incorrect Odin helpers")
	}

	ehreWorld, ehrePlayer, ehreSession, ehreStore := newCraftWorld(t, "Ehre", nil, 0)
	var soulItems [combineSlots]model.Item
	for i, index := range soulRecipes[0] {
		soulItems[i].Index = index
	}
	var soulPos [combineSlots]int8
	for i := range soulPos {
		soulPos[i] = int8(i)
	}
	placeItems(ehrePlayer.Char, soulItems, soulPos)
	ehreWorld.onCombineEhre(ehreSession, buildCombinePacket(soulItems, soulPos))
	if ehrePlayer.Char.SoulInfo != 1 || ehreStore.saves != 1 {
		t.Fatalf("Ehre: soul=%d saves=%d", ehrePlayer.Char.SoulInfo, ehreStore.saves)
	}

	odinWorld, odinPlayer, odinSession, odinStore := newCraftWorld(t, "Odin",
		map[uint16]model.ItemDef{5134: {Index: 5134}}, 0)
	var clueItems [combineSlots]model.Item
	var cluePos [combineSlots]int8
	for i := 0; i < 7; i++ {
		clueItems[i].Index = 413
		cluePos[i] = int8(i)
	}
	cluePos[7] = 7
	placeItems(odinPlayer.Char, clueItems, cluePos)
	odinWorld.onCombineOdin(odinSession, buildCombinePacket(clueItems, cluePos))
	if odinPlayer.Char.Inv[0].Index != 5134 || odinStore.saves != 1 {
		t.Fatalf("Odin clue: item=%d saves=%d", odinPlayer.Char.Inv[0].Index, odinStore.saves)
	}
}

func TestMountUtilityAndLifecycleBranches(t *testing.T) {
	if mountSuccessRate(-10) != 100 || mountSuccessRate(999) != 20 {
		t.Fatal("mountSuccessRate did not clamp the range")
	}
	for index, category := range map[uint16]int{
		2333: 0, 2336: 1, 2339: 2, 2346: 3, 2351: 4, 2354: 5, 2349: 6, 2300: -1,
	} {
		if got := growthCategory(index); got != category {
			t.Errorf("growthCategory(%d)=%d, want %d", index, got, category)
		}
	}

	mount := model.Item{Index: 2333}
	mount.SetMountLongev(59)
	//lint:ignore SA1019 native code uses the global generator; the seed makes this test deterministic
	rand.Seed(1)
	if ok, _ := mountLongevityRecover(&mount); !ok || mount.MountLongev() < 59 || mount.MountLongev() > 60 {
		t.Fatalf("longevity was not restored: %d", mount.MountLongev())
	}
	mount.SetMountLongev(60)
	if ok, _ := mountLongevityRecover(&mount); ok {
		t.Fatal("maximum longevity was restored")
	}
	if ok, _ := mountGrowth(&mount, 3344); !ok || mount.Index != 2363 {
		t.Fatalf("growth did not evolve: ok=%v index=%d", ok, mount.Index)
	}

	p, session := networkedTestPlayer(1, "Rider", 100, 100)
	egg := model.Item{Index: model.MountEggBase}
	setEggDelay(&egg, 10)
	p.Char.Equip[mountSlot] = egg
	accelerator := model.Item{Index: 999}
	p.Char.Inv[1] = accelerator
	w := worldWithNetworkedPlayers(p)
	w.store = &craftStore{}
	req := useItemRequest{dstType: placeEquip, dstPos: mountSlot}
	w.accelerateHatch(p, session, &p.Char.Inv[1], 1, req)
	if !model.IsMountBaby(p.Char.Equip[mountSlot].Index) || p.Char.Inv[1].Index != 0 {
		t.Fatal("accelerator did not transform the equipped egg")
	}

	// Consuming and transforming the egg are atomic from the player's perspective.
	// If storage fails, both player slots return to the original snapshot.
	p.Char.Equip[mountSlot] = egg
	p.Char.Inv[1] = accelerator
	oldEgg, oldAccelerator := p.Char.Equip[mountSlot], p.Char.Inv[1]
	w.store = &craftStore{err: errors.New("disk full")}
	w.accelerateHatch(p, session, &p.Char.Inv[1], 1, req)
	if p.Char.Equip[mountSlot] != oldEgg || p.Char.Inv[1] != oldAccelerator {
		t.Fatalf("accelerator failure changed state: egg=%+v/%+v accelerator=%+v/%+v",
			p.Char.Equip[mountSlot], oldEgg, p.Char.Inv[1], oldAccelerator)
	}

	adult := model.Item{Index: model.MountAdultBase}
	adult.SetMountHP(1000)
	adult.SetMountLongev(20)
	p.Char.Equip[mountSlot] = adult
	before := p.Char.Equip[mountSlot].MountLongev()
	//lint:ignore SA1019 native code uses the global generator; the seed makes this test deterministic
	rand.Seed(2)
	w.mountRiderDied(p)
	after := p.Char.Equip[mountSlot].MountLongev()
	if after > before || after < before-3 {
		t.Fatalf("mount death cost=%d -> %d", before, after)
	}
}

func TestSkillPVPAffectHelpers(t *testing.T) {
	ch := &model.Char{Score: testScore(model.Score{
		ResistFire: 1, ResistIce: 2, ResistHoly: 3, ResistThunder: 4,
	})}
	if hasActiveAffect(nil, 1) {
		t.Fatal("affect remained active on a nil character")
	}
	if !setOwnedAffect(ch, 7, 28, 10, 20, 10) || !hasActiveAffect(ch, 28) {
		t.Fatal("affect with owner was not applied")
	}
	foundOwner := false
	for _, affect := range ch.Affects {
		if affect.Type == 28 && affect.OwnerID == 7 {
			foundOwner = true
		}
	}
	if !foundOwner {
		t.Fatal("affect OwnerID was not persisted")
	}
	resists := playerElementalResists(ch)
	if resists.Fire != 1 || resists.Ice != 2 || resists.Sacred != 3 || resists.Thunder != 4 {
		t.Fatalf("resistances=%+v", resists)
	}
	if !removePlayerAffectTypes(ch, 28) || hasActiveAffect(ch, 28) ||
		removePlayerAffectTypes(ch, 99) {
		t.Fatal("incorrect affect removal")
	}
}

func TestBossEventStringCoversAllKnownAndUnknownValues(t *testing.T) {
	want := map[BossEventType]string{
		BossEventCombatStarted:          "combat_started",
		BossEventDamaged:                "damaged",
		BossEventHealthThresholdCrossed: "threshold_crossed",
		BossEventAddDied:                "add_died",
		BossEventType(255):              "desconhecido",
	}
	for event, expected := range want {
		if got := event.String(); got != expected {
			t.Errorf("event %d=%q, want %q", event, got, expected)
		}
	}
}
