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
	if !knownInboundOpcode(header.Type) {
		w.recordSecurityViolation(s, header.Type, "opcode C->S desconhecido")
		return false
	}
	if expected, exact := exactInboundPacketSize(header.Type); exact && len(pkt) != expected {
		w.recordSecurityViolation(s, header.Type,
			fmt.Sprintf("tamanho %d, esperado %d", len(pkt), expected))
		return false
	}
	if header.Type == wire.OpRebuy && len(pkt) != wire.HeaderSize && len(pkt) != repurchasePacketSize {
		w.recordSecurityViolation(s, header.Type,
			fmt.Sprintf("tamanho %d, esperado %d ou %d", len(pkt), wire.HeaderSize, repurchasePacketSize))
		return false
	}
	phase := w.phaseFor(s)
	if !opcodeAllowedInPhase(phase, header.Type) {
		w.recordSecurityViolation(s, header.Type, fmt.Sprintf("opcode fora da fase %d", phase))
		return false
	}
	return true
}

// knownInboundOpcode e a allowlist canonica da borda C->S. Um opcode sem
// parser/semantica confirmados nao chega ao dispatcher, nao cria uma label de
// metrica arbitraria e nao pode transformar log sincrono em amplificador de
// CPU/I/O. Rebuy e o unico layout conhecido com dois tamanhos validos.
func knownInboundOpcode(opcode uint16) bool {
	if opcode == wire.OpRebuy {
		return true
	}
	_, exact := exactInboundPacketSize(opcode)
	return exact
}

// exactInboundPacketSize contem somente layouts confirmados no client 7.48.
// Restringir tambem opcodes ignorados impede usar uma cauda arbitraria como
// canal de packet smuggling ou para explorar um parser futuro.
func exactInboundPacketSize(opcode uint16) (int, bool) {
	switch opcode {
	case wire.OpConnectAccount:
		return 116, true
	case wire.OpCreateCharacter:
		return 36, true
	case wire.OpDeleteCharacter:
		return 44, true
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
	case wire.OpUsePremiumFirework:
		return premiumFireworkUsePacketSize, true
	case wire.OpCapsuleInfo:
		return 16, true
	case wire.OpPutoutSeal:
		return 52, true
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
	case wire.OpDeleteItem, wire.OpUpdateItem:
		return 20, true
	case wire.OpSplitItem:
		return 24, true
	case wire.OpSetShortSkill:
		return 32, true
	case wire.OpMessageChat:
		return 140, true
	case wire.OpMessageWhisper:
		// 7.48: Header + MobName[16] + String[96] + Color DWORD.
		return 128, true
	case wire.OpChangeCity, wire.OpReqTeleport, wire.OpPKMode:
		return 16, true
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
	case wire.OpRebuy:
		// A solicitacao pode ser somente o header ou o MSG completo. A
		// validacao acima documenta e limita explicitamente as duas formas.
		return 0, false
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
		return 48, true
	case wire.OpAttackTwo:
		return 52, true
	case wire.OpAttackMulti:
		return 96, true
	case wire.OpReqRanking:
		return 20, true
	case wire.OpCombineTiny, wire.OpCombineLindy, wire.OpCombineCompositor,
		wire.OpCombineAgatha, wire.OpCombineAylin, wire.OpCombineEhre,
		wire.OpCombineOdin:
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

var routeDirections = map[byte][2]int{
	'1': {-1, -1}, '2': {0, -1}, '3': {1, -1},
	'4': {-1, 0}, '6': {1, 0},
	'7': {-1, 1}, '8': {0, 1}, '9': {1, 1},
}

func (w *World) validPlayerMovePacket(p *Player, pkt []byte) bool {
	_, _, _, _, ok := w.validatedPlayerMoveRoute(p, pkt)
	return ok
}

// validatedPlayerMoveRoute valida o plano recebido e o converte em uma rota
// que parte da posicao autoritativa atual. Repeticoes do mesmo Route[24] sao
// aceitas mesmo depois de alguns passos, mas somente o sufixo ainda nao
// percorrido pode virar autoridade.
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
	if chebyshev(p.X, p.Y, startX, startY) > maxMovementRouteBytes ||
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
		// Escolher a ultima ocorrencia impede uma rota com loop de obrigar o
		// personagem a repetir passos ja percorridos.
		currentAt := -1
		for index := range positions {
			if positions[index][0] == p.X && positions[index][1] == p.Y {
				currentAt = index
			}
		}
		if currentAt < 0 {
			// Ao virar durante uma caminhada, o client pode iniciar o novo
			// Route na posicao visual, poucos passos adiante da autoridade. Essa
			// origem so e aceita quando pertence ao plano antigo ainda pendente;
			// os passos faltantes sao preservados, nunca saltados.
			prefix, found := playerMovementPrefixTo(p, startX, startY)
			if !found || len(prefix)+len(wireRoute) > maxMovementRouteBytes {
				return 0, 0, nil, nil, false
			}
			authority := append(prefix, wireRoute...)
			return startX, startY, wireRoute, authority, true
		}
		authority := append([]byte(nil), wireRoute[currentAt:]...)
		return startX, startY, wireRoute, authority, true
	}
	// Alguns pacotes intermediarios do 7.48 chegam sem Route. Eles so podem
	// reportar um segmento curto e inteiramente transitavel. O servidor gera a
	// linha de passos para que o destino continue sendo futuro, nao um salto.
	distance := chebyshev(p.X, p.Y, targetX, targetY)
	if distance > movementSegmentLimit(p) || !w.terrain.LineOfSight(p.X, p.Y, targetX, targetY) {
		return 0, 0, nil, nil, false
	}
	authority := directMovementRoute(p.X, p.Y, targetX, targetY)
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
	return false
}

func (w *World) combatLineOfSight(fromX, fromY, toX, toY uint16) bool {
	return w.terrain.LineOfSight(fromX, fromY, toX, toY)
}
