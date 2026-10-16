package game

import "wydgo/internal/model"

// Guild kingdom is canonical; cape color is its character projection. Keep the
// cape family, identity and effects, rather than issuing a replacement item.
func alignGuildCape(ch *model.Char, kingdom byte) bool {
	if ch == nil || characterKingdom(ch) == kingdom {
		return false
	}
	tier, ok := model.KingdomCapeTierOf(ch.Equip[model.CapeSlot].Index)
	if !ok {
		if neutral, convertible := model.NeutralCape(ch.Equip[model.CapeSlot].Index); convertible {
			tier, ok = model.KingdomCapeTierOf(neutral)
		}
	}
	index, valid := model.KingdomCapeAtTier(tier, kingdom)
	if !ok || !valid {
		return false // Empty slots and unrelated items are not free cape grants.
	}
	ch.Equip[model.CapeSlot].Index = index
	return true
}

func (w *World) guildKingdomChangeAllowed(ch *model.Char, kingdom byte) bool {
	guild, member := w.guildOf(ch)
	return guild == nil || member.Rank == model.GuildRankLeader ||
		(characterKingdom(ch) == kingdom && guild.Kingdom == kingdom)
}

// Include inactive characters on connected accounts, even when their selected
// character belongs to another guild. Offline/selection accounts reconcile at
// entry from the durable registry, without loading or overwriting offline data.
func (w *World) guildKingdomAccounts(actor *Player, proposedActor *model.Account,
	guildID uint16, kingdom byte) ([]*Player, map[*Player]*model.Account) {
	seeds := []*Player{actor}
	changes := map[*Player]*model.Account{actor: proposedActor}
	for _, p := range w.players {
		if p == actor || !p.InWorld || p.Account == nil || p.Char == nil || p.Session == nil || p.Session.IsClosed() {
			continue
		}
		for i := range p.Account.Chars {
			if guild, _ := w.guildOf(&p.Account.Chars[i]); guild != nil && guild.ID == guildID {
				seeds = append(seeds, p)
				changes[p] = accountStateSnapshot(p.Account)
				break
			}
		}
	}
	for _, account := range changes {
		for i := range account.Chars {
			ch := &account.Chars[i]
			if guild, _ := w.guildOf(ch); guild != nil && guild.ID == guildID && alignGuildCape(ch, kingdom) && ch.Score != nil {
				w.recalcPlayer(ch)
			}
		}
	}
	return seeds, changes
}

func installKingdomAccounts(changes map[*Player]*model.Account) {
	for p, account := range changes {
		for i := range account.Chars {
			installed := cloneCharacterState(&account.Chars[i])
			if score := p.Account.Chars[i].Score; score != nil && installed.Score != nil {
				*score = *installed.Score
				installed.Score = score
			}
			p.Account.Chars[i] = installed
		}
	}
}

func (w *World) publishGuildKingdomAccounts(changes map[*Player]*model.Account) {
	for p := range changes {
		w.syncKingdomChange(p)
	}
}
