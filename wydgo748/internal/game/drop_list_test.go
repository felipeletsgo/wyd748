package game

import (
	"encoding/binary"
	"reflect"
	"strings"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestDropListBossUsesOwnTableAndSharedAreaReward(t *testing.T) {
	level := uint32(4)
	w := &World{items: map[uint16]model.ItemDef{412: {Index: 412}, 413: {Index: 413}, 414: {Index: 414}},
		npcs: []model.NPCDef{{Name: "Base", DisplayName: "Visible", Tipo: model.TipoMonstro, Score: &model.Score{Level: 100}, Carry: []model.Item{{Index: 412}}}},
		bossCatalog: model.BossCatalog{Bosses: []model.BossConfig{{Name: "Boss_Config", NPC: "Base", Stats: model.BossStats{Level: &level},
			Drops: []model.BossDrop{{Item: 412, ChancePercent: 20}, {Item: 413, ChancePercent: 25}}, AreaReward: model.BossAreaReward{Item: 414, Amount: 3}}}}}
	w.dropRates[0] = 1000
	var foundBoss, foundArea bool
	for _, r := range w.buildDropListIndex() {
		if r.flags == 2 && r.item == 412 {
			foundBoss = true
			if len(r.rates) != 0 || dropListChance(r, 0) != 200_000_000 || dropListChance(r, 100) != 200_000_000 {
				t.Fatal("boss inherited ordinary carry or changed its fixed chance with a player bonus")
			}
		}
		if r.flags == 8 && r.item == 414 {
			foundArea = true
			if dropListChance(r, 0) != wire.DropListChanceScale || !strings.Contains(r.nameKey, "boss config") {
				t.Fatal("shared area reward lacks its condition or search alias")
			}
		}
	}
	if !foundBoss || !foundArea {
		t.Fatal("boss drops or shared area reward are missing")
	}
}

func dropQueryPacket(mode byte, query string, offset uint32) []byte {
	b := inboundPacket(wire.OpDropListQuery, wire.DropListQuerySize)
	binary.LittleEndian.PutUint16(b[12:14], 1)
	b[14] = mode
	binary.LittleEndian.PutUint32(b[16:20], 17)
	binary.LittleEndian.PutUint32(b[20:24], offset)
	copy(b[24:88], query)
	return b
}

func TestDropListProbabilityCombinesRollsAndExactBonus(t *testing.T) {
	r := dropListRelation{rates: []int{4, 4}}
	if got := dropListChance(r, 0); got != 437_500_000 {
		t.Fatalf("two 25%% rolls: %d", got)
	}
	if got := dropListChance(r, 100); got != 750_000_000 {
		t.Fatalf("exact rate 2 with +100%%: %d", got)
	}
	if dropListChance(dropListRelation{rates: []int{32000}}, 0) != 31250 {
		t.Fatal("rare chance lost precision")
	}
	if dropListChance(dropListRelation{rates: []int{0, 32000}}, 0) != wire.DropListChanceScale {
		t.Fatal("guaranteed slot lost")
	}
}

func TestDropListSearchPagingLocalizedMaskAndReadOnly(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Reader", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.items = map[uint16]model.ItemDef{412: {Index: 412, Name: "Oriharukon_Powder"}}
	spawned := make(map[string]bool)
	for i := 0; i < 13; i++ {
		name := "Skeleton" + string(rune('A'+i))
		spawned[name] = true
		w.npcs = append(w.npcs, model.NPCDef{Name: name, Tipo: model.TipoMonstro, Score: &model.Score{Level: 100}, Carry: []model.Item{{Index: 412}}})
	}
	w.npcs[0].DisplayName = "Undead_Archer"
	w.npcs = append(w.npcs, model.NPCDef{Name: "Shop", Tipo: model.TipoNPC, Score: &model.Score{}, Carry: []model.Item{{Index: 412}}})
	w.npcs = append(w.npcs, model.NPCDef{Name: "RemovedFromNPCGener", Tipo: model.TipoMonstro, Score: &model.Score{Level: 100}, Carry: []model.Item{{Index: 412}}})
	w.npcDefinitionState = &npcDefinitionRuntime{spawned: spawned}
	before := accountStateSnapshot(p.Account)
	query := dropQueryPacket(0, "skeleton", 10)
	w.handle(command{s: p.Session, pkt: query})
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) {
		t.Fatal("no response through ingress")
	}
	if len(packet) != wire.DropListResultSize || binary.LittleEndian.Uint32(packet[24:28]) != 13 || binary.LittleEndian.Uint16(packet[32:34]) != 3 {
		t.Fatal("paging includes NPC shops or loses monsters")
	}
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
		t.Fatal("search changed economic state")
	}
	p.LastDropList = time.Time{}
	w.onDropListQuery(p.Session, dropQueryPacket(0, "undead archer", 0))
	packet, ok = p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || binary.LittleEndian.Uint32(packet[24:28]) != 1 {
		t.Fatal("the visible monster name cannot be searched")
	}
	p.LastDropList = time.Time{}
	w.onDropListQuery(p.Session, dropQueryPacket(0, "removedfromnpcgener", 0))
	packet, ok = p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || binary.LittleEndian.Uint32(packet[24:28]) != 0 || binary.LittleEndian.Uint16(packet[32:34]) != 0 {
		t.Fatal("an NPC template without an active spawn source appeared in the drop list")
	}
	query = dropQueryPacket(1, "po inexistente", 0)
	query[88+412/8] |= 1 << uint(412%8)
	p.LastDropList = time.Time{}
	w.onDropListQuery(p.Session, query)
	packet, ok = p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || binary.LittleEndian.Uint32(packet[24:28]) != 13 {
		t.Fatal("localized item mask did not resolve known ID")
	}
	w.onDropListQuery(p.Session, query)
	packet, ok = p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || packet[15] != 1 {
		t.Fatal("repeated searches were not throttled")
	}
	p.LastDropList = time.Time{}
	p.InWorld = false
	w.onDropListQuery(p.Session, query)
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("search outside world was served")
	}
}

func TestDropListEventExpiryAndCacheIsolation(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{412: {Index: 412, Name: "Powder"}},
		npcs: []model.NPCDef{{Name: "Skeleton", Tipo: model.TipoMonstro, Score: &model.Score{Level: 100}, Carry: []model.Item{{Index: 412}}},
			{Name: "RemovedFromNPCGener", Tipo: model.TipoMonstro, Score: &model.Score{Level: 100}}},
		npcDefinitionState: &npcDefinitionRuntime{spawned: map[string]bool{"Skeleton": true}}}
	w.dropRates[0] = 100
	now := time.Unix(100, 0)
	end := now.Add(time.Second)
	w.globalDrop = &globalDropState{status: control.GlobalDropStatus{Active: true, ItemID: 412, RatePercent: 50, EndsAt: &end}}
	rows := w.dropListRelations(now)
	if len(rows) != 1 || rows[0].monster != "Skeleton" || rows[0].flags != 5 || len(rows[0].fixed) != 1 {
		t.Fatal("active event was not combined")
	}
	rows = w.dropListRelations(end)
	if len(rows) != 1 || rows[0].flags != 1 || len(rows[0].fixed) != 0 {
		t.Fatal("event mutated cached ordinary drops")
	}
	if dropSearchKey("Poção_MÁGICA") != "pocao magica" {
		t.Fatal("name folding failed")
	}
	if dropSearchKey("  Poção_\t_MÁGICA \r\n") != "pocao magica" {
		t.Fatal("query whitespace normalization differs from the localized client mask")
	}
}
