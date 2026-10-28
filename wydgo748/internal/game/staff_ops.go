package game

import (
	"context"
	"errors"
	"fmt"
	"log"
	"sort"
	"strconv"
	"sync/atomic"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// MODERNIZACAO_COMPATIVEL: staff operations reuse the native 0x101 notice,
// 0x334 whisper and the authoritative disconnect lifecycle. Sanctions are
// durable rows read at login; the World keeps only their live effect.

const (
	maxStaffInFlight = 4
	// Each operation kind has its own receipt budget, so heavy notice use can
	// never block a shutdown. A receipt only serves retries of the same
	// request; past staffReceiptTTL it may be evicted to make room.
	maxStaffReceiptsPerKind = 1024
	staffReceiptTTL         = 24 * time.Hour
	maxScheduledNotices     = 32
	staffNoticeSender       = "[Staff]"
	// Restart exits with EX_TEMPFAIL so a supervisor with Restart=on-failure
	// starts the process again; a plain shutdown exits zero.
	RestartExitCode = 75
)

type staffRequest struct {
	ctx   context.Context
	label string
	run   func()
	// release frees the admission slot that let this request in.
	release func()
}

type staffReceipt struct {
	kind        string
	fingerprint string
	result      any
	at          time.Time
}

type staffNotice struct {
	id        int64
	message   string
	actor     string
	nextAt    time.Time
	remaining int
	every     time.Duration
}

type shutdownPlan struct {
	mode      string
	at        time.Time
	actor     string
	reason    string
	lastMark  time.Duration
	requested bool
}

type staffOpsState struct {
	pending  atomic.Int32
	receipts map[string]staffReceipt
	// receiptCount counts receipts per kind.
	receiptCount map[string]int
	bans         map[string]time.Time
	mutes        map[string]time.Time
	// World-owned generations fence database reads and pending authentication
	// independently per normalized account, including revocation tombstones.
	sanctionVersions map[string]uint64
	notices          []*staffNotice
	noticeSeq        int64
	shutdown         *shutdownPlan
	request          func(mode string)
}

// WithShutdownRequest installs the process hook a scheduled shutdown calls
// when its countdown ends. Without it, scheduling is refused.
func WithShutdownRequest(request func(mode string)) WorldOption {
	return func(w *World) { w.staffOps.request = request }
}

type sanctionStore interface {
	AddSanction(context.Context, string, control.SanctionCommand) (control.Sanction, bool, error)
	RevokeSanction(context.Context, int64, string, string, string, string) (control.Sanction, bool, error)
	ListSanctions(context.Context, string, int) ([]control.Sanction, time.Time, error)
	ActiveSanctionState(context.Context, string) (store.SanctionState, error)
}

func (w *World) sanctionBackend() (sanctionStore, bool) {
	backend, ok := w.store.(sanctionStore)
	return backend, ok
}

// submitStaff runs fn on the World goroutine. A request whose context ended
// before it was dequeued does not run.
func (w *World) submitStaff(ctx context.Context, label string, fn func()) error {
	if err := ctx.Err(); err != nil {
		return err
	}
	if w.staffOps.pending.Add(1) > maxStaffInFlight {
		w.staffOps.pending.Add(-1)
		return control.ErrBusy
	}
	return w.queueStaff(ctx, label, fn, func() { w.staffOps.pending.Add(-1) })
}

// queueStaff runs fn on the World goroutine; release always runs once.
func (w *World) queueStaff(ctx context.Context, label string, fn, release func()) error {
	done := make(chan struct{})
	req := &staffRequest{ctx: ctx, label: label, run: func() { defer close(done); fn() }, release: release}
	select {
	case w.commands <- command{staff: req, queuedAt: time.Now()}:
	default:
		release()
		return control.ErrBusy
	}
	select {
	case <-done:
		return nil
	case <-ctx.Done():
		return ctx.Err()
	}
}

func (w *World) handleStaff(req *staffRequest) {
	defer req.release()
	if req.ctx.Err() != nil {
		return
	}
	req.run()
}

// staffReceiptFor returns a previous result for the same actor and operation.
// conflict reports an operation ID reused for a different command.
func (w *World) staffReceiptFor(kind, actor, operation, fingerprint string) (result any, replayed, conflict bool) {
	receipt, ok := w.staffOps.receipts[kind+":"+actor+":"+operation]
	if !ok {
		return nil, false, false
	}
	if receipt.fingerprint != fingerprint {
		return nil, false, true
	}
	return receipt.result, true, false
}

// staffReceiptRoom reports whether kind can store another receipt. When the
// kind is full it first evicts its receipts older than staffReceiptTTL; a
// younger receipt is never evicted, so a prompt retry cannot repeat.
func (w *World) staffReceiptRoom(kind string) bool {
	if w.staffOps.receiptCount[kind] < maxStaffReceiptsPerKind {
		return true
	}
	now := w.now()
	for key, receipt := range w.staffOps.receipts {
		if receipt.kind == kind && now.Sub(receipt.at) > staffReceiptTTL {
			delete(w.staffOps.receipts, key)
			w.staffOps.receiptCount[kind]--
		}
	}
	return w.staffOps.receiptCount[kind] < maxStaffReceiptsPerKind
}

// rememberStaffReceipt reports false when the kind has no room left.
func (w *World) rememberStaffReceipt(kind, actor, operation, fingerprint string, result any) bool {
	if w.staffOps.receipts == nil {
		w.staffOps.receipts = make(map[string]staffReceipt)
		w.staffOps.receiptCount = make(map[string]int)
	}
	key := kind + ":" + actor + ":" + operation
	if _, exists := w.staffOps.receipts[key]; !exists {
		if !w.staffReceiptRoom(kind) {
			return false
		}
		w.staffOps.receiptCount[kind]++
	}
	w.staffOps.receipts[key] = staffReceipt{kind: kind, fingerprint: fingerprint, result: result, at: w.now()}
	return true
}

// ---- Sanctions ----

func sanctionUntilText(until time.Time) string {
	if !until.Before(store.PermanentSanction) {
		return "permanently"
	}
	return "until " + until.UTC().Format("2006-01-02 15:04") + " UTC"
}

func activeUntil(entries map[string]time.Time, account string, now time.Time) (time.Time, bool) {
	until, ok := entries[account]
	if !ok {
		return time.Time{}, false
	}
	if !until.After(now) {
		delete(entries, account)
		return time.Time{}, false
	}
	return until, true
}

func setSanctionEntry(entries *map[string]time.Time, account string, until, now time.Time) {
	if !until.After(now) {
		delete(*entries, account)
		return
	}
	if *entries == nil {
		*entries = make(map[string]time.Time)
	}
	(*entries)[account] = until
}

func laterTime(a, b time.Time) time.Time {
	if b.After(a) {
		return b
	}
	return a
}

// mutedChatChannel lists player-authored chat visible to others. Balance and
// medal queries only answer the sender and stay available. Name labels are
// checked by labelMuted instead, because clearing one must stay possible.
func mutedChatChannel(channel string) bool {
	switch channel {
	case "local", chatChannelParty, chatChannelGlobal, chatChannelCitizenship,
		chatChannelServer, chatChannelGuild, chatChannelWhisper:
		return true
	}
	return false
}

// chatMuted runs on the World goroutine from allowChat.
func (w *World) chatMuted(p *Player, channel string, now time.Time) bool {
	return mutedChatChannel(channel) && w.muteBlocks(p, now)
}

// labelMuted refuses a new tab or status text from a muted account. An empty
// text clears the label and is always allowed.
func (w *World) labelMuted(p *Player, text string, now time.Time) bool {
	return text != "" && w.muteBlocks(p, now)
}

// muteBlocks reports an active mute and tells the player until when.
func (w *World) muteBlocks(p *Player, now time.Time) bool {
	until, muted := activeUntil(w.staffOps.mutes, accountSessionKey(p.Account.Name), now)
	if muted && p.Session != nil {
		p.Session.Send(wire.MessagePanel("You are muted " + sanctionUntilText(until) + "."))
	}
	return muted
}

// admitSanctions merges the state read at login with live updates that may
// have arrived after that read. It reports a rejection message for a ban.
func (w *World) admitSanctions(account string, state store.SanctionState, now time.Time) (string, bool) {
	key := accountSessionKey(account)
	liveBan, _ := activeUntil(w.staffOps.bans, key, now)
	liveMute, _ := activeUntil(w.staffOps.mutes, key, now)
	setSanctionEntry(&w.staffOps.mutes, key, laterTime(state.MuteUntil, liveMute), now)
	if ban := laterTime(state.BanUntil, liveBan); ban.After(now) {
		return "This account is suspended " + sanctionUntilText(ban) + ".", true
	}
	return "", false
}

// applySanctionState replaces the live effect for one account and returns
// how many of its sessions were connected. A ban disconnects them through
// the normal persistence path.
func (w *World) applySanctionState(account string, state store.SanctionState, notify string) int {
	if w.cluster != nil {
		affected := 0
		_, wasMuted := activeUntil(w.staffOps.mutes, accountSessionKey(account), w.now())
		for _, channel := range w.cluster.worlds {
			w.cluster.inWorld(channel, func() { affected += channel.applyLocalSanctionStateWithMute(account, state, notify, wasMuted) })
		}
		return affected
	}
	return w.applyLocalSanctionState(account, state, notify)
}

func (w *World) applyLocalSanctionState(account string, state store.SanctionState, notify string) int {
	_, wasMuted := activeUntil(w.staffOps.mutes, accountSessionKey(account), w.now())
	return w.applyLocalSanctionStateWithMute(account, state, notify, wasMuted)
}

func (w *World) applyLocalSanctionStateWithMute(account string, state store.SanctionState, notify string, wasMuted bool) int {
	now := w.now()
	key := accountSessionKey(account)
	setSanctionEntry(&w.staffOps.bans, key, state.BanUntil, now)
	setSanctionEntry(&w.staffOps.mutes, key, state.MuteUntil, now)
	var sessions []*net.Session
	for s, p := range w.players {
		if s != nil && p != nil && p.Account != nil && p.Session == s && !s.IsClosed() &&
			accountSessionKey(p.Account.Name) == key {
			sessions = append(sessions, s)
		}
	}
	sort.Slice(sessions, func(i, j int) bool { return sessions[i].ID < sessions[j].ID })
	banned := state.BanUntil.After(now)
	for _, s := range sessions {
		if banned {
			// Close first so pending packets cannot act while cleanup/persistence runs.
			s.Close()
			w.onDisconnect(s)
			continue
		}
		switch {
		case notify == control.SanctionMute && state.MuteUntil.After(now):
			s.Send(wire.MessagePanel("You have been muted " + sanctionUntilText(state.MuteUntil) + "."))
		case notify == "revoke" && wasMuted && !state.MuteUntil.After(now):
			s.Send(wire.MessagePanel("Your chat mute has been lifted."))
		}
	}
	return len(sessions)
}

func (w *World) pushSanctionState(ctx context.Context, backend sanctionStore, account, notify string) string {
	key := accountSessionKey(account)
	var version uint64
	if err := w.submitStaff(ctx, "control.sanction.begin", func() {
		if w.staffOps.sanctionVersions == nil {
			w.staffOps.sanctionVersions = make(map[string]uint64)
		}
		w.staffOps.sanctionVersions[key]++
		version = w.staffOps.sanctionVersions[key]
	}); err != nil {
		return "pending"
	}
	state, err := backend.ActiveSanctionState(ctx, account)
	if err != nil {
		log.Printf("[web-admin] sanction live refresh account=%q: %v", account, err)
		return "pending"
	}
	affected, applied := 0, false
	if err := w.submitStaff(ctx, "control.sanction", func() {
		// A newer read may already have observed a revocation or a new ban.
		// Never let this older snapshot restore or remove its live effect.
		if w.staffOps.sanctionVersions[key] != version {
			return
		}
		affected = w.applySanctionState(account, state, notify)
		applied = true
	}); err != nil || !applied {
		return "pending"
	}
	if affected > 0 {
		return "applied"
	}
	return "offline"
}

func sanctionStoreError(err error) error {
	if store.IsAccountMissing(err) {
		return control.ErrUnknownAccount
	}
	return err
}

func (w *World) AccountSanctions(ctx context.Context, account string) (control.SanctionsView, error) {
	backend, ok := w.sanctionBackend()
	if !ok {
		return control.SanctionsView{}, control.ErrSanctionsUnavailable
	}
	if !control.ValidAccountKey(account) {
		return control.SanctionsView{}, errors.New("invalid_account")
	}
	sanctions, asOf, err := backend.ListSanctions(ctx, account, 50)
	if err != nil {
		return control.SanctionsView{}, sanctionStoreError(err)
	}
	return control.SanctionsView{Version: 1, AsOf: asOf.UTC(), Account: account, Sanctions: sanctions}, nil
}

func (w *World) ApplySanction(ctx context.Context, actor string, cmd control.SanctionCommand, authorize func() bool) (control.SanctionResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.SanctionResult{}, err
	}
	backend, ok := w.sanctionBackend()
	if !ok {
		return control.SanctionResult{}, control.ErrSanctionsUnavailable
	}
	if actor == "" || authorize == nil || !authorize() {
		return control.SanctionResult{Code: "capability_required"}, nil
	}
	sanction, duplicate, err := backend.AddSanction(ctx, actor, cmd)
	if errors.Is(err, control.ErrSanctionConflict) {
		return control.SanctionResult{Code: "operation_conflict"}, nil
	}
	if err != nil {
		return control.SanctionResult{}, sanctionStoreError(err)
	}
	notify := cmd.Kind
	if duplicate {
		notify = ""
	}
	live := w.pushSanctionState(ctx, backend, cmd.Account, notify)
	log.Printf("[web-admin] sanction actor=%q account=%q kind=%s minutes=%d id=%d replayed=%t live=%s reason=%q",
		actor, cmd.Account, cmd.Kind, cmd.DurationMinutes, sanction.ID, duplicate, live, cmd.Reason)
	return control.SanctionResult{Code: "ok", Replayed: duplicate, Live: live, Sanction: sanction}, nil
}

func (w *World) RevokeSanction(ctx context.Context, actor string, id int64, cmd control.SanctionRevokeCommand, authorize func() bool) (control.SanctionResult, error) {
	if err := cmd.Validate(); err != nil || id < 1 {
		return control.SanctionResult{}, errors.New("invalid_command")
	}
	backend, ok := w.sanctionBackend()
	if !ok {
		return control.SanctionResult{}, control.ErrSanctionsUnavailable
	}
	if actor == "" || authorize == nil || !authorize() {
		return control.SanctionResult{Code: "capability_required"}, nil
	}
	sanction, changed, err := backend.RevokeSanction(ctx, id, cmd.Account, cmd.Kind, actor, cmd.Reason)
	if errors.Is(err, control.ErrUnknownSanction) {
		return control.SanctionResult{Code: "unknown_sanction"}, nil
	}
	if err != nil {
		return control.SanctionResult{}, sanctionStoreError(err)
	}
	// Refresh even for a repeated revoke: it repairs a live state that a
	// previous attempt could not confirm.
	live := w.pushSanctionState(ctx, backend, cmd.Account, "revoke")
	log.Printf("[web-admin] sanction revoke actor=%q account=%q id=%d changed=%t live=%s reason=%q",
		actor, cmd.Account, id, changed, live, cmd.Reason)
	return control.SanctionResult{Code: "ok", Replayed: !changed, Live: live, Sanction: sanction}, nil
}

// ---- Notices ----

func (w *World) staffBroadcast(message string) int {
	delivered := 0
	for _, p := range w.globalPlayers() {
		if p != nil && p.InWorld && p.Session != nil {
			p.Session.Send(wire.MessagePanel(message))
			p.Session.Send(wire.MessageWhisper(0, staffNoticeSender, message, 7))
			delivered++
		}
	}
	return delivered
}

func (w *World) noticesView() control.NoticesView {
	view := control.NoticesView{Version: 1, AsOf: w.now().UTC(), Notices: []control.ScheduledNotice{}}
	for _, n := range w.staffOps.notices {
		view.Notices = append(view.Notices, control.ScheduledNotice{ID: n.id, Message: n.message, Actor: n.actor,
			NextAt: n.nextAt.UTC(), Remaining: n.remaining, RepeatEverySeconds: int(n.every / time.Second)})
	}
	return view
}

func (w *World) Notices(ctx context.Context) (control.NoticesView, error) {
	var view control.NoticesView
	err := w.submitStaff(ctx, "control.notice", func() { view = w.noticesView() })
	return view, err
}

func (w *World) ScheduleNotice(ctx context.Context, actor string, cmd control.NoticeCommand, authorize func() bool) (control.NoticeResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.NoticeResult{}, err
	}
	var result control.NoticeResult
	err := w.submitStaff(ctx, "control.notice", func() { result = w.scheduleNotice(actor, cmd, authorize) })
	return result, err
}

func (w *World) scheduleNotice(actor string, cmd control.NoticeCommand, authorize func() bool) control.NoticeResult {
	result := control.NoticeResult{OperationID: cmd.OperationID}
	fingerprint := fmt.Sprintf("%#v", cmd)
	if previous, replayed, conflict := w.staffReceiptFor("notice", actor, cmd.OperationID, fingerprint); conflict {
		result.Code = "operation_conflict"
		result.View = w.noticesView()
		return result
	} else if replayed {
		result = previous.(control.NoticeResult)
		result.Replayed = true
		result.View = w.noticesView()
		return result
	}
	switch {
	case actor == "" || authorize == nil || !authorize():
		result.Code = "capability_required"
	case w.shuttingDown:
		result.Code = "server_maintenance"
	case !w.staffReceiptRoom("notice"):
		result.Code = "operation_capacity"
	case len(w.staffOps.notices) >= maxScheduledNotices && (cmd.DelaySeconds > 0 || cmd.RepeatCount > 1):
		result.Code = "notice_capacity"
	default:
		now := w.now()
		remaining := cmd.RepeatCount
		next := now.Add(time.Duration(cmd.DelaySeconds) * time.Second)
		if cmd.DelaySeconds == 0 {
			result.Delivered = w.staffBroadcast(cmd.Message)
			remaining--
			next = now.Add(time.Duration(cmd.RepeatEverySeconds) * time.Second)
		}
		if remaining > 0 {
			w.staffOps.noticeSeq++
			w.staffOps.notices = append(w.staffOps.notices, &staffNotice{id: w.staffOps.noticeSeq, message: cmd.Message,
				actor: actor, nextAt: next, remaining: remaining, every: time.Duration(cmd.RepeatEverySeconds) * time.Second})
		}
		result.Code = "ok"
		log.Printf("[web-admin] notice actor=%q operation=%q delivered=%d remaining=%d message=%q",
			actor, cmd.OperationID, result.Delivered, remaining, cmd.Message)
	}
	if result.Code != "capability_required" {
		w.rememberStaffReceipt("notice", actor, cmd.OperationID, fingerprint, result)
	}
	result.View = w.noticesView()
	return result
}

func (w *World) CancelNotice(ctx context.Context, actor string, id int64, cmd control.NoticeCancelCommand, authorize func() bool) (control.NoticeResult, error) {
	if err := cmd.Validate(); err != nil || id < 1 {
		return control.NoticeResult{}, errors.New("invalid_command")
	}
	var result control.NoticeResult
	err := w.submitStaff(ctx, "control.notice", func() {
		result = control.NoticeResult{OperationID: cmd.OperationID}
		fingerprint := strconv.FormatInt(id, 10)
		if previous, replayed, conflict := w.staffReceiptFor("notice-cancel", actor, cmd.OperationID, fingerprint); conflict {
			result.Code = "operation_conflict"
		} else if replayed {
			result = previous.(control.NoticeResult)
			result.Replayed = true
		} else if actor == "" || authorize == nil || !authorize() {
			result.Code = "capability_required"
		} else if !w.staffReceiptRoom("notice-cancel") {
			result.Code = "operation_capacity"
		} else {
			result.Code = "unknown_notice"
			for i, n := range w.staffOps.notices {
				if n.id == id {
					w.staffOps.notices = append(w.staffOps.notices[:i], w.staffOps.notices[i+1:]...)
					result.Code = "ok"
					log.Printf("[web-admin] notice cancel actor=%q operation=%q id=%d", actor, cmd.OperationID, id)
					break
				}
			}
			w.rememberStaffReceipt("notice-cancel", actor, cmd.OperationID, fingerprint, result)
		}
		result.View = w.noticesView()
	})
	return result, err
}

func (w *World) MessagePlayer(ctx context.Context, actor string, cmd control.PlayerMessageCommand, authorize func() bool) (control.PlayerMessageResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.PlayerMessageResult{}, err
	}
	var result control.PlayerMessageResult
	err := w.submitStaff(ctx, "control.player-message", func() {
		result = control.PlayerMessageResult{OperationID: cmd.OperationID}
		fingerprint := fmt.Sprintf("%#v", cmd)
		if previous, replayed, conflict := w.staffReceiptFor("message", actor, cmd.OperationID, fingerprint); conflict {
			result.Code = "operation_conflict"
			return
		} else if replayed {
			result = previous.(control.PlayerMessageResult)
			result.Replayed = true
			return
		}
		switch {
		case actor == "" || authorize == nil || !authorize():
			result.Code = "capability_required"
			return
		case cmd.Epoch != w.moderationEpoch():
			result.Code = "stale_target"
			return
		case !w.staffReceiptRoom("message"):
			result.Code = "operation_capacity"
			return
		}
		result.Code = "stale_target"
		for s, p := range w.players {
			if s == nil || s.ID != cmd.ExpectedSessionID || p == nil || p.Session != s || !p.InWorld || p.Char == nil ||
				p.Account == nil || p.Char.UID != cmd.UID || accountSessionKey(p.Account.Name) != cmd.Account || s.IsClosed() {
				continue
			}
			s.Send(wire.MessagePanel(cmd.Message))
			s.Send(wire.MessageWhisper(0, staffNoticeSender, cmd.Message, 7))
			result.Code = "ok"
			break
		}
		w.rememberStaffReceipt("message", actor, cmd.OperationID, fingerprint, result)
		log.Printf("[web-admin] player message actor=%q operation=%q uid=%q result=%s message=%q",
			actor, cmd.OperationID, cmd.UID, result.Code, cmd.Message)
	})
	return result, err
}

// ---- Scheduled shutdown ----

var shutdownMarks = []time.Duration{60 * time.Minute, 30 * time.Minute, 15 * time.Minute, 10 * time.Minute,
	5 * time.Minute, 3 * time.Minute, 2 * time.Minute, time.Minute, 30 * time.Second, 10 * time.Second}

func countdownText(left time.Duration) string {
	if left >= time.Minute {
		minutes := int((left + 30*time.Second) / time.Minute)
		if minutes == 1 {
			return "1 minute"
		}
		return strconv.Itoa(minutes) + " minutes"
	}
	seconds := int((left + time.Second/2) / time.Second)
	if seconds == 1 {
		return "1 second"
	}
	return strconv.Itoa(seconds) + " seconds"
}

func shutdownNotice(mode string, left time.Duration) string {
	if mode == control.ShutdownModeRestart {
		return "Server restart in " + countdownText(left) + ". Please log out safely."
	}
	return "Server maintenance in " + countdownText(left) + ". Please log out safely."
}

// shutdownLoginsBlocked runs on the World goroutine at login start and completion.
func (w *World) shutdownLoginsBlocked(now time.Time) bool {
	plan := w.staffOps.shutdown
	return plan != nil && (plan.requested || plan.at.Sub(now) <= control.ShutdownLoginCutoff)
}

func (w *World) shutdownStatus() control.ShutdownStatus {
	now := w.now()
	status := control.ShutdownStatus{Version: 1, AsOf: now.UTC(), Available: w.staffOps.request != nil}
	if plan := w.staffOps.shutdown; plan != nil {
		at := plan.at.UTC()
		status.Scheduled, status.Mode, status.At = true, plan.mode, &at
		status.Actor, status.Reason = plan.actor, plan.reason
		status.SecondsRemaining = max(int64(plan.at.Sub(now)/time.Second), 0)
		status.LoginsBlocked = w.shutdownLoginsBlocked(now)
	}
	return status
}

func (w *World) ShutdownStatus(ctx context.Context) (control.ShutdownStatus, error) {
	var status control.ShutdownStatus
	err := w.submitStaff(ctx, "control.shutdown", func() { status = w.shutdownStatus() })
	return status, err
}

func (w *World) ShutdownCommand(ctx context.Context, actor string, cmd control.ShutdownCommand, authorize func() bool) (control.ShutdownResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.ShutdownResult{}, err
	}
	var result control.ShutdownResult
	err := w.submitStaff(ctx, "control.shutdown", func() { result = w.shutdownCommand(actor, cmd, authorize) })
	return result, err
}

func (w *World) shutdownCommand(actor string, cmd control.ShutdownCommand, authorize func() bool) control.ShutdownResult {
	result := control.ShutdownResult{OperationID: cmd.OperationID}
	fingerprint := fmt.Sprintf("%#v", cmd)
	if previous, replayed, conflict := w.staffReceiptFor("shutdown", actor, cmd.OperationID, fingerprint); conflict {
		result.Code = "operation_conflict"
		result.Status = w.shutdownStatus()
		return result
	} else if replayed {
		result = previous.(control.ShutdownResult)
		result.Replayed = true
		result.Status = w.shutdownStatus()
		return result
	}
	now := w.now()
	plan := w.staffOps.shutdown
	switch {
	case actor == "" || authorize == nil || !authorize():
		result.Code = "capability_required"
	case w.shuttingDown:
		result.Code = "server_maintenance"
	case !w.staffReceiptRoom("shutdown"):
		result.Code = "operation_capacity"
	case cmd.Action == "schedule" && w.staffOps.request == nil:
		result.Code = "shutdown_unavailable"
	case cmd.Action == "schedule" && plan != nil:
		result.Code = "already_scheduled"
	case cmd.Action == "schedule":
		delay := time.Duration(cmd.DelaySeconds) * time.Second
		w.staffOps.shutdown = &shutdownPlan{mode: cmd.Mode, at: now.Add(delay), actor: actor, reason: cmd.Reason, lastMark: delay}
		w.staffBroadcast(shutdownNotice(cmd.Mode, delay))
		result.Code = "ok"
	case plan == nil:
		result.Code = "not_scheduled"
	case plan.requested:
		result.Code = "already_started"
	default:
		w.staffOps.shutdown = nil
		w.staffBroadcast("The scheduled server maintenance was cancelled.")
		result.Code = "ok"
	}
	if result.Code != "capability_required" {
		w.rememberStaffReceipt("shutdown", actor, cmd.OperationID, fingerprint, result)
	}
	log.Printf("[web-admin] shutdown actor=%q operation=%q action=%s mode=%s delay=%ds result=%s reason=%q",
		actor, cmd.OperationID, cmd.Action, cmd.Mode, cmd.DelaySeconds, result.Code, cmd.Reason)
	result.Status = w.shutdownStatus()
	return result
}

// tickStaffOps advances the shutdown countdown and delivers due notices. A
// broadcast costs the World O(online players), so a tick sends at most one:
// the countdown first, then the earliest due notice; others wait a tick.
func (w *World) tickStaffOps(now time.Time) {
	if w.tickShutdownCountdown(now) {
		return
	}
	var due *staffNotice
	index := -1
	for i, n := range w.staffOps.notices {
		if !now.Before(n.nextAt) && (due == nil || n.nextAt.Before(due.nextAt)) {
			due, index = n, i
		}
	}
	if due == nil {
		return
	}
	w.staffBroadcast(due.message)
	due.remaining--
	if due.remaining <= 0 {
		w.staffOps.notices = append(w.staffOps.notices[:index], w.staffOps.notices[index+1:]...)
		return
	}
	due.nextAt = due.nextAt.Add(due.every)
	if !due.nextAt.After(now) {
		due.nextAt = now.Add(due.every)
	}
}

// tickShutdownCountdown reports whether it broadcast this tick.
func (w *World) tickShutdownCountdown(now time.Time) bool {
	plan := w.staffOps.shutdown
	if plan == nil || plan.requested {
		return false
	}
	left := plan.at.Sub(now)
	if left <= 0 {
		plan.requested = true
		w.staffBroadcast("The server is shutting down for maintenance now.")
		log.Printf("[web-admin] scheduled %s started actor=%q reason=%q", plan.mode, plan.actor, plan.reason)
		// The hook calls World.Shutdown, which queues a command and waits:
		// it must never run on the World goroutine.
		go w.staffOps.request(plan.mode)
		return true
	}
	// Announce once, at the smallest mark crossed since the last one.
	var crossed time.Duration
	for _, mark := range shutdownMarks {
		if left <= mark && mark < plan.lastMark {
			crossed = mark
		}
	}
	if crossed == 0 {
		return false
	}
	plan.lastMark = crossed
	w.staffBroadcast(shutdownNotice(plan.mode, left))
	return true
}
