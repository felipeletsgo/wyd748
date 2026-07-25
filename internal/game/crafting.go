package game

import (
	"fmt"
	"log"
	"math/rand"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	combinePacketSize = 84
	combineSlots      = 8
	tinyGoldCost      = 100_000_000
	// RELEASE SERVER/Common/Settings/CompRate.txt do W2PP usa 100. O default
	// compilado era 20, mas sempre era sobrescrito pelo arquivo no boot.
	tinyBaseChance = 100
)

type combineRequest struct {
	Items [combineSlots]model.Item
	Pos   [combineSlots]int8
}

// parseCombineRequest valida o snapshot enviado pelo client contra o inventario
// autoritativo. O pacote nunca escolhe o item que sera consumido: ele apenas
// referencia uma posicao cujo STRUCT_ITEM de oito bytes precisa ser identico.
func parseCombineRequest(pkt []byte, ch *model.Char) (combineRequest, error) {
	var req combineRequest
	if len(pkt) != combinePacketSize || ch == nil {
		return req, fmt.Errorf("pacote de composicao com %d bytes", len(pkt))
	}
	used := make(map[int]struct{}, combineSlots)
	for i := 0; i < combineSlots; i++ {
		req.Items[i] = decodeTradeItem(pkt[12+i*8 : 20+i*8])
		req.Pos[i] = int8(pkt[76+i])
		if req.Items[i].Index == 0 {
			continue
		}
		pos := int(req.Pos[i])
		if pos < 0 || pos >= model.PlayerCarrySlots {
			return combineRequest{}, fmt.Errorf("ingrediente %d aponta slot %d", i, pos)
		}
		if _, duplicate := used[pos]; duplicate {
			return combineRequest{}, fmt.Errorf("slot %d repetido", pos)
		}
		used[pos] = struct{}{}
		if ch.Inv[pos] != req.Items[i] {
			return combineRequest{}, fmt.Errorf("ingrediente %d diverge do inventario", i)
		}
	}
	return req, nil
}

func (w *World) combineNPC(p *Player, expectedName string) (*Mob, error) {
	if p == nil || p.Char == nil {
		return nil, fmt.Errorf("jogador sem contexto de composicao")
	}
	// Algumas janelas do 7.48 sao abertas inteiramente no client e, nesse fluxo,
	// nenhum 0x28B e enviado. CraftNPC pode portanto estar vazio ou ainda apontar
	// para o artesao anterior. Reaproveitamos o contexto somente se ele continuar
	// valido; caso contrario resolvemos o artesao esperado no grid local.
	if p.CraftNPC != 0 {
		if m, err := w.resolveNPCInteraction(p, p.CraftNPC); err == nil && craftingNPCMatches(m, expectedName) {
			return m, nil
		}
	}

	var selected *Mob
	bestDistance := npcInteractionRange + 1
	for _, m := range w.nearbyMobs(p.X, p.Y, npcInteractionRange) {
		if !p.hasVisible(m.ID) || !craftingNPCMatches(m, expectedName) {
			continue
		}
		distance := chebyshev(p.X, p.Y, m.X, m.Y)
		if distance < bestDistance {
			selected, bestDistance = m, distance
		}
	}
	if selected == nil {
		return nil, fmt.Errorf("compositor %s nao esta visivel ou ao alcance", expectedName)
	}
	p.CraftNPC = selected.ID
	return selected, nil
}

func craftingNPCMatches(m *Mob, expectedName string) bool {
	return m != nil && !m.Dead && m.Def != nil && m.Def.Extended != nil &&
		m.Def.Extended.Merchant&0x0F == craftingMerchant &&
		strings.EqualFold(generName(m.Def.Name), expectedName)
}

func tinyBlockedWeapon(index uint16) bool {
	ranges := [][2]uint16{
		{2511, 2542}, {2571, 2602}, {2811, 2842}, {2851, 2854},
		{2631, 2662}, {2691, 2722}, {2871, 2886}, {2471, 2482},
		{2911, 2926}, {2751, 2782},
	}
	for _, r := range ranges {
		if index >= r[0] && index <= r[1] {
			return true
		}
	}
	return false
}

func (w *World) tinyRecipe(req combineRequest) (model.Item, int, error) {
	for _, item := range req.Items {
		if item.Index == 747 {
			return model.Item{}, 0, fmt.Errorf("item 747 nao participa desta composicao")
		}
	}
	a, b, catalyst := req.Items[0], req.Items[1], req.Items[2]
	defA, okA := w.items[a.Index]
	defB, okB := w.items[b.Index]
	defC, okC := w.items[catalyst.Index]
	if a.Index == 0 || b.Index == 0 || catalyst.Index == 0 || !okA || !okB || !okC {
		return model.Item{}, 0, fmt.Errorf("tres ingredientes principais sao obrigatorios")
	}
	if itemAbility(a, defA, "EF_MOBTYPE") != 1 || defA.Grade < 5 || defA.Grade > 8 {
		return model.Item{}, 0, fmt.Errorf("item principal nao e arma Arch grade 5..8")
	}
	if tinyBlockedWeapon(b.Index) || defB.Grade < 5 || defB.Grade > 8 {
		return model.Item{}, 0, fmt.Errorf("arma doadora invalida")
	}
	// O nativo rejeita BASE_GetItemSanc > 237: no encoding 230+ isso comeca
	// em +12. A doadora pode estar em 0..+11, mas nunca +12..+15.
	if itemSanc(b) >= 12 {
		return model.Item{}, 0, fmt.Errorf("arma doadora acima de +11")
	}
	if itemAbility(catalyst, defC, "EF_ITEMTYPE") == 1 {
		return model.Item{}, 0, fmt.Errorf("catalisador de tipo proibido")
	}
	if defA.Pos != defB.Pos || defA.Pos != 64 && defA.Pos != 192 {
		return model.Item{}, 0, fmt.Errorf("armas precisam ocupar a mesma mao")
	}
	if itemSanc(a) < 9 || itemSanc(catalyst) < 9 {
		return model.Item{}, 0, fmt.Errorf("item principal e catalisador exigem refino +9")
	}
	result := a
	result.Eff = b.Eff
	if !setItemSanc(&result, 7) {
		return model.Item{}, 0, fmt.Errorf("efeitos da arma doadora nao comportam refino")
	}
	chance := tinyBaseChance + 5*itemAbility(b, defB, "EF_ITEMLEVEL")
	return result, clampInt(chance, 0, 100), nil
}

func combineRoll() int {
	roll := rand.Intn(115)
	if roll >= 100 {
		roll -= 15
	}
	return roll
}

func (w *World) sendCombineResult(p *Player, result uint32) {
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

func combineResultMessage(result uint32) string {
	switch result {
	case 0:
		return "Wrong combination."
	case 1:
		return "Processing complete."
	case 2:
		return "The combination failed."
	default:
		return ""
	}
}

func (w *World) onCombineTiny(s *net.Session, pkt []byte) {
	p, req, ok := w.beginCombine(s, pkt, "Tiny")
	if !ok {
		return
	}
	resultItem, chance, err := w.tinyRecipe(req)
	if err != nil || p.Char.Gold < tinyGoldCost {
		w.sendCombineResult(p, 0)
		return
	}

	w.cancelTrade(p, "composicao Tiny")
	oldInv, oldEquip, oldGold := p.Char.Inv, p.Char.Equip, p.Char.Gold
	success := combineRoll() <= chance
	changed := make(map[int]struct{}, combineSlots)
	consumeCombineItems(p.Char, req, 2, combineSlots-1, changed)
	if success {
		mainPos, donorPos := int(req.Pos[0]), int(req.Pos[1])
		p.Char.Inv[mainPos] = resultItem
		p.Char.Inv[donorPos] = model.Item{}
		changed[mainPos], changed[donorPos] = struct{}{}, struct{}{}
	}
	p.Char.Gold -= tinyGoldCost
	code := uint32(2)
	if success {
		code = 1
	}
	if w.commitCombine(p, oldInv, oldEquip, oldGold, changed, nil, code) {
		logCraftResult(s, success, chance, p.Char.Gold)
	}
}

func logCraftResult(s *net.Session, success bool, chance int, gold uint32) {
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
