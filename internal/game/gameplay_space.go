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
		(p.Char == nil || !hasActiveAffectAt(p.Char, 28, w.now()))
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
	if w.playersByCharacterUID != nil {
		if p := w.playersByCharacterUID[uid]; p != nil && p.Char != nil &&
			strings.TrimSpace(p.Char.UID) == uid && p.InWorld &&
			w.playersByID[p.ID] == p {
			return p
		}
		delete(w.playersByCharacterUID, uid)
	}
	for _, p := range w.playersByID {
		if p != nil && p.InWorld && p.Char != nil && strings.TrimSpace(p.Char.UID) == uid {
			if w.playersByCharacterUID == nil {
				w.playersByCharacterUID = make(map[string]*Player)
			}
			w.playersByCharacterUID[uid] = p
			return p
		}
	}
	return nil
}

func (w *World) indexPlayerCharacter(p *Player) {
	if w == nil || p == nil || p.Char == nil || !p.InWorld {
		return
	}
	uid := strings.TrimSpace(p.Char.UID)
	if uid == "" {
		return
	}
	if w.playersByCharacterUID == nil {
		w.playersByCharacterUID = make(map[string]*Player)
	}
	w.playersByCharacterUID[uid] = p
}

func (w *World) unindexPlayerCharacter(p *Player) {
	if w == nil || p == nil || w.playersByCharacterUID == nil || p.Char == nil {
		return
	}
	uid := strings.TrimSpace(p.Char.UID)
	if uid != "" && w.playersByCharacterUID[uid] == p {
		delete(w.playersByCharacterUID, uid)
	}
}

// resolveMobAffectSource resolves ownership without trusting a recycled
// session ID. A non-empty OwnerCharacterUID is always a player source; an
// OwnerID without that UID is reserved for a live mob source. Ownerless server
// affects are valid and simply have no kill-credit recipient.
func (w *World) resolveMobAffectSource(target *Mob, affect *model.Affect) (*Player, *Mob, bool) {
	if target == nil || affect == nil {
		return nil, nil, false
	}
	if uid := strings.TrimSpace(affect.OwnerCharacterUID); uid != "" {
		owner := w.playerByCharacterUID(uid)
		if !validPlayerMobParticipant(owner) || w.gameplaySpaceForPlayer(owner) != mobGameplaySpace(target) {
			return nil, nil, false
		}
		if target.InstanceID != "" && !instanceMemberInStage(w.instanceForMob(target), owner) {
			return nil, nil, false
		}
		return owner, nil, true
	}
	if affect.OwnerID == 0 {
		return nil, nil, true
	}
	owner := w.mobByID(affect.OwnerID)
	if owner == nil || owner.Dead || owner.HP == 0 ||
		mobGameplaySpace(owner) != mobGameplaySpace(target) {
		return nil, nil, false
	}
	return nil, owner, true
}

func (w *World) affectOwnerPlayer(a *model.Affect) *Player {
	if a == nil {
		return nil
	}
	if uid := strings.TrimSpace(a.OwnerCharacterUID); uid != "" {
		return w.playerByCharacterUID(uid)
	}
	// OwnerID alone is never enough to resolve a player source. It is an
	// ephemeral session/wire hint and may already belong to another character
	// after reconnect; callers treat this as a stale affect and remove it.
	return nil
}

func affectHasPlayerOwner(a *model.Affect) bool {
	return a != nil && (strings.TrimSpace(a.OwnerCharacterUID) != "" || a.OwnerID != 0)
}
