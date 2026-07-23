package game

import (
	"testing"

	"wydgo/internal/model"
)

// mountTestChar cria um TK nivel 1 basico para exercitar o recalc com montaria.
func mountTestChar() *model.Char {
	return &model.Char{
		Class: 0,
		Extended: testExtended(model.ExtendedScore{
			Level: 1, MaxHP: 100, MaxMP: 100, CurHP: 100, CurMP: 100,
			Str: 8, Int: 4, Dex: 7, Con: 6,
		}),
	}
}

// fireTigerCatalog = Tigre de Fogo (tipo 19): {650,100,60,28,6}.
func fireTigerCatalog() model.MountCatalog {
	return model.MountCatalog{Types: map[int]model.MountStats{
		19: {Attack: 650, Magic: 100, Evasion: 60, Resist: 28, Speed: 6, MaxHP: 30000},
	}}
}

func TestMountEquipAddsStatBonus(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{2379: {Index: 2379}}, mounts: fireTigerCatalog()}
	ch := mountTestChar()
	w.recalcPlayer(ch)
	baseAtk := effectiveExtended(ch).Attack

	// Tigre de Fogo adulta (2379, tipo 19), level 0, viva. Bonus dano = (0+20)*650/100 = 130.
	mount := model.Item{Index: 2379}
	mount.SetMountHP(20000)
	ch.Equip[14] = mount
	w.recalcPlayer(ch)
	if got := effectiveExtended(ch).Attack; got != baseAtk+130 {
		t.Fatalf("attack com montaria=%d, quer %d (base %d + 130)", got, baseAtk+130, baseAtk)
	}
	if got := effectiveExtended(ch).AttackRun & 0x0F; got != 6 {
		t.Fatalf("runSpeed=%d, quer piso 6", got)
	}

	// Montaria MORTA (HP 0) nao da bonus.
	dead := model.Item{Index: 2379}
	ch.Equip[14] = dead
	w.recalcPlayer(ch)
	if got := effectiveExtended(ch).Attack; got != baseAtk {
		t.Fatalf("attack com montaria morta=%d, quer %d", got, baseAtk)
	}
}

func TestMountDamageScalesWithLevel(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{2379: {Index: 2379}}, mounts: fireTigerCatalog()}
	ch := mountTestChar()
	w.recalcPlayer(ch)
	base := effectiveExtended(ch).Attack

	mount := model.Item{Index: 2379}
	mount.SetMountHP(20000)
	mount.SetMountLevel(80)
	ch.Equip[14] = mount
	w.recalcPlayer(ch)
	// Dano = (80+20)*650/100 = 650; magia = (80+15)*100/100 = 95.
	if got := effectiveExtended(ch).Attack; got != base+650 {
		t.Fatalf("attack lvl80=%d, quer %d", got, base+650)
	}
}

func TestMountStateAccessorsRoundTrip(t *testing.T) {
	var it model.Item
	it.SetMountHP(20000)
	it.SetMountLevel(99)
	it.SetMountLongev(37)
	it.SetMountFood(58)
	if it.MountHP() != 20000 || it.MountLevel() != 99 || it.MountLongev() != 37 || it.MountFood() != 58 {
		t.Fatalf("round-trip falhou: hp=%d lvl=%d long=%d food=%d",
			it.MountHP(), it.MountLevel(), it.MountLongev(), it.MountFood())
	}
}

func TestMountStageAndTypeHelpers(t *testing.T) {
	if model.MountType(2379) != 19 {
		t.Fatalf("tipo do Fire_Tiger adulto (2379)=%d, quer 19", model.MountType(2379))
	}
	if model.MountType(2349) != 19 {
		t.Fatalf("tipo do Fire_Tiger cria (2349)=%d, quer 19", model.MountType(2349))
	}
	if !model.IsMountAdult(2360) || !model.IsMountBaby(2330) || !model.IsMountEgg(2300) {
		t.Fatal("classificacao de estagio incorreta")
	}
	if mountStageThreshold(2330) != 25 || mountStageThreshold(2335) != 75 || mountStageThreshold(2346) != 100 {
		t.Fatal("limites de evolucao de estagio incorretos")
	}
}

func TestMountEssenceLevelsAndEvolves(t *testing.T) {
	// Cria Pig (2330) no level 24: um amago (2390) casa e sobe pra 25 -> evolui pra
	// adulta (2360), level zera, longevidade sobe.
	mount := model.Item{Index: 2330}
	mount.SetMountLevel(24)
	ok, _ := mountEssence(&mount, 2390)
	if !ok {
		t.Fatal("amago casado deveria funcionar")
	}
	if mount.Index != 2360 || mount.MountLevel() != 0 {
		t.Fatalf("apos evolucao index=%d level=%d, quer 2360/0", mount.Index, mount.MountLevel())
	}
	if mount.MountLongev() < 1 {
		t.Fatalf("longevidade deveria subir na evolucao, veio %d", mount.MountLongev())
	}
	// Amago de tipo errado e recusado.
	other := model.Item{Index: 2360}
	if ok, _ := mountEssence(&other, 2391); ok {
		t.Fatal("amago de outro tipo nao deveria casar")
	}
}

func TestMountAbsorbs25Percent(t *testing.T) {
	w := &World{mounts: fireTigerCatalog()}
	mount := model.Item{Index: 2379}
	mount.SetMountHP(20000)
	p := &Player{Char: mountTestChar()}
	p.Char.Equip[14] = mount
	// 100 de dano: cavaleiro toma 75, montaria absorve 25 no HP.
	if got := w.absorbMountDamage(p, 100); got != 75 {
		t.Fatalf("dano ao cavaleiro=%d, quer 75", got)
	}
	if hp := p.Char.Equip[14].MountHP(); hp != 19975 {
		t.Fatalf("HP da montaria=%d, quer 19975 (absorveu 25)", hp)
	}
	// Sem montaria, dano integral.
	bare := &Player{Char: mountTestChar()}
	if got := w.absorbMountDamage(bare, 100); got != 100 {
		t.Fatalf("sem montaria dano=%d, quer 100", got)
	}
}

func TestMountFreshInitAndFeedRevive(t *testing.T) {
	w := &World{mounts: fireTigerCatalog()}
	fresh := model.Item{Index: 2379} // adulta, Eff zerado
	if !w.initFreshMount(&fresh) {
		t.Fatal("montaria nova deveria inicializar")
	}
	if fresh.MountHP() != 30000 || fresh.MountFood() != model.MountMaxFood || fresh.MountLongev() < 10 {
		t.Fatalf("init: hp=%d food=%d long=%d", fresh.MountHP(), fresh.MountFood(), fresh.MountLongev())
	}
	if w.initFreshMount(&fresh) {
		t.Fatal("montaria ja inicializada nao deve reinicializar")
	}
	// Montaria morta (HP 0) com longevidade: racao revive com 100 HP + 5 comida.
	dead := model.Item{Index: 2360}
	dead.SetMountLongev(20)
	ok, _ := mountFeed(&dead, 2420)
	if !ok || dead.MountHP() != 100 || dead.MountFood() != 5 {
		t.Fatalf("revive: ok=%v hp=%d food=%d", ok, dead.MountHP(), dead.MountFood())
	}
	// Sem longevidade nao revive.
	perma := model.Item{Index: 2360}
	if ok, _ := mountFeed(&perma, 2420); ok {
		t.Fatal("montaria sem longevidade nao deveria reviver")
	}
}

func TestInitShopMountsMakesMountsAlive(t *testing.T) {
	w := &World{
		mounts: fireTigerCatalog(),
		npcs: []model.NPCDef{{
			Name:  "loja",
			Vende: []model.Item{{Index: 2379}, {Index: 787}},
		}},
	}
	w.initShopMounts()
	m := w.npcs[0].Vende[0]
	if m.MountHP() <= 0 || m.MountFood() <= 0 || m.MountLongev() <= 0 {
		t.Fatalf("montaria da loja deveria estar viva: hp=%d food=%d long=%d", m.MountHP(), m.MountFood(), m.MountLongev())
	}
	if w.npcs[0].Vende[1].Eff != ([6]byte{}) {
		t.Fatal("item comum da loja nao deveria ser inicializado")
	}
}

func TestCriaHuntExpLevelsUpAndGatesWeakMobs(t *testing.T) {
	w := &World{mounts: fireTigerCatalog()}
	p := &Player{Char: mountTestChar()} // Session nil: os sends sao pulados
	cria := model.Item{Index: 2349}     // Fire_Tiger's_Baby (tipo 19)
	cria.SetMountHP(20000)
	p.Char.Equip[14] = cria

	strong := &Mob{Def: &model.NPCDef{Extended: &model.ExtendedScore{Level: 50}}}
	w.grantMountHuntExp(p, strong) // 1a morte: progresso, sem subir
	if p.Char.Equip[14].MountLevel() != 0 {
		t.Fatalf("nao deveria subir com 1 morte, veio %d", p.Char.Equip[14].MountLevel())
	}
	w.grantMountHuntExp(p, strong) // 2a morte (criaKillsPerLevel=2): sobe pra 1
	if p.Char.Equip[14].MountLevel() != 1 {
		t.Fatalf("deveria subir pra level 1, veio %d", p.Char.Equip[14].MountLevel())
	}
	// Mob de level menor que a cria nao ensina.
	weak := &Mob{Def: &model.NPCDef{Extended: &model.ExtendedScore{Level: 0}}}
	before := p.Char.Equip[14].Eff[5]
	w.grantMountHuntExp(p, weak)
	if p.Char.Equip[14].Eff[5] != before {
		t.Fatal("mob de level baixo nao deveria dar XP de caca")
	}
	// Montaria adulta nao ganha XP de caca.
	adult := &Player{Char: mountTestChar()}
	am := model.Item{Index: 2379}
	am.SetMountHP(20000)
	adult.Char.Equip[14] = am
	w.grantMountHuntExp(adult, strong)
	if adult.Char.Equip[14].Eff[5] != 0 {
		t.Fatal("adulta nao deveria acumular XP de caca")
	}
}

func TestEggIncubationStateRoundTrip(t *testing.T) {
	var egg model.Item
	egg.Index = 2300 // Pig's_Egg
	if !model.IsMountEgg(egg.Index) {
		t.Fatal("2300 deveria ser ovo")
	}
	setEggProgress(&egg, 3)
	setEggDelay(&egg, 42)
	if eggProgress(egg) != 3 || eggDelay(egg) != 42 {
		t.Fatalf("round-trip do ovo: prog=%d delay=%d", eggProgress(egg), eggDelay(egg))
	}
}

func TestMountFeedRestoresHPAndFood(t *testing.T) {
	mount := model.Item{Index: 2360} // Pig adulto, tipo 0
	mount.SetMountHP(1000)
	mount.SetMountFood(10)
	ok, _ := mountFeed(&mount, 2420) // Pig's_Feed
	if !ok {
		t.Fatal("racao casada deveria funcionar")
	}
	if mount.MountHP() != 6000 || mount.MountFood() != 12 {
		t.Fatalf("apos racao hp=%d food=%d, quer 6000/12", mount.MountHP(), mount.MountFood())
	}
	// Montaria morta nao aceita racao.
	dead := model.Item{Index: 2360}
	if ok, _ := mountFeed(&dead, 2420); ok {
		t.Fatal("montaria morta nao deveria aceitar racao")
	}
}

func TestTintAndUntintChangeSancCodeOnly(t *testing.T) {
	item := &model.Item{Index: 1000}
	item.Eff[0], item.Eff[1] = 43, 5 // EF_SANC +5
	if !tintItem(item, 120) {
		t.Fatal("tintItem deveria pintar item refinado")
	}
	if item.Eff[0] != 120 || item.Eff[1] != 5 {
		t.Fatalf("apos tint Eff=[%d,%d], quer [120,5] (valor preservado)", item.Eff[0], item.Eff[1])
	}
	if !untintItem(item) {
		t.Fatal("untintItem deveria remover a cor")
	}
	if item.Eff[0] != 43 || item.Eff[1] != 5 {
		t.Fatalf("apos untint Eff=[%d,%d], quer [43,5]", item.Eff[0], item.Eff[1])
	}
	// Item sem sanc nao pode ser pintado.
	plain := &model.Item{Index: 1001}
	if tintItem(plain, 116) {
		t.Fatal("tintItem nao deveria pintar item sem brilho de refino")
	}
}

func TestPickVolatileAddHonorsWeightsAndEmptyPool(t *testing.T) {
	if _, ok := pickVolatileAdd(nil); ok {
		t.Fatal("pool vazio deveria devolver ok=false")
	}
	pool := []model.VolatileAdd{{Effect: 2, Value: 40, Weight: 1}}
	got, ok := pickVolatileAdd(pool)
	if !ok || got.Effect != 2 || got.Value != 40 {
		t.Fatalf("pool de 1 entrada deveria devolver ela: %+v ok=%v", got, ok)
	}
}
