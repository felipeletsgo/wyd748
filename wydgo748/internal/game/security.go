package game

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	securityViolationLimit  = 12
	securityViolationWindow = time.Minute
	maxMovementRouteBytes   = 24
	maxMovementQueuedSteps  = maxMovementRouteBytes * 2
	// The 7.48 client recalculates from its visual position, which can exceed
	// Route[24] when replaced plans leave authority behind. Recovery shares the
	// queue limit: every tile must be traversable and due on the server clock;
	// it never becomes a teleport.
	maxMovementVisualBridge = maxMovementQueuedSteps
	maxStopPositionDrift    = 3
	// The 7.48 client computes at most six steps for ActionStop 0x367, but sends
	// only Pos/Target; this packet's Route[24] is zeroed.
	maxActionStopRouteSteps       = 6
	characterLoginPacketSize      = 36
	applyBonusPacketSize          = 20
	attackOneObservedExtendedSize = 96
)

type securityState struct {
	windowStart time.Time
	violations  int
	lastLog     time.Time
}

type sessionPhase byte

const (
	phaseConnected sessionPhase = iota
	phaseAuthenticating
	phaseCharacterSelect
	phaseWorld
)

func (w *World) phaseFor(s *net.Session) sessionPhase {
	if w.authPending[s] {
		return phaseAuthenticating
	}
	p := w.players[s]
	if p == nil {
		return phaseConnected
	}
	if p.InWorld {
		return phaseWorld
	}
	return phaseCharacterSelect
}

func opcodeAllowedInPhase(phase sessionPhase, opcode uint16) bool {
	if opcode == wire.OpPing || opcode == wire.OpSysQuit {
		return true
	}
	switch phase {
	case phaseConnected:
		return opcode == wire.OpConnectAccount
	case phaseAuthenticating:
		return false
	case phaseCharacterSelect:
		switch opcode {
		case wire.OpCreateCharacter, wire.OpDeleteCharacter, wire.OpCharacterLogin,
			wire.OpCharacterTransfer:
			return true
		default:
			return false
		}
	case phaseWorld:
		switch opcode {
		case wire.OpConnectAccount, wire.OpCreateCharacter, wire.OpDeleteCharacter,
			wire.OpCharacterLogin, wire.OpCharacterTransfer:
			return false
		default:
			return true
		}
	default:
		return false
	}
}

// validateInboundCommand rejects replayed session transitions, such as a second
// CharacterLogin that would materialize the same character twice. Handlers
// retain their domain validation; this is the shared ingress boundary.
func (w *World) validateInboundCommand(s *net.Session, pkt []byte) bool {
	if s == nil || len(pkt) < wire.HeaderSize || len(pkt) > wire.MaxPacketSize {
		return w.rejectInboundCommand(s, pkt, 0, "invalid packet size")
	}
	header := wire.ParseHeader(pkt)
	if int(header.Size) != len(pkt) {
		return w.rejectInboundCommand(s, pkt, header.Type,
			fmt.Sprintf("Size=%d bytes=%d", header.Size, len(pkt)))
	}
	if !knownInboundOpcode(header.Type) {
		return w.rejectInboundCommand(s, pkt, header.Type, "unknown C->S opcode")
	}
	if allowed, expected := inboundPacketSizeAllowed(header.Type, len(pkt)); !allowed {
		return w.rejectInboundCommand(s, pkt, header.Type,
			fmt.Sprintf("size %d, expected %s", len(pkt), expected))
	}
	phase := w.phaseFor(s)
	if !opcodeAllowedInPhase(phase, header.Type) {
		return w.rejectInboundCommand(s, pkt, header.Type,
			fmt.Sprintf("opcode outside phase %d", phase))
	}
	if header.Type == wire.OpSellItem && !validSaleInventorySource(pkt) {
		return w.rejectInboundCommand(s, pkt, header.Type, "invalid sale inventory source")
	}
	w.relaxLearnedSkillIngressThrottle(s, pkt, header.Type)
	return true
}

func (w *World) rejectInboundCommand(s *net.Session, pkt []byte, opcode uint16, reason string) bool {
	if opcode == 0 && len(pkt) >= 6 {
		opcode = binary.LittleEndian.Uint16(pkt[4:6])
	}
	w.recordSecurityViolation(s, opcode, reason)
	// 0x2C2 answers a server-side challenge, not a gameplay intention. Invalid
	// framing, phase, or layout fails closed immediately.
	if s != nil && opcode == wire.OpClientIntegrityResponse {
		s.Close()
	}
	return false
}

// relaxLearnedSkillIngressThrottle removes only the GLOBAL time floor between
// skills on the network path. The busy-loop guard duplicated two stronger
// protections: Session rate limiting and gameplay SkillReady, derived from
// SkillData.Delay. Short rotations lost a different valid skill just because
// another skill had arrived less than 200 ms earlier.
//
// Cleanup follows framing/phase validation and requires SkillId to resolve to
// a skill the character actually learned. LastSkillTicks still prevents
// per-skill replay/rewind; physical attacks bypass this path and remain limited
// by acceptClientAttack/AttackRun.
func (w *World) relaxLearnedSkillIngressThrottle(s *net.Session, pkt []byte, opcode uint16) {
	switch opcode {
	case wire.OpAttackOne, wire.OpAttackTwo, wire.OpAttackMulti:
	default:
		return
	}
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	req := parseAttackSkill(pkt)
	if !isLearnedClassSkill(p.Char, req.Skill) {
		return
	}
	p.LastSkillAt = time.Time{}
}

// knownInboundOpcode is the canonical C->S allowlist. An opcode without a
// confirmed parser/semantics cannot reach dispatch, create an arbitrary metric
// label, or amplify CPU/I/O through synchronous logs. AttackOne has multiple
// observed/confirmed sizes.
func knownInboundOpcode(opcode uint16) bool {
	_, exact := exactInboundPacketSize(opcode)
	return exact
}

// inboundPacketSizeAllowed preserves strict framing while admitting variants
// observed from the real 7.48 client. An in-game capture recorded 96-byte 0x39D
// frames during magic combat; the parser uses the supported prefix fields and
// the server recalculates targets/damage. Other sizes, including arbitrary
// tails, remain rejected.
func inboundPacketSizeAllowed(opcode uint16, size int) (bool, string) {
	if opcode == wire.OpAttackOne {
		return size == 48 || size == attackOneObservedExtendedSize, "48 or 96"
	}
	expected, exact := exactInboundPacketSize(opcode)
	if !exact {
		return false, "confirmed layout"
	}
	return size == expected, fmt.Sprintf("%d", expected)
}

// exactInboundPacketSize contains 7.48 layouts and versioned coordinated
// extensions. Restricting ignored opcodes also prevents arbitrary tails from
// becoming a packet-smuggling channel or exploiting a future parser.
func exactInboundPacketSize(opcode uint16) (int, bool) {
	switch opcode {
	case wire.OpQuizAnswer:
		return wire.QuizAnswerSize, true
	case wire.OpConnectAccount:
		return 116, true
	case wire.OpCreateCharacter:
		return 36, true
	case wire.OpDeleteCharacter:
		return 44, true
	case wire.OpCharacterTransfer:
		return 52, true
	case wire.OpCharacterLogin:
		// Client 7.48: header(12) + Slot(4) + Force(4) + SecretCode(16).
		// The slot remains at @12; the tail belongs to the framing contract
		// and must not be discarded by packet validation.
		return characterLoginPacketSize, true
	case wire.OpCharacterLogout:
		return 12, true
	case wire.OpClientIntegrityResponse:
		return wire.ClientIntegrityPacketSize, true
	case wire.OpSwapItem:
		return 20, true
	case wire.OpDeposit, wire.OpWithdraw:
		return 16, true
	case wire.OpUseItem:
		return 36, true
	case wire.OpUsePremiumFirework:
		return premiumFireworkUsePacketSize, true
	case wire.OpCapsuleInfo:
		return 16, true
	case wire.OpPutoutSeal:
		return 52, true
	case wire.OpBuyToto:
		return 36, true
	case wire.OpDoJackpotBet:
		return 20, true
	case wire.OpUseNPC:
		return 20, true
	case wire.OpReqShopList:
		return 16, true
	case wire.OpBuyItem:
		return 24, true
	case wire.OpSellItem:
		return 20, true
	case wire.OpApplyBonus:
		// Header(12) + BonusType(2) + Detail(2) + TargetID(2), rounded to
		// 20 bytes by the native layout. Skill purchases use TargetID.
		return applyBonusPacketSize, true
	case wire.OpPartyRequest:
		return 44, true
	case wire.OpPartyAccept:
		return 32, true
	case wire.OpPartyRemove:
		return 16, true
	case wire.OpTrade:
		return 156, true
	case wire.OpCloseTrade:
		return 12, true
	case wire.OpAutoTrade:
		return 196, true
	case wire.OpReqTradeList:
		return 16, true
	case wire.OpReqBuyAutoTrade:
		return 36, true
	case wire.OpDropItem:
		return 32, true
	case wire.OpGetItem:
		return 28, true
	case wire.OpDeleteItem, wire.OpUpdateItem:
		return 20, true
	case wire.OpSplitItem:
		return 24, true
	case wire.OpSetShortSkill:
		return 32, true
	case wire.OpMessageChat:
		return 108, true
	case wire.OpMessageWhisper:
		// 7.48: Header + MobName[16] + String[96] + Color DWORD.
		return 128, true
	case wire.OpChangeCity, wire.OpReqTeleport, wire.OpPKMode:
		return 16, true
	case wire.OpAirMove:
		return 20, true
	case wire.OpMoveStop:
		return 36, true
	case wire.OpUpdateScore:
		return wire.HeaderSize, true
	case wire.OpRestart, wire.OpPing:
		return 12, true
	case wire.OpSysQuit:
		return 16, true
	case wire.OpAction, wire.OpActionStop, wire.OpIllusion:
		return 52, true
	case wire.OpREQMobByID:
		return 16, true
	case wire.OpGuildDeprivate:
		return 16, true
	case wire.OpInviteGuild:
		return 20, true
	case wire.OpGuildAlly, wire.OpGuildWar:
		return 20, true
	case wire.OpChallenge:
		return 16, true
	case wire.OpChallengeConfirm:
		return 20, true
	case wire.OpMotion:
		return 20, true
	case wire.OpClientUnknown2BC:
		return 108, true
	case wire.OpAttackOne:
		// 48 is the canonical compact layout. The real client also sends
		// 96 bytes with this opcode during magic combat, admitted above.
		return 48, true
	case wire.OpAttackTwo:
		return 52, true
	case wire.OpAttackMulti:
		return 96, true
	case wire.OpPlayerChallenge:
		return 20, true
	case wire.OpCombineExtracao:
		return 20, true
	case wire.OpCombineTiny, wire.OpCombineLindy, wire.OpCombineCompositor,
		wire.OpCombineAgatha, wire.OpCombineAylin, wire.OpCombineEhre,
		wire.OpCombineOdin, wire.OpCombineAlquimia:
		return combinePacketSize, true
	default:
		return 0, false
	}
}

func (w *World) recordSecurityViolation(s *net.Session, opcode uint16, reason string) {
	if s == nil {
		return
	}
	if w.security == nil {
		w.security = make(map[*net.Session]*securityState)
	}
	now := w.now()
	state := w.security[s]
	if state == nil || now.Sub(state.windowStart) >= securityViolationWindow {
		state = &securityState{windowStart: now}
		w.security[s] = state
	}
	state.violations++
	if state.lastLog.IsZero() || now.Sub(state.lastLog) >= time.Second ||
		state.violations == securityViolationLimit {
		log.Printf("[#%d] SECURITY opcode=0x%X rejected: %s (%d/%d)",
			s.ID, opcode, reason, state.violations, securityViolationLimit)
		state.lastLog = now
	}
	if state.violations >= securityViolationLimit {
		s.Close()
	}
}

func movementSegmentLimit(_ *Player) int {
	// Player 0x366 Route[24] describes the entire planned route. The client can
	// send all 24 steps even at low RunSpeed and repeat a segment while animating
	// it. Native 2*Speed applies to mob-generated movement; applying it here
	// left the server behind the client. A separate time budget limits speed.
	return maxMovementRouteBytes
}

func movementTilesPerSecond(p *Player) float64 {
	speed := 0
	if p != nil && p.Char != nil {
		speed = int(playerAttackRun(p.Char) & 0x0F)
	}
	// TMHuman interpolates one step every 1000/Speed ms. Authority shares this
	// cadence but derives Speed from server-side Score; the received field never
	// increases speed. The 7.48 client BASE_GetSpeed clamps to 1..7.
	if speed < 1 {
		speed = 1
	} else if speed > 7 {
		speed = 7
	}
	return float64(speed)
}

func movementPacketRejectionSummary(p *Player, pkt []byte) string {
	if p == nil || len(pkt) < 28 {
		return "invalid movement route/destination"
	}
	startX := binary.LittleEndian.Uint16(pkt[12:14])
	startY := binary.LittleEndian.Uint16(pkt[14:16])
	targetX, targetY := actionTarget748(pkt)
	clientSpeed := binary.LittleEndian.Uint32(pkt[16:20])
	route := pkt[28:]
	if len(route) > maxMovementRouteBytes {
		route = route[:maxMovementRouteBytes]
	}
	if nul := bytes.IndexByte(route, 0); nul >= 0 {
		route = route[:nul]
	}
	authX, authY, serverSpeed := uint16(0), uint16(0), byte(0)
	pendingX, pendingY, pendingStep, pendingLen := uint16(0), uint16(0), 0, 0
	if p != nil {
		authX, authY = p.X, p.Y
		if p.Char != nil {
			serverSpeed = playerAttackRun(p.Char) & 0x0F
		}
		pendingX, pendingY = p.MovePublishedTargetX, p.MovePublishedTargetY
		pendingStep, pendingLen = p.MoveAuthorityStep, len(p.MoveAuthorityRoute)
	}
	return fmt.Sprintf("invalid route/destination auth=(%d,%d) pos=(%d,%d) target=(%d,%d) speed=%d/%d route=%q pending=(%d,%d %d/%d)",
		authX, authY, startX, startY, targetX, targetY, clientSpeed, serverSpeed,
		string(route), pendingX, pendingY, pendingStep, pendingLen)
}

var routeDirections = map[byte][2]int{
	// The 7.48 wire uses the axis observed in real captures: "32" changes
	// (2486,2017) to (2487,2015), and "2222" reduces Y by four. Do not copy
	// another version's BASE_GetDestByAction, whose Y axis is reversed.
	'1': {-1, -1}, '2': {0, -1}, '3': {1, -1},
	'4': {-1, 0}, '6': {1, 0},
	'7': {-1, 1}, '8': {0, 1}, '9': {1, 1},
}

func (w *World) validPlayerMovePacket(p *Player, pkt []byte) bool {
	_, _, _, _, ok := w.validatedPlayerMoveRoute(p, pkt)
	return ok
}

// validatedPlayerMoveRoute validates the received plan and converts it into a
// route starting at the current authoritative position. Repeated Route[24]
// plans remain valid after some steps, but only their untraveled suffix can
// become authoritative.
func (w *World) validatedPlayerMoveRoute(p *Player, pkt []byte) (uint16, uint16, []byte, []byte, bool) {
	if p == nil || len(pkt) != 52 {
		return 0, 0, nil, nil, false
	}
	targetX, targetY := actionTarget748(pkt)
	if targetX == 0 || targetY == 0 || !w.terrain.Walkable(targetX, targetY) {
		return 0, 0, nil, nil, false
	}
	startX := binary.LittleEndian.Uint16(pkt[12:14])
	startY := binary.LittleEndian.Uint16(pkt[14:16])
	if startX == 0 || startY == 0 {
		startX, startY = p.X, p.Y
	}
	if chebyshev(p.X, p.Y, startX, startY) > maxMovementVisualBridge ||
		!w.terrain.Walkable(startX, startY) {
		return 0, 0, nil, nil, false
	}

	x, y := int(startX), int(startY)
	wireRoute := make([]byte, 0, maxMovementRouteBytes)
	positions := make([][2]uint16, 1, maxMovementRouteBytes+1)
	positions[0] = [2]uint16{startX, startY}
	for _, encoded := range pkt[28:52] {
		if encoded == 0 {
			break
		}
		direction, ok := routeDirections[encoded]
		if !ok {
			return 0, 0, nil, nil, false
		}
		nextX, nextY := x+direction[0], y+direction[1]
		if nextX <= 0 || nextY <= 0 || nextX >= 4096 || nextY >= 4096 {
			return 0, 0, nil, nil, false
		}
		if !w.terrain.RouteHeightCompatible(uint16(x), uint16(y), uint16(nextX), uint16(nextY)) {
			return 0, 0, nil, nil, false
		}
		x, y = nextX, nextY
		wireRoute = append(wireRoute, encoded)
		positions = append(positions, [2]uint16{uint16(x), uint16(y)})
	}
	if len(wireRoute) > 0 {
		if uint16(x) != targetX || uint16(y) != targetY {
			return 0, 0, nil, nil, false
		}
		// Choosing the last occurrence prevents a loop from forcing the
		// character to repeat steps already traveled.
		currentAt := -1
		for index := range positions {
			if positions[index][0] == p.X && positions[index][1] == p.Y {
				currentAt = index
			}
		}
		if currentAt < 0 {
			// On a turn, the client may start a new Route at its visual position,
			// a few steps ahead of authority. Accept that origin only if it
			// belongs to the old pending plan; preserve the missing steps
			// instead of skipping them.
			prefix, found := playerMovementPrefixTo(p, startX, startY)
			if found {
				authority := append(prefix, wireRoute...)
				if len(authority) <= maxMovementQueuedSteps {
					return startX, startY, wireRoute, authority, true
				}
			}
			// 7.48 transmits plans continuously. A lost intermediate packet can
			// leave the next PosX/Y ahead of the last known Target. Reconstruct
			// only a short, fully traversable corridor from current authority;
			// steps remain subject to the server clock, never an instant jump.
			if chebyshev(p.X, p.Y, startX, startY) <= maxMovementVisualBridge {
				bridge, found := w.shortTerrainRoute(p.X, p.Y, startX, startY,
					maxMovementVisualBridge)
				if !found {
					return 0, 0, nil, nil, false
				}
				authority := append(bridge, wireRoute...)
				if len(authority) <= maxMovementQueuedSteps {
					return startX, startY, wireRoute, authority, true
				}
			}
			return 0, 0, nil, nil, false
		}
		authority := append([]byte(nil), wireRoute[currentAt:]...)
		return startX, startY, wireRoute, authority, true
	}
	// Some intermediate 7.48 packets have no Route. They may report only a
	// short, fully traversable segment. The server generates steps so the
	// destination remains future movement, not a jump.
	distance := chebyshev(p.X, p.Y, targetX, targetY)
	if distance > movementSegmentLimit(p) {
		return 0, 0, nil, nil, false
	}
	authority, found := w.shortTerrainRoute(p.X, p.Y, targetX, targetY,
		movementSegmentLimit(p))
	if !found {
		return 0, 0, nil, nil, false
	}
	return p.X, p.Y, authority, authority, true
}

func playerMovementPrefixTo(p *Player, targetX, targetY uint16) ([]byte, bool) {
	if p == nil || !p.MovePublished || p.MoveAuthorityStep >= len(p.MoveAuthorityRoute) {
		return nil, false
	}
	x, y := int(p.X), int(p.Y)
	prefix := make([]byte, 0, len(p.MoveAuthorityRoute)-p.MoveAuthorityStep)
	for _, encoded := range p.MoveAuthorityRoute[p.MoveAuthorityStep:] {
		direction, ok := routeDirections[encoded]
		if !ok {
			return nil, false
		}
		x, y = x+direction[0], y+direction[1]
		prefix = append(prefix, encoded)
		if uint16(x) == targetX && uint16(y) == targetY {
			return prefix, true
		}
	}
	return nil, false
}

func directMovementRoute(fromX, fromY, toX, toY uint16) []byte {
	route := make([]byte, 0, chebyshev(fromX, fromY, toX, toY))
	x, y := int(fromX), int(fromY)
	for x != int(toX) || y != int(toY) {
		dx, dy := 0, 0
		if x < int(toX) {
			dx = 1
		} else if x > int(toX) {
			dx = -1
		}
		if y < int(toY) {
			dy = 1
		} else if y > int(toY) {
			dy = -1
		}
		encoded, ok := encodeRouteDirection(dx, dy)
		if !ok {
			return nil
		}
		route = append(route, encoded)
		x, y = x+dx, y+dy
	}
	return route
}

type terrainRoutePredecessor struct {
	previous uint32
	step     byte
}

type terrainRouteQueueEntry struct {
	key   uint32
	depth int
}

func terrainPositionKey(x, y uint16) uint32 {
	return uint32(x)<<16 | uint32(y)
}

func terrainPositionFromKey(key uint32) (uint16, uint16) {
	return uint16(key >> 16), uint16(key)
}

// shortTerrainRoute reconstructs only the short walk already shown visually
// between two 0x366 packets. The client does not retransmit that turn in the
// next packet, so requiring LineOfSight rejects valid paths around walls.
// The search is step-bounded and authority executes its result on the clock,
// never as a jump.
func (w *World) shortTerrainRoute(fromX, fromY, toX, toY uint16, maxSteps int) ([]byte, bool) {
	if fromX == toX && fromY == toY {
		return nil, true
	}
	if maxSteps <= 0 || chebyshev(fromX, fromY, toX, toY) > maxSteps ||
		!w.terrain.Walkable(fromX, fromY) || !w.terrain.Walkable(toX, toY) {
		return nil, false
	}
	startKey := terrainPositionKey(fromX, fromY)
	targetKey := terrainPositionKey(toX, toY)
	predecessors := map[uint32]terrainRoutePredecessor{startKey: {}}
	queue := make([]terrainRouteQueueEntry, 1, (maxSteps*2+1)*(maxSteps*2+1))
	queue[0] = terrainRouteQueueEntry{key: startKey}
	// Deterministic order. BFS still finds the fewest steps; diagonals closed
	// by two walls are rejected, as in LOS.
	steps := [...]struct {
		encoded byte
		dx, dy  int
	}{
		{'2', 0, -1}, {'3', 1, -1}, {'6', 1, 0}, {'9', 1, 1},
		{'8', 0, 1}, {'7', -1, 1}, {'4', -1, 0}, {'1', -1, -1},
	}

	for head := 0; head < len(queue); head++ {
		current := queue[head]
		if current.depth >= maxSteps {
			continue
		}
		x, y := terrainPositionFromKey(current.key)
		for _, step := range steps {
			nextX, nextY := int(x)+step.dx, int(y)+step.dy
			if nextX <= 0 || nextY <= 0 || nextX >= 4096 || nextY >= 4096 {
				continue
			}
			nx, ny := uint16(nextX), uint16(nextY)
			if !w.terrain.RouteHeightCompatible(x, y, nx, ny) {
				continue
			}
			if step.dx != 0 && step.dy != 0 &&
				!w.terrain.Walkable(uint16(int(x)+step.dx), y) &&
				!w.terrain.Walkable(x, uint16(int(y)+step.dy)) {
				continue
			}
			nextKey := terrainPositionKey(nx, ny)
			if _, seen := predecessors[nextKey]; seen {
				continue
			}
			predecessors[nextKey] = terrainRoutePredecessor{previous: current.key, step: step.encoded}
			if nextKey == targetKey {
				reversed := make([]byte, 0, current.depth+1)
				for key := targetKey; key != startKey; {
					predecessor := predecessors[key]
					reversed = append(reversed, predecessor.step)
					key = predecessor.previous
				}
				for left, right := 0, len(reversed)-1; left < right; left, right = left+1, right-1 {
					reversed[left], reversed[right] = reversed[right], reversed[left]
				}
				return reversed, true
			}
			queue = append(queue, terrainRouteQueueEntry{key: nextKey, depth: current.depth + 1})
		}
	}
	return nil, false
}

func encodeRouteDirection(dx, dy int) (byte, bool) {
	switch {
	case dx == -1 && dy == -1:
		return '1', true
	case dx == 0 && dy == -1:
		return '2', true
	case dx == 1 && dy == -1:
		return '3', true
	case dx == -1 && dy == 0:
		return '4', true
	case dx == 1 && dy == 0:
		return '6', true
	case dx == -1 && dy == 1:
		return '7', true
	case dx == 0 && dy == 1:
		return '8', true
	case dx == 1 && dy == 1:
		return '9', true
	default:
		return 0, false
	}
}

func (w *World) validReportedStop(p *Player, x, y uint16) bool {
	if p == nil || x == 0 || y == 0 || !w.terrain.Walkable(x, y) {
		return false
	}
	if chebyshev(p.X, p.Y, x, y) <= maxStopPositionDrift &&
		w.terrain.LineOfSight(p.X, p.Y, x, y) {
		return true
	}
	// FUN_0046087b copies the current visual position into 0x2CB before some
	// attacks. Accept coordinates still on the authoritative plan, while
	// onMoveStop ends the route only at the current server-side position.
	if _, found := playerMovementPrefixTo(p, x, y); found {
		return true
	}
	// For a lost intermediate plan, apply the same traversability limit as
	// 0x366. This validates the report without promoting x/y or creating future
	// movement from a stop packet.
	if chebyshev(p.X, p.Y, x, y) > maxMovementVisualBridge {
		return false
	}
	_, found := w.shortTerrainRoute(p.X, p.Y, x, y, maxMovementVisualBridge)
	return found
}

// validatedActionStopRoute reconstructs the short final segment omitted by
// the 7.48 client's ActionStop 0x367. PosX/Y@12 describes the visual origin and
// TargetX/Y@24 the stopping point; native captures include (2485,2016)->
// (2480,2015). The segment never promotes coordinates immediately; it becomes
// a new route subject to the authoritative clock.
func (w *World) validatedActionStopRoute(p *Player, pkt []byte) (uint16, uint16, []byte, bool) {
	if p == nil || len(pkt) != 52 {
		return 0, 0, nil, false
	}
	startX := binary.LittleEndian.Uint16(pkt[12:14])
	startY := binary.LittleEndian.Uint16(pkt[14:16])
	targetX, targetY := actionTarget748(pkt)
	if startX == 0 || startY == 0 {
		startX, startY = p.X, p.Y
	}
	// Some legacy callers send only PosX/Y. Treat that as stopping at the
	// same point instead of inventing another destination.
	if targetX == 0 || targetY == 0 {
		targetX, targetY = startX, startY
	}
	if !w.terrain.Walkable(startX, startY) || !w.terrain.Walkable(targetX, targetY) ||
		chebyshev(startX, startY, targetX, targetY) > maxActionStopRouteSteps {
		return 0, 0, nil, false
	}

	if p.X == targetX && p.Y == targetY {
		return targetX, targetY, nil, true
	}

	// The destination may belong to the validated plan. This is the strongest
	// reconciliation because it preserves the previously transmitted turns.
	if route, found := playerMovementPrefixTo(p, targetX, targetY); found {
		return targetX, targetY, route, true
	}

	stopRoute := directMovementRoute(startX, startY, targetX, targetY)
	if !w.movementRouteHeightCompatible(startX, startY, stopRoute) {
		return 0, 0, nil, false
	}

	var route []byte
	if p.X == startX && p.Y == startY {
		route = stopRoute
	} else if prefix, found := playerMovementPrefixTo(p, startX, startY); found {
		route = append(prefix, stopRoute...)
	} else {
		// An intermediate movement packet may be lost. Reconstruct only a
		// short traversable corridor; World still applies speed step by step.
		if chebyshev(p.X, p.Y, startX, startY) > maxMovementVisualBridge {
			return 0, 0, nil, false
		}
		bridge, found := w.shortTerrainRoute(p.X, p.Y, startX, startY,
			maxMovementVisualBridge)
		if !found {
			return 0, 0, nil, false
		}
		route = append(bridge, stopRoute...)
	}
	// PlayerMove carries at most Route[24]. Do not announce a Target the
	// published route cannot reach.
	if len(route) > maxMovementRouteBytes {
		return 0, 0, nil, false
	}
	return targetX, targetY, route, true
}

func (w *World) movementRouteHeightCompatible(startX, startY uint16, route []byte) bool {
	x, y := startX, startY
	for _, encoded := range route {
		direction, ok := routeDirections[encoded]
		if !ok {
			return false
		}
		nextX := int(x) + direction[0]
		nextY := int(y) + direction[1]
		if nextX <= 0 || nextY <= 0 || nextX >= 4096 || nextY >= 4096 ||
			!w.terrain.RouteHeightCompatible(x, y, uint16(nextX), uint16(nextY)) {
			return false
		}
		x, y = uint16(nextX), uint16(nextY)
	}
	return true
}

func (w *World) combatLineOfSight(fromX, fromY, toX, toY uint16) bool {
	return w.terrain.LineOfSight(fromX, fromY, toX, toY)
}
