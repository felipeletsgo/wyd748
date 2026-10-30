package game

import (
	"bytes"
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/wire"
)

func strdefRows(t *testing.T, lang string) [][]byte {
	t.Helper()
	raw, err := os.ReadFile(filepath.Join("..", "..", "..", "tmproject", "client748", lang, "UI", "strdef.bin"))
	if err != nil {
		t.Fatal(err)
	}
	payload := raw[:len(raw)-4]
	rows := make([][]byte, len(payload)/128)
	for i := range rows {
		row := make([]byte, 128)
		for j := range row {
			row[j] = payload[i*128+j] ^ 0x5a
		}
		rows[i] = bytes.TrimRight(row, "\x00")
	}
	return rows
}

// The server row constants must name the shipped EN/PT notices.
func TestCubeAndOXNoticeRowsMatchLocaleResources(t *testing.T) {
	en, pt := strdefRows(t, "EN"), strdefRows(t, "PT")
	for row, want := range map[int16]string{
		msgCubeRoom: "Cube %s", msgCubeFinal: "Cube Final", msgBigCube: "Big Cube",
		msgBigCubeWrong: "Wrong answer. You left Big Cube.", msgQuizCorrect: "Correct! +%s EXP.",
		msgOXEnrollmentOpen:        "OX enrollment is open for 60 seconds. Use an Entrance Ticket in the Big Cube lobby.",
		msgOXEnterDuringEnrollment: "Enter OX with an Entrance Ticket during enrollment.",
		msgOXWrong:                 "Wrong answer. You left OX without a reward.",
		msgOXEnrollmentCloses:      "OX enrollment closes in %s seconds.",
		msgOXNextQuestion:          "Next OX question in %s seconds.",
		msgCubeEntranceBlocked:     "The Cube entrance is blocked.",
		msgCubeWrong:               "Wrong answer. You left the Cube.",
	} {
		if int(row) >= len(en) || string(en[row]) != want {
			t.Fatalf("EN row %d=%q, want %q", row, en[row], want)
		}
		if len(pt[row]) == 0 || bytes.Equal(pt[row], en[row]) {
			t.Fatalf("PT row %d is not translated", row)
		}
	}
}

func TestInstanceStageNamePacketLocalizesCubeNames(t *testing.T) {
	index := func(p []byte) (uint16, int16, string) {
		return wire.ParseHeader(p).Type, int16(binary.LittleEndian.Uint16(p[14:16])), string(bytes.TrimRight(p[16:], "\x00"))
	}
	if op, row, param := index(instanceStageNamePacket("Cube 7")); op != wire.OpMessageParameterized ||
		row != msgCubeRoom-1000 || param != "7" {
		t.Fatalf("Cube 7 -> 0x%X row %d param %q", op, row, param)
	}
	if op, row, _ := index(instanceStageNamePacket("Cube Final")); op != wire.OpMessageIndexed || row != msgCubeFinal-1000 {
		t.Fatal("Cube Final is not localized")
	}
	if op, row, _ := index(instanceStageNamePacket("Big Cube")); op != wire.OpMessageIndexed || row != msgBigCube-1000 {
		t.Fatal("Big Cube is not localized")
	}
	if wire.ParseHeader(instanceStageNamePacket("Normal Room 1")).Type != wire.OpMessagePanel {
		t.Fatal("other stage names must keep the configured text")
	}
}
