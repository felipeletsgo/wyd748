package game

import (
	"fmt"
	"log"
	"sort"
	"strconv"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// MODERNIZACAO_COMPATIVEL: server-owned W2PP rules, existing chat/teleport/combat
// wire only. No client-supplied winner, score, fame, damage or calendar is used.
type warRect struct{ x1, y1, x2, y2 uint16 }

func (r warRect) contains(x, y uint16) bool { return x >= r.x1 && x <= r.x2 && y >= r.y1 && y <= r.y2 }

type cityWarZone struct {
	name                                             string
	city, arena                                      warRect
	exitX, exitY, defendX, defendY, attackX, attackY uint16
}

var cityWarZones = [...]cityWarZone{
	{"Armia", warRect{2052, 2052, 2171, 2163}, warRect{197, 213, 238, 230}, 2091, 2100, 205, 220, 228, 220},
	{"Azran", warRect{2432, 1672, 2675, 1767}, warRect{197, 149, 238, 166}, 2494, 1707, 205, 157, 228, 157},
	{"Erion", warRect{2448, 1966, 2476, 2024}, warRect{141, 213, 182, 230}, 2453, 2000, 146, 220, 173, 220},
	{"Nippleheim", warRect{3605, 3090, 3690, 3260}, warRect{141, 149, 182, 166}, 3652, 3122, 146, 157, 173, 157},
}
var towerWarArena = warRect{2476, 1852, 2545, 1918}

// DestinyServer NPCGener.txt: EndlessTower and Torre_de_Thor share this
// arena anchor. W2PP uses a generator, not the arena midpoint.
const towerWarSpawnX, towerWarSpawnY uint16 = 2507, 1873

type cityFighter struct {
	city  int
	guild uint16
}

func WithGuildWars(config model.GuildWarConfig) WorldOption {
	return func(w *World) { w.warConfig = config }
}

func (w *World) warCommit(change func() error) error {
	if w.criticalGuildReserved() {
		return errCriticalPersistenceBusy
	}
	guilds, wars := w.snapshotGuilds(), w.guilds.Wars.Clone()
	if err := change(); err != nil {
		w.restoreGuilds(guilds)
		w.guilds.Wars = wars
		return err
	}
	if err := w.saveGuildState(); err != nil {
		w.restoreGuilds(guilds)
		w.guilds.Wars = wars
		return err
	}
	return nil
}

func (w *World) warNotice(message string) {
	// Native MessagePanel holds 95 bytes and replaces the previous four-second
	// notice. Queue the four city results so they do not overwrite one another.
	log.Printf("guild wars: %s", message)
	for _, part := range splitWarNotice(message) {
		// Keep every result in chat as well, even when another game notice
		// replaces the panel or a burst fills its bounded presentation queue.
		w.broadcast(func() []byte { return wire.MessageWhisper(0, "[WAR]", part, 7) })
		if len(w.warNotices) < 64 {
			w.warNotices = append(w.warNotices, part)
		}
	}
}

func splitWarNotice(message string) []string {
	var parts []string
	for len(message) > 95 {
		cut := strings.LastIndexByte(message[:95], ' ')
		if cut <= 0 {
			cut = 95
		}
		parts = append(parts, message[:cut])
		message = strings.TrimSpace(message[cut:])
	}
	if message != "" {
		parts = append(parts, message)
	}
	return parts
}

func (w *World) flushWarNotices(now time.Time) {
	if len(w.warNotices) == 0 || now.Before(w.warNoticeNext) {
		return
	}
	message := w.warNotices[0]
	w.warNotices[0] = ""
	w.warNotices = w.warNotices[1:]
	w.warNoticeNext = now.Add(5 * time.Second)
	w.broadcast(func() []byte { return wire.MessagePanel(message) })
}

func (w *World) warGuildName(id uint16) string {
	if g := w.guilds.FindByID(id); g != nil {
		return g.Name
	}
	return "none"
}

// Persist before publication: a restart does not replay old countdowns. Only
// the currently relevant window is announced, never a backlog of warnings.
func (w *World) warCountdown(now, deadline time.Time, mask *uint16, bit uint16, minutes int, message string) error {
	remaining := deadline.Sub(now)
	if remaining <= 0 || remaining > time.Duration(minutes)*time.Minute || *mask&bit != 0 {
		return nil
	}
	if minutes > 1 && remaining <= time.Minute {
		return nil
	}
	if w.criticalWrites != nil {
		tower := mask == &w.guilds.Wars.Tower.Notices
		if !tower && mask != &w.guilds.Wars.Cities.Notices {
			return errCriticalPersistenceIdentity
		}
		return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			if tower {
				guilds.Wars.Tower.Notices |= bit
			} else {
				guilds.Wars.Cities.Notices |= bit
			}
			return nil
		}, func() { w.warNotice(message) })
	}
	if err := w.warCommit(func() error { *mask |= bit; return nil }); err != nil {
		return err
	}
	w.warNotice(message)
	return nil
}

func (w *World) tickGuildWars(now time.Time) {
	if !w.warConfig.Enabled || w.guilds == nil || w.criticalGuildReserved() || now.Before(w.warNextTick) {
		return
	}
	w.warNextTick = now.Add(time.Second)
	defer w.flushWarNotices(now)
	if w.warLocation == nil {
		var err error
		w.warLocation, err = time.LoadLocation(w.warConfig.Timezone)
		if err != nil {
			log.Printf("guild wars timezone: %v", err)
			return
		}
	}
	local := now.In(w.warLocation)
	if pending, err := w.tickGuildWarEconomy(local); pending || err != nil {
		if err != nil {
			log.Printf("guild war economy: %v", err)
		}
		return
	}
	// An interrupted city fight cannot be judged from the set of players that
	// happened to reconnect first. Cancel/refund its applications atomically.
	if !w.warInitialized {
		phase := w.guilds.Wars.Cities.Phase
		if phase == "preparing" || phase == "battle" {
			if err := w.cancelCityWar(); err != nil {
				log.Printf("recover city war: %v", err)
				return
			}
			if w.criticalGuildReserved() {
				return
			}
		}
		w.warInitialized = true
	}
	if err := w.tickTowerWar(local); err != nil {
		log.Printf("tower war: %v", err)
	}
	if w.criticalGuildReserved() {
		return
	}
	if err := w.tickCityWar(local); err != nil {
		log.Printf("city war: %v", err)
	}
	if w.criticalGuildReserved() {
		return
	}
	w.cleanCityArena()
	if err := w.deliverWarMedal(); err != nil {
		log.Printf("war medal delivery: %v", err)
	}
}

func (w *World) tickTowerWar(now time.Time) error {
	if w.criticalWrites != nil {
		return w.tickCriticalTowerWar(now)
	}
	s := &w.guilds.Wars.Tower
	if s.Day != "" && !s.Finished && !now.Before(s.EndsAt) {
		if err := w.warCommit(func() error {
			if err := w.guilds.ResolveTowerWar(); err != nil {
				return err
			}
			w.addTowerMedals(w.guilds, "finish")
			return nil
		}); err != nil {
			return err
		}
		w.publishTowerFinish()
	}
	day := now.Format("2006-01-02")
	start := time.Date(now.Year(), now.Month(), now.Day(), int(w.warConfig.TowerHour), 0, 0, 0, now.Location())
	if now.Weekday() >= time.Monday && now.Weekday() <= time.Friday &&
		!now.Before(start.Add(-5*time.Minute)) && now.Before(start.Add(time.Hour)) && s.Day != day {
		if err := w.warCommit(func() error {
			w.startTowerRound(day, start)
			return nil
		}); err != nil {
			return err
		}
	}
	if s.Day == day && !s.Finished {
		battle := s.EndsAt.Add(-time.Hour)
		for _, warning := range []struct {
			end     time.Time
			bit     uint16
			minutes int
			text    string
		}{
			{battle, 1, 5, "Tower war starts within 5 minutes! Enter with /tower in Erion."},
			{battle, 2, 1, "Tower war starts in 1 minute!"},
			{s.EndsAt, 4, 5, "Tower war ends in 5 minutes! Guild fame is awarded only on a kingdom change."},
			{s.EndsAt, 8, 1, "Tower war ends in 1 minute!"},
		} {
			if err := w.warCountdown(now, warning.end, &s.Notices, warning.bit, warning.minutes, warning.text); err != nil {
				return err
			}
		}
	}
	if w.towerBattleActive(now) && w.warTower() == nil {
		if err := w.spawnWarTower(); err != nil {
			return err
		}
	}
	if w.towerBattleActive(now) && !s.Started {
		if err := w.warCommit(func() error { s.Started = true; return nil }); err != nil {
			w.removeWarTower()
			return err
		}
		w.ejectTowerOpponents(w.guilds.TowerKingdom())
		w.warNotice("Tower war started! Use /tower in Erion. Duration: 60 minutes.")
	}
	return nil
}

func (w *World) towerBattleActive(now time.Time) bool {
	if !w.warConfig.Enabled || w.guilds == nil {
		return false
	}
	s := w.guilds.Wars.Tower
	return s.Day != "" && !s.Finished && !now.Before(s.EndsAt.Add(-time.Hour)) && now.Before(s.EndsAt)
}

func (w *World) warTower() *Mob {
	for _, m := range w.mobs {
		if m != nil && m.GuildWarTower && !m.Dead {
			return m
		}
	}
	return nil
}

func (w *World) spawnWarTower() error {
	var def *model.NPCDef
	for i := range w.npcs {
		if strings.EqualFold(w.npcs[i].Name, "Torre") {
			def = &w.npcs[i]
			break
		}
	}
	if def == nil || def.Score == nil {
		return fmt.Errorf("Torre NPC template unavailable")
	}
	// Retain the installed 7.48 visual and HP/defense, but not its legacy
	// merchant flag, AI, loot, exp or corrupted mastery/critical fields.
	copyDef, score := *def, *def.Score
	copyDef.Score, copyDef.Tipo, copyDef.Carry, copyDef.Gold, copyDef.ExpReward = &score, "monstro", nil, 0, 0
	score.Merchant, score.AttackRun, score.Attack, score.Critical = 0, 0, 0, 0
	x, y := w.findFreePosition(towerWarSpawnX, towerWarSpawnY, 3)
	if !w.terrain.Walkable(x, y) {
		return fmt.Errorf("tower spawn blocked")
	}
	id := w.allocMobID()
	if id == 0 {
		return fmt.Errorf("no mob ID for war tower")
	}
	m := &Mob{ID: id, Def: &copyDef, X: x, Y: y, HP: score.MaxHP, GenerIndex: -1, GuildWarTower: true}
	m.Segments[0].X, m.Segments[0].Y = x, y
	w.appendMobInstance(m)
	w.registerMobSpatial(m)
	w.publishRegisteredMobSpawn(m)
	return nil
}

func (w *World) removeWarTower() {
	if m := w.warTower(); m != nil {
		w.killMobWithoutPlayer(m)
	}
}

func (w *World) towerTargetAllowed(p *Player, m *Mob) bool {
	if m == nil || !m.GuildWarTower {
		return true
	}
	if !validPlayerMobParticipant(p) || w.gameplaySpaceForPlayer(p) != "" || mobGameplaySpace(m) != "" ||
		!w.towerBattleActive(w.now()) || !towerWarArena.contains(p.X, p.Y) {
		return false
	}
	g, _ := w.guildOf(p.Char)
	return g != nil && (g.Kingdom == 7 || g.Kingdom == 8) && g.Kingdom != w.guilds.TowerKingdom()
}

func (w *World) captureWarTower(p *Player, m *Mob, applied uint32) {
	if w.criticalGuildMobReserved(m) {
		return
	}
	if !w.towerTargetAllowed(p, m) {
		m.HP = minU32(m.Def.Score.MaxHP, max(1, applied))
		w.sendToMobViewProtocol(m, func(_ *Player) []byte { return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, 0, 0) })
		return
	}
	g, _ := w.guildOf(p.Char)
	owner := g.ID
	if w.criticalWrites != nil {
		if err := w.queueTowerCapture(p, m, applied); err != nil {
			m.HP = minU32(m.Def.Score.MaxHP, max(1, applied))
			log.Printf("tower capture admission: %v", err)
			w.sendToMobViewProtocol(m, func(_ *Player) []byte { return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, 0, 0) })
		}
		return
	}
	err := w.warCommit(func() error {
		if err := w.guilds.CaptureTower(owner); err != nil {
			return err
		}
		w.addTowerMedals(w.guilds, "capture")
		return nil
	})
	if err != nil {
		m.HP = minU32(m.Def.Score.MaxHP, max(1, applied))
		log.Printf("tower capture rollback: %v", err)
	} else {
		m.HP = m.Def.Score.MaxHP
		m.Affects = [16]model.Affect{}
		m.TargetID = 0
		w.warNotice(fmt.Sprintf("%s of guild %s brought the tower down! New defender: %s.", p.Char.Name, g.Name, g.Name))
		w.ejectTowerOpponents(w.guilds.TowerKingdom())
	}
	w.sendToMobViewProtocol(m, func(_ *Player) []byte { return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, 0, 0) })
}

func (w *World) ejectTowerArena() {
	for _, p := range w.players {
		if p != nil && p.InWorld && p.Char != nil && w.gameplaySpaceForPlayer(p) == "" && towerWarArena.contains(p.X, p.Y) {
			w.requestPlayerTeleport(p, 2453, 2000, nil, nil)
		}
	}
}

func (w *World) cancelCityWar() error {
	if w.criticalWrites != nil {
		return w.queueCityWarCancel()
	}
	err := w.warCommit(w.guilds.CancelCityBids)
	if err == nil {
		w.cityWarRunning = false
		for p := range w.cityFighters {
			p.Session.Send(wire.WarInfo())
		}
		w.cityFighters = nil
		w.cityWarScores = [4]map[uint16]uint64{}
		w.warNotice("City war interrupted; owners kept. Only bids from the current fame week can be refunded.")
	}
	return err
}

func (w *World) tickCityWar(now time.Time) error {
	if w.criticalWrites != nil {
		return w.tickCriticalCityWar(now)
	}
	c := &w.guilds.Wars.Cities
	if c.Day != "" && c.Phase != "finished" && !now.Before(c.StartsAt.Add(31*time.Minute)) {
		if !w.cityWarRunning {
			return w.cancelCityWar()
		}
		if err := w.finishCityWar(); err != nil {
			return err
		}
	}
	// Open only on Saturday. Starting a server on Sunday never opens late bids.
	if now.Weekday() == time.Saturday {
		sunday := now.AddDate(0, 0, 1)
		day := sunday.Format("2006-01-02")
		if c.Day != day {
			err := w.warCommit(func() error {
				territories := c.Territories
				for i := range territories {
					territories[i].Challenger, territories[i].Bid = 0, 0
				}
				*c = model.CityWarState{Day: day, Phase: "registration", FameBids: true, Territories: territories,
					StartsAt: time.Date(sunday.Year(), sunday.Month(), sunday.Day(), int(w.warConfig.CityHour), 0, 0, 0, now.Location())}
				return nil
			})
			if err == nil {
				w.warNotice("Saturday bids open until 23:59:59! /citywar <1..4> <total guild fame>. No entry fee.")
			}
			return err
		}
	}
	if c.Day == "" || c.Phase == "finished" {
		return nil
	}
	localStart := c.StartsAt.In(now.Location())
	registrationEnd := time.Date(localStart.Year(), localStart.Month(), localStart.Day(), 0, 0, 0, 0, now.Location())
	if c.Phase == "registration" && !now.Before(registrationEnd) {
		if err := w.warCommit(func() error { c.Phase = "waiting"; return nil }); err != nil {
			return err
		}
		w.warNotice("City war applications closed. Check owners and challengers with /citywar.")
	}
	for _, warning := range []struct {
		end     time.Time
		bit     uint16
		minutes int
		text    string
	}{
		{c.StartsAt, 1, 5, "City war starts in 5 minutes! Gather in the contested city; summons 3 minutes after the start."},
		{c.StartsAt, 2, 1, "City war starts in 1 minute! Limit: 26 per guild. Erion: mortals only."},
		{c.StartsAt.Add(6 * time.Minute), 8, 1, "City war combat opens in 1 minute!"},
		{c.StartsAt.Add(31 * time.Minute), 16, 5, "City wars end in 5 minutes! The surviving forces win."},
		{c.StartsAt.Add(31 * time.Minute), 32, 1, "City wars end in 1 minute!"},
	} {
		if err := w.warCountdown(now, warning.end, &c.Notices, warning.bit, warning.minutes, warning.text); err != nil {
			return err
		}
	}
	if !now.Before(c.StartsAt) && c.Notices&4 == 0 {
		if err := w.warCommit(func() error { c.Notices |= 4; return nil }); err != nil {
			return err
		}
		w.warNotice("City war started! Summons at +3 minutes; combat at +6; result at +31.")
	}
	if c.Phase == "waiting" && !now.Before(c.StartsAt.Add(3*time.Minute)) {
		// Do not summon late arrivals into a shortened battle after downtime.
		if !now.Before(c.StartsAt.Add(6 * time.Minute)) {
			return w.cancelCityWar()
		}
		if err := w.warCommit(func() error { c.Phase = "preparing"; return nil }); err != nil {
			return err
		}
		w.cityWarRunning = true
		w.summonCityFighters()
		w.warNotice("Guilds summoned! 3 minutes of preparation; combat still locked.")
	}
	if c.Phase == "preparing" && !now.Before(c.StartsAt.Add(6*time.Minute)) {
		if err := w.warCommit(func() error { c.Phase = "battle"; return nil }); err != nil {
			return err
		}
		w.warNotice("Combat open in the four cities! Duration: 25 minutes; a tie favors the defender.")
		for p, f := range w.cityFighters {
			t := c.Territories[f.city]
			enemy := t.Challenger
			if f.guild == t.Challenger {
				enemy = t.Owner
			}
			p.Session.Send(wire.GuildWarInfo(enemy))
		}
	}
	return nil
}

func (w *World) summonCityFighters() {
	w.cityFighters = make(map[*Player]cityFighter)
	players := make([]*Player, 0, len(w.players))
	for _, p := range w.players {
		if validPlayerMobParticipant(p) && p.GhostShop == nil && w.gameplaySpaceForPlayer(p) == "" {
			players = append(players, p)
		}
	}
	sort.Slice(players, func(i, j int) bool {
		return strings.ToLower(players[i].Char.Name) < strings.ToLower(players[j].Char.Name)
	})
	for city, z := range cityWarZones {
		t := w.guilds.Wars.Cities.Territories[city]
		if t.Challenger == 0 {
			continue
		}
		counts := map[uint16]int{}
		day := w.guilds.Wars.Cities.Day
		for _, p := range players {
			g, _ := w.guildOf(p.Char)
			if g == nil || g.ID != t.Owner && g.ID != t.Challenger || counts[g.ID] >= 26 || !z.city.contains(p.X, p.Y) ||
				city == 2 && !warMortal(p.Char) {
				continue
			}
			x, y := z.defendX, z.defendY
			if g.ID == t.Challenger {
				x, y = z.attackX, z.attackY
			}
			if w.criticalWrites == nil {
				w.cancelTrade(p, "city war")
			}
			fighter := cityFighter{city: city, guild: g.ID}
			if w.requestPlayerTeleport(p, x, y, func() {
				// A calendar transition may end the war while SQL is in flight.
				// The next arena cleanup ejects late arrivals, never enrolls them.
				if w.guilds != nil && w.cityWarRunning && w.guilds.Wars.Cities.Day == day &&
					w.guilds.Wars.Cities.Territories[city].Owner == t.Owner &&
					w.guilds.Wars.Cities.Territories[city].Challenger == t.Challenger && w.cityFighterValid(p, fighter) {
					w.cityFighters[p] = fighter
				}
			}, nil) {
				counts[g.ID]++
			}
		}
	}
}

func warMortal(ch *model.Char) bool {
	return ch.Evolution == "" || strings.EqualFold(ch.Evolution, "mortal")
}

func (w *World) cityFighterValid(p *Player, f cityFighter) bool {
	if !validPlayerMobParticipant(p) || w.players[p.Session] != p || !cityWarZones[f.city].arena.contains(p.X, p.Y) {
		return false
	}
	g, _ := w.guildOf(p.Char)
	return g != nil && g.ID == f.guild && w.gameplaySpaceForPlayer(p) == ""
}

func (w *World) finishCityWar() error {
	if w.criticalGuildReserved() {
		return errCriticalPersistenceBusy
	}
	if w.guilds.Wars.Cities.Phase == "finished" {
		return nil
	}
	if w.cityWarScores[0] == nil {
		for i := range w.cityWarScores {
			w.cityWarScores[i] = make(map[uint16]uint64)
		}
		for p, f := range w.cityFighters {
			if !w.cityFighterValid(p, f) {
				continue
			}
			score := uint64(playerLevel(p.Char))
			if !warMortal(p.Char) {
				score += 399
			}
			w.cityWarScores[f.city][f.guild] += score
		}
	}
	scores := w.cityWarScores
	previous := w.guilds.Wars.Cities.Territories
	change := func(guilds *model.GuildRegistry) error {
		return guilds.ResolveCityWars(scores)
	}
	publish := func() {
		w.cityWarRunning = false
		for p := range w.cityFighters {
			p.Session.Send(wire.WarInfo())
		}
		w.cityFighters = nil
		w.cityWarScores = [4]map[uint16]uint64{}
		for i, z := range cityWarZones {
			t := w.guilds.Wars.Cities.Territories[i]
			w.warNotice(fmt.Sprintf("%s: winner %s. Points defense %d x attack %d. Ownership saved; defenses: %d.", z.name, w.warGuildName(t.Owner), scores[i][previous[i].Owner], scores[i][previous[i].Challenger], t.Victories))
		}
		w.cleanCityArena()
	}
	if w.criticalWrites != nil {
		return w.queueScheduledGuildChange(change, publish)
	}
	if err := w.warCommit(func() error { return change(w.guilds) }); err != nil {
		return err
	}
	publish()
	return nil
}

func (w *World) cleanCityArena() {
	for p, f := range w.cityFighters {
		if w.criticalPlayerReserved(p) {
			continue
		}
		if !w.cityFighterValid(p, f) {
			p.Session.Send(wire.WarInfo())
			delete(w.cityFighters, p)
		}
	}
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || w.criticalPlayerReserved(p) || w.gameplaySpaceForPlayer(p) != "" {
			continue
		}
		for i, z := range cityWarZones {
			if !z.arena.contains(p.X, p.Y) {
				continue
			}
			f, ok := w.cityFighters[p]
			if !w.cityWarRunning || !ok || f.city != i {
				w.requestPlayerTeleport(p, z.exitX, z.exitY, nil, nil)
			}
		}
	}
}

// handled=true makes war areas fail closed even with PK mode enabled.
func (w *World) guildWarPvP(a, b *Player) (allowed, handled bool) {
	if !w.warConfig.Enabled || w.guilds == nil || a == nil || b == nil || a.Char == nil || b.Char == nil {
		return false, false
	}
	aSpace, bSpace := w.gameplaySpaceForPlayer(a), w.gameplaySpaceForPlayer(b)
	if aSpace != "" || bSpace != "" {
		// Public wars neither override a private runtime's combat policy nor
		// authorize attacks between different gameplay spaces.
		return false, aSpace != bSpace
	}
	for i, z := range cityWarZones {
		if z.arena.contains(a.X, a.Y) || z.arena.contains(b.X, b.Y) {
			fa, oka := w.cityFighters[a]
			fb, okb := w.cityFighters[b]
			c := w.guilds.Wars.Cities
			active := c.Phase == "battle" && !w.now().Before(c.StartsAt.Add(6*time.Minute)) && w.now().Before(c.StartsAt.Add(31*time.Minute))
			return active && oka && okb && fa.city == i && fb.city == i && fa.guild != fb.guild &&
				w.cityFighterValid(a, fa) && w.cityFighterValid(b, fb), true
		}
	}
	if towerWarArena.contains(a.X, a.Y) || towerWarArena.contains(b.X, b.Y) {
		ga, _ := w.guildOf(a.Char)
		gb, _ := w.guildOf(b.Char)
		return w.towerBattleActive(w.now()) && towerWarArena.contains(a.X, a.Y) && towerWarArena.contains(b.X, b.Y) &&
			ga != nil && gb != nil && (ga.Kingdom == 7 || ga.Kingdom == 8) && (gb.Kingdom == 7 || gb.Kingdom == 8) && ga.Kingdom != gb.Kingdom, true
	}
	return false, false
}

func (w *World) registerCityWar(p *Player, city int, bid uint32, publications ...func()) error {
	if !w.warConfig.Enabled || w.guilds == nil || !validPlayerMobParticipant(p) || city < 0 || city >= len(cityWarZones) {
		return fmt.Errorf("invalid city challenge")
	}
	now := w.now()
	w.tickGuildWars(now)
	local := w.cityWarLocal(now)
	if local.Weekday() != time.Saturday || w.guilds.Wars.Cities.Phase != "registration" {
		return fmt.Errorf("registration is only open on Saturday")
	}
	g, err := w.requireGuildLeader(p.Char)
	if err != nil {
		return err
	}
	if g.Kingdom != 7 && g.Kingdom != 8 {
		return fmt.Errorf("city wars require a guild of kingdom 7 or 8")
	}
	if p.Char.Citizenship != w.channel || !cityWarZones[city].city.contains(p.X, p.Y) || w.gameplaySpaceForPlayer(p) != "" {
		return fmt.Errorf("leader must be a citizen of this channel and inside the challenged city")
	}
	publish := func() {
		for _, publication := range publications {
			if publication != nil {
				publication()
			}
		}
	}
	if w.criticalWrites != nil {
		proposed := w.criticalGuildSnapshot()
		if err := proposed.Wars.Cities.Register(proposed.FindByID(g.ID), city, bid); err != nil {
			return err
		}
		return w.queueGuildAccountChange(p, []*Player{p}, proposed, nil,
			"The city-war application could not be saved.", nil, publish)
	}
	if err := w.warCommit(func() error { return w.guilds.Wars.Cities.Register(g, city, bid) }); err != nil {
		return err
	}
	publish()
	return nil
}

func (w *World) guildWarCommand(p *Player, command, arg string) {
	if p == nil || p.Session == nil || p.Char == nil {
		return
	}
	send := func(s string) {
		for _, part := range splitWarNotice(s) {
			p.Session.Send(wire.MessageWhisper(0, "[WAR]", part, 7))
		}
	}
	if w.guilds == nil {
		send("Guild registry unavailable.")
		return
	}
	switch command {
	case "guildfame":
		g, _ := w.guildOf(p.Char)
		if g == nil {
			send("You do not belong to a guild.")
			return
		}
		send(fmt.Sprintf("Guild %s: %d fame.", g.Name, g.Fame))
	case "tower":
		g, _ := w.guildOf(p.Char)
		if g == nil || (g.Kingdom != 7 && g.Kingdom != 8) || !validPlayerMobParticipant(p) || !w.towerBattleActive(w.now()) || w.gameplaySpaceForPlayer(p) != "" || !cityWarZones[2].city.contains(p.X, p.Y) {
			send("Tower entry requires a kingdom guild, a living character in Erion and an active tower war.")
			return
		}
		if !w.requestPlayerTeleport(p, 2450, 1855, nil, nil) {
			send("Tower entry failed.")
		}
	case "citywar":
		if arg != "" {
			parts := strings.Fields(arg)
			if len(parts) != 2 {
				send("Use /citywar <1..4> <total guild fame bid>. Only increases are charged.")
				return
			}
			city, err := strconv.Atoi(parts[0])
			bid, bidErr := strconv.ParseUint(parts[1], 10, 32)
			if err != nil || bidErr != nil {
				send("City must be 1..4; the total bid must be an unsigned 32-bit amount.")
				return
			}
			if err := w.registerCityWar(p, city-1, uint32(bid), func() {
				send(fmt.Sprintf("City %d bid saved: %d guild fame. Highest total challenges; ties retain the leader.", city, bid))
			}); err != nil {
				send(err.Error())
				return
			}
			return
		}
		for i, z := range cityWarZones {
			t := w.guilds.Wars.Cities.Territories[i]
			send(fmt.Sprintf("%d %s: owner=%d challenger=%d bid=%d victories=%d", i+1, z.name, t.Owner, t.Challenger, t.Bid, t.Victories))
		}
	}
}

// Virtual server gate prevents crossing the arena center during preparation;
// it also keeps outsiders from walking into a live or abandoned battle room.
func (w *World) guildWarStepAllowed(p *Player, x, y uint16) bool {
	if !w.warConfig.Enabled || w.guilds == nil || w.gameplaySpaceForPlayer(p) != "" {
		return true
	}
	for i, z := range cityWarZones {
		if !z.arena.contains(x, y) {
			continue
		}
		f, ok := w.cityFighters[p]
		if !ok || f.city != i || !w.cityFighterValid(p, f) {
			return false
		}
		if w.guilds.Wars.Cities.Phase == "preparing" {
			center := (z.arena.x1 + z.arena.x2) / 2
			if f.guild == w.guilds.Wars.Cities.Territories[i].Owner {
				return x < center
			}
			return x > center
		}
		return w.guilds.Wars.Cities.Phase == "battle" && w.now().Before(w.guilds.Wars.Cities.StartsAt.Add(31*time.Minute))
	}
	return true
}

// Guild dissolution must not leave a territory/entry pointing to an ID that
// may later be reused. Called inside the existing guild persistence boundary.
func (w *World) pruneGuildWarReferences() {
	if w.guilds.FindByID(w.guilds.Wars.Tower.Owner) == nil {
		w.guilds.Wars.Tower.Owner = 0
	}
	if w.guilds.FindByID(w.guilds.Wars.Castle.Owner) == nil {
		w.guilds.Wars.Castle.Owner = 0
	}
	c := &w.guilds.Wars.Cities
	for i := range c.Territories {
		t := &c.Territories[i]
		if w.guilds.FindByID(t.Owner) == nil {
			t.Owner, t.Victories = 0, 0
		}
		if w.guilds.FindByID(t.Challenger) == nil {
			t.Challenger, t.Bid = 0, 0
		}
	}
	entries := make([]model.CityWarEntry, 0, len(c.Entries))
	for _, e := range c.Entries {
		if w.guilds.FindByID(e.Guild) != nil {
			entries = append(entries, e)
		}
	}
	c.Entries = entries
	// A deleted guild keeps Kefra defeated; only the credited guild is cleared.
	if w.guilds.FindByID(w.guilds.Wars.Kefra.Guild) == nil {
		w.guilds.Wars.Kefra.Guild = 0
	}
}
