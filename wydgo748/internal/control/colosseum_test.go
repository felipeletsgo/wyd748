package control

import (
	"strings"
	"testing"
)

func TestColosseumCommandValidation(t *testing.T) {
	valid := ColosseumCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), ExpectedRevision: 1, Tier: "N", Reason: "Start today's session early"}
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	for name, change := range map[string]func(*ColosseumCommand){
		"version":           func(c *ColosseumCommand) { c.Version = 2 },
		"operation":         func(c *ColosseumCommand) { c.OperationID = "invalid!operation" },
		"epoch":             func(c *ColosseumCommand) { c.Epoch = "" },
		"revision":          func(c *ColosseumCommand) { c.ExpectedRevision = 0 },
		"tier":              func(c *ColosseumCommand) { c.Tier = "NM" },
		"reason":            func(c *ColosseumCommand) { c.Reason = "  " },
		"control character": func(c *ColosseumCommand) { c.Reason = "Start\nearly" },
	} {
		t.Run(name, func(t *testing.T) {
			cmd := valid
			change(&cmd)
			if cmd.Validate() == nil {
				t.Fatal("invalid command accepted")
			}
		})
	}
}
