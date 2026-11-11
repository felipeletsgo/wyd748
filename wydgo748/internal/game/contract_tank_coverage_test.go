package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestGuardMonsterSpaceAllowedHonorsRuntimeAndOwnerState(t *testing.T) {
	for _, test := range []struct {
		name       string
		mobSpace   string
		guardSpace string
		guardKind  guardKind
		fear       bool
		want       bool
	}{
		{name: "public guard can protect owner", want: true},
		{name: "different runtime is rejected", mobSpace: "room-a", guardSpace: "room-b"},
		{name: "unrestored runtime is rejected", mobSpace: "room-a", guardSpace: "room-a"},
		{name: "fear prevents ordinary guard aggro", fear: true},
		{name: "Sophia can protect an owner under fear", guardKind: guardSophia, fear: true, want: true},
	} {
		t.Run(test.name, func(t *testing.T) {
			w := newZoneTestWorld()
			owner, _ := networkedTestPlayer(1, "GuardOwner", 2100, 2100)
			if test.fear {
				owner.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: time.Now().Add(time.Minute)}
			}
			mob := &Mob{Def: bossTestNPC("Hostile", 100), InstanceID: test.mobSpace}
			guard := &Mob{InstanceID: test.guardSpace, guardKind: test.guardKind}
			if got := w.guardMonsterSpaceAllowed(mob, owner, guard); got != test.want {
				t.Fatalf("guard allowed=%t, want %t", got, test.want)
			}
		})
	}
}
