package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
)

// Keep distinct event variants independent from project-calibrated bosses.
// Reuse identical canonical definitions. This path uses TMSRV/run.
func runHellGate(source, output string) error {
	if err := os.MkdirAll(output, 0755); err != nil {
		return err
	}
	manifest := map[string]string{}
	for _, pair := range [][2]string{
		{"Skeleton_", "HG_CorrSkel"}, {"SkeletonArcher", "HG_SkelArch"},
		{"SkeletonMage", "HG_SkelMage"}, {"Guer_Caveira", "Guer_Caveira"},
		{"SpearSkeleton", "HG_SkelSpear"}, {"EsqGuerreiro", "HG_SkelGuard"},
	} {
		b, err := os.ReadFile(filepath.Join(source, "npc", pair[0]))
		if err != nil {
			return err
		}
		d, err := decode(pair[1], b)
		if err != nil {
			return err
		}
		data, err := json.MarshalIndent(d, "", "  ")
		if err != nil {
			return err
		}
		if err := os.WriteFile(filepath.Join(output, d.Name+".json"), append(data, '\n'), 0644); err != nil {
			return err
		}
		hash := sha256.Sum256(b)
		manifest[pair[0]] = hex.EncodeToString(hash[:])
	}
	data, err := json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		return err
	}
	if err := os.WriteFile(filepath.Join(output, "sources.json"), append(data, '\n'), 0644); err != nil {
		return err
	}
	fmt.Println("Extracted six event-only Hell Gate infantry definitions.")
	return nil
}
