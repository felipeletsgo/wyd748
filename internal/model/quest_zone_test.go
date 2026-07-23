package model

import "testing"

func TestQuestZoneContainsInclusiveBorders(t *testing.T) {
	z := QuestZone{Name: "z", X1: 10, Y1: 20, X2: 30, Y2: 40}
	cases := []struct {
		x, y uint16
		want bool
	}{
		{10, 20, true}, // canto inferior (borda inclusiva)
		{30, 40, true}, // canto superior (borda inclusiva)
		{20, 30, true}, // centro
		{9, 30, false}, // a oeste
		{31, 30, false},
		{20, 19, false}, // ao sul
		{20, 41, false},
	}
	for _, c := range cases {
		if got := z.Contains(c.x, c.y); got != c.want {
			t.Errorf("Contains(%d,%d)=%v, quer %v", c.x, c.y, got, c.want)
		}
	}
}

func TestQuestZoneFileValidate(t *testing.T) {
	good := QuestZoneFile{Version: 1, Zones: []QuestZone{{Name: "a", X1: 1, Y1: 1, X2: 2, Y2: 2}}}
	if err := good.Validate(); err != nil {
		t.Fatalf("arquivo valido recusado: %v", err)
	}
	if (&QuestZoneFile{Version: 2}).Validate() == nil {
		t.Fatal("versao errada aceita")
	}
	inverted := QuestZoneFile{Version: 1, Zones: []QuestZone{{Name: "a", X1: 5, Y1: 1, X2: 2, Y2: 2}}}
	if inverted.Validate() == nil {
		t.Fatal("retangulo invertido aceito")
	}
	noName := QuestZoneFile{Version: 1, Zones: []QuestZone{{X1: 1, Y1: 1, X2: 2, Y2: 2}}}
	if noName.Validate() == nil {
		t.Fatal("zona sem nome aceita")
	}
	dup := QuestZoneFile{Version: 1, Zones: []QuestZone{
		{Name: "a", X1: 1, Y1: 1, X2: 2, Y2: 2},
		{Name: "a", X1: 3, Y1: 3, X2: 4, Y2: 4},
	}}
	if dup.Validate() == nil {
		t.Fatal("nome duplicado aceito")
	}
}
