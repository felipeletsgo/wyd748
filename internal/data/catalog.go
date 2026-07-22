package data

import (
	"encoding/csv"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"

	"wydgo/internal/model"
)

// LoadCatalog carrega as tres tabelas exportadas do client 7.48. Itemname
// substitui o nome tecnico do itemlist; todos os demais dados vem do itemlist.
func LoadCatalog(itemListPath, itemNamePath, skillPath string) (model.Catalog, error) {
	items, err := loadItemList(itemListPath)
	if err != nil {
		return model.Catalog{}, err
	}
	if err := loadItemNames(itemNamePath, items); err != nil {
		return model.Catalog{}, err
	}
	skills, err := loadSkills(skillPath)
	if err != nil {
		return model.Catalog{}, err
	}
	return model.Catalog{Items: items, Skills: skills}, nil
}

func records(path string, visit func([]string) error) error {
	f, err := os.Open(path)
	if err != nil {
		return err
	}
	defer f.Close()
	r := csv.NewReader(f)
	r.FieldsPerRecord = -1
	r.LazyQuotes = true
	for line := 1; ; line++ {
		row, err := r.Read()
		if err == io.EOF {
			return nil
		}
		if err != nil {
			return fmt.Errorf("data: %s linha %d: %w", path, line, err)
		}
		if len(row) == 0 || strings.HasPrefix(strings.TrimSpace(row[0]), "#") {
			continue
		}
		if err := visit(row); err != nil {
			return fmt.Errorf("data: %s linha %d: %w", path, line, err)
		}
	}
}

func integer(s string) (int, error) {
	v, err := strconv.Atoi(strings.TrimSpace(s))
	if err != nil {
		return 0, fmt.Errorf("inteiro invalido %q", s)
	}
	return v, nil
}

func dotted(s string, count int) ([]int, error) {
	parts := strings.Split(strings.TrimSpace(s), ".")
	result := make([]int, count)
	for i := 0; i < len(parts) && i < count; i++ {
		if parts[i] == "" {
			continue
		}
		v, err := integer(parts[i])
		if err != nil {
			return nil, err
		}
		result[i] = v
	}
	return result, nil
}

func loadItemList(path string) (map[uint16]model.ItemDef, error) {
	items := make(map[uint16]model.ItemDef)
	err := records(path, func(row []string) error {
		if len(row) < 9 {
			return fmt.Errorf("itemlist requer ao menos 9 colunas, recebeu %d", len(row))
		}
		idx, err := integer(row[0])
		if err != nil || idx <= 0 || idx > 65535 {
			return fmt.Errorf("index invalido %q", row[0])
		}
		mesh, err := dotted(row[2], 2)
		if err != nil {
			return err
		}
		req, err := dotted(row[3], 5)
		if err != nil {
			return err
		}
		unique, err := integer(row[4])
		if err != nil {
			return err
		}
		price, err := integer(row[5])
		if err != nil || price < 0 {
			return fmt.Errorf("preco invalido %q", row[5])
		}
		pos, err := integer(row[6])
		if err != nil {
			return err
		}
		extra, err := integer(row[7])
		if err != nil {
			return err
		}
		grade, err := integer(row[8])
		if err != nil {
			return err
		}
		def := model.ItemDef{Index: uint16(idx), Name: strings.TrimSpace(row[1]),
			Mesh: mesh[0], Texture: mesh[1], ReqLevel: req[0], ReqStr: req[1],
			ReqInt: req[2], ReqDex: req[3], ReqCon: req[4], Unique: unique,
			Price: uint32(price), Pos: pos, Extra: extra, Grade: grade}
		for i := 9; i+1 < len(row); i += 2 {
			name := strings.TrimSpace(row[i])
			if name == "" {
				continue
			}
			value, err := integer(row[i+1])
			if err != nil {
				return err
			}
			def.StaticEffects = append(def.StaticEffects, model.StaticEffect{Name: name, Value: value})
		}
		items[def.Index] = def
		return nil
	})
	return items, err
}

func loadItemNames(path string, items map[uint16]model.ItemDef) error {
	return records(path, func(row []string) error {
		if len(row) < 2 {
			return fmt.Errorf("Itemname requer index,nome")
		}
		idx, err := integer(row[0])
		if err != nil || idx <= 0 || idx > 65535 {
			return fmt.Errorf("index invalido %q", row[0])
		}
		if def, ok := items[uint16(idx)]; ok {
			def.Name = strings.TrimSpace(row[1])
			items[uint16(idx)] = def
		}
		return nil
	})
}

func loadSkills(path string) (map[int]model.SkillDef, error) {
	skills := make(map[int]model.SkillDef)
	err := records(path, func(row []string) error {
		if len(row) < 24 {
			return fmt.Errorf("SkillData requer 24 colunas, recebeu %d", len(row))
		}
		values := make([]int, 13)
		for i := range values {
			v, err := integer(row[i])
			if err != nil {
				return err
			}
			values[i] = v
		}
		act, err := dotted(row[13], 8)
		if err != nil {
			return err
		}
		actAlt, err := dotted(row[14], 8)
		if err != nil {
			return err
		}
		tail := make([]int, 8)
		for i := range tail {
			v, err := integer(row[15+i])
			if err != nil {
				return err
			}
			tail[i] = v
		}
		def := model.SkillDef{Index: values[0], SkillPoint: values[1], TargetType: values[2],
			ManaSpent: values[3], Delay: values[4], Range: values[5], InstanceType: values[6],
			InstanceValue: values[7], TickType: values[8], TickValue: values[9], AffectType: values[10],
			AffectValue: values[11], AffectTime: values[12] / 4, InstanceAttribute: tail[0],
			TickAttribute: tail[1], Aggressive: tail[2], MaxTarget: tail[3], Party: tail[4],
			AffectResist: tail[5], Passive: tail[6], Unknown: tail[7], Name: strings.TrimSpace(row[23])}
		for i := range def.Act {
			def.Act[i] = byte(act[i])
			def.ActAlt[i] = byte(actAlt[i])
		}
		normalizeTKBuff759(&def)
		skills[def.Index] = def
		return nil
	})
	return skills, err
}

// normalizeTKBuff759 conserva os indices/nomes reposicionados do client 7.48,
// mas substitui a semantica dos buffs TK pelos campos do SkillData 7.59. O CSV
// local e cp1252 e permanece como fonte visual intacta.
func normalizeTKBuff759(def *model.SkillDef) {
	switch def.Index {
	case 3: // Samaritano, reposicionado no 7.48
		def.TargetType, def.ManaSpent, def.Delay, def.Range = 0, 25, 1, 5
		def.InstanceType, def.InstanceValue = 0, 0
		def.TickType, def.TickValue = 0, 0
		def.AffectType, def.AffectValue, def.AffectTime = 24, 0, 45/4
		def.Aggressive, def.MaxTarget = 0, 1
	case 13: // Possuido, reposicionado no 7.48
		def.TargetType, def.ManaSpent, def.Delay, def.Range = 0, 105, 0, 0
		def.InstanceType, def.InstanceValue = 0, 0
		def.TickType, def.TickValue = 0, 0
		def.AffectType, def.AffectValue, def.AffectTime = 14, 10, 99/4
		def.Aggressive, def.MaxTarget = 0, 1
	case 5: // Aura da Vida
		def.TargetType, def.ManaSpent, def.Delay, def.Range = 0, 53, 0, 0
		def.InstanceType, def.InstanceValue = 0, 0
		def.TickType, def.TickValue = 17, 75
		def.AffectType, def.AffectValue, def.AffectTime = 0, 0, 99/4
		def.Aggressive, def.MaxTarget = 0, 1
	case 11: // Assalto
		def.TargetType, def.ManaSpent, def.Delay, def.Range = 0, 47, 0, 0
		def.InstanceType, def.InstanceValue = 0, 0
		def.TickType, def.TickValue = 0, 0
		def.AffectType, def.AffectValue, def.AffectTime = 13, 7, 99/4
		def.Aggressive, def.MaxTarget = 0, 1
	case 15: // Armadura Critica
		def.TargetType, def.ManaSpent, def.Delay, def.Range = 0, 150, 25, 0
		def.InstanceType, def.InstanceValue = 0, 0
		def.TickType, def.TickValue = 0, 0
		def.AffectType, def.AffectValue, def.AffectTime = 31, 150, 75/4
		def.Aggressive, def.MaxTarget = 0, 1
	}
}
