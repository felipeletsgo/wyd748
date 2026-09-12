// loadtest prepara a base PostgreSQL descartavel e gera sessoes WYD 7.48.
// Nunca usa a conta fonte como bot: felipe permanece livre para observacao.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/data"
	"wydgo/internal/loadtest"
	"wydgo/internal/store"
)

func main() {
	databaseEnv := flag.String("database-url-env", "WYD_LOADTEST_DATABASE_URL",
		"variavel com a URL da base PostgreSQL descartavel")
	sourceDatabaseEnv := flag.String("source-database-url-env", "",
		"opcional: variavel com a URL da base que contem a conta modelo")
	server := flag.String("server", "127.0.0.1:8281", "endereco do TMSrv")
	sourceAccount := flag.String("source-account", "felipe", "conta modelo")
	sourceCharacter := flag.String("source-character", "", "personagem modelo; vazio = qualquer nome")
	sourceSlot := flag.Int("source-slot", 1,
		"slot exato do personagem modelo; 1 = segundo personagem (Arch), -1 = selecao legada")
	accountPrefix := flag.String("account-prefix", "bot", "prefixo das contas bot (bot0001)")
	characterPrefix := flag.String("character-prefix", "Bot", "prefixo alfabetico dos nicknames")
	botPasswordEnv := flag.String("bot-password-env", "WYD_LOADTEST_BOT_PASSWORD",
		"variavel com a senha comum dos bots")
	bots := flag.Int("bots", 990, "quantidade de bots (maximo 990)")
	ramp := flag.Duration("ramp", time.Minute, "tempo para conectar todos os bots")
	duration := flag.Duration("duration", 5*time.Minute, "duracao do teste")
	skill := flag.Int("skill", -1, "indice local de skill; -1 = ataque fisico")
	movePercent := flag.Int("move-percent", 50,
		"percentual dos ciclos que tentam movimento; o ataque continua sendo enviado")
	heightMap := flag.String("heightmap", "data/maps/HeightMap.dat", "mapa de alturas usado para escolher passos validos")
	attributeMap := flag.String("attributemap", "data/maps/AttributeMap.dat", "mapa de atributos usado para escolher passos validos")
	provision := flag.Bool("provision", false, "criar as contas na base de teste")
	reset := flag.Bool("reset", false, "apagar somente bot0001..bot0990 antes de provisionar")
	provisionOnly := flag.Bool("provision-only", false, "provisionar e nao abrir sessoes")
	flag.Parse()
	if *bots < 1 || *bots > 990 {
		log.Fatalf("bots deve ficar entre 1 e 990")
	}
	if *ramp <= 0 || *duration <= 0 {
		log.Fatalf("ramp e duration devem ser positivos")
	}
	if *sourceSlot < -1 {
		log.Fatalf("source-slot deve ser -1 ou um slot nao negativo")
	}
	terrain, err := data.LoadTerrain(*heightMap, *attributeMap)
	if err != nil {
		log.Fatalf("carregar mapas para movimento do loadtest: %v", err)
	}

	password := os.Getenv(*botPasswordEnv)
	if *provision || !*provisionOnly {
		if password == "" {
			log.Fatalf("a variavel %s esta vazia", *botPasswordEnv)
		}
		if !account.ValidPassword(password) {
			log.Fatalf("a variavel %s deve conter 4 a 12 caracteres ASCII visiveis", *botPasswordEnv)
		}
	}
	databaseURL := os.Getenv(*databaseEnv)
	if databaseURL == "" {
		log.Fatalf("a variavel %s esta vazia", *databaseEnv)
	}
	sourceDatabaseURL := databaseURL
	if *sourceDatabaseEnv != "" {
		sourceDatabaseURL = os.Getenv(*sourceDatabaseEnv)
		if sourceDatabaseURL == "" {
			log.Fatalf("a variavel %s esta vazia", *sourceDatabaseEnv)
		}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancel()
	st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: databaseURL, MaxConns: 32})
	if err != nil {
		log.Fatalf("abrir PostgreSQL de carga: %v", err)
	}
	var sourceStore *store.PostgresStore
	if sourceDatabaseURL != databaseURL {
		sourceStore, err = store.NewPostgresReadOnlyStore(ctx, store.PostgresConfig{URL: sourceDatabaseURL, MaxConns: 4})
		if err != nil {
			st.Close()
			log.Fatalf("abrir PostgreSQL do modelo: %v", err)
		}
	}
	if *reset {
		removed, err := st.ResetLoadtestAccounts(ctx, *accountPrefix)
		if err != nil {
			if sourceStore != nil {
				sourceStore.Close()
			}
			st.Close()
			log.Fatalf("reset dos bots: %v", err)
		}
		log.Printf("loadtest: %d contas %q removidas", removed, *accountPrefix)
	}
	if *provision {
		source := loadtest.AccountStore(st)
		if sourceStore != nil {
			source = sourceStore
		}
		result, err := loadtest.ProvisionBotsFromSource(st, source, loadtest.ProvisionConfig{
			SourceAccount: *sourceAccount, SourceCharacter: *sourceCharacter,
			SourceSlot: sourceSlot,
			BotCount:   *bots, AccountPrefix: *accountPrefix,
			CharacterPrefix: *characterPrefix, BotPassword: password,
		})
		if err != nil {
			if sourceStore != nil {
				sourceStore.Close()
			}
			st.Close()
			log.Fatalf("provisionar bots: %v", err)
		}
		log.Printf("loadtest: modelo_materializado=%t provisionados=%d ignorados=%d",
			result.SourceCreated, result.Created, result.Skipped)
	}
	if sourceStore != nil {
		sourceStore.Close()
	}
	st.Close()
	if *provisionOnly {
		return
	}
	report := loadtest.RunBots(context.Background(), loadtest.RunConfig{
		Address: *server, BotCount: *bots, AccountPrefix: *accountPrefix,
		Password: password, Ramp: *ramp, Duration: *duration, Skill: *skill,
		MovePercent: *movePercent,
		Terrain:     terrain,
	}, log.Default())
	encoded, _ := json.MarshalIndent(report, "", "  ")
	fmt.Println(string(encoded))
}
