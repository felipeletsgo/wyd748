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
		wire.OpCharacterTransfer:       52,
		wire.OpDeclareServerWar:        serverWarLetterPacketSize,
		wire.OpRefuseServerWar:         serverWarLetterPacketSize,
		wire.OpBuyLotteryTicket:        lotteryTicketPacketSize,
		wire.OpHellStoreBuy:            hellStorePacketSize,
		wire.OpStoreGoldRequest:        storeGoldRequestPacketSize,
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
		wire.OpAirMove:                 20,
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
		wire.OpPlayerChallenge:         20,
		wire.OpCombineExtracao:         20,
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
		wire.OpCombineAlquimia:         combinePacketSize,
	}
	for opcode, expected := range tests {
		got, exact := exactInboundPacketSize(opcode)
		if !exact || got != expected {
			t.Errorf("opcode 0x%X: size=%d exact=%v, want=%d",
				opcode, got, exact, expected)
		}
	}
	if size, exact := exactInboundPacketSize(0xFFFF); exact || size != 0 {
		t.Fatalf("an unknown opcode got framing: size=%d exact=%v", size, exact)
	}
}

func TestConfirmedEconomicPacketsRejectAppendedPayload(t *testing.T) {
	opcodes := []uint16{
		wire.OpDeleteCharacter, wire.OpMessageWhisper, wire.OpUsePremiumFirework,
		wire.OpCombineTiny, wire.OpCombineLindy, wire.OpCombineCompositor,
		wire.OpCombineAgatha, wire.OpCombineAylin, wire.OpCombineEhre,
		wire.OpCombineOdin, wire.OpCombineExtracao, wire.OpCombineAlquimia,
	}
	for _, opcode := range opcodes {
		t.Run(fmt.Sprintf("0x%X", opcode), func(t *testing.T) {
			p, _ := networkedTestPlayer(1, "Framing", 2100, 2100)
			w := worldWithNetworkedPlayers(p)
			expected, _ := exactInboundPacketSize(opcode)
			if w.validateInboundCommand(p.Session, inboundPacket(opcode, expected+8)) {
				t.Fatalf("opcode 0x%X accepted an arbitrary tail", opcode)
			}
		})
	}
}

func TestRemovedRebuyOpcodeIsRejected(t *testing.T) {
	const removedRebuyOpcode = 0x3E8
	if knownInboundOpcode(removedRebuyOpcode) {
		t.Fatal("the removed buy-back opcode is still in the C->S allowlist")
	}
	p, _ := networkedTestPlayer(1, "NoRebuy", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	if w.validateInboundCommand(p.Session, inboundPacket(removedRebuyOpcode, wire.HeaderSize)) {
		t.Fatal("the removed buy-back opcode was accepted by the server")
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
		t.Fatal("a repeated CharacterLogin during gameplay was accepted")
	}
	if state := w.security[p.Session]; state == nil || state.violations != 1 {
		t.Fatalf("the replay was not counted: %+v", state)
	}
}

func TestValidateInboundCommandAcceptsCharacterLogin748Layout(t *testing.T) {
	w, _, session := newEnterWorldPlayer(t, 100, 100)
	packet := inboundPacket(wire.OpCharacterLogin, characterLoginPacketSize)

	if !w.validateInboundCommand(session, packet) {
		t.Fatal("the 36-byte CharacterLogin of the 7.48 client was refused")
	}
}

func TestValidateInboundCommandRejectsTruncatedCharacterLogin(t *testing.T) {
	w, _, session := newEnterWorldPlayer(t, 100, 100)
	packet := inboundPacket(wire.OpCharacterLogin, 16)

	if w.validateInboundCommand(session, packet) {
		t.Fatal("a truncated 16-byte CharacterLogin was accepted")
	}
}

func TestValidateInboundCommandRejectsAttackWithAppendedPayload(t *testing.T) {
	p, _ := networkedTestPlayer(1, "PacketSize", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpAttackOne, 512)
	if w.validateInboundCommand(p.Session, packet) {
		t.Fatal("an attack with an arbitrary tail was accepted")
	}
}

func TestCCAttackFramingPreservesConfirmedLayouts(t *testing.T) {
	for _, tc := range []struct {
		name   string
		opcode uint16
		size   int
		accept bool
	}{
		{"ported-multi-with-stop-opcode", wire.OpActionStop, 96, false},
		{"legacy-single-extended-still-accepted", wire.OpAttackOne, 96, true},
		{"single", wire.OpAttackOne, 48, true},
		{"double", wire.OpAttackTwo, 52, true},
		{"multi", wire.OpAttackMulti, 96, true},
		{"movement-stop-unchanged", wire.OpActionStop, 52, true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			p, _ := networkedTestPlayer(1, "CCFraming", 2100, 2100)
			w := worldWithNetworkedPlayers(p)
			got := w.validateInboundCommand(p.Session, inboundPacket(tc.opcode, tc.size))
			if got != tc.accept {
				t.Fatalf("opcode 0x%X size %d: accepted=%v, want %v", tc.opcode, tc.size, got, tc.accept)
			}
		})
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
				t.Fatalf("opcode 0x%X accepted an arbitrary tail", opcode)
			}
		})
	}
}

func TestValidateInboundCommandAcceptsApplyBonus748Layout(t *testing.T) {
	p, _ := networkedTestPlayer(1, "ApplyBonus", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpApplyBonus, applyBonusPacketSize)

	if !w.validateInboundCommand(p.Session, packet) {
		t.Fatal("the 20-byte ApplyBonus of the 7.48 client was refused")
	}
}

func TestValidateInboundCommandRejectsTruncatedApplyBonus(t *testing.T) {
	p, _ := networkedTestPlayer(1, "ApplyBonusShort", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpApplyBonus, 18)

	if w.validateInboundCommand(p.Session, packet) {
		t.Fatal("a truncated ApplyBonus without the native 20-byte layout was accepted")
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
		t.Fatalf("teleport through 0x366 applied: (%d,%d)", p.X, p.Y)
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
		t.Fatalf("the route crossed a wall: (%d,%d)", p.X, p.Y)
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
		t.Fatalf("a future destination became authority: x=%d published=%v", p.X, p.MovePublished)
	}

	clock.Advance(249 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2100 {
		t.Fatalf("a step completed before the server-side interval: x=%d", p.X)
	}

	clock.Advance(time.Millisecond) // RunSpeed 4: one step every 250 ms.
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2101 {
		t.Fatalf("the first step did not complete in the expected time: x=%d", p.X)
	}

	clock.Advance(1750 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2108 || p.MovePublished {
		t.Fatalf("the route did not end at the destination: x=%d published=%v", p.X, p.MovePublished)
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
		t.Fatalf("RunSpeed 7 did not advance seven tiles in one second: (%d,%d) moving=%v",
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
				t.Fatalf("native direction %q was refused", test.encoded)
			}

			clock.Advance(time.Second)
			w.advancePlayerMovement(p, clock.Now())
			if p.X != targetX || p.Y != targetY {
				t.Fatalf("direction %q ended at (%d,%d), want (%d,%d)",
					test.encoded, p.X, p.Y, targetX, targetY)
			}
			encoded := directMovementRoute(originX, originY, targetX, targetY)
			if len(encoded) != 1 || encoded[0] != test.encoded {
				t.Fatalf("the encoder produced %q for delta (%d,%d), want %q",
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

	// RunSpeed 4 is 4 tiles/s. Updating the destination every 50 ms must not
	// restart the clock of the first step indefinitely.
	w.advancePlayerMovement(p, clock.Now())
	if p.X < 2101 {
		t.Fatalf("continuous replanning froze authority at x=%d", p.X)
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

	// The 7.48 client sends plans continuously. If an intermediate segment did
	// not arrive, the next PosX may be a few tiles past the published destination,
	// but the server must still walk the whole corridor on its own time.
	second := make([]byte, 52)
	binary.LittleEndian.PutUint16(second[12:14], p.X+10)
	binary.LittleEndian.PutUint16(second[14:16], p.Y)
	binary.LittleEndian.PutUint16(second[24:26], p.X+18)
	binary.LittleEndian.PutUint16(second[26:28], p.Y)
	copy(second[28:], []byte("66666666"))
	w.onMove(p.Session, second)
	if p.X != 2100 {
		t.Fatalf("a future origin became authority at once: x=%d", p.X)
	}
	if !p.MovePublished || p.MovePublishedTargetX != 2118 {
		t.Fatalf("a continuous segment was not reconciled: published=%v target=%d",
			p.MovePublished, p.MovePublishedTargetX)
	}

	clock.Advance(5 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2118 || p.Y != 2100 {
		t.Fatalf("the reconciled corridor did not reach the destination: (%d,%d)", p.X, p.Y)
	}
}

func TestMovementReconcilesCurvedVisualLeadWithoutLineOfSight(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.X, p.Y = 1116, 1713
	p.Char.X, p.Char.Y = p.X, p.Y

	// The straight line authority->Pos crosses a blocked cell, but a short valid
	// curve exists. This is the shape of the first packet refused in the real test:
	// auth=(1116,1713), Pos=(1114,1710), Target=(1116,1707), Route="233".
	w.terrain.Height[1712*model.TerrainWidth+1115] = model.TerrainBlockedByte
	if w.terrain.LineOfSight(1116, 1713, 1114, 1710) {
		t.Fatal("the fixture should block only the straight bridge")
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
		t.Fatalf("the curved bridge was refused or applied as a jump: moving=%v pos=(%d,%d)",
			p.MovePublished, p.X, p.Y)
	}
	clock.Advance(2 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 1116 || p.Y != 1707 || p.MovePublished {
		t.Fatalf("the curved bridge did not end at the destination: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
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

	// First packet of the capture that started the cascade of refusals in Noatum.
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], 1114)
	binary.LittleEndian.PutUint16(move[14:16], 1710)
	binary.LittleEndian.PutUint32(move[16:20], 6)
	binary.LittleEndian.PutUint16(move[24:26], 1116)
	binary.LittleEndian.PutUint16(move[26:28], 1707)
	copy(move[28:], []byte("233"))
	w.onMove(p.Session, move)
	if !p.MovePublished || p.MovePublishedTargetX != 1116 || p.MovePublishedTargetY != 1707 {
		t.Fatalf("the real Noatum replanning was refused: moving=%v target=(%d,%d)",
			p.MovePublished, p.MovePublishedTargetX, p.MovePublishedTargetY)
	}
	if p.MovePublishedStartX != 1116 || p.MovePublishedStartY != 1713 {
		t.Fatalf("the observer got the early visual origin instead of the authority: (%d,%d)",
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
		t.Fatalf("the route published to the observer ended at (%d,%d)", x, y)
	}
}

func TestMovementVisualCatchupUsesMaximumSpeedWithoutAcceleratingFutureRoute(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.Char.Score.AttackRun = 1
	applyScore(p.Char)

	// Seven steps up to the Pos already walked visually + one future step.
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
		t.Fatalf("the catch-up did not use the maximum speed: (%d,%d)", p.X, p.Y)
	}
	clock.Advance(999 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2107 {
		t.Fatalf("the future stretch was accelerated together with the catch-up: x=%d", p.X)
	}
	clock.Advance(time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2108 || p.MovePublished {
		t.Fatalf("the future stretch did not respect RunSpeed 1: x=%d moving=%v", p.X, p.MovePublished)
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

	// A public mob at the same physical coordinate does not exist inside the
	// private room, so it cannot block the player's step.
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
		t.Fatalf("a public mob blocked a private runtime: x=%d", p.X)
	}

	// An entity of the room itself is still an authoritative collision.
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
		t.Fatalf("the player walked through an entity of its own runtime: x=%d", p.X)
	}
}

func TestMovementCrossesOccupiedIntermediateTileButNotOccupiedDestination(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock

	// The NPC stands in the middle of the route, like the test shops in Armia.
	// The native client walks through it because the final destination is free.
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
		t.Fatalf("an NPC on the way interrupted the route: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
	}

	// An occupied final coordinate stays blocked server-side.
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
		t.Fatalf("an occupied destination was not blocked: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
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
		t.Fatal("the attack used the future destination as its origin")
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
		t.Fatal("an NPC at the future destination was opened")
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
		t.Fatal("an item at the future destination was picked up")
	}
	if p.X != 2100 {
		t.Fatalf("an interaction promoted the future destination: x=%d", p.X)
	}
}

func TestMovementAcceptsFull748RouteForSlowCharacter(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.Char.Score.AttackRun = 0 // reproduces the old artificial limit of 8.

	packet := make([]byte, 52)
	binary.LittleEndian.PutUint16(packet[12:14], p.X)
	binary.LittleEndian.PutUint16(packet[14:16], p.Y)
	binary.LittleEndian.PutUint16(packet[24:26], p.X+11)
	binary.LittleEndian.PutUint16(packet[26:28], p.Y)
	for i := 0; i < 11; i++ {
		packet[28+i] = '6'
	}

	if !w.validPlayerMovePacket(p, packet) {
		t.Fatal("a legitimate 11-tile route of the 7.48 client was refused")
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
		t.Fatal("the first send of the route was refused")
	}

	// onMove would already have recorded the destination, but the client still
	// sends PosX at the origin while interpolating the same segment.
	p.X += 11
	if !w.validPlayerMovePacket(p, packet) {
		t.Fatal("a legitimate repeat of the route during the animation was refused")
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
		t.Fatalf("Stop repositioned the player: (%d,%d)", p.X, p.Y)
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
		t.Fatalf("Stop promoted the future destination at once: (%d,%d)", p.X, p.Y)
	}
	clock.Advance(1250 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2105 || p.Y != 2100 {
		t.Fatalf("Stop did not arrive on the authoritative clock: (%d,%d)", p.X, p.Y)
	}
}

func TestActionStopAcceptsNative748FiveTileFinalPlan(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.X, p.Y = 2485, 2016
	p.Char.X, p.Char.Y = p.X, p.Y

	// Real 7.48 capture: Pos=(2485,2016), Speed=3,
	// Target=(2480,2015), Route[24] zeroed.
	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], 2485)
	binary.LittleEndian.PutUint16(stop[14:16], 2016)
	binary.LittleEndian.PutUint32(stop[16:20], 3)
	binary.LittleEndian.PutUint16(stop[24:26], 2480)
	binary.LittleEndian.PutUint16(stop[26:28], 2015)
	w.onActionStop(p.Session, stop)
	if p.X != 2485 || p.Y != 2016 {
		t.Fatalf("ActionStop granted an early destination: (%d,%d)", p.X, p.Y)
	}
	if !p.MovePublished || p.MovePublishedTargetX != 2480 || p.MovePublishedTargetY != 2015 {
		t.Fatal("the native ActionStop did not create the final authoritative plan")
	}
	clock.Advance(1250 * time.Millisecond) // RunSpeed 4: five steps.
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2480 || p.Y != 2015 || p.MovePublished {
		t.Fatalf("ActionStop did not end at the target: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
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
		t.Fatalf("an invalid ActionStop changed the movement: (%d,%d) moving=%v", p.X, p.Y, p.MovePublished)
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
		t.Fatalf("the attack went through a wall: hp=%d target=%d", mob.HP, mob.TargetID)
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
		t.Fatalf("a drop without a save did not roll back: inv=%d ground=%d", p.Char.Inv[0].Index, len(w.groundItems))
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
		t.Fatal("a pickup without a save did not restore the inventory/ground object")
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
		t.Fatal("a remote purchase in a stale shop was accepted")
	}

	p.X = shop.X
	p.ShopNPC = shop.ID
	p.Char.Equip[0] = model.Item{Index: 400}
	sell := make([]byte, 20)
	binary.LittleEndian.PutUint16(sell[12:14], shop.ID)
	sell[14], sell[16] = placeEquip, 0
	w.onSellItem(p.Session, sell)
	if p.Char.Equip[0].Index != 400 || p.Char.Gold != 5000 {
		t.Fatal("a forged sale of the face slot was accepted")
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
		t.Fatalf("Delay=4 produced cooldown %s, want 4s", remaining)
	}
}
