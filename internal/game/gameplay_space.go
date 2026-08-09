package game

import (
	"strings"

	"wydgo/internal/model"
)

// mobGameplaySpace is the authoritative interaction space of a mob. The
// empty string is the public world; a non-empty value is a runtime instance.
// It deliberately does not infer ownership from coordinates: two event rooms
// may occupy the same physical map area.
func mobGameplaySpace(m *Mob) string {
	if m == nil {
		return ""
	}
	return strings.TrimSpace(m.InstanceID)
}

func (w *World) hostileMob(m *Mob) bool {
	return m != nil && m.Def != nil && m.Def.IsMonster() && m.SummonerID == 0
}

// playerCanInteractWithMob is the single combat/skill boundary for hostile
// mobs. A public monster is targetable only from the public world; an event
// monster is targetable only by a member of that exact runtime. NPCs and
// player summons are intentionally excluded from this predicate.
func (w *World) playerCanInteractWithMob(p *Player, m *Mob) bool {
	if p == nil || !w.hostileMob(m) {
		return false
	}
	// A few pure targeting helpers intentionally use a zero-value Player in
	// unit tests. Production sessions always have InWorld/Char populated; keep
	// that fixture compatibility without weakening any real session path.
	if p.Char == nil {
		if p.ID != 0 || p.InWorld {
			return false
		}
	} else if !validPlayerMobParticipant(p) {
		return false
	}
	if w.gameplaySpaceForPlayer(p) != mobGameplaySpace(m) {
		return false
	}
	if m.InstanceID != "" {
		// The attacker may still be hidden at this point. Aggressive attack
		// paths break Hide only after target validation; requiring the AI's
		// hidden-target rule here would reject the attack before that break.
		return instanceMemberInStage(w.instanceForMob(m), p)
	}
	return m.insideLeash(p)
}

// mobCanTargetPlayer mirrors playerCanInteractWithMob for AI and boss action
// execution. Keeping both directions in one helper prevents a mob from being
// visible/targetable in one path while still attacking across a runtime wall
// in another path.
func (w *World) mobCanTargetPlayer(m *Mob, p *Player) bool {
	return w.playerCanInteractWithMob(p, m) &&
		(p.Char == nil || !hasActiveAffect(p.Char, 28))
}

func validPlayerMobParticipant(p *Player) bool {
	return p != nil && p.InWorld && p.Char != nil && playerCurHP(p.Char) > 0
}

// playerByCharacterUID resolves the stable persisted identity. Session IDs
// are intentionally not used here because they are recycled after logout.
func (w *World) playerByCharacterUID(uid string) *Player {
	uid = strings.TrimSpace(uid)
	if w == nil || uid == "" {
		return nil
	}
	for _, p := range w.playersByID {
		if p != nil && p.Char != nil && strings.TrimSpace(p.Char.UID) == uid {
			return p
		}
	}
	return nil
}

func (w *World) affectOwnerPlayer(a *model.Affect) *Player {
	if a == nil {
		return nil
	}
	if uid := strings.TrimSpace(a.OwnerCharacterUID); uid != "" {
		return w.playerByCharacterUID(uid)
	}
	if a.OwnerID != 0 {
		// OwnerID is retained only as an in-memory compatibility fallback for
		// old tests/active sessions. Persisted affects without a UID are dropped
		// at load, so a recycled ID cannot resurrect an old debuff.
		return w.playerByID(a.OwnerID)
	}
	return nil
}

func affectHasPlayerOwner(a *model.Affect) bool {
	return a != nil && (strings.TrimSpace(a.OwnerCharacterUID) != "" || a.OwnerID != 0)
}
