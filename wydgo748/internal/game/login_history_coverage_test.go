package game

import (
	"context"
	"errors"
	"testing"
	"time"
)

type loginHistoryCoverageCall struct {
	account  string
	clientIP string
	channel  int
	active   bool
	failed   bool
}

type loginHistoryCoverageStore struct {
	*craftStore
	calls chan loginHistoryCoverageCall
}

func (s *loginHistoryCoverageStore) RecordAccountLogin(ctx context.Context, account, clientIP string, channel int) error {
	failed := account == "failing-account"
	s.calls <- loginHistoryCoverageCall{account: account, clientIP: clientIP, channel: channel,
		active: ctx.Err() == nil, failed: failed}
	if failed {
		return errors.New("injected login-history write failure")
	}
	return nil
}

func TestStartLoginHistorySkipsUnsupportedStoresAndRecordsOnConfiguredChannel(t *testing.T) {
	unsupported := &World{store: &craftStore{}}
	unsupported.startLoginHistory()
	if unsupported.loginHistory != nil {
		t.Fatal("unsupported store started a login-history worker")
	}

	for _, channelID := range []byte{0, 4} {
		t.Run(map[bool]string{true: "default channel", false: "configured channel"}[channelID == 0], func(t *testing.T) {
			backend := &loginHistoryCoverageStore{craftStore: &craftStore{}, calls: make(chan loginHistoryCoverageCall, 2)}
			w := &World{store: backend}
			w.operational.ChannelID = channelID
			w.startLoginHistory()
			if w.loginHistory == nil {
				t.Fatal("supported store did not start login-history recording")
			}
			w.recordLogin("failing-account", "198.51.100.10")
			w.recordLogin("normal-account", "198.51.100.11")
			close(w.loginHistory)

			wantChannel := int(channelID)
			if wantChannel == 0 {
				wantChannel = 1
			}
			seen := map[string]loginHistoryCoverageCall{}
			for range 2 {
				select {
				case call := <-backend.calls:
					seen[call.account] = call
				case <-time.After(time.Second):
					t.Fatal("login-history worker did not persist queued records")
				}
			}
			if failing := seen["failing-account"]; failing.account == "" || !failing.active || !failing.failed ||
				failing.clientIP != "198.51.100.10" || failing.channel != wantChannel {
				t.Fatalf("failed login record = %+v, want channel %d", failing, wantChannel)
			}
			if normal := seen["normal-account"]; normal.account == "" || !normal.active || normal.failed ||
				normal.clientIP != "198.51.100.11" || normal.channel != wantChannel {
				t.Fatalf("normal login record = %+v, want channel %d", normal, wantChannel)
			}
		})
	}
}
