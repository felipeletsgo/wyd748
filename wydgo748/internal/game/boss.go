package game

import (
	"log"
	"time"
)

// boss.go -- boss runtime and registration.
//
// ARCHITECTURE (DOCS/IMPLEMENTED.md, Bosses): every boss IS an ordinary Mob and
// keeps taking part in the grid, visibility, combat, death and packets like any
// other. The extra behavior lives in a PARALLEL BossRuntime, indexed by the mob
// ID. Practical consequences:
//
//   - the ordinary mob AI does not change and pays nothing for bosses existing;
//   - there is no `if mob.IsBoss` scattered across the server: the exclusive
//     logic is concentrated here and in boss_*.go;
//   - no goroutine per boss -- the World remains the only executor.

// BossPendingAction is an already accepted action waiting for its execution time.
type BossPendingAction struct {
	// Generation identifies THIS execution. A callback acts only while the
	// generation still matches; reset, cancellation and replacement increment the
	// counter and invalidate what was scheduled.
	Generation uint64
	ActionID   BossActionID
	TargetID   uint16
	ExecuteAt  time.Time
	Priority   int
	// Interruptible lets a higher-priority action replace it.
	Interruptible bool
}

// BossRuntime is the behavior state of a boss. The PHYSICAL state (position,
// HP, affects) stays in the Mob.
type BossRuntime struct {
	MobID   uint16
	Profile *BossProfile

	Phase BossPhaseID

	// InCombat marks that the encounter started. It never returns to false: the
	// boss HP STAYS where the players left it, on purpose -- a boss with very
	// high HP is meant to fall over hours, and restoring health would make a
	// fight across several sessions impossible.
	InCombat bool

	Cooldowns map[BossActionID]time.Time
	Pending   *BossPendingAction
	// ConsumedRules keeps the Once rules already accepted.
	ConsumedRules map[BossRuleID]struct{}

	// Generation grows with every started action, cancellation or reset.
	Generation uint64

	// Adds are the mobs summoned by THIS encounter, for counting and cleanup.
	Adds map[uint16]struct{}

	// crossedThresholds avoids re-emitting a threshold already crossed while
	// the encounter has not reset.
	crossedThresholds map[int]struct{}
	// pendingThresholds keeps a mandatory transition whose event was evaluated
	// while no action could be accepted because of a priority or pending-action
	// conflict. The threshold becomes crossed only after it is accepted.
	pendingThresholds map[int]BossEvent
}

// newBossRuntime creates the runtime in the profile's initial state.
func newBossRuntime(mobID uint16, profile *BossProfile) *BossRuntime {
	return &BossRuntime{
		MobID:             mobID,
		Profile:           profile,
		Phase:             profile.InitialPhase,
		Cooldowns:         make(map[BossActionID]time.Time, len(profile.Actions)),
		ConsumedRules:     make(map[BossRuleID]struct{}),
		Adds:              make(map[uint16]struct{}),
		crossedThresholds: make(map[int]struct{}),
		pendingThresholds: make(map[int]BossEvent),
	}
}

// actionReady reports whether the action is off cooldown.
func (b *BossRuntime) actionReady(actionID BossActionID, now time.Time) bool {
	ready, tracked := b.Cooldowns[actionID]
	return !tracked || !now.Before(ready)
}

// addsAlive counts the encounter's living adds.
func (b *BossRuntime) addsAlive() int { return len(b.Adds) }

// RegisterBoss binds a profile to an already spawned mob. It returns an error
// instead of panicking: a malformed profile must not bring down the whole world boot.
func (w *World) RegisterBoss(mobID uint16, profile *BossProfile) error {
	if profile == nil {
		return errBossProfileNil
	}
	if profile.rulesByEvent == nil {
		if err := profile.Compile(); err != nil {
			return err
		}
	}
	mob := w.mobsByID[mobID]
	if mob == nil {
		return errBossMobMissing
	}
	if w.bosses == nil {
		w.bosses = make(map[uint16]*BossRuntime)
	}
	w.bosses[mobID] = newBossRuntime(mobID, profile)
	log.Printf("BOSS %q registered on mob id=%d (%d rules)", profile.ID, mobID, len(profile.Rules))
	return nil
}

// UnregisterBoss removes the runtime (final death, despawn). The encounter's
// adds are removed with it; otherwise they would be orphaned in the world.
func (w *World) UnregisterBoss(mobID uint16) {
	boss := w.bosses[mobID]
	if boss == nil {
		return
	}
	w.removeBossAdds(boss)
	delete(w.bosses, mobID)
}

// bossFor returns a mob's runtime, or nil if it is not a boss. It is the only
// entry point used by ordinary gameplay paths -- a map lookup, cheap enough to
// stay on the damage path.
func (w *World) bossFor(mobID uint16) *BossRuntime {
	if len(w.bosses) == 0 {
		return nil
	}
	return w.bosses[mobID]
}

// removeBossAdds removes the encounter's living adds from the world. It follows
// the same path as summons (removePlayerSummons): it hides them from whoever saw
// them and only then discards the instance, so no ghost entry remains in the
// client's Visible list.
func (w *World) removeBossAdds(boss *BossRuntime) {
	for addID := range boss.Adds {
		if mob := w.mobsByID[addID]; mob != nil && !mob.Dead {
			for _, viewer := range w.players {
				w.hideMob(viewer, mob, 0)
			}
			mob.Dead = true
			w.removeMobInstance(mob)
		}
		delete(boss.Adds, addID)
	}
}
