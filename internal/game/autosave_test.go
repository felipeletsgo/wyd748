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
	if st.saves != 1 || st.acc == nil || st.acc == acc {
		t.Fatalf("saves=%d snapshot=%p live=%p", st.saves, st.acc, acc)
	}
	for i := range st.acc.Chars {
		if st.acc.Chars[i].X != playerEntryX || st.acc.Chars[i].Y != playerEntryY {
			t.Fatalf("char[%d] salvo em (%d,%d), quer (%d,%d)",
				i, st.acc.Chars[i].X, st.acc.Chars[i].Y, playerEntryX, playerEntryY)
		}
	}
	if acc.Chars[0].X != 1 || acc.Chars[0].Y != 1 ||
		acc.Chars[1].X != 3000 || acc.Chars[1].Y != 3000 {
		t.Fatalf("autosave alterou a conta viva: %+v", acc.Chars)
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

func TestKillRewardBatchSavesAccountOnce(t *testing.T) {
	st := &autoSaveStore{}
	session := &netpkg.Session{ID: 9}
	acc := &model.Account{Name: "felipe"}
	p := &Player{Session: session, Account: acc}
	w := &World{store: st}
	w.commitKillRewardBatch(p, []*killRewardPlan{{}}, []*model.Account{acc}, "test batch")
	if st.saves != 1 {
		t.Fatalf("lote gerou %d saves, quer 1", st.saves)
	}
	w.commitKillRewardBatch(p, nil, []*model.Account{acc}, "empty batch")
	if st.saves != 1 {
		t.Fatalf("lote sem mortes gerou save adicional: %d", st.saves)
	}
}

func TestKillRewardBatchPersistsAllPartyAccountsInOneTransaction(t *testing.T) {
	first, _ := networkedTestPlayer(1, "First", 2100, 2100)
	second, _ := networkedTestPlayer(2, "Second", 2101, 2100)
	store := &batchGameStore{}
	w := worldWithNetworkedPlayers(first, second)
	w.store = store
	w.commitKillRewardBatch(first, []*killRewardPlan{{}},
		[]*model.Account{first.Account, second.Account}, "party batch")
	if store.batchSaves != 1 {
		t.Fatalf("lote deveria usar uma transacao multi-account: saves=%d", store.batchSaves)
	}
}

func TestKillAccountsAlwaysIncludeLootOwnerAtLevelCap(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Capped", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Eligible", 2101, 2100)
	accounts := uniqueKillAccounts(killer, []partyExpShare{{player: member, reward: 100}})
	if len(accounts) != 2 || accounts[0] != killer.Account || accounts[1] != member.Account {
		t.Fatalf("contas da morte=%v, killer no cap precisa participar da transacao", accounts)
	}
}

func TestKillRewardPersistenceFailurePoisonsEveryAffectedAccount(t *testing.T) {
	first, _ := networkedTestPlayer(1, "First", 2100, 2100)
	second, _ := networkedTestPlayer(2, "Second", 2101, 2100)
	st := &batchGameStore{}
	st.err = errors.New("database unavailable")
	w := worldWithNetworkedPlayers(first, second)
	w.store = st

	w.commitKillRewardBatch(first, []*killRewardPlan{{}},
		[]*model.Account{first.Account, second.Account}, "party batch")

	for _, p := range []*Player{first, second} {
		if !p.PersistencePoisoned || !p.Session.IsClosed() {
			t.Fatalf("conta %q permaneceu gravavel apos falha: poisoned=%v closed=%v",
				p.Account.Name, p.PersistencePoisoned, p.Session.IsClosed())
		}
	}
}
