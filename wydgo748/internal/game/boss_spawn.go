package game

import (
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// boss_spawn.go -- boss spawning and respawning.
//
// Bosses do NOT go through the NPCGener. Each one has its own position and
// respawn, declared in its .lua. Only the ASSETS (face, equipment, base
// attributes) come from the catalog NPC, and the .lua may override them.
//
// This keeps the central promise: no existing world mob becomes a boss.

// bossSpawnState tracks a configured boss over time.
type bossSpawnState struct {
	revision uint64 // Increments on spawn/death; rejects stale administrative intents.
	config   model.BossConfig
	profile  *BossProfile
	// def is this boss's own NPCDef: a COPY of the base NPC with the
	// attributes overridden. A copy, not a reference, otherwise changing the boss
	// would change every ordinary mob of that NPC.
	def *model.NPCDef
	// mobID is the living instance; zero while the boss is dead.
	mobID uint16
	// respawnAt is when it respawns. Zero means no respawn is scheduled.
	respawnAt time.Time
}

// WithBossCatalog injects the bosses loaded from data/boss/.
func WithBossCatalog(catalog model.BossCatalog) WorldOption {
	return func(w *World) { w.bossCatalog = catalog }
}

// spawnConfiguredBosses builds each boss's NPCDef and materializes it. It runs
// at the end of the boot, after the NPCGener.
//
// An error here STOPS THE BOOT on purpose: a boss referencing a missing NPC is
// a content error, and failing early is better than finding out in production
// that the server boss never spawned.
func (w *World) spawnConfiguredBosses() error {
	if len(w.bossCatalog.Bosses) == 0 {
		return nil
	}
	w.bossSpawns = make([]*bossSpawnState, 0, len(w.bossCatalog.Bosses))
	for _, config := range w.bossCatalog.Bosses {
		base := w.npcDefByName(config.NPC)
		if base == nil {
			return fmt.Errorf("boss %q (%s): NPC base %q does not exist in data/npcs",
				config.ID, config.SourceFile, config.NPC)
		}
		// The NPC loader already rejects a missing score; this guard makes a
		// hand-built catalog (test, tool) return a clear error instead of
		// panicking on the dereference just below.
		if base.Score == nil {
			return fmt.Errorf("boss %q (%s): base NPC %q has no score",
				config.ID, config.SourceFile, config.NPC)
		}
		profile, err := compileBossProfile(config)
		if err != nil {
			return err
		}
		state := &bossSpawnState{
			config:  config,
			profile: profile,
			def:     bossDefFrom(base, config),
		}
		w.bossSpawns = append(w.bossSpawns, state)
		if err := w.spawnBoss(state); err != nil {
			return err
		}
	}
	log.Printf("bosses: %d encounters loaded from data/boss", len(w.bossSpawns))
	return nil
}

// bossDefFrom creates the boss NPCDef: a copy of the base NPC (assets) with
// the .lua attributes on top.
func bossDefFrom(base *model.NPCDef, config model.BossConfig) *model.NPCDef {
	def := *base // copia rasa: Equip e array, ExpReward/Gold sao escalares
	if config.Name != "" {
		def.Name = config.Name
	}
	// A boss is always hostile, even if the base NPC was not.
	def.Tipo = model.TipoMonstro

	// Score is a pointer in the base: copying the value keeps the boss from
	// changing the attributes of every mob of that NPC.
	extended := *base.Score
	def.Score = &extended

	stats := config.Stats
	if stats.Level != nil {
		def.Score.Level = *stats.Level
	}
	if stats.MaxHP != nil {
		def.Score.MaxHP = *stats.MaxHP
		def.Score.CurHP = *stats.MaxHP
	}
	if stats.Attack != nil {
		def.Score.Attack = *stats.Attack
	}
	if stats.Defense != nil {
		def.Score.Defense = *stats.Defense
	}
	if stats.AttackRun != nil {
		def.Score.AttackRun = uint32(*stats.AttackRun)
	}
	if stats.ExpReward != nil {
		def.ExpReward = *stats.ExpReward
	}
	if stats.Gold != nil {
		def.Gold = *stats.Gold
	}
	// The base NPC carry does not apply to the boss: its special drops come from
	// its own table (config.Drops), rolled in rollBossDrops. Clearing it here also
	// prevents a double drop, because killMobState still calls rollMobDrops.
	def.Carry = nil
	// Vende only makes sense for a merchant, and a boss is always hostile. Clearing
	// it cuts the slice sharing with the base NPC that the shallow copy would leave.
	def.Vende = nil
	return &def
}

// spawnBoss materializa a instancia e registra o runtime.
func (w *World) spawnBoss(state *bossSpawnState) error {
	x, y := w.findFreePosition(state.config.Spawn.X, state.config.Spawn.Y, 3)
	mobID := w.allocMobID()
	if mobID == 0 {
		return fmt.Errorf("boss %q: mob ID range exhausted", state.config.ID)
	}
	mob := &Mob{
		ID: mobID, Def: state.def, X: x, Y: y,
		HP: state.def.Score.MaxHP, GenerIndex: -1,
	}
	// Segments[0] is the "home" used by the ordinary AI leash.
	mob.Segments[0].X, mob.Segments[0].Y = state.config.Spawn.X, state.config.Spawn.Y
	w.appendMobInstance(mob)
	w.registerMobSpatial(mob)
	if err := w.RegisterBoss(mob.ID, state.profile); err != nil {
		w.removeMobInstance(mob)
		return fmt.Errorf("boss %q: %w", state.config.ID, err)
	}
	w.publishRegisteredMobSpawn(mob)
	state.mobID = mob.ID
	state.revision++
	state.respawnAt = time.Time{}

	w.announceBoss(x, y, state.config.SpawnMessage)
	log.Printf("BOSS %q spawned at (%d,%d) mob=%d hp=%d",
		state.config.ID, x, y, mob.ID, mob.HP)
	return nil
}

// bossAnnounceRadius is the reach of boss notices, in Chebyshev tiles (the
// requested 16x16 area). A boss notice is ENCOUNTER information: players on the
// other side of the map cannot act on it, and a global broadcast becomes spam
// for the whole server at every respawn.
const bossAnnounceRadius = 16

// announceBoss sends the notice only to players close enough to fight.
func (w *World) announceBoss(x, y uint16, message string) {
	if message == "" {
		return
	}
	for _, p := range w.nearbyWorldPlayers(x, y, bossAnnounceRadius) {
		p.Session.Send(wire.MessagePanel(message))
	}
}

// spawnBossAreaReward spreads the shared reward on the ground around the boss.
// It differs from Drops: those go to the inventory of whoever landed the final
// blow, while this stays on the ground for every participant to pick up.
//
// One unit per cell, in growing rings from the body -- stacking everything on
// one cell would give the whole reward to whoever stood on it.
func (w *World) spawnBossAreaReward(m *Mob, reward model.BossAreaReward) {
	if reward.Item == 0 || reward.Amount <= 0 {
		return
	}
	if _, ok := w.items[reward.Item]; !ok {
		log.Printf("BOSS: area reward skipped, item %d does not exist", reward.Item)
		return
	}
	occupied := make(map[uint32]bool, reward.Amount)
	for _, g := range w.nearbyGroundItems(m.X, m.Y, bossAnnounceRadius) {
		occupied[uint32(g.X)<<16|uint32(g.Y)] = true
	}
	placed := 0
	for radius := 1; radius <= bossAnnounceRadius && placed < reward.Amount; radius++ {
		for dx := -radius; dx <= radius && placed < reward.Amount; dx++ {
			for dy := -radius; dy <= radius && placed < reward.Amount; dy++ {
				// Only the ring edge: the smaller radii already covered the interior.
				if abs(dx) != radius && abs(dy) != radius {
					continue
				}
				x, y := int(m.X)+dx, int(m.Y)+dy
				if x < 0 || y < 0 || x > 0xFFFF || y > 0xFFFF {
					continue
				}
				key := uint32(x)<<16 | uint32(y)
				if occupied[key] {
					continue
				}
				if w.spawnGroundReward(uint16(x), uint16(y), reward.Item) {
					occupied[key] = true
					placed++
				}
			}
		}
	}
	log.Printf("BOSS %q: area reward, %d of %d units of item %d on the ground",
		m.Def.Name, placed, reward.Amount, reward.Item)
}

// spawnGroundReward puts ONE unit on the ground at the exact cell, without the
// spawnDrop jitter -- here the ring already chose the position.
func (w *World) spawnGroundReward(x, y uint16, index uint16) bool {
	id, ok := w.allocGroundItemID(index)
	if !ok {
		return false
	}
	item, err := materializeItem(model.Item{Index: index})
	if err != nil {
		return false
	}
	g := &GroundItem{ID: id, Item: item, X: x, Y: y,
		Expire: w.now().Add(groundRewardLifetime)}
	w.registerGroundItem(g)
	w.publishItemSpawn(g)
	return true
}

// groundRewardLifetime allows more time than an ordinary drop: the area reward
// falls at once and the group needs time to pick everything up.
const groundRewardLifetime = 5 * time.Minute

func abs(v int) int {
	if v < 0 {
		return -v
	}
	return v
}

// onBossMobKilled reacts to the death of a configured boss: it announces it,
// schedules the respawn and returns the state. It returns nil if the mob was not a boss.
func (w *World) onBossMobKilled(m *Mob) *bossSpawnState {
	return w.finishBossMobKilled(m, true)
}

func (w *World) finishBossMobKilled(m *Mob, publishAreaReward bool) *bossSpawnState {
	if m == nil {
		return nil
	}
	for _, state := range w.bossSpawns {
		if state.mobID != m.ID {
			continue
		}
		state.mobID = 0
		state.revision++
		w.announceBoss(m.X, m.Y, state.config.DeathMessage)
		if publishAreaReward {
			w.spawnBossAreaReward(m, state.config.AreaReward)
		}
		if state.config.RespawnDelay() > 0 {
			state.respawnAt = w.now().Add(state.config.RespawnDelay())
			log.Printf("BOSS %q died; respawns in %s", state.config.ID, state.config.RespawnDelay())
		} else {
			log.Printf("BOSS %q died; no respawn configured", state.config.ID)
		}
		return state
	}
	return nil
}

// tickBossRespawns respawns the bosses whose time has come. Called by the World tick.
func (w *World) tickBossRespawns(now time.Time) {
	for _, state := range w.bossSpawns {
		if state.mobID != 0 || state.respawnAt.IsZero() || now.Before(state.respawnAt) {
			continue
		}
		state.respawnAt = time.Time{}
		if err := w.spawnBoss(state); err != nil {
			// A respawn failure does not bring the server down: it logs and retries
			// on the next cycle instead of killing the world over one boss.
			log.Printf("BOSS %q: respawn failed: %v", state.config.ID, err)
			state.respawnAt = now.Add(time.Minute)
		}
	}
}

// setItemAmount writes the native EF_AMOUNT (the stack), mirroring how
// itemStackAmount reads it. It reuses an effect pair already marked as amount or
// takes the first free one; without room, the item keeps one unit instead of
// losing another effect.
func setItemAmount(item *model.Item, amount int) {
	if item == nil || amount <= 1 {
		return
	}
	if amount > 255 {
		amount = 255 // EF_AMOUNT is one byte on the native wire
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effectAmount || item.Eff[i*2] == 0 {
			item.Eff[i*2] = effectAmount
			item.Eff[i*2+1] = byte(amount)
			return
		}
	}
}

// rollBossDrops gives the special drops to the killer. It is INDEPENDENT of the
// native carry (cleared in bossDefFrom): the table comes from the .lua and each
// row has its own percentage chance, instead of WYD's per-slot rand()%rate.
//
// It reuses addToInv/spawnDrop, the same path as an ordinary drop: a full
// inventory makes the item fall to the ground instead of disappearing.
func (w *World) rollBossDrops(p *Player, mob *Mob, state *bossSpawnState) {
	w.publishPlannedDrops(w.planBossDrops(p, mob, state))
}

func (w *World) planBossDrops(p *Player, mob *Mob, state *bossSpawnState) []plannedDrop {
	if p == nil || p.Char == nil {
		return nil
	}
	planned := make([]plannedDrop, 0, len(state.config.Drops))
	for _, drop := range state.config.Drops {
		if drop.ChancePercent < 100 && w.intn(100) >= drop.ChancePercent {
			continue
		}
		item := model.Item{Index: drop.Item}
		if drop.Amount > 1 {
			setItemAmount(&item, drop.Amount)
		}
		if slot := addToInv(p.Char, item); slot >= 0 {
			planned = append(planned, plannedDrop{player: p, inventoryPos: slot,
				item: p.Char.Inv[slot], source: "BOSS " + state.config.ID, sourceSlot: -1})
			continue
		}
		planned = append(planned, plannedDrop{player: p, inventoryPos: -1,
			item: item, x: mob.X, y: mob.Y, instanceID: strings.TrimSpace(mob.InstanceID),
			source: "BOSS " + state.config.ID, sourceSlot: -1})
	}
	return planned
}
