package model

import "testing"

func TestNPCClientMerchantProjectionKeepsServerFunction(t *testing.T) {
	ui := byte(11)
	npc := NPCDef{Tipo: TipoNPC, Direction: 0xA0, ClientMerchant: &ui,
		Score: &Score{Version: ScoreVersion, Merchant: 74, MaxHP: 100, MaxMP: 20}}
	projected := npc.MakeScore(150)
	if projected.Merchant != 0xAB || projected.CurHP != 100 || projected.CurMP != 20 {
		t.Fatalf("unexpected projection: %+v", projected)
	}
	if npc.Score.Merchant != 74 || npc.Score.CurHP != 0 {
		t.Fatal("projection changed the authoritative template")
	}
	npc.ClientMerchant = nil
	if got := npc.MakeScore(50).Merchant; got != 0xAA {
		t.Fatalf("default selector = %#x, want %#x", got, 0xAA)
	}
	ui = 0
	npc.ClientMerchant = &ui
	if got := npc.MakeScore(50).Merchant; got != 0xA0 {
		t.Fatalf("explicit zero selector = %#x, want %#x", got, 0xA0)
	}
}
