package game

import "time"

func movementStepInterval(p *Player) time.Duration {
	tilesPerSecond := movementTilesPerSecond(p)
	if tilesPerSecond <= 0 {
		tilesPerSecond = 1
	}
	return time.Duration(float64(time.Second) / tilesPerSecond)
}

func movementCatchupStepInterval() time.Duration {
	return time.Second / 6
}

func movementStepDeadline(p *Player, step int) time.Time {
	if p == nil || step <= 0 || p.MoveAuthorityStartedAt.IsZero() {
		return time.Time{}
	}
	catchup := p.MoveAuthorityCatchupSteps
	if catchup < 0 {
		catchup = 0
	} else if catchup > len(p.MoveAuthorityRoute) {
		catchup = len(p.MoveAuthorityRoute)
	}
	catchupUsed := minInt(step, catchup)
	normalUsed := step - catchupUsed
	return p.MoveAuthorityStartedAt.
		Add(time.Duration(catchupUsed) * movementCatchupStepInterval()).
		Add(time.Duration(normalUsed) * p.MoveAuthorityStepInterval)
}

func movementStepsDue(p *Player, now time.Time) int {
	if p == nil || p.MoveAuthorityStepInterval <= 0 || p.MoveAuthorityStartedAt.IsZero() ||
		now.Before(p.MoveAuthorityStartedAt) {
		return 0
	}
	catchup := p.MoveAuthorityCatchupSteps
	if catchup < 0 {
		catchup = 0
	} else if catchup > len(p.MoveAuthorityRoute) {
		catchup = len(p.MoveAuthorityRoute)
	}
	elapsed := now.Sub(p.MoveAuthorityStartedAt)
	catchupDuration := time.Duration(catchup) * movementCatchupStepInterval()
	if elapsed < catchupDuration {
		return int(elapsed / movementCatchupStepInterval())
	}
	return catchup + int((elapsed-catchupDuration)/p.MoveAuthorityStepInterval)
}

func samePlayerMovementDestination(p *Player, targetX, targetY uint16) bool {
	return p != nil && p.MovePublished &&
		p.MovePublishedTargetX == targetX && p.MovePublishedTargetY == targetY
}

func (w *World) beginPlayerMovement(p *Player, fromX, fromY, targetX, targetY uint16,
	wireRoute, authorityRoute []byte, catchupSteps int, now time.Time) {
	if p == nil || len(authorityRoute) == 0 {
		return
	}
	// O client 7.48 atualiza o destino continuamente, inclusive antes do proximo
	// passo visual vencer. Preserve o deadline ja em curso: reinicia-lo em cada
	// 0x366 permite que uma sequencia legitima congele a autoridade para sempre.
	nextStepAt := now
	preserveStepDeadline := false
	if p.MovePublished && len(p.MoveAuthorityRoute) > p.MoveAuthorityStep &&
		p.MoveAuthorityStepInterval > 0 && !p.MoveAuthorityStartedAt.IsZero() {
		preserveStepDeadline = true
		nextStepAt = movementStepDeadline(p, p.MoveAuthorityStep+1)
		if nextStepAt.Before(now) {
			nextStepAt = now
		}
	}
	w.publishPlayerMove(p, fromX, fromY, targetX, targetY, wireRoute)
	p.MoveAuthorityRoute = append(p.MoveAuthorityRoute[:0], authorityRoute...)
	p.MoveAuthorityStep = 0
	if catchupSteps < 0 {
		catchupSteps = 0
	} else if catchupSteps > len(authorityRoute) {
		catchupSteps = len(authorityRoute)
	}
	p.MoveAuthorityCatchupSteps = catchupSteps
	p.MoveAuthorityX, p.MoveAuthorityY = p.X, p.Y
	p.MoveAuthorityStepInterval = movementStepInterval(p)
	p.MoveAuthorityStartedAt = now
	if preserveStepDeadline {
		firstInterval := p.MoveAuthorityStepInterval
		if p.MoveAuthorityCatchupSteps > 0 {
			firstInterval = movementCatchupStepInterval()
		}
		p.MoveAuthorityStartedAt = nextStepAt.Add(-firstInterval)
	}
}

// advancePlayerMovement e a unica rotina que transforma intencao de rota em
// coordenada autoritativa. O client pode repetir/reordenar planos, mas nunca
// escolhe quantos passos ja venceram.
func (w *World) advancePlayerMovement(p *Player, now time.Time) {
	if p == nil || !p.MovePublished || len(p.MoveAuthorityRoute) == 0 ||
		p.MoveAuthorityStepInterval <= 0 {
		return
	}
	// Teleportes e recalls alteram a coordenada por outro fluxo. Um plano antigo
	// jamais pode continuar andando a partir do novo mapa/local.
	if p.X != p.MoveAuthorityX || p.Y != p.MoveAuthorityY {
		clearPublishedPlayerMove(p)
		return
	}
	due := movementStepsDue(p, now)
	if due <= p.MoveAuthorityStep {
		return
	}
	if due > len(p.MoveAuthorityRoute) {
		due = len(p.MoveAuthorityRoute)
	}
	oldX, oldY := p.X, p.Y
	for p.MoveAuthorityStep < due {
		encoded := p.MoveAuthorityRoute[p.MoveAuthorityStep]
		direction, ok := routeDirections[encoded]
		if !ok {
			clearPublishedPlayerMove(p)
			return
		}
		nextX := uint16(int(p.X) + direction[0])
		nextY := uint16(int(p.Y) + direction[1])
		if !w.terrain.RouteHeightCompatible(p.X, p.Y, nextX, nextY) {
			w.publishPlayerStop(p)
			return
		}
		// O client nativo testa ocupação para escolher o destino final, não para
		// cada tile intermediário da Route. Bloquear um intermediário fazia o
		// servidor parar atrás da animação ao atravessar as fileiras de NPCs de
		// Armia. Continue permitindo cruzamento, mas nunca finalize empilhado na
		// mesma entidade do gameplay space.
		isFinalStep := p.MoveAuthorityStep+1 == len(p.MoveAuthorityRoute)
		if isFinalStep && w.positionOccupiedInGameplaySpace(nextX, nextY,
			w.gameplaySpaceForPlayer(p), nil, p, nil) {
			w.publishPlayerStop(p)
			return
		}
		p.X, p.Y = nextX, nextY
		p.MoveAuthorityStep++
	}
	p.MoveAuthorityX, p.MoveAuthorityY = p.X, p.Y
	p.Char.X, p.Char.Y = p.X, p.Y
	if shop := w.ghostShops[p.BrowsingGhostShopID]; shop == nil ||
		!inView(p.X, p.Y, shop.X, shop.Y) {
		p.BrowsingGhostShopID = 0
	}
	if p.MoveAuthorityStep == len(p.MoveAuthorityRoute) {
		clearPublishedPlayerMove(p)
	}
	if p.X != oldX || p.Y != oldY {
		w.refreshPlayerVisibilityAfterMove(p, oldX, oldY)
	}
}

func (w *World) advanceAllPlayerMovement(now time.Time) {
	for _, p := range w.players {
		if p != nil && p.InWorld && p.Char != nil {
			w.advancePlayerMovement(p, now)
		}
	}
}
