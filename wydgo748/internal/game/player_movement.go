package game

import (
	"time"

	"wydgo/internal/wire"
)

func movementStepInterval(p *Player) time.Duration {
	tilesPerSecond := movementTilesPerSecond(p)
	if tilesPerSecond <= 0 {
		tilesPerSecond = 1
	}
	return time.Duration(float64(time.Second) / tilesPerSecond)
}

func movementCatchupStepInterval() time.Duration {
	// The bridge catches up visual steps already traveled at the native 7.48 cap.
	// Future steps return to the interval derived from the authoritative score.
	return time.Second / 7
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
	if p == nil || p.Char == nil || playerCurHP(p.Char) == 0 || len(authorityRoute) == 0 {
		return
	}
	if !w.oxStepAllowed(p, targetX, targetY) ||
		w.isOXMember(p) && w.ox.phase == "question" && !now.Before(w.ox.deadline) {
		w.publishPlayerStop(p)
		p.Session.Send(wire.ActionRouteCorrection(p.ID, p.X, p.Y))
		return
	}
	// The 7.48 client continuously updates its destination, even before the next
	// visual step is due. Preserve the current deadline: resetting it on every
	// 0x366 would let a legitimate sequence freeze authority indefinitely.
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

// advancePlayerMovement is the only routine that turns route intentions into
// authoritative coordinates. The client may repeat or reorder plans, but it
// never decides how many steps have elapsed.
func (w *World) advancePlayerMovement(p *Player, now time.Time) {
	if p == nil || !p.MovePublished || len(p.MoveAuthorityRoute) == 0 ||
		p.MoveAuthorityStepInterval <= 0 || w.criticalPlayerReserved(p) {
		return
	}
	if p.Char == nil || playerCurHP(p.Char) == 0 {
		clearPublishedPlayerMove(p)
		return
	}
	if w.isOXMember(p) && w.ox.phase == "question" && now.After(w.ox.deadline) {
		now = w.ox.deadline
	}
	// Teleports and recalls change coordinates through another flow. An old
	// plan must never keep walking from the new map or location.
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
	lastCity := -1
	for p.MoveAuthorityStep < due {
		encoded := p.MoveAuthorityRoute[p.MoveAuthorityStep]
		direction, ok := routeDirections[encoded]
		if !ok {
			clearPublishedPlayerMove(p)
			break
		}
		nextX := uint16(int(p.X) + direction[0])
		nextY := uint16(int(p.Y) + direction[1])
		if !w.oxStepAllowed(p, nextX, nextY) || !w.guildWarStepAllowed(p, nextX, nextY) ||
			!hiddenValleyDestinationAllowed(p, nextX, nextY, now) {
			w.publishPlayerStop(p)
			break
		}
		if !w.newbieZoneDestinationAllowed(p, nextX, nextY) {
			w.publishPlayerStop(p)
			if p.Session != nil {
				p.Session.Send(wire.MessagePanel(newbieZoneMessage))
			}
			break
		}
		if !w.terrain.RouteHeightCompatible(p.X, p.Y, nextX, nextY) {
			w.publishPlayerStop(p)
			break
		}
		// The native client checks occupancy at the final destination, not at
		// every intermediate route tile. Blocking one of those tiles left the
		// server behind the animation in Armia's NPC rows. Allow crossing, but
		// never finish stacked with another entity in the gameplay space.
		isFinalStep := p.MoveAuthorityStep+1 == len(p.MoveAuthorityRoute)
		if isFinalStep && w.positionOccupiedInGameplaySpace(nextX, nextY,
			w.gameplaySpaceForPlayer(p), nil, p, nil) {
			w.publishPlayerStop(p)
			break
		}
		p.X, p.Y = nextX, nextY
		p.MoveAuthorityStep++
		if city, ok := bindableCityAt(p.X, p.Y); ok {
			lastCity = city
		}
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
	// Persistence retains the full account and position until acknowledgement.
	// Queue once after all accepted steps and their state updates are complete.
	if lastCity >= 0 {
		w.bindPlayerHomeCityIndex(p, lastCity)
	}
}

func (w *World) advanceAllPlayerMovement(now time.Time) {
	for _, p := range w.players {
		if p != nil && p.InWorld && p.Char != nil {
			w.advancePlayerMovement(p, now)
		}
	}
}
