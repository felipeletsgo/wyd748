// loadtest prepares a disposable PostgreSQL database and starts WYD 7.48 sessions.
// It never uses the source account as a bot, leaving felipe available for observation.
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
		"environment variable containing the disposable PostgreSQL database URL")
	sourceDatabaseEnv := flag.String("source-database-url-env", "",
		"optional environment variable for the database containing the source account")
	server := flag.String("server", "127.0.0.1:8281", "TMSrv address")
	sourceAccount := flag.String("source-account", "felipe", "source account")
	sourceCharacter := flag.String("source-character", "", "source character; empty selects any name")
	sourceSlot := flag.Int("source-slot", 1,
		"exact source character slot; 1 = second character (Arch), -1 = legacy selection")
	accountPrefix := flag.String("account-prefix", "bot", "bot account prefix (bot0001)")
	characterPrefix := flag.String("character-prefix", "Bot", "alphabetic prefix for character names")
	botPasswordEnv := flag.String("bot-password-env", "WYD_LOADTEST_BOT_PASSWORD",
		"environment variable containing the shared bot password")
	bots := flag.Int("bots", 990, "number of bots (maximum 990)")
	ramp := flag.Duration("ramp", time.Minute, "time to connect all bots")
	duration := flag.Duration("duration", 5*time.Minute, "test duration")
	skill := flag.Int("skill", -1, "local skill index; -1 = physical attack")
	movePercent := flag.Int("move-percent", 50,
		"percentage of cycles attempting movement; attacks are still sent")
	heightMap := flag.String("heightmap", "data/maps/HeightMap.dat", "height map used to choose valid steps")
	attributeMap := flag.String("attributemap", "data/maps/AttributeMap.dat", "attribute map used to choose valid steps")
	provision := flag.Bool("provision", false, "create accounts in the test database")
	reset := flag.Bool("reset", false, "delete only bot0001..bot0990 before provisioning")
	provisionOnly := flag.Bool("provision-only", false, "provision without opening sessions")
	flag.Parse()
	if *bots < 1 || *bots > 990 {
		log.Fatalf("bots must be between 1 and 990")
	}
	if *ramp <= 0 || *duration <= 0 {
		log.Fatalf("ramp and duration must be positive")
	}
	if *sourceSlot < -1 {
		log.Fatalf("source-slot must be -1 or a nonnegative slot")
	}
	terrain, err := data.LoadTerrain(*heightMap, *attributeMap)
	if err != nil {
		log.Fatalf("load terrain maps for load test movement: %v", err)
	}

	password := os.Getenv(*botPasswordEnv)
	if *provision || !*provisionOnly {
		if password == "" {
			log.Fatalf("environment variable %s is empty", *botPasswordEnv)
		}
		if !account.ValidPassword(password) {
			log.Fatalf("environment variable %s must contain 4 to 10 visible ASCII characters", *botPasswordEnv)
		}
	}
	databaseURL := os.Getenv(*databaseEnv)
	if databaseURL == "" {
		log.Fatalf("environment variable %s is empty", *databaseEnv)
	}
	sourceDatabaseURL := databaseURL
	if *sourceDatabaseEnv != "" {
		sourceDatabaseURL = os.Getenv(*sourceDatabaseEnv)
		if sourceDatabaseURL == "" {
			log.Fatalf("environment variable %s is empty", *sourceDatabaseEnv)
		}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancel()
	st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: databaseURL, MaxConns: 32})
	if err != nil {
		log.Fatalf("open load-test PostgreSQL database: %v", err)
	}
	var sourceStore *store.PostgresStore
	if sourceDatabaseURL != databaseURL {
		sourceStore, err = store.NewPostgresReadOnlyStore(ctx, store.PostgresConfig{URL: sourceDatabaseURL, MaxConns: 4})
		if err != nil {
			st.Close()
			log.Fatalf("open source PostgreSQL database: %v", err)
		}
	}
	if *reset {
		removed, err := st.ResetLoadtestAccounts(ctx, *accountPrefix)
		if err != nil {
			if sourceStore != nil {
				sourceStore.Close()
			}
			st.Close()
			log.Fatalf("reset bot accounts: %v", err)
		}
		log.Printf("loadtest: %d accounts with prefix %q removed", removed, *accountPrefix)
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
			log.Fatalf("provision bots: %v", err)
		}
		log.Printf("loadtest: source_created=%t created=%d skipped=%d",
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
