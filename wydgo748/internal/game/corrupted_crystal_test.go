package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/wire"
)

func crystalTestWorld(t *testing.T) (*World, *fakeClock, *Mob) {
	t.Helper()
	w, clock := bossSpawnTestWorld()
	config, err := ReadCorruptedCrystal("../../data/content/corrupted_crystal.json")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv",
		"../../data/catalog/ItemEffect.h", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.items, w.skills = catalog.Items, catalog.Skills
	w.npcs, err = data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	w.terrain, err = data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	WithCorruptedCrystal(config)(w)
	if err := w.initCorruptedCrystal(clock.Now()); err != nil {
		t.Fatal(err)
	}
	if len(w.corruptedCrystal.live) != 1 {
		t.Fatal("the configured origin did not spawn a crystal")
	}
	for _, mob := range w.corruptedCrystal.live {
		return w, clock, mob
	}
	panic("unreachable")
}

func TestCorruptedCrystalEncounter(t *testing.T) {
	const crystalHP, mediumHP, smallHP = 100_000_000, 10_000_000, 1_000_000
	w, clock, crystal := crystalTestWorld(t)
	event := w.corruptedCrystal
	if crystal.X != 1104 || crystal.Y != 1711 || crystal.HP != crystalHP {
		t.Fatalf("unexpected crystal spawn at (%d,%d) with %d HP", crystal.X, crystal.Y, crystal.HP)
	}
	player := addZonePlayer(w, 1, crystal.X+1, crystal.Y, 1000)
	w.showMob(player, crystal)
	spawn, ok := player.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(spawn) || wire.ParseHeader(spawn).Type != wire.OpCreateMob ||
		binary.LittleEndian.Uint16(spawn[34:36]) != CrystalAppearance ||
		binary.LittleEndian.Uint32(spawn[160:164]) != crystalHP ||
		binary.LittleEndian.Uint32(spawn[168:172]) != crystalHP {
		t.Fatal("the ordinary spawn packet did not carry the reserved appearance and full HP")
	}
	crystal.HP = 75_000_001
	player.Char.Score.Attack, player.Char.Score.Accuracy = 1000, 1000
	applyScore(player.Char)
	w.onAttack(player.Session, physicalAttackPacket(1000, crystal.ID, crystal.X, crystal.Y))
	if crystal.HP > 75_000_000 || crystal.HP <= 50_000_000 {
		t.Fatalf("attack did not cross the first crack threshold: HP=%d", crystal.HP)
	}
	updated := false
	for packet, ok := player.Session.DequeuePacketForTest(); ok; packet, ok = player.Session.DequeuePacketForTest() {
		if !wire.Decrypt(packet) {
			t.Fatal("invalid outbound packet checksum")
		}
		if header := wire.ParseHeader(packet); header.Type == wire.OpSetHpMp && header.ID == crystal.ID {
			updated = binary.LittleEndian.Uint32(packet[12:16]) == crystal.HP &&
				binary.LittleEndian.Uint32(packet[20:24]) == crystalHP
		}
	}
	if !updated {
		t.Fatal("the attacker did not receive a full-width health update")
	}
	observer := addZonePlayer(w, 2, crystal.X, crystal.Y+1, 1000)
	w.showMob(observer, crystal)
	spawn, ok = observer.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(spawn) || wire.ParseHeader(spawn).Type != wire.OpCreateMob ||
		binary.LittleEndian.Uint32(spawn[168:172]) != crystal.HP {
		t.Fatal("the observer did not receive the damaged crystal snapshot")
	}
	crystal.HP = 0
	seen := make(map[uint16]bool)
	dropTiles := make(map[crystalTile]bool)
	dropUIDs := make(map[string]bool)
	checkDrop := func(mob *Mob, want int) {
		t.Helper()
		count := 0
		for _, drop := range w.groundItems {
			if seen[drop.ID] {
				continue
			}
			seen[drop.ID] = true
			count++
			tile := crystalTile{drop.X, drop.Y}
			radius := chebyshev(mob.X, mob.Y, tile.x, tile.y)
			if radius == 0 || radius > crystalSearchRadius || dropTiles[tile] ||
				!w.crystalTerrain(tile.x, tile.y) || !w.terrain.HeightCompatible(mob.X, mob.Y, tile.x, tile.y) {
				t.Fatalf("crystal %d dropped on an invalid or occupied tile: %v", mob.ID, tile)
			}
			if drop.Item.Index != 413 || itemStackAmount(drop.Item) != 1 || drop.Item.UID == "" || dropUIDs[drop.Item.UID] {
				t.Fatalf("expected one unique Laktorerium Powder, got %+v", drop.Item)
			}
			dropTiles[tile] = true
			dropUIDs[drop.Item.UID] = true
		}
		if count != want {
			t.Fatalf("crystal %d dropped %d units, want %d", mob.ID, count, want)
		}
	}
	w.killMobState(player, crystal, crystalHP, crystalHP)
	checkDrop(crystal, 20)
	if event.phase != crystalMedium || len(event.live) != 0 {
		t.Fatal("crystal death must schedule the wave after shattering")
	}
	shatter := false
	for packet, ok := observer.Session.DequeuePacketForTest(); ok; packet, ok = observer.Session.DequeuePacketForTest() {
		if !wire.Decrypt(packet) {
			t.Fatal("invalid outbound packet checksum")
		}
		if header := wire.ParseHeader(packet); header.Type == wire.OpRemoveMob && header.ID == crystal.ID {
			shatter = binary.LittleEndian.Uint32(packet[12:16]) == 1
		}
	}
	if !shatter {
		t.Fatal("the observer did not receive the crystal death animation trigger")
	}
	clock.Advance(crystalShatterDelay - time.Nanosecond)
	w.tickCorruptedCrystal(clock.Now())
	if len(event.live) != 0 {
		t.Fatal("the wave spawned before the animation deadline")
	}
	clock.Advance(time.Nanosecond)
	w.tickCorruptedCrystal(clock.Now())
	if len(event.live) != 5 {
		t.Fatalf("spawned %d medium crystals, want 5", len(event.live))
	}
	for _, medium := range event.live {
		if medium.crystalRole != crystalMedium || medium.HP != mediumHP ||
			medium.Def.Equip.Rosto.Index != crystalMediumAppearance ||
			chebyshev(crystal.X, crystal.Y, medium.X, medium.Y) > crystalSearchRadius {
			t.Fatalf("invalid medium crystal: %+v", medium)
		}
		medium.HP = 0
		w.killMobState(player, medium, mediumHP, mediumHP)
		checkDrop(medium, 4)
		w.tickCorruptedCrystal(clock.Now())
		if len(event.mediumOrigins) < 5 && (event.phase != crystalMedium || len(event.live) != 5-len(event.mediumOrigins)) {
			t.Fatal("small crystals must wait for all five medium crystals")
		}
	}
	if event.phase != crystalSmall || len(event.live) != 0 {
		t.Fatal("the last medium crystal must schedule the small wave after shattering")
	}
	clock.Advance(crystalShatterDelay)
	w.tickCorruptedCrystal(clock.Now())
	if len(event.live) != 20 {
		t.Fatalf("spawned %d small crystals, want 20", len(event.live))
	}
	// World order preserves spawn order, with four children per medium origin.
	next := 0
	for _, small := range w.mobs {
		if event.live[small.ID] != small {
			continue
		}
		parent := event.mediumOrigins[next/4]
		distance := chebyshev(parent.x, parent.y, small.X, small.Y)
		if small.crystalRole != crystalSmall || small.HP != smallHP ||
			small.Def.Equip.Rosto.Index != crystalSmallAppearance || distance < 1 || distance > crystalSearchRadius {
			t.Fatalf("invalid small crystal for medium at %v: %+v", parent, small)
		}
		next++
	}
	if next != 20 {
		t.Fatalf("world contains %d small crystals, want 20", next)
	}
	for _, small := range event.live {
		small.HP = 0
		w.killMobState(player, small, smallHP, smallHP)
		checkDrop(small, 1)
		w.onCrystalMobKilled(small, true, clock.Now()) // Ignore repeated finalization.
	}
	w.tickCorruptedCrystal(clock.Now())
	if len(w.groundItems) != 60 || event.phase != crystalLarge || len(event.live) != 0 || len(event.drops) != 0 {
		t.Fatal("the complete encounter must drop 60 single units and schedule a new large crystal")
	}
	clock.Advance(time.Duration(event.config.RespawnSeconds) * time.Second)
	occupant := addZonePlayer(w, 3, crystal.X, crystal.Y, 1000)
	w.tickCorruptedCrystal(clock.Now())
	if len(event.live) != 0 {
		t.Fatal("an occupied tile must delay the crystal respawn")
	}
	occupant.X++
	w.updatePlayerSpatial(occupant)
	clock.Advance(crystalRetryDelay)
	w.tickCorruptedCrystal(clock.Now())
	if len(event.live) != 1 || event.live[crystal.ID] != nil || len(event.attackers) != 0 {
		t.Fatal("respawn must publish a new encounter with no prior attackers")
	}
	for _, fresh := range event.live {
		if fresh.X != crystal.X || fresh.Y != crystal.Y || fresh.HP != crystalHP {
			t.Fatal("the crystal did not respawn on its exact tile at full health")
		}
	}
}

func TestCorruptedCrystalRetriesBlockedDrops(t *testing.T) {
	w, clock, crystal := crystalTestWorld(t)
	player := addZonePlayer(w, 1, crystal.X+1, crystal.Y, 1000)
	center := crystalTile{crystal.X, crystal.Y}
	free := 2
	w.visitCrystalTiles(center, 1, crystalSearchRadius, func(tile crystalTile) bool {
		if free > 0 {
			free--
		} else if !w.spawnGroundReward(tile.x, tile.y, 412) {
			t.Fatal("could not place a ground item to block the drop tile")
		}
		return false
	})
	crystal.HP = 0
	w.killMobState(player, crystal, crystal.Def.Score.MaxHP, crystal.Def.Score.MaxHP)
	event := w.corruptedCrystal
	if len(event.drops) != 1 || event.drops[0].remaining != 18 {
		t.Fatalf("expected 18 pending powders, got %+v", event.drops)
	}
	firstDrops := make(map[uint16]*GroundItem)
	for _, item := range w.groundItems {
		if item.Item.Index == crystalPowderItem {
			firstDrops[item.ID] = item
		} else {
			w.unregisterGroundItem(item)
		}
	}
	if len(firstDrops) != 2 {
		t.Fatalf("expected two immediate powders, got %d", len(firstDrops))
	}
	clock.Advance(crystalRetryDelay - time.Nanosecond)
	w.tickCorruptedCrystal(clock.Now())
	if len(w.groundItems) != 2 {
		t.Fatal("the remaining drops appeared before the retry")
	}
	clock.Advance(time.Nanosecond)
	w.tickCorruptedCrystal(clock.Now())
	if len(event.drops) != 0 || len(w.groundItems) != 20 {
		t.Fatal("the retry must deliver exactly the pending powders")
	}
	for id, item := range firstDrops {
		if w.groundItems[id] != item {
			t.Fatal("the retry replaced an existing powder")
		}
	}
}

func TestCorruptedCrystalPulse(t *testing.T) {
	w, clock, crystal := crystalTestWorld(t)
	attacker := addZonePlayer(w, 1, crystal.X+4, crystal.Y+4, 1000)
	observer := addZonePlayer(w, 2, crystal.X+1, crystal.Y, 1000)
	outside := addZonePlayer(w, 3, crystal.X+5, crystal.Y, 1000)
	w.showMob(attacker, crystal)
	w.notifyMobDamaged(crystal, crystal.HP, attacker.ID, 1)
	w.recordCrystalAttack(crystal, outside, 1)
	w.recordCrystalAttack(crystal, observer, 0)
	clock.Advance(crystalPulseInterval - time.Nanosecond)
	w.tickCorruptedCrystal(clock.Now())
	if playerCurHP(attacker.Char) != 1000 {
		t.Fatal("pulse fired before eight seconds")
	}
	clock.Advance(time.Nanosecond)
	w.tickCorruptedCrystal(clock.Now())
	hp := playerCurHP(attacker.Char)
	if hp == 1000 || playerCurHP(observer.Char) != 1000 || playerCurHP(outside.Char) != 1000 {
		t.Fatal("pulse must hit only attackers inside four tiles")
	}
	pulses := 0
	for packet, ok := attacker.Session.DequeuePacketForTest(); ok; packet, ok = attacker.Session.DequeuePacketForTest() {
		if !wire.Decrypt(packet) {
			t.Fatal("invalid outbound packet checksum")
		}
		if wire.ParseHeader(packet).Type == wire.OpAttackOne {
			pulses++
			if len(packet) != 60 || binary.LittleEndian.Uint16(packet[24:26]) != crystalThunderSkill ||
				packet[28] != crystalPulseMotion || binary.LittleEndian.Uint16(packet[44:46]) != attacker.ID ||
				binary.LittleEndian.Uint32(packet[56:60]) != 1000-hp {
				t.Fatal("pulse must send the Thunderbolt visual and the authoritative wide damage")
			}
		}
	}
	if pulses != 1 {
		t.Fatalf("expected one pulse packet, got %d", pulses)
	}
	w.tickCorruptedCrystal(clock.Now())
	if playerCurHP(attacker.Char) != hp {
		t.Fatal("pulse repeated in the same tick")
	}
	clock.Advance(crystalPulseInterval)
	w.tickCorruptedCrystal(clock.Now())
	if playerCurHP(attacker.Char) >= hp || crystal.X != w.corruptedCrystal.config.X || crystal.Y != w.corruptedCrystal.config.Y {
		t.Fatal("the stationary crystal did not pulse again after eight seconds")
	}
}
