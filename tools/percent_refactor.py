#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def read(path):
    return (ROOT / path).read_text(encoding="utf-8")

def write(path, text):
    (ROOT / path).write_text(text, encoding="utf-8")

def replace_once(text, old, new, label):
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected 1 occurrence, found {count}")
    return text.replace(old, new, 1)

write("internal/game/percent_roll.go", r'''package game

import "fmt"

type percentRoll struct {
	Roll    int
	Chance  int
	Success bool
}

func (w *World) rollPercent(chance int) percentRoll {
	chance = clampInt(chance, 0, 100)
	roll := w.intn(100) + 1
	return percentRoll{
		Roll:    roll,
		Chance:  chance,
		Success: roll <= chance,
	}
}

func (r percentRoll) message() string {
	status := "Fail"
	if r.Success {
		status = "Success"
	}
	return fmt.Sprintf("%s: %d/%d", status, r.Roll, r.Chance)
}

func (r percentRoll) namedMessage(name string) string {
	if name == "" {
		return r.message()
	}
	return fmt.Sprintf("%s %s", name, r.message())
}
''')

write("internal/game/percent_roll_test.go", r'''package game

import "testing"

func TestPercentRollUsesOneToHundredInclusive(t *testing.T) {
	tests := []struct {
		name     string
		raw      int
		chance   int
		wantRoll int
		success  bool
		message  string
	}{
		{name: "95 succeeds at boundary", raw: 94, chance: 95, wantRoll: 95, success: true, message: "Success: 95/95"},
		{name: "95 fails above boundary", raw: 95, chance: 95, wantRoll: 96, success: false, message: "Fail: 96/95"},
		{name: "5 succeeds at boundary", raw: 4, chance: 5, wantRoll: 5, success: true, message: "Success: 5/5"},
		{name: "5 fails above boundary", raw: 5, chance: 5, wantRoll: 6, success: false, message: "Fail: 6/5"},
		{name: "100 includes 100", raw: 99, chance: 100, wantRoll: 100, success: true, message: "Success: 100/100"},
		{name: "zero never succeeds", raw: 0, chance: 0, wantRoll: 1, success: false, message: "Fail: 1/0"},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w := &World{rng: fixedRNG{value: tc.raw}}
			got := w.rollPercent(tc.chance)
			if got.Roll != tc.wantRoll || got.Chance != tc.chance || got.Success != tc.success {
				t.Fatalf("roll=%+v, want roll=%d chance=%d success=%t", got, tc.wantRoll, tc.chance, tc.success)
			}
			if msg := got.message(); msg != tc.message {
				t.Fatalf("message=%q, want %q", msg, tc.message)
			}
		})
	}
}
''')

path = "internal/game/crafting.go"
text = read(path)
text = replace_once(text, '\t"math/rand"\n', "", "crafting remove math/rand")
text = replace_once(text, r'''func combineRoll() int {
	roll := rand.Intn(115)
	if roll >= 100 {
		roll -= 15
	}
	return roll
}

''', "", "remove biased combineRoll")
old = r'''func (w *World) sendCombineResult(p *Player, result uint32) {
	if p != nil && p.Session != nil {
		// Fluxo nativo W2PP/Micronics: a mensagem textual e enviada antes do
		// MSG_CombineComplete (0x3A7). O sinal sozinho atualiza/fecha a janela,
		// mas nao apresenta ao jogador o motivo do resultado.
		if message := combineResultMessage(result); message != "" {
			p.Session.Send(wire.MessagePanel(message))
		}
		p.Session.Send(wire.StandardParm(wire.OpCombineComplete, wire.SceneField, result))
	}
}
'''
new = r'''func (w *World) sendCombineResult(p *Player, result uint32) {
	w.sendCombineResultMessage(p, result, "")
}

func (w *World) sendCombineResultMessage(p *Player, result uint32, message string) {
	if p != nil && p.Session != nil {
		// Fluxo nativo W2PP/Micronics: a mensagem textual e enviada antes do
		// MSG_CombineComplete (0x3A7). O sinal sozinho atualiza/fecha a janela.
		// Quando existe roll percentual, a mensagem precisa usar exatamente a
		// decisao persistida, nunca uma segunda rolagem para exibicao.
		if message == "" {
			message = combineResultMessage(result)
		}
		if message != "" {
			p.Session.Send(wire.MessagePanel(message))
		}
		p.Session.Send(wire.StandardParm(wire.OpCombineComplete, wire.SceneField, result))
	}
}
'''
text = replace_once(text, old, new, "sendCombineResult dynamic message")
text = replace_once(text,
    "success := combineRoll() <= chance",
    "roll := w.rollPercent(chance)\n\tsuccess := roll.Success",
    "Tiny roll")
text = replace_once(text,
    "if w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code) {\n\t\tlogCraftResult(s, success, chance, p.Char.Gold)\n\t}",
    "if w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll) {\n\t\tlogCraftResult(s, roll, p.Char.Gold)\n\t}",
    "Tiny commit")
old = r'''func logCraftResult(s *net.Session, success bool, chance int, gold uint32) {
	if s == nil {
		return
	}
	status := "falha"
	if success {
		status = "sucesso"
	}
	// Sem despejar os STRUCT_ITEM no log: a linha operacional fica curta para
	// auditoria e nao polui a saida com bytes de efeitos.
	log.Printf("[#%d] CRAFT Tiny %s chance=%d gold=%d", s.ID, status, chance, gold)
}
'''
new = r'''func logCraftResult(s *net.Session, roll percentRoll, gold uint32) {
	if s == nil {
		return
	}
	status := "falha"
	if roll.Success {
		status = "sucesso"
	}
	// Sem despejar os STRUCT_ITEM no log: a linha operacional fica curta para
	// auditoria e nao polui a saida com bytes de efeitos.
	log.Printf("[#%d] CRAFT Tiny %s roll=%d/%d gold=%d",
		s.ID, status, roll.Roll, roll.Chance, gold)
}
'''
text = replace_once(text, old, new, "Tiny log")
if "combineRoll()" in text or "rand.Intn(100)" in text:
    raise SystemExit("crafting.go still contains legacy percentage RNG")
write(path, text)

path = "internal/game/crafting_simple.go"
text = read(path)
text = replace_once(text, '\t"math/rand"\n', "", "crafting_simple remove math/rand")
text = replace_once(text, "now := time.Now()", "now := w.now()", "craft cooldown uses World clock")

old = r'''func (w *World) commitCombine(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{}, equipSlots map[int]struct{}, result uint32) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, nil)
}

func (w *World) commitCombineWithPlayerState(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccountAndCharStateResult(p) }, nil)
}

func (w *World) commitCombineWithRollback(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, rollback func()) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, rollback)
}

func (w *World) commitCombineWithSave(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, persist func() error, rollback func()) bool {
'''
new = r'''func (w *World) commitCombine(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{}, equipSlots map[int]struct{}, result uint32) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, nil, "")
}

func (w *World) commitCombineRoll(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, roll percentRoll) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, nil, roll.message())
}

func (w *World) commitCombineWithPlayerState(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccountAndCharStateResult(p) }, nil, "")
}

func (w *World) commitCombineWithRollback(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, rollback func()) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, rollback, "")
}

func (w *World) commitCombineWithRollbackRoll(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, rollback func(), roll percentRoll) bool {
	return w.commitCombineWithSave(p, oldInv, oldEquip, oldGold, invSlots, equipSlots,
		result, func() error { return w.saveAccount(p.Account) }, rollback, roll.message())
}

func (w *World) commitCombineWithSave(p *Player, oldInv [model.MaxCarry]model.Item,
	oldEquip [16]model.Item, oldGold uint32, invSlots map[int]struct{},
	equipSlots map[int]struct{}, result uint32, persist func() error, rollback func(), resultMessage string) bool {
'''
text = replace_once(text, old, new, "craft commit wrappers")
old = r''' 		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.sendCombineResult(p, 0)
		return false
'''.replace(' ', '', 1)
new = r''' 		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.sendCombineResultMessage(p, 0, "Save failed. Reconnect to reload the authoritative state.")
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "composition", err)
		return false
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "craft persistence poison")
text = replace_once(text,
    "\tw.sendCombineResult(p, result)\n\treturn true\n}",
    "\tw.sendCombineResultMessage(p, result, resultMessage)\n\treturn true\n}",
    "craft persisted result message")
text = replace_once(text,
    "success := combineRoll() <= clampInt(chance, 0, 100)",
    "roll := w.rollPercent(chance)\n\tsuccess := roll.Success",
    "Compositor roll")
text = replace_once(text,
    "if w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code) {\n\t\tlog.Printf(\"[#%d] CRAFT Compositor sucesso=%t chance=%d (base %d + %s)\",\n\t\t\ts.ID, success, chance, w.gameplay.CompositorBaseChance, breakdown)\n\t}",
    "if w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll) {\n\t\tlog.Printf(\"[#%d] CRAFT Compositor sucesso=%t roll=%d/%d (base %d + %s)\",\n\t\t\ts.ID, success, roll.Roll, roll.Chance, w.gameplay.CompositorBaseChance, breakdown)\n\t}",
    "Compositor commit/log")
text = replace_once(text,
    "success := combineRoll() <= chance",
    "roll := w.rollPercent(chance)\n\tsuccess := roll.Success",
    "Agatha roll")
old = r''' 	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)
}

func (w *World) onCombineAylin'''.replace(' ', '', 1)
new = r''' 	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll)
}

func (w *World) onCombineAylin'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Agatha commit")
old = r''' 	// Secrets 7.54 usa `rand()%100 <= 40`: sao 41 resultados (0..40).
	success := aylinRollSucceeds(rand.Intn(100))
'''.replace(' ', '', 1)
new = r''' 	// Politica do servidor: percentuais usam 1..100 e sucesso em roll <= chance.
	// Aylin portanto possui exatamente 40% de chance, sem o 41o valor do legado.
	roll := w.rollPercent(40)
	success := roll.Success
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Aylin roll")
old = r''' 	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)
}

func aylinRollSucceeds(roll int) bool {
	return roll >= 0 && roll <= 40
}

func (w *World) onCombineLindy'''.replace(' ', '', 1)
new = r''' 	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll)
}

func (w *World) onCombineLindy'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Aylin commit/remove helper")
if "combineRoll()" in text or "rand.Intn(100)" in text or "aylinRollSucceeds" in text:
    raise SystemExit("crafting_simple.go still contains legacy percentage RNG")
write(path, text)

path = "internal/game/crafting_advanced.go"
text = read(path)
text = replace_once(text, '\t"math/rand"\n', "", "crafting_advanced remove math/rand")
text = replace_once(text, "success := rand.Intn(100) < chance", "roll := w.rollPercent(chance)\n\t\tsuccess := roll.Success", "Ehre roll")
text = replace_once(text, "w.commitCombineWithRollback(p, oldInv, oldEquip, oldGold, changed, nil, code,\n\t\t\tfunc() { p.Char.Exp = oldExp })", "w.commitCombineWithRollbackRoll(p, oldInv, oldEquip, oldGold, changed, nil, code,\n\t\t\tfunc() { p.Char.Exp = oldExp }, roll)", "Ehre roll commit")
text = replace_once(text, "success := w.intn(100) < chance", "roll := w.rollPercent(chance)\n\tsuccess := roll.Success", "Secret Stone roll")
old = r''' 	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)
}

func (w *World) onCombineOdin'''.replace(' ', '', 1)
new = r''' 	code := uint32(2)
	if success {
		code = 1
	}
	w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll)
}

func (w *World) onCombineOdin'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Secret Stone commit")
text = replace_once(text, "success := rand.Intn(100) < clampInt(chance, 0, 100)", "roll := w.rollPercent(chance)\n\t\t\tsuccess := roll.Success", "Odin high refine roll")
old = r''' 			code := uint32(2)
			if success {
				code = 1
			}
			w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code)
			return
'''.replace(' ', '', 1)
new = r''' 			code := uint32(2)
			if success {
				code = 1
			}
			w.commitCombineRoll(p, oldInv, oldEquip, oldGold, changed, nil, code, roll)
			return
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Odin high refine commit")
if "rand.Intn(100)" in text or "w.intn(100) <" in text:
    raise SystemExit("crafting_advanced.go still contains legacy percentage RNG")
write(path, text)

path = "internal/game/consumables.go"
text = read(path)
old = r'''// refineRoll porta a rolagem de refino do W2PP (_MSG_UseItem.cpp:424): sorteia
// 0..114, corrige a faixa alta e compara com g_pCelestialRate[sanc]. Sanc fora
// da tabela nunca refina.
func refineRoll(sanc int) bool {
	if sanc < 0 || sanc >= len(celestialRate) {
		return false
	}
	rd := rand.Intn(115)
	if rd > 100 {
		rd -= 15
	}
	return rd <= celestialRate[sanc]
}
'''
new = r'''// refineChance devolve a chance configurada para o nivel atual. A decisao
// percentual e centralizada em World.rollPercent: dominio uniforme 1..100 e
// sucesso quando roll <= chance.
func refineChance(sanc int) int {
	if sanc < 0 || sanc >= len(celestialRate) {
		return 0
	}
	return clampInt(celestialRate[sanc], 0, 100)
}
'''
text = replace_once(text, old, new, "replace refineRoll")
text = replace_once(text, "success := refineRoll(sanc)", "roll := w.rollPercent(refineChance(sanc))\n\tsuccess := roll.Success", "Ori/Lac roll")
old = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*powder, *dest = oldPowder, oldDest
		log.Printf("[#%d] ERRO ao salvar refino alvo=%d: %v", s.ID, oldDest.Index, err)
		return
	}
'''.replace(' ', '', 1)
new = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*powder, *dest = oldPowder, oldDest
		log.Printf("[#%d] ERRO ao salvar refino alvo=%d: %v", s.ID, oldDest.Index, err)
		resend()
		s.Send(wire.SendItem(p.ID, byte(destType), byte(destPos), *dest))
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "refinement", err)
		return
	}
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Ori/Lac persistence poison")
old = r''' 	if success {
		s.Send(wire.MessagePanel("Refine successful!"))
	} else {
		s.Send(wire.MessagePanel("The refine failed."))
	}
'''.replace(' ', '', 1)
new = r''' 	s.Send(wire.MessagePanel(roll.message()))
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Ori/Lac player roll message")
text = replace_once(text, 'log.Printf("[#%d] refino code=%d po=%d alvo=%d +%d->%d %s",\n\t\ts.ID, code, oldPowder.Index, oldDest.Index, sanc, sanc+1, result)', 'log.Printf("[#%d] refino code=%d po=%d alvo=%d +%d->%d %s roll=%d/%d",\n\t\ts.ID, code, oldPowder.Index, oldDest.Index, sanc, sanc+1, result, roll.Roll, roll.Chance)', "Ori/Lac log")
if "refineRoll(" in text or "rand.Intn(115)" in text:
    raise SystemExit("consumables.go still contains legacy refinement RNG")
write(path, text)

path = "internal/game/indexed_use_items.go"
text = read(path)
text = replace_once(text, "success := w.intn(100) < rule.SuccessPercent", "roll := w.rollPercent(rule.SuccessPercent)\n\tsuccess := roll.Success", "Adamantite roll")
old = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		resend(target, targetType, targetPos, "")
		log.Printf("[#%d] ERRO ao salvar Adamantita item=%d: %v", s.ID, oldSource.Index, err)
		return
	}
'''.replace(' ', '', 1)
new = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*p.Char = snapshot
		resend(target, targetType, targetPos, "")
		log.Printf("[#%d] ERRO ao salvar Adamantita item=%d: %v", s.ID, oldSource.Index, err)
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "adamantite upgrade", err)
		return
	}
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Adamantite persistence poison")
old = r''' 	if success {
		if targetType == placeEquip {
			s.Send(wire.UpdateScore(p.ID, *p.Char))
			w.syncPlayerVitalsToObservers(p)
			w.refreshAppearance(p)
		}
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 3) })
		s.Send(wire.MessagePanel("The equipment became Legendary!"))
	} else {
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 0) })
		s.Send(wire.MessagePanel("Adamantite failed."))
	}
	log.Printf("[#%d] Adamantita alvo=%d resultado=%d sucesso=%v chance=%d%%",
		s.ID, oldTarget.Index, target.Index, success, rule.SuccessPercent)
'''.replace(' ', '', 1)
new = r''' 	if success {
		if targetType == placeEquip {
			s.Send(wire.UpdateScore(p.ID, *p.Char))
			w.syncPlayerVitalsToObservers(p)
			w.refreshAppearance(p)
		}
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 3) })
	} else {
		w.sendToPlayerView(p, func() []byte { return wire.Motion(p.ID, 14, 0) })
	}
	s.Send(wire.MessagePanel(roll.message()))
	if success {
		s.Send(wire.MessagePanel("The equipment became Legendary!"))
	}
	log.Printf("[#%d] Adamantita alvo=%d resultado=%d sucesso=%v roll=%d/%d",
		s.ID, oldTarget.Index, target.Index, success, roll.Roll, roll.Chance)
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "Adamantite result message/log")
if "w.intn(100) <" in text:
    raise SystemExit("indexed_use_items.go still contains legacy percentage RNG")
write(path, text)

path = "internal/game/mount.go"
text = read(path)
text = replace_once(text, "success := refineRoll(eggProgress(*egg))", "roll := w.rollPercent(refineChance(eggProgress(*egg)))\n\tsuccess := roll.Success", "egg incubation roll")
old = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*egg, *powder = oldEgg, oldPowder
		log.Printf("[#%d] ERRO ao salvar incubacao: %v", s.ID, err)
		return
	}
'''.replace(' ', '', 1)
new = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*egg, *powder = oldEgg, oldPowder
		log.Printf("[#%d] ERRO ao salvar incubacao: %v", s.ID, err)
		resend()
		s.Send(wire.SendItem(p.ID, placeInv, byte(eggSlot), *egg))
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "egg incubation", err)
		return
	}
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "egg persistence poison")
old = r''' 	resend()
	s.Send(wire.SendItem(p.ID, placeInv, byte(eggSlot), *egg))
	switch {
	case hatched:
		s.Send(wire.MessagePanel("The egg hatched! A hatchling was born -- equip it."))
	case success:
		s.Send(wire.MessagePanel(fmt.Sprintf("Incubation advanced (%d/%d).", eggProgress(*egg), threshold+1)))
	default:
		s.Send(wire.MessagePanel("The incubation failed this time."))
	}
	log.Printf("[#%d] incubacao vol=%d ovo->%d progresso=%d/%d chocou=%v",
		s.ID, vol, egg.Index, eggProgress(*egg), threshold+1, hatched)
'''.replace(' ', '', 1)
new = r''' 	resend()
	s.Send(wire.SendItem(p.ID, placeInv, byte(eggSlot), *egg))
	s.Send(wire.MessagePanel(roll.message()))
	switch {
	case hatched:
		s.Send(wire.MessagePanel("The egg hatched! A hatchling was born -- equip it."))
	case success:
		s.Send(wire.MessagePanel(fmt.Sprintf("Incubation advanced (%d/%d).", eggProgress(*egg), threshold+1)))
	}
	log.Printf("[#%d] incubacao vol=%d ovo->%d progresso=%d/%d chocou=%v roll=%d/%d",
		s.ID, vol, egg.Index, eggProgress(*egg), threshold+1, hatched, roll.Roll, roll.Chance)
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "egg result message/log")
old = r''' 	var ok bool
	var msg string
	switch rule.MountAction {
	case "essence":
		ok, msg = mountEssence(mount, item.Index)
	case "feed":
'''.replace(' ', '', 1)
new = r''' 	var ok bool
	var msg string
	var rolls []percentRoll
	switch rule.MountAction {
	case "essence":
		var outcome mountEssenceOutcome
		outcome = w.mountEssence(mount, item.Index)
		ok, msg, rolls = outcome.OK, outcome.Message, outcome.Rolls
	case "feed":
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "mount apply essence outcome")
old = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*mount, *item = old, oldItem
		log.Printf("[#%d] ERRO ao salvar mount %s: %v", s.ID, rule.MountAction, err)
		return
	}
'''.replace(' ', '', 1)
new = r''' 	if err := w.saveAccount(p.Account); err != nil {
		*mount, *item = old, oldItem
		log.Printf("[#%d] ERRO ao salvar mount %s: %v", s.ID, rule.MountAction, err)
		resend()
		s.Send(wire.SendItem(p.ID, placeEquip, byte(mslot), *mount))
		s.Send(wire.MessagePanel("Save failed. Reconnect to reload the authoritative state."))
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "mount item", err)
		return
	}
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "mount persistence poison")
old = r''' 	w.syncCriaPet(p) // cria pode ter virado adulta (some) ou continuar cria
	if msg != "" {
		s.Send(wire.MessagePanel(msg))
	}
'''.replace(' ', '', 1)
new = r''' 	w.syncCriaPet(p) // cria pode ter virado adulta (some) ou continuar cria
	for i, roll := range rolls {
		message := roll.message()
		if i > 0 {
			message = roll.namedMessage("Penalty")
		}
		s.Send(wire.MessagePanel(message))
	}
	if msg != "" {
		s.Send(wire.MessagePanel(msg))
	}
'''.replace(' ', '', 1)
text = replace_once(text, old, new, "mount roll messages")
old = r'''// mountEssence porta o handler do amago (Vol 16): restaura HP a 20000, sobe 1
// level. Em ADULTA rola a chance de sucesso (g_pSancRate[2]); na falha ha chance
// de reduzir 1 level (so level par: 20% <50, 40% 50-99, 70% >=100). Ao cruzar o
// limite de estagio, evolui (sIndex+=30).
func mountEssence(mount *model.Item, essenceIndex uint16) (bool, string) {
	if !essenceMatchesMount(essenceIndex, mount.Index) {
		return false, "This essence does not match the mount."
	}
	mount.SetMountHP(model.MountEssenceHP)
	level := mount.MountLevel()
	if model.IsMountAdult(mount.Index) {
		if level >= model.MountMaxLevel {
			return false, "The mount is already at maximum level."
		}
		if rand.Intn(100) > mountSuccessRate(level) {
			if level%2 == 0 {
				r2 := rand.Intn(100)
				if (level < 50 && r2 < 20) || (level >= 50 && level < 100 && r2 < 40) || (level >= 100 && r2 < 70) {
					mount.SetMountLevel(level - 1)
					return true, "Failure: the mount lost a level."
				}
			}
			return true, "The mount evolution failed."
		}
	}
	level++
	mount.SetMountLevel(level)
	if th := mountStageThreshold(mount.Index); th > 0 && level >= th {
		advanceMountStage(mount, 14)
		return true, "Your mount grew to the next stage!"
	}
	return true, "Your mount gained a level."
}
'''
new = r'''// mountEssenceOutcome carrega as decisoes percentuais que precisam ser
// publicadas somente depois da persistencia. Rolls[0] e o crescimento adulto;
// Rolls[1], quando existe, e a penalidade por falha em level par.
type mountEssenceOutcome struct {
	OK      bool
	Message string
	Rolls   []percentRoll
}

// mountEssence porta o handler do amago (Vol 16): restaura HP a 20000, sobe 1
// level. Em ADULTA rola a chance de sucesso (g_pSancRate[2]); na falha ha chance
// de reduzir 1 level (so level par: 20% <50, 40% 50-99, 70% >=100). Toda chance
// usa 1..100 e sucesso em roll <= chance.
func (w *World) mountEssence(mount *model.Item, essenceIndex uint16) mountEssenceOutcome {
	if !essenceMatchesMount(essenceIndex, mount.Index) {
		return mountEssenceOutcome{Message: "This essence does not match the mount."}
	}
	mount.SetMountHP(model.MountEssenceHP)
	level := mount.MountLevel()
	var rolls []percentRoll
	if model.IsMountAdult(mount.Index) {
		if level >= model.MountMaxLevel {
			return mountEssenceOutcome{Message: "The mount is already at maximum level."}
		}
		growth := w.rollPercent(mountSuccessRate(level))
		rolls = append(rolls, growth)
		if !growth.Success {
			if level%2 == 0 {
				penaltyChance := 70
				if level < 50 {
					penaltyChance = 20
				} else if level < 100 {
					penaltyChance = 40
				}
				penalty := w.rollPercent(penaltyChance)
				rolls = append(rolls, penalty)
				if penalty.Success {
					mount.SetMountLevel(level - 1)
					return mountEssenceOutcome{
						OK: true, Message: "Failure: the mount lost a level.", Rolls: rolls,
					}
				}
			}
			return mountEssenceOutcome{OK: true, Message: "The mount evolution failed.", Rolls: rolls}
		}
	}
	level++
	mount.SetMountLevel(level)
	if th := mountStageThreshold(mount.Index); th > 0 && level >= th {
		advanceMountStage(mount, 14)
		return mountEssenceOutcome{OK: true, Message: "Your mount grew to the next stage!", Rolls: rolls}
	}
	return mountEssenceOutcome{OK: true, Message: "Your mount gained a level.", Rolls: rolls}
}
'''
text = replace_once(text, old, new, "mount essence refactor")
if "rand.Intn(100)" in text:
    raise SystemExit("mount.go still contains raw rand.Intn(100) percentage RNG")
if "refineRoll(" in text:
    raise SystemExit("mount.go still references refineRoll")
write(path, text)

path = "internal/game/celestial_native_progression_test.go"
text = read(path)
text = replace_once(text, "func TestOdinSecretStoneCorrectSequenceUses95PercentAndCharges2M(t *testing.T) {", "func TestOdinSecretStoneCorrectSequenceRoll95SucceedsAndCharges2M(t *testing.T) {", "rename Odin 95 success test")
text = replace_once(text, "func TestOdinSecretStoneCorrectSequenceRoll95FailsAndConsumes(t *testing.T) {", "func TestOdinSecretStoneCorrectSequenceRoll96FailsAndConsumes(t *testing.T) {", "rename Odin 96 failure test")
text = replace_once(text, "func TestOdinRandomSevenRunesUses5PercentAndRandomSecretStone(t *testing.T) {\n\t// Valid runes, but neither one of the four Secret Stone sequences nor Fury.\n\trunes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}\n\t_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 0})", "func TestOdinRandomSevenRunesRoll5SucceedsAndCreatesRandomSecretStone(t *testing.T) {\n\t// Valid runes, but neither one of the four Secret Stone sequences nor Fury.\n\trunes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}\n\t_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, fixedRNG{value: 4})", "Odin 5 boundary")
text = replace_once(text, "func TestOdinRandomSevenRunesRoll5FailsAndConsumes(t *testing.T) {", "func TestOdinRandomSevenRunesRoll6FailsAndConsumes(t *testing.T) {", "rename Odin 6 failure test")
write(path, text)

docs = ROOT / "DOCS" / "CRAFTING.md"
if docs.exists():
    text = docs.read_text(encoding="utf-8")
    marker = "## Percentuais autoritativos"
    if marker not in text:
        text += r'''

## Percentuais autoritativos

Toda decisao percentual de composicao/refinacao usa um unico dominio uniforme
de 1 a 100. O servidor sorteia `roll = 1..100` e considera sucesso somente
quando `roll <= chance`. Assim, `95/95` e `5/5` sao sucesso; `96/95` e `6/5`
sao falha.

Operacoes percentuais economicas publicam ao jogador o mesmo roll usado pela
decisao, somente depois da persistencia confirmar, por exemplo
`Success: 40/50` ou `Fail: 60/50`. Nao existe uma segunda rolagem para exibicao.
Falha de persistencia restaura o snapshot e isola a sessao para que um save
posterior nao sobrescreva um resultado possivelmente confirmado pelo PostgreSQL.

A regra se aplica aos crafts percentuais (Tiny, Compositor, Agatha, Aylin,
Ehre e Odin), refino Ori/Lac, incubacao com Ori/Lac, Adamantita e evolucao
percentual de montaria. Selecoes aleatorias que nao representam porcentagem,
como escolher um entre quatro elementos depois de uma Secret Stone aleatoria
ter sido aprovada, continuam usando o seu dominio natural.
'''
        docs.write_text(text, encoding="utf-8")

checks = {
    "internal/game/crafting.go": ["combineRoll()", "rand.Intn(100)", "rand.Intn(115)"],
    "internal/game/crafting_simple.go": ["combineRoll()", "rand.Intn(100)", "aylinRollSucceeds"],
    "internal/game/crafting_advanced.go": ["rand.Intn(100)", "w.intn(100) <"],
    "internal/game/consumables.go": ["refineRoll(", "rand.Intn(115)"],
    "internal/game/indexed_use_items.go": ["w.intn(100) <"],
    "internal/game/mount.go": ["rand.Intn(100)", "refineRoll("],
}
for rel, needles in checks.items():
    body = read(rel)
    for needle in needles:
        if needle in body:
            raise SystemExit(f"{rel}: legacy percentage pattern remains: {needle}")

print("percentage refactor applied")
