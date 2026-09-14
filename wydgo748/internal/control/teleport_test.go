package control

import (
	"strings"
	"testing"
)

func TestTeleportCommandValidation(t *testing.T) {
	valid := TeleportCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), UID: "char-1", Account: "player", ExpectedSessionID: 1, Destination: "armia", Reason: "valid reason"}
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	for _, change := range []func(*TeleportCommand){
		func(c *TeleportCommand) { c.Destination = "arena" }, func(c *TeleportCommand) { c.Destination = "" }, func(c *TeleportCommand) { c.Version = 0 }, func(c *TeleportCommand) { c.Epoch = strings.Repeat("g", 32) },
		func(c *TeleportCommand) { c.OperationID = "short" }, func(c *TeleportCommand) { c.OperationID = "operation/000001" },
		func(c *TeleportCommand) { c.UID = "" }, func(c *TeleportCommand) { c.UID = "bad\nuid" },
		func(c *TeleportCommand) { c.Account = "PLAYER" }, func(c *TeleportCommand) { c.Account = "" },
		func(c *TeleportCommand) { c.ExpectedSessionID = 0 }, func(c *TeleportCommand) { c.ExpectedSessionID = 9007199254740992 },
		func(c *TeleportCommand) { c.Reason = "  a  " }, func(c *TeleportCommand) { c.Reason = "bad\nreason" },
		func(c *TeleportCommand) { c.Reason = strings.Repeat("á", 201) },
	} {
		c := valid
		change(&c)
		if c.Validate() == nil {
			t.Fatalf("accepted %+v", c)
		}
	}
	valid.Reason = strings.Repeat("á", 200)
	if err := valid.Validate(); err != nil {
		t.Fatal("unicode reason rejected", err)
	}
}
