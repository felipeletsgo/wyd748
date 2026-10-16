package data

import (
	"bytes"
	"encoding/binary"
	"encoding/csv"
	"os"
	"strconv"
	"testing"
)

// Decode the same 68-byte records as WYD748_LoadItemNames. This checks the
// labels actually displayed by the client, not a second handwritten name list.
func TestSkillDataNamesMatchClientItemNames(t *testing.T) {
	runtime := "../../../tmproject/client748/"
	encoded, err := os.ReadFile(runtime + "Itemname.bin")
	if err != nil {
		t.Fatal(err)
	}
	if len(encoded) == 0 || len(encoded)%68 != 0 {
		t.Fatal("invalid client item-name record size")
	}
	names := make(map[int]string)
	for offset := 0; offset < len(encoded); offset += 68 {
		index := int(int32(binary.LittleEndian.Uint32(encoded[offset:])))
		if index < 5000 || index > 5102 {
			continue
		}
		label := make([]byte, 62)
		for position := range label {
			label[position] = encoded[offset+4+position] - byte(position)
		}
		if end := bytes.IndexByte(label, 0); end >= 0 {
			label = label[:end]
		}
		names[index-5000] = string(label)
	}
	if len(names) != 103 {
		t.Fatalf("decoded %d skill names, want 103", len(names))
	}
	server, err := os.ReadFile("../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	serverRows, err := csv.NewReader(bytes.NewReader(server)).ReadAll()
	if err != nil {
		t.Fatal(err)
	}
	if len(serverRows) != 104 {
		t.Fatalf("got %d skills, want 104 including the null sentinel", len(serverRows))
	}
	for _, row := range serverRows {
		if len(row) != 23 {
			t.Fatalf("invalid skill column count: %d", len(row))
		}
		index, err := strconv.Atoi(row[0])
		if err != nil {
			t.Fatal(err)
		}
		want := names[index]
		if index == 103 {
			want = "NULL"
		}
		if want == "" || row[22] != want {
			t.Errorf("skill %d name = %q, client displays %q", index, row[22], want)
		}
	}
}
