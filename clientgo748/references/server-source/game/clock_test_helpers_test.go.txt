package game

import "time"

func WithClock(clock Clock) WorldOption {
	return func(w *World) {
		if clock != nil {
			w.clock = clock
		}
	}
}

func WithRNG(rng RNG) WorldOption {
	return func(w *World) {
		if rng != nil {
			w.rng = rng
		}
	}
}

type fakeClock struct {
	current time.Time
}

func newFakeClock(start time.Time) *fakeClock { return &fakeClock{current: start} }

func (c *fakeClock) Now() time.Time { return c.current }

func (c *fakeClock) Advance(d time.Duration) { c.current = c.current.Add(d) }

type fixedRNG struct {
	value int
}

func (r fixedRNG) Intn(n int) int {
	if n <= 0 {
		return 0
	}
	if r.value >= n {
		return n - 1
	}
	if r.value < 0 {
		return 0
	}
	return r.value
}
