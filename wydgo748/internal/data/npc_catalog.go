package data

import (
	"fmt"

	"wydgo/internal/model"
)

// CloneNPCs gives each channel independent mutable template data.
func CloneNPCs(source []model.NPCDef) []model.NPCDef {
	result := append([]model.NPCDef(nil), source...)
	for i := range result {
		if source[i].Score != nil {
			score := *source[i].Score
			result[i].Score = &score
		}
		if source[i].ClientMerchant != nil {
			merchant := *source[i].ClientMerchant
			result[i].ClientMerchant = &merchant
		}
		result[i].Carry = append([]model.Item(nil), source[i].Carry...)
		result[i].Vende = append([]model.Item(nil), source[i].Vende...)
	}
	return result
}

// ValidateNPCGenerators rejects enabled spawn records without an exact template.
func ValidateNPCGenerators(npcs []model.NPCDef, generators []model.NPCGener) error {
	known := make(map[string]bool, len(npcs))
	for _, npc := range npcs {
		known[npc.Name] = true
	}
	for _, g := range generators {
		if !g.Enabled {
			continue
		}
		for _, name := range []string{g.Leader, g.Follower} {
			if !known[name] {
				return fmt.Errorf("NPCGener[%d]: missing template %q", g.Index, name)
			}
		}
	}
	return nil
}

// ValidateNPCItems checks equipment, drops, and stock, including unplaced templates.
func ValidateNPCItems(npcs []model.NPCDef, items map[uint16]model.ItemDef) error {
	for _, npc := range npcs {
		equip := npc.Equip.Items()
		for _, group := range [][]model.Item{equip[:], npc.Carry, npc.Vende} {
			for _, item := range group {
				if item.Index != 0 {
					if _, ok := items[item.Index]; !ok {
						return fmt.Errorf("NPC catalog: template %q references unsupported item %d", npc.Name, item.Index)
					}
				}
			}
		}
	}
	return nil
}
