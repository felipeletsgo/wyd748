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
  id = "teste", npc = "Aparicao", type = "chaser",
  spawn = { x = 100, y = 200, respawn_seconds = 60 },
}`

func TestLoadBossCatalogLeCamposEExpressoes(t *testing.T) {
	dir := writeBossFile(t, `return {
  id = "teste", npc = "Aparicao", name = "Chefe", type = "phased",
  spawn = { x = 100, y = 200, respawn_seconds = 60 },
  stats = { max_hp = 1000 * 12, defense = 0 },
  skills  = { { id = 26, cooldown_seconds = 8, range = 7, max_hp_percent = 50 } },
  summons = { { npc = "Esqueleto", count = 4, max_alive = 8, cooldown_seconds = 30 } },
  phases  = { { hp_percent = 60, type = "summoner", shield_percent = 100,
                shield_until_adds_dead = true, message = "oi" } },
  drops   = { { item = 1234, chance_percent = 50, amount = 3 } },
  spawn_message = "nasceu", death_message = "morreu",
}`)
	catalog, err := LoadBossCatalog(dir)
	if err != nil {
		t.Fatalf("carregar: %v", err)
	}
	if len(catalog.Bosses) != 1 {
		t.Fatalf("bosses=%d, quer 1", len(catalog.Bosses))
	}
	boss := catalog.Bosses[0]
	// Lua avalia expressoes: e a vantagem sobre JSON.
	if boss.Stats.MaxHP == nil || *boss.Stats.MaxHP != 12000 {
		t.Errorf("max_hp=%v, quer 12000 (1000*12 avaliado)", boss.Stats.MaxHP)
	}
	// Zero explicito precisa se distinguir de campo ausente.
	if boss.Stats.Defense == nil || *boss.Stats.Defense != 0 {
		t.Errorf("defense deveria ser 0 explicito, veio %v", boss.Stats.Defense)
	}
	if boss.Stats.Attack != nil {
		t.Errorf("attack ausente deveria ficar nil, veio %v", *boss.Stats.Attack)
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
		t.Errorf("respawn=%v, quer 60s", boss.RespawnDelay())
	}
}

func TestLoadBossCatalogDiretorioAusenteNaoEErro(t *testing.T) {
	catalog, err := LoadBossCatalog(filepath.Join(t.TempDir(), "inexistente"))
	if err != nil {
		t.Fatalf("servidor sem bosses e legitimo: %v", err)
	}
	if len(catalog.Bosses) != 0 {
		t.Fatalf("bosses=%d, quer 0", len(catalog.Bosses))
	}
}

// TestLoadBossCatalogRejeitaCampoDesconhecido protege contra o erro mais
// perigoso: um campo digitado errado seria ignorado e o boss nasceria fraco.
func TestLoadBossCatalogRejeitaCampoDesconhecido(t *testing.T) {
	dir := writeBossFile(t, `return {
  id = "teste", npc = "Aparicao", type = "chaser",
  spawn = { x = 100, y = 200 },
  stats = { atack = 500 },
}`)
	_, err := LoadBossCatalog(dir)
	if err == nil {
		t.Fatal("campo desconhecido deveria ser recusado")
	}
	if !strings.Contains(err.Error(), "atack") {
		t.Fatalf("o erro deveria citar o campo errado: %v", err)
	}
}

func TestLoadBossCatalogValidaConteudo(t *testing.T) {
	for _, tc := range []struct{ nome, corpo, espera string }{
		{"tipo invalido", `return { id="a", npc="b", type="tanque", spawn={x=1,y=1} }`, "tipo"},
		{"sem spawn", `return { id="a", npc="b", type="chaser", spawn={x=0,y=0} }`, "posicao"},
		{"caster sem skills", `return { id="a", npc="b", type="caster", spawn={x=1,y=1} }`, "skills"},
		{"summoner sem summons", `return { id="a", npc="b", type="summoner", spawn={x=1,y=1} }`, "summons"},
		{"phased sem phases", `return { id="a", npc="b", type="phased", spawn={x=1,y=1} }`, "phases"},
		{"escudo sem adds", `return { id="a", npc="b", type="phased", spawn={x=1,y=1},
			phases={{hp_percent=50, shield_percent=100, shield_until_adds_dead=true}} }`, "nunca cairia"},
		{"fase duplicada", `return { id="a", npc="b", type="phased", spawn={x=1,y=1},
			phases={{hp_percent=50},{hp_percent=50}} }`, "duas fases"},
		{"drop sem chance", `return { id="a", npc="b", type="chaser", spawn={x=1,y=1},
			drops={{item=10, chance_percent=0}} }`, "chance"},
		{"numero fracionario", `return { id="a", npc="b", type="chaser", spawn={x=1.5,y=1} }`, "inteiro"},
		{"tipo errado", `return { id="a", npc="b", type="chaser", spawn={x="dez",y=1} }`, "numero"},
		{"retorno nao e tabela", `return 42`, "tabela"},
	} {
		t.Run(tc.nome, func(t *testing.T) {
			_, err := LoadBossCatalog(writeBossFile(t, tc.corpo))
			if err == nil {
				t.Fatal("deveria ser recusado")
			}
			if !strings.Contains(err.Error(), tc.espera) {
				t.Fatalf("erro %q deveria conter %q", err, tc.espera)
			}
		})
	}
}

func TestLoadBossCatalogRejeitaIDDuplicado(t *testing.T) {
	dir := t.TempDir()
	for _, name := range []string{"a.lua", "b.lua"} {
		if err := os.WriteFile(filepath.Join(dir, name), []byte(bossMinimo), 0o600); err != nil {
			t.Fatal(err)
		}
	}
	_, err := LoadBossCatalog(dir)
	if err == nil || !strings.Contains(err.Error(), "definido em") {
		t.Fatalf("ID duplicado entre arquivos deveria ser recusado: %v", err)
	}
}

// TestBossSandboxBloqueiaAcessoAoSistema e o teste de seguranca: um arquivo de
// conteudo nao pode ler disco, executar processo nem sair do diretorio.
func TestBossSandboxBloqueiaAcessoAoSistema(t *testing.T) {
	for _, tc := range []struct{ nome, corpo string }{
		{"os", `local x = os.time() return {}`},
		{"io", `local f = io.open("/etc/passwd") return {}`},
		{"dofile", `dofile("/etc/passwd") return {}`},
		{"loadfile", `loadfile("/etc/passwd") return {}`},
		{"load", `load("return 1")() return {}`},
		{"require", `require("os") return {}`},
	} {
		t.Run(tc.nome, func(t *testing.T) {
			_, err := LoadBossCatalog(writeBossFile(t, tc.corpo))
			if err == nil {
				t.Fatalf("o sandbox deveria bloquear %s", tc.nome)
			}
		})
	}
}

// TestBossSandboxMantemBibliotecasSeguras confirma o outro lado: string, table e
// math continuam disponiveis, que e o motivo de usar Lua em vez de JSON.
func TestBossSandboxMantemBibliotecasSeguras(t *testing.T) {
	dir := writeBossFile(t, `
local drops = {}
for i = 1, 3 do
  drops[i] = { item = 1000 + i, chance_percent = 100 }
end
return {
  id = string.lower("TESTE"), npc = "Aparicao", type = "chaser",
  spawn = { x = 100, y = 200 },
  stats = { max_hp = math.floor(1000.7) * 10 },
  drops = drops,
}`)
	catalog, err := LoadBossCatalog(dir)
	if err != nil {
		t.Fatalf("bibliotecas seguras deveriam funcionar: %v", err)
	}
	boss := catalog.Bosses[0]
	if boss.ID != "teste" {
		t.Errorf("string.lower nao funcionou: %q", boss.ID)
	}
	if boss.Stats.MaxHP == nil || *boss.Stats.MaxHP != 10000 {
		t.Errorf("math.floor nao funcionou: %v", boss.Stats.MaxHP)
	}
	if len(boss.Drops) != 3 {
		t.Errorf("laco gerou %d drops, quer 3", len(boss.Drops))
	}
}
