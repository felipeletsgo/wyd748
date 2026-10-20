package model

import (
	"fmt"
	"math"
	"strings"
	"time"
)

// MODERNIZACAO_COMPATIVEL: server-only domain rules recovered from the supplied
// 7.59 server. No new packet, automatic calendar, or 7.48 parity is implied.
const (
	CastleWarConquestFame uint32 = 650
	CastleSealSteps              = 23 // sub_55EC40 completes when its counter exceeds 22.
)

type CastleWarPhase string

const (
	CastlePreparing CastleWarPhase = "preparing" // Reference state 1.
	CastleAssault   CastleWarPhase = "assault"   // Reference state 2: towers and breach actors.
	CastleSeal      CastleWarPhase = "seal"      // Reference state 3: seal capture enabled.
	CastleClosing   CastleWarPhase = "closing"   // Reference state 4: capture remains enabled.
	CastleFinished  CastleWarPhase = "finished"
	CastleCancelled CastleWarPhase = "cancelled"
)

func (s CastleWarState) active() bool {
	return s.Phase == CastlePreparing || s.Phase == CastleAssault || s.Phase == CastleSeal || s.Phase == CastleClosing
}

func (s CastleWarState) validate() error {
	if s.Day == "" {
		// The ownership-only schema remains loadable without a migration.
		if s.Phase != "" || !s.StartsAt.IsZero() || s.Captured {
			return fmt.Errorf("invalid empty castle round")
		}
		return nil
	}
	if _, err := time.Parse("2006-01-02", s.Day); err != nil || s.StartsAt.IsZero() ||
		s.StartsAt.Format("2006-01-02") != s.Day || s.StartsAt.Minute() != 0 ||
		s.StartsAt.Second() != 0 || s.StartsAt.Nanosecond() != 0 {
		return fmt.Errorf("invalid castle calendar")
	}
	if !s.active() && s.Phase != CastleFinished && s.Phase != CastleCancelled {
		return fmt.Errorf("invalid castle phase %q", s.Phase)
	}
	if s.Captured && s.Phase != CastleFinished {
		return fmt.Errorf("unfinished castle capture")
	}
	return nil
}

// BeginCastleWar accepts an already-authorized calendar decision. Channel/week
// selection belongs to the future runtime coordinator, not a guessed Sunday
// schedule here. The caller persists this snapshot before creating actors.
func (r *GuildRegistry) BeginCastleWar(start time.Time) error {
	s := &r.Wars.Castle
	if err := s.validate(); err != nil {
		return err
	}
	if start.IsZero() || start.Minute() != 0 || start.Second() != 0 || start.Nanosecond() != 0 {
		return fmt.Errorf("castle start must be an exact hour")
	}
	day := start.Format("2006-01-02")
	if s.Day == day {
		if !s.StartsAt.Equal(start) {
			return fmt.Errorf("castle round start changed")
		}
		return nil // Never reopen or reset a previously admitted round.
	}
	if s.active() || s.Day > day {
		return fmt.Errorf("castle round cannot be replaced")
	}
	s.Day, s.StartsAt, s.Phase, s.Captured = day, start, CastlePreparing, false
	return nil
}

// AdvanceCastleWar models the reference minute gates, not actor effects. A
// missed opening window cancels instead of spawning an unprepared late battle;
// an absolute deadline prevents a delayed coordinator from extending capture.
// These two fail-closed recovery rules are project hardening, not native parity.
func (r *GuildRegistry) AdvanceCastleWar(now time.Time) (bool, error) {
	s := &r.Wars.Castle
	if err := s.validate(); err != nil {
		return false, err
	}
	if !s.active() || now.Before(s.StartsAt) {
		return false, nil
	}
	elapsed := now.Sub(s.StartsAt)
	if s.Phase == CastlePreparing {
		if elapsed >= 10*time.Minute {
			s.Phase = CastleCancelled
			return true, nil
		}
		if elapsed >= 6*time.Minute {
			s.Phase = CastleAssault
			return true, nil
		}
		return false, nil
	}
	if elapsed >= 55*time.Minute {
		if s.Victories == math.MaxUint32 {
			return false, fmt.Errorf("castle victory counter overflow")
		}
		s.Victories++ // sub_55EAB0 also increments after a successful capture.
		s.Phase = CastleFinished
		return true, nil
	}
	if s.Phase == CastleAssault && elapsed >= 15*time.Minute {
		s.Phase = CastleSeal
		return true, nil
	}
	if s.Phase == CastleSeal && elapsed >= 50*time.Minute {
		s.Phase = CastleClosing
		return true, nil
	}
	return false, nil
}

// CastleSealProgress is transient, per-character progress. Never persist or
// restore it across logout, restart, or a different round. Only a server-owned
// sampling loop may call Observe; 23 client packets must never mean capture.
type CastleSealProgress struct {
	day       string
	guild     uint16
	character string
	steps     uint8
}

func (r *GuildRegistry) castleSealLeader(character string, now time.Time) *Guild {
	s := r.Wars.Castle
	if s.validate() != nil || (s.Phase != CastleSeal && s.Phase != CastleClosing) ||
		now.Before(s.StartsAt.Add(15*time.Minute)) || !now.Before(s.StartsAt.Add(55*time.Minute)) {
		return nil
	}
	g, member := r.FindByCharacter(character)
	if g == nil || member == nil || member.Rank != GuildRankLeader || g.ID == s.Owner {
		return nil
	}
	// ChargedGuildList has five slots: four city owners and the castle owner.
	// Bids/challengers and a matching kingdom alone do not authorize the seal.
	for _, territory := range r.Wars.Cities.Territories {
		if territory.Owner == g.ID {
			return g
		}
	}
	return nil
}

// Observe consumes one independently scheduled server sample. eligible must
// already include living/public-space/status validation in the coordinator.
// Every invalid sample resets progress, including an unauthorized owner change.
func (p *CastleSealProgress) Observe(r *GuildRegistry, character string, x, y uint16, eligible bool, now time.Time) bool {
	var g *Guild
	if eligible && x == 1046 && y == 1690 {
		g = r.castleSealLeader(character, now)
	}
	if g == nil {
		*p = CastleSealProgress{}
		return false
	}
	name := strings.ToLower(character)
	if p.day != r.Wars.Castle.Day || p.guild != g.ID || p.character != name {
		*p = CastleSealProgress{day: r.Wars.Castle.Day, guild: g.ID, character: name}
	}
	if p.steps < CastleSealSteps {
		p.steps++
	}
	return p.steps == CastleSealSteps
}

// CaptureCastle settles a completed server-owned sample on a registry snapshot.
// Fame, ownership and the terminal marker share one write. The coordinator
// must revalidate runtime identity/position/status before submitting and at ack.
func (r *GuildRegistry) CaptureCastle(progress CastleSealProgress, now time.Time) error {
	s := &r.Wars.Castle
	if s.Phase == CastleFinished && s.Captured && s.Day == progress.day && s.Owner == progress.guild {
		return nil // A retried acknowledgement cannot award the round twice.
	}
	g := r.castleSealLeader(progress.character, now)
	if g == nil || progress.day != s.Day || progress.steps != CastleSealSteps || progress.guild != g.ID {
		return fmt.Errorf("invalid castle seal capture")
	}
	if err := g.AddFame(CastleWarConquestFame); err != nil {
		return err // AddFame checks overflow before changing the balance.
	}
	if previous := r.FindByID(s.Owner); previous != nil {
		previous.Fame -= min(previous.Fame, CastleWarConquestFame)
	}
	// The reference resets the counter on conquest, then calls sub_55EAB0,
	// which increments it once. No gold, character fame, or item is created.
	s.Owner, s.Victories, s.Captured, s.Phase = g.ID, 1, true, CastleFinished
	return nil
}
