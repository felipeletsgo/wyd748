package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestPrivateWaterRuntimeLookupWithoutLivePlayer(t *testing.T) {
	var nilWorld *World
	if got := nilWorld.privateWaterRuntimeIDForPlayer(7); got != "" {
		t.Fatalf("nil world resolved runtime %q", got)
	}
	w := newZoneTestWorld()
	if got := w.privateWaterRuntimeIDForPlayer(0); got != "" {
		t.Fatalf("zero player ID resolved runtime %q", got)
	}

	const playerID = 7
	const runtimeID = "water-normal-1:7"
	w.itemInstances = map[string]*ItemInstance{
		runtimeID: {
			Config:    model.VolatileInstance{ID: "water-normal-1"},
			RuntimeID: runtimeID,
			MemberIDs: []uint16{playerID},
		},
	}
	w.playerInstance = map[uint16]string{playerID: runtimeID}
	if got := w.privateWaterRuntimeIDForPlayer(playerID); got != runtimeID {
		t.Fatalf("runtime index without live player=%q, want %q", got, runtimeID)
	}
	if got := w.privateWaterRuntimeIDForPlayer(9); got != "" {
		t.Fatalf("untracked player resolved private Water runtime %q", got)
	}

	public := addZonePlayer(w, 8, 2100, 2100, 100)
	if got := w.privateWaterRuntimeIDForPlayer(public.ID); got != "" {
		t.Fatalf("public player resolved a private Water runtime: %q", got)
	}
}
