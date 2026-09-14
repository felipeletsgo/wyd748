package control

import (
	"context"
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

const BossesCapability = "game.boss.summon"

// Administrative contract only; the existing game spawn wire is unchanged.
type BossesCommand struct {
	Version          int    `json:"version"`
	OperationID      string `json:"operationId"`
	Epoch            string `json:"epoch"`
	BossID           string `json:"bossId"`
	ExpectedRevision uint64 `json:"expectedRevision"`
	Action           string `json:"action"`
	Reason           string `json:"reason"`
}

func (c BossesCommand) Validate() error {
	if c.Version != 1 || len(c.Epoch) != 32 || len(c.OperationID) < 16 || len(c.OperationID) > 64 ||
		c.Action != "summon" || strings.TrimSpace(c.BossID) == "" || len(c.BossID) > 128 ||
		c.ExpectedRevision == 0 || c.ExpectedRevision > 9007199254740991 ||
		!utf8.ValidString(c.Reason) || utf8.RuneCountInString(strings.TrimSpace(c.Reason)) < 3 || utf8.RuneCountInString(c.Reason) > 200 ||
		strings.IndexFunc(c.Reason+c.BossID, unicode.IsControl) >= 0 ||
		strings.IndexFunc(c.OperationID, func(r rune) bool {
			return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
		}) >= 0 {
		return errors.New("invalid_command")
	}
	return nil
}

type BossStatus struct {
	ID        string     `json:"id"`
	Name      string     `json:"name"`
	Revision  uint64     `json:"revision"`
	Alive     bool       `json:"alive"`
	MobID     uint16     `json:"mobId"`
	HP        uint32     `json:"hp"`
	MaxHP     uint32     `json:"maxHp"`
	X         uint16     `json:"x"`
	Y         uint16     `json:"y"`
	SpawnX    uint16     `json:"spawnX"`
	SpawnY    uint16     `json:"spawnY"`
	RespawnAt *time.Time `json:"respawnAt"`
}
type BossesStatus struct {
	Version int          `json:"version"`
	Epoch   string       `json:"epoch"`
	AsOf    time.Time    `json:"asOf"`
	Bosses  []BossStatus `json:"bosses"`
}
type BossesResult struct {
	OperationID string       `json:"operationId"`
	Code        string       `json:"code"`
	Replayed    bool         `json:"replayed"`
	Status      BossesStatus `json:"status"`
}
type BossesSource interface {
	BossesStatus(context.Context) (BossesStatus, error)
	BossesCommand(context.Context, string, BossesCommand, func() bool) (BossesResult, error)
}
