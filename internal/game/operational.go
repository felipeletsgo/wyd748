package game

import (
	"fmt"
	"strings"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// OperationalConfig contem limites de admissao/abuso que podem variar por
// ambiente sem alterar regras nativas, ABI ou formulas de gameplay.
type OperationalConfig struct {
	AuthAttemptsPerMinuteIP      int
	AuthAttemptsPerMinuteAccount int
	MaxAuthenticatedClientsPerIP int
	AuthHashConcurrency          int
	WorldCommandQueueCapacity    int
	ChatLocalPer10Seconds        int
	ChatWhisperPer10Seconds      int
	ChatGlobalPer10Seconds       int
	ChannelID                    byte
}

func DefaultOperationalConfig() OperationalConfig {
	return OperationalConfig{
		AuthAttemptsPerMinuteIP:      30,
		AuthAttemptsPerMinuteAccount: 10,
		MaxAuthenticatedClientsPerIP: 4,
		AuthHashConcurrency:          4,
		WorldCommandQueueCapacity:    1024,
		ChatLocalPer10Seconds:        8,
		ChatWhisperPer10Seconds:      8,
		ChatGlobalPer10Seconds:       2,
		ChannelID:                    1,
	}
}

func (c OperationalConfig) Validate() error {
	for name, value := range map[string]int{
		"auth por IP": c.AuthAttemptsPerMinuteIP, "auth por conta": c.AuthAttemptsPerMinuteAccount,
		"chat local": c.ChatLocalPer10Seconds, "chat whisper": c.ChatWhisperPer10Seconds,
		"chat global": c.ChatGlobalPer10Seconds,
	} {
		if value < 1 || value > 1000 {
			return fmt.Errorf("%s fora do intervalo 1..1000", name)
		}
	}
	if c.MaxAuthenticatedClientsPerIP < 1 || c.MaxAuthenticatedClientsPerIP > 10000 {
		return fmt.Errorf("clients autenticados por IP fora do intervalo 1..10000")
	}
	if c.ChannelID == 0 {
		return fmt.Errorf("channel ID deve ser maior que zero")
	}
	if c.AuthHashConcurrency < 1 || c.AuthHashConcurrency > 64 {
		return fmt.Errorf("concorrencia de hash deve ficar entre 1 e 64")
	}
	if c.WorldCommandQueueCapacity < 64 || c.WorldCommandQueueCapacity > 65536 {
		return fmt.Errorf("fila do World deve ficar entre 64 e 65536")
	}
	return nil
}

func WithOperationalConfig(config OperationalConfig) WorldOption {
	return func(w *World) { w.operational = config }
}

type fixedWindowRate struct {
	started  time.Time
	count    int
	notified bool
}

const (
	maxAuthIPRateKeys      = 2048
	maxAuthAccountRateKeys = 4096
	maxChatRateKeys        = 8192
)

func allowFixedWindow(states map[string]*fixedWindowRate, key string, now time.Time,
	window time.Duration, limit, maxKeys int,
) (allowed, notify bool) {
	key = strings.ToLower(strings.TrimSpace(key))
	state := states[key]
	if state == nil && len(states) >= maxKeys {
		for candidate, existing := range states {
			if existing == nil || now.Before(existing.started) || now.Sub(existing.started) >= window {
				delete(states, candidate)
			}
		}
		if len(states) >= maxKeys {
			return false, false
		}
	}
	if state == nil || now.Sub(state.started) >= window || now.Before(state.started) {
		state = &fixedWindowRate{started: now}
		states[key] = state
	}
	state.count++
	if state.count <= limit {
		return true, false
	}
	if !state.notified {
		state.notified = true
		return false, true
	}
	return false, false
}

func (w *World) allowLoginAttempt(ip, account string, now time.Time) bool {
	originKey, ok := net.OriginLimitKey(ip)
	if !ok {
		return false
	}
	if w.authRateByIP == nil {
		w.authRateByIP = make(map[string]*fixedWindowRate)
	}
	if w.authRateByAccount == nil {
		w.authRateByAccount = make(map[string]*fixedWindowRate)
	}
	defaults := DefaultOperationalConfig()
	if w.operational.AuthAttemptsPerMinuteIP <= 0 {
		w.operational.AuthAttemptsPerMinuteIP = defaults.AuthAttemptsPerMinuteIP
	}
	if w.operational.AuthAttemptsPerMinuteAccount <= 0 {
		w.operational.AuthAttemptsPerMinuteAccount = defaults.AuthAttemptsPerMinuteAccount
	}
	ipOK, _ := allowFixedWindow(w.authRateByIP, originKey, now, time.Minute,
		w.operational.AuthAttemptsPerMinuteIP, maxAuthIPRateKeys)
	accountOK, _ := allowFixedWindow(w.authRateByAccount, account, now, time.Minute,
		w.operational.AuthAttemptsPerMinuteAccount, maxAuthAccountRateKeys)
	return ipOK && accountOK
}

func (w *World) allowChat(p *Player, channel string, now time.Time) bool {
	if p == nil || p.Account == nil {
		return false
	}
	if w.chatRateByAccount == nil {
		w.chatRateByAccount = make(map[string]*fixedWindowRate)
	}
	defaults := DefaultOperationalConfig()
	if w.operational.ChatLocalPer10Seconds <= 0 {
		w.operational.ChatLocalPer10Seconds = defaults.ChatLocalPer10Seconds
	}
	if w.operational.ChatWhisperPer10Seconds <= 0 {
		w.operational.ChatWhisperPer10Seconds = defaults.ChatWhisperPer10Seconds
	}
	if w.operational.ChatGlobalPer10Seconds <= 0 {
		w.operational.ChatGlobalPer10Seconds = defaults.ChatGlobalPer10Seconds
	}
	limit := w.operational.ChatWhisperPer10Seconds
	switch channel {
	case "local":
		limit = w.operational.ChatLocalPer10Seconds
	case chatChannelGlobal:
		limit = w.operational.ChatGlobalPer10Seconds
	}
	key := p.Account.Name + "\x00" + channel
	allowed, notify := allowFixedWindow(w.chatRateByAccount, key, now, 10*time.Second, limit, maxChatRateKeys)
	if !allowed && notify && p.Session != nil {
		p.Session.Send(wire.MessagePanel("You are sending messages too quickly."))
	}
	return allowed
}
