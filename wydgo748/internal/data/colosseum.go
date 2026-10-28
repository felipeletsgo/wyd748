package data

import (
	"fmt"
	"path/filepath"
	"slices"

	"wydgo/internal/model"
)

// colosseumFiles lists each tier's generator file and the labels the 7.59
// TMSRV scheduler (sub_567780) passes to GenerateMob for that tier.
var colosseumFiles = [model.ColosseumTiers]struct {
	name   string
	labels []int
}{
	model.ColosseumTierN: {"ColiseuN.txt", []int{5, 6, 7}},
	model.ColosseumTierM: {"ColiseuM.txt", []int{0, 1, 2}},
	model.ColosseumTierA: {"ColiseuA.txt", []int{103, 104, 105, 106}},
}

// LoadColosseum reads Coliseu{N,M,A}.txt from the instance generator directory.
func LoadColosseum(generatorsDir string) (model.ColosseumGenerators, error) {
	var out model.ColosseumGenerators
	for tier, file := range colosseumFiles {
		path := filepath.Join(generatorsDir, file.name)
		geners, err := LoadNPCGener(path)
		if err != nil {
			return out, err
		}
		var labels []int
		for _, g := range geners {
			if !g.Enabled {
				return out, fmt.Errorf("%s: generator %d must be enabled", path, g.Label)
			}
			if g.MinuteGenerate != -1 {
				return out, fmt.Errorf("%s: generator %d must be event-only (MinuteGenerate -1)", path, g.Label)
			}
			labels = append(labels, g.Label)
		}
		if !slices.Equal(labels, file.labels) {
			return out, fmt.Errorf("%s: labels %v, want %v", path, labels, file.labels)
		}
		out[tier] = geners
	}
	return out, nil
}
