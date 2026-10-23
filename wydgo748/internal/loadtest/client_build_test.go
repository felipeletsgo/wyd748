package loadtest

import (
	"testing"

	"wydgo/internal/wire"
)

func TestLoadClientAdvertisesCurrentBuild(t *testing.T) {
	if !wire.CompatibleClientLogin(loginPacket("bot0001", "123456")) {
		t.Fatal("load client would be rejected by build admission")
	}
}
