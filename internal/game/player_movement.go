package game

import "time"

func movementStepInterval(p *Player) time.Duration {
	tilesPerSecond := movementTilesPerSecond(p)
	if tilesPerSecond <= 0 {
		tilesPerSecond = 1
	}
	return time.Duration(float64(time.Second) / tilesPerSecond)
}

func samePlayerMovementDestination(p *Player, targetX, targetY uint16) bool {
	return p != nil && p.MovePublished &&
		p.MovePublishedTargetX == targetX && p.MovePublishedTargetY == targetY
}

func (w *World) beginPlayerMovement(p *Player, fromX, fromY, targetX, targetY uint16, wireRoute, authorityRoute []byte, now time.Time) {
	if p == nil || len(authorityRoute) == 0 {
		return
	}
	w.publishPlayerMove(p, fromX, fromY, targetX, targetY, wireRoute)
	p.MoveAuthorityRoute = append(p.MoveAuthorityRoute[:0], authorityRoute...)
	p.MoveAuthorityStep = 0
	p.MoveAuthorityX, p.MoveAuthorityY = p.X, p.Y
	p.MoveAuthorityStartedAt = now
	p.MoveAuthorityStepInterval = movementStepInterval(p)
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
	due := int(now.Sub(p.MoveAuthorityStartedAt) / p.MoveAuthorityStepInterval)
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
		if !w.terrain.RouteHeightCompatible(p.X, p.Y, nextX, nextY) ||
			w.positionOccupiedInGameplaySpace(nextX, nextY,
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
		w.refreshPlayerVisibility(p)
	}
}

func (w *World) advanceAllPlayerMovement(now time.Time) {
	for _, p := range w.players {
		if p != nil && p.InWorld && p.Char != nil {
			w.advancePlayerMovement(p, now)
		}
	}
}
