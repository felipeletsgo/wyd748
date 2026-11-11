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

func TestLoginRateLimitGroupsIPv6By64(t *testing.T) {
	w := &World{operational: OperationalConfig{
		AuthAttemptsPerMinuteIP: 1, AuthAttemptsPerMinuteAccount: 10,
	}}
	now := time.Unix(100, 0)
	if !w.allowLoginAttempt("2001:db8:1:2::1", "first", now) {
		t.Fatal("primeira tentativa IPv6 foi recusada")
	}
	if w.allowLoginAttempt("2001:db8:1:2::2", "second", now) {
		t.Fatal("endereco temporario do mesmo /64 burlou o limite por origem")
	}
	if !w.allowLoginAttempt("2001:db8:1:3::1", "third", now) {
		t.Fatal("outro /64 IPv6 foi bloqueado indevidamente")
	}
}

func TestLoginRateLimitCanonicalizesIPv4MappedIPv6(t *testing.T) {
	w := &World{operational: OperationalConfig{
		AuthAttemptsPerMinuteIP: 1, AuthAttemptsPerMinuteAccount: 10,
	}}
	now := time.Unix(100, 0)
	if !w.allowLoginAttempt("192.0.2.44", "first", now) {
		t.Fatal("primeira tentativa IPv4 foi recusada")
	}
	if w.allowLoginAttempt("::ffff:192.0.2.44", "second", now) {
		t.Fatal("IPv4-mapped IPv6 burlou o limite da mesma origem IPv4")
	}
}

func TestLoginRateLimitRejectsInvalidOriginWithoutAllocatingState(t *testing.T) {
	w := &World{operational: OperationalConfig{
		AuthAttemptsPerMinuteIP: 1, AuthAttemptsPerMinuteAccount: 1,
	}}
	if w.allowLoginAttempt("not-an-ip", "first", time.Unix(100, 0)) {
		t.Fatal("origem invalida foi aceita pelo limitador de login")
	}
	if w.authRateByIP != nil || w.authRateByAccount != nil {
		t.Fatal("origem invalida alocou estado no limitador")
	}
}

func TestBlockedLoginOriginDoesNotAllocateAccountRateKeys(t *testing.T) {
	w := &World{operational: OperationalConfig{
		AuthAttemptsPerMinuteIP: 1, AuthAttemptsPerMinuteAccount: 100,
	}}
	now := time.Unix(100, 0)
	if !w.allowLoginAttempt("192.0.2.1", "first", now) {
		t.Fatal("primeira tentativa foi recusada")
	}
	before := len(w.authRateByAccount)
	for i := 0; i < 100; i++ {
		if w.allowLoginAttempt("192.0.2.1", fmt.Sprintf("blocked-%d", i), now) {
			t.Fatal("origem bloqueada foi aceita")
		}
	}
	if got := len(w.authRateByAccount); got != before {
		t.Fatalf("origem bloqueada criou chaves por conta: %d -> %d", before, got)
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

func TestOperationalConfigValidateCoversEveryLimitBoundary(t *testing.T) {
	minimums := OperationalConfig{
		AuthAttemptsPerMinuteIP: 1, AuthAttemptsPerMinuteAccount: 1,
		MaxAuthenticatedClientsPerIP: 1, AuthHashConcurrency: 1,
		WorldCommandQueueCapacity: 64, ChatLocalPer10Seconds: 1,
		ChatWhisperPer10Seconds: 1, ChatGlobalPer10Seconds: 1, ChannelID: 1,
	}
	maximums := OperationalConfig{
		AuthAttemptsPerMinuteIP: 1000, AuthAttemptsPerMinuteAccount: 1000,
		MaxAuthenticatedClientsPerIP: 10000, AuthHashConcurrency: 64,
		WorldCommandQueueCapacity: 65536, ChatLocalPer10Seconds: 1000,
		ChatWhisperPer10Seconds: 1000, ChatGlobalPer10Seconds: 1000, ChannelID: 255,
	}
	for _, test := range []struct {
		name   string
		config OperationalConfig
	}{
		{name: "minimums", config: minimums},
		{name: "maximums", config: maximums},
	} {
		t.Run(test.name, func(t *testing.T) {
			if err := test.config.Validate(); err != nil {
				t.Fatalf("valid boundary configuration rejected: %v", err)
			}
		})
	}

	invalid := []struct {
		name   string
		mutate func(*OperationalConfig)
	}{
		{"auth per IP below minimum", func(c *OperationalConfig) { c.AuthAttemptsPerMinuteIP = 0 }},
		{"auth per IP above maximum", func(c *OperationalConfig) { c.AuthAttemptsPerMinuteIP = 1001 }},
		{"auth per account below minimum", func(c *OperationalConfig) { c.AuthAttemptsPerMinuteAccount = 0 }},
		{"auth per account above maximum", func(c *OperationalConfig) { c.AuthAttemptsPerMinuteAccount = 1001 }},
		{"local chat below minimum", func(c *OperationalConfig) { c.ChatLocalPer10Seconds = 0 }},
		{"local chat above maximum", func(c *OperationalConfig) { c.ChatLocalPer10Seconds = 1001 }},
		{"whisper chat below minimum", func(c *OperationalConfig) { c.ChatWhisperPer10Seconds = 0 }},
		{"whisper chat above maximum", func(c *OperationalConfig) { c.ChatWhisperPer10Seconds = 1001 }},
		{"global chat below minimum", func(c *OperationalConfig) { c.ChatGlobalPer10Seconds = 0 }},
		{"global chat above maximum", func(c *OperationalConfig) { c.ChatGlobalPer10Seconds = 1001 }},
		{"clients per IP below minimum", func(c *OperationalConfig) { c.MaxAuthenticatedClientsPerIP = 0 }},
		{"clients per IP above maximum", func(c *OperationalConfig) { c.MaxAuthenticatedClientsPerIP = 10001 }},
		{"zero channel ID", func(c *OperationalConfig) { c.ChannelID = 0 }},
		{"hash concurrency below minimum", func(c *OperationalConfig) { c.AuthHashConcurrency = 0 }},
		{"hash concurrency above maximum", func(c *OperationalConfig) { c.AuthHashConcurrency = 65 }},
		{"queue capacity below minimum", func(c *OperationalConfig) { c.WorldCommandQueueCapacity = 63 }},
		{"queue capacity above maximum", func(c *OperationalConfig) { c.WorldCommandQueueCapacity = 65537 }},
	}
	for _, test := range invalid {
		t.Run(test.name, func(t *testing.T) {
			config := DefaultOperationalConfig()
			test.mutate(&config)
			if err := config.Validate(); err == nil {
				t.Fatal("invalid configuration was accepted")
			}
		})
	}
}
