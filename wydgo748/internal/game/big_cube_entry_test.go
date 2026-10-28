package game

import (
	"encoding/json"
	"os"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func shippedBigCubeConfig(t *testing.T) model.VolatileInstance {
	t.Helper()
	contents, err := os.ReadFile("../../data/content/instances.json")
	if err != nil {
		t.Fatal(err)
	}
	var config struct {
		Instances map[string]model.VolatileInstance `json:"instances"`
	}
	if err := json.Unmarshal(contents, &config); err != nil {
		t.Fatal(err)
	}
	cfg, ok := config.Instances["big-cube-native"]
	if !ok || !isBigCubeConfig(&cfg) {
		t.Fatal("shipped Big Cube configuration is missing")
	}
	return cfg
}

func TestBigCubeShippedEntryRectangleAndTicketAdmission(t *testing.T) {
	cfg := shippedBigCubeConfig(t)
	if len(cfg.EntryAreas) != 1 || len(cfg.Stages) != 1 || cfg.Stages[0].Quiz == nil ||
		cfg.Stages[0].Quiz.DurationSeconds != 10 {
		t.Fatal("Big Cube must have one entry rectangle and a ten-second question")
	}
	// Check every cell, all inclusive edges, and the immediately excluded border.
	for x := uint16(1307); x <= 1376; x++ {
		for y := uint16(1443); y <= 1744; y++ {
			want := x >= 1308 && x <= 1375 && y >= 1444 && y <= 1743
			if got := instanceTargetAllowedAt(&cfg, x, y); got != want {
				t.Fatalf("entry (%d,%d): allowed=%v, want %v", x, y, got, want)
			}
		}
	}
	for _, tc := range []struct {
		x, y    uint16
		allowed bool
	}{
		{1308, 1444, true}, {1375, 1743, true}, {1308, 1743, true}, {1375, 1444, true},
		{1342, 1466, true}, {1307, 1500, false}, {1376, 1500, false},
		{1342, 1443, false}, {1342, 1744, false}, {2100, 2100, false},
	} {
		w, p, _, store, _ := instanceTestWorld()
		p.Party = nil
		p.X, p.Y, p.Char.X, p.Char.Y = tc.x, tc.y, tc.x, tc.y
		p.Char.Inv[0] = model.Item{Index: 1737}
		rule := model.VolatileRule{Action: "instance_ticket", Consume: true, PartyMode: "solo", Instance: &cfg}
		w.useInstanceTicket(p.Session, p, &p.Char.Inv[0], 0, rule, useItemRequest{})
		if tc.allowed {
			if p.Char.Inv[0].Index != 0 || len(w.itemInstances) != 1 || store.gameSaves != 1 {
				t.Fatalf("valid entry (%d,%d) did not consume and admit once", tc.x, tc.y)
			}
		} else if p.Char.Inv[0].Index != 1737 || len(w.itemInstances) != 0 || store.gameSaves != 0 ||
			p.X != tc.x || p.Y != tc.y {
			t.Fatalf("invalid entry (%d,%d) changed ticket, position or instance", tc.x, tc.y)
		}
	}
}

func TestBigCubeLobbyPortalReturnsToNoatumWithoutUsingTicket(t *testing.T) {
	routes, err := data.LoadTeleports("../../data/world/teleports.ini")
	if err != nil {
		t.Fatal(err)
	}
	portal, ok := teleportAt(routes, 1342, 1466)
	if !ok || portal.DestX != 1058 || portal.DestY != 1725 || portal.Price != 700 {
		t.Fatalf("Big Cube lobby portal does not reverse the existing Noatum route: %+v", portal)
	}
	w, p, _, store, _ := instanceTestWorld()
	w.teleports = routes
	p.X, p.Y, p.Char.X, p.Char.Y = 1342, 1466, 1342, 1466
	p.Char.Gold = 1000
	p.Char.Inv[0] = model.Item{Index: 1737}
	w.onReqTeleport(p.Session, make([]byte, 16))
	if p.X != 1058 || p.Y != 1725 || p.Char.Gold != 300 || p.Char.Inv[0].Index != 1737 ||
		len(w.itemInstances) != 0 || store.saves != 1 {
		t.Fatalf("return portal changed instance or ticket, or failed to return: (%d,%d) gold=%d",
			p.X, p.Y, p.Char.Gold)
	}
}

func TestBigCubeQuestionIgnoresStalePreCommitTimestamp(t *testing.T) {
	w, p, _, _, clock := instanceTestWorld()
	inst := &ItemInstance{Config: bigCubeTestConfig(), MemberIDs: []uint16{p.ID}}
	beforeCommit := clock.Now()
	clock.Advance(20 * time.Second)
	if !w.startBigCubeQuestion(inst, beforeCommit) || !inst.QuizDeadline.Equal(clock.Now().Add(10*time.Second)) {
		t.Fatal("synchronous persistence latency shortened the answer window")
	}
}
