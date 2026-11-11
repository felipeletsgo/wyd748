package game

import "testing"

func TestSetPlayerInstanceIndexHandlesNilIDsAndMembershipRemoval(t *testing.T) {
	var nilWorld *World
	nilWorld.setPlayerInstanceIndex(1, "room")

	w := &World{}
	w.setPlayerInstanceIndex(0, "room")
	if w.playerInstance != nil {
		t.Fatalf("zero player ID allocated membership state: %v", w.playerInstance)
	}
	w.setPlayerInstanceIndex(1, "room-a")
	if w.playerInstance[1] != "room-a" {
		t.Fatalf("membership index=%v", w.playerInstance)
	}
	w.setPlayerInstanceIndex(2, "room-b")
	w.setPlayerInstanceIndex(1, " \t ")
	if _, exists := w.playerInstance[1]; exists || w.playerInstance[2] != "room-b" {
		t.Fatalf("blank runtime did not remove only its player: %v", w.playerInstance)
	}
}
