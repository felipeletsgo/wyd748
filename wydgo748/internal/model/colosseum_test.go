package model

import "testing"

func TestDefaultColosseumSchedule(t *testing.T) {
	c := DefaultColosseumConfig()
	if !c.Enabled || c.EventHour != 17 || c.Validate() != nil {
		t.Fatalf("unexpected default schedule: %+v", c)
	}
	c.EventHour = 21
	if c.Validate() == nil {
		t.Fatal("schedule extending beyond the same day accepted")
	}
}
