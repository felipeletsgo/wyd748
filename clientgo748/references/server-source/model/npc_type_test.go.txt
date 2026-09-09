package model

import "testing"

func TestMerchantCannotBeHostileMonster(t *testing.T) {
	def := NPCDef{Tipo: TipoMonstro, Score: &Score{Merchant: 8}}
	if def.IsMonster() {
		t.Fatal("NPC com Merchant de craft foi classificado como monstro")
	}
	def.Score.Merchant = 0
	if !def.IsMonster() {
		t.Fatal("monstro sem Merchant deixou de ser hostil")
	}
}
