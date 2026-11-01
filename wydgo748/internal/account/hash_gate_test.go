package account

import (
	"errors"
	"fmt"
	"os"
	"testing"

	"wydgo/internal/model"
)

// countingGate records whether a slot is held and can refuse admission.
type countingGate struct {
	held, acquired int
	refuse         bool
}

func (g *countingGate) Acquire() bool {
	if g.refuse {
		return false
	}
	g.held++
	g.acquired++
	return true
}

func (g *countingGate) Release() { g.held-- }

type gateObservingStore struct {
	acc       *model.Account
	gate      *countingGate
	heldReads int
}

func (s *gateObservingStore) LoadAccount(name string) (*model.Account, error) {
	s.heldReads += s.gate.held
	if s.acc == nil {
		return nil, fmt.Errorf("%w: %s", os.ErrNotExist, name)
	}
	copy := *s.acc
	return &copy, nil
}

// W2/D9: the hash slot covers PBKDF2 only, never the storage read.
func TestAuthenticateGatedHoldsSlotOnlyForHashing(t *testing.T) {
	hash, err := HashPassword("felipe")
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		name     string
		acc      *model.Account
		password string
		want     error
	}{
		{"valid", &model.Account{Name: "felipe", PasswordHash: hash}, "felipe", nil},
		{"wrong-password", &model.Account{Name: "felipe", PasswordHash: hash}, "wrong", ErrInvalidCredentials},
		{"missing-account", nil, "felipe", ErrInvalidCredentials},
	} {
		t.Run(tc.name, func(t *testing.T) {
			gate := &countingGate{}
			st := &gateObservingStore{acc: tc.acc, gate: gate}
			_, err := AuthenticateGated(st, "felipe", tc.password, gate)
			if !errors.Is(err, tc.want) && !(tc.want == nil && err == nil) {
				t.Fatalf("err = %v, want %v", err, tc.want)
			}
			if gate.acquired != 1 || gate.held != 0 || st.heldReads != 0 {
				t.Fatalf("acquired=%d held=%d reads under slot=%d", gate.acquired, gate.held, st.heldReads)
			}
		})
	}
	refused := &countingGate{refuse: true}
	st := &gateObservingStore{acc: &model.Account{Name: "felipe", PasswordHash: hash}, gate: refused}
	if _, err := AuthenticateGated(st, "felipe", "felipe", refused); !errors.Is(err, ErrHashBusy) || errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("busy hash capacity reported %v", err)
	}
}
