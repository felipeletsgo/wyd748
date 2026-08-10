package game

import (
	"fmt"
	"testing"
	"time"
)

func TestLoginRateLimitSeparatesIPAndAccount(t *testing.T) {
	w := &World{operational: OperationalConfig{
		AuthAttemptsPerMinuteIP: 2, AuthAttemptsPerMinuteAccount: 1,
	}}
	now := time.Unix(100, 0)
	if !w.allowLoginAttempt("192.0.2.1", "first", now) {
		t.Fatal("primeira tentativa foi recusada")
	}
	if w.allowLoginAttempt("192.0.2.2", "first", now) {
		t.Fatal("trocar o IP burlou o limite por conta")
	}
	if !w.allowLoginAttempt("192.0.2.1", "second", now) {
		t.Fatal("segunda tentativa permitida pelo IP foi recusada")
	}
	if w.allowLoginAttempt("192.0.2.1", "third", now) {
		t.Fatal("trocar a conta burlou o limite por IP")
	}
	if !w.allowLoginAttempt("192.0.2.1", "third", now.Add(time.Minute)) {
		t.Fatal("janela expirada nao foi renovada")
	}
}

func TestChatRateLimitIsPerAccountAndChannel(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Speaker", 2100, 2100)
	w := &World{operational: OperationalConfig{
		ChatLocalPer10Seconds: 1, ChatWhisperPer10Seconds: 1, ChatGlobalPer10Seconds: 1,
	}}
	now := time.Unix(100, 0)
	if !w.allowChat(p, "local", now) || w.allowChat(p, "local", now) {
		t.Fatal("limite do chat local nao foi aplicado")
	}
	if !w.allowChat(p, chatChannelWhisper, now) {
		t.Fatal("chat local consumiu a janela de whisper")
	}
	if !w.allowChat(p, "local", now.Add(10*time.Second)) {
		t.Fatal("janela do chat nao expirou")
	}
}

func TestRateLimitStateHasBoundedCardinality(t *testing.T) {
	states := make(map[string]*fixedWindowRate)
	now := time.Unix(100, 0)
	for i := 0; i < 8; i++ {
		allowFixedWindow(states, fmt.Sprintf("key-%d", i), now, time.Minute, 1, 4)
	}
	if len(states) != 4 {
		t.Fatalf("mapa de limiter cresceu sem limite: %d", len(states))
	}
	if allowed, _ := allowFixedWindow(states, "new", now.Add(time.Minute), time.Minute, 1, 4); !allowed {
		t.Fatal("chaves expiradas nao foram removidas para admitir nova origem")
	}
}

func TestEveryChatCommandAliasIsReservedAsCharacterName(t *testing.T) {
	for alias := range chatCommandAliases {
		if len(alias) >= 4 && validCharacterName(alias) {
			t.Errorf("alias de comando %q ainda pode ser nickname", alias)
		}
	}
}
