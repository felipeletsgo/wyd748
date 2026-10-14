package playerportal

import (
	"context"
	"encoding/json"
	"errors"
	"strings"
	"sync"
	"testing"
	"time"
)

func (b *memoryBackend) PublicGuilds(ctx context.Context) (PublicGuildSnapshot, error) {
	b.mu.Lock()
	b.guildCalls++
	snapshot, fail, wait := b.guildSnapshot, b.fail, b.guildWait
	b.mu.Unlock()
	if wait != nil {
		select {
		case <-wait:
		case <-ctx.Done():
			return PublicGuildSnapshot{}, ctx.Err()
		}
	}
	if fail {
		return PublicGuildSnapshot{}, errors.New("private account and database details")
	}
	return snapshot, nil
}

func publicGuildFixture() PublicGuildSnapshot {
	return PublicGuildSnapshot{Version: 1, AsOf: time.Date(2026, 10, 13, 12, 0, 0, 0, time.UTC), Guilds: []SavedPublicGuild{
		{ID: 1, Name: "Alpha Team", Kingdom: 7, Fame: 4294967295, Ally: 2, WarTarget: 2, Members: []PublicGuildMember{{Name: "Member", Rank: 0}, {Name: "Officer", Rank: 7}, {Name: "Leader", Rank: 9}}},
		{ID: 2, Name: "Beta", Kingdom: 8, Members: []PublicGuildMember{{Name: "OtherLeader", Rank: 9}}},
	}}
}

func TestPublicGuildProfile(t *testing.T) {
	h, b := testHandler(t)
	b.guildSnapshot = publicGuildFixture()
	w := request(h, "GET", "/api/player/v1/guilds/alpha%20team?username=SomeoneElse", "", nil, "")
	if w.Code != 200 || w.Header().Get("Cache-Control") != "no-store" {
		t.Fatal(w.Code, w.Header(), w.Body.String())
	}
	var got struct {
		Version int
		Guild   PublicGuildProfile
		Source  string
		AsOf    time.Time
	}
	if err := json.Unmarshal(w.Body.Bytes(), &got); err != nil {
		t.Fatal(err)
	}
	if got.Version != 1 || got.Guild.Name != "Alpha Team" || got.Guild.Fame != 4294967295 || got.Guild.Kingdom != 7 || got.Source != "persisted snapshot" || got.AsOf != b.guildSnapshot.AsOf || *got.Guild.Ally != "Beta" || *got.Guild.WarTarget != "Beta" {
		t.Fatal(got)
	}
	if got.Guild.Members[0].Rank != 9 || got.Guild.Members[1].Rank != 7 || b.guildSnapshot.Guilds[0].Members[0].Name != "Member" {
		t.Fatal("sorting mutated the canonical snapshot")
	}
	for _, private := range []string{`"account"`, `"id"`, `"joinedAt"`, `"wars"`, `"username"`} {
		if strings.Contains(w.Body.String(), private) {
			t.Fatal("private field exposed", private)
		}
	}
	for _, name := range []string{"Bad!", "%20Alpha", "ABCDEFGHIJKLM"} {
		if w := request(h, "GET", "/api/player/v1/guilds/"+name, "", nil, ""); w.Code != 400 {
			t.Fatal(w.Code, w.Body.String())
		}
	}
	if w := request(h, "GET", "/api/player/v1/guilds/Missing", "", nil, ""); w.Code != 404 {
		t.Fatal(w.Code)
	}
	if b.guildCalls != 1 {
		t.Fatal("selectors bypassed the shared cache", b.guildCalls)
	}
}

func TestPublicGuildCacheLifecycle(t *testing.T) {
	h, b := testHandler(t)
	b.guildSnapshot = publicGuildFixture()
	now := time.Now()
	h.now = func() time.Time { return now }
	if _, err := h.publicGuildSnapshot(context.Background()); err != nil {
		t.Fatal(err)
	}
	b.fail = true
	now = now.Add(time.Minute)
	for i := 0; i < 10; i++ {
		w := request(h, "GET", "/api/player/v1/guilds/Beta", "", nil, "")
		if w.Code != 503 || strings.Contains(w.Body.String(), "private") {
			t.Fatal(w.Code, w.Body.String())
		}
	}
	if b.guildCalls != 2 {
		t.Fatal("failure cooldown not respected", b.guildCalls)
	}
	b.fail = false
	b.guildSnapshot.Guilds = []SavedPublicGuild{}
	now = now.Add(5 * time.Second)
	if w := request(h, "GET", "/api/player/v1/guilds/Beta", "", nil, ""); w.Code != 404 {
		t.Fatal("old roster served", w.Code)
	}
}

func TestPublicGuildCoalescingAndCancellation(t *testing.T) {
	h, b := testHandler(t)
	b.guildSnapshot = publicGuildFixture()
	release := make(chan struct{})
	b.guildWait = release
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		defer wg.Done()
		_, err := h.publicGuildSnapshot(context.Background())
		if err != nil {
			t.Error(err)
		}
	}()
	deadline := time.After(time.Second)
	for {
		b.mu.Lock()
		calls := b.guildCalls
		b.mu.Unlock()
		if calls == 1 {
			break
		}
		select {
		case <-deadline:
			t.Fatal("snapshot did not start")
		default:
			time.Sleep(time.Millisecond)
		}
	}
	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	if _, err := h.publicGuildSnapshot(cancelled); !errors.Is(err, context.Canceled) {
		t.Fatal(err)
	}
	for i := 0; i < 20; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			if _, err := h.publicGuildSnapshot(context.Background()); err != nil {
				t.Error(err)
			}
		}()
	}
	close(release)
	wg.Wait()
	if b.guildCalls != 1 {
		t.Fatal("parallel requests fanned out", b.guildCalls)
	}
}

func TestPublicGuildInvalidSnapshots(t *testing.T) {
	mutations := map[string]func(*PublicGuildSnapshot){
		"version": func(s *PublicGuildSnapshot) { s.Version++ }, "nil": func(s *PublicGuildSnapshot) { s.Guilds = nil }, "timestamp": func(s *PublicGuildSnapshot) { s.AsOf = time.Time{} },
		"id": func(s *PublicGuildSnapshot) { s.Guilds[0].ID = 0 }, "name": func(s *PublicGuildSnapshot) { s.Guilds[0].Name = "<img>" }, "duplicate": func(s *PublicGuildSnapshot) { s.Guilds[1].ID = 1 },
		"kingdom": func(s *PublicGuildSnapshot) { s.Guilds[0].Kingdom = 1 }, "no leader": func(s *PublicGuildSnapshot) { s.Guilds[0].Members[2].Rank = 0 },
		"two leaders": func(s *PublicGuildSnapshot) { s.Guilds[0].Members[0].Rank = 9 }, "rank": func(s *PublicGuildSnapshot) { s.Guilds[0].Members[0].Rank = 5 },
		"duplicate officer": func(s *PublicGuildSnapshot) { s.Guilds[0].Members[0].Rank = 7 }, "duplicate member": func(s *PublicGuildSnapshot) { s.Guilds[1].Members[0].Name = "Leader" },
		"missing ally": func(s *PublicGuildSnapshot) { s.Guilds[0].Ally = 3 }, "self war": func(s *PublicGuildSnapshot) { s.Guilds[0].WarTarget = 1 },
	}
	for name, mutate := range mutations {
		t.Run(name, func(t *testing.T) {
			h, b := testHandler(t)
			b.guildSnapshot = publicGuildFixture()
			mutate(&b.guildSnapshot)
			w := request(h, "GET", "/api/player/v1/guilds/Alpha%20Team", "", nil, "")
			if w.Code != 503 {
				t.Fatal(w.Code, w.Body.String())
			}
		})
	}
}
