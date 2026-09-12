package game

import (
	"errors"
	"strings"
	"time"

	"wydgo/internal/model"
)

var (
	errNightmareLeaderEvolution = errors.New("your evolution cannot enter this Nightmare room")
	errNightmareNoMember        = errors.New("no eligible party member is available for this Nightmare room")
)

// nightmareEligibleMembers aplica as travas de entrada que o TMSrv nativo
// mantem fora da configuracao generica de tickets. Celestiais acima do nivel
// permitido nao sao transportados junto com a party; se forem o solicitante
// (o primeiro membro, sempre o lider), a abertura e recusada. O mesmo vale
// para o Arcano quando nao houver uma entrada NT disponivel.
//
// A lista retornada e uma nova fatia. Nenhuma party e alterada: somente os
// membros elegiveis sao movidos para a sala privada.
func nightmareEligibleMembers(members []*Player, cfg *model.VolatileInstance) ([]*Player, error) {
	if cfg == nil {
		return nil, errNightmareNoMember
	}
	tier := strings.ToLower(strings.TrimSpace(cfg.NightmareTier))
	if tier == "" {
		return append([]*Player(nil), members...), nil
	}
	if tier != "normal" && tier != "mystic" && tier != "arcane" {
		return nil, errNightmareNoMember
	}
	eligible := make([]*Player, 0, len(members))
	for index, member := range members {
		if member == nil || member.Char == nil || !member.InWorld {
			continue
		}
		if !isCelestial(member.Char) {
			eligible = append(eligible, member)
			continue
		}

		level := playerLevel(member.Char)
		if tier != "arcane" && level >= 180 {
			if index == 0 {
				return nil, errNightmareLeaderEvolution
			}
			continue
		}
		if tier == "arcane" {
			if level >= 239 || member.Char.NightmareTickets == 0 {
				if index == 0 {
					return nil, errNightmareLeaderEvolution
				}
				continue
			}
		}
		eligible = append(eligible, member)
	}
	if len(eligible) == 0 {
		return nil, errNightmareNoMember
	}
	return eligible, nil
}

type nightmareEntryCharge struct {
	player   *Player
	previous uint32
}

// chargeNightmareEntries reserva a entrada NT somente depois que todas as
// validacoes de sala passaram. O snapshot permite restaurar o saldo quando o
// primeiro spawn ou a persistencia falhar.
func chargeNightmareEntries(members []*Player, cfg *model.VolatileInstance) []nightmareEntryCharge {
	if cfg == nil || !strings.EqualFold(strings.TrimSpace(cfg.NightmareTier), "arcane") {
		return nil
	}
	charges := make([]nightmareEntryCharge, 0, len(members))
	for _, member := range members {
		if member == nil || member.Char == nil || !isCelestial(member.Char) ||
			member.Char.NightmareTickets == 0 {
			continue
		}
		charges = append(charges, nightmareEntryCharge{
			player: member, previous: member.Char.NightmareTickets,
		})
		member.Char.NightmareTickets--
	}
	return charges
}

func restoreNightmareEntries(charges []nightmareEntryCharge) {
	for _, charge := range charges {
		if charge.player != nil && charge.player.Char != nil {
			charge.player.Char.NightmareTickets = charge.previous
		}
	}
}

// nightmarePartyRunKey identifica a janela ativa do tier. O contador nativo
// PartyPesa reinicia quando a janela muda; usar o inicio da janela no timestamp
// evita misturar execucoes de horas diferentes e nao exige ticker por sala.
func nightmarePartyRunKey(cfg *model.VolatileInstance, now time.Time) string {
	if cfg == nil || cfg.PartyRunLimit <= 0 {
		return ""
	}
	base := cfg.ID
	if strings.TrimSpace(cfg.SharedGroup) != "" {
		base = strings.TrimSpace(cfg.SharedGroup)
	}
	if len(cfg.Schedule) == 0 {
		return base
	}
	secondOfHour := now.Minute()*60 + now.Second()
	for _, window := range cfg.Schedule {
		if window.StartMinute < 0 || window.StartMinute > 59 || window.DurationSeconds <= 0 {
			continue
		}
		startSecond := window.StartMinute * 60
		elapsed := secondOfHour - startSecond
		start := time.Date(now.Year(), now.Month(), now.Day(), now.Hour(),
			window.StartMinute, 0, 0, now.Location())
		if elapsed < 0 {
			// The active occurrence started in the previous hour.
			elapsed += 60 * 60
			start = start.Add(-time.Hour)
		}
		if elapsed >= window.DurationSeconds {
			continue
		}
		return base + ":" + start.Format(time.RFC3339)
	}
	return ""
}

func (w *World) nightmarePartyRunAllowed(cfg *model.VolatileInstance, partyMode string, now time.Time) (string, bool) {
	if cfg == nil || partyMode != "party" || cfg.PartyRunLimit <= 0 {
		return "", true
	}
	key := nightmarePartyRunKey(cfg, now)
	if key == "" {
		return "", true
	}
	return key, w.nightmarePartyRuns[key] < cfg.PartyRunLimit
}

// instancePlayerCapacity keeps maxPlayers as the native party size while a
// shared Nightmare window can contain one party per allowed run.  Without
// this distinction the first party (13 players) filled the physical zone and
// made the second/third native run impossible.
func instancePlayerCapacity(cfg *model.VolatileInstance) int {
	if cfg == nil || cfg.MaxPlayers <= 0 {
		return 0
	}
	if sharedTimedInstance(*cfg) && cfg.PartyRunLimit > 0 {
		return cfg.MaxPlayers * cfg.PartyRunLimit
	}
	return cfg.MaxPlayers
}

func (w *World) recordNightmarePartyRun(key string) {
	if key == "" {
		return
	}
	if w.nightmarePartyRuns == nil {
		w.nightmarePartyRuns = make(map[string]int)
	}
	w.nightmarePartyRuns[key]++
	w.markInstanceStateDirty()
}
