// Comando server -- servidor WYD 7.48 nativo em Go.
//
// Entry fino: carrega dados estaticos + store, sobe o World (game loop) numa
// goroutine e abre o listener, servindo cada conexao pro loop.
package main

import (
	"flag"
	"log"
	"os"
	"strings"

	"wydgo/internal/data"
	"wydgo/internal/game"
	"wydgo/internal/net"
	"wydgo/internal/store"
)

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
	teleportPath := flag.String("teleports", cfg.TeleportPath, "arquivo server-side de portais")
	accDir := flag.String("accounts", cfg.AccountsPath, "diretorio de contas")
	guildsPath := flag.String("guilds", cfg.GuildsPath, "registro de guilds (guilds.json)")
	guildsTxtPath := flag.String("guilds-txt", cfg.GuildsTxtPath, "Guilds.txt exportado para o client 7.48")
	charStatePath := flag.String("charstate", cfg.CharStatePath, "pasta do estado de sessao (buffs/moedas)")
	questsPath := flag.String("quests", cfg.QuestsPath, "definicoes de quest (quests.json)")
	itemPath := flag.String("items", cfg.ItemPath, "itemlist.csv autoritativo")
	itemNamePath := flag.String("itemnames", cfg.ItemNamePath, "Itemname.csv autoritativo")
	skillPath := flag.String("skills", cfg.SkillPath, "SkillData.csv autoritativo")
	dropRatePath := flag.String("droprates", cfg.DropRatePath, "tabela de drop rate por slot")
	volatilePath := flag.String("volatiles", cfg.VolatilePath, "funcoes server-side dos itens volatile")
	mountPath := flag.String("mounts", cfg.MountPath, "atributos das montarias por tipo")
	characterTemplatePath := flag.String("characters", cfg.CharacterTemplatePath, "layouts server-side para criacao de personagem")
	heightMapPath := flag.String("heightmap", cfg.HeightMapPath, "HeightMap.dat nativo do mapa")
	attributeMapPath := flag.String("attributemap", cfg.AttributeMapPath, "AttributeMap.dat nativo do mapa")
	flag.Parse()
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

	teleports, err := data.LoadTeleports(*teleportPath)
	if err != nil {
		log.Fatalf("carregar teleportes (%s): %v", *teleportPath, err)
	}
	log.Printf("%d teleportes carregados de %s", len(teleports), *teleportPath)

	catalog, err := data.LoadCatalog(*itemPath, *itemNamePath, *skillPath)
	if err != nil {
		log.Fatalf("carregar catalogo: %v", err)
	}
	log.Printf("catalogo server-side: %d itens e %d skills carregados", len(catalog.Items), len(catalog.Skills))

	dropRates, err := data.LoadDropRates(*dropRatePath)
	if err != nil {
		log.Fatalf("carregar drop rates (%s): %v", *dropRatePath, err)
	}
	log.Printf("tabela de drop por slot carregada de %s", *dropRatePath)

	volatiles, err := data.LoadVolatiles(*volatilePath, catalog.Items)
	if err != nil {
		log.Fatalf("carregar volatiles (%s): %v", *volatilePath, err)
	}
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

	st := store.NewJSONStore(*accDir, store.WithGuildsPath(*guildsPath),
		store.WithGuildsTxtPath(*guildsTxtPath), store.WithCharStatePath(*charStatePath))
	world, err := game.NewWorld(st, npcs, geners, catalog, dropRates, volatiles,
		characterTemplates, terrain, game.WithNPCGenerLog(cfg.NPCGenerLog),
		game.WithTeleports(teleports), game.WithGameplayConfig(cfg.Gameplay),
		game.WithQuests(quests), game.WithMounts(mounts))
	if err != nil {
		log.Fatalf("criar mundo: %v", err)
	}
	go world.Run()

	if err := net.Listen(*addr, func(s *net.Session) { s.Serve(world.Enqueue) }); err != nil {
		log.Fatalf("listen: %v", err)
	}
}
