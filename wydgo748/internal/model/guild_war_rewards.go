package model

import (
	"fmt"
	"math"
)

const (
	WarMedalGreen uint16 = 4055 // Legacy catalog label: Greed_Medal, mesh 2745.
	WarMedalGold  uint16 = 4051
	WarMedalRed   uint16 = 4053
)

// WarMedalClaim is an already-rolled reward, not an inventory blueprint tied to
// a session ID. Persist it with the capture/result; consume it atomically with
// the account inventory. Full inventories and disconnects cannot lose medals.
type WarMedalClaim struct {
	Event        string `json:"event"`
	Account      string `json:"account"`
	CharacterUID string `json:"characterUid"`
	Item         uint16 `json:"item"`
}

func (s GuildWarState) validateMedals() error {
	seen := make(map[[2]string]bool, len(s.Medals))
	for _, claim := range s.Medals {
		uid, err := NormalizeCharacterUID(claim.CharacterUID)
		key := [2]string{claim.Event, claim.CharacterUID}
		if err != nil || uid == "" || uid != claim.CharacterUID || claim.Event == "" || claim.Account == "" || seen[key] {
			return fmt.Errorf("invalid war medal claim")
		}
		if claim.Item != WarMedalGreen && claim.Item != WarMedalGold && claim.Item != WarMedalRed {
			return fmt.Errorf("invalid war medal item")
		}
		seen[key] = true
	}
	return nil
}

// TowerKingdom also reads pre-upgrade guild-only tower ownership. New rounds
// and captures persist the kingdom independently of the guild's later fate.
func (r *GuildRegistry) TowerKingdom() byte {
	if r.Wars.Tower.Kingdom != 0 {
		return r.Wars.Tower.Kingdom
	}
	if g := r.FindByID(r.Wars.Tower.Owner); g != nil {
		return g.Kingdom
	}
	return 0
}

func (r *GuildRegistry) CaptureTower(guildID uint16) error {
	g := r.FindByID(guildID)
	s := &r.Wars.Tower
	if g == nil || (g.Kingdom != 7 && g.Kingdom != 8) || g.Kingdom == r.TowerKingdom() || s.Finished {
		return fmt.Errorf("invalid tower capture")
	}
	if s.Captures == math.MaxUint64 {
		return fmt.Errorf("tower capture counter overflow")
	}
	if s.DefenderCaptures == 0 {
		s.DefenderCaptures = 1
	}
	if g.Kingdom == s.InitialKingdom && s.DefenderCaptures < math.MaxUint32 {
		s.DefenderCaptures++
	}
	s.Owner, s.Kingdom = g.ID, g.Kingdom
	s.Captures++
	return nil
}

// ResolveTowerWar follows the reference's realm-change condition. No gold or
// individual fame is granted. Caller supplies a detached registry or rollback.
func (r *GuildRegistry) ResolveTowerWar() error {
	s := &r.Wars.Tower
	if s.Finished {
		return nil
	}
	if s.Kingdom != 0 && s.Kingdom != s.InitialKingdom {
		if g := r.FindByID(s.Owner); g != nil {
			if err := g.AddFame(GuildWarFame); err != nil {
				return err
			}
		}
		penalty := uint32(1000) / max(s.DefenderCaptures, 1)
		// The reference debits all five territories, including the castle.
		owners := []uint16{r.Wars.Castle.Owner}
		for _, territory := range r.Wars.Cities.Territories {
			owners = append(owners, territory.Owner)
		}
		for _, owner := range owners {
			if g := r.FindByID(owner); g != nil && g.Kingdom == s.InitialKingdom {
				g.Fame -= min(g.Fame, penalty)
			}
		}
		s.DefenderCaptures = 1
	}
	s.Finished = true
	return nil
}
