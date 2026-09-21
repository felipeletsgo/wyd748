package webadmin

import (
	"context"
	"errors"
	"os"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

var (
	ErrInvalidPersistentTarget  = errors.New("webadmin: invalid persistent player target")
	ErrPersistentPlayerNotFound = errors.New("webadmin: persistent player not found")
	ErrInvalidAccountDirectory  = errors.New("webadmin: invalid account directory query")
)

// PersistentPlayer is an immutable projection of the character state read from
// PostgreSQL. It deliberately excludes the account password hash and raw
// inventory/cargo payloads.
type PersistentPlayer struct {
	Version       int       `json:"version"`
	AsOf          time.Time `json:"asOf"`
	Account       string    `json:"account"`
	UID           string    `json:"uid"`
	Name          string    `json:"name"`
	Class         byte      `json:"class"`
	Level         uint32    `json:"level"`
	Evolution     string    `json:"evolution"`
	HP            uint32    `json:"hp"`
	MaxHP         uint32    `json:"maxHp"`
	MP            uint32    `json:"mp"`
	MaxMP         uint32    `json:"maxMp"`
	Attack        uint32    `json:"attack"`
	MagicAttack   uint32    `json:"magicAttack"`
	Defense       uint32    `json:"defense"`
	Str           uint32    `json:"str"`
	Int           uint32    `json:"int"`
	Dex           uint32    `json:"dex"`
	Con           uint32    `json:"con"`
	StatusPoints  uint32    `json:"statusPoints"`
	MasteryPoints uint32    `json:"masteryPoints"`
	SkillPoints   uint32    `json:"skillPoints"`
	X             uint16    `json:"x"`
	Y             uint16    `json:"y"`
	Gold          uint32    `json:"gold"`
	Exp           uint32    `json:"exp"`
	Hold          uint32    `json:"hold"`
	CP            int16     `json:"cp"`
}

type AccountDirectory struct {
	Version           int              `json:"version"`
	AsOf              time.Time        `json:"asOf"`
	PresenceAvailable bool             `json:"presenceAvailable"`
	Accounts          []AccountSummary `json:"accounts"`
	NextCursor        string           `json:"nextCursor"`
}

type AccountSummary struct {
	Username   string                    `json:"username"`
	CreatedAt  time.Time                 `json:"createdAt"`
	UpdatedAt  time.Time                 `json:"updatedAt"`
	Online     bool                      `json:"online"`
	Characters []AccountCharacterSummary `json:"characters"`
}

type AccountCharacterSummary struct {
	UID       string `json:"uid"`
	Slot      int    `json:"slot"`
	Name      string `json:"name"`
	Class     byte   `json:"class"`
	Level     uint32 `json:"level"`
	Evolution string `json:"evolution"`
}

type PersistentReader interface {
	Player(context.Context, string, string) (PersistentPlayer, error)
	Accounts(context.Context, string, string, int) (AccountDirectory, error)
}

type persistentStore interface {
	ReadPersistentPlayer(context.Context, string, string) (store.PersistentPlayerSnapshot, error)
	ReadAccountDirectory(context.Context, string, string, int) (store.AccountDirectoryPageSnapshot, error)
}

type accountPersistentReader struct {
	store persistentStore
	now   func() time.Time
}

// NewAccountPersistentReader creates the narrow PostgreSQL-facing projection
// used by the admin handler. The handler never receives *model.Account.
func NewAccountPersistentReader(source persistentStore) PersistentReader {
	return &accountPersistentReader{store: source, now: time.Now}
}

func (r *accountPersistentReader) Player(ctx context.Context, accountName, uid string) (PersistentPlayer, error) {
	if r == nil || r.store == nil || ctx == nil {
		return PersistentPlayer{}, ErrInvalidPersistentTarget
	}
	accountName = strings.TrimSpace(accountName)
	if accountName == "" || len(accountName) > 12 {
		return PersistentPlayer{}, ErrInvalidPersistentTarget
	}
	normalizedUID, err := model.NormalizeCharacterUID(uid)
	if err != nil || normalizedUID == "" || normalizedUID != uid {
		return PersistentPlayer{}, ErrInvalidPersistentTarget
	}
	if err := ctx.Err(); err != nil {
		return PersistentPlayer{}, err
	}
	character, err := r.store.ReadPersistentPlayer(ctx, accountName, normalizedUID)
	if err != nil {
		if os.IsNotExist(err) {
			return PersistentPlayer{}, ErrPersistentPlayerNotFound
		}
		return PersistentPlayer{}, err
	}
	if err := ctx.Err(); err != nil {
		return PersistentPlayer{}, err
	}
	if character.UID != normalizedUID || character.Name == "" {
		return PersistentPlayer{}, ErrPersistentPlayerNotFound
	}
	return PersistentPlayer{
		Version: 1, AsOf: r.now().UTC(), Account: character.Account, UID: character.UID,
		Name: character.Name, Class: character.Class, Level: character.Level,
		Evolution: character.Evolution, HP: character.HP, MaxHP: character.MaxHP,
		MP: character.MP, MaxMP: character.MaxMP, Attack: character.Attack,
		MagicAttack: character.MagicAttack, Defense: character.Defense, Str: character.Str,
		Int: character.Int, Dex: character.Dex, Con: character.Con, StatusPoints: character.StatusPoints,
		MasteryPoints: character.MasteryPoints, SkillPoints: character.SkillPoints,
		X: character.X, Y: character.Y, Gold: character.Gold, Exp: character.Exp,
		Hold: character.Hold, CP: character.CP,
	}, nil
}

func (r *accountPersistentReader) Accounts(ctx context.Context, search, cursor string, limit int) (AccountDirectory, error) {
	if r == nil || r.store == nil || ctx == nil {
		return AccountDirectory{}, ErrInvalidAccountDirectory
	}
	search = strings.ToLower(strings.TrimSpace(search))
	cursor = strings.ToLower(strings.TrimSpace(cursor))
	if !validAccountDirectoryKey(search) || !validAccountDirectoryKey(cursor) || limit < 1 || limit > 50 {
		return AccountDirectory{}, ErrInvalidAccountDirectory
	}
	if err := ctx.Err(); err != nil {
		return AccountDirectory{}, err
	}
	page, err := r.store.ReadAccountDirectory(ctx, search, cursor, limit)
	if err != nil {
		return AccountDirectory{}, err
	}
	if err := ctx.Err(); err != nil {
		return AccountDirectory{}, err
	}
	result := AccountDirectory{
		Version: 1, AsOf: r.now().UTC(), NextCursor: page.NextCursor,
		Accounts: make([]AccountSummary, 0, len(page.Accounts)),
	}
	for _, account := range page.Accounts {
		entry := AccountSummary{
			Username: account.Name, CreatedAt: account.CreatedAt, UpdatedAt: account.UpdatedAt,
			Characters: make([]AccountCharacterSummary, 0, len(account.Characters)),
		}
		for _, character := range account.Characters {
			entry.Characters = append(entry.Characters, AccountCharacterSummary{
				UID: character.UID, Slot: character.Slot, Name: character.Name,
				Class: character.Class, Level: character.Level, Evolution: character.Evolution,
			})
		}
		result.Accounts = append(result.Accounts, entry)
	}
	return result, nil
}

func validAccountDirectoryKey(value string) bool {
	if len(value) > 12 {
		return false
	}
	for i := 0; i < len(value); i++ {
		if value[i] < 'a' || value[i] > 'z' {
			if value[i] < '0' || value[i] > '9' {
				return false
			}
		}
	}
	return true
}
