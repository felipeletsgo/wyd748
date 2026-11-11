package control

import (
	"strings"
	"testing"
)

func requireValid(t *testing.T, err error) {
	t.Helper()
	if err != nil {
		t.Fatalf("valid input rejected: %v", err)
	}
}

func requireInvalid(t *testing.T, err error) {
	t.Helper()
	if err == nil {
		t.Fatal("invalid input accepted")
	}
}

func TestSharedStaffTextAndIdentityValidators(t *testing.T) {
	for _, id := range []string{"operation-000001", strings.Repeat("A", 64)} {
		if !validOperationID(id) {
			t.Fatalf("valid operation ID rejected: %q", id)
		}
	}
	for _, id := range []string{"short", strings.Repeat("a", 65), "operation_000001"} {
		if validOperationID(id) {
			t.Fatalf("invalid operation ID accepted: %q", id)
		}
	}
	for _, account := range []string{"a", "account123", strings.Repeat("z", 12)} {
		if !ValidAccountKey(account) {
			t.Fatalf("valid account rejected: %q", account)
		}
	}
	for _, account := range []string{"", "UPPER", "bad-name", strings.Repeat("a", 13)} {
		if ValidAccountKey(account) {
			t.Fatalf("invalid account accepted: %q", account)
		}
	}
	for _, message := range []string{"hello", strings.Repeat("x", MaxNoticeBytes)} {
		if !ValidNoticeText(message) {
			t.Fatalf("valid notice rejected: %q", message)
		}
	}
	for _, message := range []string{" padded", "trailing ", "line\nbreak", "á", strings.Repeat("x", MaxNoticeBytes+1)} {
		if ValidNoticeText(message) {
			t.Fatalf("invalid notice accepted: %q", message)
		}
	}
}

func TestAccountNoteCommandValidation(t *testing.T) {
	valid := AccountNoteCommand{Version: 1, Body: "First line\nsecond line", BugRefs: []string{"BUG-123", "BUG-1234"}}
	requireValid(t, valid.Validate())
	for _, command := range []AccountNoteCommand{
		{Version: 2, Body: "valid note"},
		{Version: 1, Body: "  "},
		{Version: 1, Body: "line\tbreak"},
		{Version: 1, Body: strings.Repeat("x", 2001)},
		{Version: 1, Body: "valid note", BugRefs: []string{"BUG-12"}},
		{Version: 1, Body: "valid note", BugRefs: []string{"BUG-123", "BUG-123"}},
		{Version: 1, Body: "valid note", BugRefs: []string{"BUG-ABC"}},
		{Version: 1, Body: "valid note", BugRefs: make([]string, 11)},
	} {
		requireInvalid(t, command.Validate())
	}
}

func TestBossDonateAndRecoveryCommandValidation(t *testing.T) {
	boss := BossesCommand{Version: 1, OperationID: "operation-000001", Epoch: strings.Repeat("a", 32),
		BossID: "kefra", ExpectedRevision: 1, Action: "summon", Reason: "event support"}
	requireValid(t, boss.Validate())
	boss.ExpectedRevision = 0
	requireInvalid(t, boss.Validate())

	donate := DonateCreditCommand{Version: 1, RequestID: "request-0000000001", Account: "account1", Amount: 1, Reason: "manual grant"}
	requireValid(t, donate.Validate())
	for _, mutate := range []func(*DonateCreditCommand){
		func(c *DonateCreditCommand) { c.RequestID = "short" },
		func(c *DonateCreditCommand) { c.Account = "Bad" },
		func(c *DonateCreditCommand) { c.Amount = 0 },
		func(c *DonateCreditCommand) { c.Amount = MaxDonateAmount + 1 },
		func(c *DonateCreditCommand) { c.Reason = "x" },
	} {
		bad := donate
		mutate(&bad)
		requireInvalid(t, bad.Validate())
	}

	recheck := PersistenceRecheckCommand{Version: 1, OperationID: "operation-000001", Reason: "check commit"}
	requireValid(t, recheck.Validate())
	recheck.Reason = "check\ncommit"
	requireInvalid(t, recheck.Validate())
}

func TestPlayerVersusPlayerAndModerationCommandValidation(t *testing.T) {
	for _, action := range []string{"lock", "open"} {
		requireValid(t, PvPDoorCommand{Version: 1, OperationID: "operation-000001", Action: action, Reason: "event control"}.Validate())
	}
	requireInvalid(t, PvPDoorCommand{Version: 1, OperationID: "operation-000001", Action: "toggle", Reason: "event control"}.Validate())

	for _, kind := range []string{SanctionBan, SanctionMute} {
		requireValid(t, SanctionCommand{Version: 1, RequestID: "request-0000000001", Account: "account1", Kind: kind, Reason: "policy breach"}.Validate())
		requireValid(t, SanctionRevokeCommand{Version: 1, Account: "account1", Kind: kind, Reason: "appeal accepted"}.Validate())
	}
	if capability, ok := SanctionCapability(SanctionBan); !ok || capability != BanCapability {
		t.Fatalf("ban capability=%q ok=%v", capability, ok)
	}
	if capability, ok := SanctionCapability(SanctionMute); !ok || capability != MuteCapability {
		t.Fatalf("mute capability=%q ok=%v", capability, ok)
	}
	if capability, ok := SanctionCapability("unknown"); ok || capability != "" {
		t.Fatalf("unknown sanction capability=%q ok=%v", capability, ok)
	}
	requireInvalid(t, SanctionCommand{Version: 1, RequestID: "request-0000000001", Account: "account1", Kind: SanctionBan, DurationMinutes: MaxSanctionMinutes + 1, Reason: "policy breach"}.Validate())
	requireInvalid(t, SanctionRevokeCommand{Version: 1, Account: "account1", Kind: "unknown", Reason: "appeal accepted"}.Validate())
}

func TestNoticePlayerMessageAndShutdownValidation(t *testing.T) {
	notice := NoticeCommand{Version: 1, OperationID: "operation-000001", Message: "Server event", RepeatCount: 1}
	requireValid(t, notice.Validate())
	notice.RepeatCount, notice.RepeatEverySeconds = 2, 60
	requireValid(t, notice.Validate())
	for _, mutate := range []func(*NoticeCommand){
		func(c *NoticeCommand) { c.RepeatCount = 0 },
		func(c *NoticeCommand) { c.RepeatCount = 1; c.RepeatEverySeconds = 60 },
		func(c *NoticeCommand) { c.RepeatCount = 2; c.RepeatEverySeconds = 59 },
		func(c *NoticeCommand) { c.DelaySeconds = 86401 },
		func(c *NoticeCommand) { c.Message = "line\nbreak" },
	} {
		bad := notice
		mutate(&bad)
		requireInvalid(t, bad.Validate())
	}
	requireValid(t, NoticeCancelCommand{Version: 1, OperationID: "operation-000001"}.Validate())
	requireInvalid(t, NoticeCancelCommand{Version: 1, OperationID: "bad"}.Validate())
	requireValid(t, PlayerMessageCommand{Version: 1, OperationID: "operation-000001", Epoch: strings.Repeat("0", 32),
		UID: "character-uid", Account: "account1", ExpectedSessionID: 12, Message: "Please wait"}.Validate())
	requireInvalid(t, PlayerMessageCommand{Version: 1, OperationID: "operation-000001", Epoch: strings.Repeat("0", 32),
		UID: "character-uid", Account: "account1", ExpectedSessionID: 12, Message: " bad"}.Validate())

	for _, mode := range []string{ShutdownModeShutdown, ShutdownModeRestart} {
		requireValid(t, ShutdownCommand{Version: 1, OperationID: "operation-000001", Action: "schedule", Mode: mode,
			DelaySeconds: 60, Reason: "scheduled maintenance"}.Validate())
	}
	requireValid(t, ShutdownCommand{Version: 1, OperationID: "operation-000001", Action: "cancel", Reason: "maintenance canceled"}.Validate())
	requireInvalid(t, ShutdownCommand{Version: 1, OperationID: "operation-000001", Action: "schedule", Mode: "shutdown",
		DelaySeconds: 59, Reason: "scheduled maintenance"}.Validate())
	requireInvalid(t, ShutdownCommand{Version: 1, OperationID: "operation-000001", Action: "cancel", Mode: "restart",
		Reason: "maintenance canceled"}.Validate())
}
