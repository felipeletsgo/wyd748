package game

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

type sanctionTestStore struct {
	*craftStore
	state    store.SanctionState
	stateErr error
	added    []control.SanctionCommand
}

func (s *sanctionTestStore) AddSanction(_ context.Context, actor string, cmd control.SanctionCommand) (control.Sanction, bool, error) {
	for _, previous := range s.added {
		if previous.RequestID == cmd.RequestID {
			if previous != cmd {
				return control.Sanction{}, false, control.ErrSanctionConflict
			}
			return control.Sanction{ID: 1, Account: cmd.Account, Kind: cmd.Kind, Actor: actor}, true, nil
		}
	}
	s.added = append(s.added, cmd)
	return control.Sanction{ID: int64(len(s.added)), Account: cmd.Account, Kind: cmd.Kind, Actor: actor, Active: true}, false, nil
}

func (s *sanctionTestStore) RevokeSanction(_ context.Context, id int64, account, kind, actor, _ string) (control.Sanction, bool, error) {
	if id != 1 {
		return control.Sanction{}, false, control.ErrUnknownSanction
	}
	return control.Sanction{ID: id, Account: account, Kind: kind, RevokedBy: actor}, true, nil
}

func (s *sanctionTestStore) ListSanctions(context.Context, string, int) ([]control.Sanction, time.Time, error) {
	return []control.Sanction{}, time.Now(), nil
}

func (s *sanctionTestStore) ActiveSanctionState(context.Context, string) (store.SanctionState, error) {
	return s.state, s.stateErr
}

func panelText(t *testing.T, p *Player) string {
	t.Helper()
	pkt := nextPacket(t, p)
	if wire.ParseHeader(pkt).Type != wire.OpMessagePanel {
		t.Fatalf("expected notice, got opcode 0x%X", wire.ParseHeader(pkt).Type)
	}
	return string(bytes.TrimRight(pkt[12:107], "\x00"))
}

func drainPackets(p *Player) {
	for {
		if _, ok := p.Session.DequeuePacketForTest(); !ok {
			return
		}
	}
}

// runStaffQueue executes queued staff commands like the World loop would.
func runStaffQueue(w *World, stop <-chan struct{}) {
	for {
		select {
		case cmd := <-w.commands:
			w.handle(cmd)
		case <-stop:
			return
		}
	}
}

func TestSanctionBanDisconnectsAndBlocksLogin(t *testing.T) {
	w, p, base := handlerTestWorld(t)
	p.Account.Name = "player"
	w.claimAccountSession(p.Session, p.Account.Name)
	clock := newFakeClock(time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	w.clock = clock
	until := clock.Now().Add(time.Hour)
	if affected := w.applySanctionState("player", store.SanctionState{BanUntil: until}, control.SanctionBan); affected != 1 {
		t.Fatal("ban did not reach the online session", affected)
	}
	if !p.Session.IsClosed() || len(w.players) != 0 || w.accountSessions["player"] != nil || base.saves != 1 {
		t.Fatal("ban must use the authoritative disconnect path", base.saves)
	}
	// A login whose durable read predates the ban is still refused.
	message, banned := w.admitSanctions("Player", store.SanctionState{}, clock.Now())
	if !banned || message != "This account is suspended until 2026-10-21 13:00 UTC." || len(message) > control.MaxNoticeBytes {
		t.Fatal(message, banned)
	}
	clock.Advance(time.Hour)
	if _, banned := w.admitSanctions("player", store.SanctionState{}, clock.Now()); banned {
		t.Fatal("expired ban still blocks login")
	}
	if message, banned := w.admitSanctions("player", store.SanctionState{BanUntil: store.PermanentSanction}, clock.Now()); !banned || !strings.Contains(message, "permanently") {
		t.Fatal(message)
	}
}

func TestSanctionMuteBlocksChatAndLifts(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Account.Name = "player"
	clock := newFakeClock(time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	w.clock = clock
	if w.applySanctionState("player", store.SanctionState{MuteUntil: clock.Now().Add(30 * time.Minute)}, control.SanctionMute) != 1 {
		t.Fatal("mute did not reach the player")
	}
	if got := panelText(t, p); got != "You have been muted until 2026-10-21 12:30 UTC." {
		t.Fatal(got)
	}
	for _, channel := range []string{"local", chatChannelParty, chatChannelGlobal, chatChannelCitizenship, chatChannelServer, chatChannelGuild, chatChannelWhisper} {
		if w.allowChat(p, channel, clock.Now()) {
			t.Fatal("muted channel allowed", channel)
		}
		if got := panelText(t, p); !strings.HasPrefix(got, "You are muted until") {
			t.Fatal(got)
		}
	}
	if !w.allowChat(p, "balance", clock.Now()) {
		t.Fatal("self-only balance query must stay available")
	}
	// Name labels: a muted player cannot set new text but can clear it.
	p.TabText, p.StatusMessage = "Old tab", "Old status"
	w.setPlayerTab(p.Session, p, "Buy gold here", 1)
	if got := panelText(t, p); !strings.HasPrefix(got, "You are muted until") || p.TabText != "Old tab" {
		t.Fatal("muted player set a tab", got, p.TabText)
	}
	w.setPlayerStatus(p.Session, p, "Spam status")
	if got := panelText(t, p); !strings.HasPrefix(got, "You are muted until") || p.StatusMessage != "Old status" {
		t.Fatal("muted player set a status", got)
	}
	w.setPlayerTab(p.Session, p, "", 0)
	w.setPlayerStatus(p.Session, p, "")
	if p.TabText != "" || p.StatusMessage != "" {
		t.Fatal("muted player could not clear labels", p.TabText, p.StatusMessage)
	}
	drainPackets(p)
	w.applySanctionState("player", store.SanctionState{}, "revoke")
	if got := panelText(t, p); got != "Your chat mute has been lifted." {
		t.Fatal(got)
	}
	if !w.allowChat(p, "local", clock.Now()) {
		t.Fatal("revoked mute still blocks chat")
	}
	w.applySanctionState("player", store.SanctionState{MuteUntil: clock.Now().Add(time.Minute)}, "")
	clock.Advance(time.Minute)
	if !w.allowChat(p, "local", clock.Now()) || len(w.staffOps.mutes) != 0 {
		t.Fatal("expired mute still blocks chat")
	}
}

func TestApplySanctionPersistsBeforeLiveEffect(t *testing.T) {
	w, p, base := handlerTestWorld(t)
	p.Account.Name = "player"
	backend := &sanctionTestStore{craftStore: base, state: store.SanctionState{MuteUntil: time.Now().Add(time.Hour)}}
	w.store = backend
	w.commands = make(chan command, 4)
	stop := make(chan struct{})
	defer close(stop)
	go runStaffQueue(w, stop)
	cmd := control.SanctionCommand{Version: 1, RequestID: "request-000000001", Account: "player", Kind: control.SanctionMute, DurationMinutes: 60, Reason: "spam in global"}
	if r, err := w.ApplySanction(context.Background(), "operator", cmd, func() bool { return false }); err != nil || r.Code != "capability_required" || len(backend.added) != 0 {
		t.Fatal("revoked staff persisted a sanction", r, err)
	}
	r, err := w.ApplySanction(context.Background(), "operator", cmd, func() bool { return true })
	if err != nil || r.Code != "ok" || r.Replayed || r.Live != "applied" || len(backend.added) != 1 {
		t.Fatal(r, err)
	}
	r, err = w.ApplySanction(context.Background(), "operator", cmd, func() bool { return true })
	if err != nil || !r.Replayed || len(backend.added) != 1 {
		t.Fatal("retry duplicated the sanction", r, err)
	}
	cmd.Reason = "different reason"
	if r, _ := w.ApplySanction(context.Background(), "operator", cmd, func() bool { return true }); r.Code != "operation_conflict" {
		t.Fatal(r)
	}
	backend.stateErr = errors.New("database down")
	cmd.RequestID, cmd.Reason = "request-000000002", "spam in global"
	if r, _ := w.ApplySanction(context.Background(), "operator", cmd, func() bool { return true }); r.Code != "ok" || r.Live != "pending" {
		t.Fatal("unconfirmed live state must be reported", r)
	}
	revoke := control.SanctionRevokeCommand{Version: 1, Account: "player", Kind: control.SanctionMute, Reason: "appeal accepted"}
	if r, _ := w.RevokeSanction(context.Background(), "operator", 9, revoke, func() bool { return true }); r.Code != "unknown_sanction" {
		t.Fatal(r)
	}
}

func TestLoginRejectsBannedAndUnverifiedSanctions(t *testing.T) {
	for name, result := range map[string]*loginResult{
		"unverified": {accountName: "Felipe", account: loginTestAccount(), sanctionErr: errors.New("timeout")},
		"banned":     {accountName: "Felipe", account: loginTestAccount(), sanctions: store.SanctionState{BanUntil: store.PermanentSanction}},
	} {
		t.Run(name, func(t *testing.T) {
			session := newLoginTestSession(1, 16)
			st := &loginReloadStore{account: loginTestAccount()}
			w := loginResultWorld(st, session)
			w.onLoginResult(session, result)
			if w.players[session] != nil || w.accountSessions["felipe"] != nil || st.loads != 0 || w.authPending[session] {
				t.Fatal("login admitted")
			}
			pkt, ok := session.DequeuePacketForTest()
			if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel {
				t.Fatal("rejection notice missing")
			}
		})
	}
	session := newLoginTestSession(1, 64)
	st := &loginReloadStore{account: loginTestAccount()}
	w := loginResultWorld(st, session)
	w.onLoginResult(session, &loginResult{accountName: "Felipe", account: loginTestAccount(),
		sanctions: store.SanctionState{MuteUntil: store.PermanentSanction}})
	if w.players[session] == nil || w.staffOps.mutes["felipe"] != store.PermanentSanction {
		t.Fatal("a muted account must log in with its mute loaded")
	}
}

func TestStaffNoticesScheduleRepeatAndCancel(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	w.clock = clock
	allow := func() bool { return true }
	now := control.NoticeCommand{Version: 1, OperationID: "operation-0000001", Message: "Double EXP starts now!", RepeatCount: 1}
	r := w.scheduleNotice("operator", now, allow)
	if r.Code != "ok" || r.Delivered != 1 || len(r.View.Notices) != 0 || panelText(t, p) != now.Message {
		t.Fatal(r)
	}
	drainPackets(p)
	if replay := w.scheduleNotice("operator", now, allow); !replay.Replayed || replay.Delivered != 1 || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("replay delivered the notice again", replay)
	}
	now.Message = "Other"
	if conflict := w.scheduleNotice("operator", now, allow); conflict.Code != "operation_conflict" {
		t.Fatal(conflict)
	}
	repeat := control.NoticeCommand{Version: 1, OperationID: "operation-0000002", Message: "Maintenance tonight", DelaySeconds: 60, RepeatCount: 2, RepeatEverySeconds: 120}
	r = w.scheduleNotice("operator", repeat, allow)
	if r.Code != "ok" || r.Delivered != 0 || len(r.View.Notices) != 1 || r.View.Notices[0].Remaining != 2 {
		t.Fatal(r)
	}
	clock.Advance(59 * time.Second)
	w.tickStaffOps(clock.Now())
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("notice delivered early")
	}
	clock.Advance(time.Second)
	w.tickStaffOps(clock.Now())
	if panelText(t, p) != repeat.Message || len(w.staffOps.notices) != 1 || w.staffOps.notices[0].remaining != 1 {
		t.Fatal("first repetition missing")
	}
	drainPackets(p)
	id := w.staffOps.notices[0].id
	cancel := control.NoticeCancelCommand{Version: 1, OperationID: "operation-0000003"}
	if denied := w.scheduleNotice("operator", control.NoticeCommand{Version: 1, OperationID: "operation-0000004", Message: "x", RepeatCount: 1}, func() bool { return false }); denied.Code != "capability_required" {
		t.Fatal(denied)
	}
	w.commands = make(chan command, 1)
	stop := make(chan struct{})
	defer close(stop)
	go runStaffQueue(w, stop)
	if r, err := w.CancelNotice(context.Background(), "operator", id, cancel, allow); err != nil || r.Code != "ok" || len(r.View.Notices) != 0 {
		t.Fatal(r, err)
	}
	if r, _ := w.CancelNotice(context.Background(), "operator", id, control.NoticeCancelCommand{Version: 1, OperationID: "operation-0000005"}, allow); r.Code != "unknown_notice" {
		t.Fatal(r)
	}
}

func TestStaffPlayerMessageTargetsObservedSession(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	p.Account.Name = "player"
	cmd := control.PlayerMessageCommand{Version: 1, OperationID: "operation-0000001", Epoch: w.moderationEpoch(), UID: p.Char.UID,
		Account: "player", ExpectedSessionID: p.Session.ID, Message: "Please contact staff on Discord."}
	w.commands = make(chan command, 1)
	stop := make(chan struct{})
	defer close(stop)
	go runStaffQueue(w, stop)
	r, err := w.MessagePlayer(context.Background(), "operator", cmd, func() bool { return true })
	if err != nil || r.Code != "ok" || panelText(t, p) != cmd.Message {
		t.Fatal(r, err)
	}
	cmd.OperationID, cmd.ExpectedSessionID = "operation-0000002", p.Session.ID+1
	if r, _ := w.MessagePlayer(context.Background(), "operator", cmd, func() bool { return true }); r.Code != "stale_target" {
		t.Fatal(r)
	}
	cmd.Message = "Badé"
	if _, err := w.MessagePlayer(context.Background(), "operator", cmd, func() bool { return true }); err == nil {
		t.Fatal("non-ASCII notice accepted")
	}
}

func TestScheduledShutdownCountdownAndCancel(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	w.clock = clock
	allow := func() bool { return true }
	schedule := control.ShutdownCommand{Version: 1, OperationID: "operation-0000001", Action: "schedule", Mode: control.ShutdownModeRestart, DelaySeconds: 300, Reason: "apply patch 10042"}
	if r := w.shutdownCommand("operator", schedule, allow); r.Code != "shutdown_unavailable" {
		t.Fatal("scheduled without a process hook", r)
	}
	requested := make(chan string, 1)
	w.staffOps.request = func(mode string) { requested <- mode }
	schedule.OperationID = "operation-0000002"
	r := w.shutdownCommand("operator", schedule, allow)
	if r.Code != "ok" || !r.Status.Scheduled || r.Status.SecondsRemaining != 300 || !r.Status.Available || r.Status.LoginsBlocked {
		t.Fatal(r)
	}
	if got := panelText(t, p); got != "Server restart in 5 minutes. Please log out safely." {
		t.Fatal(got)
	}
	drainPackets(p)
	schedule.OperationID = "operation-0000003"
	if again := w.shutdownCommand("operator", schedule, allow); again.Code != "already_scheduled" {
		t.Fatal(again)
	}
	// One announcement per crossed mark, even when ticks skip several marks.
	clock.Advance(170 * time.Second)
	w.tickStaffOps(clock.Now())
	if got := panelText(t, p); got != "Server restart in 2 minutes. Please log out safely." {
		t.Fatal(got)
	}
	drainPackets(p)
	w.tickStaffOps(clock.Now())
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("countdown repeated a mark")
	}
	clock.Advance(70 * time.Second)
	if !w.shutdownLoginsBlocked(clock.Now()) {
		t.Fatal("logins open in the final minute")
	}
	cancel := control.ShutdownCommand{Version: 1, OperationID: "operation-0000004", Action: "cancel", Reason: "patch postponed"}
	if r := w.shutdownCommand("operator", cancel, allow); r.Code != "ok" || r.Status.Scheduled {
		t.Fatal(r)
	}
	drainPackets(p)
	if w.shutdownLoginsBlocked(clock.Now()) {
		t.Fatal("cancel left logins blocked")
	}
	schedule.OperationID, schedule.DelaySeconds, schedule.Mode = "operation-0000005", 60, control.ShutdownModeShutdown
	if r := w.shutdownCommand("operator", schedule, allow); r.Code != "ok" {
		t.Fatal(r)
	}
	clock.Advance(time.Minute)
	w.tickStaffOps(clock.Now())
	select {
	case mode := <-requested:
		if mode != control.ShutdownModeShutdown {
			t.Fatal(mode)
		}
	case <-time.After(time.Second):
		t.Fatal("shutdown hook not called")
	}
	cancel.OperationID = "operation-0000006"
	if r := w.shutdownCommand("operator", cancel, allow); r.Code != "already_started" {
		t.Fatal("started shutdown was cancelled", r)
	}
}

func TestStaffBroadcastsAreCappedAtOnePerTick(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	w.clock = clock
	w.staffOps.request = func(string) {}
	allow := func() bool { return true }
	for i, message := range []string{"First notice", "Second notice"} {
		cmd := control.NoticeCommand{Version: 1, OperationID: "operation-000000" + string(rune('a'+i)) + "x", Message: message, DelaySeconds: 60, RepeatCount: 1}
		if r := w.scheduleNotice("operator", cmd, allow); r.Code != "ok" {
			t.Fatal(r)
		}
	}
	shutdown := control.ShutdownCommand{Version: 1, OperationID: "operation-0000009", Action: "schedule", Mode: control.ShutdownModeShutdown, DelaySeconds: 180, Reason: "cap test"}
	if r := w.shutdownCommand("operator", shutdown, allow); r.Code != "ok" {
		t.Fatal(r)
	}
	drainPackets(p)
	clock.Advance(time.Minute) // both notices and the 2-minute mark are due
	want := []string{"Server maintenance in 2 minutes. Please log out safely.", "First notice", "Second notice"}
	for _, message := range want {
		w.tickStaffOps(clock.Now())
		if got := panelText(t, p); got != message {
			t.Fatalf("got %q, want %q", got, message)
		}
		if p.Session.QueuedPacketsForTest() != 1 { // the matching [Staff] chat line only
			t.Fatal("more than one broadcast in a tick")
		}
		drainPackets(p)
	}
	w.tickStaffOps(clock.Now())
	if p.Session.QueuedPacketsForTest() != 0 || len(w.staffOps.notices) != 0 {
		t.Fatal("delivered notices were not removed")
	}
}

func TestLoginHistoryIsQueuedWithoutBlocking(t *testing.T) {
	w := &World{loginHistory: make(chan loginRecord, 1)}
	w.recordLogin("Player", "198.51.100.4")
	before := metricLoginHistoryDropped.Value()
	w.recordLogin("Other", "198.51.100.5") // queue full: dropped, never blocks the World
	if got := <-w.loginHistory; got.account != "Player" || got.clientIP != "198.51.100.4" {
		t.Fatal(got)
	}
	if metricLoginHistoryDropped.Value() != before+1 {
		t.Fatal("dropped record not counted")
	}
	(&World{}).recordLogin("nobody", "") // stores without history: no-op
}

func TestStaffReceiptBudgetsArePerKindAndExpire(t *testing.T) {
	w, _, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Date(2026, 10, 22, 12, 0, 0, 0, time.UTC))
	w.clock = clock
	allow := func() bool { return true }
	for i := range maxStaffReceiptsPerKind {
		cmd := control.NoticeCommand{Version: 1, OperationID: fmt.Sprintf("operation-%010d", i), Message: "spam", RepeatCount: 1}
		if r := w.scheduleNotice("operator", cmd, allow); r.Code != "ok" {
			t.Fatal(i, r.Code)
		}
	}
	extra := control.NoticeCommand{Version: 1, OperationID: "operation-extra-000", Message: "one more", RepeatCount: 1}
	if r := w.scheduleNotice("operator", extra, allow); r.Code != "operation_capacity" {
		t.Fatal("notice budget not enforced", r.Code)
	}
	w.staffOps.request = func(string) {}
	shutdown := control.ShutdownCommand{Version: 1, OperationID: "operation-shutdown1", Action: "schedule", Mode: control.ShutdownModeRestart, DelaySeconds: 600, Reason: "budget test"}
	if r := w.shutdownCommand("operator", shutdown, allow); r.Code != "ok" {
		t.Fatal("notices exhausted the shutdown budget", r.Code)
	}
	// A prompt retry of an old notice still replays instead of resending.
	first := control.NoticeCommand{Version: 1, OperationID: fmt.Sprintf("operation-%010d", 0), Message: "spam", RepeatCount: 1}
	if r := w.scheduleNotice("operator", first, allow); !r.Replayed {
		t.Fatal("young receipt evicted")
	}
	clock.Advance(staffReceiptTTL + time.Minute)
	extra.OperationID = "operation-extra-001"
	if r := w.scheduleNotice("operator", extra, allow); r.Code != "ok" || w.staffOps.receiptCount["notice"] != 1 || w.staffOps.receiptCount["shutdown"] != 1 {
		t.Fatal("expired receipts were not evicted", r.Code, w.staffOps.receiptCount)
	}
}
