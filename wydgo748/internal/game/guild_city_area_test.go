package game

import (
	"encoding/binary"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/lang"
	"wydgo/internal/model"
)

func guildCityTerrain() model.TerrainMap {
	terrain := loadedFlatTerrain()
	for _, zone := range homeCityZones {
		x, y := zone.guildReturnX, zone.guildReturnY
		terrain.Attribute[(int(y)/4)*model.AttributeWidth+int(x)/4] = 0x21
	}
	// The reported Armia tile and the four-cell boundary used by route tests.
	terrain.Attribute[(2144/4)*model.AttributeWidth+2092/4] = 0x21
	return terrain
}

func setGuildCityOwner(w *World, city int, owner uint16) {
	if city < len(w.guilds.Wars.Cities.Territories) {
		w.guilds.Wars.Cities.Territories[city].Owner = owner
	} else {
		w.guilds.Wars.Castle.Owner = owner
	}
}

func addGuildCityMember(w *World, p *Player, id uint16) {
	w.guilds.Guilds = append(w.guilds.Guilds,
		guildWithMembers(id, fmt.Sprintf("Guild%d", id), member(p.Char.Name, p.Account.Name, model.GuildRankMember)))
	p.Char.GuildID = id
}

func TestGuildCityAreaAccessUsesCanonicalMemberAndNativeLevel(t *testing.T) {
	terrain := guildCityTerrain()
	for city, zone := range homeCityZones {
		for _, tc := range []struct {
			name   string
			owner  uint16
			guild  uint16
			cached uint16
			level  uint32
			want   bool
		}{
			{"owner", 17, 17, 17, 399, true},
			{"outsider", 17, 42, 42, 399, false},
			{"guildless", 17, 0, 0, 399, false},
			{"forged-copy", 17, 0, 17, 399, false},
			{"stale-copy", 17, 17, 0, 399, true},
			{"unowned-guildless", 0, 0, 0, 399, true},
			{"unowned-member", 0, 17, 17, 399, false},
			{"displayed-401", 17, 42, 42, 400, true},
			{"native-gm", 17, 0, 0, 1000, true},
		} {
			t.Run(fmt.Sprintf("%d/%s", city, tc.name), func(t *testing.T) {
				w, p, _ := handlerTestWorld(t)
				w.terrain = terrain
				setGuildCityOwner(w, city, tc.owner)
				if tc.guild != 0 {
					addGuildCityMember(w, p, tc.guild)
				}
				p.Char.GuildID, p.Char.Score.Level = tc.cached, tc.level
				before := accountStateSnapshot(p.Account)
				if got := w.guildCityAreaAllowed(p.Char, zone.guildReturnX, zone.guildReturnY); got != tc.want {
					t.Fatalf("allowed=%v want=%v", got, tc.want)
				}
				if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
					t.Fatal("an access check mutated the character projection")
				}
				if !w.guildCityAreaAllowed(p.Char, zone.returnX, zone.returnY) {
					t.Fatal("public return outside the marked cell became private")
				}
			})
		}
	}
}

func TestGuildCityAreaUsesActualAttributeAndCurrentWorldOwner(t *testing.T) {
	terrain, err := data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	w, p, _ := handlerTestWorld(t)
	w.terrain = terrain
	addGuildCityMember(w, p, 17)
	for city, zone := range homeCityZones {
		if !terrain.GuildArea(zone.guildReturnX, zone.guildReturnY) {
			t.Fatalf("city %d guild return is not marked private in the shipped map", city)
		}
		setGuildCityOwner(w, city, 17)
		if !w.guildCityAreaAllowed(p.Char, zone.guildReturnX, zone.guildReturnY) {
			t.Fatal("owning member cannot enter the shipped guild area")
		}
		setGuildCityOwner(w, city, 42)
		if w.guildCityAreaAllowed(p.Char, zone.guildReturnX, zone.guildReturnY) {
			t.Fatal("membership in another city or world authorized this city")
		}
	}
	if !terrain.GuildArea(2092, 2147) || w.guildCityAreaAllowed(p.Char, 2092, 2147) {
		t.Fatal("the reported Armia access is still unrestricted")
	}
	other, _, _ := handlerTestWorld(t)
	other.terrain = terrain
	other.guilds = w.guilds.Clone()
	setGuildCityOwner(other, 0, 17)
	if !other.guildCityAreaAllowed(p.Char, 2092, 2147) || w.guildCityAreaAllowed(p.Char, 2092, 2147) {
		t.Fatal("the selected world's owner was not respected")
	}
}

func TestGuildCityAreaMovementStopsAtIntermediatePrivateCellAndRecalls(t *testing.T) {
	for _, owner := range []bool{false, true} {
		t.Run(fmt.Sprint(owner), func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			w.terrain = guildCityTerrain()
			setGuildCityOwner(w, 0, 17)
			if owner {
				addGuildCityMember(w, p, 17)
			}
			p.Char.Score.Level = 399
			p.X, p.Y, p.Char.X, p.Char.Y = 2090, 2145, 2090, 2145
			w.updatePlayerSpatial(p)
			now, route := w.now(), []byte("666666")
			// The destination is public, but intermediate cells 2092-2095 are private.
			w.beginPlayerMovement(p, p.X, p.Y, 2096, 2145, route, route, 0, now)
			w.advancePlayerMovement(p, now.Add(3*time.Second))
			if owner {
				if p.X != 2096 || p.Y != 2145 {
					t.Fatal("owner route did not cross its private cells")
				}
			} else if w.terrain.GuildArea(p.X, p.Y) || p.Y == 2145 || st.saves != 1 || p.MovePublished {
				t.Fatal("outsider crossed a private intermediate cell or failed to recall once")
			}
			if p.X != p.Char.X || p.Y != p.Char.Y {
				t.Fatal("movement/recall left position projections inconsistent")
			}
		})
	}
}

func TestGuildCityAreaTeleportAndFreeCellFallback(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	w.terrain = guildCityTerrain()
	setGuildCityOwner(w, 0, 17)
	before := accountStateSnapshot(p.Account)
	if w.teleportPlayer(p, 2092, 2147) || st.saves != 0 || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
		t.Fatal("rejected private teleport changed state or persisted a position")
	}
	blocker, _ := networkedTestPlayer(2, "Blocker", 2091, 2147)
	w.players[blocker.Session], w.playersByID[blocker.ID] = blocker, blocker
	w.updatePlayerSpatial(blocker)
	x, y, ok := w.resolvePlayerTeleportDestination(p, 2091, 2147)
	if !ok || (x == 2091 && y == 2147) || w.terrain.GuildArea(x, y) {
		t.Fatal("public teleport fallback chose the adjacent private cell or lost an available public cell")
	}
	addGuildCityMember(w, p, 17)
	if !w.teleportPlayer(p, 2092, 2147) || p.X != 2092 || p.Y != 2147 || st.saves != 1 {
		t.Fatal("owning member cannot teleport to its private area")
	}
	w.guilds.Guilds = nil
	if _, _, ok := w.resolvePlayerTeleportDestination(p, 2092, 2147); ok {
		t.Fatal("stale GuildID retained teleport access after canonical membership was removed")
	}
}

func TestGuildCityAreaGuardRecallsMembershipAndOwnershipChanges(t *testing.T) {
	for _, change := range []string{"owner", "member"} {
		t.Run(change, func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			w.terrain = guildCityTerrain()
			setGuildCityOwner(w, 0, 17)
			addGuildCityMember(w, p, 17)
			p.X, p.Y, p.Char.X, p.Char.Y = 2092, 2147, 2092, 2147
			w.updatePlayerSpatial(p)
			w.tickGuildCityAreas(w.now())
			if p.X != 2092 || p.Y != 2147 || st.saves != 0 {
				t.Fatal("the area guard recalled an authorized member")
			}
			if change == "owner" {
				setGuildCityOwner(w, 0, 42)
			} else {
				w.guilds.Guilds = nil // Keep the stale denormalized GuildID deliberately.
			}
			w.tickGuildCityAreas(w.now())
			w.tickGuildCityAreas(w.now())
			if w.terrain.GuildArea(p.X, p.Y) || st.saves != 1 || p.MovePublished {
				t.Fatal("revoked access was retained or guard repeated its recall")
			}
		})
	}
}

func TestGuildCityAreaCriticalTeleportSettlesBeforeRevokedAccessRecall(t *testing.T) {
	w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
	w.terrain = guildCityTerrain()
	guild, _ := w.guildOf(p.Char)
	if guild == nil {
		t.Fatal("critical fixture requires canonical membership")
	}
	setGuildCityOwner(w, 0, guild.ID)
	before := accountStateSnapshot(p.Account)
	if err := w.queuePlayerTeleport(p, accountStateSnapshot(p.Account), 2092, 2147, "guild area test", nil, nil, nil); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, base.entered)
	setGuildCityOwner(w, 0, 42)
	w.tickGuildCityAreas(w.now())
	if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || len(base.entered) != 0 {
		t.Fatal("the guard changed or duplicated an admitted account write")
	}
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if p.X != 2092 || p.Y != 2147 || p.Session.IsClosed() || w.shuttingDown || w.criticalPlayerReserved(p) {
		t.Fatal("ownership change invalidated a committed teleport or retained its fence")
	}
	w.tickGuildCityAreas(w.now())
	awaitCriticalWrite(t, base.entered)
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if w.terrain.GuildArea(p.X, p.Y) || w.criticalPlayerReserved(p) || fallback.saves != 0 || p.Session.IsClosed() {
		t.Fatal("revoked access did not settle a separate durable safe recall")
	}
}

func TestGuildCityAreaAlliesAndMissingStateDoNotGrantAccess(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = guildCityTerrain()
	setGuildCityOwner(w, 0, 17)
	addGuildCityMember(w, p, 42)
	w.guilds.Guilds[0].Ally = 17
	if w.guildCityAreaAllowed(p.Char, 2092, 2147) {
		t.Fatal("an alliance granted ownership access")
	}
	w.guilds = nil
	if w.guildCityAreaAllowed(p.Char, 2092, 2147) || w.guildCityAreaAllowed(nil, 2092, 2147) {
		t.Fatal("missing ownership or character state granted private access")
	}
	p.Char.Score = nil
	if w.guildCityAreaAllowed(p.Char, 2092, 2147) {
		t.Fatal("missing level state granted private access")
	}
	if !w.guildCityAreaAllowed(nil, 2086, 2093) {
		t.Fatal("a public cell requires guild state")
	}
	w.terrain.Attribute[(2200/4)*model.AttributeWidth+2200/4] = 0x20
	if !w.guildCityAreaAllowed(nil, 2200, 2200) {
		t.Fatal("an attribute outside the five native cities became a city restriction")
	}
}

func TestGuildCityAreaGuardPreservesNativeLevelAndPrivateRuntimes(t *testing.T) {
	for _, mode := range []string{"raw-400", "raw-1000", "private-runtime"} {
		t.Run(mode, func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			w.terrain = guildCityTerrain()
			setGuildCityOwner(w, 0, 17)
			p.X, p.Y, p.Char.X, p.Char.Y = 2092, 2147, 2092, 2147
			switch mode {
			case "raw-400":
				p.Char.Score.Level = 400
			case "raw-1000":
				p.Char.Score.Level = 1000
			case "private-runtime":
				w.itemInstances = map[string]*ItemInstance{"private-city-test": {RuntimeID: "private-city-test", MemberIDs: []uint16{p.ID}}}
				// A private runtime may overlap this map. Its public exit still needs permission.
				if _, _, ok := w.resolvePlayerTeleportDestination(p, 2092, 2147); ok {
					t.Fatal("the source runtime bypassed public destination access")
				}
			}
			w.tickGuildCityAreas(w.now())
			if p.X != 2092 || p.Y != 2147 || st.saves != 0 || !w.guildCityAreaStepAllowed(p, 2093, 2147) {
				t.Fatal("the guard changed a native level exemption or private runtime")
			}
		})
	}
}

func TestGuildCityAreaEntryDoesNotRestoreUnauthorizedSavedPosition(t *testing.T) {
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	w.terrain = guildCityTerrain()
	w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	setGuildCityOwner(w, 0, 17)
	p.Account.Chars[0].X, p.Account.Chars[0].Y, p.Account.Chars[0].GuildID = 2092, 2147, 17
	w.onEnterWorld(s, enterWorldPacket(0))
	if !p.InWorld || p.Char == nil || w.terrain.GuildArea(p.X, p.Y) || !w.guildCityAreaAllowed(p.Char, p.X, p.Y) {
		t.Fatal("entry restored a private saved position or rejected the public return")
	}
}

func TestGuildCityAreaRejectedPaidPortalPreservesGold(t *testing.T) {
	for _, critical := range []bool{false, true} {
		t.Run(fmt.Sprint(critical), func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			var base *criticalTestStore
			if critical {
				w, p, _, _, base, _, _ = criticalTreasuryFixture(t, 100)
			}
			w.terrain = guildCityTerrain()
			setGuildCityOwner(w, 0, 42)
			w.teleports = []model.Teleport{{SourceX: p.X, SourceY: p.Y, DestX: 2092, DestY: 2147, Price: 50}}
			p.Char.Gold = 100
			before := accountStateSnapshot(p.Account)
			w.onReqTeleport(p.Session, make([]byte, 16))
			if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || w.criticalPlayerReserved(p) || st.saves != 0 ||
				(base != nil && len(base.entered) != 0) {
				t.Fatal("a forbidden paid destination consumed gold or queued a durable write")
			}
		})
	}
}

func TestGuildCityAreaCriticalRecallRejectionAndRetry(t *testing.T) {
	w, p, _, fallback, base, q, release := criticalTreasuryFixture(t, 100)
	w.terrain = guildCityTerrain()
	setGuildCityOwner(w, 0, 42)
	p.X, p.Y, p.Char.X, p.Char.Y = 2092, 2147, 2092, 2147
	w.updatePlayerSpatial(p)
	before := accountStateSnapshot(p.Account)
	base.failure = criticalCertifiedRejection(t)
	w.tickGuildCityAreas(w.now())
	awaitCriticalWrite(t, base.entered)
	w.tickGuildCityAreas(w.now())
	if len(base.entered) != 0 || !w.criticalPlayerReserved(p) || !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) {
		t.Fatal("the guard duplicated or exposed a pending recall")
	}
	release()
	result := awaitCriticalResult(t, q)
	base.failure = nil
	w.applyCriticalCompletion(result)
	if !reflect.DeepEqual(before, accountStateSnapshot(p.Account)) || w.criticalPlayerReserved(p) || p.Session.IsClosed() || w.shuttingDown {
		t.Fatal("a certified recall rejection changed state or contained the player")
	}
	w.tickGuildCityAreas(w.now())
	awaitCriticalWrite(t, base.entered)
	w.applyCriticalCompletion(awaitCriticalResult(t, q))
	if w.terrain.GuildArea(p.X, p.Y) || w.criticalPlayerReserved(p) || p.Session.IsClosed() || fallback.saves != 0 {
		t.Fatal("a later guard tick did not commit one safe recall")
	}
}

func TestGuildCityAreaNoticeUsesChannelLanguage(t *testing.T) {
	for _, tc := range []struct {
		language lang.Language
		want     string
	}{
		{lang.English, guildCityAreaMessage},
		{lang.Portuguese, portugueseText(t, "Apenas membros da guild dona desta cidade podem entrar nesta área.")},
		{lang.Language(255), guildCityAreaMessage},
	} {
		w, p, _ := handlerTestWorld(t)
		w.language = tc.language
		if got := w.trFor(p, guildCityAreaMessage); got != tc.want || len(got) > 95 {
			t.Fatalf("notice language %d = %q, want %q within the native panel", tc.language, got, tc.want)
		}
	}
}

func TestGuildCityAreaIllusionRejectsWithoutSpendingManaOrCooldown(t *testing.T) {
	for _, owner := range []bool{false, true} {
		t.Run(fmt.Sprint(owner), func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			w.terrain = guildCityTerrain()
			setGuildCityOwner(w, 0, 17)
			if owner {
				addGuildCityMember(w, p, 17)
			}
			p.Char.Class, p.Char.LearnedSkill = 3, 1<<1
			p.X, p.Y, p.Char.X, p.Char.Y = 2090, 2147, 2090, 2147
			w.updatePlayerSpatial(p)
			w.skills = map[int]model.SkillDef{73: {Index: 73, ManaSpent: 10, Range: 8}}
			packet := make([]byte, 28)
			binary.LittleEndian.PutUint16(packet[24:26], 2092)
			binary.LittleEndian.PutUint16(packet[26:28], 2147)
			mp := playerCurMP(p.Char)
			w.onIllusionMove(p, packet)
			if owner {
				if p.X != 2092 || p.Y != 2147 || playerCurMP(p.Char) >= mp || p.SkillReady[73].IsZero() {
					t.Fatal("an owning member's Illusion was blocked")
				}
			} else if p.X != 2090 || p.Y != 2147 || playerCurMP(p.Char) != mp || !p.SkillReady[73].IsZero() {
				t.Fatal("rejected Illusion changed position, mana or cooldown")
			}
		})
	}
}

func TestGuildCityAreaSkillPullUsesTargetMembership(t *testing.T) {
	w, caster, _ := handlerTestWorld(t)
	w.terrain = guildCityTerrain()
	setGuildCityOwner(w, 0, 17)
	addGuildCityMember(w, caster, 17)
	caster.X, caster.Y, caster.Char.X, caster.Char.Y = 2093, 2147, 2093, 2147
	w.updatePlayerSpatial(caster)
	target, _ := networkedTestPlayer(2, "PullTarget", 2090, 2147)
	w.players[target.Session], w.playersByID[target.ID] = target, target
	w.updatePlayerSpatial(target)
	if changed := w.applySupportSkillToTargets(caster, []*Player{target}, model.SkillDef{Index: 42}, 50); len(changed) != 0 || target.X != 2090 {
		t.Fatal("the caster's ownership authorized an outsider's Teleport skill pull")
	}
	if x, y := w.findFreeGameplayPosition(caster, target, 2092, 2147, 2); x != target.X || y != target.Y {
		t.Fatal("a public combat jump or revival pull admitted the displaced outsider")
	}
	// Public fallback must not choose an empty private cell next to a blocker.
	x, y := w.findFreeGameplayPosition(target, target, caster.X, caster.Y, 2)
	if w.terrain.GuildArea(x, y) {
		t.Fatal("skill displacement used an unauthorized fallback")
	}
	addGuildCityMember(w, target, 17)
	if changed := w.applySupportSkillToTargets(caster, []*Player{target}, model.SkillDef{Index: 42}, 50); len(changed) != 1 || !w.terrain.GuildArea(target.X, target.Y) {
		t.Fatal("an owning target could not be pulled into its guild's area")
	}
}
