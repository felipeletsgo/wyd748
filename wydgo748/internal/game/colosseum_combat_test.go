package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestColosseumMonsterDamageAcrossTiersAndWaves(t *testing.T) {
	for tier := 0; tier < model.ColosseumTiers; tier++ {
		t.Run([]string{"N", "M", "A"}[tier], func(t *testing.T) {
			w, clock := colosseumWorld(t, time.Date(2026, 10, 23, 18, 6, 0, 0, time.UTC))
			for wave := range colosseumWaves[tier] {
				w.spawnColosseumWave(tier, wave)
			}
			if colosseumMobs(w) == 0 {
				t.Fatal("event waves spawned no monsters")
			}
			player := addZonePlayer(w, 1, 2630, 1726, 100)
			for _, spawned := range w.mobs {
				// The event fixture supplies minimal templates. Make a hostile copy
				// without changing templates shared with other event tests.
				mob := *spawned
				def := *mob.Def
				def.Tipo = model.TipoMonstro
				mob.Def = &def
				// Melee and spell calculations converge on this authoritative boundary.
				for _, damage := range []uint32{0, 1, 500, ^uint32(0)} {
					setPlayerCurHP(player.Char, 100)
					w.applyMobDamageToPlayer(&mob, player, damage, clock.Now(), func(uint32) []byte { return nil })
					if got, want := 100-playerCurHP(player.Char), min(damage, 1); got != want {
						t.Fatalf("tier=%d generator=%d monster=%q incoming=%d applied=%d, want %d",
							tier, mob.GenerIndex, mob.Def.Name, damage, got, want)
					}
					for {
						if _, ok := player.Session.DequeuePacketForTest(); !ok {
							break
						}
					}
				}

				// Reusing a template, position and generator index outside the event
				// must not grant its damage cap, even with a copied generator.
				otherGenerator := *mob.generator
				mob.generator = &otherGenerator
				setPlayerCurHP(player.Char, 100)
				w.applyMobDamageToPlayer(&mob, player, 50, clock.Now(), func(uint32) []byte { return nil })
				if got := playerCurHP(player.Char); got != 50 {
					t.Fatalf("non-event monster HP result=%d, want 50", got)
				}
			}
		})
	}
}
