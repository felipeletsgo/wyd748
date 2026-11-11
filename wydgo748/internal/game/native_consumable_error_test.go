package game

import "testing"

func TestNativeConsumableRejectionErrorIncludesReason(t *testing.T) {
	got := (&nativeConsumableRejection{message: "invalid item state"}).Error()
	if got != "native consumable rejected: invalid item state" {
		t.Fatalf("rejection error=%q", got)
	}
}
