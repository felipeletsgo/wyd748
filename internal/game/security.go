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
	// O client 7.48 recalcula a rota a partir da posicao visual, que pode passar
	// de Route[24] quando a autoridade ficou atrasada por planos substituidos.
	// A recuperacao usa o mesmo teto da fila: cada tile ainda precisa formar uma
	// rota transitavel e vencer no relogio server-side, nunca vira teleporte.
	maxMovementVisualBridge = maxMovementQueuedSteps
	maxStopPositionDrift    = 3
	// O client 7.48 calcula no maximo seis passos ao montar ActionStop 0x367,
	// mas transmite apenas Pos/Target; a Route[24] desse pacote vem zerada.
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
		return w.rejectInboundCommand(s, pkt, 0, "tamanho de pacote invalido")
	}
	header := wire.ParseHeader(pkt)
	if int(header.Size) != len(pkt) {
		return w.rejectInboundCommand(s, pkt, header.Type,
			fmt.Sprintf("Size=%d bytes=%d", header.Size, len(pkt)))
	}
	if !knownInboundOpcode(header.Type) {
		return w.rejectInboundCommand(s, pkt, header.Type, "opcode C->S desconhecido")
	}
	if allowed, expected := inboundPacketSizeAllowed(header.Type, len(pkt)); !allowed {
		return w.rejectInboundCommand(s, pkt, header.Type,
			fmt.Sprintf("tamanho %d, esperado %s", len(pkt), expected))
	}
	phase := w.phaseFor(s)
	if !opcodeAllowedInPhase(phase, header.Type) {
		return w.rejectInboundCommand(s, pkt, header.Type,
			fmt.Sprintf("opcode fora da fase %d", phase))
	}
	w.relaxLearnedSkillIngressThrottle(s, pkt, header.Type)
	return true
}

func (w *World) rejectInboundCommand(s *net.Session, pkt []byte, opcode uint16, reason string) bool {
	if opcode == 0 && len(pkt) >= 6 {
		opcode = binary.LittleEndian.Uint16(pkt[4:6])
	}
	w.recordSecurityViolation(s, opcode, reason)
	// O 0x2C2 e uma resposta a um desafio server-side e nao uma intencao de
	// gameplay. Framing, fase ou layout invalidos sao fail-closed imediatamente.
	if s != nil && opcode == wire.OpClientIntegrityResponse {
		s.Close()
	}
	return false
}

// relaxLearnedSkillIngressThrottle remove somente o piso temporal GLOBAL entre
// skills no caminho de rede real. Ele existia como protecao de busy-loop, mas
// duplicava duas protecoes mais fortes: o rate-limit da Session e, no gameplay,
// SkillReady derivado de SkillData.Delay. Em rotacoes curtas fazia uma skill
// diferente e valida ser descartada apenas porque outra havia chegado <200 ms
// antes.
//
// A limpeza ocorre depois de framing/fase validados e apenas quando SkillId
// resolve para uma skill realmente aprendida pelo personagem. LastSkillTicks
// continua impedindo replay/rewind por skill; ataques fisicos nao passam aqui e
// continuam limitados por acceptClientAttack/AttackRun.
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

// knownInboundOpcode e a allowlist canonica da borda C->S. Um opcode sem
// parser/semantica confirmados nao chega ao dispatcher, nao cria uma label de
// metrica arbitraria e nao pode transformar log sincrono em amplificador de
// CPU/I/O. Rebuy e AttackOne possuem mais de um tamanho observado/confirmado.
func knownInboundOpcode(opcode uint16) bool {
	if opcode == wire.OpRebuy {
		return true
	}
	_, exact := exactInboundPacketSize(opcode)
	return exact
}

// inboundPacketSizeAllowed preserva framing estrito, mas admite variantes que
// o client 7.48 real comprovadamente envia. Em captura in-game, 0x39D chegou com
// 96 bytes durante combate magico; o parser usa apenas os campos autoritativos
// do prefixo e o servidor recalcula alvos/dano. Outros tamanhos continuam
// recusados, inclusive caudas arbitrarias.
func inboundPacketSizeAllowed(opcode uint16, size int) (bool, string) {
	if opcode == wire.OpAttackOne {
		return size == 48 || size == attackOneObservedExtendedSize, "48 ou 96"
	}
	if opcode == wire.OpRebuy {
		return size == wire.HeaderSize || size == repurchasePacketSize,
			fmt.Sprintf("%d ou %d", wire.HeaderSize, repurchasePacketSize)
	}
	expected, exact := exactInboundPacketSize(opcode)
	if !exact {
		return false, "layout confirmado"
	}
	return size == expected, fmt.Sprintf("%d", expected)
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
		return 108, true
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
		// A solicitacao pode ser somente o header ou o MSG completo. O helper
		// inboundPacketSizeAllowed limita explicitamente as duas formas.
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
		// 48 e o layout compacto canonico. O client real tambem emite 96 bytes
		// com este mesmo opcode em combate magico; a variante e admitida acima.
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
	// TMHuman interpola um passo a cada 1000/Speed ms. A autoridade usa a mesma
	// cadencia, mas deriva Speed do Score server-side; o campo recebido no pacote
	// nunca aumenta a velocidade. BASE_GetSpeed do client 7.48 limita 1..7.
	if speed < 1 {
		speed = 1
	} else if speed > 7 {
		speed = 7
	}
	return float64(speed)
}

func movementPacketRejectionSummary(p *Player, pkt []byte) string {
	if p == nil || len(pkt) < 28 {
		return "rota/destino de movimento invalido"
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
	return fmt.Sprintf("rota/destino invalido auth=(%d,%d) pos=(%d,%d) target=(%d,%d) speed=%d/%d route=%q pending=(%d,%d %d/%d)",
		authX, authY, startX, startY, targetX, targetY, clientSpeed, serverSpeed,
		string(route), pendingX, pendingY, pendingStep, pendingLen)
}

var routeDirections = map[byte][2]int{
	// O wire do client 7.48 usa o eixo observado em capturas reais: "32"
	// transforma (2486,2017) em (2487,2015), e "2222" reduz Y em quatro.
	// Nao copiar BASE_GetDestByAction de outra versao: ali o eixo Y e oposto.
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
			if found {
				authority := append(prefix, wireRoute...)
				if len(authority) <= maxMovementQueuedSteps {
					return startX, startY, wireRoute, authority, true
				}
			}
			// O 7.48 transmite planos continuamente. Quando um pacote intermediario
			// se perde, PosX/Y do proximo plano pode estar adiante do ultimo
			// Target conhecido. Reconstrua apenas um corredor curto e inteiramente
			// transitavel a partir da autoridade atual; os passos continuam sujeitos
			// ao relogio server-side e nunca viram um salto imediato.
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
	// Alguns pacotes intermediarios do 7.48 chegam sem Route. Eles so podem
	// reportar um segmento curto e inteiramente transitavel. O servidor gera a
	// linha de passos para que o destino continue sendo futuro, nao um salto.
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

// shortTerrainRoute reconstrói somente a pequena parte da caminhada já
// percorrida visualmente entre dois pacotes 0x366. O client não retransmite
// essa curva no pacote seguinte, portanto exigir LineOfSight rejeita caminhos
// legítimos ao redor de paredes. A busca é limitada em passos e o resultado
// continua sendo executado pelo relógio autoritativo, nunca aplicado como salto.
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
	// Ordem determinística. A BFS ainda encontra o menor número de passos;
	// diagonais fechadas por duas paredes são rejeitadas como no LOS.
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
	// FUN_0046087b copia a posicao visual corrente para o 0x2CB antes de certos
	// ataques. Aceite uma coordenada que ainda pertence ao plano autoritativo,
	// mas onMoveStop apenas encerra a rota na posicao server-side atual.
	if _, found := playerMovementPrefixTo(p, x, y); found {
		return true
	}
	// Se o plano intermediario se perdeu, aplique o mesmo limite transitavel do
	// 0x366. Isto valida a plausibilidade do relato sem promover x/y nem criar
	// movimento futuro a partir de um pacote de parada.
	if chebyshev(p.X, p.Y, x, y) > maxMovementVisualBridge {
		return false
	}
	_, found := w.shortTerrainRoute(p.X, p.Y, x, y, maxMovementVisualBridge)
	return found
}

// validatedActionStopRoute reconstrói o pequeno trecho final omitido pelo
// ActionStop 0x367 do client 7.48. PosX/Y@12 descreve a origem visual e
// TargetX/Y@24 o ponto onde o avatar vai parar; capturas nativas mostram, por
// exemplo, (2485,2016)->(2480,2015). O trecho nunca promove coordenadas de
// imediato: ele apenas vira uma nova rota sujeita ao relógio autoritativo.
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
	// Alguns callers antigos enviam somente PosX/Y. Trate-o como parada no
	// próprio ponto, sem inventar um destino diferente.
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

	// O destino pode estar no plano já validado. Esta é a reconciliação mais
	// forte porque preserva exatamente as curvas transmitidas anteriormente.
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
		// Um pacote de movimento intermediário pode ter se perdido. Reconstrua
		// somente um corredor curto e transitável; a velocidade continua sendo
		// aplicada passo a passo pelo World.
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
	// PlayerMove carrega no máximo Route[24]. Não anuncie aos observadores um
	// Target que a rota publicada não consegue alcançar.
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
