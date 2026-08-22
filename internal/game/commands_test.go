package game

import (
	"os"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestTownCommandIsNotRegistered(t *testing.T) {
	w := &World{}
	if w.dispatchChatCommand(nil, nil, "town", "") {
		t.Fatal("/town ainda foi consumido como comando")
	}
}

func TestNightmareTimeMessageUsesClientProtocol(t *testing.T) {
	now := time.Date(2026, time.January, 2, 3, 4, 5, 0, time.UTC)
	if got := nightmareTimeMessage(now); got != "!!030405" {
		t.Fatalf("mensagem de sincronismo = %q, esperado !!030405", got)
	}
}

func TestChaosPointMessageUsesSignedDomain(t *testing.T) {
	if got := chaosPointMessage(75); got != "Chaos Point: 75 (range -75..+75)" {
		t.Fatalf("mensagem de CP positivo = %q", got)
	}
	if got := chaosPointMessage(-75); got != "Chaos Point: -75 (range -75..+75)" {
		t.Fatalf("mensagem de CP negativo = %q", got)
	}
}

func TestFameCommandReadsCharacterCounter(t *testing.T) {
	p, _ := networkedTestPlayer(1, "FameTester", 2100, 2100)
	p.SpecialCoins = map[string]uint32{fameCounter: 123}
	w := worldWithNetworkedPlayers(p)
	before := p.Session.QueuedPacketsForTest()
	if !w.dispatchChatCommand(p.Session, p, "fame", "") {
		t.Fatal("/fame nao foi consumido")
	}
	if got := p.Session.QueuedPacketsForTest(); got != before+1 {
		t.Fatalf("/fame nao enviou aviso: fila %d -> %d", before, got)
	}
	if got := fameMessage(p); got != "Fame: 123" {
		t.Fatalf("mensagem de fama = %q", got)
	}
}

func TestFameMessageNilPlayerIsZero(t *testing.T) {
	if got := fameMessage(nil); got != "Fame: 0" {
		t.Fatalf("fama de jogador nil = %q", got)
	}
}

func TestParryCommandReportsOneAuthoritativeMatchup(t *testing.T) {
	viewer, _ := networkedTestPlayer(1, "Viewer", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	viewer.Char.Score.Dex = 4_000
	target.Char.Score.Dex = 0
	applyExtendedScore(viewer.Char)
	applyExtendedScore(target.Char)
	w := worldWithNetworkedPlayers(viewer, target)
	before := viewer.Session.QueuedPacketsForTest()
	if !w.dispatchChatCommand(viewer.Session, viewer, "parry", "Target") {
		t.Fatal("/parry was not consumed")
	}
	if got := viewer.Session.QueuedPacketsForTest(); got != before+1 {
		t.Fatalf("/parry packets=%d want=%d", got, before+1)
	}
	if accuracy := playerVersusPlayerAccuracy(viewer.Char, target.Char); accuracy != 100 {
		t.Fatalf("viewer accuracy=%d want=100", accuracy)
	}
	if evasion := combatEvasionPercent(playerDex(viewer.Char),
		playerEvasionBonusPoints(viewer.Char), playerAccuracyBonusPoints(target.Char),
		playerHasConcentration(target.Char)); evasion != 80 {
		t.Fatalf("viewer evasion=%d want=80", evasion)
	}
}

func TestParseChatText(t *testing.T) {
	pkt := make([]byte, 140)
	copy(pkt[12:], "  /limparinv  \x00texto ignorado")
	got, ok := parseChatText(pkt)
	if !ok || got != commandClearInventory {
		t.Fatalf("chat=%q ok=%v", got, ok)
	}
	if _, ok := parseChatText(pkt[:12]); ok {
		t.Fatal("chat sem payload aceito")
	}
}

func TestParseWhisperTargetForSlashCommand(t *testing.T) {
	pkt := make([]byte, 128) // tamanho real do MSG_MessageWhisper 7.48
	copy(pkt[12:28], "limparinv")
	target, ok := parseWhisperTarget(pkt)
	if !ok || target != "limparinv" {
		t.Fatalf("target=%q ok=%v", target, ok)
	}
	if _, ok := parseWhisperTarget(pkt[:27]); ok {
		t.Fatal("whisper truncado aceito")
	}
}

func TestNormalizeQuotedWhisperTarget(t *testing.T) {
	for input, want := range map[string]string{
		"Felipe2":      "Felipe2",
		"/Felipe2":     "Felipe2",
		"\"Felipe2\"":  "Felipe2",
		"/\"Felipe2\"": "Felipe2",
	} {
		if got := normalizeWhisperTarget(input); got != want {
			t.Fatalf("normalize %q = %q, quer %q", input, got, want)
		}
	}
}

func TestParseWhisperText748Layout(t *testing.T) {
	pkt := make([]byte, 128)
	copy(pkt[12:28], "Felipe")
	copy(pkt[28:124], "!Carta de teste\x00ignorado")
	message, ok := parseWhisperText(pkt)
	if !ok || message != "!Carta de teste" {
		t.Fatalf("message=%q ok=%v", message, ok)
	}
}

func TestParseSlashWhisperCommandFromChatPacket(t *testing.T) {
	tests := []struct {
		input, target, body string
	}{
		{"/Felipe2 teste normal", "Felipe2", "teste normal"},
		{"/\"Felipe2\" teste com aspas", "Felipe2", "teste com aspas"},
		{"/\"Felipe2\" !carta", "Felipe2", "!carta"},
	}
	for _, tc := range tests {
		target, body, ok := parseSlashWhisperCommand(tc.input)
		if !ok || target != tc.target || body != tc.body {
			t.Fatalf("parse %q = (%q,%q,%v)", tc.input, target, body, ok)
		}
	}
	if _, _, ok := parseSlashWhisperCommand("/limparinv"); ok {
		t.Fatal("comando sem corpo virou whisper")
	}
}

func TestPrivateWhisperUsesNativeCommandWithoutVisualPrefix(t *testing.T) {
	target, body, ok := parseSlashWhisperCommand("/Felipe2 mensagem")
	if !ok || target != "Felipe2" || body != "mensagem" {
		t.Fatalf("/nick deixou de ser o comando nativo: (%q,%q,%v)", target, body, ok)
	}
}

func TestClearInventoryIncludesHiddenStructuralSlot(t *testing.T) {
	ch := model.Char{}
	ch.Inv[0] = model.Item{Index: 400}
	ch.Inv[62] = model.Item{Index: 401}
	ch.Inv[63] = model.Item{Index: 402}
	if removed := clearInventory(&ch); removed != 3 {
		t.Fatalf("removeu %d, esperado 3", removed)
	}
	for i, item := range ch.Inv {
		if item != (model.Item{}) {
			t.Fatalf("inv[%d] permaneceu: %+v", i, item)
		}
	}
}

// TestChatCommandsAcceptBothLanguages trava a decisao: as mensagens do jogo
// passaram para ingles (o client 7.48 e em ingles), mas os comandos DIGITADOS
// continuam aceitando o nome original em portugues.
//
// Existe porque a traducao em massa chegou a renomear `case "convidar"` para
// `case "invite"`, trocando silenciosamente o que o jogador precisa digitar.
func TestChatCommandsAcceptBothLanguages(t *testing.T) {
	pairs := [][2]string{
		{"criar", "create"},
		{"convidar", "invite"},
		{"aceitar", "accept"},
		{"sair", "leave"},
		{"expulsar", "expel"},
		{"limparinv", "clearinv"},
		{"reino", "kingdom"},
		{"rei", "king"},
		{"criarsub", "subcreate"},
	}
	src, err := os.ReadFile("commands.go")
	if err != nil {
		t.Fatal(err)
	}
	body := string(src)
	for _, pair := range pairs {
		for _, name := range pair {
			if !strings.Contains(body, `"`+name+`"`) {
				t.Errorf("o comando %q sumiu do dispatcher", name)
			}
		}
	}
}
