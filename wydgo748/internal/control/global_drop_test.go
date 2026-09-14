package control

import (
	"strings"
	"testing"
)

func TestGlobalDropCommandValidation(t *testing.T) {
	valid := GlobalDropCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), Action: "start", Reason: "test event", ItemID: 500, RatePercent: 1, MaxDrops: 10000}
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	for _, change := range []func(*GlobalDropCommand){
		func(c *GlobalDropCommand) { c.RatePercent = 0 }, func(c *GlobalDropCommand) { c.RatePercent = 101 },
		func(c *GlobalDropCommand) { c.MaxDrops = 0 }, func(c *GlobalDropCommand) { c.MaxDrops = -1 },
		func(c *GlobalDropCommand) { c.MaxDrops = 10000001 }, func(c *GlobalDropCommand) { c.DurationSeconds = -1 },
		func(c *GlobalDropCommand) { c.DurationSeconds = 604801 }, func(c *GlobalDropCommand) { c.ItemID = 0 },
		func(c *GlobalDropCommand) { c.Reason = "  " }, func(c *GlobalDropCommand) { c.Reason = "reason\nforged log" },
		func(c *GlobalDropCommand) { c.OperationID = "short" }, func(c *GlobalDropCommand) { c.Action = "arbitrary-lua" },
		func(c *GlobalDropCommand) { c.Epoch = "" }, func(c *GlobalDropCommand) { c.Version = 2 },
	} {
		c := valid
		change(&c)
		if c.Validate() == nil {
			t.Fatalf("invalid accepted: %+v", c)
		}
	}
	valid.MaxDrops = 0
	valid.DurationSeconds = 3600
	valid.RatePercent = 100
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	valid.Action = "stop"
	valid.ExpectedEventID = "event-1"
	valid.ItemID = 0
	valid.RatePercent = 0
	valid.DurationSeconds = 0
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	valid.ItemID = 500
	if valid.Validate() == nil {
		t.Fatal("stop accepted hidden configuration")
	}
}
