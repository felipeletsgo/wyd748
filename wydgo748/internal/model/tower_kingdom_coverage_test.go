package model

import "testing"

func TestTowerKingdomUsesLegacyOwnerOrZero(t *testing.T) {
	tests := []struct {
		name string
		wars TowerWarState
		want byte
	}{
		{name: "legacy guild owner", wars: TowerWarState{Owner: 7}, want: 8},
		{name: "missing owner", wars: TowerWarState{Owner: 99}, want: 0},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			registry := GuildRegistry{
				Guilds: []Guild{{ID: 7, Kingdom: 8}},
				Wars:   GuildWarState{Tower: test.wars},
			}
			if got := registry.TowerKingdom(); got != test.want {
				t.Fatalf("TowerKingdom()=%d, want %d", got, test.want)
			}
		})
	}
}
