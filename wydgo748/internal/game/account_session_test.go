package game

import (
	"testing"

	"wydgo/internal/model"
	netpkg "wydgo/internal/net"
)

func TestAccountSessionIsExclusiveCaseInsensitive(t *testing.T) {
	w := &World{}
	first := &netpkg.Session{ID: 1}
	second := &netpkg.Session{ID: 2}
	if !w.claimAccountSession(first, "Felipe") {
		t.Fatal("the first session should reserve the account")
	}
	if w.claimAccountSession(second, "felipe") {
		t.Fatal("a second session reserved the same account with different capitalization")
	}
	if !w.claimAccountSession(first, "FELIPE") {
		t.Fatal("the owner lost its reservation")
	}
}

func TestAccountSessionReleaseOnlyByOwner(t *testing.T) {
	w := &World{}
	owner := &netpkg.Session{ID: 1}
	other := &netpkg.Session{ID: 2}
	acc := &model.Account{Name: "felipe"}
	w.claimAccountSession(owner, acc.Name)
	w.releaseAccountSession(other, acc)
	if w.claimAccountSession(other, acc.Name) {
		t.Fatal("another session released the owner's account")
	}
	w.releaseAccountSession(owner, acc)
	if !w.claimAccountSession(other, acc.Name) {
		t.Fatal("the account remained reserved after owner cleanup")
	}
}

func TestAccountSessionRetainsClosedSocketUntilDisconnectCommand(t *testing.T) {
	w := &World{}
	owner := netpkg.NewTestSession(1, 1)
	other := netpkg.NewTestSession(2, 1)
	if !w.claimAccountSession(owner, "Felipe") {
		t.Fatal("the initial session did not reserve the account")
	}
	owner.Close() // The transport can close before World processes its disconnect.
	if w.claimAccountSession(other, "felipe") {
		t.Fatal("relogin reclaimed an account before its owner finished cleanup")
	}
}
