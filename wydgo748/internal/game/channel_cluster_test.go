package game

import (
	"context"
	"errors"
	"reflect"
	"testing"
	"wydgo/internal/control"

	"wydgo/internal/data"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestChannelGuildPromotionSettlesInOwningWorld(t *testing.T) {
	f := criticalGuildJoinFixture(t, false)
	w := f.w
	// Reuse a numeric entity ID in another world: shared guild operations must
	// retain the target session, never resolve it through the initiator's IDs.
	target, _ := networkedTestPlayer(f.actor.ID, "Remote", 2100, 2100)
	other := worldWithNetworkedPlayers(target)
	other.store, other.channel = w.store, 2
	w.channel = 1
	c, err := NewChannelCluster(w.store)
	if err != nil {
		t.Fatal(err)
	}
	c.owner = w.criticalWrites
	if err := c.attach(w); err != nil {
		t.Fatal(err)
	}
	if err := c.attach(other); err != nil {
		t.Fatal(err)
	}
	other.criticalWrites = c.owner
	c.enter(w)
	if !w.claimAccountSession(target.Session, target.Account.Name) {
		t.Fatal("remote account claim")
	}
	target.Char.GuildID = 1
	w.guilds.Guilds[0].Members = append(w.guilds.Guilds[0].Members, model.GuildMember{
		Character: target.Char.Name, Account: target.Account.Name, Rank: model.GuildRankMember,
	})
	request := func() error {
		return w.queueGuildPromotion(f.actor, target, w.guilds.FindByID(1), model.GuildRankSubFirst)
	}
	if err := request(); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, f.base.entered)
	// World rotation while a detached transaction is held must not invalidate
	// the retained registry or publish success before the worker acknowledges.
	c.enter(other)
	c.enter(w)
	if !errors.Is(request(), errCriticalPersistenceBusy) || target.Char.GuildRank != model.GuildRankMember || target.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("held cross-channel promotion mutated, replayed or published")
	}
	f.release()
	awaitReadyCriticalResults(t, f.q, 1)
	c.enter(other)
	other.processCriticalCompletions(1)
	if w.shuttingDown || other.shuttingDown || target.Char.GuildRank != model.GuildRankSubFirst ||
		w.guilds.FindByID(1).Member(target.Char.Name).Rank != model.GuildRankSubFirst ||
		target.Session.QueuedPacketsForTest() == 0 || len(f.q.pending) != 0 {
		t.Fatal("cross-channel promotion failed to settle and publish on its owning world")
	}
	if f.actor.Char.GuildRank != model.GuildRankLeader {
		t.Fatal("local entity ID collision changed the actor")
	}
}

func TestChannel2OfficialWorldBoot(t *testing.T) {
	base, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	gens, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	f, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	npcs := base
	err = data.ValidateNPCItems(npcs, f.items)
	if err != nil {
		t.Fatal(err)
	}
	for _, n := range npcs {
		switch n.Name {
		case "KnightLeader":
			if class, ok := sefirotClassForNPC(&n); !ok || class != 0 {
				t.Error("KnightLeader lost its Sefirot handler")
			}
		case "KingHarabard":
			if kingdomForNPC(&n) != model.KingdomHekalotia {
				t.Error("KingHarabard lost kingdom progression")
			}
		case "KingGlantuar":
			if kingdomForNPC(&n) != model.KingdomAkelonia {
				t.Error("KingGlantuar lost kingdom progression")
			}
		case "KingdomBrocker":
			if !isKingdomBroker(&n) {
				t.Error("KingdomBrocker lost its handler")
			}
		}
	}
	operational := DefaultOperationalConfig()
	operational.ChannelID = 2
	w, err := NewWorld(&craftStore{}, npcs, gens, model.Catalog{Items: f.items}, [model.MaxCarry]int{}, f.volatiles,
		model.CharacterTemplateFile{}, f.terrain, WithOperationalConfig(operational), WithInitItems(f.initItems), WithNPCGenerLog("quiet"))
	if err != nil {
		t.Fatal(err)
	}
	if len(w.generators) == 0 || len(w.mobs) == 0 {
		t.Fatal("empty official population")
	}
	quests, err := data.LoadQuests("../../data/content/quests.json")
	if err != nil {
		t.Fatal(err)
	}
	operational.ChannelID = 1
	first, err := NewWorld(&craftStore{}, npcs, gens, model.Catalog{Items: f.items}, [model.MaxCarry]int{}, f.volatiles,
		model.CharacterTemplateFile{}, f.terrain, WithOperationalConfig(operational), WithQuests(quests), WithNPCGenerLog("quiet"))
	if err != nil {
		t.Fatal(err)
	}
	if len(first.generators) != len(w.generators) {
		t.Fatal("channels loaded different general populations")
	}
	for i := range first.generators {
		if !reflect.DeepEqual(first.generators[i].def, w.generators[i].def) ||
			first.generators[i].leader == w.generators[i].leader {
			t.Fatalf("group %d differs or shares mutable template data across channels", i)
		}
	}
	for i := range w.npcs {
		if w.npcs[i].Score == npcs[i].Score && npcs[i].Score != nil {
			t.Fatalf("world shares mutable source score for %q", npcs[i].Name)
		}
	}
	t.Logf("channel 2 boot: %d generators, %d mobs", len(w.generators), len(w.mobs))
}

func TestChannelClusterSharedAdmissionAndLocalWorlds(t *testing.T) {
	backend := &craftStore{}
	c, err := NewChannelCluster(backend)
	if err != nil {
		t.Fatal(err)
	}
	first, second := newZoneTestWorld(), newZoneTestWorld()
	first.store, second.store = backend, backend
	first.channel, second.channel = 1, 2
	first.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	first.accountSessions = make(map[string]*gameNet.Session)
	first.authClientsByIP = make(map[string]map[*gameNet.Session]struct{})
	first.authClientsByNetwork = make(map[string]map[*gameNet.Session]struct{})
	if err := c.attach(first); err != nil {
		t.Fatal(err)
	}
	if err := c.attach(second); err != nil {
		t.Fatal(err)
	}
	a, b := gameNet.NewTestSession(1, 256), gameNet.NewTestSession(2, 256)
	if !first.claimAccountSession(a, "Shared") || second.claimAccountSession(b, "shared") {
		t.Fatal("same account admitted on both channels")
	}
	second.releaseAccountSession(b, &model.Account{Name: "Shared"})
	if second.claimAccountSession(b, "Shared") {
		t.Fatal("foreign disconnect released account ownership")
	}
	first.releaseAccountSession(a, &model.Account{Name: "Shared"})
	if !second.claimAccountSession(b, "Shared") {
		t.Fatal("account did not become available after cleanup")
	}
	if first.guilds != second.guilds || reflect.ValueOf(first.players).Pointer() == reflect.ValueOf(second.players).Pointer() {
		t.Fatal("incorrect shared/local ownership")
	}
}

func TestChannelInstanceSnapshotsPreserveOtherWorld(t *testing.T) {
	c := &ChannelCluster{}
	a, b := &World{cluster: c, channel: 1}, &World{cluster: c, channel: 2}
	one := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, NightmarePartyRuns: map[string]int{"one": 1}}
	two := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, NightmarePartyRuns: map[string]int{"two": 2}}
	a.committedChannelInstances(a.persistedChannelInstances(one))
	b.committedChannelInstances(b.persistedChannelInstances(two))
	one.NightmarePartyRuns["one"] = 3
	a.committedChannelInstances(a.persistedChannelInstances(one))
	if got := b.restoredChannelInstances(c.instances); !reflect.DeepEqual(got, two) {
		t.Fatalf("channel 1 overwrote channel 2: %+v", got)
	}
	if got := a.restoredChannelInstances(c.instances); !reflect.DeepEqual(got, one) {
		t.Fatalf("lost channel 1: %+v", got)
	}
}

func TestChannelAdministrationRoutesBySessionAcrossWorlds(t *testing.T) {
	first, local, backend, _ := kickFixture(t)
	second, remote, _, target := teleportFixture(t)
	second.store = backend
	first.channel, second.channel = 1, 2
	// The two channel-local IDs deliberately collide.
	remote.ID = local.ID
	local.Char.UID = "local-character"
	local.Account.Name = "local"
	c, err := NewChannelCluster(backend)
	if err != nil {
		t.Fatal(err)
	}
	if err := c.attach(first); err != nil {
		t.Fatal(err)
	}
	if err := c.attach(second); err != nil {
		t.Fatal(err)
	}
	c.enter(first)
	query := &controlRequest{ctx: context.Background(), reply: make(chan control.Overview, 1)}
	first.handleControl(query)
	snapshot := <-query.reply
	if snapshot.Online != 2 || len(snapshot.Players) != 2 {
		t.Fatalf("missing remote player: %+v", snapshot)
	}
	presence := &accountPresenceRequest{ctx: context.Background(), accounts: []string{"local", "player"}, reply: make(chan map[string]bool, 1)}
	first.handleAccountPresence(presence)
	if got := <-presence.reply; !got["local"] || !got["player"] {
		t.Fatal(got)
	}
	target.Epoch = snapshot.ModerationEpoch
	originalX, originalY := local.X, local.Y
	if result := executeTeleport(first, target, true); result.Code != "ok" {
		t.Fatal(result)
	}
	if remote.X != cityWarZones[0].exitX || local.X != originalX || local.Y != originalY {
		t.Fatal("teleport used the wrong spatial world")
	}
	kick := control.KickCommand{Version: target.Version, OperationID: "remote-kick-00001", Epoch: target.Epoch, UID: target.UID, Account: target.Account, ExpectedSessionID: target.ExpectedSessionID, Reason: target.Reason}
	if result := executeKick(first, kick, true); result.Code != "ok" {
		t.Fatal(result)
	}
	if !remote.Session.IsClosed() || local.Session.IsClosed() || len(second.players) != 0 || len(first.players) != 1 {
		t.Fatal("kick used a channel-local ID without the session identity")
	}
	if c.active != first {
		t.Fatal("administrative routing changed actor ownership")
	}
}
