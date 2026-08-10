package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

type charStateMemoryStore struct {
	craftStore
	state      *model.CharState
	loadErr    error
	saveErr    error
	loads      int
	syncSaves  int
	asyncSaves int
}

type atomicCharStateMemoryStore struct {
	charStateMemoryStore
	atomicSaves     int
	atomicErr       error
	lastUID         string
	accountSnapshot *model.Account
}

func (s *atomicCharStateMemoryStore) SavePlayerState(_ *model.GuildRegistry,
	account *model.Account, uid string, state *model.CharState) error {
	s.atomicSaves++
	s.lastUID = uid
	s.state = state
	s.accountSnapshot = account
	return s.atomicErr
}

func (s *charStateMemoryStore) LoadCharState(string) (*model.CharState, error) {
	s.loads++
	return s.state, s.loadErr
}

func (s *charStateMemoryStore) SaveCharState(_ string, state *model.CharState) error {
	s.syncSaves++
	s.state = state
	return s.saveErr
}

func (s *charStateMemoryStore) SaveCharStateAsync(_ string, state *model.CharState) error {
	s.asyncSaves++
	s.state = state
	return s.saveErr
}

// TestCharStateRoundTripsActiveBuffs cobre o objetivo do felipe: buffs (e moedas)
// sobrevivem ao relog. buildCharState descarta expirados; applyCharState restaura
// os que ainda valem num personagem novo.
func TestCharStateRoundTripsActiveBuffs(t *testing.T) {
	now := time.Now()
	p := &Player{Char: &model.Char{}, SpecialCoins: map[string]uint32{"wyden": 5}}
	p.Char.Affects[0] = model.Affect{Type: 4, ClientType: 4, Value: 25, ExpiresAt: now.Add(10 * time.Minute)}
	p.Char.Affects[1] = model.Affect{Type: 30, ExpiresAt: now.Add(-time.Minute)} // ja expirado

	state := buildCharState(p, now)
	if len(state.Affects) != 1 || state.Affects[0].Type != 4 || state.Affects[0].Value != 25 {
		t.Fatalf("buildCharState nao filtrou o affect expirado: %+v", state.Affects)
	}
	if state.SpecialCoins["wyden"] != 5 {
		t.Fatalf("moeda especial nao persistida: %+v", state.SpecialCoins)
	}

	fresh := &Player{Char: &model.Char{}}
	(&World{}).applyCharState(fresh, state, now)
	if a := activePlayerAffect(fresh.Char, 4); a == nil || a.Value != 25 {
		t.Fatalf("buff nao restaurado no relog: %+v", fresh.Char.Affects)
	}
	if fresh.SpecialCoins["wyden"] != 5 {
		t.Fatalf("moeda nao restaurada: %+v", fresh.SpecialCoins)
	}
}

// TestApplyCharStateDropsExpiredOnLoad garante que um buff cujo tempo ja passou
// entre o logout e o login NAO volta ativo.
func TestApplyCharStateDropsExpiredOnLoad(t *testing.T) {
	now := time.Now()
	state := &model.CharState{Version: model.CharStateVersion, Affects: []model.PersistedAffect{
		{Type: 4, Value: 10, ExpiresUnix: now.Add(5 * time.Minute).Unix()},
		{Type: 30, ExpiresUnix: now.Add(-time.Minute).Unix()},
	}}
	p := &Player{Char: &model.Char{}}
	(&World{}).applyCharState(p, state, now)
	if activePlayerAffect(p.Char, 4) == nil {
		t.Fatal("buff valido nao foi restaurado")
	}
	if activePlayerAffect(p.Char, 30) != nil {
		t.Fatal("buff expirado foi restaurado indevidamente")
	}
}

func TestCharStateStoreLoadSyncAndAsyncPaths(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Stateful", 2100, 2100)
	expires := time.Now().Add(time.Hour).Unix()
	st := &charStateMemoryStore{state: &model.CharState{
		Version:      model.CharStateVersion,
		Affects:      []model.PersistedAffect{{Type: 4, Value: 30, ExpiresUnix: expires}},
		SpecialCoins: map[string]uint32{"kefra_ticket": 7},
	}}
	w := worldWithNetworkedPlayers(p)
	w.store = st

	if err := w.loadCharStateInto(p); err != nil {
		t.Fatal(err)
	}
	if st.loads != 1 || activePlayerAffect(p.Char, 4) == nil ||
		p.SpecialCoins["kefra_ticket"] != 7 {
		t.Fatalf("load do sidecar incompleto: loads=%d coins=%v affects=%v",
			st.loads, p.SpecialCoins, p.Char.Affects)
	}
	if err := w.saveCharStateResult(p); err != nil || st.syncSaves != 1 {
		t.Fatalf("save sincrono falhou: saves=%d err=%v", st.syncSaves, err)
	}
	w.saveCharStateAsync(p)
	if st.asyncSaves != 1 {
		t.Fatal("autosave nao usou fronteira assincrona")
	}

	st.saveErr = errors.New("disk")
	w.saveCharState(p)
	w.saveCharStateAsync(p)
	if st.syncSaves != 2 || st.asyncSaves != 2 {
		t.Fatalf("caminhos de erro nao foram exercitados: sync=%d async=%d", st.syncSaves, st.asyncSaves)
	}
}

func TestCharStateStoreGuardsAndLoadFailures(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Stateful", 2100, 2100)
	st := &charStateMemoryStore{loadErr: errors.New("read")}
	w := worldWithNetworkedPlayers(p)
	w.store = st
	if err := w.loadCharStateInto(p); err == nil {
		t.Fatal("falha do loader deveria ser propagada")
	}
	if st.loads != 1 || p.SpecialCoins != nil {
		t.Fatal("falha de load alterou estado do jogador")
	}

	st.loadErr = nil
	st.state = nil
	if err := w.loadCharStateInto(p); err != nil {
		t.Fatal(err)
	}
	if st.loads != 2 {
		t.Fatal("sidecar ausente nao percorreu o loader")
	}
	w.store = &craftStore{} // store sem a interface opcional
	if err := w.loadCharStateInto(p); err != nil {
		t.Fatal(err)
	}
	if err := w.saveCharStateResult(p); err != nil {
		t.Fatalf("store sem sidecar deveria ser no-op: %v", err)
	}
	w.saveCharStateAsync(nil)
	(&World{}).applyCharState(nil, nil, time.Now())
}

func TestAccountAndCharStateUseAtomicStoreWhenAvailable(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Stateful", 2100, 2100)
	p.Char.UID = "11111111111141118111111111111111"
	p.SpecialCoins = map[string]uint32{"fame": 200}
	st := &atomicCharStateMemoryStore{}
	w := worldWithNetworkedPlayers(p)
	w.store = st

	if err := w.saveAccountAndCharStateResult(p); err != nil {
		t.Fatal(err)
	}
	if st.atomicSaves != 1 || st.syncSaves != 0 || st.saves != 0 {
		t.Fatalf("persistencia nao foi atomica: atomic=%d charstate=%d conta=%d",
			st.atomicSaves, st.syncSaves, st.saves)
	}
	if st.lastUID != p.Char.UID || st.state.SpecialCoins["fame"] != 200 {
		t.Fatalf("snapshot atomico incorreto: uid=%q state=%+v", st.lastUID, st.state)
	}
}

func TestCharacterLogoutIsNotConfirmedWhenAtomicStateSaveFails(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Stateful", 2100, 2100)
	p.Char.UID = "11111111111141118111111111111111"
	st := &atomicCharStateMemoryStore{atomicErr: errors.New("postgres unavailable")}
	w := worldWithNetworkedPlayers(p)
	w.store = st
	before := p.Session.QueuedPacketsForTest()
	w.onCharacterLogout(p.Session, make([]byte, 12))
	if !p.InWorld || p.Char == nil || p.CharSlot < 0 {
		t.Fatal("falha do charstate removeu o personagem do mundo")
	}
	if p.Session.QueuedPacketsForTest() != before+1 {
		t.Fatal("logout com falha deveria enviar somente aviso, nao confirmacao")
	}
}
