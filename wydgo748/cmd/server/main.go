// Command server runs the native WYD 7.48 server in Go.
//
// This entry point loads static data and storage, starts the World game loop,
// and serves incoming connections through the listener.
package main

import (
	"context"
	"flag"
	"log"
	stdnet "net"
	stdhttp "net/http"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	// Register /debug/pprof on the default mux. It is reachable only when
	// debug_address is configured, which requires a loopback address.
	_ "net/http/pprof"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/game"
	"wydgo/internal/net"
	"wydgo/internal/store"
)

// shutdownTimeout bounds final persistence. It allows hundreds of accounts to
// be saved while remaining below systemd's default 90-second TimeoutStopSec.
const shutdownTimeout = 20 * time.Second

// defaultGeneratorExtraPath excludes test content from normal startup.
// The overlay remains available by explicitly passing -gener-extra.
const defaultGeneratorExtraPath = ""

// serveDebug exposes expvar (/debug/vars) and pprof (/debug/pprof) on loopback.
// LoadServerConfig rejects public addresses rather than exposing diagnostics.
func serveDebug(address string) {
	log.Printf("diagnostics at http://%s/debug/vars and /debug/pprof (loopback only)", address)
	server := &stdhttp.Server{
		Addr:              address,
		ReadHeaderTimeout: 5 * time.Second,
	}
	if err := server.ListenAndServe(); err != nil {
		log.Printf("diagnostics: %v", err)
	}
}

func configPathFromArgs(args []string) string {
	const defaultPath = "data/server.txt"
	for i, arg := range args {
		if arg == "-config" || arg == "--config" {
			if i+1 < len(args) {
				return args[i+1]
			}
			return defaultPath
		}
		if value, ok := strings.CutPrefix(arg, "-config="); ok {
			return value
		}
		if value, ok := strings.CutPrefix(arg, "--config="); ok {
			return value
		}
	}
	return defaultPath
}

func main() {
	configPath := configPathFromArgs(os.Args[1:])
	cfg, err := data.LoadServerConfig(configPath)
	if err != nil {
		log.Fatal(err)
	}
	flag.String("config", configPath, "text configuration file")
	addr := flag.String("addr", cfg.ListenAddress, "listen address (host:port)")
	npcPath := flag.String("npcs", cfg.NPCPath, "NPC directory (one .json file per NPC)")
	generPath := flag.String("gener", cfg.GeneratorPath, "default NPCGener.txt spawn file")
	generExtraPath := flag.String("gener-extra", defaultGeneratorExtraPath,
		"additional generator file (e.g. data/NPCGenerTest.txt); empty disables it")
	teleportPath := flag.String("teleports", cfg.TeleportPath, "server-side portal file")
	networkAdmissionPath := flag.String("network-admission", cfg.NetworkAdmissionPath,
		"server-side VPS/VPN/datacenter network admission policy")
	clientIntegrityPath := flag.String("client-integrity", cfg.ClientIntegrityPath,
		"server-side client integrity probe manifest")
	accDir := flag.String("accounts", cfg.AccountsPath, "account directory")
	guildsPath := flag.String("guilds", cfg.GuildsPath, "guild registry (guilds.json)")
	guildsTxtPath := flag.String("guilds-txt", cfg.GuildsTxtPath, "Guilds.txt exported for the 7.48 client")
	charStatePath := flag.String("charstate", cfg.CharStatePath, "session state directory (buffs/currency)")
	questsPath := flag.String("quests", cfg.QuestsPath, "quest definitions (quests.json)")
	questZonesPath := flag.String("quest_zones", cfg.QuestZonesPath, "area reset zones (quest_zones.json)")
	initItemsPath := flag.String("init_items", cfg.InitItemsPath, "persistent world objects (init_items.csv)")
	bossPath := flag.String("boss", cfg.BossPath, "boss directory (data/boss/*.lua)")
	itemPath := flag.String("items", cfg.ItemPath, "authoritative itemlist.csv")
	itemNamePath := flag.String("itemnames", cfg.ItemNamePath, "authoritative Itemname.csv")
	itemEffectPath := flag.String("itemeffects", cfg.ItemEffectPath, "authoritative ItemEffect.h")
	skillPath := flag.String("skills", cfg.SkillPath, "authoritative SkillData.csv")
	dropRatePath := flag.String("droprates", cfg.DropRatePath, "per-slot drop-rate table")
	volatilePath := flag.String("volatiles", cfg.VolatilePath, "server-side volatile item functions")
	instancesPath := flag.String("instances", cfg.InstancesPath, "server-side instance configuration")
	replictionPath := flag.String("repliction", cfg.ReplictionPath, "native Repliction tables")
	mountPath := flag.String("mounts", cfg.MountPath, "mount attributes by type")
	characterTemplatePath := flag.String("characters", cfg.CharacterTemplatePath, "server-side character creation templates")
	heightMapPath := flag.String("heightmap", cfg.HeightMapPath, "native map HeightMap.dat")
	attributeMapPath := flag.String("attributemap", cfg.AttributeMapPath, "native map AttributeMap.dat")
	debugAddr := flag.String("debug_address", cfg.DebugAddress,
		"loopback diagnostics address (expvar/pprof); empty disables it")
	controlAddr := flag.String("control-address", "", "loopback-only Control API; empty disables it")
	flag.Parse()
	// Bind before loading the World: a bad private listener must fail at boot.
	var controlListener stdnet.Listener
	controlToken := os.Getenv("WYD_CONTROL_TOKEN")
	if *controlAddr != "" {
		if err := control.LoopbackAddress(*controlAddr); err != nil || len(controlToken) < 32 {
			log.Fatal("control-address requires literal loopback and WYD_CONTROL_TOKEN with at least 32 characters")
		}
		controlListener, err = stdnet.Listen("tcp", *controlAddr)
		if err != nil {
			log.Fatal("could not open the private control listener")
		}
		defer controlListener.Close()
	}
	// The flag overrides the file, so repeat the loopback check; otherwise
	// -debug_address 0.0.0.0:6060 would expose pprof publicly.
	if *debugAddr != "" {
		if err := data.ValidateDebugAddress(*debugAddr); err != nil {
			log.Fatalf("debug_address: %v", err)
		}
	}
	log.Printf("configuration loaded from %s", configPath)
	log.Printf("global balance: exp_minimum=%d exp_rate=%d%% party_exp_bonus=%d%%/member",
		cfg.Gameplay.EXPMinimum, cfg.Gameplay.EXPRatePercent,
		cfg.Gameplay.PartyEXPBonusPercent)

	npcs, err := data.LoadNPCs(*npcPath)
	if err != nil {
		log.Fatalf("load NPCs (%s): %v", *npcPath, err)
	}
	log.Printf("%d NPCs loaded from %s", len(npcs), *npcPath)

	geners, err := data.LoadNPCGener(*generPath)
	if err != nil {
		log.Fatalf("load NPCGener (%s): %v", *generPath, err)
	}
	log.Printf("%d generators loaded from %s", len(geners), *generPath)
	if extraPath := strings.TrimSpace(*generExtraPath); extraPath != "" {
		extraGeners, err := data.LoadNPCGener(extraPath)
		if err != nil {
			log.Fatalf("load additional NPCGener (%s): %v", extraPath, err)
		}
		// LoadNPCGener numbers each file from zero, like the native table.
		// Combining two files creates one effective order; reindexing avoids
		// GenerIndex collisions with generators from the base file.
		baseIndex := len(geners)
		for i := range extraGeners {
			extraGeners[i].Index = baseIndex + i
		}
		geners = append(geners, extraGeners...)
		log.Printf("%d additional generators loaded from %s", len(extraGeners), extraPath)
	}

	teleports, err := data.LoadTeleports(*teleportPath)
	if err != nil {
		log.Fatalf("load teleports (%s): %v", *teleportPath, err)
	}
	log.Printf("%d teleports loaded from %s", len(teleports), *teleportPath)

	networkAdmission, err := data.LoadNetworkAdmission(*networkAdmissionPath)
	if err != nil {
		log.Fatalf("load network admission policy (%s): %v", *networkAdmissionPath, err)
	}
	log.Printf("network admission policy: %d ranges loaded", len(networkAdmission.Rules))

	clientIntegrity, err := data.LoadClientIntegrity(*clientIntegrityPath)
	if err != nil {
		log.Fatalf("load client integrity manifest (%s): %v", *clientIntegrityPath, err)
	}
	log.Printf("client integrity: %d probes loaded", len(clientIntegrity.Probes))

	catalog, err := data.LoadCatalog(*itemPath, *itemNamePath, *itemEffectPath, *skillPath)
	if err != nil {
		log.Fatalf("load catalog: %v", err)
	}
	log.Printf("server-side catalog: %d items, %d effects, and %d skills loaded",
		len(catalog.Items), len(catalog.ItemEffects), len(catalog.Skills))

	dropRates, err := data.LoadDropRates(*dropRatePath)
	if err != nil {
		log.Fatalf("load drop rates (%s): %v", *dropRatePath, err)
	}
	log.Printf("per-slot drop table loaded from %s", *dropRatePath)

	volatiles, err := data.LoadVolatilesWithInstances(
		*volatilePath, *instancesPath, catalog.Items, catalog.Skills)
	if err != nil {
		log.Fatalf("load volatile items/instances (%s, %s): %v",
			*volatilePath, *instancesPath, err)
	}
	repliction, err := data.LoadRepliction(*replictionPath, catalog.Items)
	if err != nil {
		log.Fatalf("load Repliction (%s): %v", *replictionPath, err)
	}
	volatiles.Repliction = repliction
	active := 0
	for id := range volatiles.ItemCodes {
		rule, _, _ := volatiles.Rule(id)
		// "generic" has no behavior yet; any other registered action is a
		// gameplay function (restore/gold/teleport/buff/grant_exp/...).
		if rule.Action != "" && rule.Action != "generic" {
			active++
		}
	}
	log.Printf("server-side volatile items: %d items, %d codes, %d items with active functions",
		len(volatiles.ItemCodes), len(volatiles.Codes), active)

	mounts, err := data.LoadMounts(*mountPath)
	if err != nil {
		log.Fatalf("load mounts (%s): %v", *mountPath, err)
	}
	log.Printf("mounts: %d types with stat bonuses (matching g_pMountBonus)", len(mounts.Types))

	characterTemplates, err := data.LoadCharacterTemplates(*characterTemplatePath, catalog.Items)
	if err != nil {
		log.Fatalf("load character templates (%s): %v", *characterTemplatePath, err)
	}
	log.Printf("%d character templates loaded; spawn at (%d,%d)",
		len(characterTemplates.Classes), characterTemplates.Spawn.X, characterTemplates.Spawn.Y)

	terrain, err := data.LoadTerrain(*heightMapPath, *attributeMapPath)
	if err != nil {
		log.Fatalf("load terrain: %v", err)
	}
	log.Printf("terrain maps loaded: %dx%d heights and %dx%d attributes",
		4096, 4096, 1024, 1024)

	quests, err := data.LoadQuests(*questsPath)
	if err != nil {
		log.Fatalf("load quests: %v", err)
	}

	questZones, err := data.LoadQuestZones(*questZonesPath)
	if err != nil {
		log.Fatalf("load quest zones: %v", err)
	}

	bosses, err := data.LoadBossCatalog(*bossPath)
	if err != nil {
		log.Fatalf("load bosses: %v", err)
	}
	log.Printf("%d bosses loaded from %s", len(bosses.Bosses), *bossPath)

	initItems, err := data.LoadInitItems(*initItemsPath, catalog.Items)
	if err != nil {
		log.Fatalf("load world objects: %v", err)
	}

	var st store.Store
	var postgresStore *store.PostgresStore
	switch cfg.DatabaseDriver {
	case "postgres":
		databaseURL := cfg.DatabaseURL
		if databaseURL == "" {
			databaseURL = os.Getenv(cfg.DatabaseURLEnv)
		}
		if databaseURL == "" {
			log.Fatalf("PostgreSQL is configured, but %s is empty", cfg.DatabaseURLEnv)
		}
		postgresStore, err = store.NewPostgresStore(context.Background(), store.PostgresConfig{
			URL: databaseURL, MaxConns: int32(cfg.DatabaseMaxConns), GuildsTxtPath: *guildsTxtPath,
			OperationTimeout: time.Duration(cfg.CriticalPersistenceTimeoutMS) * time.Millisecond,
		})
		if err != nil {
			log.Fatalf("open PostgreSQL: %v", err)
		}
		defer postgresStore.Close()
		st = postgresStore
		log.Printf("authoritative persistence: PostgreSQL (maximum pool size=%d)", cfg.DatabaseMaxConns)
	case "json":
		st = store.NewJSONStore(*accDir, store.WithGuildsPath(*guildsPath),
			store.WithGuildsTxtPath(*guildsTxtPath), store.WithCharStatePath(*charStatePath))
		log.Printf("development persistence: JSON at %s", *accDir)
	default:
		log.Fatalf("unknown database_driver %q", cfg.DatabaseDriver)
	}
	worldOptions := []game.WorldOption{
		game.WithNPCGenerLog(cfg.NPCGenerLog),
		game.WithGameplayLog(cfg.GameplayLog),
		game.WithTeleports(teleports), game.WithGameplayConfig(cfg.Gameplay),
		game.WithGuildWars(cfg.GuildWars),
		game.WithNetworkAdmission(networkAdmission),
		game.WithClientIntegrity(clientIntegrity),
		game.WithOperationalConfig(game.OperationalConfig{
			AuthAttemptsPerMinuteIP:      int(cfg.AuthAttemptsPerMinIP),
			AuthAttemptsPerMinuteAccount: int(cfg.AuthAttemptsPerMinAccount),
			MaxAuthenticatedClientsPerIP: int(cfg.MaxAuthenticatedClientsPerIP),
			AuthHashConcurrency:          int(cfg.AuthHashConcurrency),
			WorldCommandQueueCapacity:    int(cfg.WorldCommandQueueCapacity),
			ChatLocalPer10Seconds:        int(cfg.ChatLocalPer10Secs),
			ChatWhisperPer10Seconds:      int(cfg.ChatWhisperPer10Secs),
			ChatGlobalPer10Seconds:       int(cfg.ChatGlobalPer10Secs),
			ChannelID:                    byte(cfg.ChannelID),
		}),
		game.WithQuests(quests), game.WithQuestZones(questZones), game.WithMounts(mounts),
		game.WithBossCatalog(bosses), game.WithInitItems(initItems),
		game.WithLoadtestSpawn(cfg.LoadtestSpawn, cfg.LoadtestAccountPrefix),
	}
	if uxmal, ok := volatiles.Instances["uxmal"]; ok && uxmal.Uxmal != nil {
		worldOptions = append(worldOptions, game.WithUxmal(uxmal))
		log.Printf("Uxmal loaded: %d rooms, ticket=%d", len(uxmal.Stages), uxmal.Uxmal.TicketItem)
	}
	world, err := game.NewWorld(st, npcs, geners, catalog, dropRates, volatiles,
		characterTemplates, terrain, worldOptions...)
	if err != nil {
		log.Fatalf("create world: %v", err)
	}
	go world.Run()
	stopWebAdmin, webErr := startWebAdmin(cfg, world, postgresStore)
	if webErr != nil {
		log.Printf("staff panel failed to start: %v; game server remains active", webErr)
		stopWebAdmin = func() {}
	}
	defer stopWebAdmin()
	if controlListener != nil {
		handler, err := control.NewHandler(world, controlToken)
		if err != nil {
			log.Fatal("invalid Control API configuration")
		}
		controlServer := &stdhttp.Server{Handler: handler, ReadHeaderTimeout: 2 * time.Second, ReadTimeout: 3 * time.Second, WriteTimeout: 4 * time.Second, IdleTimeout: 30 * time.Second, MaxHeaderBytes: 8192}
		go func() {
			log.Printf("private Control API at %s (read-only)", *controlAddr)
			if err := controlServer.Serve(controlListener); err != nil && err != stdhttp.ErrServerClosed {
				log.Print("Control API unavailable")
			}
		}()
	}

	if *debugAddr != "" {
		go serveDebug(*debugAddr)
	}

	// SIGTERM (systemd/deploy) and SIGINT (Ctrl+C) persist state before exit.
	// Otherwise queued autosaves are discarded and players return to stale state.
	signals := make(chan os.Signal, 1)
	signal.Notify(signals, syscall.SIGTERM, syscall.SIGINT)
	go func() {
		sig := <-signals
		log.Printf("received signal %v: persisting state before exit", sig)
		stopWebAdmin()
		if world.Shutdown(shutdownTimeout) {
			log.Print("shutdown complete")
			os.Exit(0)
		}
		// Shutdown already logged the incomplete drain; a nonzero exit status
		// makes the failure visible to systemd.
		os.Exit(1)
	}()

	listenerConfig := net.ListenerConfig{
		OutputQueueSize:      int(cfg.SessionQueueCapacity),
		MaxConnections:       int(cfg.MaxConnections),
		MaxConnectionsPerIP:  int(cfg.MaxConnectionsPerIP),
		HandshakeTimeout:     time.Duration(cfg.HandshakeTimeoutSecs) * time.Second,
		SessionIdleTimeout:   time.Duration(cfg.SessionIdleTimeoutSecs) * time.Second,
		FrameReadTimeout:     time.Duration(cfg.FrameReadTimeoutSecs) * time.Second,
		InboundPacketsPerSec: int(cfg.InboundPacketsPerSec),
		InboundBytesPerSec:   int(cfg.InboundBytesPerSec),
	}
	if err := net.ListenWithConfig(*addr, listenerConfig, func(s *net.Session) { s.Serve(world.Enqueue) }); err != nil {
		log.Fatalf("listen: %v", err)
	}
}
