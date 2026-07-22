// Comando npcconvert -- converte os arquivos de mob binarios do TMSrv 7.54
// (STRUCT_MOB de 756 bytes, um por arquivo na pasta npc/ do Micronics) para o
// formato JSON de NPCDef do wyd-go.
//
// Layout do STRUCT_MOB 7.54 (Secrets/BaseStruct.h, truncado em 756B no arquivo):
//
//	Name[16]@0, Merchant@17, ClassInfo@20, Gold@24, Exp@28, LastPos@32,
//	bStatus@36, Status@64 (a fonte autoritativa: ReadMob faz BaseScore=Status),
//	Equip[16]@92, Inventory[64]@220, ... Resist1-4@752-755.
//
// STRUCT_STATUS (28B): Level@0 Defense@2 Attack@4 Merchant@6 ChaosRate@7
//
//	maxHP@8 maxMP@10 curHP@12 curMP@14 STR@16 INT@18 DEX@20 CON@22 masters@24.
//
// STRUCT_ITEM (8B): Index@0 + 6 bytes de efeito.
//
// Gold/Exp possuem lixo em parte da distribuicao; valores dentro dos tetos
// documentados sao preservados e os demais zerados. A saida fica numa pasta
// separada para preservar os NPCs curados a mao em data/npcs.
package main

import (
	"encoding/binary"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"

	"wydgo/internal/model"
)

const mobFileSize = 756

const (
	maxSaneNPCGold = 100_000
	maxSaneNPCExp  = 10_000_000
)

const (
	offStatus = 64  // bloco Status (STRUCT_STATUS autoritativo)
	offEquip  = 92  // Equip[16]
	offCarry  = 220 // Inventory[64] (tabela de drop / estoque)
	offResist = 752 // Resist1..4
)

func u16(b []byte, off int) uint16 { return binary.LittleEndian.Uint16(b[off : off+2]) }

// cstr le ate o primeiro NUL.
func cstr(b []byte) string {
	if i := indexByte(b, 0); i >= 0 {
		return string(b[:i])
	}
	return string(b)
}

func indexByte(b []byte, c byte) int {
	for i := range b {
		if b[i] == c {
			return i
		}
	}
	return -1
}

// item le um STRUCT_ITEM (8B) em off.
func item(b []byte, off int) model.Item {
	var it model.Item
	it.Index = u16(b, off)
	copy(it.Eff[:], b[off+2:off+8])
	return it
}

// inventoryTrim devolve os 64 slots cortados ate o ultimo item ocupado,
// PRESERVANDO buracos e os tres pares de efeito. Tanto drops quanto produtos
// de loja usam esses efeitos (quantidade, refino, opcoes e marcadores nativos).
func inventoryTrim(b []byte) ([]model.Item, int) {
	items := make([]model.Item, 64)
	last := -1
	invalid := 0
	for i := 0; i < 64; i++ {
		it := item(b, offCarry+i*8)
		switch {
		case it.Index == 0:
			it = model.Item{}
		case int(it.Index) >= model.ItemListSize:
			// Quatro arquivos da distribuicao Micronics possuem palavras
			// residuais (11k..23k) no meio do Inventory. Nao sao item IDs.
			it = model.Item{}
			invalid++
		}
		items[i] = it
		if items[i].Index != 0 {
			last = i
		}
	}
	if last < 0 {
		return nil, invalid
	}
	return items[:last+1], invalid
}

type conversionReport struct {
	invalidItems  int
	discardedGold int
	discardedExp  int
}

// convert monta um NPCDef a partir dos 756 bytes do STRUCT_MOB.
func convert(name string, b []byte) (model.NPCDef, conversionReport) {
	s := offStatus
	var report conversionReport
	// STRUCT_MOB.Merchant@17 e a funcao server-side real. Status.Merchant
	// mistura o nibble baixo dessa funcao com a direcao visual no nibble alto.
	// Usar Status aqui transformava Kibita 43 em 251 e perdia o dispatch nativo.
	merchant := b[17]
	direction := b[s+6] & 0xF0
	tipo := model.TipoMonstro
	if merchant != 0 {
		tipo = model.TipoNPC // tem tipo de NPC (loja/quest/guarda) -> nao atacavel
	}

	var eq [16]model.Item
	for i := 0; i < 16; i++ {
		eq[i] = item(b, offEquip+i*8)
		if int(eq[i].Index) >= model.ItemListSize {
			eq[i] = model.Item{}
			report.invalidItems++
		}
	}
	equip := model.Equip{
		Rosto: eq[0], Elmo: eq[1], Armadura: eq[2], Calca: eq[3], Luva: eq[4],
		Bota: eq[5], Arma: eq[6], Escudo: eq[7],
		Acessorio1: eq[8], Acessorio2: eq[9], Acessorio3: eq[10],
		Acessorio4: eq[11], Acessorio5: eq[12], Acessorio6: eq[13],
		Montaria: eq[14], Capa: eq[15],
	}

	gold := binary.LittleEndian.Uint32(b[24:28])
	if gold > maxSaneNPCGold {
		gold = 0
		report.discardedGold++
	}
	exp := binary.LittleEndian.Uint32(b[28:32])
	if exp > maxSaneNPCExp {
		exp = 0
		report.discardedExp++
	}

	def := model.NPCDef{
		Name:      name,
		Tipo:      tipo,
		Equip:     equip,
		ClassInfo: b[20],
		Extended: &model.ExtendedScore{
			Version:       model.ExtendedScoreVersion,
			Level:         uint32(u16(b, s+0)),
			Defense:       uint32(u16(b, s+2)),
			Attack:        uint32(u16(b, s+4)),
			MagicAttack:   uint32(b[749]),
			MaxHP:         uint32(u16(b, s+8)),
			MaxMP:         uint32(u16(b, s+10)),
			CurHP:         uint32(u16(b, s+8)),
			CurMP:         uint32(u16(b, s+10)),
			Str:           uint32(u16(b, s+16)),
			Int:           uint32(u16(b, s+18)),
			Dex:           uint32(u16(b, s+20)),
			Con:           uint32(u16(b, s+22)),
			Mastery:       [4]uint32{uint32(b[s+24]), uint32(b[s+25]), uint32(b[s+26]), uint32(b[s+27])},
			Merchant:      merchant,
			AttackRun:     b[s+7], // ChaosRate no 7.54 = AttackRun no wyd-go
			Critical:      uint32(b[742]),
			SaveMana:      uint32(b[743]),
			RegenHP:       uint32(b[750]),
			RegenMP:       uint32(b[751]),
			ResistFire:    uint32(b[offResist]),
			ResistIce:     uint32(b[offResist+1]),
			ResistHoly:    uint32(b[offResist+2]),
			ResistThunder: uint32(b[offResist+3]),
		},
		ExpReward:    exp,
		Gold:         gold,
		Direction:    direction,
		LearnedSkill: binary.LittleEndian.Uint32(b[732:736]),
		SkillBar:     [4]byte{b[744], b[745], b[746], b[747]},
	}

	// O Inventory[64] do mob e a tabela de drop (monstro) OU o estoque de loja (npc).
	inv, invalidInventory := inventoryTrim(b)
	report.invalidItems += invalidInventory
	if tipo == model.TipoMonstro {
		def.Carry = inv
	} else {
		def.Vende = inv
		if merchant == 19 {
			def.Vende = skillShopInventory(inv)
		}
	}
	return def, report
}

// Os mestres Micronics guardam os tres ramos nas paginas 0, 27 e 54 do
// Inventory[64]. O client 7.48, porem, abre uma unica grade de 27 posicoes:
// 8 skills + separador, repetido tres vezes.
func skillShopInventory(inv []model.Item) []model.Item {
	out := make([]model.Item, 27)
	classStart := uint16(0)
	if len(inv) > 0 && inv[0].Index >= 5000 && inv[0].Index <= 5095 {
		classStart = 5000 + ((inv[0].Index-5000)/24)*24
	}
	for branch, source := range []int{0, 27, 54} {
		for i := 0; i < 8; i++ {
			if source+i < len(inv) {
				out[branch*9+i] = inv[source+i]
			}
			// Cada classe possui exatamente 24 livros. A distribuicao Micronics
			// omite o Evilator (5071) do Mestre_Archi; complete lacunas dos 24
			// slots utilizaveis sem tocar nos separadores 8/17/26.
			if out[branch*9+i].Index == 0 && classStart != 0 {
				out[branch*9+i].Index = classStart + uint16(branch*8+i)
			}
		}
	}
	return out
}

// safeFileName transforma o nome do mob num nome de arquivo estavel.
func safeFileName(name string) string {
	r := strings.NewReplacer("/", "_", "\\", "_", ":", "_", "*", "_", "?", "_",
		"\"", "_", "<", "_", ">", "_", "|", "_")
	out := strings.TrimSpace(r.Replace(name))
	if out == "" {
		out = "sem_nome"
	}
	return out
}

func main() {
	in := flag.String("in", "../Server Star Micronics/TMSRV/run/npc", "pasta de arquivos de mob binarios do Micronics")
	out := flag.String("out", "data/npcs-micronics", "pasta de saida dos JSON")
	flag.Parse()

	entries, err := os.ReadDir(*in)
	if err != nil {
		log.Fatalf("ler pasta de entrada %s: %v", *in, err)
	}
	if err := os.MkdirAll(*out, 0o755); err != nil {
		log.Fatalf("criar pasta de saida %s: %v", *out, err)
	}

	converted, skipped := 0, 0
	var report conversionReport
	names := map[string]int{}
	for _, e := range entries {
		if e.IsDir() {
			continue
		}
		name := e.Name()
		if strings.EqualFold(filepath.Ext(name), ".dll") {
			continue
		}
		path := filepath.Join(*in, name)
		b, err := os.ReadFile(path)
		if err != nil {
			log.Printf("SKIP %s: %v", name, err)
			skipped++
			continue
		}
		if len(b) < mobFileSize {
			log.Printf("SKIP %s: %d bytes (< %d, nao e STRUCT_MOB)", name, len(b), mobFileSize)
			skipped++
			continue
		}
		def, convertedReport := convert(name, b[:mobFileSize])
		report.invalidItems += convertedReport.invalidItems
		report.discardedGold += convertedReport.discardedGold
		report.discardedExp += convertedReport.discardedExp
		names[def.Tipo]++

		data, err := json.MarshalIndent(def, "", "  ")
		if err != nil {
			log.Printf("SKIP %s: marshal: %v", name, err)
			skipped++
			continue
		}
		dst := filepath.Join(*out, safeFileName(name)+".json")
		if err := os.WriteFile(dst, append(data, '\n'), 0o644); err != nil {
			log.Printf("SKIP %s: gravar: %v", name, err)
			skipped++
			continue
		}
		converted++
	}
	fmt.Printf("convertidos=%d (monstro=%d npc=%d) pulados=%d -> %s\n",
		converted, names[model.TipoMonstro], names[model.TipoNPC], skipped, *out)
	if report.invalidItems > 0 {
		fmt.Printf("indices de item invalidos removidos=%d (limite ItemList=%d)\n",
			report.invalidItems, model.ItemListSize)
	}
	fmt.Printf("recompensas corrompidas zeradas: gold=%d exp=%d\n",
		report.discardedGold, report.discardedExp)
}
