package game

import (
	"reflect"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func (w *World) queueKingdomCape(p *Player, kingdom byte, change kingdomCapeChange, cost uint32,
	useEmblem bool, publish func()) error {
	if p == nil || p.Char == nil || p.Char.Score == nil || p.Account == nil || !bindCriticalPlayer(p).current(w) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalGuildMutationReserved(p) {
		return errCriticalPersistenceBusy
	}
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	if useEmblem {
		ch.Equip[13] = model.Item{}
	} else if !consumeSapphires(ch, cost) {
		return errCriticalPersistenceIdentity
	}
	if change.Preserve {
		ch.Equip[model.CapeSlot].Index = change.Index
	} else {
		ch.Equip[model.CapeSlot] = model.Item{Index: change.Index, UID: ch.Equip[model.CapeSlot].UID}
	}
	w.ensureCapeEmblem(ch)
	w.recalcPlayer(ch)
	registry := w.guilds
	write := criticalWrite{}
	// Kingdom admission reads guild membership even for non-leaders. Retain the
	// aggregate so a concurrent membership transaction cannot invalidate it.
	if registry != nil {
		write.guilds = w.criticalGuildSnapshot()
		if guild, member := w.guildOf(p.Char); guild != nil && member != nil && member.Rank == model.GuildRankLeader {
			write.guilds.FindByID(guild.ID).Kingdom = kingdom
		}
	}
	var retained *model.GuildRegistry
	if registry != nil {
		retained = w.criticalGuildSnapshot()
	}
	hooks := criticalAccountMutationHooks{
		check: func() error {
			if w.guilds != registry || (registry != nil && !reflect.DeepEqual(w.criticalGuildSnapshot(), retained)) {
				return errCriticalPersistenceIdentity
			}
			return nil
		},
		reject: func() {
			if !p.Session.IsClosed() {
				p.Session.Send(wire.SendItem(p.ID, placeEquip, 13, p.Char.Equip[13]))
				p.Session.Send(wire.SendItem(p.ID, placeEquip, model.CapeSlot, p.Char.Equip[model.CapeSlot]))
			}
		},
	}
	return w.queueCriticalAccountWrite(p, proposed, write, "kingdom cape", "Save failed. The kingdom was not changed.", func() {
		installed, score := cloneCharacterState(ch), p.Char.Score
		*score = *installed.Score
		installed.Score = score
		*p.Char = installed
		if write.guilds != nil {
			*w.guilds = *write.guilds
		}
	}, func() {
		w.syncKingdomChange(p)
		if publish != nil {
			publish()
		}
	}, true, hooks)
}
