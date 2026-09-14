package control

import (
	"strings"
	"testing"
)

func TestKickCommandValidation(t *testing.T) {
	valid := KickCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32), UID: "char-1", Account: "player", ExpectedSessionID: 1, Reason: "valid reason"}
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	for _, change := range []func(*KickCommand){
		func(c *KickCommand) { c.Version = 0 }, func(c *KickCommand) { c.Epoch = strings.Repeat("g", 32) },
		func(c *KickCommand) { c.OperationID = "short" }, func(c *KickCommand) { c.OperationID = "operation/000001" },
		func(c *KickCommand) { c.UID = "" }, func(c *KickCommand) { c.UID = "bad\nuid" },
		func(c *KickCommand) { c.Account = "PLAYER" }, func(c *KickCommand) { c.Account = "" },
		func(c *KickCommand) { c.ExpectedSessionID = 0 }, func(c *KickCommand) { c.ExpectedSessionID = 9007199254740992 },
		func(c *KickCommand) { c.Reason = "  a  " }, func(c *KickCommand) { c.Reason = "bad\nreason" },
		func(c *KickCommand) { c.Reason = strings.Repeat("á", 201) },
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
