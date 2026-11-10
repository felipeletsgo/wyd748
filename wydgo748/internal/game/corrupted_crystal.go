package game

import (
	"encoding/json"
	"fmt"
	"math"
	"os"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// CrystalAppearance is reserved in the existing 12-bit mob face field.
const (
	CrystalAppearance       uint16 = 4095
	crystalMediumAppearance uint16 = 4094
	crystalSmallAppearance  uint16 = 4093
)

const (
	crystalShatterDelay  = 1200 * time.Millisecond
	crystalRetryDelay    = 2 * time.Second
	crystalSearchRadius  = 4
	crystalPulseInterval = 8 * time.Second
	crystalThunderSkill  = 33
	crystalPulseMotion   = 254
	crystalMediumCount   = 5
	crystalSmallCount    = 4
	crystalPowderItem    = 413
	defaultMediumHP      = 10_000_000
	defaultSmallHP       = 1_000_000
)

type CorruptedCrystalConfig struct {
	X, Y            uint16
	CrystalHP       uint32
	MediumHP        uint32
	SmallHP         uint32
	PulsePower      uint32
	RespawnSeconds  int
	DeadlineSeconds int
}

// ReadCorruptedCrystal loads the optional public encounter before either channel starts.
func ReadCorruptedCrystal(path string) (CorruptedCrystalConfig, error) {
	if path == "" {
		return CorruptedCrystalConfig{}, nil
	}
	config := CorruptedCrystalConfig{MediumHP: defaultMediumHP, SmallHP: defaultSmallHP}
	raw, err := os.ReadFile(path)
	if err != nil {
		return config, err
	}
	if err := json.Unmarshal(raw, &config); err != nil {
		return config, err
	}
	if !config.valid() {
		return config, fmt.Errorf("invalid corrupted crystal configuration")
	}
	return config, nil
}

func (c CorruptedCrystalConfig) valid() bool {
	return c.X > crystalSearchRadius && c.Y > crystalSearchRadius &&
		int(c.X)+crystalSearchRadius < model.TerrainWidth &&
		int(c.Y)+crystalSearchRadius < model.TerrainHeight &&
		c.CrystalHP > 0 && c.CrystalHP <= math.MaxInt32 &&
		c.MediumHP > 0 && c.MediumHP <= math.MaxInt32 &&
		c.SmallHP > 0 && c.SmallHP <= math.MaxInt32 &&
		c.PulsePower > 0 && c.PulsePower <= 100000 &&
		c.RespawnSeconds >= 60 && c.RespawnSeconds <= 86400 &&
		c.DeadlineSeconds >= 10 && c.DeadlineSeconds <= 3600
}

type crystalPhase uint8

const (
	crystalNone crystalPhase = iota
	crystalLarge
	crystalMedium
	crystalSmall
)

type crystalTile struct{ x, y uint16 }

type crystalDrop struct {
	tile      crystalTile
	remaining int
}

// Only the channel World owns this state. No timers or mob IDs survive restart.
type corruptedCrystal struct {
	config        CorruptedCrystalConfig
	defs          [crystalSmall + 1]*model.NPCDef
	phase         crystalPhase
	live          map[uint16]*Mob
	mediumOrigins []crystalTile
	due           time.Time
	deadline      time.Time
	nextPulse     time.Time
	attackers     map[*Player]*model.Char
	drops         []crystalDrop
	dropRetry     time.Time
}

func WithCorruptedCrystal(config CorruptedCrystalConfig) WorldOption {
	return func(w *World) {
		if config != (CorruptedCrystalConfig{}) {
			w.corruptedCrystal = &corruptedCrystal{config: config}
		}
	}
}

func (w *World) initCorruptedCrystal(now time.Time) error {
	event := w.corruptedCrystal
	if event == nil {
		return nil
	}
	c := event.config
	if !c.valid() {
		return fmt.Errorf("invalid corrupted crystal configuration")
	}
	if _, exists := w.items[crystalPowderItem]; !exists {
		return fmt.Errorf("corrupted crystal reward %d is missing", crystalPowderItem)
	}
	if _, exists := w.skills[crystalThunderSkill]; !exists {
		return fmt.Errorf("corrupted crystal thunder skill %d is missing", crystalThunderSkill)
	}
	if !w.crystalTerrain(c.X, c.Y) {
		return fmt.Errorf("corrupted crystal origin (%d,%d) is outside valid terrain", c.X, c.Y)
	}
	for _, tier := range []struct {
		phase      crystalPhase
		appearance uint16
		hp         uint32
		name       string
	}{
		{crystalLarge, CrystalAppearance, c.CrystalHP, "Corrupt Crystal"},
		{crystalMedium, crystalMediumAppearance, c.MediumHP, "Medium Crystal"},
		{crystalSmall, crystalSmallAppearance, c.SmallHP, "Small Crystal"},
	} {
		if _, exists := w.items[tier.appearance]; exists {
			return fmt.Errorf("corrupted crystal appearance %d conflicts with the item catalog", tier.appearance)
		}
		event.defs[tier.phase] = &model.NPCDef{
			Name: tier.name, Tipo: model.TipoMonstro,
			Equip: model.Equip{Rosto: model.Item{Index: tier.appearance}},
			Score: &model.Score{Version: model.ScoreVersion, Level: 153, MaxHP: tier.hp},
		}
	}
	event.phase = crystalLarge
	w.tickCorruptedCrystal(now)
	return nil
}

func (w *World) crystalTerrain(x, y uint16) bool {
	return w.terrain.Walkable(x, y) && !w.terrain.NewbieZone(x, y) && !w.terrain.GuildArea(x, y)
}

func (w *World) tickCorruptedCrystal(now time.Time) {
	event := w.corruptedCrystal
	if event == nil || event.phase == crystalNone {
		return
	}
	if len(event.drops) > 0 && !now.Before(event.dropRetry) {
		pending := event.drops[:0]
		for _, drop := range event.drops {
			if drop = w.scatterCrystalDrop(drop); drop.remaining > 0 {
				pending = append(pending, drop)
			}
		}
		event.drops = pending
		event.dropRetry = now.Add(crystalRetryDelay)
	}
	if !event.deadline.IsZero() && !now.Before(event.deadline) {
		for _, mob := range event.live {
			if mob.Dead {
				return // Let pending kill persistence complete or restore the mob.
			}
		}
		for _, mob := range event.live {
			for _, player := range w.nearbyWorldPlayers(mob.X, mob.Y, viewHalfX) {
				w.hideMob(player, mob, 0)
			}
			w.removeMobInstance(mob)
		}
		w.resetCorruptedCrystal(now)
		return
	}
	if event.phase == crystalLarge && !now.Before(event.nextPulse) {
		for _, mob := range event.live {
			w.pulseCorruptedCrystal(event, mob, now)
		}
	}
	if len(event.live) > 0 || now.Before(event.due) {
		return
	}
	// Drain the previous cycle before starting another so pending drops stay bounded.
	if event.phase == crystalLarge && len(event.drops) > 0 {
		return
	}
	if !w.spawnCrystalPhase(event) {
		event.due = now.Add(crystalRetryDelay)
		return
	}
	switch event.phase {
	case crystalLarge:
		event.nextPulse = now.Add(crystalPulseInterval)
		event.attackers = make(map[*Player]*model.Char)
		w.announceCrystal(event, "_WG_Crystal_Event_Appeared")
	case crystalMedium:
		w.announceCrystal(event, "_WG_Crystal_Event_MediumWave")
	case crystalSmall:
		w.announceCrystal(event, "_WG_Crystal_Event_SmallWave")
	}
}

// Rotate each offset around the center before trying the next point on a ring.
func (w *World) visitCrystalTiles(center crystalTile, minRadius, maxRadius int, visit func(crystalTile) bool) {
	if minRadius == 0 && w.crystalTerrain(center.x, center.y) && visit(center) {
		return
	}
	for radius := max(1, minRadius); radius <= maxRadius; radius++ {
		for step := 0; step < 2*radius; step++ {
			dx, dy := step-radius, -radius
			for side := 0; side < 4; side++ {
				x, y := int(center.x)+dx, int(center.y)+dy
				dx, dy = -dy, dx
				if x < 0 || y < 0 || x >= model.TerrainWidth || y >= model.TerrainHeight {
					continue
				}
				tile := crystalTile{uint16(x), uint16(y)}
				if w.crystalTerrain(tile.x, tile.y) &&
					w.terrain.HeightCompatible(center.x, center.y, tile.x, tile.y) && visit(tile) {
					return
				}
			}
		}
	}
}

// Register the entire wave before publishing it. Each medium origin supplies
// exactly four small crystals, and failed placement rolls back the whole wave.
func (w *World) spawnCrystalPhase(event *corruptedCrystal) bool {
	origins := []crystalTile{{event.config.X, event.config.Y}}
	count, minRadius, maxRadius := 1, 0, 0
	switch event.phase {
	case crystalMedium:
		count, minRadius, maxRadius = crystalMediumCount, 2, crystalSearchRadius
	case crystalSmall:
		origins = event.mediumOrigins
		count, minRadius, maxRadius = crystalSmallCount, 1, crystalSearchRadius
	}
	placed := make([]*Mob, 0, len(origins)*count)
	def := event.defs[event.phase]
	for _, origin := range origins {
		target := len(placed) + count
		w.visitCrystalTiles(origin, minRadius, maxRadius, func(tile crystalTile) bool {
			if w.positionOccupiedExcept(tile.x, tile.y, nil, nil) {
				return false
			}
			id := w.allocMobID()
			if id == 0 {
				return true
			}
			mob := &Mob{ID: id, Def: def, X: tile.x, Y: tile.y, HP: def.Score.MaxHP,
				GenerIndex: -1, crystalRole: event.phase}
			mob.Segments[0].X, mob.Segments[0].Y = tile.x, tile.y
			w.appendMobInstance(mob)
			w.registerMobSpatial(mob)
			placed = append(placed, mob)
			return len(placed) == target
		})
		if len(placed) != target {
			for _, mob := range placed {
				w.removeMobInstance(mob)
			}
			return false
		}
	}
	event.live = make(map[uint16]*Mob, len(placed))
	for _, mob := range placed {
		event.live[mob.ID] = mob
		w.publishRegisteredMobSpawn(mob)
	}
	return true
}

func (w *World) scatterCrystalDrop(drop crystalDrop) crystalDrop {
	occupied := make(map[crystalTile]bool)
	for _, item := range w.nearbyGroundItems(drop.tile.x, drop.tile.y, crystalSearchRadius) {
		occupied[crystalTile{item.X, item.Y}] = true
	}
	w.visitCrystalTiles(drop.tile, 1, crystalSearchRadius, func(tile crystalTile) bool {
		if occupied[tile] {
			return false
		}
		if !w.spawnGroundReward(tile.x, tile.y, crystalPowderItem) {
			return true
		}
		drop.remaining--
		return drop.remaining == 0
	})
	return drop
}

func (w *World) onCrystalMobKilled(mob *Mob, rewardCommitted bool, now time.Time) {
	event := w.corruptedCrystal
	if event == nil || mob.crystalRole == crystalNone || event.live[mob.ID] != mob {
		return
	}
	delete(event.live, mob.ID)
	tile := crystalTile{mob.X, mob.Y}
	if rewardCommitted {
		count := [...]int{crystalLarge: 20, crystalMedium: 4, crystalSmall: 1}[mob.crystalRole]
		drop := w.scatterCrystalDrop(crystalDrop{tile, count})
		if drop.remaining > 0 {
			event.drops = append(event.drops, drop)
			event.dropRetry = now.Add(crystalRetryDelay)
		}
	}
	if mob.crystalRole == crystalMedium {
		event.mediumOrigins = append(event.mediumOrigins, tile)
	}
	if len(event.live) > 0 {
		return
	}
	switch mob.crystalRole {
	case crystalLarge:
		event.attackers = nil
		event.phase = crystalMedium
	case crystalMedium:
		event.phase = crystalSmall
	case crystalSmall:
		w.announceCrystal(event, "_WG_Crystal_Event_Victory")
		w.resetCorruptedCrystal(now)
		return
	}
	event.deadline = now.Add(time.Duration(event.config.DeadlineSeconds) * time.Second)
	event.due = now.Add(crystalShatterDelay)
}

func (w *World) resetCorruptedCrystal(now time.Time) {
	event := w.corruptedCrystal
	event.live = nil
	event.attackers = nil
	event.mediumOrigins = nil
	event.phase = crystalLarge
	event.due = now.Add(time.Duration(event.config.RespawnSeconds) * time.Second)
	event.deadline = time.Time{}
}

func (w *World) recordCrystalAttack(mob *Mob, player *Player, damage uint32) {
	if mob.crystalRole == crystalLarge && player != nil && damage != 0 {
		event := w.corruptedCrystal
		if event != nil && event.live[mob.ID] == mob {
			event.attackers[player] = player.Char
		}
	}
}

func (w *World) pulseCorruptedCrystal(event *corruptedCrystal, mob *Mob, now time.Time) {
	event.nextPulse = now.Add(crystalPulseInterval)
	if mob.Dead || mob.HP == 0 {
		return
	}
	thunder := w.skills[crystalThunderSkill]
	for player, char := range event.attackers {
		if !player.InWorld || player.Char != char || w.playerByID(player.ID) != player {
			delete(event.attackers, player)
			continue
		}
		if !validMobTargetAt(player, now) || w.criticalPlayerReserved(player) ||
			w.gameplaySpaceForPlayer(player) != mobGameplaySpace(mob) ||
			chebyshev(mob.X, mob.Y, player.X, player.Y) > crystalSearchRadius {
			continue
		}
		damage := w.skillFinalDamage(int(event.config.PulsePower), playerDefense(char), 0)
		damage = applySkillResistance(damage, thunder.InstanceType, playerElementalResists(char), false)
		w.applyMobDamageToPlayer(mob, player, uint32(max(1, damage)), now, func(applied uint32) []byte {
			return wire.SkillHits(mob.ID, mob.X, mob.Y, player.X, player.Y, 0, 0,
				crystalThunderSkill, crystalPulseMotion, 0, 1,
				[]wire.SkillTarget{{ID: player.ID, Damage: applied, MaxHP: playerMaxHP(char)}})
		})
	}
}

func (w *World) announceCrystal(event *corruptedCrystal, key string) {
	for _, player := range w.nearbyWorldPlayers(event.config.X, event.config.Y, bossAnnounceRadius) {
		if w.gameplaySpaceForPlayer(player) == "" {
			player.Session.Send(wire.MessagePanel(w.msg(key)))
		}
	}
}

func (w *World) crystalMobName(mob *Mob) string {
	switch mob.crystalRole {
	case crystalLarge:
		return w.msg("_WG_Crystal_Event_Object_Name")
	case crystalMedium:
		return w.msg("_WG_Crystal_Event_Medium_Name")
	case crystalSmall:
		return w.msg("_WG_Crystal_Event_Small_Name")
	default:
		return mob.Def.VisibleName()
	}
}
