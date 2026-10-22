package control

import (
	"context"
	"errors"
	"strings"
	"time"
)

// Staff operations contract (DOCS/server/staff-operations.md):
// account sanctions, staff notices, scheduled shutdown and the audit history.
const (
	BanCapability       = "moderation.account.ban"
	MuteCapability      = "moderation.account.mute"
	NoticeCapability    = "server.notice"
	ShutdownCapability  = "server.shutdown"
	AuditReadCapability = "staff.audit.read"
)

const (
	SanctionBan  = "ban"
	SanctionMute = "mute"
	// MaxSanctionMinutes is one year; zero means permanent.
	MaxSanctionMinutes = 525600
	// MaxNoticeBytes fits the 95-byte text field of the 0x101 panel and
	// 0x334 whisper packets with the terminating zero.
	MaxNoticeBytes = 94
)

var (
	ErrSanctionsUnavailable = errors.New("control: sanctions unavailable")
	ErrSanctionConflict     = errors.New("control: sanction request conflict")
	ErrUnknownSanction      = errors.New("control: unknown sanction")
	ErrAuditUnavailable     = errors.New("control: audit unavailable")
)

func validOperationID(id string) bool {
	return len(id) >= 16 && len(id) <= 64 && strings.IndexFunc(id, func(r rune) bool {
		return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
	}) < 0
}

// ValidAccountKey accepts the canonical lowercase account name.
func ValidAccountKey(account string) bool {
	return len(account) >= 1 && len(account) <= 12 && strings.IndexFunc(account, func(r rune) bool {
		return !(r >= 'a' && r <= 'z' || r >= '0' && r <= '9')
	}) < 0
}

// ValidNoticeText accepts printable ASCII only: the 7.48 client renders these
// packets with its legacy code page, and a multibyte sequence could be cut.
func ValidNoticeText(message string) bool {
	trimmed := strings.TrimSpace(message)
	return trimmed == message && len(message) >= 1 && len(message) <= MaxNoticeBytes &&
		strings.IndexFunc(message, func(r rune) bool { return r < 0x20 || r > 0x7e }) < 0
}

// SanctionCommand records a ban or chat mute. RequestID makes a retry
// idempotent; DurationMinutes zero means permanent.
type SanctionCommand struct {
	Version         int    `json:"version"`
	RequestID       string `json:"requestId"`
	Account         string `json:"account"`
	Kind            string `json:"kind"`
	DurationMinutes int    `json:"durationMinutes"`
	Reason          string `json:"reason"`
}

func (c SanctionCommand) Validate() error {
	if c.Version != 1 || !validOperationID(c.RequestID) || !ValidAccountKey(c.Account) ||
		(c.Kind != SanctionBan && c.Kind != SanctionMute) ||
		c.DurationMinutes < 0 || c.DurationMinutes > MaxSanctionMinutes || !validReason(c.Reason) {
		return errors.New("invalid_command")
	}
	return nil
}

// SanctionRevokeCommand lifts one sanction before it expires. Kind selects
// the capability and must match the stored sanction.
type SanctionRevokeCommand struct {
	Version int    `json:"version"`
	Account string `json:"account"`
	Kind    string `json:"kind"`
	Reason  string `json:"reason"`
}

func (c SanctionRevokeCommand) Validate() error {
	if c.Version != 1 || !ValidAccountKey(c.Account) || (c.Kind != SanctionBan && c.Kind != SanctionMute) || !validReason(c.Reason) {
		return errors.New("invalid_command")
	}
	return nil
}

// SanctionCapability maps a sanction kind to the capability that controls it.
func SanctionCapability(kind string) (string, bool) {
	switch kind {
	case SanctionBan:
		return BanCapability, true
	case SanctionMute:
		return MuteCapability, true
	}
	return "", false
}

type Sanction struct {
	ID              int64      `json:"id"`
	Account         string     `json:"account"`
	Kind            string     `json:"kind"`
	Reason          string     `json:"reason"`
	Actor           string     `json:"actor"`
	DurationMinutes int        `json:"durationMinutes"`
	CreatedAt       time.Time  `json:"createdAt"`
	ExpiresAt       *time.Time `json:"expiresAt"`
	RevokedAt       *time.Time `json:"revokedAt"`
	RevokedBy       string     `json:"revokedBy"`
	RevokeReason    string     `json:"revokeReason"`
	Active          bool       `json:"active"`
}

type SanctionsView struct {
	Version   int        `json:"version"`
	AsOf      time.Time  `json:"asOf"`
	Account   string     `json:"account"`
	Sanctions []Sanction `json:"sanctions"`
}

// SanctionResult reports the durable record and its live effect. Live is
// "applied" when online sessions were updated, "offline" when none were
// connected, and "pending" when the World did not confirm; the durable record
// still governs the next login in that case.
type SanctionResult struct {
	Code     string   `json:"code"`
	Replayed bool     `json:"replayed"`
	Live     string   `json:"live"`
	Sanction Sanction `json:"sanction"`
}

type SanctionSource interface {
	AccountSanctions(context.Context, string) (SanctionsView, error)
	ApplySanction(context.Context, string, SanctionCommand, func() bool) (SanctionResult, error)
	RevokeSanction(context.Context, string, int64, SanctionRevokeCommand, func() bool) (SanctionResult, error)
}

// NoticeCommand broadcasts a staff notice now or later, optionally repeated.
type NoticeCommand struct {
	Version            int    `json:"version"`
	OperationID        string `json:"operationId"`
	Message            string `json:"message"`
	DelaySeconds       int    `json:"delaySeconds"`
	RepeatCount        int    `json:"repeatCount"`
	RepeatEverySeconds int    `json:"repeatEverySeconds"`
}

func (c NoticeCommand) Validate() error {
	if c.Version != 1 || !validOperationID(c.OperationID) || !ValidNoticeText(c.Message) ||
		c.DelaySeconds < 0 || c.DelaySeconds > 86400 || c.RepeatCount < 1 || c.RepeatCount > 100 ||
		(c.RepeatCount == 1 && c.RepeatEverySeconds != 0) ||
		(c.RepeatCount > 1 && (c.RepeatEverySeconds < 60 || c.RepeatEverySeconds > 86400)) {
		return errors.New("invalid_command")
	}
	return nil
}

type NoticeCancelCommand struct {
	Version     int    `json:"version"`
	OperationID string `json:"operationId"`
}

func (c NoticeCancelCommand) Validate() error {
	if c.Version != 1 || !validOperationID(c.OperationID) {
		return errors.New("invalid_command")
	}
	return nil
}

type ScheduledNotice struct {
	ID                 int64     `json:"id"`
	Message            string    `json:"message"`
	Actor              string    `json:"actor"`
	NextAt             time.Time `json:"nextAt"`
	Remaining          int       `json:"remaining"`
	RepeatEverySeconds int       `json:"repeatEverySeconds"`
}

type NoticesView struct {
	Version int               `json:"version"`
	AsOf    time.Time         `json:"asOf"`
	Notices []ScheduledNotice `json:"notices"`
}

type NoticeResult struct {
	OperationID string      `json:"operationId"`
	Code        string      `json:"code"`
	Replayed    bool        `json:"replayed"`
	Delivered   int         `json:"delivered"`
	View        NoticesView `json:"view"`
}

// PlayerMessageCommand sends a staff message to one observed session. It uses
// the same epoch/session envelope as KickCommand.
type PlayerMessageCommand struct {
	Version           int    `json:"version"`
	OperationID       string `json:"operationId"`
	Epoch             string `json:"epoch"`
	UID               string `json:"uid"`
	Account           string `json:"account"`
	ExpectedSessionID int64  `json:"expectedSessionId"`
	Message           string `json:"message"`
}

func (c PlayerMessageCommand) Validate() error {
	envelope := KickCommand{Version: c.Version, OperationID: c.OperationID, Epoch: c.Epoch, UID: c.UID,
		Account: c.Account, ExpectedSessionID: c.ExpectedSessionID, Reason: "message"}
	if envelope.Validate() != nil || !ValidNoticeText(c.Message) {
		return errors.New("invalid_command")
	}
	return nil
}

type PlayerMessageResult struct {
	OperationID string `json:"operationId"`
	Code        string `json:"code"`
	Replayed    bool   `json:"replayed"`
}

type NoticeSource interface {
	Notices(context.Context) (NoticesView, error)
	ScheduleNotice(context.Context, string, NoticeCommand, func() bool) (NoticeResult, error)
	CancelNotice(context.Context, string, int64, NoticeCancelCommand, func() bool) (NoticeResult, error)
	MessagePlayer(context.Context, string, PlayerMessageCommand, func() bool) (PlayerMessageResult, error)
}

const (
	ShutdownModeShutdown = "shutdown"
	ShutdownModeRestart  = "restart"
	// Logins are refused during the final minute of a scheduled shutdown.
	ShutdownLoginCutoff = time.Minute
)

// ShutdownCommand schedules or cancels a maintenance shutdown.
type ShutdownCommand struct {
	Version      int    `json:"version"`
	OperationID  string `json:"operationId"`
	Action       string `json:"action"`
	Mode         string `json:"mode"`
	DelaySeconds int    `json:"delaySeconds"`
	Reason       string `json:"reason"`
}

func (c ShutdownCommand) Validate() error {
	if c.Version != 1 || !validOperationID(c.OperationID) || !validReason(c.Reason) {
		return errors.New("invalid_command")
	}
	switch c.Action {
	case "schedule":
		if (c.Mode != ShutdownModeShutdown && c.Mode != ShutdownModeRestart) || c.DelaySeconds < 60 || c.DelaySeconds > 86400 {
			return errors.New("invalid_command")
		}
	case "cancel":
		if c.Mode != "" || c.DelaySeconds != 0 {
			return errors.New("invalid_command")
		}
	default:
		return errors.New("invalid_command")
	}
	return nil
}

type ShutdownStatus struct {
	Version          int        `json:"version"`
	AsOf             time.Time  `json:"asOf"`
	Scheduled        bool       `json:"scheduled"`
	Mode             string     `json:"mode"`
	At               *time.Time `json:"at"`
	SecondsRemaining int64      `json:"secondsRemaining"`
	Actor            string     `json:"actor"`
	Reason           string     `json:"reason"`
	LoginsBlocked    bool       `json:"loginsBlocked"`
	Available        bool       `json:"available"`
}

type ShutdownResult struct {
	OperationID string         `json:"operationId"`
	Code        string         `json:"code"`
	Replayed    bool           `json:"replayed"`
	Status      ShutdownStatus `json:"status"`
}

type ShutdownSource interface {
	ShutdownStatus(context.Context) (ShutdownStatus, error)
	ShutdownCommand(context.Context, string, ShutdownCommand, func() bool) (ShutdownResult, error)
}

// AuditEntry is one persisted staff panel event. Details hold the action
// attributes; credentials are never passed to the audit call.
type AuditEntry struct {
	ID        int64          `json:"id"`
	At        time.Time      `json:"at"`
	Actor     string         `json:"actor"`
	Action    string         `json:"action"`
	Result    string         `json:"result"`
	RequestID string         `json:"requestId"`
	ClientIP  string         `json:"clientIp"`
	Method    string         `json:"method"`
	Path      string         `json:"path"`
	Details   map[string]any `json:"details"`
}

type AuditQuery struct {
	Actor  string
	Action string
	Before int64
	Limit  int
}

type AuditPage struct {
	Version    int          `json:"version"`
	AsOf       time.Time    `json:"asOf"`
	Entries    []AuditEntry `json:"entries"`
	NextBefore int64        `json:"nextBefore"`
}

type AuditStore interface {
	AppendStaffAudit(context.Context, AuditEntry) error
	ListStaffAudit(context.Context, AuditQuery) (AuditPage, error)
	// PruneStaffAudit deletes up to limit rows older than before, oldest first.
	PruneStaffAudit(context.Context, time.Time, int) (int64, error)
}
