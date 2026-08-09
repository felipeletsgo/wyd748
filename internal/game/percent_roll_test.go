package game

import "testing"

func TestPercentRollUsesOneToHundredInclusive(t *testing.T) {
	tests := []struct {
		name     string
		raw      int
		chance   int
		wantRoll int
		success  bool
		message  string
	}{
		{name: "95 succeeds at boundary", raw: 94, chance: 95, wantRoll: 95, success: true, message: "Success: 95/95"},
		{name: "95 fails above boundary", raw: 95, chance: 95, wantRoll: 96, success: false, message: "Fail: 96/95"},
		{name: "5 succeeds at boundary", raw: 4, chance: 5, wantRoll: 5, success: true, message: "Success: 5/5"},
		{name: "5 fails above boundary", raw: 5, chance: 5, wantRoll: 6, success: false, message: "Fail: 6/5"},
		{name: "100 includes 100", raw: 99, chance: 100, wantRoll: 100, success: true, message: "Success: 100/100"},
		{name: "zero never succeeds", raw: 0, chance: 0, wantRoll: 1, success: false, message: "Fail: 1/0"},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w := &World{rng: fixedRNG{value: tc.raw}}
			got := w.rollPercent(tc.chance)
			if got.Roll != tc.wantRoll || got.Chance != tc.chance || got.Success != tc.success {
				t.Fatalf("roll=%+v, want roll=%d chance=%d success=%t", got, tc.wantRoll, tc.chance, tc.success)
			}
			if msg := got.message(); msg != tc.message {
				t.Fatalf("message=%q, want %q", msg, tc.message)
			}
		})
	}
}
