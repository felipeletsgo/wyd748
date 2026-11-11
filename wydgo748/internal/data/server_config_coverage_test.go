package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestLoadServerConfigRejectsEveryOperationalRangeBoundary(t *testing.T) {
	for _, test := range []struct{ name, config string }{
		{"workers zero", "critical_persistence_workers=0\n"},
		{"workers consume pool", "database_max_conns=4\ncritical_persistence_workers=4\n"},
		{"critical capacity below workers", "critical_persistence_capacity=3\n"},
		{"critical capacity above maximum", "critical_persistence_capacity=65537\n"},
		{"session queue zero", "session_queue_capacity=0\n"},
		{"session queue above maximum", "session_queue_capacity=8193\n"},
		{"world queue below minimum", "world_command_queue_capacity=63\n"},
		{"world queue above maximum", "world_command_queue_capacity=65537\n"},
		{"hash concurrency zero", "auth_hash_concurrency=0\n"},
		{"hash concurrency above maximum", "auth_hash_concurrency=65\n"},
		{"connections zero", "max_connections=0\n"},
		{"connections above maximum", "max_connections=10001\n"},
		{"per IP connections zero", "max_connections_per_ip=0\n"},
		{"per IP connections exceed global", "max_connections=10\nmax_connections_per_ip=11\n"},
		{"authenticated clients zero", "max_authenticated_clients_per_ip=0\n"},
		{"authenticated clients exceed per IP", "max_connections_per_ip=4\nmax_authenticated_clients_per_ip=5\n"},
		{"handshake timeout zero", "handshake_timeout_seconds=0\n"},
		{"handshake timeout above maximum", "handshake_timeout_seconds=61\n"},
		{"idle timeout below minimum", "session_idle_timeout_seconds=299\n"},
		{"idle timeout above maximum", "session_idle_timeout_seconds=3601\n"},
		{"frame timeout zero", "frame_read_timeout_seconds=0\n"},
		{"frame timeout above maximum", "frame_read_timeout_seconds=61\n"},
		{"TCP user timeout above maximum", "tcp_user_timeout_seconds=601\n"},
		{"keepalive idle below minimum", "tcp_keepalive_idle_seconds=4\n"},
		{"keepalive idle above maximum", "tcp_keepalive_idle_seconds=601\n"},
		{"keepalive interval zero", "tcp_keepalive_interval_seconds=0\n"},
		{"keepalive interval above maximum", "tcp_keepalive_interval_seconds=121\n"},
		{"keepalive count zero", "tcp_keepalive_count=0\n"},
		{"keepalive count above maximum", "tcp_keepalive_count=21\n"},
		{"inbound packet rate below minimum", "inbound_packets_per_second=15\n"},
		{"inbound packet rate above maximum", "inbound_packets_per_second=4097\n"},
		{"inbound byte rate below minimum", "inbound_bytes_per_second=16383\n"},
		{"inbound byte rate above maximum", "inbound_bytes_per_second=16777217\n"},
		{"IP auth rate zero", "auth_attempts_per_minute_ip=0\n"},
		{"IP auth rate above maximum", "auth_attempts_per_minute_ip=1001\n"},
		{"account auth rate zero", "auth_attempts_per_minute_account=0\n"},
		{"account auth rate above maximum", "auth_attempts_per_minute_account=1001\n"},
		{"local chat rate zero", "chat_local_per_10_seconds=0\n"},
		{"local chat rate above maximum", "chat_local_per_10_seconds=101\n"},
		{"whisper rate zero", "chat_whisper_per_10_seconds=0\n"},
		{"whisper rate above maximum", "chat_whisper_per_10_seconds=101\n"},
		{"global chat rate zero", "chat_global_per_10_seconds=0\n"},
		{"global chat rate above maximum", "chat_global_per_10_seconds=101\n"},
		{"persistence timeout below minimum", "critical_persistence_timeout_ms=99\n"},
		{"persistence timeout above maximum", "critical_persistence_timeout_ms=10001\n"},
		{"database read timeout below minimum", "database_read_timeout_ms=99\n"},
		{"database read timeout above maximum", "database_read_timeout_ms=30001\n"},
		{"channel zero", "channel_id=0\n"},
		{"channel above byte range", "channel_id=256\n"},
		{"invalid primary language", "channel_language=fr\n"},
		{"invalid secondary language", "channel2_language=fr\n"},
		{"secondary channel ID must be one", "channel2_enabled=true\nchannel_id=2\n"},
		{"secondary channel listener must differ", "channel2_enabled=true\nchannel2_address=0.0.0.0:8281\n"},
		{"secondary channel address invalid", "channel2_enabled=true\nchannel2_address=127.0.0.1:0\n"},
		{"channel peer invalid", "channel_peers=256\n"},
		{"channel peer repeats", "channel_peers=2,2\n"},
		{"channel peer includes self", "channel_peers=1\n"},
		{"peer mode conflicts with channel two", "channel2_enabled=true\nchannel_peers=2\n"},
		{"loadtest prefix must start with bot", "loadtest_spawn=1162,1700\nloadtest_account_prefix=test\n"},
		{"loadtest prefix length", "loadtest_spawn=1162,1700\nloadtest_account_prefix=bot123456\n"},
		{"loadtest prefix characters", "loadtest_spawn=1162,1700\nloadtest_account_prefix=bot_test\n"},
	} {
		t.Run(test.name, func(t *testing.T) {
			if _, err := LoadServerConfig(writeServerConfig(t, test.config)); err == nil {
				t.Fatalf("accepted invalid configuration %q", strings.TrimSpace(test.config))
			}
		})
	}
}

func TestLoadServerConfigParsesChannelPeersAndRejectsMalformedConfig(t *testing.T) {
	cfg, err := LoadServerConfig(writeServerConfig(t, "channel_peers=2, 3,,\nweb_admin_log_reader_socket=/var/run/wyd/log-reader.sock\n"))
	if err != nil || len(cfg.ChannelPeers) != 2 || cfg.ChannelPeers[0] != 2 || cfg.ChannelPeers[1] != 3 ||
		cfg.WebAdminLogReaderSocket != "/var/run/wyd/log-reader.sock" {
		t.Fatalf("parsed channel peer/socket settings: %+v err=%v", cfg, err)
	}
	for _, input := range []string{
		"web_admin_log_reader_socket=relative.sock\n",
		"loadtest_spawn=1162\n",
		"loadtest_spawn=x,1700\n",
		"loadtest_spawn=1162,-1\n",
		"\nunknown_setting=value\n",
		"listen_address=\n",
	} {
		if _, err := LoadServerConfig(writeServerConfig(t, input)); err == nil {
			t.Errorf("accepted malformed configuration %q", strings.TrimSpace(input))
		}
	}
	if _, err := LoadServerConfig(filepath.Join(t.TempDir(), "missing.txt")); err == nil {
		t.Fatal("accepted missing configuration file")
	}
	tooLong := filepath.Join(t.TempDir(), "too-long.txt")
	if err := os.WriteFile(tooLong, []byte(strings.Repeat("x", 70*1024)), 0600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadServerConfig(tooLong); err == nil || !strings.Contains(err.Error(), "token too long") {
		t.Fatalf("scanner error=%v, want token limit", err)
	}
}

func TestLoadServerConfigParsesRemainingOperationalAndContentKeys(t *testing.T) {
	path := writeServerConfig(t, `
colosseum_enabled=false
colosseum_event_hour=17
database_url=postgres://wyd:secret@127.0.0.1/wyd
language_dir=data/language-test
npcs=data/npcs-test
guilds_txt=runtime/Guilds-test.txt
quest_zones=data/quest-zones-test.json
init_items=data/init-test.csv
boss=data/boss-test
items=data/items-test.csv
itemnames=data/item-names-test.csv
itemeffects=data/item-effects-test.h
skills=data/skills-test.csv
droprates=data/drops-test.json
volatiles=data/volatiles-test.json
instances=data/instances-test.json
instance_generators=data/instance-generators-test
repliction=data/repliction-test.json
mounts=data/mounts-test.json
character_templates=data/templates-test.json
network_admission=data/admission-test.json
client_integrity=data/integrity-test.json
debug_address=localhost:6060
channel_peers=2,3
loadtest_spawn=1162,1700
loadtest_account_prefix= BotQA`+" "+`
exp_rate=125
compositor_refine_8=6
`)
	cfg, err := LoadServerConfig(path)
	if err != nil {
		t.Fatal(err)
	}
	if cfg.Colosseum.Enabled || cfg.Colosseum.EventHour != 17 || cfg.DatabaseURL != "postgres://wyd:secret@127.0.0.1/wyd" ||
		cfg.LanguageDir != "data/language-test" || cfg.NPCPath != "data/npcs-test" ||
		cfg.GuildsTxtPath != "runtime/Guilds-test.txt" || cfg.QuestZonesPath != "data/quest-zones-test.json" ||
		cfg.InitItemsPath != "data/init-test.csv" || cfg.BossPath != "data/boss-test" || cfg.ItemPath != "data/items-test.csv" ||
		cfg.ItemNamePath != "data/item-names-test.csv" || cfg.ItemEffectPath != "data/item-effects-test.h" ||
		cfg.SkillPath != "data/skills-test.csv" || cfg.DropRatePath != "data/drops-test.json" ||
		cfg.VolatilePath != "data/volatiles-test.json" || cfg.InstancesPath != "data/instances-test.json" ||
		cfg.InstanceGeneratorsPath != "data/instance-generators-test" || cfg.ReplictionPath != "data/repliction-test.json" ||
		cfg.MountPath != "data/mounts-test.json" || cfg.CharacterTemplatePath != "data/templates-test.json" ||
		cfg.NetworkAdmissionPath != "data/admission-test.json" || cfg.ClientIntegrityPath != "data/integrity-test.json" ||
		cfg.DebugAddress != "localhost:6060" || len(cfg.ChannelPeers) != 2 || cfg.ChannelPeers[0] != 2 || cfg.ChannelPeers[1] != 3 ||
		cfg.LoadtestSpawn.X != 1162 || cfg.LoadtestSpawn.Y != 1700 || cfg.LoadtestAccountPrefix != "botqa" ||
		cfg.Gameplay.EXPRatePercent != 125 || cfg.Gameplay.CompositorRefineChance[1] != 6 {
		t.Fatalf("configuration keys were not preserved: %+v", cfg)
	}
}

func TestLoadServerConfigReportsFinalValidationFailures(t *testing.T) {
	if prefixes, err := ParseTrustedProxies(" \t"); err != nil || len(prefixes) != 0 {
		t.Fatalf("empty trusted proxy list = %v, %v", prefixes, err)
	}
	if _, err := LoadServerConfig(writeServerConfig(t, "listen_address=127.0.0.1\n")); err == nil {
		t.Fatal("listen address without a port was accepted")
	}
	if _, err := LoadServerConfig(writeServerConfig(t, "colosseum_event_hour=21\n")); err == nil {
		t.Fatal("event schedule crossing midnight was accepted")
	}
}
