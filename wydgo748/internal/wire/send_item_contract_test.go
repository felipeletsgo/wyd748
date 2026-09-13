package wire

import (
	"bytes"
	"encoding/hex"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func loadProtocolFixture(t *testing.T, name string) []byte {
	t.Helper()
	_, sourceFile, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("cannot locate send_item_contract_test.go")
	}
	path := filepath.Join(filepath.Dir(sourceFile), "..", "..", "..", "testdata", "protocol", name)
	raw, err := os.ReadFile(path)
	if err != nil {
		t.Fatalf("read protocol fixture %s: %v", name, err)
	}
	var tokens []string
	for _, line := range strings.Split(string(raw), "\n") {
		if comment := strings.IndexByte(line, '#'); comment >= 0 {
			line = line[:comment]
		}
		tokens = append(tokens, strings.Fields(line)...)
	}
	decoded, err := hex.DecodeString(strings.Join(tokens, ""))
	if err != nil {
		t.Fatalf("decode protocol fixture %s: %v", name, err)
	}
	return decoded
}

// Fixture tambem consumido pelo teste C++ da fronteira de recepcao. Campos
// internos do item (UID/datas) nunca podem ampliar os oito bytes do payload.
func TestSendItemLegacyFrame(t *testing.T) {
	item := model.Item{Index: 0x1234, Eff: [6]byte{1, 2, 3, 4, 5, 6},
		UID: "server-only", ActivatedUnix: 123, ExpiresUnix: 456}
	want := loadProtocolFixture(t, "send_item_0x182_24.hex")
	packet := SendItem(0x1234, 2, 127, item)
	// O builder deixa Size zerado; quem enquadra e cifra e o transporte.
	FinishPacket(packet, 7)
	got, checksumOK, err := ReadPacket(bytes.NewReader(packet))
	if err != nil || !checksumOK {
		t.Fatalf("SendItem roundtrip: checksum=%v err=%v", checksumOK, err)
	}
	// O fixture plaintext normaliza somente os dois bytes da cifragem.
	got[2], got[3] = 0, 0
	if !bytes.Equal(got, want) {
		t.Fatalf("SendItem wire mismatch: got % x, want % x", got, want)
	}
}
