package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math/rand"
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
			t.Fatal("Enqueue perdeu sessao/pacote")
		}
	})

	t.Run("cargo access binds packet to nearby banker", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Cargo", 100, 100)
		banker := &Mob{ID: 1000, X: 102, Y: 100, Def: &model.NPCDef{
			Name: "Cargo", Extended: &model.ExtendedScore{
				Version: model.ExtendedScoreVersion, Merchant: 2,
			},
		}}
		w := testSpatialWorld([]*Mob{banker}, p)
		pkt := make([]byte, 20)
		binary.LittleEndian.PutUint32(pkt[16:20], uint32(banker.ID))
		if !w.validCargoAccess(p, pkt) {
			t.Fatal("banker visivel foi recusado")
		}
		binary.LittleEndian.PutUint32(pkt[16:20], 0x1_0000)
		if w.validCargoAccess(p, pkt) || w.validCargoAccess(nil, pkt) ||
			w.validCargoAccess(p, pkt[:19]) {
			t.Fatal("cargo aceitou id DWORD, jogador nil ou pacote truncado")
		}
	})

	t.Run("illusion uses skill data mana and destination", func(t *testing.T) {
		p, _ := networkedTestPlayer(1, "Huntress", 100, 100)
		p.Char.Class = 3
		p.Char.LearnedSkill = 1 << 1
		p.Char.Extended.Mastery[1] = 40
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
			t.Fatal("quest bloqueada nao informou a validacao")
		}
	})

	t.Run("summon movement respects cooldown", func(t *testing.T) {
		summon := &Mob{ID: 1000, X: 100, Y: 100, Def: testNPCDef(
			model.ExtendedScore{AttackRun: 4, MaxHP: 100, CurHP: 100})}
		w := testSpatialWorld([]*Mob{summon})
		now := time.Unix(2_000_000_000, 0)
		w.moveSummonToward(summon, 110, 100, 1, now)
		movedX := summon.X
		if movedX <= 100 || summon.NextMove.IsZero() {
			t.Fatalf("summon nao moveu: x=%d next=%v", summon.X, summon.NextMove)
		}
		w.moveSummonToward(summon, 110, 100, 1, now)
		if summon.X != movedX {
			t.Fatal("summon ignorou o cooldown de movimento")
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
			t.Fatal("craft atomico valido nao foi confirmado")
		}

		oldInv, oldEquip, oldGold = p.Char.Inv, p.Char.Equip, p.Char.Gold
		p.Char.Inv[0] = model.Item{Index: 888}
		st.atomicErr = errors.New("database unavailable")
		if w.commitCombineWithPlayerState(p, oldInv, oldEquip, oldGold,
			map[int]struct{}{0: {}}, nil, 1) || p.Char.Inv[0].Index != 777 {
			t.Fatal("falha do craft atomico nao restaurou o inventario")
		}
	})
}

func TestSummonCombatCoversAttackFollowPassiveAndImmobileKinds(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "BeastMaster", 100, 100)
	target := &Mob{ID: 1500, X: 102, Y: 100, HP: 1000,
		Def: testNPCDef(model.ExtendedScore{
			Defense: 0, MaxHP: 1000, CurHP: 1000,
		})}
	attacker := &Mob{ID: 1600, X: 101, Y: 100, HP: 100,
		SummonerID: owner.ID, SummonRange: 2,
		Def: testNPCDef(model.ExtendedScore{
			Attack: 500, AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	follower := &Mob{ID: 1601, X: 50, Y: 50, HP: 100,
		SummonerID: owner.ID,
		Def: testNPCDef(model.ExtendedScore{
			Attack: 100, AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	pet := &Mob{ID: 1602, X: 85, Y: 85, HP: 100,
		SummonerID: owner.ID, SummonKind: summonKindMount,
		Def: testNPCDef(model.ExtendedScore{
			AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	wall := &Mob{ID: 1603, X: 80, Y: 80, HP: 100,
		SummonerID: owner.ID, SummonKind: summonKindThornWall,
		Def: testNPCDef(model.ExtendedScore{
			AttackRun: 4, MaxHP: 100, CurHP: 100,
		})}
	w := testSpatialWorld([]*Mob{target, attacker, follower, pet, wall}, owner)
	for _, summon := range []*Mob{attacker, follower, pet, wall} {
		w.summons[summon.ID] = summon
	}
	owner.CombatTargetID = target.ID
	now := time.Unix(2_000_000_000, 0)
	w.tickSummonCombat(now)
	if target.HP != 500 || attacker.TargetID != target.ID ||
		attacker.NextAttack.IsZero() {
		t.Fatalf("ataque da evocacao: hp=%d target=%d next=%v",
			target.HP, attacker.TargetID, attacker.NextAttack)
	}
	// Remove a ordem: evocacao comum e cria passam a apenas acompanhar.
	owner.CombatTargetID = 0
	oldFollowerX, oldPetX, oldWallX := follower.X, pet.X, wall.X
	w.tickSummonCombat(now.Add(3 * time.Second))
	if follower.X == oldFollowerX || pet.X == oldPetX {
		t.Fatalf("seguidores nao acompanharam: normal=%d/%d pet=%d/%d",
			oldFollowerX, follower.X, oldPetX, pet.X)
	}
	if wall.X != oldWallX {
		t.Fatal("Thorn Wall se moveu")
	}
}

func networkedTestPlayer(id uint16, name string, x, y uint16) (*Player, *gameNet.Session) {
	session := gameNet.NewTestSession(int64(id), 128)
	ch := &model.Char{
		Name: name, UID: fmt.Sprintf("test-character-%d-%s", id, name), X: x, Y: y,
		Extended: testExtended(model.ExtendedScore{
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
	applyExtendedScore(p.Char)
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
		t.Fatalf("convite nao registrado: from=%d until=%v", member.InviteFrom, member.InviteUntil)
	}
	if memberSession.QueuedPacketsForTest() != 1 {
		t.Fatalf("convite enviou %d pacotes", memberSession.QueuedPacketsForTest())
	}

	accept := make([]byte, 30)
	binary.LittleEndian.PutUint16(accept[12:14], leader.ID)
	copy(accept[14:30], leader.Char.Name)
	w.onPartyAccept(memberSession, accept)
	if leader.Party == nil || member.Party != leader.Party || len(leader.Party.Members) != 2 ||
		leader.Party.leader() != leader {
		t.Fatalf("party nao foi formada: leader=%+v memberParty=%p", leader.Party, member.Party)
	}

	remove := make([]byte, 16) // target zero = o proprio membro sai
	w.onPartyRemove(memberSession, remove)
	if leader.Party != nil || member.Party != nil {
		t.Fatalf("party de dois nao foi dissolvida: leader=%p member=%p", leader.Party, member.Party)
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
		t.Fatal("convite expirado foi aceito ou nao foi limpo")
	}

	party := &Party{Members: []*Player{leader, member, other}}
	leader.Party, member.Party, other.Party = party, party, party
	remove := make([]byte, 16)
	binary.LittleEndian.PutUint32(remove[12:16], uint32(other.ID))
	w.onPartyRemove(memberSession, remove)
	if other.Party != party {
		t.Fatal("membro comum expulsou outro membro")
	}
	if memberSession.QueuedPacketsForTest() == 0 || otherSession.QueuedPacketsForTest() != 0 {
		t.Fatal("recusa de expulsao nao foi enviada somente ao solicitante")
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
		t.Fatalf("whisper+carta enviaram %d pacotes", recipientSession.QueuedPacketsForTest())
	}
	w.deliverWhisper(senderSession, sender, "Offline", "hello")
	if senderSession.QueuedPacketsForTest() != 1 {
		t.Fatalf("offline nao gerou aviso: %d", senderSession.QueuedPacketsForTest())
	}

	party := &Party{Members: []*Player{sender, recipient}}
	sender.Party, recipient.Party = party, party
	w.sendPartyChat(sender, "party")
	if recipientSession.QueuedPacketsForTest() != 3 {
		t.Fatalf("chat party nao chegou: %d", recipientSession.QueuedPacketsForTest())
	}
	if got := w.localChatObservers(sender); got != 1 {
		t.Fatalf("observadores locais=%d", got)
	}
	if got := w.playerByCharacterName("recipient"); got != recipient {
		t.Fatalf("lookup por nome=%p, quer %p", got, recipient)
	}

	recipient.Char.Citizenship = 2
	w.sendCharacterInfo(senderSession, sender, "Recipient")
	if senderSession.QueuedPacketsForTest() != 2 {
		t.Fatalf("consulta de personagem nao respondeu: %d", senderSession.QueuedPacketsForTest())
	}
	if displayLevel(recipient.Char) != 11 || characterCitizenship(recipient.Char) != "Canal 2" {
		t.Fatalf("info incorreta: level=%d citizenship=%q",
			displayLevel(recipient.Char), characterCitizenship(recipient.Char))
	}
	if displayLevel(nil) != 1 || characterCitizenship(nil) != "" {
		t.Fatal("fallback de info nil incorreto")
	}
}

func TestDeathLetterAndHPDebugProduceClientFeedback(t *testing.T) {
	victim, session := networkedTestPlayer(1, "Victim", 2100, 2100)
	w := worldWithNetworkedPlayers(victim)
	w.receiveDeathLetter(victim, "Gremlin", "mob")
	if session.QueuedPacketsForTest() != 1 {
		t.Fatalf("carta de morte nao enviada: %d", session.QueuedPacketsForTest())
	}
	w.receiveDeathLetter(nil, "Gremlin", "mob")
	w.receiveDeathLetter(victim, "", "mob")

	w.dumpHPProjection(session, victim)
	if session.QueuedPacketsForTest() != 5 {
		t.Fatalf("hpdebug deveria acrescentar quatro mensagens, fila=%d", session.QueuedPacketsForTest())
	}
	w.dumpHPProjection(session, nil)
}

func TestNPCInteractionResolutionAndMessages(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Player", 100, 100)
	npc := &Mob{ID: 1000, X: 102, Y: 100, Def: &model.NPCDef{Name: "Merchant"}}
	w := testSpatialWorld([]*Mob{npc}, p)

	if _, err := w.resolveNPCInteraction(nil, npc.ID); err != errNPCNotFound {
		t.Fatalf("player nil: %v", err)
	}
	if _, err := w.resolveNPCInteraction(p, 9999); err != errNPCNotFound {
		t.Fatalf("npc ausente: %v", err)
	}
	if _, err := w.resolveNPCInteraction(p, npc.ID); err != errNPCNotVisible {
		t.Fatalf("npc invisivel: %v", err)
	}
	p.Visible[npc.ID] = struct{}{}
	if got, err := w.resolveNPCInteraction(p, npc.ID); err != nil || got != npc {
		t.Fatalf("npc valido: got=%p err=%v", got, err)
	}
	p.X = 200
	if _, err := w.resolveNPCInteraction(p, npc.ID); err == nil {
		t.Fatal("npc distante foi aceito")
	} else if npcInteractionMessage(err) != "Move closer to interact." {
		t.Fatalf("mensagem distancia=%q", npcInteractionMessage(err))
	}
	if npcInteractionMessage(errNPCNotVisible) != "That character is not visible." ||
		npcInteractionMessage(errNPCNotFound) != "That character is not available." {
		t.Fatal("mensagens de interacao incorretas")
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
		t.Fatalf("teleporte: pos=(%d,%d) gold=%d saves=%d", p.X, p.Y, p.Char.Gold, st.saves)
	}

	pk := make([]byte, 16)
	binary.LittleEndian.PutUint32(pk[12:16], 1)
	w.onPKMode(session, pk)
	if !p.PKMode {
		t.Fatal("PK mode nao foi ativado")
	}
	binary.LittleEndian.PutUint32(pk[12:16], 0)
	w.onPKMode(session, pk)
	if p.PKMode {
		t.Fatal("PK mode nao foi desativado")
	}

	challenge := make([]byte, 16)
	binary.LittleEndian.PutUint32(challenge[12:16], 77)
	before := session.QueuedPacketsForTest()
	w.onGuildChallenge(session, challenge)
	if session.QueuedPacketsForTest() != before+1 {
		t.Fatal("desafio de guild nao retornou aviso seguro")
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
	if criticalHit(100, nil) {
		t.Fatal("criticalHit com cursor nil")
	}

	attacker, _ := networkedTestPlayer(1, "Attacker", 100, 100)
	target, _ := networkedTestPlayer(2, "Target", 101, 100)
	attacker.Char.Extended.Attack = 2000
	attacker.Char.Extended.Dex = 1000
	attacker.Char.Extended.Accuracy = 1000
	target.Char.Extended.Dex = 0
	target.Char.Extended.Defense = 10
	applyExtendedScore(attacker.Char)
	applyExtendedScore(target.Char)
	mob := &Mob{
		ID: 1000, HP: 1000,
		Def: testNPCDef(model.ExtendedScore{
			Level: 1, Attack: 1000, Defense: 10, Str: 100, Dex: 0, MaxHP: 1000, CurHP: 1000,
		}),
	}
	if damage := playerHitsMob(attacker, mob); damage == 0 {
		t.Fatal("ataque autoritativo contra mob deu zero")
	}
	if damage := playerHitsPlayer(attacker, target); damage == 0 {
		t.Fatal("ataque autoritativo PvP deu zero")
	}
	if damage := mobHitsPlayer(mob, target.Char); damage == 0 {
		t.Fatal("ataque autoritativo do mob deu zero")
	}
	if playerHitsMob(nil, mob) != 0 || playerHitsPlayer(nil, target) != 0 || mobHitsPlayer(nil, target.Char) != 0 {
		t.Fatal("combate nil deveria retornar zero")
	}
	if playerSkillPoints(attacker.Char) != 9 || playerAttackRun(attacker.Char) != 4 {
		t.Fatal("accessors Extended retornaram valor errado")
	}
}

func TestMobCombatAppliesDamageAndLethalState(t *testing.T) {
	target, session := networkedTestPlayer(1, "Target", 101, 100)
	mob := &Mob{
		ID: 1000, X: 100, Y: 100, HP: 1000, Awake: true, TargetID: target.ID,
		Def: testNPCDef(model.ExtendedScore{
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
		t.Fatalf("dano nao letal: hp=%d attacker=%d", playerCurHP(target.Char), target.LastAttackerID)
	}
	w.applyMobDamageToPlayer(mob, target, 5000, now.Add(time.Second), func(applied uint32) []byte {
		return make([]byte, 12)
	})
	if playerCurHP(target.Char) != 0 || target.DeadAt.IsZero() || mob.TargetID != 0 {
		t.Fatalf("morte nao aplicada: hp=%d dead=%v target=%d",
			playerCurHP(target.Char), target.DeadAt, mob.TargetID)
	}
	if session.QueuedPacketsForTest() == 0 {
		t.Fatal("combate nao publicou pacotes")
	}
}

func TestAdvancedCraftHelpersAndDeterministicRecipes(t *testing.T) {
	if advancedEvolution(nil, "celestial") {
		t.Fatal("evolution nil aceita")
	}
	ch := &model.Char{Evolution: "Celestial"}
	if !advancedEvolution(ch, "mortal", "celestial") || advancedEvolution(ch, "arch") {
		t.Fatal("advancedEvolution nao ignorou caixa/lista")
	}
	var req combineRequest
	req.Items[0].Index, req.Items[1].Index = 1, 2
	if !exactRecipe(req, []uint16{1, 2}) || exactRecipe(req, []uint16{1}) {
		t.Fatal("exactRecipe incorreta")
	}
	for _, index := range []uint16{540, 551, 595, 663, 1738} {
		if !odinTargetBlocked(index) {
			t.Errorf("alvo Odin %d deveria estar bloqueado", index)
		}
	}
	if odinTargetBlocked(500) || odinRefineBonus(11) != 5 || odinRefineBonus(14) != 1 ||
		odinRefineBonus(10) != 0 {
		t.Fatal("helpers Odin incorretos")
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
		t.Fatal("mountSuccessRate nao limitou faixa")
	}
	for index, category := range map[uint16]int{
		2333: 0, 2336: 1, 2339: 2, 2346: 3, 2351: 4, 2354: 5, 2349: 6, 2300: -1,
	} {
		if got := growthCategory(index); got != category {
			t.Errorf("growthCategory(%d)=%d, quer %d", index, got, category)
		}
	}

	mount := model.Item{Index: 2333}
	mount.SetMountLongev(59)
	//lint:ignore SA1019 o codigo nativo usa o gerador global; a semente torna este teste deterministico
	rand.Seed(1)
	if ok, _ := mountLongevityRecover(&mount); !ok || mount.MountLongev() < 59 || mount.MountLongev() > 60 {
		t.Fatalf("longevidade nao recuperada: %d", mount.MountLongev())
	}
	mount.SetMountLongev(60)
	if ok, _ := mountLongevityRecover(&mount); ok {
		t.Fatal("longevidade maxima foi recuperada")
	}
	if ok, _ := mountGrowth(&mount, 3344); !ok || mount.Index != 2363 {
		t.Fatalf("growth nao evoluiu: ok=%v index=%d", ok, mount.Index)
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
		t.Fatal("acelerador nao transformou o ovo equipado")
	}

	// O consumo e a transformacao do ovo sao atomicos do ponto de vista do
	// jogador: se o store falhar, ambos os slots voltam ao snapshot original.
	p.Char.Equip[mountSlot] = egg
	p.Char.Inv[1] = accelerator
	oldEgg, oldAccelerator := p.Char.Equip[mountSlot], p.Char.Inv[1]
	w.store = &craftStore{err: errors.New("disco cheio")}
	w.accelerateHatch(p, session, &p.Char.Inv[1], 1, req)
	if p.Char.Equip[mountSlot] != oldEgg || p.Char.Inv[1] != oldAccelerator {
		t.Fatalf("falha do acelerador alterou estado: ovo=%+v/%+v acelerador=%+v/%+v",
			p.Char.Equip[mountSlot], oldEgg, p.Char.Inv[1], oldAccelerator)
	}

	adult := model.Item{Index: model.MountAdultBase}
	adult.SetMountHP(1000)
	adult.SetMountLongev(20)
	p.Char.Equip[mountSlot] = adult
	before := p.Char.Equip[mountSlot].MountLongev()
	//lint:ignore SA1019 o codigo nativo usa o gerador global; a semente torna este teste deterministico
	rand.Seed(2)
	w.mountRiderDied(p)
	after := p.Char.Equip[mountSlot].MountLongev()
	if after > before || after < before-3 {
		t.Fatalf("custo de morte da montaria=%d -> %d", before, after)
	}
}

func TestSkillPVPAffectHelpers(t *testing.T) {
	ch := &model.Char{Extended: testExtended(model.ExtendedScore{
		ResistFire: 1, ResistIce: 2, ResistHoly: 3, ResistThunder: 4,
	})}
	if hasActiveAffect(nil, 1) {
		t.Fatal("affect ativo em char nil")
	}
	if !setOwnedAffect(ch, 7, 28, 10, 20, 10) || !hasActiveAffect(ch, 28) {
		t.Fatal("affect com owner nao foi aplicado")
	}
	foundOwner := false
	for _, affect := range ch.Affects {
		if affect.Type == 28 && affect.OwnerID == 7 {
			foundOwner = true
		}
	}
	if !foundOwner {
		t.Fatal("OwnerID do affect nao persistiu")
	}
	resists := playerElementalResists(ch)
	if resists.Fire != 1 || resists.Ice != 2 || resists.Sacred != 3 || resists.Thunder != 4 {
		t.Fatalf("resistencias=%+v", resists)
	}
	if !removePlayerAffectTypes(ch, 28) || hasActiveAffect(ch, 28) ||
		removePlayerAffectTypes(ch, 99) {
		t.Fatal("remocao de affects incorreta")
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
			t.Errorf("event %d=%q, quer %q", event, got, expected)
		}
	}
}
