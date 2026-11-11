package game

import (
	"os"
	"path/filepath"
	"testing"
)

func TestReadCorruptedCrystalHandlesEmptyMissingMalformedAndInvalidConfig(t *testing.T) {
	t.Run("disabled", func(t *testing.T) {
		config, err := ReadCorruptedCrystal("")
		if err != nil || config != (CorruptedCrystalConfig{}) {
			t.Fatalf("empty path returned config=%+v err=%v", config, err)
		}
	})

	dir := t.TempDir()
	missing := filepath.Join(dir, "missing.json")
	t.Run("missing file", func(t *testing.T) {
		config, err := ReadCorruptedCrystal(missing)
		if err == nil || config.MediumHP != defaultMediumHP || config.SmallHP != defaultSmallHP {
			t.Fatalf("missing file returned config=%+v err=%v", config, err)
		}
	})

	malformed := filepath.Join(dir, "malformed.json")
	if err := os.WriteFile(malformed, []byte("{"), 0o600); err != nil {
		t.Fatal(err)
	}
	t.Run("malformed JSON", func(t *testing.T) {
		config, err := ReadCorruptedCrystal(malformed)
		if err == nil || config.MediumHP != defaultMediumHP || config.SmallHP != defaultSmallHP {
			t.Fatalf("malformed JSON returned config=%+v err=%v", config, err)
		}
	})

	valid := filepath.Join(dir, "valid.json")
	if err := os.WriteFile(valid, []byte(`{"X":100,"Y":100,"CrystalHP":50000,"PulsePower":10,"RespawnSeconds":60,"DeadlineSeconds":10}`), 0o600); err != nil {
		t.Fatal(err)
	}
	t.Run("valid config keeps optional defaults", func(t *testing.T) {
		config, err := ReadCorruptedCrystal(valid)
		if err != nil || config.X != 100 || config.Y != 100 || config.CrystalHP != 50000 ||
			config.MediumHP != defaultMediumHP || config.SmallHP != defaultSmallHP {
			t.Fatalf("valid config returned config=%+v err=%v", config, err)
		}
	})

	invalid := filepath.Join(dir, "invalid.json")
	if err := os.WriteFile(invalid, []byte(`{"X":10,"Y":200,"CrystalHP":0,"PulsePower":1,"RespawnSeconds":60,"DeadlineSeconds":10}`), 0o600); err != nil {
		t.Fatal(err)
	}
	t.Run("invalid bounds", func(t *testing.T) {
		config, err := ReadCorruptedCrystal(invalid)
		if err == nil || err.Error() != "invalid corrupted crystal configuration" ||
			config.MediumHP != defaultMediumHP || config.SmallHP != defaultSmallHP {
			t.Fatalf("invalid config returned config=%+v err=%v", config, err)
		}
	})
}
