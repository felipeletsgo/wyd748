package game

import (
	"errors"
	"wydgo/internal/model"
)

// MODERNIZACAO_COMPATIVEL: supplied TMSRVIN dword_647968, first 64 values.
// Rows select result 1744..1751; columns select source 1752..1759.
var archStoneThresholds = [8][8]int{
	{56, 21, 3, 3, 0, 0, 0, 0},
	{80, 76, 21, 10, 0, 0, 0, 0},
	{92, 86, 76, 25, 0, 0, 0, 0},
	{95, 90, 85, 80, 0, 0, 0, 0},
	{0, 0, 0, 0, 50, 9, 2, 2},
	{0, 0, 0, 0, 62, 59, 8, 5},
	{0, 0, 0, 0, 68, 63, 58, 10},
	{0, 0, 0, 0, 70, 65, 62, 60},
}

func isArchMonsterStone(index uint16) bool { return index >= 1752 && index <= 1759 }

func archStoneResult(index uint16, roll int) uint16 {
	if !isArchMonsterStone(index) || roll <= 1 || roll > 100 {
		return 0
	}
	for result, row := range archStoneThresholds {
		if roll <= row[index-1752] {
			return uint16(1744 + result)
		}
	}
	return 0
}

func (w *World) convertArchMonsterStone(target *model.Item, powderCode int) (percentRoll, error) {
	if target == nil || !isArchMonsterStone(target.Index) || powderCode != 4 && powderCode != 5 {
		return percentRoll{}, errors.New("invalid Arch stone conversion")
	}
	roll := percentRoll{Roll: w.intn(100) + 1}
	for _, row := range archStoneThresholds {
		roll.Chance = maxInt(roll.Chance, row[target.Index-1752])
	}
	result := archStoneResult(target.Index, roll.Roll)
	if result != 0 {
		if _, exists := w.items[result]; !exists {
			return roll, errors.New("Arch stone result is absent from the catalog")
		}
		target.Index, roll.Success = result, true
	}
	return roll, nil
}
