package data

import "testing"

// W9 phase 5: a channel process names the channels of the other processes.
func TestChannelPeersConfiguration(t *testing.T) {
	cfg, err := LoadServerConfig(writeServerConfig(t, "channel_id=2\nchannel_peers=1, 3\n"))
	if err != nil || cfg.ChannelID != 2 || len(cfg.ChannelPeers) != 2 || cfg.ChannelPeers[0] != 1 || cfg.ChannelPeers[1] != 3 {
		t.Fatalf("channel peers: %+v %v", cfg.ChannelPeers, err)
	}
	for _, invalid := range []string{
		"channel_id=1\nchannel_peers=1\n",
		"channel_peers=2,2\n",
		"channel_peers=0\n",
		"channel_peers=x\n",
		"channel2_enabled=true\nchannel_peers=2\n",
	} {
		if _, err := LoadServerConfig(writeServerConfig(t, invalid)); err == nil {
			t.Fatalf("accepted %q", invalid)
		}
	}
	if cfg, err := LoadServerConfig(writeServerConfig(t, "channel_id=1\n")); err != nil || len(cfg.ChannelPeers) != 0 {
		t.Fatalf("default peers: %+v %v", cfg.ChannelPeers, err)
	}
}
