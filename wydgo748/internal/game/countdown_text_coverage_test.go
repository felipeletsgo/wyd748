package game

import (
	"testing"
	"time"
)

func TestCountdownTextPluralityAndRoundingBoundaries(t *testing.T) {
	w := &World{}
	for _, test := range []struct {
		name string
		left time.Duration
		want string
	}{
		{name: "one minute", left: time.Minute, want: w.msg("_WG_Countdown_One_Minute")},
		{name: "rounds to one minute", left: 89 * time.Second, want: w.msg("_WG_Countdown_One_Minute")},
		{name: "rounds to two minutes", left: 90 * time.Second, want: w.msg("_WG_Countdown_Minutes", 2)},
		{name: "one second", left: time.Second, want: w.msg("_WG_Countdown_One_Second")},
		{name: "rounds to one second", left: 1499 * time.Millisecond, want: w.msg("_WG_Countdown_One_Second")},
		{name: "rounds to two seconds", left: 1500 * time.Millisecond, want: w.msg("_WG_Countdown_Seconds", 2)},
		{name: "zero seconds", want: w.msg("_WG_Countdown_Seconds", 0)},
	} {
		t.Run(test.name, func(t *testing.T) {
			if got := w.countdownText(test.left); got != test.want {
				t.Fatalf("countdownText(%s)=%q, want %q", test.left, got, test.want)
			}
		})
	}
}
