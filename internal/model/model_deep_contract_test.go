package model

import (
	"encoding/json"
	"strings"
	"testing"
)

func validDeepChar(name, uid string) Char {
	return Char{
		UID: uid, Name: name, Evolution: "mortal",
		Extended: &ExtendedScore{
			Version: ExtendedScoreVersion,
			Level: 1, MaxHP: 100, CurHP: 100, MaxMP: 50, CurMP: 50,
		},
	}
}

func validDeepAccount() *Account {
	return &Account{
		Name: "account", PasswordHash: "hash",
		Chars: []Char{validDeepChar("Hero", "11111111111141118111111111111111")},
	}
}

func validDeepCelestialForm(evolution string) *CelestialForm {
	return &CelestialForm{
		Evolution: evolution, Class: 1, Face: Item{Index: 1},
		Extended: &ExtendedScore{
			Version: ExtendedScoreVersion, Level: 10,
			MaxHP: 100, CurHP: 100, MaxMP: 50, CurMP: 50,
		},
	}
}

func mutateJSON(t *testing.T, encoded []byte, mutate func(map[string]any)) []byte {
	t.Helper()
	var doc map[string]any
	if err := json.Unmarshal(encoded, &doc); err != nil {
		t.Fatal(err)
	}
	mutate(doc)
	out, err := json.Marshal(doc)
	if err != nil {
		t.Fatal(err)
	}
	return out
}

func TestCoreUIDClampAndItemJSONContracts(t *testing.T) {
	if ClampCP(MinCP-1) != MinCP || ClampCP(MaxCP+1) != MaxCP || ClampCP(7) != 7 ||
		clampCP(-100) != MinCP {
		t.Fatal("ClampCP nao saturou faixa assinada")
	}

	plain := Item{Index: 10}
	body, err := json.Marshal(plain)
	if err != nil || strings.Contains(string(body), "eff") {
		t.Fatalf("item sem efeitos serializou eff: %s err=%v", body, err)
	}
	withEffects := Item{Index: 10, Eff: [6]byte{1, 2, 3, 4, 5, 6}}
	body, err = json.Marshal(withEffects)
	if err != nil || !strings.Contains(string(body), "eff") {
		t.Fatalf("item com efeitos perdeu eff: %s err=%v", body, err)
	}
	if !withEffects.WireEqual(Item{Index: 10, UID: "different", Eff: withEffects.Eff}) ||
		withEffects.WireEqual(Item{Index: 11, Eff: withEffects.Eff}) {
		t.Fatal("WireEqual considerou metadata server-side ou ignorou wire")
	}

	for _, normalize := range []func(string) (string, error){NormalizeItemUID, NormalizeCharacterUID} {
		if got, err := normalize(""); err != nil || got != "" {
			t.Fatalf("UID vazio=%q err=%v", got, err)
		}
		if _, err := normalize("short"); err == nil {
			t.Fatal("UID curto foi aceito")
		}
		if _, err := normalize(strings.Repeat("z", 32)); err == nil {
			t.Fatal("UID nao hexadecimal foi aceito")
		}
		upper := "AAAAAAAAAAAA4AAA8AAAAAAAAAAAAAAA"
		got, err := normalize(upper)
		if err != nil || got != strings.ToLower(upper) {
			t.Fatalf("UID normalizavel=%q err=%v", got, err)
		}
	}
	for _, generate := range []func() (string, error){NewItemUID, NewCharacterUID} {
		uid, err := generate()
		if err != nil || len(uid) != 32 || uid[12] != '4' || !strings.Contains("89ab", string(uid[16])) {
			t.Fatalf("UID v4 invalido=%q err=%v", uid, err)
		}
	}
}

func TestExtendedScoreValidationAndCompatibilityCoversAllLimits(t *testing.T) {
	var nilScore *ExtendedScore
	if nilScore.Validate() == nil || nilScore.ValidatePlayerState() == nil {
		t.Fatal("ExtendedScore nil foi aceito")
	}
	if err := (&ExtendedScore{Version: ExtendedScoreVersion + 1}).Validate(); err == nil {
		t.Fatal("versao errada foi aceita")
	}

	base := ExtendedScore{Version: ExtendedScoreVersion, MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100}
	tooHigh := MaxExtendedScoreValue + 1
	fields := []func(*ExtendedScore){
		func(e *ExtendedScore) { e.Level = tooHigh }, func(e *ExtendedScore) { e.Attack = tooHigh },
		func(e *ExtendedScore) { e.MagicAttack = tooHigh }, func(e *ExtendedScore) { e.Defense = tooHigh },
		func(e *ExtendedScore) { e.MaxHP = tooHigh }, func(e *ExtendedScore) { e.MaxMP = tooHigh },
		func(e *ExtendedScore) { e.CurHP = tooHigh }, func(e *ExtendedScore) { e.CurMP = tooHigh },
		func(e *ExtendedScore) { e.Str = tooHigh }, func(e *ExtendedScore) { e.Int = tooHigh },
		func(e *ExtendedScore) { e.Dex = tooHigh }, func(e *ExtendedScore) { e.Con = tooHigh },
		func(e *ExtendedScore) { e.Accuracy = tooHigh }, func(e *ExtendedScore) { e.Evasion = tooHigh },
		func(e *ExtendedScore) { e.Parry = tooHigh }, func(e *ExtendedScore) { e.Critical = tooHigh },
		func(e *ExtendedScore) { e.Range = tooHigh }, func(e *ExtendedScore) { e.ResistFire = tooHigh },
		func(e *ExtendedScore) { e.ResistIce = tooHigh }, func(e *ExtendedScore) { e.ResistHoly = tooHigh },
		func(e *ExtendedScore) { e.ResistThunder = tooHigh }, func(e *ExtendedScore) { e.SaveMana = tooHigh },
		func(e *ExtendedScore) { e.MagicAmp = tooHigh }, func(e *ExtendedScore) { e.RegenHP = tooHigh },
		func(e *ExtendedScore) { e.RegenMP = tooHigh }, func(e *ExtendedScore) { e.StatusPts = tooHigh },
		func(e *ExtendedScore) { e.MasterPts = tooHigh }, func(e *ExtendedScore) { e.SkillPts = tooHigh },
	}
	for i, mutate := range fields {
		e := base
		mutate(&e)
		if err := e.Validate(); err == nil {
			t.Fatalf("campo wide %d acima do maximo foi aceito", i)
		}
	}
	e := base
	e.Mastery[2] = tooHigh
	if err := e.Validate(); err == nil {
		t.Fatal("mastery wide acima do maximo foi aceita")
	}

	playerLimits := []func(*ExtendedScore){
		func(e *ExtendedScore) { e.ResistFire = 101 }, func(e *ExtendedScore) { e.ResistIce = 101 },
		func(e *ExtendedScore) { e.ResistHoly = 101 }, func(e *ExtendedScore) { e.ResistThunder = 101 },
		func(e *ExtendedScore) { e.SaveMana = 100 }, func(e *ExtendedScore) { e.RegenHP = 256 },
		func(e *ExtendedScore) { e.RegenMP = 256 }, func(e *ExtendedScore) { e.CurHP = e.MaxHP + 1 },
		func(e *ExtendedScore) { e.CurMP = e.MaxMP + 1 },
	}
	for i, mutate := range playerLimits {
		e := base
		mutate(&e)
		if err := e.ValidatePlayerState(); err == nil {
			t.Fatalf("limite funcional %d foi aceito", i)
		}
	}
	if err := base.ValidatePlayerState(); err != nil {
		t.Fatalf("score valido recusado: %v", err)
	}

	if CompatibilityHPScale(30_000) != 1 || CompatibilityHPScale(30_001) != 2 {
		t.Fatal("CompatibilityHPScale divergiu")
	}
	if ProjectHPDelta(0, 10_000) != 0 || ProjectHPDelta(1, 60_000) != 1 ||
		ProjectHPDelta(999_999, 0) != 30_000 || ProjectHPDelta(999_999, 60_000) != 30_000 {
		t.Fatal("ProjectHPDelta nao cobriu arredondamento/teto")
	}
	cur, max := projectCompatibilityPair(200, 100)
	if cur != 100 || max != 100 {
		t.Fatalf("pair clamp=%d/%d", cur, max)
	}
	cur, max = projectCompatibilityPair(0, 60_000)
	if cur != 0 || max != 30_000 {
		t.Fatalf("pair wide=%d/%d", cur, max)
	}
	if minScoreValue(3, 4) != 3 || minScoreValue(5, 4) != 4 {
		t.Fatal("minScoreValue divergiu")
	}
	if nilScore.CompatibilityScore() != (WireScore{}) {
		t.Fatal("CompatibilityScore nil nao retornou zero")
	}
	wide := ExtendedScore{Level: 100_000, Defense: 2_000, Attack: 3_000, Str: 4_000, Int: 5_000, Dex: 6_000, Con: 7_000,
		MaxHP: 60_000, CurHP: 90_000, MaxMP: 60_000, CurMP: 30_000, Mastery: [4]uint32{300, 1, 2, 3}, Merchant: 4, AttackRun: 5}
	projected := wide.CompatibilityScore()
	if projected.Level != 65_535 || projected.Defense != 1_000 || projected.Attack != 1_000 ||
		projected.Str != 1_000 || projected.Mastery[0] != 255 || projected.CurHP != projected.MaxHP ||
		projected.Merchant != 4 || projected.AttackRun != 5 {
		t.Fatalf("CompatibilityScore wide=%+v", projected)
	}
}

func TestCelestialFormValidationCoversAllBranches(t *testing.T) {
	var nilForm *CelestialForm
	if err := nilForm.Validate(); err != nil {
		t.Fatalf("forma nil deveria ser valida: %v", err)
	}
	for _, tc := range []struct {
		name string
		form *CelestialForm
	}{
		{"evolution", func() *CelestialForm { f := validDeepCelestialForm("arch"); return f }()},
		{"class", func() *CelestialForm { f := validDeepCelestialForm("celestial"); f.Class = 4; return f }()},
		{"face", func() *CelestialForm { f := validDeepCelestialForm("celestial"); f.Face = Item{}; return f }()},
		{"score", func() *CelestialForm { f := validDeepCelestialForm("celestial"); f.Extended = nil; return f }()},
		{"level", func() *CelestialForm { f := validDeepCelestialForm("celestial"); f.Extended.Level = 200; return f }()},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if err := tc.form.Validate(); err == nil {
				t.Fatal("forma invalida foi aceita")
			}
		})
	}
	for _, evo := range []string{" celestial ", "SUBCELESTIAL"} {
		if err := validDeepCelestialForm(evo).Validate(); err != nil {
			t.Fatalf("forma valida %q recusada: %v", evo, err)
		}
	}
}

func TestCharJSONStrictnessChaosMigrationAndProgression(t *testing.T) {
	var nilChar *Char
	if !nilChar.IsEmpty() || !(&Char{}).IsEmpty() || (&Char{QuestsDone: []int32{1}}).IsEmpty() {
		t.Fatal("IsEmpty divergiu")
	}
	if body, err := json.Marshal(Char{}); err != nil || string(body) != "null" {
		t.Fatalf("char vazio=%s err=%v", body, err)
	}
	if _, err := json.Marshal(Char{Gold: 1}); err == nil {
		t.Fatal("char sem nome com estado foi serializado")
	}

	char := validDeepChar("Hero", "11111111111141118111111111111111")
	body, err := json.Marshal(char)
	if err != nil || !strings.Contains(string(body), `"chaosVersion":1`) {
		t.Fatalf("char atual nao marcou chaos: %s err=%v", body, err)
	}
	var decoded Char
	if err := json.Unmarshal([]byte("null"), &decoded); err != nil || !decoded.IsEmpty() {
		t.Fatalf("unmarshal null=%+v err=%v", decoded, err)
	}
	for _, bad := range [][]byte{
		[]byte(`{"name":`),
		mutateJSON(t, body, func(m map[string]any) { m["unknown"] = 1 }),
		append(append([]byte(nil), body...), []byte(` {}`)...),
	} {
		if err := json.Unmarshal(bad, &decoded); err == nil {
			t.Fatalf("char JSON invalido foi aceito: %s", bad)
		}
	}

	legacyZero := mutateJSON(t, body, func(m map[string]any) {
		delete(m, "chaosVersion"); delete(m, "cp"); m["chaos"] = 0
	})
	if err := json.Unmarshal(legacyZero, &decoded); err != nil || decoded.CP != 0 || decoded.ChaosVersion != ChaosFormatVersion {
		t.Fatalf("legacy chaos zero=%+v err=%v", decoded, err)
	}
	legacyHigh := mutateJSON(t, body, func(m map[string]any) {
		delete(m, "chaosVersion"); delete(m, "cp"); m["chaos"] = 150
	})
	if err := json.Unmarshal(legacyHigh, &decoded); err != nil || decoded.CP != MaxCP {
		t.Fatalf("legacy chaos high=%+v err=%v", decoded, err)
	}
	both := mutateJSON(t, body, func(m map[string]any) { m["chaos"] = 75; m["cp"] = 0 })
	if err := json.Unmarshal(both, &decoded); err == nil {
		t.Fatal("cp+chaos simultaneos foram aceitos")
	}
	oldNeutral := mutateJSON(t, body, func(m map[string]any) { delete(m, "chaosVersion"); m["cp"] = float64(MaxCP) })
	if err := json.Unmarshal(oldNeutral, &decoded); err != nil || decoded.CP != 0 || decoded.ChaosVersion != ChaosFormatVersion {
		t.Fatalf("migracao cp75=%+v err=%v", decoded, err)
	}

	mutations := []func(map[string]any){
		func(m map[string]any) { m["chaosVersion"] = 99 },
		func(m map[string]any) { m["chaosVersion"] = 1; m["cp"] = 76 },
		func(m map[string]any) { m["extendedScore"] = nil },
		func(m map[string]any) { m["archCrystals"] = 5 },
		func(m map[string]any) { m["soulInfo"] = 11 },
		func(m map[string]any) { m["celestialArchTier"] = 6 },
		func(m map[string]any) { m["celestialCytheraTier"] = 5 },
		func(m map[string]any) { m["evolution"] = "celestial"; ext := m["extendedScore"].(map[string]any); ext["level"] = 200 },
		func(m map[string]any) { m["alternateCelestial"] = map[string]any{"evolution":"celestial","class":1,"face":map[string]any{"index":1},"extendedScore":map[string]any{"version":2,"level":10,"maxHP":1,"curHP":1,"maxMP":1,"curMP":1}} },
		func(m map[string]any) { m["evolution"] = "celestial"; m["alternateCelestial"] = map[string]any{"evolution":"celestial","class":1,"face":map[string]any{"index":1},"extendedScore":map[string]any{"version":2,"level":10,"maxHP":1,"curHP":1,"maxMP":1,"curMP":1}} },
		func(m map[string]any) { m["inv"] = []any{} },
	}
	for i, mutate := range mutations {
		bad := mutateJSON(t, body, mutate)
		if err := json.Unmarshal(bad, &decoded); err == nil {
			t.Fatalf("mutacao Char invalida %d foi aceita", i)
		}
	}

	validAlt := char
	validAlt.Evolution = "celestial"
	validAlt.AlternateCelestial = validDeepCelestialForm("subcelestial")
	altBody, err := json.Marshal(validAlt)
	if err != nil || json.Unmarshal(altBody, &decoded) != nil || decoded.AlternateCelestial == nil {
		t.Fatalf("dupla forma valida nao fez round-trip: err=%v decoded=%+v", err, decoded)
	}
}

func TestAccountJSONAndValidationCoverIdentitySlotsTimingAndCapsuleErrors(t *testing.T) {
	account := validDeepAccount()
	body, err := json.Marshal(account)
	if err != nil {
		t.Fatal(err)
	}
	var decoded Account
	if err := json.Unmarshal(body, &decoded); err != nil || decoded.Name != account.Name {
		t.Fatalf("round-trip account=%+v err=%v", decoded, err)
	}
	for _, bad := range [][]byte{
		mutateJSON(t, body, func(m map[string]any) { m["unknown"] = 1 }),
		append(append([]byte(nil), body...), []byte(` {}`)...),
		mutateJSON(t, body, func(m map[string]any) { m["cargo"] = []any{} }),
	} {
		if err := json.Unmarshal(bad, &decoded); err == nil {
			t.Fatalf("account JSON invalido aceito: %s", bad)
		}
	}

	var nilAccount *Account
	if nilAccount.Validate() == nil {
		t.Fatal("Account nil aceito")
	}
	cases := []func(*Account){
		func(a *Account) { a.Name = "" },
		func(a *Account) { a.PasswordHash = "" },
		func(a *Account) { a.Chars = make([]Char, 5) },
		func(a *Account) { a.Chars = append(a.Chars, Char{Gold: 1}) },
		func(a *Account) { a.Chars[0].Extended.CurHP = 101 },
		func(a *Account) { a.Chars[0].AlternateCelestial = validDeepCelestialForm("arch") },
		func(a *Account) { a.Chars[0].UID = "" },
		func(a *Account) { a.Chars[0].UID = strings.ToUpper(a.Chars[0].UID) },
		func(a *Account) { a.Chars = append(a.Chars, validDeepChar("Other", a.Chars[0].UID)) },
		func(a *Account) { a.Chars[0].ArchMortalUID = "bad" },
		func(a *Account) { a.Chars[0].Inv[PlayerCarrySlots] = Item{Index: 1} },
		func(a *Account) { a.Chars[0].Equip[0] = Item{Index: 1, ActivatedUnix: -1} },
		func(a *Account) { a.Chars[0].Inv[0] = Item{Index: 1, ActivatedUnix: 1, ExpiresUnix: 0} },
		func(a *Account) { a.Cargo[0] = Item{Index: 1, ActivatedUnix: 10, ExpiresUnix: 5} },
		func(a *Account) { a.Cargo[0] = Item{UID: "11111111111141118111111111111111"} },
		func(a *Account) { a.Cargo[0] = Item{Index: 1, UID: "bad", ActivatedUnix: 1, ExpiresUnix: 2} },
		func(a *Account) { a.Cargo[PlayerCargoSlots] = Item{Index: 1} },
	}
	for i, mutate := range cases {
		a := validDeepAccount()
		mutate(a)
		if err := a.Validate(); err == nil {
			t.Fatalf("Account invalida %d foi aceita", i)
		}
	}
	if err := validDeepAccount().Validate(); err != nil {
		t.Fatalf("Account valida recusada: %v", err)
	}

	// Selo preenchido sem snapshot deve falhar antes de qualquer lookup de UI.
	sealOnly := validDeepAccount()
	sealOnly.Cargo[0] = NewCelestialSeal("aaaaaaaaaaaa4aaa8aaaaaaaaaaaaaaa", 7)
	if err := sealOnly.Validate(); err == nil {
		t.Fatal("selo preenchido sem snapshot foi aceito")
	}
}

func TestVisualItemAndAncientProjectionCoversAllNativeBranches(t *testing.T) {
	if VisualItemCode(Item{}, false) != 0 || AncientCode(Item{}) != 0 {
		t.Fatal("item vazio ganhou visual")
	}

	deadMount := Item{Index: MountAdultBase}
	deadMount.SetMountHP(0)
	if VisualItemCode(deadMount, true) != 0 {
		t.Fatal("montaria morta permaneceu visivel")
	}
	aliveMount := Item{Index: MountAdultBase}
	aliveMount.SetMountHP(100)
	aliveMount.Eff[2] = 20
	if got := VisualItemCode(aliveMount, true); got != aliveMount.Index+2*0x1000 {
		t.Fatalf("visual mount=%d", got)
	}
	legacyDead := Item{Index: 2360, Eff: [6]byte{0, 128}}
	legacyDead.SetMountHP(100)
	legacyDead.Eff[1] = 128
	if VisualItemCode(legacyDead, true) != 0 {
		t.Fatal("montaria legacy morta permaneceu visivel")
	}

	plain := Item{Index: 100}
	if VisualItemCode(plain, false) != 100 || AncientCode(plain) != 0 {
		t.Fatal("item sem refino mudou visual")
	}
	colored := Item{Index: 100, Eff: [6]byte{116, 20}}
	if VisualItemCode(colored, false) != 100+9*0x1000 || AncientCode(colored) != 116 {
		t.Fatal("tintura nao saturou/projetou")
	}
	refined := Item{Index: 100, Eff: [6]byte{43, 17}}
	if VisualItemCode(refined, false) != 100+7*0x1000 || AncientCode(refined) != 43 {
		t.Fatal("refino comum divergiu")
	}
	for value, ancient := range map[byte]byte{230: 0x10, 231: 0x20, 232: 0x30, 233: 0x40, 255: 0x20} {
		item := Item{Index: 100, Eff: [6]byte{43, value}}
		if got := AncientCode(item); got != ancient {
			t.Fatalf("AncientCode(%d)=%02x want=%02x", value, got, ancient)
		}
		visual := VisualItemCode(item, false)
		if visual == item.Index {
			t.Fatalf("ancient %d nao alterou visual", value)
		}
	}

	equip := Equip{Rosto: plain, Montaria: aliveMount, Capa: refined}
	items := equip.Items()
	if items[0].Index != plain.Index || items[14].Index != aliveMount.Index || items[15].Index != refined.Index {
		t.Fatal("Equip.Items perdeu slots")
	}
	if equip.Slots()[0] != plain.Index || equip.AncientCodes()[15] != 43 {
		t.Fatal("projecoes Equip divergiram")
	}
}
