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

func TestCharJSONPersistsOnlyExtendedScore(t *testing.T) {
	ch := Char{
		Name:    "felipe",
		NextExp: 649715,
		Extended: &ExtendedScore{
			Version: ExtendedScoreVersion, Level: 50, Attack: 200,
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
		bytes.Contains(data, []byte(`"score":`)) {
		t.Fatalf("JSON persistiu estado legado/derivado: %s", data)
	}
	if !bytes.Contains(data, []byte(`"extendedScore":{"version":2,"level":50,"attack":200`)) {
		t.Fatalf("extended score autoritativo ausente: %s", data)
	}

	var loaded Char
	if err := json.Unmarshal(data, &loaded); err != nil {
		t.Fatal(err)
	}
	if loaded.Extended == nil || loaded.Extended.Level != 50 || loaded.Extended.Attack != 200 {
		t.Fatalf("round-trip perdeu extended score: %+v", loaded.Extended)
	}
	if loaded.Extended.ResistFire != 10 || loaded.Extended.ResistThunder != 40 {
		t.Fatalf("round-trip perdeu resistencias: %+v", loaded.Extended)
	}
}

func TestCharJSONUsesNativeMountAndCapeSlots(t *testing.T) {
	ch := Char{Name: "Slots", Extended: &ExtendedScore{Version: ExtendedScoreVersion}}
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

func TestCharJSONRejectsMissingExtendedScore(t *testing.T) {
	raw := []byte(`{"name":"x","equip":{},"inv":[]}`)
	var ch Char
	if err := json.Unmarshal(raw, &ch); err == nil {
		t.Fatal("personagem sem extendedScore foi aceito")
	}
}

func TestCharJSONRejectsObsoleteExtendedScoreVersion(t *testing.T) {
	raw := []byte(`{"name":"x","extendedScore":{"version":1},"equip":{},"inv":[]}`)
	var ch Char
	if err := json.Unmarshal(raw, &ch); err == nil {
		t.Fatal("extendedScore obsoleto foi aceito")
	}
}

func TestCharJSONRequiresAllStructuralInventorySlots(t *testing.T) {
	raw := []byte(`{"name":"x","extendedScore":{"version":2},"equip":{},"inv":[]}`)
	var ch Char
	if err := json.Unmarshal(raw, &ch); err == nil {
		t.Fatal("inventario incompleto foi aceito")
	}
}

func TestCompatibilityScoreCannotBePersistedAsJSON(t *testing.T) {
	data, err := json.Marshal(WireScore{Level: 400, Attack: 5000, MaxHP: 30000})
	if err != nil {
		t.Fatal(err)
	}
	if string(data) != "{}" {
		t.Fatalf("projecao de ABI foi serializada: %s", data)
	}
}

func TestExtendedScoreRejectsFunctionalLimitViolations(t *testing.T) {
	tests := []struct {
		name   string
		mutate func(*ExtendedScore)
	}{
		{"save mana above 99", func(e *ExtendedScore) { e.SaveMana = 100 }},
		{"parry above 100", func(e *ExtendedScore) { e.Parry = 101 }},
		{"fire resistance above 100", func(e *ExtendedScore) { e.ResistFire = 101 }},
		{"ice resistance above 100", func(e *ExtendedScore) { e.ResistIce = 101 }},
		{"holy resistance above 100", func(e *ExtendedScore) { e.ResistHoly = 101 }},
		{"thunder resistance above 100", func(e *ExtendedScore) { e.ResistThunder = 101 }},
		{"hp regen above 255", func(e *ExtendedScore) { e.RegenHP = 256 }},
		{"mp regen above 255", func(e *ExtendedScore) { e.RegenMP = 256 }},
		{"current hp above maximum", func(e *ExtendedScore) { e.CurHP = 101 }},
		{"current mp above maximum", func(e *ExtendedScore) { e.CurMP = 101 }},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			extended := &ExtendedScore{Version: ExtendedScoreVersion,
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
	validScore := func() *ExtendedScore {
		return &ExtendedScore{Version: ExtendedScoreVersion, MaxHP: 100, CurHP: 100,
			MaxMP: 100, CurMP: 100}
	}
	account := &Account{Name: "duplicate", PasswordHash: "hash", Chars: []Char{
		{UID: uid, Name: "First", Extended: validScore()},
		{UID: uid, Name: "Second", Extended: validScore()},
	}}
	if err := account.Validate(); err == nil {
		t.Fatal("dois personagens ativos com o mesmo UID foram aceitos")
	}
}

func TestExtendedCharJSONNeverPersistsLegacyProjection(t *testing.T) {
	ch := Char{Name: "Wide", Extended: &ExtendedScore{
		Version: ExtendedScoreVersion,
		MaxHP:   500_000, MaxMP: 400_000, CurHP: 480_000, CurMP: 350_000,
	}}
	data, err := json.Marshal(ch)
	if err != nil {
		t.Fatal(err)
	}
	if bytes.Contains(data, []byte(`"score":`)) {
		t.Fatalf("personagem wide ainda gravou score legado: %s", data)
	}
	var loaded Char
	if err := json.Unmarshal(data, &loaded); err != nil {
		t.Fatal(err)
	}
	if loaded.Extended == nil || loaded.Extended.CurHP != 480_000 || loaded.Extended.CurMP != 350_000 {
		t.Fatalf("recursos wide nao foram persistidos: %+v", loaded.Extended)
	}
}

func TestExtendedCompatibilityScoreIsProportionalAndSafe(t *testing.T) {
	e := &ExtendedScore{
		Level: 400, Attack: 300_000, Defense: 200_000,
		MaxHP: 500_000, CurHP: 250_000,
		MaxMP: 400_000, CurMP: 100_000,
		Str: 150_000, Int: 160_000, Dex: 170_000, Con: 180_000,
	}
	score := e.CompatibilityScore()
	if score.Attack != 1_000 || score.Defense != 1_000 ||
		score.Str != 1_000 || score.Int != 1_000 {
		t.Fatalf("projecao de atributos insegura: %+v", score)
	}
	if score.MaxHP > 30_000 || score.MaxMP > 30_000 ||
		score.CurHP*2 < score.MaxHP-1 || score.CurHP*2 > score.MaxHP+1 {
		t.Fatalf("projecao de recursos incorreta: %+v", score)
	}
}

func TestNPCUsesExtendedStatsAndOnlyProjectsAtWireBoundary(t *testing.T) {
	def := NPCDef{
		Extended: &ExtendedScore{
			Version: ExtendedScoreVersion,
			Level:   400, Attack: 350_000, MagicAttack: 420_000, Defense: 275_000,
			MaxHP: 1_000_000, MaxMP: 500_000,
			Str: 180_000, Int: 190_000, Dex: 170_000, Con: 200_000,
			Mastery: [4]uint32{300, 301, 302, 303},
		},
	}
	ext := def.MakeExtendedScore(750_000)
	if ext.Attack != 350_000 || ext.MagicAttack != 420_000 ||
		ext.Defense != 275_000 || ext.CurHP != 750_000 || ext.MaxHP != 1_000_000 ||
		ext.Mastery[0] != 300 {
		t.Fatalf("NPC perdeu valores extended: %+v", ext)
	}
	compatibility := ext.CompatibilityScore()
	if compatibility.Attack != 1_000 || compatibility.Defense != 1_000 ||
		compatibility.MaxHP > 30_000 || compatibility.CurHP*4 < compatibility.MaxHP*3-1 ||
		compatibility.CurHP*4 > compatibility.MaxHP*3+1 || compatibility.Mastery[0] != 255 {
		t.Fatalf("projecao visual do NPC incorreta: %+v", compatibility)
	}
}

func TestNPCJSONPersistsOnlyExtendedScore(t *testing.T) {
	def := NPCDef{Name: "Mago", Extended: &ExtendedScore{
		Version: ExtendedScoreVersion, MagicAttack: 123456,
	}}
	data, err := json.Marshal(def)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Contains(data, []byte(`"extendedScore"`)) ||
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
		Name:     "Invalido",
		Extended: &ExtendedScore{Version: ExtendedScoreVersion},
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
			Extended: &ExtendedScore{
				Version: ExtendedScoreVersion,
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
		Extended: &ExtendedScore{Version: ExtendedScoreVersion, Level: 90},
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
		Extended: &ExtendedScore{Version: ExtendedScoreVersion, Level: 90},
	}
	carrier := Char{
		UID: carrierUID, Name: "Carrier",
		Extended: &ExtendedScore{Version: ExtendedScoreVersion},
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
		Extended: &ExtendedScore{Version: ExtendedScoreVersion},
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
		Extended: &ExtendedScore{
			Version: ExtendedScoreVersion,
			Level:   199,
		},
	}
	if err := valid.Validate(); err != nil {
		t.Fatalf("forma Celestial valida foi recusada: %v", err)
	}

	tooHigh := *valid
	tooHigh.Extended = &ExtendedScore{
		Version: ExtendedScoreVersion,
		Level:   200,
	}
	if err := tooHigh.Validate(); err == nil {
		t.Fatal("forma Celestial acima do level interno 199 foi aceita")
	}

	sameForm := Char{
		Name:      "Duplicado",
		Evolution: "celestial",
		Extended: &ExtendedScore{
			Version: ExtendedScoreVersion,
		},
		AlternateCelestial: &CelestialForm{
			Evolution: "celestial",
			Face:      Item{Index: 6},
			Extended: &ExtendedScore{
				Version: ExtendedScoreVersion,
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
