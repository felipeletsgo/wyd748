package game

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/lang"
	"wydgo/internal/model"
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

// The server catalog imports the WYD-Go strdef rows unchanged in both languages.
func TestProjectMessageRowsMatchTheClientLocaleRows(t *testing.T) {
	en, pt := strdefRows(t, "EN"), strdefRows(t, "PT")
	catalog, err := lang.Load(filepath.Join("..", "..", lang.DefaultDir))
	if err != nil {
		t.Fatal(err)
	}
	rows := map[int16]string{}
	for row, key := range projectMessageKeys {
		rows[row] = key
	}
	for row := int16(459); row <= 483; row++ {
		rows[row] = fmt.Sprintf("_WG_Quiz_%d", row)
	}
	for row, key := range rows {
		for _, c := range []struct {
			language lang.Language
			text     []byte
		}{{lang.English, en[row]}, {lang.Portuguese, pt[row]}} {
			if got, _ := catalog.Raw(c.language, key); got != string(c.text) {
				t.Errorf("row %d %s = %q, catalog %s = %q", row, c.language.Code(), c.text, key, got)
			}
		}
	}
}

func TestProjectMessagesUseTheChannelLanguage(t *testing.T) {
	text := func(p []byte) string {
		if wire.ParseHeader(p).Type != wire.OpMessagePanel {
			t.Fatalf("packet 0x%X is not a top notice", wire.ParseHeader(p).Type)
		}
		return string(bytes.TrimRight(p[12:107], "\x00"))
	}
	pt, en := &World{language: lang.Portuguese}, &World{}
	for name, want := range map[string]string{
		"Cube 7": "Cubo da Maldade 7", "Cube Final": "Cubo da Maldade - Final", "Big Cube": "Grande Cubo",
	} {
		if got := text(pt.instanceStageNamePacket(name)); got != portugueseText(t, want) {
			t.Errorf("stage %q = %q, want %q", name, got, want)
		}
		if got := text(en.instanceStageNamePacket(name)); got != name {
			t.Errorf("English stage %q = %q", name, got)
		}
	}
	if got := text(pt.quizCorrectPacket(1500)); got != "Correto! +1500 EXP." {
		t.Fatalf("Portuguese quiz reward = %q", got)
	}
	if got := text(en.projectMessage(msgOXNextQuestion, "50")); got != "Next OX question in 50 seconds." {
		t.Fatalf("English OX countdown = %q", got)
	}
	// Native rows keep the client index.
	if op := wire.ParseHeader(pt.projectMessage(155)).Type; op != wire.OpMessageIndexed {
		t.Fatalf("native row 155 sent as 0x%X", op)
	}
	quiz := &model.VolatileInstanceQuiz{Question: "The_M_key_shows_the_minimap_in_the_lower-right_corner.", TextIndex: 459}
	question := pt.instanceQuestionPacket(7, quiz, 10000)
	if binary.LittleEndian.Uint16(question[12:14]) != 1 || binary.LittleEndian.Uint16(question[14:16]) != 0 {
		t.Fatal("the question must carry its text as version 1")
	}
	if got := string(bytes.TrimRight(question[20:116], "\x00")); got != portugueseText(t, "A tecla M mostra o minimapa no canto inferior direito.") {
		t.Fatalf("Portuguese question = %q", got)
	}
}
