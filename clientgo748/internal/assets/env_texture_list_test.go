package assets

import (
	"errors"
	"path/filepath"
	"runtime"
	"testing"
)

func TestLoadOfficialEnvTextureList(t *testing.T) {
	_, filename, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller failed")
	}
	path := filepath.Join(filepath.Dir(filename), "..", "..", "assets", "current", "Env", "EnvTextureList3.bin")
	list, err := LoadEnvTextureListFile(path)
	if err != nil {
		t.Fatal(err)
	}
	tests := map[uint16]string{
		38:  `Env\Tile01010.wys`,
		39:  `Env\Tile01011.wys`,
		392: `Env\MTile09090.wys`,
	}
	for slot, want := range tests {
		got, ok := list.Path(slot)
		if !ok || got != want {
			t.Fatalf("slot %d = %q, ok=%v; want %q", slot, got, ok, want)
		}
	}
}

func TestParseEnvTextureListRejectsMalformedInput(t *testing.T) {
	if _, err := ParseEnvTextureList(make([]byte, 10)); !errors.Is(err, ErrInvalidEnvTextureList) {
		t.Fatalf("short list error = %v", err)
	}
	data := make([]byte, envTextureRecordCount*envTextureRecordSize)
	for i := 0; i < envTextureNameLimit; i++ {
		data[i] = 'X'
	}
	if _, err := ParseEnvTextureList(data); !errors.Is(err, ErrInvalidEnvTextureList) {
		t.Fatalf("unterminated path error = %v", err)
	}
}
