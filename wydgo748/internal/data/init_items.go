package data

import (
	"fmt"
	"strings"

	"wydgo/internal/model"
)

// LoadInitItems reads permanent world objects in the native InitItem.csv
// column order: index,x,y,rotation. It accepts trailing '#' comments.
// Validation fails startup for unknown items, invalid positions, or collisions.
func LoadInitItems(path string, items map[uint16]model.ItemDef) ([]model.InitItem, error) {
	var out []model.InitItem
	err := records(path, func(row []string) error {
		if len(row) < 4 {
			return fmt.Errorf("expected index,x,y,rotation")
		}
		values := make([]int, 4)
		for i := 0; i < 4; i++ {
			field := row[i]
			if comment := strings.IndexByte(field, '#'); comment >= 0 {
				field = field[:comment]
			}
			v, err := integer(field)
			if err != nil {
				return err
			}
			if v < 0 || v > 65535 {
				return fmt.Errorf("value out of range: %d", v)
			}
			values[i] = v
		}
		if values[3] > 255 {
			return fmt.Errorf("rotation %d exceeds the client packet byte range", values[3])
		}
		obj := model.InitItem{
			Index:  uint16(values[0]),
			X:      uint16(values[1]),
			Y:      uint16(values[2]),
			Rotate: byte(values[3]),
		}
		if err := obj.Validate(); err != nil {
			return err
		}
		definition, ok := items[obj.Index]
		if !ok {
			return fmt.Errorf("item %d is missing from the item catalog", obj.Index)
		}
		for _, effect := range definition.StaticEffects {
			if effect.Name == "EF_GROUND" && effect.Value > 0 && effect.Value < 10 && obj.Rotate > 3 {
				return fmt.Errorf("ground item %d rotation %d exceeds the client mask range 0..3", obj.Index, obj.Rotate)
			}
		}
		out = append(out, obj)
		return nil
	})
	if err != nil {
		return nil, err
	}
	// The client keeps only one ground item per grid cell.
	occupied := make(map[uint32]int, len(out))
	for i, obj := range out {
		key := uint32(obj.X)<<16 | uint32(obj.Y)
		if previous, duplicate := occupied[key]; duplicate {
			return nil, fmt.Errorf("data: %s: items %d and %d occupy the same cell (%d,%d)",
				path, out[previous].Index, obj.Index, obj.X, obj.Y)
		}
		occupied[key] = i
	}
	return out, nil
}
