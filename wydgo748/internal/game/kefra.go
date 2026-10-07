package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
)

// Kefra lifecycle, ported from the W2PP TMSrv (MobKilled.cpp, GetFunc.cpp,
// ProcessSecMinTimer.cpp). Killing the Kefra boss marks him defeated, credits
// the killer's guild with 100 fame and announces it. While he stays defeated
// the Hall of Kefra portal leads to Uxmal (W2PP: xv 2364, yv 3924 -> 3250,
// 1703), and his generator does not bring him back. Every Tuesday at 12:00
// (war timezone) he returns and the passage closes again.
//
// His guards, Lich Crunt and Lich Vatama (W2PP KEFRA_MOB_INITIAL..END; names
// from the 7.48 UI/PotalPos.txt), live only while he lives: they respawn at
// once when killed, leave the world when he dies, and return with him.
const (
	kefraBossName   = "Kefra"
	kefraGuildFame  = uint32(100)
	kefraGateX      = 2364
	kefraGateY      = 3924
	kefraResetDay   = time.Tuesday
	kefraResetHour  = 12
	kefraGateClosed = "Kefra still guards the passage."
)

var kefraGuardNames = map[string]bool{"Lich_Crunt": true, "Lich_Vatama": true}

func isKefraGuardDef(def *model.NPCDef) bool {
	return def != nil && kefraGuardNames[def.Name]
}

func isKefraGuard(m *Mob) bool {
	return m != nil && m.InstanceID == "" && isKefraGuardDef(m.Def)
}

func isKefraBoss(m *Mob) bool {
	return m != nil && m.Def != nil && m.Def.Name == kefraBossName && m.InstanceID == ""
}

func (w *World) kefraDefeated() bool {
	return w.guilds != nil && w.guilds.Wars.Kefra.Defeated
}

func isKefraGate(portal model.Teleport) bool {
	return samePortalAttributeBlock(portal.SourceX, portal.SourceY, kefraGateX, kefraGateY)
}

// generatorBlockedByKefra keeps a defeated Kefra, and his guards, from
// respawning through their NPCGener entries until the weekly reset.
func (w *World) generatorBlockedByKefra(g *generState) bool {
	if g == nil || g.leader == nil || !w.kefraDefeated() {
		return false
	}
	return g.leader.Name == kefraBossName || isKefraGuardDef(g.leader)
}

// removeKefraGuards takes the guards out of the world when Kefra dies. They
// leave without a death: no reward, drop or kill credit.
func (w *World) removeKefraGuards() {
	guards := make([]*Mob, 0, len(kefraGuardNames)*2)
	for _, m := range w.mobs {
		if isKefraGuard(m) && !m.Dead {
			guards = append(guards, m)
		}
	}
	for _, m := range guards {
		m.Dead = true
		w.publishMobRemoval(m)
		if g := w.mobGenerator(m); g != nil && g.current > 0 {
			g.current--
		}
		w.removeMobInstance(m)
	}
}

// respawnKefraGuard brings a killed guard back at once while Kefra lives, as
// W2PP GenerateMob does on its death. The generator counter is already lower.
func (w *World) respawnKefraGuard(m *Mob) {
	if !isKefraGuard(m) || w.kefraDefeated() {
		return
	}
	if g := w.mobGenerator(m); g != nil {
		w.spawnGroup(g)
	}
}

func (w *World) kefraLocation() *time.Location {
	if w.warLocation != nil {
		return w.warLocation
	}
	if w.warConfig.Timezone != "" {
		if location, err := time.LoadLocation(w.warConfig.Timezone); err == nil {
			return location
		}
	}
	return time.Local
}

// kefraResetAt is the first Tuesday 12:00 strictly after the defeat.
func kefraResetAt(defeatedAt time.Time, location *time.Location) time.Time {
	local := defeatedAt.In(location)
	reset := time.Date(local.Year(), local.Month(), local.Day(), kefraResetHour, 0, 0, 0, location)
	reset = reset.AddDate(0, 0, (int(kefraResetDay)-int(reset.Weekday())+7)%7)
	if !reset.After(local) {
		reset = reset.AddDate(0, 0, 7)
	}
	return reset
}

// stageKefraDefeat writes Kefra's defeat, and the killer guild's fame, into the
// in-memory guild state only. The caller persists it in the same transaction as
// the kill and calls undo if that fails, so memory never holds a defeat (or a
// fame grant) that a restart would not load. A guild already at the fame limit
// still gets the defeat, without fame.
func (w *World) stageKefraDefeat(killer *Player) (guild *model.Guild, undo func()) {
	guilds, wars := w.snapshotGuilds(), w.guilds.Wars.Clone()
	undo = func() {
		w.restoreGuilds(guilds)
		w.guilds.Wars = wars
	}
	state := model.KefraState{Defeated: true, DefeatedAt: w.now()}
	if killer != nil && killer.Char != nil {
		guild, _ = w.guildOf(killer.Char)
	}
	if guild != nil {
		state.Guild = guild.ID
		if err := guild.AddFame(kefraGuildFame); err != nil {
			log.Printf("kefra: guild %d fame not credited: %v", guild.ID, err)
		}
	}
	w.guilds.Wars.Kefra = state
	return guild, undo
}

// stageKefraKill stages the defeat for the first not-yet-defeated Kefra among
// the kill plans. It returns nil when no plan kills Kefra.
func (w *World) stageKefraKill(plans []*killRewardPlan) (*killRewardPlan, *model.Guild, func()) {
	if w.guilds == nil || w.kefraDefeated() {
		return nil, nil, nil
	}
	for _, plan := range plans {
		if plan != nil && isKefraBoss(plan.mob) {
			guild, undo := w.stageKefraDefeat(plan.killer)
			return plan, guild, undo
		}
	}
	return nil, nil, nil
}

// onKefraKilled handles a Kefra death without a reward commit (no killer). The
// defeat only exists once warCommit has persisted it; otherwise Kefra counts as
// alive, in memory as after a restart, and his generator brings him back.
func (w *World) onKefraKilled(killer *Player, m *Mob) {
	if !isKefraBoss(m) || w.guilds == nil || w.kefraDefeated() {
		return
	}
	var guild *model.Guild
	err := w.warCommit(func() error {
		guild, _ = w.stageKefraDefeat(killer)
		return nil
	})
	if err != nil {
		log.Printf("CRITICAL: Kefra defeat not persisted (%v); Kefra stays undefeated", err)
		return
	}
	w.announceKefraDefeat(killer, guild)
}

// announceKefraDefeat runs only after the defeat is durable.
func (w *World) announceKefraDefeat(killer *Player, guild *model.Guild) {
	message := "Kefra has been defeated. The way to Uxmal is open."
	if guild != nil {
		message = fmt.Sprintf("Guild %s has defeated Kefra. The way to Uxmal is open.", guild.Name)
	}
	w.removeKefraGuards()
	w.warNotice(message)
	w.flushWarNotices(w.now())
	log.Printf("kefra: defeated by %q, guild=%d", playerName(killer), w.guilds.Wars.Kefra.Guild)
}

// tickKefra restores Kefra at the weekly reset, also after a restart that
// missed it. His generator spawns him on its next cycle.
func (w *World) tickKefra(now time.Time) {
	if !w.kefraDefeated() {
		return
	}
	if now.Before(kefraResetAt(w.guilds.Wars.Kefra.DefeatedAt, w.kefraLocation())) {
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			guilds.Wars.Kefra = model.KefraState{}
			return nil
		}, func() { log.Printf("kefra: weekly reset, the boss returns") }); err != nil {
			log.Printf("kefra weekly reset admission: %v", err)
		}
		return
	}
	previous := w.guilds.Wars.Kefra
	if err := w.warCommit(func() error {
		w.guilds.Wars.Kefra = model.KefraState{}
		return nil
	}); err != nil {
		log.Printf("kefra weekly reset not persisted (%v); retrying", err)
		w.guilds.Wars.Kefra = previous
		return
	}
	log.Printf("kefra: weekly reset, the boss returns")
}

func playerName(p *Player) string {
	if p == nil || p.Char == nil {
		return ""
	}
	return p.Char.Name
}
