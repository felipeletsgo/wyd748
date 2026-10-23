package data

import (
	"path/filepath"
	"testing"
)

// These are event objectives, not ordinary wave leaders or packs. World bosses
// have separate Lua score overrides, covered by TestConfiguredWorldBossesHaveLargeCON.
func TestProductionMonsterCONPolicy(t *testing.T) {
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	bosses := map[string]uint32{
		"COL_Lugefer":  1300,
		"RT_Balrog":    2000,
		"RT_BunnyKing": 3000,
		"RT_MageBoss":  1500,
		"RT_Valkyrie":  2500,
	}
	corrected := map[string]bool{
		"COL_SCyclop2": false, "COL_SCyclops": false, "COL_Talos": false,
		"COL_WCyclop2": false, "COL_WCyclops": false,
		"HG_CorrSkel": false, "HG_SkelArch": false, "HG_SkelGuard": false,
		"HG_SkelMage": false, "HG_SkelSpear": false,
		"RT_HellLich": false, "RT_HellLord": false, "RT_HellLord2": false,
		"RT_Knight": false, "RT_LichKing": false, "RT_Ranger": false,
		"RT_Shadow": false, "RT_Shaman": false, "RT_Sulrang": false,
		"RT_Templar": false,
	}
	seenBosses := 0
	for _, npc := range npcs {
		if !npc.IsMonster() || npc.Score == nil {
			continue
		}
		if want, ok := bosses[npc.Name]; ok {
			seenBosses++
			if npc.CombatRole != "boss" || npc.Score.Con != want {
				t.Errorf("boss %q role=%q CON=%d, want boss and %d", npc.Name, npc.CombatRole, npc.Score.Con, want)
			}
		} else if npc.Score.Con > 1000 {
			t.Errorf("ordinary monster %q CON=%d exceeds 1000", npc.Name, npc.Score.Con)
		}
		if _, ok := corrected[npc.Name]; ok {
			corrected[npc.Name] = true
			if npc.Score.Con != 100 {
				t.Errorf("corrected monster %q CON=%d, want 100", npc.Name, npc.Score.Con)
			}
		}
	}
	if seenBosses != len(bosses) {
		t.Errorf("found %d event bosses, want %d", seenBosses, len(bosses))
	}
	for name, seen := range corrected {
		if !seen {
			t.Errorf("corrected monster %q is missing", name)
		}
	}
}
