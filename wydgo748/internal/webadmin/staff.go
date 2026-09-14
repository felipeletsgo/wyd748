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
		if len(staff.Capabilities) == 0 || len(staff.Capabilities) > 5 {
			return nil, errors.New("invalid capabilities")
		}
		for _, cap := range staff.Capabilities {
			if cap != StatusCapability && cap != PlayersCapability && cap != control.GlobalDropCapability && cap != control.QuizCapability && cap != control.BossesCapability {
				return nil, errors.New("unsupported capability")
			}
		}
	}
	return entries, nil
}
