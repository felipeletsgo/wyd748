// Package data carrega os dados estaticos do jogo (NPCs, itens, config) de
// arquivos JSON legiveis no boot. Formato editavel a mao -- nada de binario.
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

// LoadNPCs le os NPCs de "path". Se path for uma PASTA, cada arquivo .json e um
// NPC (um objeto por arquivo, igual as contas em data/accounts/).
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

// loadNPCDir le todos os *.json da pasta, um NPC por arquivo, em ordem estavel.
func loadNPCDir(dir string) ([]model.NPCDef, error) {
	files, err := filepath.Glob(filepath.Join(dir, "*.json"))
	if err != nil {
		return nil, err
	}
	sort.Strings(files) // ordem deterministica -> ClientId estavel entre boots
	var npcs []model.NPCDef
	for _, f := range files {
		part, err := parseNPCFile(f)
		if err != nil {
			return nil, err
		}
		npcs = append(npcs, part...)
	}
	return npcs, nil
}

// parseNPCFile decodifica exatamente um NPC por arquivo.
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
			err = fmt.Errorf("conteudo JSON adicional")
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
			return fmt.Errorf("data: %s NPC[%d] sem nome", path, i)
		}
		if npc.Tipo == "" {
			npc.Tipo = model.TipoNPC
		}
		if npc.Tipo != model.TipoNPC && npc.Tipo != model.TipoMonstro {
			return fmt.Errorf("data: %s NPC[%d] %q possui tipo invalido %q",
				path, i, npc.Name, npc.Tipo)
		}
		if len(npcs[i].Carry) > model.MaxCarry {
			return fmt.Errorf("data: %s NPC[%d] %q possui %d slots de carry; maximo %d",
				path, i, npcs[i].Name, len(npcs[i].Carry), model.MaxCarry)
		}
		if len(npc.Vende) > model.MaxCarry {
			return fmt.Errorf("data: %s NPC[%d] %q possui %d slots de venda; maximo %d",
				path, i, npc.Name, len(npc.Vende), model.MaxCarry)
		}
		if npc.Tipo == model.TipoMonstro && len(npc.Vende) != 0 {
			return fmt.Errorf("data: %s NPC[%d] %q e monstro mas possui estoque de loja",
				path, i, npc.Name)
		}
		if npc.Tipo == model.TipoNPC && len(npc.Carry) != 0 {
			return fmt.Errorf("data: %s NPC[%d] %q e npc mas possui tabela de drop",
				path, i, npc.Name)
		}
		for slot, item := range npc.Equip.Items() {
			if item.UID != "" {
				return fmt.Errorf("data: %s NPC[%d] %q equip[%d] possui UID de instancia",
					path, i, npc.Name, slot)
			}
			if int(item.Index) >= model.ItemListSize {
				return fmt.Errorf("data: %s NPC[%d] %q equip[%d]=%d excede ItemList",
					path, i, npc.Name, slot, item.Index)
			}
		}
		for kind, items := range map[string][]model.Item{"carry": npc.Carry, "vende": npc.Vende} {
			for slot, item := range items {
				if item.UID != "" {
					return fmt.Errorf("data: %s NPC[%d] %q %s[%d] possui UID de instancia",
						path, i, npc.Name, kind, slot)
				}
				if int(item.Index) >= model.ItemListSize {
					return fmt.Errorf("data: %s NPC[%d] %q %s[%d]=%d excede ItemList",
						path, i, npc.Name, kind, slot, item.Index)
				}
			}
		}
		if npc.Score == nil {
			return fmt.Errorf("data: %s NPC[%d] %q sem extendedScore", path, i, npc.Name)
		}
		score := npc.Score
		if err := score.Validate(); err != nil {
			return fmt.Errorf("data: %s NPC[%d] %q: %w", path, i, npc.Name, err)
		}
		if npc.Direction&0x0F != 0 {
			return fmt.Errorf("data: %s NPC[%d] %q possui direction invalido 0x%02X",
				path, i, npc.Name, npc.Direction)
		}
		if score.ResistFire > 255 || score.ResistIce > 255 ||
			score.ResistHoly > 255 || score.ResistThunder > 255 {
			return fmt.Errorf("data: %s NPC[%d] %q possui resistencia fora de byte",
				path, i, npc.Name)
		}
	}
	return nil
}
