package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func writeBossFile(t *testing.T, contents string) string {
	t.Helper()
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "boss.lua"), []byte(contents), 0o600); err != nil {
		t.Fatal(err)
	}
	return dir
}

const bossMinimo = `return {
  id = "test", npc = "Apparition", type = "chaser",
  spawn = { x = 100, y = 200, respawn_seconds = 60 },
}`

func TestLoadBossCatalogReadsFieldsAndExpressions(t *testing.T) {
	dir := writeBossFile(t, `return {
  id = "test", npc = "Apparition", name = "Boss", type = "phased",
  spawn = { x = 100, y = 200, respawn_seconds = 60 },
  stats = { max_hp = 1000 * 12, defense = 0 },
  skills  = { { id = 26, cooldown_seconds = 8, range = 7, max_hp_percent = 50 } },
  summons = { { npc = "Skeleton", count = 4, max_alive = 8, cooldown_seconds = 30 } },
  phases  = { { hp_percent = 60, type = "summoner", shield_percent = 100,
                shield_until_adds_dead = true, message = "hello" } },
  drops   = { { item = 1234, chance_percent = 50, amount = 3 } },
  spawn_message = "spawned", death_message = "died",
}`)
	catalog, err := LoadBossCatalog(dir)
	if err != nil {
		t.Fatalf("load: %v", err)
	}
	if len(catalog.Bosses) != 1 {
		t.Fatalf("bosses=%d, want 1", len(catalog.Bosses))
	}
	boss := catalog.Bosses[0]
	// Lua evaluates expressions, unlike JSON.
	if boss.Stats.MaxHP == nil || *boss.Stats.MaxHP != 12000 {
		t.Errorf("max_hp=%v, want 12000 (1000*12 evaluated)", boss.Stats.MaxHP)
	}
	// Explicit zero must differ from an absent field.
	if boss.Stats.Defense == nil || *boss.Stats.Defense != 0 {
		t.Errorf("defense should be explicitly zero, got %v", boss.Stats.Defense)
	}
	if boss.Stats.Attack != nil {
		t.Errorf("absent attack should remain nil, got %v", *boss.Stats.Attack)
	}
	if len(boss.Skills) != 1 || boss.Skills[0].MaxHPPercent != 50 {
		t.Errorf("skills=%+v", boss.Skills)
	}
	if len(boss.Summons) != 1 || boss.Summons[0].MaxAlive != 8 {
		t.Errorf("summons=%+v", boss.Summons)
	}
	if len(boss.Phases) != 1 || !boss.Phases[0].ShieldUntilAddsDead {
		t.Errorf("phases=%+v", boss.Phases)
	}
	if len(boss.Drops) != 1 || boss.Drops[0].Amount != 3 {
		t.Errorf("drops=%+v", boss.Drops)
	}
	if boss.RespawnDelay().Seconds() != 60 {
		t.Errorf("respawn=%v, want 60s", boss.RespawnDelay())
	}
}

func TestLoadBossCatalogMissingDirectoryIsValid(t *testing.T) {
	catalog, err := LoadBossCatalog(filepath.Join(t.TempDir(), "missing"))
	if err != nil {
		t.Fatalf("a server without bosses is valid: %v", err)
	}
	if len(catalog.Bosses) != 0 {
		t.Fatalf("bosses=%d, want 0", len(catalog.Bosses))
	}
}

// TestLoadBossCatalogRejectsUnknownField prevents a typo from silently
// weakening the spawned boss.
func TestLoadBossCatalogRejectsUnknownField(t *testing.T) {
	dir := writeBossFile(t, `return {
  id = "test", npc = "Apparition", type = "chaser",
  spawn = { x = 100, y = 200 },
  stats = { atack = 500 },
}`)
	_, err := LoadBossCatalog(dir)
	if err == nil {
		t.Fatal("unknown field should be rejected")
	}
	if !strings.Contains(err.Error(), "atack") {
		t.Fatalf("error should name the misspelled field: %v", err)
	}
}

func TestLoadBossCatalogValidatesContent(t *testing.T) {
	for _, tc := range []struct{ name, body, want string }{
		{"invalid type", `return { id="a", npc="b", type="tank", spawn={x=1,y=1} }`, "invalid type"},
		{"missing spawn", `return { id="a", npc="b", type="chaser", spawn={x=0,y=0} }`, "spawn position"},
		{"caster without skills", `return { id="a", npc="b", type="caster", spawn={x=1,y=1} }`, "skills"},
		{"summoner without summons", `return { id="a", npc="b", type="summoner", spawn={x=1,y=1} }`, "summons"},
		{"phased without phases", `return { id="a", npc="b", type="phased", spawn={x=1,y=1} }`, "phases"},
		{"shield without adds", `return { id="a", npc="b", type="phased", spawn={x=1,y=1},
			phases={{hp_percent=50, shield_percent=100, shield_until_adds_dead=true}} }`, "shield cannot fall"},
		{"duplicate phase", `return { id="a", npc="b", type="phased", spawn={x=1,y=1},
			phases={{hp_percent=50},{hp_percent=50}} }`, "two phases"},
		{"drop without chance", `return { id="a", npc="b", type="chaser", spawn={x=1,y=1},
			drops={{item=10, chance_percent=0}} }`, "chance"},
		{"fractional number", `return { id="a", npc="b", type="chaser", spawn={x=1.5,y=1} }`, "integer"},
		{"wrong value type", `return { id="a", npc="b", type="chaser", spawn={x="ten",y=1} }`, "number"},
		{"return is not a table", `return 42`, "return value must be a table"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			_, err := LoadBossCatalog(writeBossFile(t, tc.body))
			if err == nil {
				t.Fatal("invalid content should be rejected")
			}
			if !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("error %q should contain %q", err, tc.want)
			}
		})
	}
}

func TestLoadBossCatalogRejectsMalformedLists(t *testing.T) {
	for _, tc := range []struct {
		name string
		body string
		want string
	}{
		{
			name: "zero index",
			body: `return { id="a", npc="b", type="caster", spawn={x=1,y=1},
				skills={ [0]={id=1, cooldown_seconds=1} } }`,
			want: "positive integer keys",
		},
		{
			name: "fractional index",
			body: `return { id="a", npc="b", type="caster", spawn={x=1,y=1},
				skills={ [1.5]={id=1, cooldown_seconds=1} } }`,
			want: "positive integer keys",
		},
		{
			name: "gap",
			body: `return { id="a", npc="b", type="caster", spawn={x=1,y=1},
				skills={ [1]={id=1, cooldown_seconds=1}, [3]={id=2, cooldown_seconds=1} } }`,
			want: "contiguous list",
		},
	} {
		t.Run(tc.name, func(t *testing.T) {
			_, err := LoadBossCatalog(writeBossFile(t, tc.body))
			if err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("malformed list should contain %q, got %v", tc.want, err)
			}
		})
	}
}

func TestLoadBossCatalogRejectsDuplicateID(t *testing.T) {
	dir := t.TempDir()
	for _, name := range []string{"a.lua", "b.lua"} {
		if err := os.WriteFile(filepath.Join(dir, name), []byte(bossMinimo), 0o600); err != nil {
			t.Fatal(err)
		}
	}
	_, err := LoadBossCatalog(dir)
	if err == nil || !strings.Contains(err.Error(), "defined in both") {
		t.Fatalf("duplicate ID across files should be rejected: %v", err)
	}
}

// TestBossSandboxBlocksSystemAccess ensures content cannot read disk, execute
// processes, or access the network.
func TestBossSandboxBlocksSystemAccess(t *testing.T) {
	for _, tc := range []struct{ name, body string }{
		{"os", `local x = os.time() return {}`},
		{"io", `local f = io.open("/etc/passwd") return {}`},
		{"dofile", `dofile("/etc/passwd") return {}`},
		{"loadfile", `loadfile("/etc/passwd") return {}`},
		{"load", `load("return 1")() return {}`},
		{"require", `require("os") return {}`},
	} {
		t.Run(tc.name, func(t *testing.T) {
			_, err := LoadBossCatalog(writeBossFile(t, tc.body))
			if err == nil {
				t.Fatalf("sandbox should block %s", tc.name)
			}
		})
	}
}

// TestBossSandboxKeepsSafeLibraries confirms that string, table, and math
// operations remain available, unlike plain JSON.
func TestBossSandboxKeepsSafeLibraries(t *testing.T) {
	dir := writeBossFile(t, `
local drops = {}
for i = 1, 3 do
  drops[i] = { item = 1000 + i, chance_percent = 100 }
end
return {
  id = string.lower("TEST"), npc = "Apparition", type = "chaser",
  spawn = { x = 100, y = 200 },
  stats = { max_hp = math.floor(1000.7) * 10 },
  drops = drops,
}`)
	catalog, err := LoadBossCatalog(dir)
	if err != nil {
		t.Fatalf("safe libraries should work: %v", err)
	}
	boss := catalog.Bosses[0]
	if boss.ID != "test" {
		t.Errorf("string.lower did not work: %q", boss.ID)
	}
	if boss.Stats.MaxHP == nil || *boss.Stats.MaxHP != 10000 {
		t.Errorf("math.floor did not work: %v", boss.Stats.MaxHP)
	}
	if len(boss.Drops) != 3 {
		t.Errorf("loop generated %d drops, want 3", len(boss.Drops))
	}
}
