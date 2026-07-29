package game

import (
	"encoding/binary"
	"fmt"
	"log"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	securityViolationLimit   = 12
	securityViolationWindow  = time.Minute
	maxMovementRouteBytes    = 24
	maxStopPositionDrift     = 3
	characterLoginPacketSize = 36
	applyBonusPacketSize     = 20
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
		case wire.OpCreateCharacter, wire.OpDeleteCharacter, wire.OpCharacterLogin:
			return true
		default:
			return false
		}
	case phaseWorld:
		switch opcode {
		case wire.OpConnectAccount, wire.OpCreateCharacter, wire.OpDeleteCharacter,
			wire.OpCharacterLogin:
			return false
		default:
			return true
		}
	default:
		return false
	}
}

// validateInboundCommand impede replay de transicoes de sessao (por exemplo,
// reenviar CharacterLogin para materializar o mesmo char duas vezes). Handlers
// continuam validando os campos especificos; esta e a fronteira comum.
func (w *World) validateInboundCommand(s *net.Session, pkt []byte) bool {
	if s == nil || len(pkt) < wire.HeaderSize || len(pkt) > wire.MaxPacketSize {
		if s != nil {
			w.recordSecurityViolation(s, 0, "tamanho de pacote invalido")
		}
		return false
	}
	header := wire.ParseHeader(pkt)
	if int(header.Size) != len(pkt) {
		w.recordSecurityViolation(s, header.Type, fmt.Sprintf("Size=%d bytes=%d", header.Size, len(pkt)))
		return false
	}
	if expected, exact := exactInboundPacketSize(header.Type); exact && len(pkt) != expected {
		w.recordSecurityViolation(s, header.Type,
			fmt.Sprintf("tamanho %d, esperado %d", len(pkt), expected))
		return false
	}
	phase := w.phaseFor(s)
	if !opcodeAllowedInPhase(phase, header.Type) {
		w.recordSecurityViolation(s, header.Type, fmt.Sprintf("opcode fora da fase %d", phase))
		return false
	}
	return true
}

// exactInboundPacketSize contem somente layouts confirmados no client 7.48.
// Opcodes de craft/chat com variantes ficam no parser especifico ate haver uma
// captura que feche todos os tamanhos aceitos.
func exactInboundPacketSize(opcode uint16) (int, bool) {
	switch opcode {
	case wire.OpConnectAccount:
		return 116, true
	case wire.OpCreateCharacter:
		return 36, true
	case wire.OpCharacterLogin:
		// Client 7.48: header(12) + Slot(4) + Force(4) + SecretCode(16).
		// O slot continua em @12; a cauda faz parte do contrato de framing e
		// nao pode ser descartada pela validacao anti-packet.
		return characterLoginPacketSize, true
	case wire.OpCharacterLogout:
		return 12, true
	case wire.OpSwapItem:
		return 20, true
	case wire.OpDeposit, wire.OpWithdraw:
		return 16, true
	case wire.OpUseItem:
		return 36, true
	case wire.OpUseNPC:
		return 20, true
	case wire.OpReqShopList:
		return 16, true
	case wire.OpBuyItem:
		return 24, true
	case wire.OpSellItem:
		return 20, true
	case wire.OpApplyBonus:
		// Header(12) + BonusType(2) + Detail(2) + TargetID(2), arredondado
		// pelo layout nativo para 20 bytes. TargetID e usado na compra de skill.
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
	case wire.OpSetShortSkill:
		return 32, true
	case wire.OpChangeCity, wire.OpReqTeleport, wire.OpPKMode:
		return 16, true
	case wire.OpMoveStop:
		return 36, true
	case wire.OpRestart, wire.OpPing:
		return 12, true
	case wire.OpSysQuit:
		return 16, true
	case wire.OpAction, wire.OpActionStop:
		return 52, true
	case wire.OpREQMobByID:
		return 16, true
	case wire.OpMotion:
		return 20, true
	case wire.OpClientUnknown2BC:
		return 108, true
	case wire.OpAttackOne:
		return 48, true
	case wire.OpAttackTwo:
		return 52, true
	case wire.OpAttackMulti:
		return 96, true
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
		log.Printf("[#%d] SEGURANCA opcode=0x%X recusado: %s (%d/%d)",
			s.ID, opcode, reason, state.violations, securityViolationLimit)
		state.lastLog = now
	}
	if state.violations >= securityViolationLimit {
		s.Close()
	}
}

func movementSegmentLimit(_ *Player) int {
	// No 0x366 do jogador, Route[24] descreve a rota planejada inteira. O client
	// pode enviar os 24 passos mesmo com RunSpeed baixo e repetir o segmento
	// enquanto ainda o anima. A regra nativa 2*Speed pertence ao movimento
	// gerado para mobs; aplicá-la aqui deixava o servidor para trás do client.
	// A velocidade efetiva continua limitada separadamente pelo budget temporal.
	return maxMovementRouteBytes
}

func movementTilesPerSecond(p *Player) float64 {
	speed := 0
	if p != nil && p.Char != nil {
		speed = int(playerAttackRun(p.Char) & 0x0F)
	}
	// Folga acima da animacao nominal para jitter e pacotes agrupados. Ainda
	// limita um client adulterado a uma progressao proporcional ao runspeed
	// server-side, nunca ao valor que ele escreveu na memoria local.
	return 6 + float64(speed)*1.5
}

func (w *World) consumeMovementBudget(p *Player, distance int) bool {
	if p == nil || distance <= 0 {
		return p != nil
	}
	now := w.now()
	capacity := float64(movementSegmentLimit(p))
	if p.MoveBudgetAt.IsZero() {
		p.MoveBudgetAt = now
		// O primeiro pacote e uma intencao de rota futura, nao um teleporte ja
		// percorrido. Permite um segmento wire valido, mas inicia o budget vazio
		// para impedir que varios segmentos novos sejam enfileirados no mesmo
		// instante.
		if float64(distance) > capacity {
			return false
		}
		p.MoveBudget = 0
		return true
	} else if elapsed := now.Sub(p.MoveBudgetAt); elapsed > 0 {
		p.MoveBudget += elapsed.Seconds() * movementTilesPerSecond(p)
		if p.MoveBudget > capacity {
			p.MoveBudget = capacity
		}
		p.MoveBudgetAt = now
	}
	if float64(distance) > p.MoveBudget {
		return false
	}
	p.MoveBudget -= float64(distance)
	return true
}

var routeDirections = map[byte][2]int{
	'1': {-1, -1}, '2': {0, -1}, '3': {1, -1},
	'4': {-1, 0}, '6': {1, 0},
	'7': {-1, 1}, '8': {0, 1}, '9': {1, 1},
}

func (w *World) validPlayerMovePacket(p *Player, pkt []byte) bool {
	if p == nil || len(pkt) != 52 {
		return false
	}
	targetX, targetY := actionTarget748(pkt)
	if targetX == 0 || targetY == 0 || !w.terrain.Walkable(targetX, targetY) {
		return false
	}
	startX := binary.LittleEndian.Uint16(pkt[12:14])
	startY := binary.LittleEndian.Uint16(pkt[14:16])
	if startX == 0 || startY == 0 {
		startX, startY = p.X, p.Y
	}
	// O servidor guarda o destino planejado imediatamente, enquanto o client
	// ainda interpola a partir de PosX/PosY e pode repetir o mesmo 0x366. Logo a
	// origem pode ficar atras do destino autoritativo por toda a rota, mas nunca
	// alem do proprio Route[24].
	if chebyshev(p.X, p.Y, startX, startY) > maxMovementRouteBytes ||
		!w.terrain.Walkable(startX, startY) {
		return false
	}

	x, y := int(startX), int(startY)
	steps := 0
	for _, encoded := range pkt[28:52] {
		if encoded == 0 {
			break
		}
		direction, ok := routeDirections[encoded]
		if !ok {
			return false
		}
		nextX, nextY := x+direction[0], y+direction[1]
		if nextX <= 0 || nextY <= 0 || nextX >= 4096 || nextY >= 4096 {
			return false
		}
		if !w.terrain.RouteHeightCompatible(uint16(x), uint16(y), uint16(nextX), uint16(nextY)) {
			return false
		}
		x, y = nextX, nextY
		steps++
		if steps > movementSegmentLimit(p) {
			return false
		}
	}
	if steps > 0 {
		return uint16(x) == targetX && uint16(y) == targetY &&
			w.consumeMovementBudget(p, chebyshev(p.X, p.Y, targetX, targetY))
	}
	// Alguns pacotes intermediarios do 7.48 chegam sem Route. Eles so podem
	// reportar um segmento curto e inteiramente transitavel.
	distance := chebyshev(p.X, p.Y, targetX, targetY)
	return distance <= movementSegmentLimit(p) &&
		w.terrain.LineOfSight(p.X, p.Y, targetX, targetY) &&
		w.consumeMovementBudget(p, distance)
}

func (w *World) validReportedStop(p *Player, x, y uint16) bool {
	return p != nil && x > 0 && y > 0 &&
		chebyshev(p.X, p.Y, x, y) <= maxStopPositionDrift &&
		w.terrain.Walkable(x, y) && w.terrain.LineOfSight(p.X, p.Y, x, y)
}

func (w *World) combatLineOfSight(fromX, fromY, toX, toY uint16) bool {
	return w.terrain.LineOfSight(fromX, fromY, toX, toY)
}
