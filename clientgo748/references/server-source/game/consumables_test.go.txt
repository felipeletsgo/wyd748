package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

// TestDamageBuffAffectScalesByPercent cobre a acao "buff" com affect Type 4: o
// felipe optou por ESCALAR por tier, entao Value = % somado a 100 e aplicado a
// ataque fisico e magico. Kappa(10%)/Competente(15%)/Mental(25%) so mudam Value.
func TestDamageBuffAffectScalesByPercent(t *testing.T) {
	for _, tc := range []struct {
		pct                   int
		wantAtk, wantMagicAtk uint32
	}{
		{10, 110, 220}, {15, 115, 230}, {25, 125, 250},
	} {
		ch := &model.Char{Score: testScore(model.Score{Attack: 100, MagicAttack: 200})}
		ch.Affects[0] = model.Affect{Type: 4, Value: tc.pct, ExpiresAt: time.Now().Add(time.Minute)}
		(&World{}).applyAffectStats(ch)
		e := effectiveScore(ch)
		if e.Attack != tc.wantAtk || e.MagicAttack != tc.wantMagicAtk {
			t.Fatalf("buff %d%%: atk=%d matk=%d (queria %d/%d)",
				tc.pct, e.Attack, e.MagicAttack, tc.wantAtk, tc.wantMagicAtk)
		}
	}
}

func TestParseUseItemRequest(t *testing.T) {
	pkt := make([]byte, 36)
	binary.LittleEndian.PutUint32(pkt[12:16], placeInv)
	binary.LittleEndian.PutUint32(pkt[16:20], 62)
	binary.LittleEndian.PutUint32(pkt[20:24], placeInv)
	binary.LittleEndian.PutUint32(pkt[24:28], 0)
	if req, ok := parseUseItemRequest(pkt); !ok || req.srcPos != 62 {
		t.Fatalf("pacote valido rejeitado: %+v ok=%v", req, ok)
	}
	if _, ok := parseUseItemRequest(pkt[:35]); ok {
		t.Fatal("pacote truncado aceito")
	}
	binary.LittleEndian.PutUint32(pkt[16:20], 256)
	if _, ok := parseUseItemRequest(pkt); ok {
		t.Fatal("slot DWORD foi truncado/aceito")
	}
	binary.LittleEndian.PutUint32(pkt[16:20], model.PlayerCarrySlots)
	if _, ok := parseUseItemRequest(pkt); ok {
		t.Fatal("slot invisivel 63 foi aceito")
	}
}

// TestAccumulateAffectCapsAt24h cobre o bau de EXP e as comidas: cada uso SOMA
// tempo, satura no teto e recusa novos usos (sem consumir) quando cheio.
func TestAccumulateAffectCapsAt24h(t *testing.T) {
	ch := &model.Char{}
	const perUse, cap24h = 900, 10800 // 2h por uso, 24h de teto (blocos de 8s)
	applied := 0
	for i := 0; i < 50; i++ {
		if accumulateAffect(ch, affectDoubleExp, 0, 0, perUse, cap24h) {
			applied++
		}
	}
	if applied == 0 || applied >= 50 {
		t.Fatalf("acumulacao nao saturou no teto: aplicou %d/50", applied)
	}
	a := activePlayerAffect(ch, affectDoubleExp)
	if a == nil {
		t.Fatal("affect do bau expirou indevidamente")
	}
	if remaining := int(time.Until(a.ExpiresAt) / (8 * time.Second)); remaining > cap24h {
		t.Fatalf("duracao ultrapassou o teto de 24h: %d blocos", remaining)
	}
}

func TestVolatileBuffPackageIsAtomic(t *testing.T) {
	rule := model.VolatileRule{Action: "buff", Affects: []model.VolatileAffect{
		{SkillID: 43, Level: 320, DurationUnits: 320},
		{SkillID: 44, Level: 320, DurationUnits: 320},
		{SkillID: 45, Level: 320, DurationUnits: 320},
		{SkillID: 41, Level: 320, DurationUnits: 320},
	}}
	ch := &model.Char{}
	w := &World{skills: map[int]model.SkillDef{
		41: {AffectType: 2, AffectValue: 1},
		43: {AffectType: 11, AffectValue: 15},
		44: {AffectType: 9, AffectValue: 90},
		45: {AffectType: 15, AffectValue: 7},
	}}
	if result := w.applyVolatileBuff(ch, rule, model.Item{}); result != volatileBuffApplied {
		t.Fatal("pacote Love valido foi recusado")
	}
	for affectType, value := range map[byte]int{2: 1, 11: 15, 9: 90, 15: 7} {
		affect := activePlayerAffect(ch, affectType)
		if affect == nil || affect.Value != value {
			t.Fatalf("affect %d do pacote Love = %+v, valor esperado=%d",
				affectType, affect, value)
		}
	}

	// Deixe apenas tres slots disponiveis: o quarto affect deve falhar e a
	// operacao precisa restaurar o personagem inteiro.
	full := &model.Char{}
	for index := 0; index < len(full.Affects)-3; index++ {
		full.Affects[index] = model.Affect{
			// Tipos fora do pacote Love: o teste precisa chegar de fato ao
			// quarto affect sem slot, nao ser recusado antes por colisao.
			Type: byte(100 + index), ExpiresAt: time.Now().Add(time.Hour),
		}
	}
	before := cloneCharacterState(full)
	if result := w.applyVolatileBuff(full, rule, model.Item{}); result != volatileBuffRejected {
		t.Fatal("pacote parcial foi aceito sem quatro slots")
	}
	for index := range full.Affects {
		if full.Affects[index] != before.Affects[index] {
			t.Fatalf("rollback do pacote falhou no slot %d", index)
		}
	}
}

func TestVolatileBuffMissingSkillRollsBackWholePackage(t *testing.T) {
	w := &World{skills: map[int]model.SkillDef{
		43: {AffectType: 11, AffectValue: 15},
	}}
	ch := &model.Char{}
	ch.Affects[0] = model.Affect{
		Type: 4, Value: 25, ExpiresAt: time.Now().Add(time.Hour),
	}
	before := cloneCharacterState(ch)
	rule := model.VolatileRule{Action: "buff", Affects: []model.VolatileAffect{
		{SkillID: 43, DurationUnits: 320},
		{SkillID: 44, DurationUnits: 320},
	}}
	if result := w.applyVolatileBuff(ch, rule, model.Item{}); result != volatileBuffRejected {
		t.Fatal("pacote com SkillData ausente foi aceito")
	}
	for index := range ch.Affects {
		if ch.Affects[index] != before.Affects[index] {
			t.Fatalf("falha de SkillData deixou affect parcial no slot %d", index)
		}
	}
}

// TestDoubleExpBuffDoublesReward garante que a EXP so dobra com o affect 39 ativo.
func TestDoubleExpBuffDoublesReward(t *testing.T) {
	ch := &model.Char{}
	if got := expWithDoubleBuff(ch, 1000); got != 1000 {
		t.Fatalf("sem bau a EXP deveria ser integral: %d", got)
	}
	accumulateAffect(ch, affectDoubleExp, 0, 0, 900, 10800)
	if got := expWithDoubleBuff(ch, 1000); got != 2000 {
		t.Fatalf("com bau a EXP deveria dobrar: %d", got)
	}
	if got := expWithDoubleBuff(ch, ^uint32(0)); got != ^uint32(0) {
		t.Fatalf("clamp de overflow no teto de uint32 falhou: %d", got)
	}
}

// TestSetItemSancRoundTrips garante que gravar um nivel de refino e le-lo de
// volta bate, inclusive no encoding especial de +10..+15.
func TestSetItemSancRoundTrips(t *testing.T) {
	for level := 1; level <= 15; level++ {
		item := model.Item{Index: 100}
		if !setItemSanc(&item, level) {
			t.Fatalf("+%d nao pode ser gravado num item vazio", level)
		}
		if got := itemSanc(item); got != level {
			t.Fatalf("+%d gravado leu como +%d (eff=%v)", level, got, item.Eff)
		}
	}
}

// TestSetItemSancUsesFreeSlotThenRejectsFull cobre a arquitetura: item novo
// (Eff vazio) refina no primeiro slot livre; item com os 3 slots ocupados por
// adds nao-sanc nao pode ganhar sanc e retorna false.
func TestSetItemSancUsesFreeSlotThenRejectsFull(t *testing.T) {
	fresh := model.Item{Index: 100}
	if !setItemSanc(&fresh, 1) || fresh.Eff[0] != 43 || fresh.Eff[1] != 1 {
		t.Fatalf("refino inicial nao usou o slot livre: %v", fresh.Eff)
	}
	// Rerefinar reusa o MESMO slot, nao cria outro.
	setItemSanc(&fresh, 2)
	count := 0
	for i := 0; i < 3; i++ {
		if fresh.Eff[i*2] == 43 {
			count++
		}
	}
	if count != 1 || itemSanc(fresh) != 2 {
		t.Fatalf("rerefino duplicou o slot de sanc: %v", fresh.Eff)
	}

	full := model.Item{Index: 100, Eff: [6]byte{2, 10, 3, 20, 8, 30}} // 3 adds nao-sanc
	if setItemSanc(&full, 1) {
		t.Fatalf("item sem slot livre nao deveria aceitar sanc: %v", full.Eff)
	}
}

// TestRefineChanceCertainAtZero confirma que +0 -> +1 tem chance 100 e que
// sanc fora da tabela recebe chance zero.
func TestRefineChanceCertainAtZero(t *testing.T) {
	if got := refineChance(0); got != 100 {
		t.Fatalf("chance de +0=%d, quer 100", got)
	}
	if refineChance(15) != 0 || refineChance(-1) != 0 {
		t.Fatal("sanc fora da tabela deveria ter chance zero")
	}
	w := &World{rng: fixedRNG{value: 99}} // visible roll 100
	if roll := w.rollPercent(refineChance(0)); !roll.Success || roll.Roll != 100 {
		t.Fatalf("100/100 deveria ser sucesso: %+v", roll)
	}
}

// TestRemovePlayerSummonsDespawnsOnlyOwners cobre o bug do felipe: evocacoes
// orfas seguindo o proximo player a reusar o ID. So as do dono somem.
func TestRemovePlayerSummonsDespawnsOnlyOwners(t *testing.T) {
	w := &World{mobs: []*Mob{
		{ID: 100, SummonerID: 1, Def: &model.NPCDef{}},
		{ID: 101, SummonerID: 1, Def: &model.NPCDef{}},
		{ID: 102, SummonerID: 2, Def: &model.NPCDef{}},
		{ID: 103, Def: &model.NPCDef{}}, // mob normal
	}, summons: make(map[uint16]*Mob)}
	for _, mob := range w.mobs {
		if mob.SummonerID != 0 {
			w.summons[mob.ID] = mob
		}
	}
	w.removePlayerSummons(1)
	if len(w.mobs) != 2 {
		t.Fatalf("deveria sobrar 2 mobs, sobrou %d", len(w.mobs))
	}
	for _, m := range w.mobs {
		if m.SummonerID == 1 {
			t.Fatal("evocacao do dono 1 nao foi removida")
		}
	}
	before := len(w.mobs)
	w.removePlayerSummons(0) // ownerID 0 nunca remove nada
	if len(w.mobs) != before {
		t.Fatal("ownerID 0 removeu mobs indevidamente")
	}
}

func TestConsumeOneSupportsStacks(t *testing.T) {
	stack := model.Item{Index: 400, Eff: [6]byte{effectAmount, 3}}
	consumeOne(&stack)
	if stack.Index != 400 || stack.Eff[1] != 2 {
		t.Fatalf("pilha apos consumo = %+v", stack)
	}
	consumeOne(&stack)
	consumeOne(&stack)
	if stack != (model.Item{}) {
		t.Fatalf("ultima unidade nao zerou o slot: %+v", stack)
	}

	single := model.Item{Index: 405}
	consumeOne(&single)
	if single != (model.Item{}) {
		t.Fatalf("item unitario nao foi consumido: %+v", single)
	}
}

func TestApplyPotionUsesServerCatalogAndClamps(t *testing.T) {
	ch := model.Char{Score: testScore(model.Score{CurHP: 80, MaxHP: 100, CurMP: 20, MaxMP: 100})}
	def := model.ItemDef{StaticEffects: []model.StaticEffect{
		{Name: "EF_VOLATILE", Value: 1}, {Name: "EF_HP", Value: 50}, {Name: "EF_MP", Value: 50},
	}}
	hp, mp := applyRestore(&ch, model.Item{Index: 400}, def,
		model.VolatileRule{ValueSource: "item_effects"})
	if hp != 20 || mp != 50 || playerCurHP(&ch) != 100 || playerCurMP(&ch) != 70 {
		t.Fatalf("resultado da pocao incorreto: hp=%d mp=%d score=%+v", hp, mp, effectiveScore(&ch))
	}
}

func TestApplyPotionReportsEffectiveRestore(t *testing.T) {
	ch := model.Char{Score: testScore(model.Score{CurHP: 90, MaxHP: 100, CurMP: 100, MaxMP: 100})}
	def := model.ItemDef{StaticEffects: []model.StaticEffect{
		{Name: "EF_VOLATILE", Value: 1}, {Name: "EF_HP", Value: 50}, {Name: "EF_MP", Value: 50},
	}}
	hp, mp := applyRestore(&ch, model.Item{Index: 400}, def,
		model.VolatileRule{ValueSource: "item_effects"})
	if hp != 10 || mp != 0 {
		t.Fatalf("ganho efetivo incorreto: hp=%d mp=%d", hp, mp)
	}
}

func TestVolatileGoldValueUsesConfiguredSource(t *testing.T) {
	def := model.ItemDef{Price: 50_000_000, StaticEffects: []model.StaticEffect{
		{Name: "EF_HWORDCOIN", Value: 117}, {Name: "EF_LWORDCOIN", Value: 48},
	}}
	if got := volatileGoldValue(model.VolatileRule{ValueSource: "price"}, model.Item{}, def); got != 50_000_000 {
		t.Fatalf("gold por price=%d", got)
	}
	if got := volatileGoldValue(model.VolatileRule{ValueSource: "coin_effects"}, model.Item{}, def); got != 30_000 {
		t.Fatalf("gold por efeitos=%d", got)
	}
}
