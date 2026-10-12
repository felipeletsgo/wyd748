package data

import (
	"encoding/json"
	"fmt"
	"os"
	"strconv"

	"wydgo/internal/model"
)

// LoadMounts reads data/mounts.json: base attributes per mount TYPE (0..29),
// ported from the TMProject g_pMountBonus[30][6] table, and the timed premium
// horses keyed by item index. A missing file is not an error: it yields an
// empty catalog and mounts simply add no bonus.
func LoadMounts(path string) (model.MountCatalog, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		if os.IsNotExist(err) {
			return model.MountCatalog{Types: map[int]model.MountStats{}}, nil
		}
		return model.MountCatalog{}, err
	}
	var file struct {
		Types   map[int]model.MountStats           `json:"types"`
		Premium map[string]model.PremiumMountStats `json:"premium"`
	}
	if err := json.Unmarshal(b, &file); err != nil {
		return model.MountCatalog{}, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if file.Types == nil {
		file.Types = map[int]model.MountStats{}
	}
	for t := range file.Types {
		if t < 0 || t >= model.MountTypeCount {
			return model.MountCatalog{}, fmt.Errorf("data: %s invalid mount type %d (0..%d)", path, t, model.MountTypeCount-1)
		}
	}
	premium := make(map[uint16]model.PremiumMountStats, len(file.Premium))
	for key, stats := range file.Premium {
		index, err := strconv.ParseUint(key, 10, 16)
		if err != nil || index < 3980 || index > 3999 {
			return model.MountCatalog{}, fmt.Errorf("data: %s invalid premium horse %q (3980..3999)", path, key)
		}
		if stats.Speed < 1 || stats.Speed > 6 || stats.DurationDays <= 0 || stats.Attack < 0 ||
			stats.MagicPercent < 0 || stats.Evasion < 0 || stats.Resist < 0 {
			return model.MountCatalog{}, fmt.Errorf("data: %s premium horse %d needs speed 1..6, a positive lifetime and non-negative stats", path, index)
		}
		premium[uint16(index)] = stats
	}
	return model.MountCatalog{Types: file.Types, Premium: premium}, nil
}
