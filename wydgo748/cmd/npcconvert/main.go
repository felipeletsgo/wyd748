// npcconvert reads the exact 756-byte legacy or 1024-byte 7.59 mob layout.
// It writes candidates to a separate directory; it does not choose gameplay
// roles, replace live generators, or establish client-version compatibility.
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
	offStatus = 64  // authoritative STRUCT_STATUS
	offEquip  = 92  // Equip[16]
	offCarry  = 220 // Inventory[64] (drops or shop stock)
	offResist = 752 // Resist1..4
)

func u16(b []byte, off int) uint16 { return binary.LittleEndian.Uint16(b[off : off+2]) }

// item reads one eight-byte STRUCT_ITEM at off.
func item(b []byte, off int) model.Item {
	var it model.Item
	it.Index = u16(b, off)
	copy(it.Eff[:], b[off+2:off+8])
	return it
}

// inventoryTrim retains slots through the last occupied item, preserving holes
// and all three effect pairs used by drops and shop products.
func inventoryTrim(b []byte) ([]model.Item, int) {
	return inventoryAt(b, offCarry)
}

func inventoryAt(b []byte, offset int) ([]model.Item, int) {
	items := make([]model.Item, 64)
	last := -1
	invalid := 0
	for i := 0; i < 64; i++ {
		it := item(b, offset+i*8)
		switch {
		case it.Index == 0:
			it = model.Item{}
		case int(it.Index) >= model.ItemListSize:
			// Four legacy distribution records contain residual words
			// (11k..23k) in Inventory, outside the item table.
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

// convert builds an NPCDef from the 756-byte legacy STRUCT_MOB.
func convert(name string, b []byte) (model.NPCDef, conversionReport) {
	s := offStatus
	var report conversionReport
	// STRUCT_MOB.Merchant@17 is the full server function. Status.Merchant
	// combines its low nibble with the visual direction in the high nibble.
	// Reading Status as the function changed Kibita 43 to 251.
	merchant := b[17]
	direction := b[s+6] & 0xF0
	tipo := model.TipoMonstro
	if merchant != 0 {
		tipo = model.TipoNPC // service NPCs are not attackable
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
		Score: &model.Score{
			Version:       model.ScoreVersion,
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
			Merchant:      uint32(merchant),
			AttackRun:     uint32(b[s+7]), // legacy ChaosRate maps to AttackRun
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

	// Inventory[64] is either monster drops or NPC shop stock.
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

// decodeNPC selects by exact size. Truncating a 7.59 record to 756 bytes
// would read its position/base status as equipment and its equipment as stock.
func decodeNPC(name string, b []byte) (model.NPCDef, conversionReport, error) {
	if len(b) == mobFileSize {
		def, report := convert(name, b)
		return def, report, nil
	}
	if len(b) != 1024 {
		return model.NPCDef{}, conversionReport{}, fmt.Errorf("%s: unsupported mob size %d (expected 756 or 1024)", name, len(b))
	}
	u32 := func(off int) uint32 { return binary.LittleEndian.Uint32(b[off:]) }
	if u32(784) != 0 {
		return model.NPCDef{}, conversionReport{}, fmt.Errorf("%s: upper learned-skill word is not representable in the 7.48 contract", name)
	}
	exp := binary.LittleEndian.Uint64(b[32:40])
	if exp > uint64(^uint32(0)) {
		return model.NPCDef{}, conversionReport{}, fmt.Errorf("%s: EXP %d exceeds the current representation", name, exp)
	}
	score := &model.Score{
		Version: model.ScoreVersion, Level: u32(92), Defense: u32(96), Attack: u32(100),
		Merchant: uint32(b[17]), AttackRun: uint32(b[105]), MaxHP: u32(108), MaxMP: u32(112),
		CurHP: u32(108), CurMP: u32(112), Str: uint32(u16(b, 124)), Int: uint32(u16(b, 126)),
		Dex: uint32(u16(b, 128)), Con: uint32(u16(b, 130)),
		Mastery:  [4]uint32{uint32(u16(b, 132)), uint32(u16(b, 134)), uint32(u16(b, 136)), uint32(u16(b, 138))},
		Critical: uint32(b[794]), SaveMana: uint32(b[795]), MagicAttack: uint32(b[801]),
		RegenHP: uint32(b[802]), RegenMP: uint32(b[803]), ResistFire: uint32(b[804]),
		ResistIce: uint32(b[805]), ResistHoly: uint32(b[806]), ResistThunder: uint32(b[807]),
	}
	if err := score.Validate(); err != nil {
		return model.NPCDef{}, conversionReport{}, fmt.Errorf("%s: %w", name, err)
	}
	var eq [16]model.Item
	for i := range eq {
		eq[i] = item(b, 140+i*8)
		if int(eq[i].Index) >= model.ItemListSize {
			return model.NPCDef{}, conversionReport{}, fmt.Errorf("%s: unsupported equipment item %d at slot %d", name, eq[i].Index, i)
		}
		if eq[i].Index == 0 {
			eq[i] = model.Item{}
		}
	}
	inv, invalid := inventoryAt(b, 268)
	if invalid != 0 {
		return model.NPCDef{}, conversionReport{}, fmt.Errorf("%s: %d unsupported inventory items", name, invalid)
	}
	def := model.NPCDef{Name: name, Tipo: model.TipoMonstro, ClassInfo: b[20], Score: score,
		Gold: u32(24), ExpReward: uint32(exp), Direction: b[104] & 0xf0, LearnedSkill: u32(780),
		SkillBar: [4]byte{b[796], b[797], b[798], b[799]},
		Equip: model.Equip{Rosto: eq[0], Elmo: eq[1], Armadura: eq[2], Calca: eq[3], Luva: eq[4], Bota: eq[5],
			Arma: eq[6], Escudo: eq[7], Acessorio1: eq[8], Acessorio2: eq[9], Acessorio3: eq[10],
			Acessorio4: eq[11], Acessorio5: eq[12], Acessorio6: eq[13], Montaria: eq[14], Capa: eq[15]}}
	if score.Merchant == 0 {
		def.Carry = inv
	} else {
		def.Tipo = model.TipoNPC
		def.Vende = inv
		if score.Merchant == 19 {
			def.Vende = skillShopInventory(inv)
		}
	}
	return def, conversionReport{}, nil
}

// Skill masters store the three branches at Inventory offsets 0, 27 and 54.
// The supported client displays one 27-slot grid: eight skills and a separator
// for each branch.
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
			// Each class has 24 books. The legacy Mestre_Archi record omits
			// Evilator (5071); fill usable gaps without changing separators.
			if out[branch*9+i].Index == 0 && classStart != 0 {
				out[branch*9+i].Index = classStart + uint16(branch*8+i)
			}
		}
	}
	return out
}

// safeFileName produces a stable filename without changing stored names.
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
	in := flag.String("in", "", "input directory containing binary mob records (required)")
	out := flag.String("out", "", "separate candidate JSON output directory (required)")
	only := flag.String("only", "", "comma-separated exact input filenames; fail if any selected record cannot be converted")
	flag.Parse()
	if strings.TrimSpace(*in) == "" || strings.TrimSpace(*out) == "" {
		log.Fatal("provide -in and -out explicitly")
	}
	selected := make(map[string]bool)
	if *only != "" {
		for _, name := range strings.Split(*only, ",") {
			selected[strings.TrimSpace(name)] = false
		}
	}

	entries, err := os.ReadDir(*in)
	if err != nil {
		log.Fatalf("read input directory %s: %v", *in, err)
	}
	if err := os.MkdirAll(*out, 0o755); err != nil {
		log.Fatalf("create output directory %s: %v", *out, err)
	}

	converted, skipped := 0, 0
	var report conversionReport
	names := map[string]int{}
	for _, e := range entries {
		if e.IsDir() {
			continue
		}
		name := e.Name()
		if len(selected) != 0 {
			if _, ok := selected[name]; !ok {
				continue
			}
		}
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
		def, convertedReport, err := decodeNPC(name, b)
		if err != nil {
			log.Printf("SKIP %s: %v", name, err)
			skipped++
			continue
		}
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
			log.Printf("SKIP %s: write: %v", name, err)
			skipped++
			continue
		}
		converted++
		if len(selected) != 0 {
			selected[name] = true
		}
	}
	fmt.Printf("converted=%d (monster=%d npc=%d) skipped=%d -> %s\n",
		converted, names[model.TipoMonstro], names[model.TipoNPC], skipped, *out)
	if report.invalidItems > 0 {
		fmt.Printf("removed invalid item indexes=%d (ItemList limit=%d)\n",
			report.invalidItems, model.ItemListSize)
	}
	fmt.Printf("cleared corrupt legacy rewards: gold=%d exp=%d\n",
		report.discardedGold, report.discardedExp)
	for name, done := range selected {
		if !done {
			log.Fatalf("selected record %q was not converted", name)
		}
	}
}
