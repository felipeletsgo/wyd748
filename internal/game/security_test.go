package game

import (
	"encoding/binary"
	"errors"
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
		wire.OpConnectAccount:   116,
		wire.OpCreateCharacter:  36,
		wire.OpCharacterLogin:   characterLoginPacketSize,
		wire.OpCharacterLogout:  12,
		wire.OpSwapItem:         20,
		wire.OpDeposit:          16,
		wire.OpWithdraw:         16,
		wire.OpUseItem:          36,
		wire.OpUseNPC:           20,
		wire.OpReqShopList:      16,
		wire.OpBuyItem:          24,
		wire.OpSellItem:         20,
		wire.OpApplyBonus:       applyBonusPacketSize,
		wire.OpPartyRequest:     44,
		wire.OpPartyAccept:      32,
		wire.OpPartyRemove:      16,
		wire.OpTrade:            156,
		wire.OpCloseTrade:       12,
		wire.OpAutoTrade:        196,
		wire.OpReqTradeList:     16,
		wire.OpReqBuyAutoTrade:  36,
		wire.OpDropItem:         32,
		wire.OpGetItem:          28,
		wire.OpDeleteItem:       deleteItemPacketSize,
		wire.OpSplitItem:        splitItemPacketSize,
		wire.OpUpdateItem:       20,
		wire.OpSetShortSkill:    32,
		wire.OpChangeCity:       16,
		wire.OpReqTeleport:      16,
		wire.OpPKMode:           16,
		wire.OpMoveStop:         36,
		wire.OpRestart:          12,
		wire.OpPing:             12,
		wire.OpSysQuit:          16,
		wire.OpAction:           52,
		wire.OpActionStop:       52,
		wire.OpIllusion:         52,
		wire.OpREQMobByID:       16,
		wire.OpMotion:           20,
		wire.OpClientUnknown2BC: 108,
		wire.OpReqRanking:       20,
		wire.OpAttackOne:        48,
		wire.OpAttackTwo:        52,
		wire.OpAttackMulti:      96,
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

func TestMovementBudgetRejectsRapidValidSegments(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock

	first := make([]byte, 52)
	binary.LittleEndian.PutUint16(first[12:14], p.X)
	binary.LittleEndian.PutUint16(first[14:16], p.Y)
	binary.LittleEndian.PutUint16(first[24:26], p.X+8)
	binary.LittleEndian.PutUint16(first[26:28], p.Y)
	w.onMove(p.Session, first)
	if p.X != 2108 {
		t.Fatalf("primeiro segmento legitimo recusado: x=%d", p.X)
	}

	second := make([]byte, 52)
	binary.LittleEndian.PutUint16(second[12:14], p.X)
	binary.LittleEndian.PutUint16(second[14:16], p.Y)
	binary.LittleEndian.PutUint16(second[24:26], p.X+1)
	binary.LittleEndian.PutUint16(second[26:28], p.Y)
	w.onMove(p.Session, second)
	if p.X != 2108 {
		t.Fatalf("segmento sem tempo disponivel foi aceito: x=%d", p.X)
	}

	clock.Advance(100 * time.Millisecond) // runspeed 4: repoe 1,2 tile.
	w.onMove(p.Session, second)
	if p.X != 2109 {
		t.Fatalf("orcamento nao recuperou com o tempo: x=%d", p.X)
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
