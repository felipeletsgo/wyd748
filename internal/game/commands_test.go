package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestTownCommandIsNotRegistered(t *testing.T) {
	w := &World{}
	if w.dispatchChatCommand(nil, nil, "town", "") {
		t.Fatal("/town ainda foi consumido como comando")
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
