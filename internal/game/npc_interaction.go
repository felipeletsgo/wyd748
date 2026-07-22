package game

import (
	"errors"
	"fmt"
)

// npcInteractionRange e o alcance maximo (Chebyshev) para interagir com um NPC.
//
// O valor vem de CAPTURA, nao de analogia: o client 7.48 envia o clique a pelo
// menos 6 tiles (visto em log recusando 5 e 6 e aceitando 1..4). Um raio de 4
// -- que era o palpite inicial, copiado do pickupRange -- recusava clique
// legitimo e o jogador via "nada acontece". 8 acomoda o observado com folga e
// continua MUITO abaixo da area de interesse (+-32), que era o buraco real:
// antes desta funcao nenhuma interacao validava distancia, nem a loja, entao
// dava para abrir loja de qualquer canto da tela.
const npcInteractionRange = 8

var (
	errNPCNotFound   = errors.New("npc inexistente")
	errNPCNotVisible = errors.New("npc fora da visao")
	errNPCTooFar     = errors.New("npc fora de alcance")
)

// resolveNPCInteraction e o ponto UNICO de validacao de um clique em NPC,
// usado por loja, mestre, cargo e quest. Valida os tres requisitos na ordem
// mais barata primeiro: existencia, visibilidade e distancia.
//
// Visibilidade importa alem da distancia: o cliente so pode agir sobre uma
// entidade que o servidor materializou para ele. Sem isso um cliente adulterado
// interagiria com NPC que nunca entrou na area de interesse.
func (w *World) resolveNPCInteraction(p *Player, npcID uint16) (*Mob, error) {
	if p == nil || p.Char == nil {
		return nil, errNPCNotFound
	}
	m := w.mobByID(npcID)
	if m == nil || m.Def == nil {
		return nil, errNPCNotFound
	}
	if !p.hasVisible(m.ID) {
		return nil, errNPCNotVisible
	}
	if distance := chebyshev(p.X, p.Y, m.X, m.Y); distance > npcInteractionRange {
		return nil, fmt.Errorf("%w (%d tiles)", errNPCTooFar, distance)
	}
	return m, nil
}

// npcInteractionMessage traduz a recusa em texto para o painel do cliente.
func npcInteractionMessage(err error) string {
	switch {
	case errors.Is(err, errNPCTooFar):
		return "Aproxime-se para interagir."
	case errors.Is(err, errNPCNotVisible):
		return "Esse personagem nao esta visivel."
	default:
		return "Esse personagem nao esta disponivel."
	}
}
