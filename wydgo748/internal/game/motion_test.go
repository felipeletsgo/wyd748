package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/wire"
)

func motionIntent(motion, parm uint16) []byte {
	pkt := inboundPacket(wire.OpMotion, 20)
	binary.LittleEndian.PutUint16(pkt[6:8], 999) // The claimed ID is not authoritative.
	binary.LittleEndian.PutUint16(pkt[12:14], motion)
	binary.LittleEndian.PutUint16(pkt[14:16], parm)
	binary.LittleEndian.PutUint32(pkt[16:20], 0x7FC00000) // Discard the client-supplied NaN.
	return pkt
}

func TestMotionRoundtripPublishesAuthoritativePlayerEmote(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Emoter", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	outsider, _ := networkedTestPlayer(3, "Outsider", 2200, 2200)
	observer.show(owner.ID)
	w := worldWithNetworkedPlayers(owner, observer, outsider)

	w.onMotion(owner.Session, motionIntent(21, 0))

	if got := owner.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("sender received %d echoes, want 1", got)
	}
	if got := observer.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("observer received %d motions, want 1", got)
	}
	if got := outsider.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("out-of-view player received %d motions", got)
	}
}

func TestMotionRejectsClientOwnedEffects(t *testing.T) {
	for _, tc := range []struct {
		name   string
		motion uint16
		parm   uint16
	}{
		{name: "system motion", motion: 100},
		{name: "server-side motion", motion: 14},
		{name: "character effect", motion: 21, parm: 1},
		{name: "clear death", motion: 21, parm: 2},
		{name: "level up", motion: 21, parm: 3},
	} {
		t.Run(tc.name, func(t *testing.T) {
			owner, _ := networkedTestPlayer(1, "Emoter", 2100, 2100)
			observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
			observer.show(owner.ID)
			w := worldWithNetworkedPlayers(owner, observer)

			w.onMotion(owner.Session, motionIntent(tc.motion, tc.parm))

			if owner.Session.QueuedPacketsForTest() != 0 ||
				observer.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("client-controlled effect was published")
			}
		})
	}
}

func TestMotionRejectsDeadPlayerAndAllowsEmoteAfterRevival(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Emoter", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	observer.show(owner.ID)
	w := worldWithNetworkedPlayers(owner, observer)

	setPlayerCurHP(owner.Char, 0)
	w.onMotion(owner.Session, motionIntent(21, 0))
	if owner.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("dead-player emote was published")
	}

	setPlayerCurHP(owner.Char, 100)
	w.onMotion(owner.Session, motionIntent(21, 0))
	if owner.Session.QueuedPacketsForTest() != 1 || observer.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("revived-player emote was not published to owner and observer")
	}
}
