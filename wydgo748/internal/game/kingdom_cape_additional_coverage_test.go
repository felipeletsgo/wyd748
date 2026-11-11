package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestKingdomCapeForJoinRejectsInvalidAndIncompleteCharacters(t *testing.T) {
	for _, tc := range []struct {
		name    string
		char    *model.Char
		kingdom byte
		wantErr error
	}{
		{"invalid kingdom", kingdomTestChar(399, "", 0), model.KingdomNeutral, errKingdomInvalidCape},
		{"nil character", nil, model.KingdomHekalotia, errKingdomNeedLevel220},
		{"missing score", &model.Char{}, model.KingdomHekalotia, errKingdomNeedLevel220},
		{"below first join level", kingdomTestChar(218, "", 0), model.KingdomHekalotia, errKingdomNeedLevel220},
		{"below advanced cape level", kingdomTestChar(254, "", 3193), model.KingdomHekalotia, errKingdomNeedLevel256},
		{"unknown neutral cape", kingdomTestChar(399, "", 999), model.KingdomHekalotia, errKingdomInvalidCape},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if _, err := kingdomCapeForJoin(tc.char, tc.kingdom); !errors.Is(err, tc.wantErr) {
				t.Fatalf("kingdomCapeForJoin() error=%v, want %v", err, tc.wantErr)
			}
		})
	}
}

func TestKingdomCapeForJoinConvertsLegacyNeutralCape(t *testing.T) {
	change, err := kingdomCapeForJoin(kingdomTestChar(219, "", 548), model.KingdomAkelonia)
	if err != nil || change.Index != 546 || change.Tier != model.CapeTierBasic || !change.Preserve {
		t.Fatalf("legacy neutral cape conversion=%+v err=%v", change, err)
	}
}
