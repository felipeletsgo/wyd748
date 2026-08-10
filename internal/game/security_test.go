package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func inboundPacket(opcode uint16, size int) []byte {
	packet := make([]byte, size)
	binary.LittleEndian.PutUint16(packet[0:2], uint16(size))
	binary.LittleEndian.PutUint16(packet[4:6], opcode)
	return packet
}

func TestExactInboundPacketSizeCoversEveryConfirmed748Opcode(t *testing.T) {
	tests := map[uint16]int{
		wire.OpConnectAccount:     116,
		wire.OpCreateCharacter:    36,
		wire.OpDeleteCharacter:    44,
		wire.OpCharacterLogin:     characterLoginPacketSize,
		wire.OpCharacterLogout:    12,
		wire.OpSwapItem:           20,
		wire.OpDeposit:            16,
		wire.OpWithdraw:           16,
		wire.OpUseItem:            36,
		wire.OpUsePremiumFirework: premiumFireworkUsePacketSize,
		wire.OpUseNPC:             20,
		wire.OpReqShopList:        16,
		wire.OpBuyItem:            24,
		wire.OpSellItem:           20,
		wire.OpApplyBonus:         applyBonusPacketSize,
		wire.OpPartyRequest:       44,
		wire.OpPartyAccept:        32,
		wire.OpPartyRemove:        16,
		wire.OpTrade:              156,
		wire.OpCloseTrade:         12,
		wire.OpAutoTrade:          196,
		wire.OpReqTradeList:       16,
		wire.OpReqBuyAutoTrade:    36,
		wire.OpDropItem:           32,
		wire.OpGetItem:            28,
		wire.OpDeleteItem:         deleteItemPacketSize,
		wire.OpSplitItem:          splitItemPacketSize,
		wire.OpUpdateItem:         20,
		wire.OpSetShortSkill:      32,
		wire.OpMessageChat:        140,
		wire.OpMessageWhisper:     128,
		wire.OpChangeCity:         16,
		wire.OpReqTeleport:        16,
		wire.OpPKMode:             16,
		wire.OpMoveStop:           36,
		wire.OpUpdateScore:        wire.HeaderSize,
		wire.OpRestart:            12,
		wire.OpPing:               12,
		wire.OpSysQuit:            16,
		wire.OpAction:             52,
		wire.OpActionStop:         52,
		wire.OpIllusion:           52,
		wire.OpREQMobByID:         16,
		wire.OpGuildDeprivate:     16,
		wire.OpInviteGuild:        20,
		wire.OpGuildAlly:          20,
		wire.OpGuildWar:           20,
		wire.OpChallenge:          16,
		wire.OpChallengeConfirm:   20,
		wire.OpMotion:             20,
		wire.OpClientUnknown2BC:   108,
		wire.OpReqRanking:         20,
		wire.OpAttackOne:          48,
		wire.OpAttackTwo:          52,
		wire.OpAttackMulti:        96,
		wire.OpCombineTiny:        combinePacketSize,
		wire.OpCombineLindy:       combinePacketSize,
		wire.OpCombineCompositor:  combinePacketSize,
		wire.OpCombineAgatha:      combinePacketSize,
		wire.OpCombineAylin:       combinePacketSize,
		wire.OpCombineEhre:        combinePacketSize,
		wire.OpCombineOdin:        combinePacketSize,
	}
	for opcode, expected := range tests {
		got, exact := exactInboundPacketSize(opcode)
		if !exact || got != expected {
			t.Errorf("opcode 0x%X: size=%d exact=%v, esperado=%d",
				opcode, got, exact, expected)
		}
	}
	if size, exact := exactInboundPacketSize(0xFFFF); exact || size != 0 {
		t.Fatalf("opcode desconhecido ganhou framing: size=%d exact=%v", size, exact)
	}
	if size, exact := exactInboundPacketSize(wire.OpRebuy); exact || size != 0 {
		t.Fatalf("recompra deveria aceitar os dois tamanhos: size=%d exact=%v", size, exact)
	}
}

func TestConfirmedEconomicPacketsRejectAppendedPayload(t *testing.T) {
	opcodes := []uint16{
		wire.OpDeleteCharacter, wire.OpMessageWhisper, wire.OpUsePremiumFirework,
		wire.OpCombineTiny, wire.OpCombineLindy, wire.OpCombineCompositor,
		wire.OpCombineAgatha, wire.OpCombineAylin, wire.OpCombineEhre,
		wire.OpCombineOdin,
	}
	for _, opcode := range opcodes {
		t.Run(fmt.Sprintf("0x%X", opcode), func(t *testing.T) {
			p, _ := networkedTestPlayer(1, "Framing", 2100, 2100)
			w := worldWithNetworkedPlayers(p)
			expected, _ := exactInboundPacketSize(opcode)
			if w.validateInboundCommand(p.Session, inboundPacket(opcode, expected+8)) {
				t.Fatalf("opcode 0x%X aceitou cauda arbitraria", opcode)
			}
		})
	}
}

func TestRebuyInboundSizeAllowsClient748RequestForms(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Rebuy", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	for _, size := range []int{wire.HeaderSize, repurchasePacketSize} {
		packet := inboundPacket(wire.OpRebuy, size)
		binary.LittleEndian.PutUint16(packet[6:8], p.ID)
		if !w.validateInboundCommand(p.Session, packet) {
			t.Fatalf("recompra size=%d foi recusada", size)
		}
	}
	if w.validateInboundCommand(p.Session, inboundPacket(wire.OpRebuy, repurchasePacketSize-1)) {
		t.Fatal("recompra truncada foi aceita")
	}
}

func loadedFlatTerrain() model.TerrainMap {
	return model.TerrainMap{
		Height:    make([]byte, model.TerrainCells),
		Attribute: make([]byte, model.AttributeCells),
	}
}

func TestValidateInboundCommandRejectsCharacterLoginReplayInWorld(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Replay", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.security = nil
	packet := inboundPacket(wire.OpCharacterLogin, characterLoginPacketSize)

	if w.validateInboundCommand(p.Session, packet) {
		t.Fatal("CharacterLogin repetido durante gameplay foi aceito")
	}
	if state := w.security[p.Session]; state == nil || state.violations != 1 {
		t.Fatalf("replay nao foi contabilizado: %+v", state)
	}
}

func TestValidateInboundCommandAcceptsCharacterLogin748Layout(t *testing.T) {
	w, _, session := newEnterWorldPlayer(t, 100, 100)
	packet := inboundPacket(wire.OpCharacterLogin, characterLoginPacketSize)

	if !w.validateInboundCommand(session, packet) {
		t.Fatal("CharacterLogin de 36 bytes do client 7.48 foi recusado")
	}
}

func TestValidateInboundCommandRejectsTruncatedCharacterLogin(t *testing.T) {
	w, _, session := newEnterWorldPlayer(t, 100, 100)
	packet := inboundPacket(wire.OpCharacterLogin, 16)

	if w.validateInboundCommand(session, packet) {
		t.Fatal("CharacterLogin truncado de 16 bytes foi aceito")
	}
}

func TestValidateInboundCommandRejectsAttackWithAppendedPayload(t *testing.T) {
	p, _ := networkedTestPlayer(1, "PacketSize", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpAttackOne, 512)
	if w.validateInboundCommand(p.Session, packet) {
		t.Fatal("ataque com cauda arbitraria foi aceito")
	}
}

func TestValidateInboundCommandRejectsGuildControlPayloadSmuggling(t *testing.T) {
	for _, opcode := range []uint16{
		wire.OpGuildDeprivate, wire.OpGuildAlly, wire.OpGuildWar,
		wire.OpChallenge, wire.OpChallengeConfirm,
	} {
		t.Run(fmt.Sprintf("0x%X", opcode), func(t *testing.T) {
			p, _ := networkedTestPlayer(1, "GuildPacket", 2100, 2100)
			w := worldWithNetworkedPlayers(p)
			expected, _ := exactInboundPacketSize(opcode)
			if w.validateInboundCommand(p.Session, inboundPacket(opcode, expected+16)) {
				t.Fatalf("opcode 0x%X aceitou cauda arbitraria", opcode)
			}
		})
	}
}

func TestValidateInboundCommandAcceptsApplyBonus748Layout(t *testing.T) {
	p, _ := networkedTestPlayer(1, "ApplyBonus", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpApplyBonus, applyBonusPacketSize)

	if !w.validateInboundCommand(p.Session, packet) {
		t.Fatal("ApplyBonus de 20 bytes do client 7.48 foi recusado")
	}
}

func TestValidateInboundCommandRejectsTruncatedApplyBonus(t *testing.T) {
	p, _ := networkedTestPlayer(1, "ApplyBonusShort", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpApplyBonus, 18)

	if w.validateInboundCommand(p.Session, packet) {
		t.Fatal("ApplyBonus truncado sem o layout nativo de 20 bytes foi aceito")
	}
}

func TestMovementRejectsTeleportAndBlockedRoute(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.security = nil

	teleport := make([]byte, 52)
	binary.LittleEndian.PutUint16(teleport[12:14], p.X)
	binary.LittleEndian.PutUint16(teleport[14:16], p.Y)
	binary.LittleEndian.PutUint16(teleport[24:26], p.X+100)
	binary.LittleEndian.PutUint16(teleport[26:28], p.Y)
	w.onMove(p.Session, teleport)
	if p.X != 2100 || p.Y != 2100 {
		t.Fatalf("teleporte por 0x366 aplicado: (%d,%d)", p.X, p.Y)
	}

	w.terrain = loadedFlatTerrain()
	w.terrain.Height[int(p.Y)*model.TerrainWidth+int(p.X+1)] = model.TerrainBlockedByte
	blocked := make([]byte, 52)
	binary.LittleEndian.PutUint16(blocked[12:14], p.X)
	binary.LittleEndian.PutUint16(blocked[14:16], p.Y)
	binary.LittleEndian.PutUint16(blocked[24:26], p.X+2)
	binary.LittleEndian.PutUint16(blocked[26:28], p.Y)
	blocked[28], blocked[29] = '6', '6'
	w.onMove(p.Session, blocked)
	if p.X != 2100 || p.Y != 2100 {
		t.Fatalf("rota atravessou parede: (%d,%d)", p.X, p.Y)
	}
}

func TestMovementAdvancesOnlyAsAuthoritativeTimePasses(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock

	first := make([]byte, 52)
	binary.LittleEndian.PutUint16(first[12:14], p.X)
	binary.LittleEndian.PutUint16(first[14:16], p.Y)
	binary.LittleEndian.PutUint16(first[24:26], p.X+8)
	binary.LittleEndian.PutUint16(first[26:28], p.Y)
	w.onMove(p.Session, first)
	if p.X != 2100 || !p.MovePublished {
		t.Fatalf("destino futuro virou autoridade: x=%d published=%v", p.X, p.MovePublished)
	}

	clock.Advance(80 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2100 {
		t.Fatalf("passo venceu antes do intervalo server-side: x=%d", p.X)
	}

	clock.Advance(20 * time.Millisecond) // runspeed 4: 12 tiles/s.
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2101 {
		t.Fatalf("primeiro passo nao venceu no tempo esperado: x=%d", p.X)
	}

	clock.Advance(600 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2108 || p.MovePublished {
		t.Fatalf("rota nao terminou no destino: x=%d published=%v", p.X, p.MovePublished)
	}
}

func TestMovementCollisionUsesPlayersGameplaySpace(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	w.playerInstance = make(map[uint16]string)
	w.itemInstances = map[string]*ItemInstance{
		"private-a": {RuntimeID: "private-a", MemberIDs: []uint16{p.ID}},
	}
	w.playerInstance[p.ID] = "private-a"

	// Um mob publico na mesma coordenada fisica nao existe dentro da sala
	// privada e portanto nao pode bloquear o passo do jogador.
	public := &Mob{ID: 1000, X: p.X + 1, Y: p.Y, HP: 100,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	w.appendMobInstance(public)
	w.registerMobSpatial(public)

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], p.X+1)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	move[28] = '6'
	w.onMove(p.Session, move)
	clock.Advance(100 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2101 {
		t.Fatalf("mob publico bloqueou runtime privado: x=%d", p.X)
	}

	// Uma entidade da propria sala continua sendo colisao autoritativa.
	private := &Mob{ID: 1001, X: p.X + 1, Y: p.Y, HP: 100, InstanceID: "private-a",
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	w.appendMobInstance(private)
	w.registerMobSpatial(private)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[24:26], p.X+1)
	w.onMove(p.Session, move)
	clock.Advance(100 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2101 {
		t.Fatalf("jogador atravessou entidade do proprio runtime: x=%d", p.X)
	}
}

func TestMovementDoesNotGrantInteractionsAtFutureDestination(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.Char.Extended.Attack = 10_000
	p.Char.Extended.Accuracy = 10_000
	applyExtendedScore(p.Char)

	targetX := p.X + 12
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[24:26], targetX)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	w.onMove(p.Session, move)

	hostile := &Mob{ID: 1000, X: targetX, Y: p.Y, HP: 100,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	w.appendMobInstance(hostile)
	w.registerMobSpatial(hostile)
	p.show(hostile.ID)
	w.onAttack(p.Session, physicalAttackPacket(1000, hostile.ID, hostile.X, hostile.Y))
	if hostile.HP != 100 {
		t.Fatal("ataque usou o destino futuro como origem")
	}

	merchant := &Mob{ID: 1100, X: targetX, Y: p.Y,
		Def: &model.NPCDef{Name: "Merchant", Extended: testExtended(model.ExtendedScore{Merchant: 1})}}
	w.appendMobInstance(merchant)
	w.registerMobSpatial(merchant)
	p.show(merchant.ID)
	useNPC := make([]byte, 20)
	binary.LittleEndian.PutUint16(useNPC[12:14], merchant.ID)
	w.onUseNPC(p.Session, useNPC)
	if p.ShopNPC != 0 {
		t.Fatal("NPC no destino futuro foi aberto")
	}

	ground := &GroundItem{ID: 10001, X: targetX, Y: p.Y, Item: model.Item{Index: 412},
		Expire: clock.Now().Add(time.Minute)}
	w.items[412] = model.ItemDef{Index: 412}
	w.registerGroundItem(ground)
	get := make([]byte, 28)
	binary.LittleEndian.PutUint32(get[12:16], placeInv)
	binary.LittleEndian.PutUint16(get[20:22], ground.ID)
	w.onGetItem(p.Session, get)
	if w.groundItems[ground.ID] == nil || p.Char.Inv[0].Index != 0 {
		t.Fatal("item no destino futuro foi coletado")
	}
	if p.X != 2100 {
		t.Fatalf("alguma interacao promoveu o destino futuro: x=%d", p.X)
	}
}

func TestMovementAcceptsFull748RouteForSlowCharacter(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.Char.Extended.AttackRun = 0 // reproduz o antigo limite artificial de 8.

	packet := make([]byte, 52)
	binary.LittleEndian.PutUint16(packet[12:14], p.X)
	binary.LittleEndian.PutUint16(packet[14:16], p.Y)
	binary.LittleEndian.PutUint16(packet[24:26], p.X+11)
	binary.LittleEndian.PutUint16(packet[26:28], p.Y)
	for i := 0; i < 11; i++ {
		packet[28+i] = '6'
	}

	if !w.validPlayerMovePacket(p, packet) {
		t.Fatal("rota legitima de 11 tiles do client 7.48 foi recusada")
	}
}

func TestMovementAcceptsRepeatedRouteWhileClientInterpolates(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()

	packet := make([]byte, 52)
	binary.LittleEndian.PutUint16(packet[12:14], p.X)
	binary.LittleEndian.PutUint16(packet[14:16], p.Y)
	binary.LittleEndian.PutUint16(packet[24:26], p.X+11)
	binary.LittleEndian.PutUint16(packet[26:28], p.Y)
	for i := 0; i < 11; i++ {
		packet[28+i] = '6'
	}
	if !w.validPlayerMovePacket(p, packet) {
		t.Fatal("primeiro envio da rota foi recusado")
	}

	// onMove ja teria registrado o destino, mas o client ainda envia PosX na
	// origem durante a interpolacao do mesmo segmento.
	p.X += 11
	if !w.validPlayerMovePacket(p, packet) {
		t.Fatal("repeticao legitima da rota em animacao foi recusada")
	}
}

func TestStopPacketsCannotRepositionPlayer(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], p.X+20)
	binary.LittleEndian.PutUint16(stop[14:16], p.Y)
	w.onActionStop(p.Session, stop)

	moveStop := make([]byte, 36)
	binary.LittleEndian.PutUint32(moveStop[20:24], uint32(p.X+20))
	binary.LittleEndian.PutUint32(moveStop[24:28], uint32(p.Y))
	w.onMoveStop(p.Session, moveStop)
	if p.X != 2100 || p.Y != 2100 {
		t.Fatalf("Stop reposicionou player: (%d,%d)", p.X, p.Y)
	}
}

func TestStopCannotClaimFutureValidatedRoutePosition(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.MovePublished = true
	p.MovePublishedStartX, p.MovePublishedStartY = 2100, 2100
	p.MovePublishedTargetX, p.MovePublishedTargetY = 2110, 2100
	copy(p.MovePublishedRoute[:], []byte("6666666666"))
	if w.validReportedStop(p, 2105, 2100) {
		t.Fatal("Stop promoveu uma coordenada futura apenas por pertencer a Route[24]")
	}
	if !w.validReportedStop(p, 2100, 2100) {
		t.Fatal("Stop na coordenada autoritativa atual foi recusado")
	}
}

func TestPhysicalAttackCannotCrossBlockedTerrain(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	w.terrain.Height[int(p.Y)*model.TerrainWidth+int(p.X+1)] = model.TerrainBlockedByte
	p.Char.Extended.Attack = 10_000
	p.Char.Extended.Accuracy = 10_000
	applyExtendedScore(p.Char)
	mob := &Mob{
		ID: 1000, X: p.X + 2, Y: p.Y, HP: 10_000,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 10_000, CurHP: 10_000}),
	}
	w.registerMobSpatial(mob)
	p.show(mob.ID)

	w.onAttack(p.Session, physicalAttackPacket(1000, mob.ID, mob.X, mob.Y))
	if mob.HP != 10_000 || mob.TargetID != 0 {
		t.Fatalf("ataque atravessou parede: hp=%d target=%d", mob.HP, mob.TargetID)
	}
}

func TestGroundItemTransactionsRollbackOnSaveFailure(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	w.items[412] = model.ItemDef{Index: 412}
	st.err = errors.New("disk")
	p.Char.Inv[0] = model.Item{Index: 412}

	drop := make([]byte, 32)
	binary.LittleEndian.PutUint32(drop[12:16], placeInv)
	binary.LittleEndian.PutUint32(drop[16:20], 0)
	w.onDropItem(p.Session, drop)
	if p.Char.Inv[0].Index != 412 || len(w.groundItems) != 0 {
		t.Fatalf("drop sem save nao fez rollback: inv=%d ground=%d", p.Char.Inv[0].Index, len(w.groundItems))
	}

	p.Char.Inv[0] = model.Item{}
	g := &GroundItem{ID: 10001, Item: model.Item{Index: 412}, X: p.X, Y: p.Y}
	w.groundItems[g.ID] = g
	get := make([]byte, 28)
	binary.LittleEndian.PutUint32(get[12:16], placeInv)
	binary.LittleEndian.PutUint32(get[16:20], 0)
	binary.LittleEndian.PutUint16(get[20:22], g.ID)
	w.onGetItem(p.Session, get)
	if p.Char.Inv[0].Index != 0 || w.groundItems[g.ID] != g {
		t.Fatal("pickup sem save nao restaurou inventario/objeto do chao")
	}
}

func TestShopOperationsRevalidateRangeAndRejectEquipmentSale(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.items[400] = model.ItemDef{Index: 400, Price: 1000}
	shop := &Mob{ID: 1100, X: p.X + 1, Y: p.Y, Def: &model.NPCDef{
		Name: "Merchant", Tipo: model.TipoNPC,
		Extended: &model.ExtendedScore{Merchant: nativeShopMerchant},
		Vende:    []model.Item{{Index: 400}},
	}}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.ShopNPC = shop.ID
	p.Char.Gold = 5000
	p.X += 20

	buy := make([]byte, 24)
	w.onBuyItem(p.Session, buy)
	if p.Char.Inv[0].Index != 0 || p.Char.Gold != 5000 || p.ShopNPC != 0 {
		t.Fatal("compra remota em loja stale foi aceita")
	}

	p.X = shop.X
	p.ShopNPC = shop.ID
	p.Char.Equip[0] = model.Item{Index: 400}
	sell := make([]byte, 20)
	sell[14], sell[16] = placeEquip, 0
	w.onSellItem(p.Session, sell)
	if p.Char.Equip[0].Index != 400 || p.Char.Gold != 5000 {
		t.Fatal("venda forjada do slot de rosto foi aceita")
	}
}

func TestSkillDelayUsesSecondsFromCatalog(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Char.Class = 1
	p.Char.LearnedSkill = 1 << 3
	p.Char.Extended.Mastery[1] = 40
	applyExtendedScore(p.Char)
	skill := model.SkillDef{
		Index: 27, Name: "Heal", InstanceValue: 10, ManaSpent: 1,
		Delay: 4, Range: 6, MaxTarget: 1,
	}
	w.skills = map[int]model.SkillDef{27: skill}

	before := time.Now()
	w.onSkillAttack(p, skillCastRequest{Skill: 27})
	remaining := p.SkillReady[27].Sub(before)
	if remaining < 4*time.Second || remaining >= 5*time.Second {
		t.Fatalf("Delay=4 gerou cooldown %s, esperado 4s", remaining)
	}
}
