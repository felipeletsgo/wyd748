package data

import (
	"bufio"
	"encoding/csv"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"wydgo/internal/model"
)

// LoadCatalog loads the authoritative catalog sources. Itemname replaces only
// the itemlist name; ItemEffect translates the persisted effect IDs; every
// skill field comes from SkillData.
func LoadCatalog(itemListPath, itemNamePath string, paths ...string) (model.Catalog, error) {
	var itemEffectPath, skillPath string
	switch len(paths) {
	case 1:
		// Compatibility for internal consumers: ItemEffect.h stays mandatory
		// and is found next to the itemlist.
		itemEffectPath = filepath.Join(filepath.Dir(itemListPath), "ItemEffect.h")
		skillPath = paths[0]
	case 2:
		itemEffectPath, skillPath = paths[0], paths[1]
	default:
		return model.Catalog{}, fmt.Errorf("LoadCatalog requires SkillData and, optionally, an explicit ItemEffect")
	}
	effects, err := loadItemEffects(itemEffectPath)
	if err != nil {
		return model.Catalog{}, err
	}
	items, err := loadItemList(itemListPath)
	if err != nil {
		return model.Catalog{}, err
	}
	if err := validateStaticEffects(items, effects); err != nil {
		return model.Catalog{}, err
	}
	if err := loadItemNames(itemNamePath, items); err != nil {
		return model.Catalog{}, err
	}
	for index, def := range items {
		def.DynamicEffectNames = effects
		items[index] = def
	}
	skills, err := loadSkills(skillPath)
	if err != nil {
		return model.Catalog{}, err
	}
	return model.Catalog{Items: items, Skills: skills, ItemEffects: effects}, nil
}

func validateStaticEffects(items map[uint16]model.ItemDef, effects map[byte]string) error {
	known := make(map[string]struct{}, len(effects))
	for _, name := range effects {
		known[name] = struct{}{}
	}
	for index, def := range items {
		for i, effect := range def.StaticEffects {
			if _, ok := known[effect.Name]; !ok {
				// The 7.48 item list writes some effects as a numeric ID (for
				// example 111, the client's EF_NOTRADE). When ItemEffect.h names
				// that ID, use the name so rules that look an effect up by name
				// (trade, refine, consumables) see it; otherwise keep the number.
				if numeric, err := strconv.Atoi(effect.Name); err == nil && numeric > 0 && numeric <= 255 {
					if name, named := effects[byte(numeric)]; named {
						def.StaticEffects[i].Name = name
					}
					continue
				}
				return fmt.Errorf("itemlist item %d uses effect %q missing from ItemEffect.h", index, effect.Name)
			}
		}
		items[index] = def
	}
	return nil
}

func loadItemEffects(path string) (map[byte]string, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("open ItemEffect %q: %w", path, err)
	}
	defer file.Close()

	effects := make(map[byte]string)
	names := make(map[string]byte)
	scanner := bufio.NewScanner(file)
	for line := 1; scanner.Scan(); line++ {
		fields := strings.Fields(strings.TrimSpace(scanner.Text()))
		if len(fields) < 3 || fields[0] != "#define" || !strings.HasPrefix(fields[1], "EF_") {
			continue
		}
		value, err := strconv.Atoi(fields[2])
		if err != nil || value <= 0 || value > 255 {
			return nil, fmt.Errorf("ItemEffect %s line %d: invalid ID %q", path, line, fields[2])
		}
		id, name := byte(value), fields[1]
		if previous, ok := effects[id]; ok && previous != name {
			return nil, fmt.Errorf("ItemEffect %s line %d: duplicate ID %d (%s/%s)", path, line, id, previous, name)
		}
		if previous, ok := names[name]; ok && previous != id {
			return nil, fmt.Errorf("ItemEffect %s line %d: duplicate effect %s (%d/%d)", path, line, name, previous, id)
		}
		effects[id], names[name] = name, id
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("read ItemEffect %q: %w", path, err)
	}
	if len(effects) == 0 {
		return nil, fmt.Errorf("ItemEffect %q has no EF_* definitions", path)
	}
	return effects, nil
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
			return fmt.Errorf("data: %s line %d: %w", path, line, err)
		}
		if len(row) == 0 || strings.HasPrefix(strings.TrimSpace(row[0]), "#") {
			continue
		}
		if err := visit(row); err != nil {
			return fmt.Errorf("data: %s line %d: %w", path, line, err)
		}
	}
}

func integer(s string) (int, error) {
	v, err := strconv.Atoi(strings.TrimSpace(s))
	if err != nil {
		return 0, fmt.Errorf("invalid integer %q", s)
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
			return fmt.Errorf("itemlist needs at least 9 columns, got %d", len(row))
		}
		idx, err := integer(row[0])
		if err != nil || idx <= 0 || idx >= model.ItemListSize {
			return fmt.Errorf("invalid index %q", row[0])
		}
		if _, duplicate := items[uint16(idx)]; duplicate {
			return fmt.Errorf("itemlist has duplicate index %d", idx)
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
			return fmt.Errorf("invalid price %q", row[5])
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
			return fmt.Errorf("itemname needs index,name")
		}
		idx, err := integer(row[0])
		if err != nil || idx <= 0 || idx >= model.ItemListSize {
			return fmt.Errorf("invalid index %q", row[0])
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
		// 7.48 layout: Id + 12 parameters + Act + ActAlt + 7 final
		// attributes + Name = 23 columns. The Unknown field found in some
		// exports of later versions is not part of this source.
		if len(row) != 23 {
			return fmt.Errorf("SkillData needs exactly 23 columns, got %d", len(row))
		}
		values := make([]int, 13)
		for i := range values {
			v, err := integer(row[i])
			if err != nil {
				return err
			}
			values[i] = v
		}
		if values[0] < 0 || values[0] >= model.SkillListSize {
			return fmt.Errorf("SkillData has an index outside the 7.48 table: %d", values[0])
		}
		act, err := dotted(row[13], 8)
		if err != nil {
			return err
		}
		actAlt, err := dotted(row[14], 8)
		if err != nil {
			return err
		}
		if _, duplicate := skills[values[0]]; duplicate {
			return fmt.Errorf("SkillData has duplicate index %d", values[0])
		}
		for i := range act {
			if act[i] < 0 || act[i] > 255 || actAlt[i] < 0 || actAlt[i] > 255 {
				return fmt.Errorf("SkillData skill %d has Act[%d] outside a byte", values[0], i)
			}
		}
		tail := make([]int, 7)
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
			AffectResist: tail[5], Passive: tail[6], Name: strings.TrimSpace(row[22])}
		for i := range def.Act {
			def.Act[i] = byte(act[i])
			def.ActAlt[i] = byte(actAlt[i])
		}
		skills[def.Index] = def
		return nil
	})
	return skills, err
}
