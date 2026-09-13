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
	{"Armia", warRect{2052, 2052, 2171, 2163}, warRect{197, 213, 238, 230}, 2086, 2093, 205, 220, 228, 220},
	{"Azran", warRect{2432, 1672, 2675, 1767}, warRect{197, 149, 238, 166}, 2494, 1707, 205, 157, 228, 157},
	{"Erion", warRect{2448, 1966, 2476, 2024}, warRect{141, 213, 182, 230}, 2453, 2000, 146, 220, 173, 220},
	{"Nippleheim", warRect{3605, 3090, 3690, 3260}, warRect{141, 149, 182, 166}, 3652, 3122, 146, 157, 173, 157},
}
var towerWarArena = warRect{2445, 1850, 2546, 1920}

type cityFighter struct {
	city  int
	guild uint16
}

func WithGuildWars(config model.GuildWarConfig) WorldOption {
	return func(w *World) { w.warConfig = config }
}

func (w *World) warCommit(change func() error) error {
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
		w.broadcast(func() []byte { return wire.MessageWhisper(0, "[GUERRA]", part, 7) })
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
	return "nenhuma"
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
	if err := w.warCommit(func() error { *mask |= bit; return nil }); err != nil {
		return err
	}
	w.warNotice(message)
	return nil
}

func (w *World) tickGuildWars(now time.Time) {
	if !w.warConfig.Enabled || w.guilds == nil || now.Before(w.warNextTick) {
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
	// An interrupted city fight cannot be judged from the set of players that
	// happened to reconnect first. Cancel/refund its applications atomically.
	if !w.warInitialized {
		phase := w.guilds.Wars.Cities.Phase
		if phase == "preparing" || phase == "battle" {
			if err := w.cancelCityWar(); err != nil {
				log.Printf("recover city war: %v", err)
				return
			}
		}
		w.warInitialized = true
	}
	local := now.In(w.warLocation)
	if err := w.tickTowerWar(local); err != nil {
		log.Printf("tower war: %v", err)
	}
	if err := w.tickCityWar(local); err != nil {
		log.Printf("city war: %v", err)
	}
	w.cleanCityArena()
}

func (w *World) tickTowerWar(now time.Time) error {
	s := &w.guilds.Wars.Tower
	if s.Day != "" && !s.Finished && !now.Before(s.EndsAt) {
		owner := s.Owner
		if err := w.warCommit(func() error {
			if g := w.guilds.FindByID(owner); g != nil {
				if err := g.AddFame(model.GuildWarFame); err != nil {
					return err
				}
			}
			w.guilds.Wars.Tower.Finished = true
			return nil
		}); err != nil {
			return err
		}
		w.removeWarTower()
		w.ejectTowerArena()
		if owner != 0 {
			w.warNotice(fmt.Sprintf("Guerra de torre encerrada! Vencedora: %s. Recompensa: 100 de fama de guilda.", w.warGuildName(owner)))
		} else {
			w.warNotice("Guerra de torre encerrada sem vencedora. Nenhuma recompensa concedida.")
		}
	}
	day := now.Format("2006-01-02")
	start := time.Date(now.Year(), now.Month(), now.Day(), int(w.warConfig.TowerHour), 0, 0, 0, now.Location())
	if now.Weekday() >= time.Monday && now.Weekday() <= time.Friday &&
		!now.Before(start) && now.Before(start.Add(35*time.Minute)) && s.Day != day {
		if err := w.warCommit(func() error {
			w.guilds.Wars.Tower = model.TowerWarState{Day: day, EndsAt: start.Add(35 * time.Minute)}
			return nil
		}); err != nil {
			return err
		}
	}
	if s.Day == day && !s.Finished {
		battle := s.EndsAt.Add(-30 * time.Minute)
		for _, warning := range []struct {
			end     time.Time
			bit     uint16
			minutes int
			text    string
		}{
			{battle, 1, 5, "Guerra de torre comeca em ate 5 minutos! Entrada por /tower em Erion."},
			{battle, 2, 1, "Guerra de torre comeca em 1 minuto!"},
			{s.EndsAt, 4, 5, "Guerra de torre termina em 5 minutos! A ultima guilda dona recebe 100 de fama."},
			{s.EndsAt, 8, 1, "Guerra de torre termina em 1 minuto!"},
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
		w.ejectTowerArena()
		w.warNotice("Guerra de torre iniciada! Use /tower em Erion. Duracao: 30 minutos.")
	}
	return nil
}

func (w *World) towerBattleActive(now time.Time) bool {
	if !w.warConfig.Enabled || w.guilds == nil {
		return false
	}
	s := w.guilds.Wars.Tower
	return s.Day != "" && !s.Finished && !now.Before(s.EndsAt.Add(-30*time.Minute)) && now.Before(s.EndsAt)
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
	x, y := w.findFreePosition(2495, 1885, 3)
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
	if !validPlayerMobParticipant(p) || !w.towerBattleActive(w.now()) || !towerWarArena.contains(p.X, p.Y) {
		return false
	}
	g, _ := w.guildOf(p.Char)
	return g != nil && g.ID != w.guilds.Wars.Tower.Owner
}

func (w *World) captureWarTower(p *Player, m *Mob, applied uint32) {
	if !w.towerTargetAllowed(p, m) {
		m.HP = minU32(m.Def.Score.MaxHP, max(1, applied))
		w.sendToMobViewProtocol(m, func(_ *Player) []byte { return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, 0, 0) })
		return
	}
	g, _ := w.guildOf(p.Char)
	owner := g.ID
	err := w.warCommit(func() error { w.guilds.Wars.Tower.Owner = owner; return nil })
	if err != nil {
		m.HP = minU32(m.Def.Score.MaxHP, max(1, applied))
		log.Printf("tower capture rollback: %v", err)
	} else {
		m.HP = m.Def.Score.MaxHP
		m.Affects = [16]model.Affect{}
		m.TargetID = 0
		w.warNotice(fmt.Sprintf("%s da guilda %s derrubou a torre! Nova defensora: %s.", p.Char.Name, g.Name, g.Name))
		w.ejectTowerArena()
	}
	w.sendToMobViewProtocol(m, func(_ *Player) []byte { return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, 0, 0) })
}

func (w *World) ejectTowerArena() {
	for _, p := range w.players {
		if p.InWorld && p.Char != nil && towerWarArena.contains(p.X, p.Y) {
			w.teleportPlayer(p, 2453, 2000)
		}
	}
}

func (w *World) cancelCityWar() error {
	err := w.warCommit(func() error {
		c := &w.guilds.Wars.Cities
		for _, e := range c.Entries {
			if g := w.guilds.FindByID(e.Guild); g != nil {
				if err := g.AddFame(model.GuildWarFame); err != nil {
					return err
				}
			}
		}
		for i := range c.Territories {
			c.Territories[i].Challenger, c.Territories[i].Bid = 0, 0
		}
		c.Entries, c.Phase = nil, "finished"
		return nil
	})
	if err == nil {
		w.cityWarRunning = false
		for p := range w.cityFighters {
			p.Session.Send(wire.WarInfo())
		}
		w.cityFighters = nil
		w.cityWarScores = [4]map[uint16]uint64{}
		w.warNotice("Guerra de cidades cancelada por interrupcao. Inscricoes devolvidas: 100 de fama por guilda; donos preservados.")
	}
	return err
}

func (w *World) tickCityWar(now time.Time) error {
	c := &w.guilds.Wars.Cities
	if c.Day != "" && c.Phase != "finished" && !now.Before(c.StartsAt.Add(21*time.Minute)) {
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
				*c = model.CityWarState{Day: day, Phase: "registration", Territories: territories,
					StartsAt: time.Date(sunday.Year(), sunday.Month(), sunday.Day(), int(w.warConfig.CityHour), 0, 0, 0, now.Location())}
				return nil
			})
			if err == nil {
				w.warNotice("Inscricoes para a guerra de cidades abertas ate meia-noite! /citywar 1..4. Custo: 100 de fama.")
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
		w.warNotice("Inscricoes da guerra de cidades encerradas. Consulte donos e desafiantes com /citywar.")
	}
	for _, warning := range []struct {
		end     time.Time
		bit     uint16
		minutes int
		text    string
	}{
		{c.StartsAt, 1, 5, "Guerra de cidades comeca em 5 minutos! Reunam-se na cidade disputada; convocacao 3 minutos apos o inicio."},
		{c.StartsAt, 2, 1, "Guerra de cidades comeca em 1 minuto! Limite: 26 por guilda. Erion: somente mortais."},
		{c.StartsAt.Add(6 * time.Minute), 8, 1, "Combate das guerras de cidades sera liberado em 1 minuto!"},
		{c.StartsAt.Add(21 * time.Minute), 16, 5, "Guerras de cidades terminam em 5 minutos! Vencem as forcas sobreviventes."},
		{c.StartsAt.Add(21 * time.Minute), 32, 1, "Guerras de cidades terminam em 1 minuto!"},
	} {
		if err := w.warCountdown(now, warning.end, &c.Notices, warning.bit, warning.minutes, warning.text); err != nil {
			return err
		}
	}
	if !now.Before(c.StartsAt) && c.Notices&4 == 0 {
		if err := w.warCommit(func() error { c.Notices |= 4; return nil }); err != nil {
			return err
		}
		w.warNotice("Guerra de cidades iniciada! Convocacao em +3 minutos; combate em +6; resultado em +21.")
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
		w.warNotice("Guildas convocadas! Preparacao de 3 minutos; combate ainda bloqueado.")
	}
	if c.Phase == "preparing" && !now.Before(c.StartsAt.Add(6*time.Minute)) {
		if err := w.warCommit(func() error { c.Phase = "battle"; return nil }); err != nil {
			return err
		}
		w.warNotice("Combate liberado nas quatro cidades! Duracao: 15 minutos; empate favorece a defensora.")
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
			w.cancelTrade(p, "city war")
			if w.teleportPlayer(p, x, y) {
				w.cityFighters[p] = cityFighter{city: city, guild: g.ID}
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
	if err := w.warCommit(func() error {
		c := &w.guilds.Wars.Cities
		for i := range c.Territories {
			t := &c.Territories[i]
			if t.Challenger != 0 && scores[i][t.Challenger] > scores[i][t.Owner] {
				t.Owner, t.Victories = t.Challenger, 0
			} else if t.Owner != 0 && t.Victories < 4 {
				t.Victories++
			}
			t.Challenger, t.Bid = 0, 0
		}
		c.Phase, c.Entries = "finished", nil
		return nil
	}); err != nil {
		return err
	}
	w.cityWarRunning = false
	for p := range w.cityFighters {
		p.Session.Send(wire.WarInfo())
	}
	w.cityFighters = nil
	w.cityWarScores = [4]map[uint16]uint64{}
	for i, z := range cityWarZones {
		t := w.guilds.Wars.Cities.Territories[i]
		w.warNotice(fmt.Sprintf("%s: vencedora %s. Pontos defesa %d x ataque %d. Dominio salvo; defesas: %d.", z.name, w.warGuildName(t.Owner), scores[i][previous[i].Owner], scores[i][previous[i].Challenger], t.Victories))
	}
	w.cleanCityArena()
	return nil
}

func (w *World) cleanCityArena() {
	for p, f := range w.cityFighters {
		if !w.cityFighterValid(p, f) {
			p.Session.Send(wire.WarInfo())
			delete(w.cityFighters, p)
		}
	}
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil {
			continue
		}
		for i, z := range cityWarZones {
			if !z.arena.contains(p.X, p.Y) {
				continue
			}
			f, ok := w.cityFighters[p]
			if !w.cityWarRunning || !ok || f.city != i {
				w.teleportPlayer(p, z.exitX, z.exitY)
			}
		}
	}
}

// handled=true makes war areas fail closed even with PK mode enabled.
func (w *World) guildWarPvP(a, b *Player) (allowed, handled bool) {
	if !w.warConfig.Enabled || w.guilds == nil || a == nil || b == nil || a.Char == nil || b.Char == nil {
		return false, false
	}
	for i, z := range cityWarZones {
		if z.arena.contains(a.X, a.Y) || z.arena.contains(b.X, b.Y) {
			fa, oka := w.cityFighters[a]
			fb, okb := w.cityFighters[b]
			c := w.guilds.Wars.Cities
			active := c.Phase == "battle" && !w.now().Before(c.StartsAt.Add(6*time.Minute)) && w.now().Before(c.StartsAt.Add(21*time.Minute))
			return active && oka && okb && fa.city == i && fb.city == i && fa.guild != fb.guild &&
				w.cityFighterValid(a, fa) && w.cityFighterValid(b, fb), true
		}
	}
	if towerWarArena.contains(a.X, a.Y) || towerWarArena.contains(b.X, b.Y) {
		ga, _ := w.guildOf(a.Char)
		gb, _ := w.guildOf(b.Char)
		return w.towerBattleActive(w.now()) && towerWarArena.contains(a.X, a.Y) && towerWarArena.contains(b.X, b.Y) && ga != nil && gb != nil && ga.ID != gb.ID, true
	}
	return false, false
}

func (w *World) registerCityWar(p *Player, city int) error {
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
	if p.Char.Citizenship != w.channel || !cityWarZones[city].city.contains(p.X, p.Y) || w.gameplaySpaceForPlayer(p) != "" {
		return fmt.Errorf("leader must be a citizen of this channel and inside the challenged city")
	}
	return w.warCommit(func() error { return w.guilds.Wars.Cities.Register(g, city) })
}

func (w *World) guildWarCommand(p *Player, command, arg string) {
	if p == nil || p.Session == nil || p.Char == nil {
		return
	}
	send := func(s string) {
		for _, part := range splitWarNotice(s) {
			p.Session.Send(wire.MessageWhisper(0, "[GUERRA]", part, 7))
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
		if g == nil || !validPlayerMobParticipant(p) || !w.towerBattleActive(w.now()) || w.gameplaySpaceForPlayer(p) != "" || !cityWarZones[2].city.contains(p.X, p.Y) {
			send("Tower entry requires a guild, a living character in Erion and an active tower war.")
			return
		}
		if !w.teleportPlayer(p, 2450, 1855) {
			send("Tower entry failed.")
		}
	case "citywar":
		if arg != "" {
			city, err := strconv.Atoi(strings.TrimSpace(arg))
			if err != nil {
				send("Use /citywar 1..4 (Armia, Azran, Erion, Nippleheim). Fee: 100 guild fame.")
				return
			}
			if err := w.registerCityWar(p, city-1); err != nil {
				send(err.Error())
				return
			}
			send("Application saved. Fee: 100 guild fame. The highest fame bid challenges the city.")
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
	if !w.warConfig.Enabled || w.guilds == nil {
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
		return w.guilds.Wars.Cities.Phase == "battle" && w.now().Before(w.guilds.Wars.Cities.StartsAt.Add(21*time.Minute))
	}
	return true
}

// Guild dissolution must not leave a territory/entry pointing to an ID that
// may later be reused. Called inside the existing guild persistence boundary.
func (w *World) pruneGuildWarReferences() {
	if w.guilds.FindByID(w.guilds.Wars.Tower.Owner) == nil {
		w.guilds.Wars.Tower.Owner = 0
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
}
