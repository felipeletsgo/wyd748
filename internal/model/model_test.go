package model

import (
	"bytes"
	"encoding/json"
	"testing"
)

func TestItemEffLoadsFromJSON(t *testing.T) {
	// Pilha de 120: EF_AMOUNT(61)=120 embutido via eff.
	var it Item
	if err := json.Unmarshal([]byte(`{"index":2390,"eff":[61,120,0,0,0,0]}`), &it); err != nil {
		t.Fatal(err)
	}
	if it.Index != 2390 || it.Eff[0] != 61 || it.Eff[1] != 120 {
		t.Fatalf("eff: index=%d eff=%v", it.Index, it.Eff)
	}
}

func TestCharJSONPersistsCanonicalScore(t *testing.T) {
	ch := Char{
		Name:    "felipe",
		NextExp: 649715,
		Score: &Score{
			Version: ScoreVersion, Level: 50, Attack: 200,
			StatusPts: 250, MasterPts: 100, SkillPts: 150,
			ResistFire: 10, ResistIce: 20, ResistHoly: 30, ResistThunder: 40,
		},
	}
	data, err := json.Marshal(ch)
	if err != nil {
		t.Fatal(err)
	}
	if bytes.Contains(data, []byte("baseScore")) ||
		bytes.Contains(data, []byte("nextExp")) ||
		bytes.Contains(data, []byte(`"extendedScore":`)) {
		t.Fatalf("JSON persistiu estado legado/derivado: %s", data)
	}
	if !bytes.Contains(data, []byte(`"score":{"version":2,"level":50,"attack":200`)) {
		t.Fatalf("score canonico ausente: %s", data)
	}

	var loaded Char
	if err := json.Unmarshal(data, &loaded); err != nil {
		t.Fatal(err)
	}
	if loaded.Score == nil || loaded.Score.Level != 50 || loaded.Score.Attack != 200 {
		t.Fatalf("round-trip perdeu score: %+v", loaded.Score)
	}
	if loaded.Score.ResistFire != 10 || loaded.Score.ResistThunder != 40 {
		t.Fatalf("round-trip perdeu resistencias: %+v", loaded.Score)
	}
}

func TestCharJSONUsesNativeMountAndCapeSlots(t *testing.T) {
	ch := Char{Name: "Slots", Score: &Score{Version: ScoreVersion}}
	ch.Equip[14] = Item{Index: 2376}
	ch.Equip[15] = Item{Index: 545}
	data, err := json.Marshal(ch)
	if err != nil {
		t.Fatal(err)
	}
	if bytes.Contains(data, []byte(`"manto"`)) ||
		!bytes.Contains(data, []byte(`"montaria":{"index":2376`)) ||
		!bytes.Contains(data, []byte(`"capa":{"index":545`)) {
		t.Fatalf("layout persistido nao e o nativo: %s", data)
	}
	var loaded Char
	if err := json.Unmarshal(data, &loaded); err != nil {
		t.Fatal(err)
	}
	if loaded.Equip[14].Index != 2376 || loaded.Equip[15].Index != 545 {
		t.Fatalf("round-trip mudou slots: mount=%d cape=%d", loaded.Equip[14].Index, loaded.Equip[15].Index)
	}
}

func TestCharJSONRejectsMissingScore(t *testing.T) {
	raw := []byte(`{"name":"x","equip":{},"inv":[]}`)
	var ch Char
	if err := json.Unmarshal(raw, &ch); err == nil {
		t.Fatal("personagem sem score foi aceito")
	}
}

func TestCharJSONRejectsObsoleteScoreVersion(t *testing.T) {
	raw := []byte(`{"name":"x","score":{"version":1},"equip":{},"inv":[]}`)
	var ch Char
	if err := json.Unmarshal(raw, &ch); err == nil {
		t.Fatal("score obsoleto foi aceito")
	}
}

func TestCharJSONRequiresAllStructuralInventorySlots(t *testing.T) {
	raw := []byte(`{"name":"x","score":{"version":2},"equip":{},"inv":[]}`)
	var ch Char
	if err := json.Unmarshal(raw, &ch); err == nil {
		t.Fatal("inventario incompleto foi aceito")
	}
}

func TestScoreRejectsFunctionalLimitViolations(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*Score)
	}{
		{"save mana above 99", func(e *Score) { e.SaveMana = 100 }},
		{"fire resistance above 100", func(e *Score) { e.ResistFire = 101 }},
		{"ice resistance above 100", func(e *Score) { e.ResistIce = 101 }},
		{"holy resistance above 100", func(e *Score) { e.ResistHoly = 101 }},
		{"thunder resistance above 100", func(e *Score) { e.ResistThunder = 101 }},
		{"hp regen above 255", func(e *Score) { e.RegenHP = 256 }},
		{"mp regen above 255", func(e *Score) { e.RegenMP = 256 }},
		{"current hp above maximum", func(e *Score) { e.CurHP = 101 }},
		{"current mp above maximum", func(e *Score) { e.CurMP = 101 }},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			extended := &Score{Version: ScoreVersion,
				MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100}
			test.mutate(extended)
			if err := extended.ValidatePlayerState(); err == nil {
				t.Fatalf("estado invalido foi aceito: %+v", extended)
			}
		})
	}
}

func TestEmptyCharacterSlotUsesNull(t *testing.T) {
	data, err := json.Marshal(Char{})
	if err != nil {
		t.Fatal(err)
	}
	if string(data) != "null" {
		t.Fatalf("slot vazio serializado como %s", data)
	}
	var loaded Char
	if err := json.Unmarshal(data, &loaded); err != nil {
		t.Fatal(err)
	}
	if !loaded.IsEmpty() {
		t.Fatalf("slot null produziu estado: %+v", loaded)
	}
}

func TestEmptyCharacterSlotDetectsEveryLatePersistedField(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*Char)
	}{
		{"citizenship", func(c *Char) { c.Citizenship = 1 }},
		{"saved x", func(c *Char) { c.SavedX = 2100 }},
		{"saved y", func(c *Char) { c.SavedY = 2100 }},
		{"nightmare tickets", func(c *Char) { c.NightmareTickets = 1 }},
		{"nightmare cooldown", func(c *Char) { c.LastNightmareUnix = 1 }},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			var ch Char
			tc.mutate(&ch)
			if ch.IsEmpty() {
				t.Fatalf("campo persistido foi descartado como slot vazio: %+v", ch)
			}
			if _, err := json.Marshal(ch); err == nil {
				t.Fatal("slot sem nome contendo estado deveria ser recusado")
			}
		})
	}
}

func TestAccountValidateRejectsDuplicateCharacterUID(t *testing.T) {
	uid := "11111111111141118111111111111111"
	validScore := func() *Score {
		return &Score{Version: ScoreVersion, MaxHP: 100, CurHP: 100,
			MaxMP: 100, CurMP: 100}
	}
	account := &Account{Name: "duplicate", PasswordHash: "hash", Chars: []Char{
		{UID: uid, Name: "First", Score: validScore()},
		{UID: uid, Name: "Second", Score: validScore()},
	}}
	if err := account.Validate(); err == nil {
		t.Fatal("dois personagens ativos com o mesmo UID foram aceitos")
	}
}

func TestWideCharJSONPersistsCanonicalScore(t *testing.T) {
	ch := Char{Name: "Wide", Score: &Score{
		Version: ScoreVersion,
		MaxHP:   500_000, MaxMP: 400_000, CurHP: 480_000, CurMP: 350_000,
	}}
	data, err := json.Marshal(ch)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Contains(data, []byte(`"score":`)) || bytes.Contains(data, []byte(`"extendedScore":`)) {
		t.Fatalf("personagem wide nao gravou somente o score canonico: %s", data)
	}
	var loaded Char
	if err := json.Unmarshal(data, &loaded); err != nil {
		t.Fatal(err)
	}
	if loaded.Score == nil || loaded.Score.CurHP != 480_000 || loaded.Score.CurMP != 350_000 {
		t.Fatalf("recursos wide nao foram persistidos: %+v", loaded.Score)
	}
}

func TestNPCUsesCanonicalScoreWithoutProjection(t *testing.T) {
	def := NPCDef{
		Score: &Score{
			Version: ScoreVersion,
			Level:   400, Attack: 350_000, MagicAttack: 420_000, Defense: 275_000,
			MaxHP: 1_000_000, MaxMP: 500_000,
			Str: 180_000, Int: 190_000, Dex: 170_000, Con: 200_000,
			Mastery: [4]uint32{300, 301, 302, 303},
		},
	}
	score := def.MakeScore(750_000)
	if score.Attack != 350_000 || score.MagicAttack != 420_000 ||
		score.Defense != 275_000 || score.CurHP != 750_000 || score.MaxHP != 1_000_000 ||
		score.Mastery[0] != 300 {
		t.Fatalf("NPC perdeu valores do Score canonico: %+v", score)
	}
}

func TestNPCJSONPersistsOnlyScore(t *testing.T) {
	def := NPCDef{Name: "Mago", Score: &Score{
		Version: ScoreVersion, MagicAttack: 123456,
	}}
	data, err := json.Marshal(def)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Contains(data, []byte(`"score"`)) ||
		bytes.Contains(data, []byte(`"magic":`)) {
		t.Fatalf("NPC nao usa estrutura unica: %s", data)
	}
}

func TestNPCEquipLayoutKeepsEffects(t *testing.T) {
	raw := []byte(`{"rosto":{"index":202},"acessorio1":{"index":800},"montaria":{"index":2361},"capa":{"index":3197}}`)
	var equip Equip
	if err := json.Unmarshal(raw, &equip); err != nil {
		t.Fatal(err)
	}
	if equip.Rosto.Index != 202 || equip.Acessorio1.Index != 800 ||
		equip.Montaria.Index != 2361 || equip.Capa.Index != 3197 {
		t.Fatalf("layout de equipamento do NPC incorreto: %+v", equip)
	}

	item := Item{Index: 100, Eff: [6]byte{43, 235}}
	if got := VisualItemCode(item, false); got != 100+11*0x1000 {
		t.Fatalf("codigo visual +11=0x%X", got)
	}
	if got := AncientCode(item); got != 0x20 {
		t.Fatalf("AnctCode +11=0x%X", got)
	}
}

func TestVisualItemCodeHidesDeadMountWithoutDeletingIt(t *testing.T) {
	mount := Item{Index: MountAdultBase}
	mount.SetMountLevel(40)
	mount.SetMountHP(0)
	if got := VisualItemCode(mount, true); got != 0 {
		t.Fatalf("montaria morta projetada como 0x%X, esperado slot vazio", got)
	}
	if mount.Index != MountAdultBase {
		t.Fatalf("projecao alterou item autoritativo: %d", mount.Index)
	}
	mount.SetMountHP(20)
	if got := VisualItemCode(mount, true); got == 0 {
		t.Fatal("montaria revivida continuou visualmente vazia")
	}
}

func TestAccountRejectsReservedInventorySlot(t *testing.T) {
	ch := Char{
		Name:  "Invalido",
		Score: &Score{Version: ScoreVersion},
	}
	ch.Inv[PlayerCarrySlots] = Item{Index: 4011}
	acc := &Account{Name: "conta", PasswordHash: "hash", Chars: []Char{ch}}
	if err := acc.Validate(); err == nil {
		t.Fatal("slot estrutural invisivel do inventario foi aceito")
	}
}

func TestAccountRejectsCharacterWithoutStableUID(t *testing.T) {
	acc := &Account{
		Name: "conta", PasswordHash: "hash",
		Chars: []Char{{
			Name: "SemUID",
			Score: &Score{
				Version: ScoreVersion,
			},
		}},
	}
	if err := acc.Validate(); err == nil {
		t.Fatal("personagem persistido sem UID foi aceito")
	}
}

func TestAccountRejectsActiveCharacterDuplicatedByCelestialCapsule(t *testing.T) {
	uid, err := NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	itemUID, err := NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	ch := Char{
		Name: "Celestial", UID: uid, Evolution: "celestial",
		Score: &Score{Version: ScoreVersion, Level: 90},
	}
	seal := NewCelestialSeal(itemUID, 1)
	acc := &Account{
		Name: "conta", PasswordHash: "hash", Chars: []Char{ch},
		CelestialCapsules: []CelestialCapsule{{
			ID: 1, ItemUID: itemUID, SourceUID: uid, Character: ch,
		}},
	}
	acc.Cargo[0] = seal
	if err := acc.Validate(); err == nil {
		t.Fatal("UID ativo e UID encapsulado foram aceitos simultaneamente")
	}
}

func TestAccountAcceptsCelestialSealInVisibleCarry(t *testing.T) {
	sourceUID, err := NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	carrierUID, err := NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	itemUID, err := NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	sealed := Char{
		UID: sourceUID, Name: "Sealed", Evolution: "celestial",
		Score: &Score{Version: ScoreVersion, Level: 90},
	}
	carrier := Char{
		UID: carrierUID, Name: "Carrier",
		Score: &Score{Version: ScoreVersion},
	}
	carrier.Inv[0] = NewCelestialSeal(itemUID, 7)
	acc := &Account{
		Name: "conta", PasswordHash: "hash", Chars: []Char{carrier},
		CelestialCapsules: []CelestialCapsule{{
			ID: 7, ItemUID: itemUID, SourceUID: sourceUID, Character: sealed,
		}},
	}
	if err := acc.Validate(); err != nil {
		t.Fatalf("selo valido no Carry foi recusado: %v", err)
	}

	acc.Cargo[0] = carrier.Inv[0]
	if err := acc.Validate(); err == nil {
		t.Fatal("selo duplicado no Carry/Cargo foi aceito")
	}
}

func TestAccountRejectsFilledCelestialSealWithoutSnapshot(t *testing.T) {
	carrierUID, err := NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	itemUID, err := NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	carrier := Char{
		UID: carrierUID, Name: "Carrier",
		Score: &Score{Version: ScoreVersion},
	}
	carrier.Inv[0] = NewCelestialSeal(itemUID, 9)
	acc := &Account{Name: "conta", PasswordHash: "hash", Chars: []Char{carrier}}
	if err := acc.Validate(); err == nil {
		t.Fatal("selo preenchido sem snapshot foi aceito")
	}
}

func TestCelestialFormValidationRejectsInvalidPersistentState(t *testing.T) {
	valid := &CelestialForm{
		Evolution: "subcelestial",
		Class:     2,
		Face:      Item{Index: 18},
		Score: &Score{
			Version: ScoreVersion,
			Level:   199,
		},
	}
	if err := valid.Validate(); err != nil {
		t.Fatalf("forma Celestial valida foi recusada: %v", err)
	}

	tooHigh := *valid
	tooHigh.Score = &Score{
		Version: ScoreVersion,
		Level:   200,
	}
	if err := tooHigh.Validate(); err == nil {
		t.Fatal("forma Celestial acima do level interno 199 foi aceita")
	}

	sameForm := Char{
		Name:      "Duplicado",
		Evolution: "celestial",
		Score: &Score{
			Version: ScoreVersion,
		},
		AlternateCelestial: &CelestialForm{
			Evolution: "celestial",
			Face:      Item{Index: 6},
			Score: &Score{
				Version: ScoreVersion,
			},
		},
	}
	raw, err := json.Marshal(sameForm)
	if err != nil {
		t.Fatal(err)
	}
	var decoded Char
	if err := json.Unmarshal(raw, &decoded); err == nil {
		t.Fatal("duas formas Celestial iguais foram aceitas")
	}
}
