package game

import (
	"context"
	"os"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// Run with -p 1: these fixtures temporarily retain the isolated guild singleton.
func TestCriticalGuildCreatePostgresCommitAndRejection(t *testing.T) {
	criticalGuildCommandPostgres(t, "create")
}

func TestCriticalGuildJoinPostgresCommitAndRejection(t *testing.T) {
	for _, operation := range []string{"accept", "recruit"} {
		t.Run(operation, func(t *testing.T) { criticalGuildCommandPostgres(t, operation) })
	}
}

func criticalGuildCommandPostgres(t *testing.T, operation string) {
	t.Helper()
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	for _, reject := range []bool{false, true} {
		name := "commit"
		if reject {
			name = "ledger-rejection"
		}
		t.Run(name, func(t *testing.T) {
			ctx := context.Background()
			st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 2})
			if err != nil {
				t.Fatal(err)
			}
			t.Cleanup(st.Close)
			conn, err := pgx.Connect(ctx, url)
			if err != nil {
				t.Fatal(err)
			}
			t.Cleanup(func() { _ = conn.Close(ctx) })
			var previous []byte
			var version int64
			if err := conn.QueryRow(ctx, `SELECT payload,version FROM guild_state WHERE singleton=true`).Scan(&previous, &version); err != nil && err != pgx.ErrNoRows {
				t.Fatal(err)
			}
			t.Cleanup(func() {
				var err error
				if previous == nil {
					_, err = conn.Exec(ctx, `DELETE FROM guild_state WHERE singleton=true`)
				} else {
					_, err = conn.Exec(ctx, `UPDATE guild_state SET payload=$1,version=$2 WHERE singleton=true`, previous, version)
				}
				if err != nil {
					t.Errorf("restore isolated guild aggregate: %v", err)
				}
			})
			w, p, fallback := handlerTestWorld(t)
			w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
			p.Char.GuildID, p.Char.GuildRank = 0, 0
			w.warConfig.Enabled = false
			w.clock = newFakeClock(time.Date(2026, 10, 5, 12, 0, 0, 0, time.UTC))
			var target *Player
			if operation != "create" {
				configureCriticalTreasury(w, p, 100)
				p.Char.Gold = guildInviteCost + 100
				target, _ = networkedTestPlayer(2, "JoinPG", p.X, p.Y)
				w.players[target.Session], w.playersByID[target.ID] = target, target
				w.updatePlayerSpatial(target)
				target.GuildInviteFrom, target.GuildInviteUntil = 1, w.now().Add(guildInviteTTL)
			}
			suffix := time.Now().UTC().Format("150405.000000000")
			p.Account.Name, p.Char.Name = "guildcreatepg"+suffix, "CreatePG"
			if target != nil {
				// Keep canonical membership aligned with the fixture's unique names.
				w.guilds.Guilds[0].Members[0].Character = p.Char.Name
				w.guilds.Guilds[0].Members[0].Account = p.Account.Name
			}
			delete(w.playersByCharacterUID, p.Char.UID)
			p.Char.UID = ""
			players := []*Player{p}
			if target != nil {
				target.Account.Name, target.Char.UID = "guildjoinpg"+suffix, ""
				players = append(players, target)
			}
			foreign := &model.Account{Name: "guildforeign" + suffix, PasswordHash: "hash", Chars: []model.Char{{
				Name: "ForeignCreate", Score: &model.Score{Version: model.ScoreVersion}, Inv: [64]model.Item{{Index: 4011}},
			}}}
			t.Cleanup(func() {
				keys := []string{strings.ToLower(p.Account.Name), strings.ToLower(foreign.Name)}
				if target != nil {
					keys = append(keys, strings.ToLower(target.Account.Name))
				}
				if _, err := conn.Exec(ctx, `DELETE FROM accounts WHERE name_key=ANY($1)`,
					keys); err != nil {
					t.Errorf("remove isolated guild command accounts: %v", err)
				}
			})
			seedAccounts := []*model.Account{p.Account, foreign}
			if target != nil {
				seedAccounts = append(seedAccounts, target.Account)
			}
			for _, account := range seedAccounts {
				if err := st.CreateAccount(account); err != nil {
					t.Fatal(err)
				}
			}
			for _, player := range players {
				w.indexPlayerCharacter(player)
				if !w.claimAccountSession(player.Session, player.Account.Name) {
					t.Fatal("PostgreSQL guild command account admission failed")
				}
			}
			retained, before := w.criticalGuildSnapshot(), accountStateSnapshot(p.Account)
			baseline := []*model.Account{before}
			if target != nil {
				baseline = append(baseline, accountStateSnapshot(target.Account))
			}
			if err := st.SaveCriticalGameState(retained, nil, "", nil, baseline...); err != nil {
				t.Fatal(err)
			}
			gate, release := criticalGate(t)
			backend := &criticalHeldTreasuryPostgresStore{&criticalHeldPostgresStore{
				PostgresStore: st, entered: make(chan criticalWrite, 2), release: gate,
			}}
			if reject {
				backend.foreign = foreign.Chars[0].Inv[0]
			}
			q := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			assertDurable := func(accounts []*model.Account, guilds *model.GuildRegistry) {
				t.Helper()
				for _, original := range baseline {
					account := original
					for _, proposed := range accounts {
						if proposed.Name == original.Name {
							account = proposed
						}
					}
					loaded, err := st.LoadAccount(account.Name)
					if err != nil || loaded == nil || loaded.Chars[0].GuildID != account.Chars[0].GuildID ||
						loaded.Chars[0].GuildRank != account.Chars[0].GuildRank || loaded.Chars[0].UID != account.Chars[0].UID ||
						loaded.Chars[0].Gold != account.Chars[0].Gold || loaded.Chars[0].Inv != account.Chars[0].Inv || loaded.CargoGold != account.CargoGold {
						t.Fatalf("durable guild command account differs from the atomic proposal: %v", err)
					}
				}
				registry, err := st.LoadGuilds()
				if err != nil || !reflect.DeepEqual(registry, guilds) {
					t.Fatalf("durable guild command registry differs from the atomic proposal: %v", err)
				}
			}
			request := func() {
				switch operation {
				case "create":
					w.guildCommandCreate(p.Session, p, "Created")
				case "accept":
					w.guildCommandAccept(target.Session, target, "")
				case "recruit":
					w.onInviteGuild(p.Session, criticalGuildTestPacket(wire.OpInviteGuild, p.ID, target.ID, 0))
				}
			}
			request()
			proposal := awaitCriticalWrite(t, backend.entered)
			request()
			assertDurable(baseline, retained)
			release()
			result := awaitCriticalResult(t, q)
			if reject != store.IsWriteRejected(result.err) || (!reject && result.err != nil) {
				t.Fatalf("unexpected PostgreSQL guild command outcome: %v", result.err)
			}
			if reject {
				assertDurable(baseline, retained)
			} else {
				assertDurable(proposal.accounts, proposal.guilds)
			}
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
				!w.criticalGuildReserved() || (operation != "accept" && !w.criticalPlayerReserved(p)) || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
				t.Fatal("database guild command bypassed actor acknowledgement or replayed automatically")
			}
			if target != nil && (!reflect.DeepEqual(accountStateSnapshot(target.Account), baseline[1]) || !w.criticalPlayerReserved(target) ||
				target.Session.QueuedPacketsForTest() != 0 || target.GuildInviteFrom != 1) {
				t.Fatal("database guild join published or released membership early")
			}
			w.applyCriticalCompletion(result)
			if reject {
				initiator, message := p, "Save failed. The guild was not created."
				if operation == "accept" {
					initiator, message = target, "Save failed. You did not join the guild."
				}
				if operation == "recruit" {
					message = "Save failed. The recruitment was not applied."
				}
				assertCriticalPickupPacket(t, initiator, wire.MessagePanel(message))
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
					len(q.pending) != 0 || len(backend.entered) != 0 {
					t.Fatal("certified guild command rejection changed retained state or replayed automatically")
				}
				if target != nil && (!reflect.DeepEqual(accountStateSnapshot(target.Account), baseline[1]) || target.GuildInviteFrom != 1) {
					t.Fatal("certified database rejection changed membership or invitation")
				}
				w.clock = newFakeClock(w.now().Add(guildInviteCooldown))
				request()
				proposal = awaitCriticalWrite(t, backend.entered)
				w.applyCriticalCompletion(awaitCriticalResult(t, q))
				assertDurable(proposal.accounts, proposal.guilds)
			}
			wantPackets := 4
			if operation == "accept" {
				wantPackets = 1
			}
			if operation == "recruit" {
				wantPackets = 3
			}
			if p.Char.GuildID != 1 || p.Char.GuildRank != model.GuildRankLeader ||
				!reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) || w.criticalGuildReserved() || len(q.pending) != 0 ||
				w.shuttingDown || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != wantPackets {
				t.Fatal("production guild command did not settle one atomic account/guild proposal")
			}
			if target != nil && (target.Char.GuildID != 1 || target.Char.GuildRank != model.GuildRankMember || target.Session.QueuedPacketsForTest() != 4 ||
				(operation == "accept" && target.GuildInviteFrom != 0) || (operation == "recruit" && p.Char.Gold != before.Chars[0].Gold-guildInviteCost)) {
				t.Fatal("database guild join omitted committed membership/payment")
			}
		})
	}
}
