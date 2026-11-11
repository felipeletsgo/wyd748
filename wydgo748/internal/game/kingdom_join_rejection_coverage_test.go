package game

import (
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func TestJoinKingdomRejectsUnsupportedCharacterStates(t *testing.T) {
	for _, tc := range []struct {
		name      string
		level     uint32
		evolution string
		cape      uint16
		kingdom   byte
	}{
		{"dead", 399, "", 0, model.KingdomHekalotia},
		{"already has kingdom", 219, "", 545, model.KingdomHekalotia},
		{"opposite kingdom", 399, "", 545, model.KingdomAkelonia},
		{"below displayed level 220", 218, "", 0, model.KingdomHekalotia},
		{"below displayed level 256", 254, "", 3193, model.KingdomHekalotia},
		{"mortal with master cape", 399, "", 3199, model.KingdomHekalotia},
		{"unsupported cape", 399, "", 999, model.KingdomHekalotia},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st := handlerTestWorld(t)
			p.Char.Score.Level = tc.level
			p.Char.Evolution = tc.evolution
			p.Char.Equip[model.CapeSlot] = model.Item{Index: tc.cape}
			if tc.name == "dead" {
				setPlayerCurHP(p.Char, 0)
			}
			before := cloneCharacterState(p.Char)
			w.joinKingdom(p.Session, p, tc.kingdom)
			if st.saves != 0 || !reflect.DeepEqual(*p.Char, before) {
				t.Fatalf("rejected join changed character or wrote storage: saves=%d", st.saves)
			}
			if p.Session.QueuedPacketsForTest() == 0 {
				t.Fatal("rejected join did not explain the requirement")
			}
		})
	}
}
