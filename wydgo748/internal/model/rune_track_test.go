package model

import "testing"

func TestRuneTrackGeneratorRoomRangesAndGaps(t *testing.T) {
	for _, test := range []struct {
		label int
		room  int
	}{
		{5661, -1}, {5662, 0}, {5714, 0}, {5715, 1}, {5774, 1},
		{5775, 6}, {5797, 6}, {5798, 2}, {5857, 2}, {5858, 4},
		{5907, 4}, {5908, 5}, {5956, 5}, {5957, 3}, {5984, 3},
		{5985, 6}, {5988, 6}, {5989, -1}, {5797 + 1, 2},
	} {
		if got := RuneTrackGeneratorRoom(test.label); got != test.room {
			t.Errorf("RuneTrackGeneratorRoom(%d) = %d, want %d", test.label, got, test.room)
		}
	}
}
