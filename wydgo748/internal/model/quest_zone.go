package model

import "fmt"

// QuestZoneFileVersion follows the strict contract of the other data files:
// exact version, no unknown field, no trailing content.
const QuestZoneFileVersion = 1

// QuestZone is a world rectangle (borders inclusive) emptied by the periodic
// reset: every player inside is returned to town each cycle. It ports the
// WYD 7.48 ClearArea mechanism (level quest areas expel the player after about
// ten minutes, reviving a dead player).
type QuestZone struct {
	Name string `json:"name"`
	// NPC explicitly binds the entry guard; Name remains the zone identifier.
	NPC string `json:"npc,omitempty"`
	X1  uint16 `json:"x1"`
	Y1  uint16 `json:"y1"`
	X2  uint16 `json:"x2"`
	Y2  uint16 `json:"y2"`
}

// EntryNPC retains the legacy Name binding when an older profile omits NPC.
func (z QuestZone) EntryNPC() string {
	if z.NPC != "" {
		return z.NPC
	}
	return z.Name
}

// Contains reports whether the tile is inside the rectangle, borders included,
// like the native `>=`/`<=` comparisons.
func (z QuestZone) Contains(x, y uint16) bool {
	return x >= z.X1 && x <= z.X2 && y >= z.Y1 && y <= z.Y2
}

func (z *QuestZone) Validate() error {
	if z == nil {
		return fmt.Errorf("missing quest zone")
	}
	if z.Name == "" {
		return fmt.Errorf("quest zone without a name")
	}
	if z.X2 < z.X1 || z.Y2 < z.Y1 {
		return fmt.Errorf("zone %q has an inverted rectangle (%d,%d)-(%d,%d)",
			z.Name, z.X1, z.Y1, z.X2, z.Y2)
	}
	return nil
}

// QuestZoneFile is the content of data/content/quest_zones.json. Zones are quest areas
// guarded by their NPC entry (ClearAreaQuest). ClearAreas are emptied by the
// same reset cycle without an entry guard (native ClearArea, such as the Game
// Room territories).
type QuestZoneFile struct {
	Version    int         `json:"version"`
	Zones      []QuestZone `json:"zones"`
	ClearAreas []QuestZone `json:"clearAreas,omitempty"`
}

func (f *QuestZoneFile) Validate() error {
	if f == nil {
		return fmt.Errorf("missing zone file")
	}
	if f.Version != QuestZoneFileVersion {
		return fmt.Errorf("zone file version %d; want %d", f.Version, QuestZoneFileVersion)
	}
	names := make(map[string]struct{}, len(f.Zones)+len(f.ClearAreas))
	for _, zones := range [][]QuestZone{f.Zones, f.ClearAreas} {
		for i := range zones {
			if err := zones[i].Validate(); err != nil {
				return err
			}
			if _, dup := names[zones[i].Name]; dup {
				return fmt.Errorf("duplicate zone %q", zones[i].Name)
			}
			names[zones[i].Name] = struct{}{}
		}
	}
	return nil
}
