package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestDionys759AccessoryLotteryCoversRareAndTieredResults(t *testing.T) {
	tests := []struct {
		name  string
		rolls []int
		want  model.Item
	}{
		{name: "rare amulet one", rolls: []int{0}, want: model.Item{Index: 540}},
		{name: "rare amulet two", rolls: []int{100}, want: model.Item{Index: 541}},
		{
			name:  "basic tier accessory",
			rolls: []int{200, 0, 0, 0, 0, 0, 0, 0},
			want:  model.Item{Index: 551, Eff: [6]byte{43, 0, 4, 20, 4, 20}},
		},
		{
			name:  "second tier mixed options",
			rolls: []int{4200, 7000, 2200, 0, 0, 7600, 0, 0},
			want:  model.Item{Index: 558, Eff: [6]byte{43, 0, 2, 10, 26, 9}},
		},
		{
			name:  "third tier mastery accessory",
			rolls: []int{7200, 4500, 8000, 0, 0, 7600, 0, 0},
			want:  model.Item{Index: 561, Eff: [6]byte{43, 0, 26, 9, 26, 9}},
		},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			rng := &sequenceRNG{values: append([]int(nil), tc.rolls...)}
			got := (&World{rng: rng}).hell759Accessory()
			if got != tc.want || len(rng.values) != 0 {
				t.Fatalf("lottery result=%+v, want %+v; unused rolls=%v", got, tc.want, rng.values)
			}
		})
	}
}
