package webadmin

import (
	"crypto/sha256"
	"encoding/json"
	"errors"
	"io"
	"os"
	"strings"

	"wydgo/internal/control"
)

const StatusCapability = "server.status"
const PlayersCapability = "moderation.player.search"

type Staff struct {
	Capabilities []string `json:"capabilities"`
}

func (s Staff) permits(capability string) bool {
	for _, c := range s.Capabilities {
		if c == capability {
			return true
		}
	}
	return false
}

// StaffPermits re-reads the staff file and reports whether the canonical
// account holds the capability. A missing or invalid file grants nothing.
func StaffPermits(path, account, capability string) bool {
	entries, err := ReadStaff(path)
	if err != nil {
		return false
	}
	staff, ok := entries[account]
	return ok && staff.permits(capability)
}

func (s Staff) fingerprint() [32]byte { b, _ := json.Marshal(s); return sha256.Sum256(b) }

// ReadStaff is deliberately re-read for each request: revocation fails closed.
// Provision out of band with owner-only file permissions; no enrollment HTTP API.
func ReadStaff(path string) (map[string]Staff, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, errors.New("staff file unavailable")
	}
	defer f.Close()
	info, err := f.Stat()
	if err != nil || !info.Mode().IsRegular() || info.Size() > 32768 {
		return nil, errors.New("invalid staff file")
	}
	decoder := json.NewDecoder(io.LimitReader(f, 32769))
	decoder.DisallowUnknownFields()
	var entries map[string]Staff
	if decoder.Decode(&entries) != nil {
		return nil, errors.New("invalid staff JSON")
	}
	var extra any
	if decoder.Decode(&extra) != io.EOF {
		return nil, errors.New("trailing staff JSON")
	}
	if len(entries) == 0 || len(entries) > 128 {
		return nil, errors.New("expected 1..128 staff entries")
	}
	for name, staff := range entries {
		if name != strings.ToLower(name) || len(name) < 1 || len(name) > 12 || strings.IndexFunc(name, func(r rune) bool { return !(r >= 'a' && r <= 'z' || r >= '0' && r <= '9') }) >= 0 {
			return nil, errors.New("invalid canonical staff account")
		}
		if len(staff.Capabilities) == 0 || len(staff.Capabilities) > 32 {
			return nil, errors.New("invalid capabilities")
		}
		for _, cap := range staff.Capabilities {
			if cap != StatusCapability && cap != PlayersCapability && cap != control.GlobalDropCapability && cap != control.QuizCapability && cap != control.BossesCapability && cap != control.KickCapability && cap != control.TeleportCapability &&
				cap != control.PremiumCatalogCapability && cap != control.DonateCreditCapability && cap != control.GotoCapability &&
				cap != control.NPCDefinitionCapability &&
				cap != control.PvPDoorCapability && cap != control.OXEventCapability && cap != control.ColosseumCapability &&
				cap != control.BanCapability && cap != control.MuteCapability && cap != control.NoticeCapability &&
				cap != control.ShutdownCapability && cap != control.AuditReadCapability &&
				cap != control.AccountIPCapability && cap != control.LogsCapability {
				return nil, errors.New("unsupported capability")
			}
		}
	}
	return entries, nil
}
