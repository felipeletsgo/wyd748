package game

import (
	"bytes"
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func channel2RegressionDefinitions(t *testing.T) map[string]*model.NPCDef {
	t.Helper()
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	if err := data.ValidateNPCItems(npcs, catalog.Items); err != nil {
		t.Fatal(err)
	}
	byID := make(map[string]*model.NPCDef, len(npcs))
	for i := range npcs {
		byID[npcs[i].Name] = &npcs[i]
	}
	return byID
}

func TestChannel2ImportedServicesCannotAttackPlayers(t *testing.T) {
	defs := channel2RegressionDefinitions(t)
	for _, name := range []string{"Agata", "Conjurer_Ehre", "Aylin", "Compositor", "Tiny", "GateKeeper", "TownWatcher", "TownWatcher_", "Mercenario", "prisang_Nell", "Conjurer_Rassel", "King_Glantuar", "King_Harabard"} {
		t.Run(name, func(t *testing.T) {
			def := defs[name]
			if def == nil {
				t.Fatal("missing imported service")
			}
			p, _ := networkedTestPlayer(1, "Visitor", 100, 100)
			m := &Mob{ID: 1000, X: 100, Y: 100, HP: 9000, TargetID: p.ID, Awake: true, Def: def}
			w := testSpatialWorld([]*Mob{m}, p)
			w.activeMobs[m.ID] = m
			hp := playerCurHP(p.Char)
			now := time.Unix(2_000_000_000, 0)
			w.tickActiveMobActions(now)
			w.mobAttackPlayer(m, p, now)
			w.applyMobDamageToPlayer(m, p, 9999, now, func(uint32) []byte {
				t.Fatal("friendly service published player damage")
				return nil
			})
			if m.TargetID != 0 || m.Awake || playerCurHP(p.Char) != hp {
				t.Fatal("service retained combat state or damaged a player")
			}
		})
	}
}

func TestChannel2ImportedArtisansResolveCraftingContext(t *testing.T) {
	defs := channel2RegressionDefinitions(t)
	for name, artisan := range map[string]string{"Agata": "Agatha", "Conjurer_Ehre": "Ehre", "AlchemyOdin": "Odin", "Aylin": "Aylin", "Compositor": "Compositor", "Tiny": "Tiny"} {
		t.Run(name, func(t *testing.T) {
			def := defs[name]
			if def == nil || def.Score == nil || def.Score.Merchant != craftingMerchant {
				t.Fatal("missing crafting selector")
			}
			if def.MakeScore(1).Merchant&0x0F != craftingMerchant {
				t.Fatal("client projection lost the crafting window selector")
			}
			p, _ := networkedTestPlayer(1, "Visitor", 100, 100)
			m := &Mob{ID: 1000, X: 101, Y: 100, Def: def}
			w := testSpatialWorld([]*Mob{m}, p)
			p.show(m.ID)
			got, err := w.combineNPC(p, artisan)
			if err != nil || got != m || p.CraftNPC != m.ID {
				t.Fatalf("crafting context: mob=%p err=%v", got, err)
			}
		})
	}
}

func TestChannel2DistinctKingVariantsKeepTheirServices(t *testing.T) {
	defs := channel2RegressionDefinitions(t)
	for _, names := range [][2]string{{"KingGlantuar", "King_Glantuar"}, {"KingHarabard", "King_Harabard"}} {
		service, decorative := defs[names[0]], defs[names[1]]
		if service == nil || decorative == nil || service == decorative {
			t.Fatalf("king variants were merged: %v", names)
		}
		if service.Score.Merchant != 15 || kingdomForNPC(service) == model.KingdomNeutral {
			t.Fatalf("kingdom service lost its handler: %s", service.Name)
		}
		if decorative.Score.Merchant != 0 || kingdomForNPC(decorative) != model.KingdomNeutral {
			t.Fatalf("decorative king acquired a progression handler: %s", decorative.Name)
		}
		if service.Score.Attack == decorative.Score.Attack {
			t.Fatalf("king variants lost their separate stats: %v", names)
		}
	}
}

func TestImportedVisibleNameSurvivesSpawnRecoveryAndReentry(t *testing.T) {
	defs := channel2RegressionDefinitions(t)
	base, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	for i := range base {
		def := &base[i]
		if def.IsMonster() {
			defs["shared:"+def.Name] = def
		}
	}
	monsters := 0
	for source, original := range defs {
		if source != "KnightLeader" && !original.IsMonster() {
			continue
		}
		if original.IsMonster() {
			monsters++
		}
		t.Run(source, func(t *testing.T) {
			for _, useDisplayName := range []bool{true, false} {
				def := *original
				want := original.VisibleName()
				if !useDisplayName {
					// Overlong internal IDs require an explicit visible label.
					if len(def.Name) > model.MaxNPCDisplayNameBytes {
						continue
					}
					def.DisplayName = ""
					want = def.Name
				}
				first, _ := networkedTestPlayer(1, "First", 2100, 2100)
				second, _ := networkedTestPlayer(2, "Second", 2101, 2100)
				w := worldWithNetworkedPlayers(first, second)
				m := &Mob{ID: 1000, X: 2102, Y: 2100, Def: &def}
				w.registerMobSpatial(m)
				request := make([]byte, 16)
				binary.LittleEndian.PutUint16(request[12:14], m.ID)
				for _, p := range []*Player{first, second} {
					for _, action := range []func(){
						func() { w.showMob(p, m) },
						func() { w.onREQMobByID(p.Session, request) },
						func() { w.hideMob(p, m, 0); w.showMob(p, m) },
						func() { p.hide(m.ID); w.onREQMobByID(p.Session, request) },
					} {
						action()
						creates := 0
						for _, packet := range labelPackets(t, p.Session) {
							if binary.LittleEndian.Uint16(packet[4:6]) != wire.OpCreateMob {
								continue
							}
							creates++
							if got := string(bytes.TrimRight(packet[18:34], "\x00")); got != want {
								t.Fatalf("visible label = %q, want %q", got, want)
							}
							if packet[33] != 0 {
								t.Fatal("NPC name has no terminating NUL in its 16-byte field")
							}
						}
						if creates != 1 {
							t.Fatalf("CreateMob packets = %d, want 1", creates)
						}
					}
				}
			}
		})
	}
	if monsters == 0 {
		t.Fatal("no monster label cases exercised")
	}
	t.Logf("verified %d monster definitions and KnightLeader", monsters)
}
