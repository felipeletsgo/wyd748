// Comando server -- servidor WYD 7.48 nativo em Go.
//
// Entry fino: carrega dados estaticos + store, sobe o World (game loop) numa
// goroutine e abre o listener, servindo cada conexao pro loop.
package main

import (
	"context"
	"flag"
	"log"
	stdhttp "net/http"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	// Registra /debug/pprof no mux padrao. So fica acessivel se debug_address
	// estiver configurado, e a configuracao exige loopback.
	_ "net/http/pprof"

	"wydgo/internal/data"
	"wydgo/internal/game"
	"wydgo/internal/net"
	"wydgo/internal/store"
)

// shutdownTimeout limita a persistencia final. Generoso o bastante para gravar
// centenas de contas, curto o bastante para o systemd nao matar o processo
// antes (o padrao do TimeoutStopSec e 90 s).
const shutdownTimeout = 20 * time.Second

// serveDebug sobe expvar (/debug/vars) e pprof (/debug/pprof) em loopback. A
// validacao de que o endereco NAO e publico fica em data.LoadServerConfig, que
// derruba o boot em vez de expor o diagnostico.
func serveDebug(address string) {
	log.Printf("diagnostico em http://%s/debug/vars e /debug/pprof (somente loopback)", address)
	server := &stdhttp.Server{
		Addr:              address,
		ReadHeaderTimeout: 5 * time.Second,
	}
	if err := server.ListenAndServe(); err != nil {
		log.Printf("diagnostico: %v", err)
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
	flag.String("config", configPath, "arquivo texto de configuracao")
	addr := flag.String("addr", cfg.ListenAddress, "endereco de escuta (host:porta)")
	npcPath := flag.String("npcs", cfg.NPCPath, "pasta de NPCs (um .json por NPC)")
	generPath := flag.String("gener", cfg.GeneratorPath, "arquivo padrao de spawn NPCGener.txt")
	generExtraPath := flag.String("gener-extra", "data/NPCGenerTest.txt",
		"arquivo adicional de geradores para lojas/fixtures de teste; vazio desliga")
	teleportPath := flag.String("teleports", cfg.TeleportPath, "arquivo server-side de portais")
	networkAdmissionPath := flag.String("network-admission", cfg.NetworkAdmissionPath,
		"politica server-side de redes VPS/VPN/datacenter")
	accDir := flag.String("accounts", cfg.AccountsPath, "diretorio de contas")
	guildsPath := flag.String("guilds", cfg.GuildsPath, "registro de guilds (guilds.json)")
	guildsTxtPath := flag.String("guilds-txt", cfg.GuildsTxtPath, "Guilds.txt exportado para o client 7.48")
	charStatePath := flag.String("charstate", cfg.CharStatePath, "pasta do estado de sessao (buffs/moedas)")
	questsPath := flag.String("quests", cfg.QuestsPath, "definicoes de quest (quests.json)")
	questZonesPath := flag.String("quest_zones", cfg.QuestZonesPath, "zonas de reset de area (quest_zones.json)")
	initItemsPath := flag.String("init_items", cfg.InitItemsPath, "objetos permanentes do mundo (init_items.csv)")
	bossPath := flag.String("boss", cfg.BossPath, "diretorio dos bosses (data/boss/*.lua)")
	itemPath := flag.String("items", cfg.ItemPath, "itemlist.csv autoritativo")
	itemNamePath := flag.String("itemnames", cfg.ItemNamePath, "Itemname.csv autoritativo")
	itemEffectPath := flag.String("itemeffects", cfg.ItemEffectPath, "ItemEffect.h autoritativo")
	skillPath := flag.String("skills", cfg.SkillPath, "SkillData.csv autoritativo")
	dropRatePath := flag.String("droprates", cfg.DropRatePath, "tabela de drop rate por slot")
	volatilePath := flag.String("volatiles", cfg.VolatilePath, "funcoes server-side dos itens volatile")
	replictionPath := flag.String("repliction", cfg.ReplictionPath, "tabelas nativas do Repliction")
	mountPath := flag.String("mounts", cfg.MountPath, "atributos das montarias por tipo")
	characterTemplatePath := flag.String("characters", cfg.CharacterTemplatePath, "layouts server-side para criacao de personagem")
	heightMapPath := flag.String("heightmap", cfg.HeightMapPath, "HeightMap.dat nativo do mapa")
	attributeMapPath := flag.String("attributemap", cfg.AttributeMapPath, "AttributeMap.dat nativo do mapa")
	debugAddr := flag.String("debug_address", cfg.DebugAddress,
		"endereco loopback do diagnostico (expvar/pprof); vazio desliga")
	flag.Parse()
	// A flag sobrescreve o arquivo, entao repete a checagem de loopback: sem
	// isso, -debug_address 0.0.0.0:6060 exporia pprof publicamente.
	if *debugAddr != "" {
		if err := data.ValidateDebugAddress(*debugAddr); err != nil {
			log.Fatalf("debug_address: %v", err)
		}
	}
	log.Printf("configuracao carregada de %s", configPath)
	log.Printf("balanceamento global: exp_minimum=%d exp_rate=%d%% party_exp_bonus=%d%%/membro",
		cfg.Gameplay.EXPMinimum, cfg.Gameplay.EXPRatePercent,
		cfg.Gameplay.PartyEXPBonusPercent)

	npcs, err := data.LoadNPCs(*npcPath)
	if err != nil {
		log.Fatalf("carregar NPCs (%s): %v", *npcPath, err)
	}
	log.Printf("%d NPCs carregados de %s", len(npcs), *npcPath)

	geners, err := data.LoadNPCGener(*generPath)
	if err != nil {
		log.Fatalf("carregar NPCGener (%s): %v", *generPath, err)
	}
	log.Printf("%d geradores carregados de %s", len(geners), *generPath)
	if extraPath := strings.TrimSpace(*generExtraPath); extraPath != "" {
		extraGeners, err := data.LoadNPCGener(extraPath)
		if err != nil {
			log.Fatalf("carregar NPCGener adicional (%s): %v", extraPath, err)
		}
		// LoadNPCGener numera cada arquivo a partir de zero, como a tabela nativa.
		// Ao compor dois arquivos, a ordem efetiva passa a ser a lista combinada:
		// reindexar evita colisao de GenerIndex com os geradores do arquivo base.
		baseIndex := len(geners)
		for i := range extraGeners {
			extraGeners[i].Index = baseIndex + i
		}
		geners = append(geners, extraGeners...)
		log.Printf("%d geradores adicionais carregados de %s", len(extraGeners), extraPath)
	}

	teleports, err := data.LoadTeleports(*teleportPath)
	if err != nil {
		log.Fatalf("carregar teleportes (%s): %v", *teleportPath, err)
	}
	log.Printf("%d teleportes carregados de %s", len(teleports), *teleportPath)

	networkAdmission, err := data.LoadNetworkAdmission(*networkAdmissionPath)
	if err != nil {
		log.Fatalf("carregar politica de admissao de rede (%s): %v", *networkAdmissionPath, err)
	}
	log.Printf("politica de admissao de rede: %d faixa(s) carregada(s)", len(networkAdmission.Rules))

	catalog, err := data.LoadCatalog(*itemPath, *itemNamePath, *itemEffectPath, *skillPath)
	if err != nil {
		log.Fatalf("carregar catalogo: %v", err)
	}
	log.Printf("catalogo server-side: %d itens, %d efeitos e %d skills carregados",
		len(catalog.Items), len(catalog.ItemEffects), len(catalog.Skills))

	dropRates, err := data.LoadDropRates(*dropRatePath)
	if err != nil {
		log.Fatalf("carregar drop rates (%s): %v", *dropRatePath, err)
	}
	log.Printf("tabela de drop por slot carregada de %s", *dropRatePath)

	volatiles, err := data.LoadVolatiles(*volatilePath, catalog.Items, catalog.Skills)
	if err != nil {
		log.Fatalf("carregar volatiles (%s): %v", *volatilePath, err)
	}
	repliction, err := data.LoadRepliction(*replictionPath, catalog.Items)
	if err != nil {
		log.Fatalf("carregar repliction (%s): %v", *replictionPath, err)
	}
	volatiles.Repliction = repliction
	active := 0
	for id := range volatiles.ItemCodes {
		rule, _, _ := volatiles.Rule(id)
		// "generic" ainda nao tem comportamento; qualquer outra acao registrada e
		// uma funcao de jogo real (restore/gold/teleport/buff/grant_exp/...).
		if rule.Action != "" && rule.Action != "generic" {
			active++
		}
	}
	log.Printf("volatiles server-side: %d itens, %d codigos, %d itens com funcao ativa",
		len(volatiles.ItemCodes), len(volatiles.Codes), active)

	mounts, err := data.LoadMounts(*mountPath)
	if err != nil {
		log.Fatalf("carregar montarias (%s): %v", *mountPath, err)
	}
	log.Printf("montarias: %d tipos com bonus de stat (fiel ao g_pMountBonus)", len(mounts.Types))

	characterTemplates, err := data.LoadCharacterTemplates(*characterTemplatePath, catalog.Items)
	if err != nil {
		log.Fatalf("carregar layouts de personagem (%s): %v", *characterTemplatePath, err)
	}
	log.Printf("%d layouts de personagem carregados; nascimento em (%d,%d)",
		len(characterTemplates.Classes), characterTemplates.Spawn.X, characterTemplates.Spawn.Y)

	terrain, err := data.LoadTerrain(*heightMapPath, *attributeMapPath)
	if err != nil {
		log.Fatalf("carregar terreno: %v", err)
	}
	log.Printf("mapas de terreno carregados: %dx%d alturas e %dx%d atributos",
		4096, 4096, 1024, 1024)

	quests, err := data.LoadQuests(*questsPath)
	if err != nil {
		log.Fatalf("carregar quests: %v", err)
	}

	questZones, err := data.LoadQuestZones(*questZonesPath)
	if err != nil {
		log.Fatalf("carregar zonas de quest: %v", err)
	}

	bosses, err := data.LoadBossCatalog(*bossPath)
	if err != nil {
		log.Fatalf("carregar bosses: %v", err)
	}
	log.Printf("%d bosses carregados de %s", len(bosses.Bosses), *bossPath)

	initItems, err := data.LoadInitItems(*initItemsPath, catalog.Items)
	if err != nil {
		log.Fatalf("carregar objetos de mundo: %v", err)
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
			log.Fatalf("PostgreSQL configurado, mas %s esta vazia", cfg.DatabaseURLEnv)
		}
		postgresStore, err = store.NewPostgresStore(context.Background(), store.PostgresConfig{
			URL: databaseURL, MaxConns: int32(cfg.DatabaseMaxConns), GuildsTxtPath: *guildsTxtPath,
			OperationTimeout: time.Duration(cfg.CriticalPersistenceTimeoutMS) * time.Millisecond,
		})
		if err != nil {
			log.Fatalf("abrir PostgreSQL: %v", err)
		}
		defer postgresStore.Close()
		st = postgresStore
		log.Printf("persistencia autoritativa: PostgreSQL (pool maximo=%d)", cfg.DatabaseMaxConns)
	case "json":
		st = store.NewJSONStore(*accDir, store.WithGuildsPath(*guildsPath),
			store.WithGuildsTxtPath(*guildsTxtPath), store.WithCharStatePath(*charStatePath))
		log.Printf("persistencia de desenvolvimento: JSON em %s", *accDir)
	default:
		log.Fatalf("database_driver desconhecido %q", cfg.DatabaseDriver)
	}
	worldOptions := []game.WorldOption{
		game.WithNPCGenerLog(cfg.NPCGenerLog),
		game.WithGameplayLog(cfg.GameplayLog),
		game.WithTeleports(teleports), game.WithGameplayConfig(cfg.Gameplay),
		game.WithNetworkAdmission(networkAdmission),
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
		log.Printf("Uxmal carregado: %d salas, ticket=%d", len(uxmal.Stages), uxmal.Uxmal.TicketItem)
	}
	world, err := game.NewWorld(st, npcs, geners, catalog, dropRates, volatiles,
		characterTemplates, terrain, worldOptions...)
	if err != nil {
		log.Fatalf("criar mundo: %v", err)
	}
	go world.Run()

	if *debugAddr != "" {
		go serveDebug(*debugAddr)
	}

	// SIGTERM (systemd/deploy) e SIGINT (Ctrl+C) persistem antes de sair. Sem
	// isso o que estiver na fila de autosave e descartado e o jogador volta com
	// estado velho.
	signals := make(chan os.Signal, 1)
	signal.Notify(signals, syscall.SIGTERM, syscall.SIGINT)
	go func() {
		sig := <-signals
		log.Printf("sinal %v recebido: persistindo estado antes de sair", sig)
		if world.Shutdown(shutdownTimeout) {
			log.Print("desligamento concluido")
			os.Exit(0)
		}
		// Drain incompleto ja foi logado por Shutdown; sair com codigo != 0
		// deixa isso visivel no systemd.
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
