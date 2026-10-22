package game

import (
	"context"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

func runOXEvent(w *World, action string, allowed bool) control.OXEventResult {
	cmd := control.OXEventCommand{Version: 1, OperationID: "operation-" + strings.ReplaceAll(action, "_", "-") + "-000001", Action: action, Reason: "ox event"}
	req := &oxEventRequest{ctx: context.Background(), actor: "operator", command: &cmd,
		authorize: func() bool { return allowed }, reply: make(chan control.OXEventResult, 1)}
	w.handleOXEvent(req)
	return <-req.reply
}

// 7.59 TMSRV quizdoor drives the six OX gates; staff needs the capability.
func TestOXEventDoorsAndMode(t *testing.T) {
	w, _ := colosseumWorld(t, time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	if r := runOXEvent(w, "doors_open", false); r.Code != "capability_required" || !r.Status.DoorsLocked {
		t.Fatalf("unauthorized open = %+v", r)
	}
	if r := runOXEvent(w, "doors_open", true); r.Code != "ok" || r.Status.Doors != 6 || r.Status.DoorsLocked {
		t.Fatalf("open = %+v", r)
	}
	if r := runOXEvent(w, "doors_lock", true); r.Code != "ok" || !r.Status.DoorsLocked {
		t.Fatalf("lock = %+v", r)
	}
	if r := runOXEvent(w, "doors_lock", true); r.Code != "unchanged" {
		t.Fatalf("repeated lock = %+v", r)
	}
	if r := runOXEvent(w, "mode_on", true); r.Code != "ok" || !r.Status.QuizMode || !w.oxQuizMode {
		t.Fatalf("mode on = %+v", r)
	}
	if r := runOXEvent(w, "mode_on", true); r.Code != "unchanged" {
		t.Fatalf("repeated mode on = %+v", r)
	}
	if r := runOXEvent(w, "mode_off", true); r.Code != "ok" || r.Status.QuizMode {
		t.Fatalf("mode off = %+v", r)
	}
}

func TestOXEventStartOpensArenaOnce(t *testing.T) {
	w, _ := colosseumWorld(t, time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	if r := runOXEvent(w, "start", false); r.Code != "capability_required" || r.Status.QuizMode {
		t.Fatalf("unauthorized start = %+v", r)
	}
	if r := runOXEvent(w, "start", true); r.Code != "ok" || !r.Status.QuizMode || r.Status.DoorsLocked {
		t.Fatalf("start = %+v", r)
	}
	runOXEvent(w, "doors_lock", true)
	if r := runOXEvent(w, "start", true); r.Code != "unchanged" || !r.Status.DoorsLocked {
		t.Fatalf("repeated start unlocked an active question = %+v", r)
	}
	runOXEvent(w, "mode_off", true)
	if r := runOXEvent(w, "start", true); r.Code != "ok" || r.Status.DoorsLocked {
		t.Fatalf("next event = %+v", r)
	}
	w = newZoneTestWorld()
	if r := runOXEvent(w, "start", true); r.Code != "doors_unavailable" || r.Status.QuizMode {
		t.Fatalf("started without six gates = %+v", r)
	}
}

// With quiz mode the Entrance_Ticket enters the OX arena and is spent;
// without it the ticket stays the Big Cube ticket.
func TestOXTicketFollowsQuizMode(t *testing.T) {
	w, _ := colosseumWorld(t, time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	p := addZonePlayer(w, 1, 2100, 2100, 500)
	p.Char.Inv[2] = model.Item{Index: 1737} // Entrance_Ticket, rule 51 with quizArena
	if _, ok := w.oxTicketRule(p.Char.Inv[2]); ok {
		t.Fatal("the ticket entered the OX arena without quiz mode")
	}
	w.oxQuizMode = true
	rule, ok := w.oxTicketRule(p.Char.Inv[2])
	if !ok {
		t.Fatal("quiz mode did not route the ticket to the OX arena")
	}
	if _, other := w.oxTicketRule(model.Item{Index: colosseumElixirItem}); other {
		t.Fatal("another item was routed to the OX arena")
	}
	w.useOXTicket(p.Session, p, &p.Char.Inv[2], 2, rule)
	if p.Char.Inv[2].Index != 0 || chebyshev(p.X, p.Y, 1340, 1492) > 3 {
		t.Fatalf("ticket=%d position=(%d,%d)", p.Char.Inv[2].Index, p.X, p.Y)
	}
}

// quizresultx moves the X side to (1341,1465) and revives the dead there;
// the O side stays.
func TestOXResultMovesTheLosingSide(t *testing.T) {
	w, _ := colosseumWorld(t, time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	loser := addZonePlayer(w, 1, 1335, 1500, 0)
	winner := addZonePlayer(w, 2, 1345, 1500, 500)
	winner.Visible[loser.ID] = struct{}{}
	r := runOXEvent(w, "result_x", true)
	if r.Code != "ok" || r.Moved != 1 {
		t.Fatalf("result_x = %+v", r)
	}
	if chebyshev(loser.X, loser.Y, oxExitX, oxExitY) > 3 || playerCurHP(loser.Char) == 0 {
		t.Fatalf("loser at (%d,%d) hp=%d", loser.X, loser.Y, playerCurHP(loser.Char))
	}
	if !sentRemoveMob(winner, loser.ID, 3) {
		t.Fatal("observers kept the loser's corpse")
	}
	if winner.X != 1345 || winner.Y != 1500 {
		t.Fatal("the winning side was moved")
	}
}

func TestOXResultMatchesStatusAtTheGateBoundary(t *testing.T) {
	for _, action := range []string{"result_x", "result_o"} {
		t.Run(action, func(t *testing.T) {
			w, _ := colosseumWorld(t, time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
			players := make([]*Player, 4)
			for i := range players {
				players[i] = addZonePlayer(w, uint16(i+1), uint16(1340+i), 1500, 500)
			}
			before := w.oxEventSnapshot()
			if before.PlayersX != 1 || before.PlayersO != 2 {
				t.Fatalf("boundary counts = %+v", before)
			}
			result := runOXEvent(w, action, true)
			wantMoved := before.PlayersX
			if action == "result_o" {
				wantMoved = before.PlayersO
			}
			if result.Moved != wantMoved {
				t.Fatalf("moved %d, counted %d", result.Moved, wantMoved)
			}
			for i, p := range players {
				wantMove := action == "result_x" && i == 0 || action == "result_o" && i >= 2
				moved := p.Y != 1500
				if moved != wantMove {
					t.Fatalf("column %d moved=%v, want %v", 1340+i, moved, wantMove)
				}
			}
		})
	}
}
