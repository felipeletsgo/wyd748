package model

import "fmt"

// Colosseum tiers follow the 7.59 TMSRV hours EventHour+1 (N), +2 (M) and
// +3 (A); each tier owns its generator file under data/NPCGener.
const (
	ColosseumTierN = iota
	ColosseumTierM
	ColosseumTierA
	ColosseumTiers
)

// ColosseumConfig holds the 7.59 ServerInit->EventHour (default 19). The
// event uses the server-local timezone shared with the guild wars.
type ColosseumConfig struct {
	Enabled   bool
	EventHour uint32
}

func DefaultColosseumConfig() ColosseumConfig {
	return ColosseumConfig{Enabled: true, EventHour: 19}
}

func (c ColosseumConfig) Validate() error {
	// The third tier starts at EventHour+3 and must stay within the same day.
	if c.EventHour > 20 {
		return fmt.Errorf("colosseum event hour must be 0..20")
	}
	return nil
}

// ColosseumGenerators are the event-only generators of each tier, keyed by
// their 7.59 NPCGener label (the GenerateMob argument).
type ColosseumGenerators [ColosseumTiers][]NPCGener
