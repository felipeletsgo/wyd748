package game

import (
	"fmt"
	"testing"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

func TestMapSnapshotIncludesSortedPlayerAndMobDetails(t *testing.T) {
	p, _ := networkedTestPlayer(1, "MapZ", 2100, 2100)
	q, _ := networkedTestPlayer(2, "MapA", 2101, 2100)
	p.Char.UID, p.Char.Score.Level, p.Char.RuntimeScore = "z-player", 44, nil
	q.Char.UID, q.Char.Score.Level, q.Char.RuntimeScore.Level = "a-player", 22, 99
	q.Session = nil
	w := worldWithNetworkedPlayers(p, q)

	ordinary := &Mob{ID: 20, X: 2100, Y: 2100, HP: 15, Def: &model.NPCDef{Name: "Ordinary NPC", Tipo: "npc"}}
	boss := &Mob{ID: 10, X: 2101, Y: 2100, HP: 50, Def: &model.NPCDef{Name: "Boss", Tipo: model.TipoMonstro, Score: &model.Score{MaxHP: 500}}}
	w.registerMobSpatial(ordinary)
	w.registerMobSpatial(boss)
	w.bosses = map[uint16]*BossRuntime{boss.ID: {MobID: boss.ID}}

	snapshot := w.mapSnapshot(2100, 2100, 16)
	if len(snapshot.Players) != 2 || snapshot.Players[0].UID != "a-player" || snapshot.Players[0].Level != 99 || snapshot.Players[0].SessionID != 0 ||
		snapshot.Players[1].UID != "z-player" || snapshot.Players[1].Level != 44 || snapshot.Players[1].SessionID == 0 {
		t.Fatalf("player rows were incomplete or unsorted: %+v", snapshot.Players)
	}
	if len(snapshot.Mobs) != 2 || snapshot.Mobs[0].ID != boss.ID || !snapshot.Mobs[0].Boss || snapshot.Mobs[0].NPC ||
		snapshot.Mobs[0].HP != 50 || snapshot.Mobs[0].MaxHP != 500 || snapshot.Mobs[1].ID != ordinary.ID || !snapshot.Mobs[1].NPC || snapshot.Mobs[1].Boss {
		t.Fatalf("mob rows were incomplete or unsorted: %+v", snapshot.Mobs)
	}
}

func TestMapSnapshotMarksPlayerTruncation(t *testing.T) {
	players := make([]*Player, control.MaxMapPlayers+1)
	for i := range players {
		p, _ := networkedTestPlayer(uint16(i+1), fmt.Sprintf("MapPlayer%03d", i), 2100, 2100)
		players[i] = p
	}
	w := worldWithNetworkedPlayers(players...)
	snapshot := w.mapSnapshot(2100, 2100, 16)
	if len(snapshot.Players) != control.MaxMapPlayers || len(snapshot.Mobs) != 0 || !snapshot.Truncated {
		t.Fatalf("player bound: players=%d mobs=%d truncated=%v", len(snapshot.Players), len(snapshot.Mobs), snapshot.Truncated)
	}
}

func TestMapSnapshotMarksMobTruncation(t *testing.T) {
	w := testSpatialWorld(nil)
	for i := 1; i <= control.MaxMapMobs+1; i++ {
		w.registerMobSpatial(&Mob{ID: uint16(i), X: 2100, Y: 2100, Def: &model.NPCDef{Name: "NPC", Tipo: "npc"}})
	}

	snapshot := w.mapSnapshot(2100, 2100, 16)
	if len(snapshot.Players) != 0 || len(snapshot.Mobs) != control.MaxMapMobs || !snapshot.Truncated {
		t.Fatalf("mob bound: players=%d mobs=%d truncated=%v", len(snapshot.Players), len(snapshot.Mobs), snapshot.Truncated)
	}
}
