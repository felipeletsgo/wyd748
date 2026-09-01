package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/data"
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
		wire.OpConnectAccount:          116,
		wire.OpCreateCharacter:         36,
		wire.OpDeleteCharacter:         44,
		wire.OpCharacterLogin:          characterLoginPacketSize,
		wire.OpCharacterLogout:         12,
		wire.OpClientIntegrityResponse: wire.ClientIntegrityPacketSize,
		wire.OpSwapItem:                20,
		wire.OpDeposit:                 16,
		wire.OpWithdraw:                16,
		wire.OpUseItem:                 36,
		wire.OpUsePremiumFirework:      premiumFireworkUsePacketSize,
		wire.OpUseNPC:                  20,
		wire.OpReqShopList:             16,
		wire.OpBuyItem:                 24,
		wire.OpBuyToto:                 36,
		wire.OpDoJackpotBet:            20,
		wire.OpSellItem:                20,
		wire.OpApplyBonus:              applyBonusPacketSize,
		wire.OpPartyRequest:            44,
		wire.OpPartyAccept:             32,
		wire.OpPartyRemove:             16,
		wire.OpTrade:                   156,
		wire.OpCloseTrade:              12,
		wire.OpAutoTrade:               196,
		wire.OpReqTradeList:            16,
		wire.OpReqBuyAutoTrade:         36,
		wire.OpDropItem:                32,
		wire.OpGetItem:                 28,
		wire.OpDeleteItem:              deleteItemPacketSize,
		wire.OpSplitItem:               splitItemPacketSize,
		wire.OpUpdateItem:              20,
		wire.OpSetShortSkill:           32,
		wire.OpMessageChat:             108,
		wire.OpMessageWhisper:          128,
		wire.OpChangeCity:              16,
		wire.OpReqTeleport:             16,
		wire.OpPKMode:                  16,
		wire.OpMoveStop:                36,
		wire.OpUpdateScore:             wire.HeaderSize,
		wire.OpRestart:                 12,
		wire.OpPing:                    12,
		wire.OpSysQuit:                 16,
		wire.OpAction:                  52,
		wire.OpActionStop:              52,
		wire.OpIllusion:                52,
		wire.OpREQMobByID:              16,
		wire.OpGuildDeprivate:          16,
		wire.OpInviteGuild:             20,
		wire.OpGuildAlly:               20,
		wire.OpGuildWar:                20,
		wire.OpChallenge:               16,
		wire.OpChallengeConfirm:        20,
		wire.OpMotion:                  20,
		wire.OpClientUnknown2BC:        108,
		wire.OpReqRanking:              20,
		wire.OpAttackOne:               48,
		wire.OpAttackTwo:               52,
		wire.OpAttackMulti:             96,
		wire.OpCombineTiny:             combinePacketSize,
		wire.OpCombineLindy:            combinePacketSize,
		wire.OpCombineCompositor:       combinePacketSize,
		wire.OpCombineAgatha:           combinePacketSize,
		wire.OpCombineAylin:            combinePacketSize,
		wire.OpCombineEhre:             combinePacketSize,
		wire.OpCombineOdin:             combinePacketSize,
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

	clock.Advance(249 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2100 {
		t.Fatalf("passo venceu antes do intervalo server-side: x=%d", p.X)
	}

	clock.Advance(time.Millisecond) // RunSpeed 4: um passo a cada 250 ms.
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2101 {
		t.Fatalf("primeiro passo nao venceu no tempo esperado: x=%d", p.X)
	}

	clock.Advance(1750 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2108 || p.MovePublished {
		t.Fatalf("rota nao terminou no destino: x=%d published=%v", p.X, p.MovePublished)
	}
}

func TestMovementUsesNative748MaximumSpeedSeven(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.Char.Score.AttackRun = 7
	applyScore(p.Char)

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint32(move[16:20], 7)
	binary.LittleEndian.PutUint16(move[24:26], p.X+7)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	copy(move[28:], []byte("6666666"))
	w.onMove(p.Session, move)

	clock.Advance(time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2107 || p.Y != 2100 || p.MovePublished {
		t.Fatalf("RunSpeed 7 nao avancou sete tiles em um segundo: (%d,%d) moving=%v",
			p.X, p.Y, p.MovePublished)
	}
}

func TestMovementUsesCaptured748RouteDirections(t *testing.T) {
	tests := []struct {
		encoded byte
		dx      int
		dy      int
	}{
		{encoded: '1', dx: -1, dy: -1},
		{encoded: '2', dx: 0, dy: -1},
		{encoded: '3', dx: 1, dy: -1},
		{encoded: '4', dx: -1, dy: 0},
		{encoded: '6', dx: 1, dy: 0},
		{encoded: '7', dx: -1, dy: 1},
		{encoded: '8', dx: 0, dy: 1},
		{encoded: '9', dx: 1, dy: 1},
	}

	for _, test := range tests {
		t.Run(string(test.encoded), func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			w.terrain = loadedFlatTerrain()
			clock := newFakeClock(time.Unix(100, 0))
			w.clock = clock
			originX, originY := p.X, p.Y
			targetX := uint16(int(originX) + test.dx)
			targetY := uint16(int(originY) + test.dy)

			move := make([]byte, 52)
			binary.LittleEndian.PutUint16(move[12:14], originX)
			binary.LittleEndian.PutUint16(move[14:16], originY)
			binary.LittleEndian.PutUint16(move[24:26], targetX)
			binary.LittleEndian.PutUint16(move[26:28], targetY)
			move[28] = test.encoded
			w.onMove(p.Session, move)
			if !p.MovePublished {
				t.Fatalf("direcao nativa %q foi recusada", test.encoded)
			}

			clock.Advance(time.Second)
			w.advancePlayerMovement(p, clock.Now())
			if p.X != targetX || p.Y != targetY {
				t.Fatalf("direcao %q terminou em (%d,%d), esperado (%d,%d)",
					test.encoded, p.X, p.Y, targetX, targetY)
			}
			encoded := directMovementRoute(originX, originY, targetX, targetY)
			if len(encoded) != 1 || encoded[0] != test.encoded {
				t.Fatalf("encoder gerou %q para delta (%d,%d), esperado %q",
					encoded, test.dx, test.dy, test.encoded)
			}
		})
	}
}

func TestMovementContinuousReplansPreserveAuthoritativeCadence(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock

	originX := p.X
	fromX := originX
	for targetX := originX + 4; targetX <= originX+20; targetX += 4 {
		move := make([]byte, 52)
		binary.LittleEndian.PutUint16(move[12:14], fromX)
		binary.LittleEndian.PutUint16(move[14:16], p.Y)
		binary.LittleEndian.PutUint16(move[24:26], targetX)
		binary.LittleEndian.PutUint16(move[26:28], p.Y)
		for i := fromX; i < targetX; i++ {
			move[28+int(i-fromX)] = '6'
		}
		w.advancePlayerMovement(p, clock.Now())
		w.onMove(p.Session, move)
		fromX = targetX
		clock.Advance(50 * time.Millisecond)
	}

	// RunSpeed 4 equivale a 4 tiles/s. Atualizar o destino a cada 50 ms nao
	// pode reiniciar indefinidamente o relogio do primeiro passo.
	w.advancePlayerMovement(p, clock.Now())
	if p.X < 2101 {
		t.Fatalf("replanejamento continuo congelou autoridade em x=%d", p.X)
	}
}

func TestMovementBridgesSkippedClientSegmentWithoutGrantingFutureAuthority(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock

	first := make([]byte, 52)
	binary.LittleEndian.PutUint16(first[12:14], p.X)
	binary.LittleEndian.PutUint16(first[14:16], p.Y)
	binary.LittleEndian.PutUint16(first[24:26], p.X+7)
	binary.LittleEndian.PutUint16(first[26:28], p.Y)
	copy(first[28:], []byte("6666666"))
	w.onMove(p.Session, first)

	clock.Advance(50 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())

	// O 7.48 envia planos continuamente. Se um segmento intermediario nao
	// chegou, o proximo PosX pode estar poucos tiles alem do destino publicado,
	// mas o servidor ainda deve percorrer todo o corredor no seu proprio tempo.
	second := make([]byte, 52)
	binary.LittleEndian.PutUint16(second[12:14], p.X+10)
	binary.LittleEndian.PutUint16(second[14:16], p.Y)
	binary.LittleEndian.PutUint16(second[24:26], p.X+18)
	binary.LittleEndian.PutUint16(second[26:28], p.Y)
	copy(second[28:], []byte("66666666"))
	w.onMove(p.Session, second)
	if p.X != 2100 {
		t.Fatalf("origem futura virou autoridade imediatamente: x=%d", p.X)
	}
	if !p.MovePublished || p.MovePublishedTargetX != 2118 {
		t.Fatalf("segmento continuo nao foi reconciliado: published=%v target=%d",
			p.MovePublished, p.MovePublishedTargetX)
	}

	clock.Advance(5 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2118 || p.Y != 2100 {
		t.Fatalf("corredor reconciliado nao chegou ao destino: (%d,%d)", p.X, p.Y)
	}
}

func TestMovementReconcilesCurvedVisualLeadWithoutLineOfSight(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.X, p.Y = 1116, 1713
	p.Char.X, p.Char.Y = p.X, p.Y

	// A reta autoridade->Pos cruza uma célula bloqueada, mas existe uma curva
	// curta válida. Este é o formato do primeiro pacote recusado no teste real:
	// auth=(1116,1713), Pos=(1114,1710), Target=(1116,1707), Route="233".
	w.terrain.Height[1712*model.TerrainWidth+1115] = model.TerrainBlockedByte
	if w.terrain.LineOfSight(1116, 1713, 1114, 1710) {
		t.Fatal("fixture deveria bloquear apenas a ponte em linha reta")
	}
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], 1114)
	binary.LittleEndian.PutUint16(move[14:16], 1710)
	binary.LittleEndian.PutUint32(move[16:20], 6)
	binary.LittleEndian.PutUint16(move[24:26], 1116)
	binary.LittleEndian.PutUint16(move[26:28], 1707)
	copy(move[28:], []byte("233"))
	w.onMove(p.Session, move)
	if !p.MovePublished || p.X != 1116 || p.Y != 1713 {
		t.Fatalf("ponte curva foi recusada ou aplicada como salto: moving=%v pos=(%d,%d)",
			p.MovePublished, p.X, p.Y)
	}
	clock.Advance(2 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 1116 || p.Y != 1707 || p.MovePublished {
		t.Fatalf("ponte curva nao terminou no destino: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
	}
}

func TestMovementAcceptsCapturedNoatumReplanOnAuthoritativeTerrain(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	terrain, err := data.LoadTerrain("../../data/maps/HeightMap.dat", "../../data/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	w.terrain = terrain
	p.X, p.Y = 1116, 1713
	p.Char.X, p.Char.Y = p.X, p.Y
	p.Char.Score.AttackRun = 6
	applyScore(p.Char)

	// Primeiro pacote da captura que iniciou a cascata de recusas em Noatum.
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], 1114)
	binary.LittleEndian.PutUint16(move[14:16], 1710)
	binary.LittleEndian.PutUint32(move[16:20], 6)
	binary.LittleEndian.PutUint16(move[24:26], 1116)
	binary.LittleEndian.PutUint16(move[26:28], 1707)
	copy(move[28:], []byte("233"))
	w.onMove(p.Session, move)
	if !p.MovePublished || p.MovePublishedTargetX != 1116 || p.MovePublishedTargetY != 1707 {
		t.Fatalf("replanejamento real de Noatum foi recusado: moving=%v target=(%d,%d)",
			p.MovePublished, p.MovePublishedTargetX, p.MovePublishedTargetY)
	}
	if p.MovePublishedStartX != 1116 || p.MovePublishedStartY != 1713 {
		t.Fatalf("observer recebeu origem visual adiantada em vez da autoridade: (%d,%d)",
			p.MovePublishedStartX, p.MovePublishedStartY)
	}
	x, y := int(p.MovePublishedStartX), int(p.MovePublishedStartY)
	for _, encoded := range p.MovePublishedRoute {
		if encoded == 0 {
			break
		}
		direction := routeDirections[encoded]
		x, y = x+direction[0], y+direction[1]
	}
	if x != 1116 || y != 1707 {
		t.Fatalf("rota publicada ao observer terminou em (%d,%d)", x, y)
	}
}

func TestMovementVisualCatchupUsesMaximumSpeedWithoutAcceleratingFutureRoute(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.Char.Score.AttackRun = 1
	applyScore(p.Char)

	// Sete passos até o Pos já percorrido visualmente + um passo futuro.
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X+7)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], p.X+8)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	move[28] = '6'
	w.onMove(p.Session, move)

	clock.Advance(time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2107 || p.Y != 2100 {
		t.Fatalf("catch-up nao usou velocidade maxima: (%d,%d)", p.X, p.Y)
	}
	clock.Advance(999 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2107 {
		t.Fatalf("trecho futuro foi acelerado junto com catch-up: x=%d", p.X)
	}
	clock.Advance(time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2108 || p.MovePublished {
		t.Fatalf("trecho futuro nao respeitou RunSpeed 1: x=%d moving=%v", p.X, p.MovePublished)
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
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w.appendMobInstance(public)
	w.registerMobSpatial(public)

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], p.X+1)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	move[28] = '6'
	w.onMove(p.Session, move)
	clock.Advance(250 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2101 {
		t.Fatalf("mob publico bloqueou runtime privado: x=%d", p.X)
	}

	// Uma entidade da propria sala continua sendo colisao autoritativa.
	private := &Mob{ID: 1001, X: p.X + 1, Y: p.Y, HP: 100, InstanceID: "private-a",
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w.appendMobInstance(private)
	w.registerMobSpatial(private)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[24:26], p.X+1)
	w.onMove(p.Session, move)
	clock.Advance(250 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2101 {
		t.Fatalf("jogador atravessou entidade do proprio runtime: x=%d", p.X)
	}
}

func TestMovementCrossesOccupiedIntermediateTileButNotOccupiedDestination(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock

	// O NPC está no meio da rota, como as lojas de teste em Armia. O client
	// nativo permite atravessá-lo porque o destino final continua livre.
	npc := &Mob{ID: 1000, X: p.X + 2, Y: p.Y, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100, Merchant: 1})}
	w.appendMobInstance(npc)
	w.registerMobSpatial(npc)
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], p.X+4)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	copy(move[28:], []byte("6666"))
	w.onMove(p.Session, move)
	clock.Advance(time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2104 || p.Y != 2100 || p.MovePublished {
		t.Fatalf("NPC intermediario interrompeu a rota: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
	}

	// A coordenada final ocupada continua bloqueada server-side.
	w.unregisterMobSpatial(npc)
	npc.X = p.X + 2
	w.registerMobSpatial(npc)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[24:26], p.X+2)
	move[28], move[29], move[30] = '6', '6', 0
	w.onMove(p.Session, move)
	clock.Advance(500 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2105 || p.Y != 2100 || p.MovePublished {
		t.Fatalf("destino ocupado nao foi bloqueado: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
	}
}

func TestMovementDoesNotGrantInteractionsAtFutureDestination(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.Char.Score.Attack = 10_000
	p.Char.Score.Accuracy = 10_000
	applyScore(p.Char)

	targetX := p.X + 12
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[24:26], targetX)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	w.onMove(p.Session, move)

	hostile := &Mob{ID: 1000, X: targetX, Y: p.Y, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w.appendMobInstance(hostile)
	w.registerMobSpatial(hostile)
	p.show(hostile.ID)
	w.onAttack(p.Session, physicalAttackPacket(1000, hostile.ID, hostile.X, hostile.Y))
	if hostile.HP != 100 {
		t.Fatal("ataque usou o destino futuro como origem")
	}

	merchant := &Mob{ID: 1100, X: targetX, Y: p.Y,
		Def: &model.NPCDef{Name: "Merchant", Score: testScore(model.Score{Merchant: 1})}}
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
	p.Char.Score.AttackRun = 0 // reproduz o antigo limite artificial de 8.

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
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.MovePublished = true
	p.MovePublishedStartX, p.MovePublishedStartY = 2100, 2100
	p.MovePublishedTargetX, p.MovePublishedTargetY = 2110, 2100
	copy(p.MovePublishedRoute[:], []byte("6666666666"))
	p.MoveAuthorityRoute = []byte("6666666666")
	p.MoveAuthorityX, p.MoveAuthorityY = p.X, p.Y
	p.MoveAuthorityStepInterval = movementStepInterval(p)
	p.MoveAuthorityStartedAt = clock.Now()

	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], 2100)
	binary.LittleEndian.PutUint16(stop[14:16], 2100)
	binary.LittleEndian.PutUint16(stop[24:26], 2105)
	binary.LittleEndian.PutUint16(stop[26:28], 2100)
	w.onActionStop(p.Session, stop)
	if p.X != 2100 || p.Y != 2100 {
		t.Fatalf("Stop promoveu destino futuro imediatamente: (%d,%d)", p.X, p.Y)
	}
	clock.Advance(1250 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2105 || p.Y != 2100 {
		t.Fatalf("Stop nao chegou pelo relogio autoritativo: (%d,%d)", p.X, p.Y)
	}
}

func TestActionStopAcceptsNative748FiveTileFinalPlan(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.X, p.Y = 2485, 2016
	p.Char.X, p.Char.Y = p.X, p.Y

	// Captura real 7.48: Pos=(2485,2016), Speed=3,
	// Target=(2480,2015), Route[24] zerada.
	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], 2485)
	binary.LittleEndian.PutUint16(stop[14:16], 2016)
	binary.LittleEndian.PutUint32(stop[16:20], 3)
	binary.LittleEndian.PutUint16(stop[24:26], 2480)
	binary.LittleEndian.PutUint16(stop[26:28], 2015)
	w.onActionStop(p.Session, stop)
	if p.X != 2485 || p.Y != 2016 {
		t.Fatalf("ActionStop concedeu destino antecipado: (%d,%d)", p.X, p.Y)
	}
	if !p.MovePublished || p.MovePublishedTargetX != 2480 || p.MovePublishedTargetY != 2015 {
		t.Fatal("ActionStop nativo nao criou o plano final autoritativo")
	}
	clock.Advance(1250 * time.Millisecond) // RunSpeed 4: cinco passos.
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2480 || p.Y != 2015 || p.MovePublished {
		t.Fatalf("ActionStop nao terminou no alvo: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
	}
}

func TestActionStopRejectsRouteLongerThanNativeLimit(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], p.X)
	binary.LittleEndian.PutUint16(stop[14:16], p.Y)
	binary.LittleEndian.PutUint16(stop[24:26], p.X+7)
	binary.LittleEndian.PutUint16(stop[26:28], p.Y)
	w.onActionStop(p.Session, stop)
	if p.X != 2100 || p.Y != 2100 || p.MovePublished {
		t.Fatalf("ActionStop invalido alterou movimento: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
	}
}

func TestPhysicalAttackCannotCrossBlockedTerrain(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	w.terrain.Height[int(p.Y)*model.TerrainWidth+int(p.X+1)] = model.TerrainBlockedByte
	p.Char.Score.Attack = 10_000
	p.Char.Score.Accuracy = 10_000
	applyScore(p.Char)
	mob := &Mob{
		ID: 1000, X: p.X + 2, Y: p.Y, HP: 10_000,
		Def: testNPCDef(model.Score{MaxHP: 10_000, CurHP: 10_000}),
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
		Score: &model.Score{Merchant: nativeShopMerchant},
		Vende: []model.Item{{Index: 400}},
	}}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.ShopNPC = shop.ID
	p.Char.Gold = 5000
	p.X += 20

	buy := make([]byte, 24)
	// Keep the forged request bound to the opened merchant so this assertion
	// specifically exercises the server-side distance revalidation.
	binary.LittleEndian.PutUint16(buy[12:14], shop.ID)
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
	p.Char.Score.Mastery[1] = 40
	applyScore(p.Char)
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
