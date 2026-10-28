// runetrack759 extracts event populations into a staging directory.
// Identical adapted templates reuse their canonical catalog identities.
package main

import (
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"wydgo/internal/model"
)

var aliases = map[string]string{
	"Archer_Elf": "Archer_Elf", "Barlog": "RT_Balrog", "Bragi": "Bragi",
	"CabuncleKing": "RT_BunnyKing", "DarkShadow": "RT_Shadow", "Fighter_Elf": "Fighter_Elf",
	"Freyja": "Freyja", "GoldCabuncle": "GoldCabuncle", "Heimdall": "Heimdall",
	"Hell_Rujeper": "RT_HellLord", "Hell_Rujeper_": "RT_HellLord2", "HellAmonChief": "HellAmonChief",
	"HellAmonWarrior": "HellAmonWarrior", "HellBatorero": "HellBatorero", "HellLich_": "RT_HellLich",
	"HellRanger": "RT_Ranger", "HellShaman": "RT_Shaman", "HellSorcerer": "RT_Sorcerer",
	"HellTemplar": "RT_Templar", "HellZakum": "HellZakum", "HellZakum_": "HellZakum_",
	"KalintzFight": "RT_Knight", "KalintzMage": "KalintzMage", "LichCruntKing": "RT_LichKing",
	"MageKalintz": "RT_MageBoss", "Sulrang": "RT_Sulrang", "ThorTower": "ThorTower",
	"Valdor": "Valdor", "Valkyrier": "RT_Valkyrie", "Vidar": "Vidar",
}

func decode(name string, b []byte) (model.NPCDef, error) {
	if len(b) != 1024 {
		return model.NPCDef{}, fmt.Errorf("%s: expected the 1024-byte 7.59 layout, got %d", name, len(b))
	}
	u16 := func(off int) uint32 { return uint32(binary.LittleEndian.Uint16(b[off:])) }
	u32 := func(off int) uint32 { return binary.LittleEndian.Uint32(b[off:]) }
	item := func(off int) (model.Item, error) {
		it := model.Item{Index: uint16(u16(off))}
		copy(it.Eff[:], b[off+2:off+8])
		if int(it.Index) >= model.ItemListSize {
			return it, fmt.Errorf("%s: invalid item %d at offset %d", name, it.Index, off)
		}
		if it.Index == 0 {
			it = model.Item{}
		}
		return it, nil
	}
	// The widened 48-byte Status starts at 92, not the old 28-byte block at 64.
	s := &model.Score{Version: model.ScoreVersion, Level: u32(92), Defense: u32(96), Attack: u32(100),
		Merchant: uint32(b[17]), AttackRun: uint32(b[105]), MaxHP: u32(108), MaxMP: u32(112),
		CurHP: u32(108), CurMP: u32(112), Str: u16(124), Int: u16(126), Dex: u16(128), Con: u16(130),
		Mastery:  [4]uint32{u16(132), u16(134), u16(136), u16(138)},
		Critical: uint32(b[794]), SaveMana: uint32(b[795]), MagicAttack: uint32(b[801]),
		RegenHP: uint32(b[802]), RegenMP: uint32(b[803]), ResistFire: uint32(b[804]),
		ResistIce: uint32(b[805]), ResistHoly: uint32(b[806]), ResistThunder: uint32(b[807])}
	if err := s.Validate(); err != nil {
		return model.NPCDef{}, fmt.Errorf("%s: %w", name, err)
	}
	var eq [16]model.Item
	for i := range eq {
		var err error
		eq[i], err = item(140 + i*8)
		if err != nil {
			return model.NPCDef{}, err
		}
	}
	if binary.LittleEndian.Uint64(b[32:]) > uint64(^uint32(0)) || u32(784) != 0 {
		return model.NPCDef{}, fmt.Errorf("%s: EXP or learned skills exceed the current server representation", name)
	}
	d := model.NPCDef{Name: name, Tipo: model.TipoMonstro, ClassInfo: b[20], Score: s,
		Gold: u32(24), ExpReward: u32(32), Direction: b[104] & 0xf0, LearnedSkill: u32(780),
		SkillBar: [4]byte{b[796], b[797], b[798], b[799]},
		Equip: model.Equip{Rosto: eq[0], Elmo: eq[1], Armadura: eq[2], Calca: eq[3], Luva: eq[4], Bota: eq[5],
			Arma: eq[6], Escudo: eq[7], Acessorio1: eq[8], Acessorio2: eq[9], Acessorio3: eq[10],
			Acessorio4: eq[11], Acessorio5: eq[12], Acessorio6: eq[13], Montaria: eq[14], Capa: eq[15]}}
	inv := make([]model.Item, 64)
	last := -1
	for i := range inv {
		var err error
		inv[i], err = item(268 + 8*i)
		if err != nil {
			return model.NPCDef{}, err
		}
		if inv[i].Index != 0 {
			last = i
		}
	}
	if s.Merchant != 0 {
		d.Tipo = model.TipoNPC
		d.Vende = inv[:last+1]
	} else {
		d.Carry = inv[:last+1]
	}
	return d, nil
}

func run(source, output string) error {
	input, err := os.ReadFile(filepath.Join(source, "NPCGener.txt"))
	if err != nil {
		return err
	}
	header := regexp.MustCompile(`(?m)^#\s*\[\s*(\d+)\][^\r\n]*`)
	fields := regexp.MustCompile(`^\s*(Leader|Follower|MinuteGenerate|MaxNumMob|MinGroup|MaxGroup|RouteType|Formation|(?:Start|Segment[123]|Dest)(?:X|Y|Range|Wait|Action)):\s*(\S+)`)
	indices := header.FindAllSubmatchIndex(input, -1)
	var rooms [7]strings.Builder
	for room := range rooms {
		fmt.Fprintf(&rooms[room], "// Rune Track +%d: instance-only population; labels and routes are preserved.\n", room)
	}
	used := map[string]bool{}
	seenGenerators := map[int]bool{}
	count := 0
	for i, h := range indices {
		var id int
		if _, err := fmt.Sscanf(string(input[h[2]:h[3]]), "%d", &id); err != nil {
			return fmt.Errorf("invalid generator ID %q: %w", input[h[2]:h[3]], err)
		}
		if id < 5662 || id > 5988 {
			continue
		}
		if seenGenerators[id] {
			return fmt.Errorf("generator %d is duplicated", id)
		}
		seenGenerators[id] = true
		end := len(input)
		if i+1 < len(indices) {
			end = indices[i+1][0]
		}
		text := &rooms[model.RuneTrackGeneratorRoom(id)]
		fmt.Fprintf(text, "\n#[%d]\n", id)
		for _, line := range strings.Split(string(input[h[1]:end]), "\n") {
			m := fields.FindStringSubmatch(line)
			if m == nil {
				continue
			}
			value := m[2]
			if m[1] == "Leader" || m[1] == "Follower" {
				if value == "0" {
					continue
				}
				alias, ok := aliases[value]
				if !ok {
					return fmt.Errorf("generator %d: missing alias for %q", id, value)
				}
				used[value] = true
				value = alias
			}
			fmt.Fprintf(text, "%s: %s\n", m[1], value)
		}
		count++
	}
	if count != 327 {
		return fmt.Errorf("expected 327 generators, got %d", count)
	}
	if err := os.MkdirAll(output, 0755); err != nil {
		return err
	}
	manifest := map[string]string{}
	names := make([]string, 0, len(used))
	for name := range used {
		names = append(names, name)
	}
	sort.Strings(names)
	for _, name := range names {
		b, err := os.ReadFile(filepath.Join(source, "npc", name))
		if err != nil {
			return err
		}
		d, err := decode(aliases[name], b)
		if err != nil {
			return err
		}
		if len(d.Name) > 12 {
			return fmt.Errorf("alias %q exceeds the wire name budget", d.Name)
		}
		data, err := json.MarshalIndent(d, "", "  ")
		if err != nil {
			return err
		}
		if err = os.WriteFile(filepath.Join(output, d.Name+".json"), append(data, '\n'), 0644); err != nil {
			return err
		}
		hash := sha256.Sum256(b)
		manifest[name] = hex.EncodeToString(hash[:])
	}
	for room := range rooms {
		if err = os.WriteFile(filepath.Join(output, fmt.Sprintf("RuneTrack%d.txt", room)), []byte(rooms[room].String()), 0644); err != nil {
			return err
		}
	}
	m, _ := json.MarshalIndent(manifest, "", "  ")
	if err = os.WriteFile(filepath.Join(output, "sources.json"), append(m, '\n'), 0644); err != nil {
		return err
	}
	fmt.Printf("Extracted %d event-only NPC definitions and %d generator blocks.\n", len(names), count)
	return nil
}

func main() {
	source := flag.String("source", "", "Explicit historical TMSRV2/run directory (read-only)")
	output := flag.String("output", "", "Explicit staging directory; does not install data")
	hellGate := flag.Bool("hell-gate", false, "Extract the Hell Gate infantry from TMSRV/run instead")
	colosseum := flag.Bool("colosseum", false, "Extract the Colosseum population from TMSRV/run instead")
	flag.Parse()
	if *source == "" || *output == "" {
		fmt.Fprintln(os.Stderr, "Both -source and -output are required")
		os.Exit(2)
	}
	extract := run
	if *hellGate {
		extract = runHellGate
	}
	if *colosseum {
		extract = runColosseum
	}
	if err := extract(*source, *output); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}
