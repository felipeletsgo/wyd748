package game

import (
	"context"
	"encoding/binary"
	"fmt"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func runOXEvent(w *World, action string, allowed bool) control.OXEventResult {
	cmd := control.OXEventCommand{Version: 1, OperationID: "operation-" + strings.ReplaceAll(action, "_", "-") + "-000001", Action: action, Reason: "ox event"}
	req := &oxEventRequest{ctx: context.Background(), actor: "operator", command: &cmd,
		authorize: func() bool { return allowed }, reply: make(chan control.OXEventResult, 1)}
	w.handleOXEvent(req)
	return <-req.reply
}

// Staff must hold the capability for both automatic and manual gate controls.
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

func oxTestPlayer(w *World, id uint16, x, y uint16) *Player {
	p := addZonePlayer(w, id, x, y, 500)
	p.Account.Name = fmt.Sprintf("ox-%d", id)
	p.Char.UID = p.Account.Name
	p.Char.X, p.Char.Y = x, y
	w.indexPlayerCharacter(p)
	w.claimAccountSession(p.Session, p.Account.Name)
	return p
}

func enterOXTest(t *testing.T, w *World, p *Player) {
	t.Helper()
	p.Char.Inv[2] = model.Item{Index: 1737, UID: fmt.Sprintf("ticket-%d", p.ID)}
	w.onUseItem(p.Session, useItemPacket(2, 0))
	if !w.isOXMember(p) || p.Char.Inv[2].Index != 0 {
		t.Fatal("ticket did not admit the player")
	}
	drainPlayerPackets(p)
}

func placeOXTest(w *World, p *Player, x, y uint16) {
	p.X, p.Y, p.Char.X, p.Char.Y = x, y, x, y
	w.updatePlayerSpatial(p)
}

func TestOXTicketInactiveClosedAndWrongAreaAreNotConsumed(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	p := oxTestPlayer(w, 1, 1340, 1460)
	p.Char.Inv[2] = model.Item{Index: 1737}
	w.onUseItem(p.Session, useItemPacket(2, 0))
	if p.Char.Inv[2].Index != 1737 || p.Y != 1460 || len(w.itemInstances) != 0 {
		t.Fatal("inactive ticket was consumed or opened a private instance")
	}
	if !oxNoticeContains(p, "inactive") {
		t.Fatal("missing inactive explanation")
	}
	runOXEvent(w, "start", true)
	placeOXTest(w, p, 2100, 2100)
	w.onUseItem(p.Session, useItemPacket(2, 0))
	if p.Char.Inv[2].Index != 1737 || !oxNoticeContains(p, "entry area") {
		t.Fatal("wrong-area admission was not rejected")
	}
	placeOXTest(w, p, 1340, 1460)
	clock.Advance(time.Minute)
	w.onUseItem(p.Session, useItemPacket(2, 0))
	if p.Char.Inv[2].Index != 1737 || !oxNoticeContains(p, "closed") {
		t.Fatal("deadline admitted a late ticket")
	}
}

func oxNoticeContains(p *Player, text string) bool {
	found := false
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			return found
		}
		if wire.Decrypt(packet) && strings.Contains(string(packet[12:]), text) {
			found = true
		}
	}
}

func TestOXAutomaticEnrollmentAnswerAndNextQuestionCadence(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	a := oxTestPlayer(w, 1, 1340, 1460)
	b := oxTestPlayer(w, 2, 1338, 1460)
	outsider := oxTestPlayer(w, 3, 1350, 1460)
	r := runOXEvent(w, "start", true)
	if r.Code != "ok" || r.Status.Phase != "enrollment" || r.Status.Questions != 24 || !r.Status.EndsAt.Equal(clock.Now().Add(time.Minute)) {
		t.Fatalf("start=%+v", r)
	}
	enterOXTest(t, w, a)
	enterOXTest(t, w, b)
	drainPlayerPackets(a)
	drainPlayerPackets(outsider)
	clock.Advance(59999 * time.Millisecond)
	w.tickOXEvent(clock.Now())
	if w.ox.phase != "enrollment" || a.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("early first question")
	}
	clock.Advance(time.Millisecond)
	w.tickOXEvent(clock.Now())
	if w.ox.phase != "question" || !w.ox.deadline.Equal(clock.Now().Add(10*time.Second)) {
		t.Fatal("missing full response window")
	}
	for _, op := range []uint16{wire.OpInstanceTime, wire.OpInstanceQuestion} {
		packet, ok := a.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != op {
			t.Fatalf("missing private packet %#x", op)
		}
		if op == wire.OpInstanceQuestion && (binary.LittleEndian.Uint32(packet[16:20]) != 10000 || wire.ParseHeader(packet).ID != a.ID) {
			t.Fatal("incorrect question lifetime or recipient")
		}
	}
	if outsider.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("question leaked to outsider")
	}
	// The first configured answer is X. Only A earns its existing 8000 EXP.
	placeOXTest(w, a, 1366, 1500)
	placeOXTest(w, b, 1316, 1500)
	drainPlayerPackets(a)
	clock.Advance(9999 * time.Millisecond)
	w.tickOXEvent(clock.Now())
	if a.Char.Exp != 0 || a.Session.QueuedPacketsForTest() != 0 || w.ox.phase != "question" {
		t.Fatal("question ended before ten seconds")
	}
	clock.Advance(time.Millisecond)
	w.tickOXEvent(clock.Now())
	if a.Char.Exp != 8000 || b.Char.Exp != 0 || !w.isOXMember(a) || w.isOXMember(b) || oxArena.contains(b.X, b.Y) {
		t.Fatal("incorrect rewards, survivors or elimination")
	}
	if w.ox.phase != "waiting" || !w.ox.deadline.Equal(clock.Now().Add(50*time.Second)) {
		t.Fatal("next-question wait is not 50 seconds")
	}
	if !oxNoticeContains(a, "50 seconds") {
		t.Fatal("missing next-question countdown")
	}
	drainPlayerPackets(a)
	clock.Advance(49999 * time.Millisecond)
	w.tickOXEvent(clock.Now())
	if w.ox.phase != "waiting" || a.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("next question started early")
	}
	clock.Advance(time.Millisecond)
	w.tickOXEvent(clock.Now())
	if w.ox.phase != "question" || w.ox.round != 1 || !w.ox.deadline.Equal(clock.Now().Add(10*time.Second)) {
		t.Fatal("next question did not follow the minute cadence")
	}
}

func TestOXSideOverridesGeometryAndFullWindow(t *testing.T) {
	for _, action := range []string{"result_x", "result_o"} {
		t.Run(action, func(t *testing.T) {
			w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
			runOXEvent(w, "start", true)
			players := make([]*Player, 4)
			for i := range players {
				players[i] = oxTestPlayer(w, uint16(i+1), 1340, 1460)
				enterOXTest(t, w, players[i])
				placeOXTest(w, players[i], uint16(1340+i), 1500)
			}
			clock.Advance(time.Minute)
			w.tickOXEvent(clock.Now())
			before := w.oxEventSnapshot()
			if before.PlayersX != 2 || before.PlayersO != 1 {
				t.Fatalf("boundary counts=%+v", before)
			}
			if r := runOXEvent(w, action, true); r.Code != "ok" {
				t.Fatalf("override=%+v", r)
			}
			if r := runOXEvent(w, action, true); r.Code != "unchanged" {
				t.Fatal("duplicate override was applied twice")
			}
			clock.Advance(9999 * time.Millisecond)
			w.tickOXEvent(clock.Now())
			for _, p := range players {
				if p.Y != 1500 || p.Char.Exp != 0 {
					t.Fatal("override shortened response window")
				}
			}
			clock.Advance(time.Millisecond)
			w.tickOXEvent(clock.Now())
			for i, p := range players {
				wantLose := i == 1 || action == "result_o" && i == 0 || action == "result_x" && i >= 2
				if (p.Y != 1500) != wantLose || (p.Char.Exp == 0) != wantLose || w.isOXMember(p) == wantLose {
					t.Fatalf("column %d: lost=%v exp=%d", 1340+i, p.Y != 1500, p.Char.Exp)
				}
			}
		})
	}
}

func TestOXDeadLoserRemovesCorpseAndDoesNotEarnReward(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	runOXEvent(w, "start", true)
	loser := oxTestPlayer(w, 1, 1340, 1460)
	winner := oxTestPlayer(w, 2, 1338, 1460)
	enterOXTest(t, w, loser)
	enterOXTest(t, w, winner)
	placeOXTest(w, loser, 1360, 1500)
	placeOXTest(w, winner, 1366, 1500)
	winner.Visible[loser.ID] = struct{}{}
	setPlayerCurHP(loser.Char, 0)
	clock.Advance(time.Minute)
	w.tickOXEvent(clock.Now())
	clock.Advance(10 * time.Second)
	w.tickOXEvent(clock.Now())
	if loser.Char.Exp != 0 || playerCurHP(loser.Char) == 0 || oxArena.contains(loser.X, loser.Y) || !sentRemoveMob(winner, loser.ID, 3) {
		t.Fatal("dead loser retained reward, arena place or corpse")
	}
}

func TestOXAllQuestionsCompleteAndStopEvacuates(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	runOXEvent(w, "start", true)
	p := oxTestPlayer(w, 1, 1340, 1460)
	enterOXTest(t, w, p)
	clock.Advance(time.Minute)
	var expected uint32
	for i, q := range w.ox.questions {
		w.tickOXEvent(clock.Now())
		drainPlayerPackets(p)
		if w.ox.phase != "question" || w.ox.round != i {
			t.Fatalf("round %d not active", i)
		}
		x := uint16(1366)
		if q.Answer {
			x = 1316
		}
		placeOXTest(w, p, x, 1500)
		clock.Advance(10 * time.Second)
		w.tickOXEvent(clock.Now())
		expected += q.RewardExp
		if p.Char.Exp != expected {
			t.Fatalf("round %d EXP=%d want %d", i, p.Char.Exp, expected)
		}
		drainPlayerPackets(p)
		clock.Advance(50 * time.Second)
	}
	if w.oxQuizMode || w.ox.phase != "inactive" {
		t.Fatal("event did not finish after 24 questions")
	}
	w.tickOXEvent(clock.Now())
	if oxArena.contains(p.X, p.Y) || w.isOXMember(p) {
		t.Fatal("completed event left a participant in the arena")
	}
	if r := runOXEvent(w, "start", true); r.Code != "ok" {
		t.Fatal("next event was blocked by stale membership")
	}
}
