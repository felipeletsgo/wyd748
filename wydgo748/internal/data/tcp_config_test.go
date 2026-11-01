package data

import "testing"

func TestTCPFailureDetectionConfigBounds(t *testing.T) {
	defaults := DefaultServerConfig()
	if defaults.TCPUserTimeoutSecs != 30 || defaults.TCPKeepAliveIdleSecs != 15 ||
		defaults.TCPKeepAliveIntervalSecs != 5 || defaults.TCPKeepAliveCount != 4 {
		t.Fatalf("unexpected TCP defaults: %+v", defaults)
	}
	for _, setting := range []string{"tcp_user_timeout_seconds=0", "tcp_user_timeout_seconds=600",
		"tcp_keepalive_idle_seconds=5", "tcp_keepalive_interval_seconds=120", "tcp_keepalive_count=20"} {
		if _, err := LoadServerConfig(writeServerConfig(t, setting+"\n")); err != nil {
			t.Fatalf("%s rejected: %v", setting, err)
		}
	}
	for _, setting := range []string{"tcp_user_timeout_seconds=601", "tcp_keepalive_idle_seconds=4",
		"tcp_keepalive_interval_seconds=0", "tcp_keepalive_count=0", "tcp_keepalive_count=21"} {
		if _, err := LoadServerConfig(writeServerConfig(t, setting+"\n")); err == nil {
			t.Fatalf("%s accepted", setting)
		}
	}
}
