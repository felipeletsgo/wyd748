package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	netpkg "wydgo/internal/net"
)

type autoSaveStore struct {
	saves int
	acc   *model.Account
	err   error
}

func (s *autoSaveStore) LoadAccount(string) (*model.Account, error) {
	return nil, errors.New("nao usado")
}

func (s *autoSaveStore) SaveAccount(acc *model.Account) error {
	s.saves++
	s.acc = acc
	return s.err
}

func (s *autoSaveStore) CharacterNameExists(string) (bool, error) { return false, nil }

func TestAutoSavePinsEveryCharacterAtFixedEntryPosition(t *testing.T) {
	st := &autoSaveStore{}
	acc := &model.Account{Name: "felipe", Chars: []model.Char{
		{Name: "felipe", X: 1, Y: 1},
		{Name: "outro", X: 3000, Y: 3000},
	}}
	session := &netpkg.Session{ID: 7}
	p := &Player{Session: session, Account: acc, Char: &acc.Chars[0], InWorld: true, X: 2200, Y: 2100}
	w := &World{store: st, players: map[*netpkg.Session]*Player{session: p}}
	now := time.Now()
	w.autoSaveAccounts(now)
	if st.saves != 1 || st.acc != acc {
		t.Fatalf("saves=%d conta=%p, quer 1/%p", st.saves, st.acc, acc)
	}
	for i := range acc.Chars {
		if acc.Chars[i].X != playerEntryX || acc.Chars[i].Y != playerEntryY {
			t.Fatalf("char[%d] salvo em (%d,%d), quer (%d,%d)",
				i, acc.Chars[i].X, acc.Chars[i].Y, playerEntryX, playerEntryY)
		}
	}
	if !w.nextAutoSave.Equal(now.Add(accountAutoSaveInterval)) {
		t.Fatalf("proximo autosave=%v", w.nextAutoSave)
	}
}

func TestAutoSaveIgnoresPlayerOutsideWorld(t *testing.T) {
	st := &autoSaveStore{}
	session := &netpkg.Session{ID: 8}
	w := &World{store: st, players: map[*netpkg.Session]*Player{
		session: {Session: session, Account: &model.Account{Name: "x"}, Char: &model.Char{}},
	}}
	w.autoSaveAccounts(time.Now())
	if st.saves != 0 {
		t.Fatalf("salvou %d conta(s) fora do mundo", st.saves)
	}
}

func TestMultiKillBatchSavesAccountOnce(t *testing.T) {
	st := &autoSaveStore{}
	session := &netpkg.Session{ID: 9}
	acc := &model.Account{Name: "felipe"}
	p := &Player{Session: session, Account: acc}
	w := &World{store: st}
	w.saveMultiKillBatch(p, 8)
	if st.saves != 1 {
		t.Fatalf("lote de 8 mortes gerou %d saves, quer 1", st.saves)
	}
	w.saveMultiKillBatch(p, 0)
	if st.saves != 1 {
		t.Fatalf("lote sem mortes gerou save adicional: %d", st.saves)
	}
}
