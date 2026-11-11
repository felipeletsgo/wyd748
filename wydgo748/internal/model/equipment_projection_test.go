package model

import "testing"

func TestEquipProjectsAllSixteenSlotsAndAncientCodes(t *testing.T) {
	items := [16]Item{}
	for i := range items {
		items[i].Index = uint16(100 + i)
	}
	items[0].Eff = [6]byte{43, 8}
	items[14].Index = MountAdultBase
	items[14].SetMountHP(300)
	items[14].Eff[2] = 10
	items[15] = Item{Index: 200, Eff: [6]byte{115, 232}}

	equip := Equip{
		Rosto: items[0], Elmo: items[1], Armadura: items[2], Calca: items[3],
		Luva: items[4], Bota: items[5], Arma: items[6], Escudo: items[7],
		Acessorio1: items[8], Acessorio2: items[9], Acessorio3: items[10],
		Acessorio4: items[11], Acessorio5: items[12], Acessorio6: items[13],
		Montaria: items[14], Capa: items[15],
	}
	if got := equip.Items(); got != items {
		t.Fatalf("Equip.Items order differs from its sixteen named slots: %#v", got)
	}
	slots := equip.Slots()
	if slots[0] != 0x8000+100 || slots[14] != uint16(MountAdultBase)+0x1000 || slots[15] != 0xA000+200 {
		t.Fatalf("projected equipment slots = [%d, %d, %d], want [%d, %d, %d]", slots[0], slots[14], slots[15], 0x8000+100, uint16(MountAdultBase)+0x1000, 0xA000+200)
	}
	ancient := equip.AncientCodes()
	if ancient[0] != 43 || ancient[14] != 0 || ancient[15] != 0x30 {
		t.Fatalf("ancient codes = [%d, %d, %d], want [43, 0, 48]", ancient[0], ancient[14], ancient[15])
	}

	equip.Montaria.SetMountHP(0)
	if got := equip.Slots()[14]; got != 0 {
		t.Fatalf("wounded mount slot projected as %d, want empty", got)
	}
}

func TestNPCNameAndCityGuardPolicies(t *testing.T) {
	npc := NPCDef{Name: "Kibita"}
	if npc.VisibleName() != "Kibita" || npc.InteractionName() != "Kibita" {
		t.Fatal("NPC default names should use its canonical name")
	}
	npc.DisplayName = "Kibita the Guide"
	npc.ServiceIdentity = "Kibita"
	if npc.VisibleName() != "Kibita the Guide" || npc.InteractionName() != "Kibita" {
		t.Fatal("display and service identity overrides were not kept separate")
	}

	if (*NPCDef)(nil).IsCityGuard() {
		t.Fatal("nil NPC was classified as a city guard")
	}
	cityGuard := NPCDef{Tipo: TipoNPC, CombatRole: "city_guard"}
	if !cityGuard.IsCityGuard() {
		t.Fatal("explicit friendly city guard was not recognized")
	}
	for _, npc := range []NPCDef{
		{Tipo: TipoNPC},
		{Tipo: TipoMonstro, CombatRole: "city_guard"},
		{Tipo: TipoNPC, CombatRole: "guard"},
	} {
		if npc.IsCityGuard() {
			t.Fatalf("non-guard NPC was classified as a city guard: %+v", npc)
		}
	}
}

func TestNPCMeshAndScoreProjection(t *testing.T) {
	npc := &NPCDef{Score: &Score{Version: 99, Merchant: 0x23, MaxHP: 500, CurHP: 400, MaxMP: 90, CurMP: 20}}
	npc.Equip.Rosto.Index = 321
	npc.Direction = 0xA7
	mesh := npc.Mesh()
	if len(mesh) != 16 || mesh[0] != 321 {
		t.Fatalf("NPC mesh length/value = %d/%d, want 16/321", len(mesh), mesh[0])
	}
	score := npc.MakeScore(600)
	if score.Version != ScoreVersion || score.CurHP != 500 || score.CurMP != 90 || score.Merchant != 0xA3 {
		t.Fatalf("projected NPC score = %+v", score)
	}
	if npc.Score.Version != 99 || npc.Score.CurHP != 400 || npc.Score.Merchant != 0x23 {
		t.Fatal("score projection modified the authoritative NPC template")
	}
	clientMerchant := byte(0x05)
	npc.ClientMerchant = &clientMerchant
	if got := npc.MakeScore(50).Merchant; got != 0xA5 {
		t.Fatalf("client merchant override produced %#x, want 0xA5", got)
	}
	if got := (*NPCDef)(nil).MakeScore(10); got == nil || got.Version != ScoreVersion || got.CurHP != 0 {
		t.Fatalf("nil NPC score fallback = %+v", got)
	}
	if got := (&NPCDef{}).MakeScore(10); got == nil || got.Version != ScoreVersion || got.CurHP != 0 {
		t.Fatalf("missing NPC attributes fallback = %+v", got)
	}
}
