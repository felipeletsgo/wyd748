// Package data loads static game definitions from editable JSON files at boot.
package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"

	"wydgo/internal/model"
)

// LoadNPCs reads a file or a directory containing one NPC per JSON file.
func LoadNPCs(path string) ([]model.NPCDef, error) {
	info, err := os.Stat(path)
	if err != nil {
		return nil, err
	}
	if info.IsDir() {
		return loadNPCDir(path)
	}
	npc, err := parseNPCFile(path)
	return npc, err
}

// loadNPCDir reads JSON files in stable order.
func loadNPCDir(dir string) ([]model.NPCDef, error) {
	files, err := filepath.Glob(filepath.Join(dir, "*.json"))
	if err != nil {
		return nil, err
	}
	sort.Strings(files) // deterministic ordering keeps IDs stable across boots
	var npcs []model.NPCDef
	seen := make(map[string]string, len(files))
	for _, f := range files {
		part, err := parseNPCFile(f)
		if err != nil {
			return nil, err
		}
		for _, npc := range part {
			if previous, exists := seen[npc.Name]; exists {
				return nil, fmt.Errorf("data: duplicate template %q in %s and %s", npc.Name, previous, f)
			}
			seen[npc.Name] = f
			npcs = append(npcs, npc)
		}
	}
	return npcs, nil
}

// parseNPCFile decodes exactly one NPC per file.
func parseNPCFile(path string) ([]model.NPCDef, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var one model.NPCDef
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&one); err != nil {
		return nil, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			err = fmt.Errorf("additional JSON content")
		}
		return nil, fmt.Errorf("data: parse %s: %w", path, err)
	}
	npcs := []model.NPCDef{one}
	return npcs, validateNPCs(path, npcs)
}

func validateNPCs(path string, npcs []model.NPCDef) error {
	for i := range npcs {
		npc := &npcs[i]
		if npc.Name == "" {
			return fmt.Errorf("data: %s NPC[%d] has no name", path, i)
		}
		if len(npc.VisibleName()) > model.MaxNPCDisplayNameBytes {
			return fmt.Errorf("data: %s NPC[%d] %q display name exceeds %d bytes", path, i, npc.Name, model.MaxNPCDisplayNameBytes)
		}
		for _, b := range []byte(npc.VisibleName()) {
			if b < 32 || b > 126 {
				return fmt.Errorf("data: %s NPC[%d] %q display name contains an unsupported byte", path, i, npc.Name)
			}
		}
		if npc.Tipo == "" {
			npc.Tipo = model.TipoNPC
		}
		if npc.Tipo != model.TipoNPC && npc.Tipo != model.TipoMonstro {
			return fmt.Errorf("data: %s NPC[%d] %q has invalid type %q",
				path, i, npc.Name, npc.Tipo)
		}
		if len(npcs[i].Carry) > model.MaxCarry {
			return fmt.Errorf("data: %s NPC[%d] %q has %d carry slots; maximum %d",
				path, i, npcs[i].Name, len(npcs[i].Carry), model.MaxCarry)
		}
		if len(npc.Vende) > model.MaxCarry {
			return fmt.Errorf("data: %s NPC[%d] %q has %d shop slots; maximum %d",
				path, i, npc.Name, len(npc.Vende), model.MaxCarry)
		}
		if npc.Tipo == model.TipoMonstro && len(npc.Vende) != 0 {
			return fmt.Errorf("data: %s NPC[%d] %q is a monster with shop stock",
				path, i, npc.Name)
		}
		if npc.Tipo == model.TipoNPC && len(npc.Carry) != 0 {
			return fmt.Errorf("data: %s NPC[%d] %q is an NPC with monster drops",
				path, i, npc.Name)
		}
		for slot, item := range npc.Equip.Items() {
			if item.UID != "" {
				return fmt.Errorf("data: %s NPC[%d] %q equip[%d] has an instance UID",
					path, i, npc.Name, slot)
			}
			if int(item.Index) >= model.ItemListSize {
				return fmt.Errorf("data: %s NPC[%d] %q equip[%d]=%d exceeds ItemList",
					path, i, npc.Name, slot, item.Index)
			}
		}
		for kind, items := range map[string][]model.Item{"carry": npc.Carry, "vende": npc.Vende} {
			for slot, item := range items {
				if item.UID != "" {
					return fmt.Errorf("data: %s NPC[%d] %q %s[%d] has an instance UID",
						path, i, npc.Name, kind, slot)
				}
				if int(item.Index) >= model.ItemListSize {
					return fmt.Errorf("data: %s NPC[%d] %q %s[%d]=%d exceeds ItemList",
						path, i, npc.Name, kind, slot, item.Index)
				}
			}
		}
		if npc.Score == nil {
			return fmt.Errorf("data: %s NPC[%d] %q has no score", path, i, npc.Name)
		}
		score := npc.Score
		if err := score.Validate(); err != nil {
			return fmt.Errorf("data: %s NPC[%d] %q: %w", path, i, npc.Name, err)
		}
		if npc.Direction&0x0F != 0 {
			return fmt.Errorf("data: %s NPC[%d] %q has invalid direction 0x%02X",
				path, i, npc.Name, npc.Direction)
		}
		if npc.ClientMerchant != nil && (npc.Tipo != model.TipoNPC || *npc.ClientMerchant > 15) {
			return fmt.Errorf("data: %s NPC[%d] %q has an invalid client merchant override", path, i, npc.Name)
		}
		if score.ResistFire > 255 || score.ResistIce > 255 ||
			score.ResistHoly > 255 || score.ResistThunder > 255 {
			return fmt.Errorf("data: %s NPC[%d] %q has resistance outside the byte range",
				path, i, npc.Name)
		}
	}
	return nil
}

// ValidateNPCDefinition applies the same strict schema and field checks used
// for source files, then validates every equipment, carry, and shop item.
func ValidateNPCDefinition(npc model.NPCDef, items map[uint16]model.ItemDef) (model.NPCDef, error) {
	validated := []model.NPCDef{npc}
	if err := validateNPCs("PostgreSQL NPC definition", validated); err != nil {
		return model.NPCDef{}, err
	}
	if err := ValidateNPCItems(validated, items); err != nil {
		return model.NPCDef{}, err
	}
	return validated[0], nil
}
