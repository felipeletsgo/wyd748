package model

import (
	"encoding/json"
	"testing"
)

func TestAccountUnmarshalRequiresExactCargoAndRejectsUnknownFields(t *testing.T) {
	payload := struct {
		Name         string `json:"name"`
		PasswordHash string `json:"passwordHash"`
		Chars        []Char `json:"chars"`
		Cargo        []Item `json:"cargo"`
	}{
		Name: "account", PasswordHash: "hash", Cargo: make([]Item, MaxCargo),
	}
	payload.Cargo[0] = Item{Index: 4011}
	raw, err := json.Marshal(payload)
	if err != nil {
		t.Fatal(err)
	}
	var acc Account
	if err := json.Unmarshal(raw, &acc); err != nil {
		t.Fatalf("cargo estrutural valido foi recusado: %v", err)
	}
	if acc.Name != "account" || acc.Cargo[0].Index != 4011 {
		t.Fatalf("round-trip da conta incompleto: name=%q cargo0=%d", acc.Name, acc.Cargo[0].Index)
	}

	payload.Cargo = make([]Item, MaxCargo-1)
	raw, _ = json.Marshal(payload)
	if err := json.Unmarshal(raw, &acc); err == nil {
		t.Fatal("cargo estrutural curto foi aceito")
	}

	unknown := append([]byte(nil), raw...)
	_ = unknown
	validMap := map[string]any{
		"name": "account", "passwordHash": "hash", "chars": []any{},
		"cargo": make([]Item, MaxCargo), "unexpected": true,
	}
	raw, _ = json.Marshal(validMap)
	if err := json.Unmarshal(raw, &acc); err == nil {
		t.Fatal("campo desconhecido na conta foi aceito")
	}
}

func TestEquipAdaptersPreserveAllSixteenSlots(t *testing.T) {
	var equip Equip
	items := []*Item{
		&equip.Rosto, &equip.Elmo, &equip.Armadura, &equip.Calca, &equip.Luva, &equip.Bota,
		&equip.Arma, &equip.Escudo, &equip.Acessorio1, &equip.Acessorio2, &equip.Acessorio3,
		&equip.Acessorio4, &equip.Acessorio5, &equip.Acessorio6, &equip.Montaria, &equip.Capa,
	}
	for i := range items {
		items[i].Index = uint16(100 + i)
	}
	// A montaria usa uma faixa própria para que VisualItemCode trate o slot 14
	// pelo ramo de mount em vez de como item comum.
	equip.Montaria.Index = MountAdultBase
	equip.Montaria.SetMountHP(100)
	equip.Arma.Eff = [6]byte{43, 1, 0, 0, 0, 0}

	flat := equip.Items()
	if flat[0].Index != 100 || flat[6].Index != 106 || flat[14].Index != MountAdultBase || flat[15].Index != 115 {
		t.Fatalf("Equip.Items perdeu ordem ABI: %+v", flat)
	}
	slots := equip.Slots()
	if slots[0] != VisualItemCode(equip.Rosto, false) || slots[14] != VisualItemCode(equip.Montaria, true) {
		t.Fatalf("Equip.Slots divergiu do adapter visual: %v", slots)
	}
	anct := equip.AncientCodes()
	if anct[6] != AncientCode(equip.Arma) || anct[14] != AncientCode(equip.Montaria) {
		t.Fatalf("Equip.AncientCodes divergiu: %v", anct)
	}
}

func TestNPCMeshIsExactlyEquipSlots(t *testing.T) {
	npc := &NPCDef{}
	npc.Equip.Rosto.Index = 10
	npc.Equip.Arma.Index = 20
	mesh := npc.Mesh()
	if len(mesh) != 16 || mesh[0] != VisualItemCode(npc.Equip.Rosto, false) || mesh[6] != VisualItemCode(npc.Equip.Arma, false) {
		t.Fatalf("NPC.Mesh=%v", mesh)
	}
}
