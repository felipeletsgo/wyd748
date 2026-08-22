package game

import (
	"encoding/binary"
	"errors"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func newCharacterPacket(slot int, name string, class int) []byte {
	pkt := make([]byte, 36)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(slot))
	copy(pkt[16:32], name)
	binary.LittleEndian.PutUint32(pkt[32:36], uint32(class))
	return pkt
}

func installTestCharacterTemplates(w *World) {
	w.charSpawn = model.CharacterSpawn{X: 2100, Y: 2100}
	for class := 0; class < 4; class++ {
		w.charTemplates[class] = model.CharacterTemplate{
			Name: "Class", Class: byte(class),
			Score: model.Score{
				Version: model.ScoreVersion,
				MaxHP:   100, CurHP: 100, MaxMP: 100, CurMP: 100,
			},
		}
	}
}

func TestCreateCharacterHandlerPersistsTemplateAndReservesName(t *testing.T) {
	w, p, session := newDeleteWorld(t)
	installTestCharacterTemplates(w)
	st := w.store.(*craftStore)

	w.onCreateCharacter(session, newCharacterPacket(2, "FelipeTest", 3))
	if len(p.Account.Chars) != 3 || p.Account.Chars[2].Name != "FelipeTest" ||
		p.Account.Chars[2].Class != 3 || p.Account.Chars[2].X != playerEntryX {
		t.Fatalf("personagem criado incorretamente: %+v", p.Account.Chars)
	}
	if _, err := model.NormalizeCharacterUID(p.Account.Chars[2].UID); err != nil ||
		p.Account.Chars[2].UID == "" {
		t.Fatalf("personagem criado sem UID valido: %q err=%v",
			p.Account.Chars[2].UID, err)
	}
	if st.saves != 1 || !nameIsTaken(w, "felipetest") {
		t.Fatalf("criacao nao persistiu/reservou nome: saves=%d names=%v", st.saves, w.charNames)
	}
	if characterCount(p.Account.Chars) != 1 {
		t.Fatalf("contagem de personagens=%d", characterCount(p.Account.Chars))
	}
}

func TestCreateCharacterHandlerRejectsInvalidDuplicateAndOccupiedSlot(t *testing.T) {
	w, p, session := newDeleteWorld(t, model.Char{Name: "Existing"})
	installTestCharacterTemplates(w)
	st := w.store.(*craftStore)
	w.charNames["takenname"] = struct{}{}

	for _, pkt := range [][]byte{
		newCharacterPacket(-1, "ValidName", 0),
		newCharacterPacket(4, "ValidName", 0),
		newCharacterPacket(1, "abc", 0),
		newCharacterPacket(1, "ValidName", 4),
		newCharacterPacket(1, "TakenName", 0),
		newCharacterPacket(0, "OtherName", 0),
	} {
		w.onCreateCharacter(session, pkt)
	}
	if st.saves != 0 || len(p.Account.Chars) != 1 || p.Account.Chars[0].Name != "Existing" {
		t.Fatal("criacao recusada alterou/persistiu a conta")
	}

	p.InWorld = true
	w.onCreateCharacter(session, newCharacterPacket(1, "OtherName", 0))
	w.onCreateCharacter(session, []byte{1, 2, 3})
	if st.saves != 0 {
		t.Fatal("jogador em mundo/pacote curto criou personagem")
	}
}

func TestCreateCharacterHandlerRollsBackSaveAndMissingTemplate(t *testing.T) {
	w, p, session := newDeleteWorld(t)
	installTestCharacterTemplates(w)
	st := w.store.(*craftStore)
	st.err = errors.New("disk")

	w.onCreateCharacter(session, newCharacterPacket(0, "Rollback", 0))
	if len(p.Account.Chars) != 0 || nameIsTaken(w, "Rollback") {
		t.Fatal("falha de save conservou personagem/nome")
	}

	st.err = nil
	w.charTemplates[1] = model.CharacterTemplate{}
	w.onCreateCharacter(session, newCharacterPacket(0, "NoLayout", 1))
	if len(p.Account.Chars) != 0 || st.saves != 1 {
		t.Fatal("classe sem layout alterou a conta")
	}
}

type nameLookupStore struct {
	craftStore
	taken bool
	err   error
}

func (s *nameLookupStore) CharacterNameExists(name string) (bool, error) {
	if strings.EqualFold(name, "ErrorName") {
		return false, s.err
	}
	return s.taken, nil
}

func TestCharacterNameTakenUsesIndexOrStoreFallback(t *testing.T) {
	w := &World{charNames: map[string]struct{}{"indexed": {}}}
	if taken, err := w.characterNameTaken("Indexed"); !taken || err != nil {
		t.Fatalf("indice nao resolveu nome: taken=%v err=%v", taken, err)
	}

	st := &nameLookupStore{taken: true, err: errors.New("lookup")}
	w.charNames = nil
	w.store = st
	if taken, err := w.characterNameTaken("AnyName"); !taken || err != nil {
		t.Fatalf("fallback do store nao resolveu nome: taken=%v err=%v", taken, err)
	}
	if _, err := w.characterNameTaken("ErrorName"); !errors.Is(err, st.err) {
		t.Fatalf("erro do store nao propagou: %v", err)
	}
}
