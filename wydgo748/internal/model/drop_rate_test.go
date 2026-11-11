package model

import (
	"fmt"
	"testing"
)

func TestDropRateForSlotLevelBands(t *testing.T) {
	base := [MaxCarry]int{}
	base[0] = 10_000
	tests := []struct {
		level int
		want  int
	}{
		{-1, 400},
		{9, 400},
		{10, 500},
		{19, 500},
		{20, 600},
		{29, 600},
		{30, 700},
		{39, 700},
		{40, 800},
		{59, 800},
		{60, 9_900},
	}
	for _, test := range tests {
		t.Run(fmt.Sprintf("level_%d", test.level), func(t *testing.T) {
			got, guaranteed := DropRateForSlot(base, 0, test.level)
			if got != test.want || guaranteed {
				t.Fatalf("DropRateForSlot(level=%d) = (%d, %t), want (%d, false)", test.level, got, guaranteed, test.want)
			}
		})
	}
}

func TestDropRateForSlotSpecialAndUnadjustedSlots(t *testing.T) {
	base := [MaxCarry]int{}
	base[8], base[9], base[10] = 10, 0, 80
	base[11] = 123
	base[24] = 123
	for slot, want := range map[int]int{8: 4, 9: 4, 10: 4, 24: 123} {
		got, guaranteed := DropRateForSlot(base, slot, 1)
		if got != want || guaranteed {
			t.Errorf("slot %d = (%d, %t), want (%d, false)", slot, got, guaranteed, want)
		}
	}
	if got, guaranteed := DropRateForSlot(base, 11, 100); got != 0 || !guaranteed {
		t.Fatalf("guaranteed slot 11 = (%d, %t), want (0, true)", got, guaranteed)
	}
}

func TestDropRateForHighSlotsAndCap(t *testing.T) {
	base := [MaxCarry]int{}
	base[60] = 10_000
	for _, test := range []struct {
		level int
		want  int
	}{
		{169, 9_000},
		{170, 6_000},
		{200, 5_000},
		{230, 4_300},
		{255, 3_800},
		{320, 5_000},
	} {
		got, guaranteed := DropRateForSlot(base, 60, test.level)
		if got != test.want || guaranteed {
			t.Errorf("slot 60 level %d = (%d, %t), want (%d, false)", test.level, got, guaranteed, test.want)
		}
	}
	base[24] = 32_001
	if got, guaranteed := DropRateForSlot(base, 24, 10); got != 32_000 || guaranteed {
		t.Fatalf("capped rate = (%d, %t), want (32000, false)", got, guaranteed)
	}
}
