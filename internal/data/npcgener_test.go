package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestLoadNPCGenerStandardFormat(t *testing.T) {
	path := filepath.Join(t.TempDir(), "NPCGener.txt")
	content := `// comentario
# [ 12 ]
MinuteGenerate: 2
MaxNumMob: 10
MinGroup: 2
MaxGroup: 4
Leader: Gremlin
Follower: Mago_Gremlin
RouteType: 2
Formation: 1
StartX: 2200
StartY: 2100
StartRange: 5
StartWait: 10
Segment1X: 2210
Segment1Y: 2110
DestX: 2220
DestY: 2120
DestAction: voltar
`
	if err := os.WriteFile(path, []byte(content), 0600); err != nil {
		t.Fatal(err)
	}
	got, err := LoadNPCGener(path)
	if err != nil {
		t.Fatal(err)
	}
	if len(got) != 1 || got[0].Index != 0 {
		t.Fatalf("indice deve seguir ordem das secoes: %+v", got)
	}
	g := got[0]
	if g.Leader != "Gremlin" || g.Follower != "Mago_Gremlin" || g.MinGroup != 2 || g.MaxGroup != 4 {
		t.Fatalf("grupo parseado incorretamente: %+v", g)
	}
	if g.Segments[0].X != 2200 || g.Segments[1].X != 2210 || g.Segments[4].Action != "voltar" {
		t.Fatalf("segmentos parseados incorretamente: %+v", g.Segments)
	}
}

// TestLoadNPCGenerRealWorldQuirks cobre as tolerancias necessarias para carregar
// NPCGener.txt editados a mao (como o do Micronics): linhas fora do vocabulario
// (ex.: comentario malformado com uma barra so) sao ignoradas, e MaxGroup abaixo
// de MinGroup e normalizado em vez de abortar o boot -- comportamento do nativo.
func TestLoadNPCGenerRealWorldQuirks(t *testing.T) {
	path := filepath.Join(t.TempDir(), "NPCGener.txt")
	content := `# [ 0 ]
MinuteGenerate: 1
MaxNumMob: 10
MinGroup: 4
MaxGroup: 2
Leader: Gremlin
/ Capa Verde - Troca
CampoDesconhecido: qualquer_coisa
StartX: 2200
StartY: 2100
`
	if err := os.WriteFile(path, []byte(content), 0600); err != nil {
		t.Fatal(err)
	}
	got, err := LoadNPCGener(path)
	if err != nil {
		t.Fatalf("linhas desconhecidas nao deveriam ser erro: %v", err)
	}
	if len(got) != 1 {
		t.Fatalf("esperava 1 gerador, veio %d", len(got))
	}
	g := got[0]
	if g.MaxGroup != g.MinGroup || g.MinGroup != 4 {
		t.Fatalf("MaxGroup deveria ser normalizado para MinGroup=4: Min=%d Max=%d", g.MinGroup, g.MaxGroup)
	}
	if g.Follower != "Gremlin" {
		t.Fatalf("Follower vazio deveria herdar o Leader: %q", g.Follower)
	}
	if g.Segments[0].X != 2200 || g.Segments[0].Y != 2100 {
		t.Fatalf("campos validos apos as linhas desconhecidas deveriam parsear: %+v", g.Segments[0])
	}
}

// TestLoadNPCGenerBadValueOnKnownKey garante que a leniencia com chaves
// desconhecidas NAO mascara valor invalido em campo reconhecido.
func TestLoadNPCGenerBadValueOnKnownKey(t *testing.T) {
	path := filepath.Join(t.TempDir(), "NPCGener.txt")
	content := "# [ 0 ]\nLeader: Gremlin\nMaxNumMob: abc\n"
	if err := os.WriteFile(path, []byte(content), 0600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadNPCGener(path); err == nil {
		t.Fatal("valor nao-inteiro em MaxNumMob deveria ser erro")
	}
}

func TestLoadNPCGenerDisabledSection(t *testing.T) {
	path := filepath.Join(t.TempDir(), "NPCGener.txt")
	if err := os.WriteFile(path, []byte("#* [ 0 ]\nLeader: Inexistente\n"), 0600); err != nil {
		t.Fatal(err)
	}
	got, err := LoadNPCGener(path)
	if err != nil {
		t.Fatal(err)
	}
	if len(got) != 1 || got[0].Enabled {
		t.Fatalf("secao #* deveria ficar desativada: %+v", got)
	}
}

func TestMicronicsNPCDatasetMatchesNPCGener(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatalf("carregar NPCs convertidos: %v", err)
	}
	// Piso, nao igualdade: a base convertida do Micronics tem 476 NPCs e
	// adicionar um novo (ex.: GremlinBIG) e legitimo. O guarda existe para
	// pegar perda em massa -- pasta errada, conversao quebrada --, nao para
	// travar o dataset.
	const micronicsBaseNPCs = 476
	if len(npcs) < micronicsBaseNPCs {
		t.Fatalf("NPCs convertidos=%d, esperado ao menos %d", len(npcs), micronicsBaseNPCs)
	}
	geners, err := LoadNPCGener("../../data/NPCGener.txt")
	if err != nil {
		t.Fatalf("carregar NPCGener Micronics: %v", err)
	}

	templates := make(map[string]bool, len(npcs)*2)
	for _, npc := range npcs {
		templates[npc.Name] = true
		templates[strings.ReplaceAll(npc.Name, "_", " ")] = true
	}
	for _, gener := range geners {
		if !gener.Enabled {
			continue
		}
		for role, name := range map[string]string{
			"Leader": gener.Leader, "Follower": gener.Follower,
		} {
			if !templates[name] && !templates[strings.ReplaceAll(name, "_", " ")] {
				t.Errorf("NPCGener[%d] %s=%q nao possui template convertido",
					gener.Index, role, name)
			}
		}
	}
}
