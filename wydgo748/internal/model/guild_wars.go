package model

import (
	"fmt"
	"math"
	"time"
	_ "time/tzdata" // The Windows server must not depend on host zoneinfo files.
)

const GuildWarFame = uint32(100)

// CityWarConquestFame is the conquest adjustment in the 7.59 city result
// routine. Registration costs and successful defenses are separate rules.
const CityWarConquestFame = uint32(50)

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
	FameResetDay string          `json:"fameResetDay,omitempty"` // Last Sunday midnight processed in the war timezone.
	Tower        TowerWarState   `json:"tower"`
	Cities       CityWarState    `json:"cities"`
	Kefra        KefraState      `json:"kefra"`
	Castle       CastleWarState  `json:"castle"`
	Medals       []WarMedalClaim `json:"medals,omitempty"`
}

// CastleWarState keeps Noatum ownership and round settlement in the guild
// transaction. The domain controller does not enable the event scheduler.
type CastleWarState struct {
	Owner     uint16         `json:"owner,omitempty"`
	Victories uint32         `json:"victories,omitempty"`
	Day       string         `json:"day,omitempty"`
	StartsAt  time.Time      `json:"startsAt,omitempty"`
	Phase     CastleWarPhase `json:"phase,omitempty"`
	Captured  bool           `json:"captured,omitempty"`
}

// ReconcileCastleOwner follows the 7.59 city-war finish routine: retain the
// castle owner while it matches any city entry, otherwise select the first
// nonzero city owner in territory order. This includes the reference's zero
// match: an empty castle stays empty while at least one city is unowned.
func (s *GuildWarState) ReconcileCastleOwner() {
	for _, territory := range s.Cities.Territories {
		if territory.Owner == s.Castle.Owner {
			return
		}
	}
	for _, territory := range s.Cities.Territories {
		if territory.Owner != 0 {
			s.Castle.Owner = territory.Owner
			return
		}
	}
}

// ResolveCityWars applies one settlement to a registry snapshot. The caller
// owns rollback or detached proposal admission and persists the entire registry
// before publishing winners, fame or castle ownership.
func (r *GuildRegistry) ResolveCityWars(scores [4]map[uint16]uint64) error {
	if r.Wars.Cities.Phase == "finished" {
		return nil
	}
	for i := range r.Wars.Cities.Territories {
		t := &r.Wars.Cities.Territories[i]
		if t.Challenger != 0 && scores[i][t.Challenger] > scores[i][t.Owner] {
			winner := r.FindByID(t.Challenger)
			if winner == nil {
				return fmt.Errorf("city winner does not exist")
			}
			if err := winner.AddFame(CityWarConquestFame); err != nil {
				return err
			}
			if loser := r.FindByID(t.Owner); loser != nil {
				// sub_588340 clamps negative fame to zero, not unsigned wrap.
				loser.Fame -= min(loser.Fame, CityWarConquestFame)
			}
			t.Owner, t.Victories = t.Challenger, 0
		} else if t.Owner != 0 && t.Victories < 4 {
			t.Victories++
		}
		t.Challenger, t.Bid = 0, 0
	}
	r.Wars.ReconcileCastleOwner()
	r.Wars.Cities.Phase, r.Wars.Cities.Entries = "finished", nil
	return nil
}

// KefraState ports W2PP KefraLive: Kefra lives while Defeated is false. Once
// killed it records the killer's guild (zero without one) and stays defeated
// until the weekly reset; the passage to Uxmal opens only meanwhile.
type KefraState struct {
	Defeated   bool      `json:"defeated,omitempty"`
	Guild      uint16    `json:"guild,omitempty"`
	DefeatedAt time.Time `json:"defeatedAt,omitempty"`
}

type TowerWarState struct {
	Started          bool      `json:"started,omitempty"`
	Notices          uint16    `json:"notices,omitempty"`
	Day              string    `json:"day,omitempty"`
	EndsAt           time.Time `json:"endsAt,omitempty"`
	Owner            uint16    `json:"owner,omitempty"`
	Finished         bool      `json:"finished,omitempty"`
	Kingdom          byte      `json:"kingdom,omitempty"`
	InitialKingdom   byte      `json:"initialKingdom,omitempty"`
	DefenderCaptures uint32    `json:"defenderCaptures,omitempty"` // Native divisor: starts at 1, then increments on defending-realm recapture.
	Captures         uint64    `json:"captures,omitempty"`
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
	Bid   uint32 `json:"bid,omitempty"`
}

type CityWarState struct {
	FameBids    bool             `json:"fameBids,omitempty"` // False identifies legacy fixed-fee applications.
	Notices     uint16           `json:"notices,omitempty"`
	Day         string           `json:"day,omitempty"` // Sunday's local date, also the replay key.
	StartsAt    time.Time        `json:"startsAt,omitempty"`
	Phase       string           `json:"phase,omitempty"` // registration, waiting, preparing, battle, finished
	Territories [4]CityTerritory `json:"territories"`
	Entries     []CityWarEntry   `json:"entries,omitempty"` // One cumulative bid per guild and city.
}

func (s GuildWarState) Clone() GuildWarState {
	s.Cities.Entries = append([]CityWarEntry(nil), s.Cities.Entries...)
	s.Medals = append([]WarMedalClaim(nil), s.Medals...)
	return s
}

func (g *Guild) AddFame(amount uint32) error {
	if amount > math.MaxUint32-g.Fame {
		return fmt.Errorf("guild fame overflow")
	}
	g.Fame += amount
	return nil
}

// Register spends only the increase in a guild's cumulative city bid. There is
// no entry fee or threshold. Equal totals retain the incumbent challenger.
func (s *CityWarState) Register(g *Guild, city int, bid uint32) error {
	if s.Phase != "registration" || g == nil || city < 0 || city >= len(s.Territories) {
		return fmt.Errorf("city registration is closed or invalid")
	}
	t := &s.Territories[city]
	if g.ID == 0 || t.Owner == g.ID {
		return fmt.Errorf("a guild cannot challenge its own city")
	}
	if !s.FameBids && len(s.Entries) != 0 {
		return fmt.Errorf("legacy applications must be migrated before bidding")
	}
	index, previous := -1, uint32(0)
	for i, entry := range s.Entries {
		if entry.Guild == g.ID && entry.City == city {
			index, previous = i, entry.Bid
			break
		}
	}
	if bid < previous {
		return fmt.Errorf("a bid cannot be reduced")
	}
	if bid-previous > g.Fame {
		return fmt.Errorf("not enough guild fame for this bid increase")
	}
	if index >= 0 && bid == previous {
		return nil
	}
	g.Fame -= bid - previous
	s.FameBids = true
	if t.Challenger == 0 || bid > t.Bid {
		t.Challenger, t.Bid = g.ID, bid
	}
	if index >= 0 {
		s.Entries[index].Bid = bid
	} else {
		s.Entries = append(s.Entries, CityWarEntry{Guild: g.ID, City: city, Bid: bid})
	}
	return nil
}

// CancelCityBids cannot carry last week's spent fame into a new week. Legacy
// fixed fees are refunded during migration without converting any treasury.
func (r *GuildRegistry) CancelCityBids() error {
	c := &r.Wars.Cities
	for _, entry := range c.Entries {
		amount := GuildWarFame
		if c.FameBids {
			amount = entry.Bid
			if r.Wars.FameResetDay >= c.Day {
				amount = 0
			}
		}
		if g := r.FindByID(entry.Guild); g != nil {
			if err := g.AddFame(amount); err != nil {
				return err
			}
		}
	}
	for i := range c.Territories {
		c.Territories[i].Challenger, c.Territories[i].Bid = 0, 0
	}
	c.Entries, c.Phase = nil, "finished"
	return nil
}

// ResetWeeklyGuildFame closes Saturday's bids and resets only unspent guild
// fame. Challenger selection survives. The first call records an upgrade
// baseline without retroactively destroying an existing week's balance.
func (r *GuildRegistry) ResetWeeklyGuildFame(local time.Time) bool {
	boundary := time.Date(local.Year(), local.Month(), local.Day(), 0, 0, 0, 0, local.Location()).AddDate(0, 0, -int(local.Weekday()))
	day := boundary.Format("2006-01-02")
	if c := &r.Wars.Cities; c.Phase == "registration" && c.Day <= day {
		c.Phase = "waiting"
	}
	if r.FameResetDay > r.Wars.FameResetDay {
		r.Wars.FameResetDay = r.FameResetDay
	}
	if r.Wars.FameResetDay >= day {
		return false
	}
	if r.Wars.FameResetDay != "" {
		for i := range r.Guilds {
			r.Guilds[i].Fame = 0
		}
	}
	r.Wars.FameResetDay = day
	r.FameResetDay = day
	if c := &r.Wars.Cities; c.Phase == "registration" && c.Day <= day {
		c.Phase = "waiting"
	}
	return true
}

func (s GuildWarState) Validate(r *GuildRegistry) error {
	if err := s.Castle.validate(); err != nil {
		return err
	}
	if err := s.validateMedals(); err != nil {
		return err
	}
	for _, kingdom := range []byte{s.Tower.Kingdom, s.Tower.InitialKingdom} {
		if kingdom != 0 && kingdom != 7 && kingdom != 8 {
			return fmt.Errorf("invalid tower kingdom")
		}
	}
	if s.FameResetDay != "" {
		day, err := time.Parse("2006-01-02", s.FameResetDay)
		if err != nil || day.Weekday() != time.Sunday {
			return fmt.Errorf("invalid guild fame reset calendar")
		}
	}
	exists := func(id uint16) bool { return id == 0 || r.FindByID(id) != nil }
	if !exists(s.Tower.Owner) {
		return fmt.Errorf("tower owner does not exist")
	}
	if !exists(s.Castle.Owner) {
		return fmt.Errorf("castle owner does not exist")
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
	seen := make(map[[2]int]bool)
	for _, e := range c.Entries {
		key := [2]int{int(e.Guild), e.City}
		if e.Guild == 0 || !exists(e.Guild) || e.City < 0 || e.City >= 4 || seen[key] {
			return fmt.Errorf("invalid city entry")
		}
		seen[key] = true
	}
	if k := s.Kefra; !exists(k.Guild) || k.Defeated == k.DefeatedAt.IsZero() || !k.Defeated && k.Guild != 0 {
		return fmt.Errorf("invalid Kefra state")
	}
	return nil
}
