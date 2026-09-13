package model

import (
	"fmt"
	"math"
	"time"
	_ "time/tzdata" // The Windows server must not depend on host zoneinfo files.
)

const GuildWarFame = uint32(100)

// GuildWarTreasuryCap bounds the city treasury so a corrupted economy cannot
// turn a single tax event into an unbounded withdrawal source.
const GuildWarTreasuryCap uint64 = 200_000_000_000

type GuildWarConfig struct {
	Enabled   bool
	Timezone  string
	TowerHour uint32
	CityHour  uint32
}

func DefaultGuildWarConfig() GuildWarConfig {
	return GuildWarConfig{Enabled: true, Timezone: "America/Sao_Paulo", TowerHour: 21, CityHour: 20}
}

func (c GuildWarConfig) Validate() error {
	if c.TowerHour > 23 || c.CityHour > 23 {
		return fmt.Errorf("war hour must be 0..23")
	}
	if c.Timezone == "" {
		return fmt.Errorf("war timezone is required")
	}
	_, err := time.LoadLocation(c.Timezone)
	return err
}

// Wars and Fame share the existing guild registry transaction/journal. A result
// marker and its award must never be committed independently.
type GuildWarState struct {
	Tower  TowerWarState `json:"tower"`
	Cities CityWarState  `json:"cities"`
}

type TowerWarState struct {
	Started  bool      `json:"started,omitempty"`
	Notices  uint16    `json:"notices,omitempty"`
	Day      string    `json:"day,omitempty"`
	EndsAt   time.Time `json:"endsAt,omitempty"`
	Owner    uint16    `json:"owner,omitempty"`
	Finished bool      `json:"finished,omitempty"`
}

type CityTerritory struct {
	Owner      uint16 `json:"owner,omitempty"`
	Challenger uint16 `json:"challenger,omitempty"`
	Bid        uint32 `json:"bid,omitempty"`
	Victories  byte   `json:"victories,omitempty"`
	Treasury   uint64 `json:"treasury,omitempty"`
}

type CityWarEntry struct {
	Guild uint16 `json:"guild"`
	City  int    `json:"city"`
}

type CityWarState struct {
	Notices     uint16           `json:"notices,omitempty"`
	Day         string           `json:"day,omitempty"` // Sunday's local date, also the replay key.
	StartsAt    time.Time        `json:"startsAt,omitempty"`
	Phase       string           `json:"phase,omitempty"` // registration, waiting, preparing, battle, finished
	Territories [4]CityTerritory `json:"territories"`
	Entries     []CityWarEntry   `json:"entries,omitempty"` // one paid application per guild/week
}

func (s GuildWarState) Clone() GuildWarState {
	s.Cities.Entries = append([]CityWarEntry(nil), s.Cities.Entries...)
	return s
}

func (g *Guild) AddFame(amount uint32) error {
	if amount > math.MaxUint32-g.Fame {
		return fmt.Errorf("guild fame overflow")
	}
	g.Fame += amount
	return nil
}

// Register uses the pre-fee fame as the bid, as in W2PP. Ties preserve the first
// challenger. A losing application still costs 100; replay never costs again.
func (s *CityWarState) Register(g *Guild, city int) error {
	if s.Phase != "registration" || g == nil || city < 0 || city >= len(s.Territories) {
		return fmt.Errorf("city registration is closed or invalid")
	}
	for _, entry := range s.Entries {
		if entry.Guild == g.ID {
			return fmt.Errorf("guild already registered this week")
		}
	}
	for _, territory := range s.Territories {
		if territory.Owner == g.ID {
			return fmt.Errorf("city owners cannot challenge another city")
		}
	}
	if g.Fame < GuildWarFame {
		return fmt.Errorf("registration requires 100 guild fame")
	}
	bid := g.Fame
	g.Fame -= GuildWarFame
	t := &s.Territories[city]
	if t.Challenger == 0 || bid > t.Bid {
		t.Challenger, t.Bid = g.ID, bid
	}
	s.Entries = append(s.Entries, CityWarEntry{Guild: g.ID, City: city})
	return nil
}

func (s GuildWarState) Validate(r *GuildRegistry) error {
	exists := func(id uint16) bool { return id == 0 || r.FindByID(id) != nil }
	if !exists(s.Tower.Owner) {
		return fmt.Errorf("tower owner does not exist")
	}
	if s.Tower.Day != "" {
		if _, err := time.Parse("2006-01-02", s.Tower.Day); err != nil || s.Tower.EndsAt.IsZero() {
			return fmt.Errorf("invalid tower calendar")
		}
	}
	c := s.Cities
	switch c.Phase {
	case "":
		if c.Day != "" || len(c.Entries) != 0 {
			return fmt.Errorf("invalid empty city phase")
		}
	case "registration", "waiting", "preparing", "battle", "finished":
		if _, err := time.Parse("2006-01-02", c.Day); err != nil || c.StartsAt.IsZero() {
			return fmt.Errorf("invalid city calendar")
		}
	default:
		return fmt.Errorf("invalid city phase %q", c.Phase)
	}
	for _, t := range c.Territories {
		if !exists(t.Owner) || !exists(t.Challenger) || t.Victories > 4 || t.Owner != 0 && t.Owner == t.Challenger || t.Treasury > GuildWarTreasuryCap {
			return fmt.Errorf("invalid city territory")
		}
	}
	seen := make(map[uint16]bool)
	for _, e := range c.Entries {
		if e.Guild == 0 || !exists(e.Guild) || e.City < 0 || e.City >= 4 || seen[e.Guild] {
			return fmt.Errorf("invalid city entry")
		}
		seen[e.Guild] = true
	}
	return nil
}
