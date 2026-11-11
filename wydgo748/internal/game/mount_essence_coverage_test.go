package game

import (
	"testing"

	"wydgo/internal/model"
)

type mountEssenceSequenceRNG struct {
	values []int
	calls  int
}

func (r *mountEssenceSequenceRNG) Intn(n int) int {
	if n <= 0 {
		return 0
	}
	value := 0
	if r.calls < len(r.values) {
		value = r.values[r.calls]
	}
	r.calls++
	if value < 0 {
		return 0
	}
	if value >= n {
		return n - 1
	}
	return value
}

func TestMountEssenceAdultGrowthOutcomes(t *testing.T) {
	tests := []struct {
		name        string
		level       int
		rng         []int
		wantLevel   int
		wantRolls   int
		wantOutcome string
		wantOK      bool
	}{
		{name: "growth succeeds", level: 20, rng: []int{0}, wantLevel: 21, wantRolls: 1, wantOutcome: "Your mount gained a level.", wantOK: true},
		{name: "odd level growth fails", level: 21, rng: []int{99}, wantLevel: 21, wantRolls: 1, wantOutcome: "The mount evolution failed.", wantOK: true},
		{name: "even level loses level", level: 20, rng: []int{99, 0}, wantLevel: 19, wantRolls: 2, wantOutcome: "Failure: the mount lost a level.", wantOK: true},
		{name: "even level avoids penalty", level: 20, rng: []int{99, 99}, wantLevel: 20, wantRolls: 2, wantOutcome: "The mount evolution failed.", wantOK: true},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			rng := &mountEssenceSequenceRNG{values: tc.rng}
			w := &World{rng: rng}
			mount := model.Item{Index: 2360}
			mount.SetMountLevel(tc.level)
			outcome := w.mountEssence(&mount, 2390)
			if outcome.OK != tc.wantOK || outcome.Message != tc.wantOutcome || mount.MountLevel() != tc.wantLevel {
				t.Fatalf("outcome=%+v level=%d, want ok=%v message=%q level=%d", outcome, mount.MountLevel(), tc.wantOK, tc.wantOutcome, tc.wantLevel)
			}
			if len(outcome.Rolls) != tc.wantRolls || rng.calls != tc.wantRolls {
				t.Fatalf("rolls=%+v rng calls=%d, want %d", outcome.Rolls, rng.calls, tc.wantRolls)
			}
			if mount.MountHP() != model.MountEssenceHP {
				t.Fatalf("essence HP=%d, want %d", mount.MountHP(), model.MountEssenceHP)
			}
		})
	}
}

func TestMountEssenceRejectsMaximumAdultAfterRestoringHP(t *testing.T) {
	w := &World{rng: fixedRNG{value: 0}}
	mount := model.Item{Index: 2360}
	mount.SetMountLevel(model.MountMaxLevel)
	outcome := w.mountEssence(&mount, 2390)
	if outcome.OK || outcome.Message != "The mount is already at maximum level." || mount.MountLevel() != model.MountMaxLevel {
		t.Fatalf("maximum-level result=%+v level=%d", outcome, mount.MountLevel())
	}
	if mount.MountHP() != model.MountEssenceHP || len(outcome.Rolls) != 0 {
		t.Fatalf("maximum-level essence state hp=%d rolls=%+v", mount.MountHP(), outcome.Rolls)
	}
}
