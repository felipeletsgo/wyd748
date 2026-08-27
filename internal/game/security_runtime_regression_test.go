package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestValidateInboundCommandAcceptsObservedAttackOne96Layout(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Attack96", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	packet := inboundPacket(wire.OpAttackOne, attackOneObservedExtendedSize)

	if !w.validateInboundCommand(p.Session, packet) {
		t.Fatal("0x39D de 96 bytes observado no client 7.48 foi recusado")
	}
	if state := w.security[p.Session]; state != nil && state.violations != 0 {
		t.Fatalf("0x39D/96 valido foi contabilizado como violacao: %+v", state)
	}
}

func TestValidateInboundCommandStillRejectsArbitraryAttackOneTail(t *testing.T) {
	p, _ := networkedTestPlayer(1, "AttackTail", 2100, 2100)
	w := worldWithNetworkedPlayers(p)

	if w.validateInboundCommand(p.Session, inboundPacket(wire.OpAttackOne, 80)) {
		t.Fatal("0x39D com tamanho nao observado foi aceito")
	}
}

func TestMovementReconcilesObservedVisualLeadWithoutTeleport(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.X, p.Y = 1115, 1706
	p.Char.X, p.Char.Y = p.X, p.Y

	// Reproduz a primeira recusa do log real:
	// auth=(1115,1706) pos=(1128,1708) target=(1134,1708) route="666666".
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], 1128)
	binary.LittleEndian.PutUint16(move[14:16], 1708)
	binary.LittleEndian.PutUint32(move[16:20], 6)
	binary.LittleEndian.PutUint16(move[24:26], 1134)
	binary.LittleEndian.PutUint16(move[26:28], 1708)
	copy(move[28:34], []byte("666666"))

	_, _, wireRoute, authorityRoute, ok := w.validatedPlayerMoveRoute(p, move)
	if !ok {
		t.Fatal("plano legitimo com 13 tiles de lead visual foi recusado")
	}
	if len(wireRoute) != 6 {
		t.Fatalf("wire route=%d, want 6", len(wireRoute))
	}
	if len(authorityRoute) <= len(wireRoute) || len(authorityRoute) > maxMovementQueuedSteps {
		t.Fatalf("ponte autoritativa invalida: wire=%d authority=%d", len(wireRoute), len(authorityRoute))
	}
	if p.X != 1115 || p.Y != 1706 {
		t.Fatalf("reconciliacao promoveu PosX/Y do client como teleporte: (%d,%d)", p.X, p.Y)
	}
}

func TestMovementAcceptsObservedTwentyFiveTileVisualLeadWithoutTeleport(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.X, p.Y = 1115, 1706
	p.Char.X, p.Char.Y = p.X, p.Y

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], 1140) // 25 tiles a frente.
	binary.LittleEndian.PutUint16(move[14:16], 1706)
	binary.LittleEndian.PutUint16(move[24:26], 1141)
	binary.LittleEndian.PutUint16(move[26:28], 1706)
	move[28] = '6'

	_, _, wireRoute, authorityRoute, ok := w.validatedPlayerMoveRoute(p, move)
	if !ok {
		t.Fatal("lead visual real de 25 tiles foi recusado por apenas um tile")
	}
	if len(wireRoute) != 1 || len(authorityRoute) != 26 ||
		len(authorityRoute) > maxMovementQueuedSteps {
		t.Fatalf("fila reconciliada invalida: wire=%d authority=%d",
			len(wireRoute), len(authorityRoute))
	}
	if p.X != 1115 || p.Y != 1706 {
		t.Fatalf("ponte de 25 tiles virou teleporte: (%d,%d)", p.X, p.Y)
	}
}

func TestMovementStillRejectsVisualLeadBeyondAuthorityQueue(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.X, p.Y = 1115, 1706
	p.Char.X, p.Char.Y = p.X, p.Y

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X+maxMovementQueuedSteps)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], p.X+maxMovementQueuedSteps+1)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	move[28] = '6'

	if _, _, _, _, ok := w.validatedPlayerMoveRoute(p, move); ok {
		t.Fatal("ponte mais rota acima da fila autoritativa foi aceita")
	}
}

func TestMovementRejectsVisualBridgeThroughBlockedTerrain(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.X, p.Y = 1115, 1706
	p.Char.X, p.Char.Y = p.X, p.Y

	// A parede excede o raio da busca nos dois sentidos; origem e destino ficam
	// caminhaveis, mas nenhuma ponte de ate 48 passos consegue atravessa-la.
	for y := int(p.Y) - maxMovementVisualBridge; y <= int(p.Y)+maxMovementVisualBridge; y++ {
		w.terrain.Height[y*model.TerrainWidth+int(p.X)+1] = model.TerrainBlockedByte
	}
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X+2)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], p.X+3)
	binary.LittleEndian.PutUint16(move[26:28], p.Y)
	move[28] = '6'

	if _, _, _, _, ok := w.validatedPlayerMoveRoute(p, move); ok {
		t.Fatal("ponte visual atravessou terreno bloqueado")
	}
}

func TestMovementAcceptsObservedSpeedSevenReplanSequence(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	p.X, p.Y = 2253, 2098
	p.Char.X, p.Char.Y = p.X, p.Y
	p.Char.Score.AttackRun = 7
	applyScore(p.Char)

	// Sequencia que causou as recusas em cascata no client real. Cada Pos e a
	// posicao visual corrente usada pelo 7.48 ao substituir a rota em andamento.
	plans := []struct {
		startX, startY   uint16
		targetX, targetY uint16
		route            string
	}{
		{2278, 2101, 2282, 2101, "6666"},
		{2285, 2102, 2288, 2100, "336"},
		{2298, 2101, 2303, 2099, "33666"},
		{2306, 2100, 2310, 2096, "3333"},
		{2314, 2103, 2319, 2102, "36666"},
		{2321, 2096, 2326, 2095, "36666"},
		{2329, 2091, 2336, 2091, "6666666"},
		{2337, 2094, 2344, 2091, "3336666"},
		{2345, 2096, 2351, 2094, "336666"},
		{2353, 2097, 2360, 2098, "9666666"},
		{2361, 2099, 2368, 2101, "9966666"},
	}
	for index, plan := range plans {
		if index > 0 {
			clock.Advance(time.Second)
			w.advancePlayerMovement(p, clock.Now())
		}
		move := make([]byte, 52)
		binary.LittleEndian.PutUint16(move[12:14], plan.startX)
		binary.LittleEndian.PutUint16(move[14:16], plan.startY)
		binary.LittleEndian.PutUint32(move[16:20], 7)
		binary.LittleEndian.PutUint16(move[24:26], plan.targetX)
		binary.LittleEndian.PutUint16(move[26:28], plan.targetY)
		copy(move[28:], plan.route)
		w.onMove(p.Session, move)

		if state := w.security[p.Session]; state != nil && state.violations != 0 {
			t.Fatalf("plano %d contou violacao legitima: %+v", index+1, state)
		}
		if !p.MovePublished || p.MovePublishedTargetX != plan.targetX ||
			p.MovePublishedTargetY != plan.targetY {
			t.Fatalf("plano %d foi recusado: moving=%v target=(%d,%d)", index+1,
				p.MovePublished, p.MovePublishedTargetX, p.MovePublishedTargetY)
		}
	}

	clock.Advance(10 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2368 || p.Y != 2101 || p.MovePublished {
		t.Fatalf("sequencia nao terminou pelo relogio autoritativo: (%d,%d) moving=%v",
			p.X, p.Y, p.MovePublished)
	}
}

func TestMoveStopAcceptsNativeVisualPositionWithoutRepositioning(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	p.X, p.Y = 2253, 2098
	p.Char.X, p.Char.Y = p.X, p.Y

	// Mesmo sem o 0x366 intermediario, a posicao visual plausivel do 0x2CB nao
	// e uma tentativa de teleporte: o handler deve apenas parar na autoridade.
	moveStop := make([]byte, 36)
	binary.LittleEndian.PutUint32(moveStop[20:24], 2278)
	binary.LittleEndian.PutUint32(moveStop[24:28], 2101)
	w.onMoveStop(p.Session, moveStop)

	if state := w.security[p.Session]; state != nil && state.violations != 0 {
		t.Fatalf("MoveStop visual legitimo contou violacao: %+v", state)
	}
	if p.X != 2253 || p.Y != 2098 {
		t.Fatalf("MoveStop promoveu coordenada visual: (%d,%d)", p.X, p.Y)
	}
}

func TestMoveStopStillRejectsVisualPositionBeyondAuthorityQueue(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()

	moveStop := make([]byte, 36)
	binary.LittleEndian.PutUint32(moveStop[20:24], uint32(p.X)+maxMovementQueuedSteps+1)
	binary.LittleEndian.PutUint32(moveStop[24:28], uint32(p.Y))
	w.onMoveStop(p.Session, moveStop)

	if state := w.security[p.Session]; state == nil || state.violations != 1 {
		t.Fatalf("MoveStop absurdo nao contou violacao: %+v", state)
	}
	if p.X != 2100 || p.Y != 2100 {
		t.Fatalf("MoveStop absurdo reposicionou autoridade: (%d,%d)", p.X, p.Y)
	}
}
