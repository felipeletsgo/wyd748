package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"
)

// colosseumAliases keep the 7.59 Colosseum monsters independent from the
// ordinary catalog and within the 12-byte wire name budget.
var colosseumAliases = map[string]string{
	"ForceCyclops": "COL_FCyclops", "SavageCyclops": "COL_SCyclops", "Ciclope_Arqs": "COL_CycArch",
	"WildCyclops": "COL_WCyclops", "MageCyclops": "COL_MCyclops", "OrcSniper_": "COL_OSniper",
	"ForceOrc": "COL_FOrc", "SavageOrc": "COL_SOrc", "OrcMage": "COL_OMage", "WildOrc": "COL_WOrc",
	"ForceCyclop_": "COL_FCyclop2", "SavageCyclops_": "COL_SCyclop2", "Ciclope_Arq__": "COL_CycArch2",
	"MightyTalos": "COL_Talos", "WildCyclops_": "COL_WCyclop2", "Cav_Lugefer_": "COL_Lugefer",
}

// colosseumFiles maps each Colosseum hour to its 7.59 generator labels:
// EventHour+1 (mortals) 5..7, EventHour+2 (no Celestials) 0..2 and
// EventHour+3 (item 3170) 103..106 (TMSRVIN sub_567780).
var colosseumFiles = []struct {
	name, title string
	labels      []int
}{
	{"ColiseuN.txt", "Colosseum N (EventHour+1, mortals only)", []int{5, 6, 7}},
	{"ColiseuM.txt", "Colosseum M (EventHour+2, no Celestials)", []int{0, 1, 2}},
	{"ColiseuA.txt", "Colosseum A (EventHour+3, item 3170)", []int{103, 104, 105, 106}},
}

func runColosseum(source, output string) error {
	input, err := os.ReadFile(filepath.Join(source, "NPCGener.txt"))
	if err != nil {
		return err
	}
	header := regexp.MustCompile(`(?m)^#\s*\[\s*(\d+)\][^\r\n]*`)
	fields := regexp.MustCompile(`^\s*(Leader|Follower|MinuteGenerate|MaxNumMob|MinGroup|MaxGroup|RouteType|Formation|(?:Start|Segment[123]|Dest)(?:X|Y|Range|Wait|Action)):\s*(\S+)`)
	blocks := map[int]string{}
	indices := header.FindAllSubmatchIndex(input, -1)
	for i, h := range indices {
		var id int
		if _, err := fmt.Sscanf(string(input[h[2]:h[3]]), "%d", &id); err != nil {
			return fmt.Errorf("invalid generator ID %q: %w", input[h[2]:h[3]], err)
		}
		end := len(input)
		if i+1 < len(indices) {
			end = indices[i+1][0]
		}
		if _, exists := blocks[id]; exists {
			return fmt.Errorf("generator %d is duplicated", id)
		}
		blocks[id] = string(input[h[1]:end])
	}
	if err := os.MkdirAll(output, 0755); err != nil {
		return err
	}
	used := map[string]bool{}
	for _, file := range colosseumFiles {
		var text strings.Builder
		fmt.Fprintf(&text, "// %s: 7.59 TMSRV generators; labels and route values are preserved.\n", file.title)
		fmt.Fprintf(&text, "// Event-only: MinuteGenerate -1 never spawns in the open world.\n")
		for _, label := range file.labels {
			block, ok := blocks[label]
			if !ok {
				return fmt.Errorf("generator %d is missing", label)
			}
			fmt.Fprintf(&text, "\n#\t[%d]\n", label)
			for _, line := range strings.Split(block, "\n") {
				m := fields.FindStringSubmatch(line)
				if m == nil {
					continue
				}
				value := m[2]
				if m[1] == "Leader" || m[1] == "Follower" {
					alias, ok := colosseumAliases[value]
					if !ok {
						return fmt.Errorf("generator %d: missing alias for %q", label, value)
					}
					used[value] = true
					value = alias
				}
				fmt.Fprintf(&text, "\t%s:\t%s\n", m[1], value)
			}
		}
		if err := os.WriteFile(filepath.Join(output, file.name), []byte(text.String()), 0644); err != nil {
			return err
		}
	}
	manifest := map[string]string{}
	for name := range used {
		b, err := os.ReadFile(filepath.Join(source, "npc", name))
		if err != nil {
			return err
		}
		d, err := decode(colosseumAliases[name], b)
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
		if err := os.WriteFile(filepath.Join(output, d.Name+".json"), append(data, '\n'), 0644); err != nil {
			return err
		}
		hash := sha256.Sum256(b)
		manifest[name] = hex.EncodeToString(hash[:])
	}
	data, _ := json.MarshalIndent(manifest, "", "  ")
	if err := os.WriteFile(filepath.Join(output, "colosseum-sources.json"), append(data, '\n'), 0644); err != nil {
		return err
	}
	fmt.Printf("Extracted %d Colosseum NPC definitions into three generator files.\n", len(used))
	return nil
}
