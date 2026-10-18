package game

// Water, Magic Chamber and scheduled Nightmare monsters grant EXP, never kill
// loot. Room completion rewards retain their existing rules.
func expOnlyInstance(inst *ItemInstance) bool {
	if inst != nil {
		switch inst.Config.ID {
		case "magic-chamber-normal", "magic-chamber-mystic", "magic-chamber-arcane":
			return true
		}
	}
	return scheduledNightmare(inst) || isDurablePrivateWaterInstance(inst)
}

// Level, attributes and spells must not raise leveling-instance hits above one.
// Preserve misses and subsequent player absorption instead of forcing a hit.
func (w *World) levelingInstanceDamage(m *Mob, damage uint32) uint32 {
	if expOnlyInstance(w.instanceForMob(m)) {
		return min(damage, 1)
	}
	return damage
}
