package game

import (
	"log"
	"time"

	"wydgo/internal/model"
)

// charStateStore e a persistencia opcional do estado de sessao (buffs, moedas).
// Segue o padrao do guildStore: o game conhece so a interface minima e faz
// assercao, sem obrigar todo Store a implementa-la.
type charStateStore interface {
	LoadCharState(name string) (*model.CharState, error)
	SaveCharState(name string, state *model.CharState) error
}

// buildCharState coleta os affects AINDA ativos (tempo absoluto) e as moedas do
// jogador para persistir. Affects expirados sao descartados aqui.
func buildCharState(p *Player, now time.Time) *model.CharState {
	state := &model.CharState{Version: model.CharStateVersion}
	if p.Char != nil {
		for _, a := range p.Char.Affects {
			if a.Type == 0 || !a.ExpiresAt.After(now) {
				continue
			}
			state.Affects = append(state.Affects, model.PersistedAffect{
				Type: a.Type, ClientType: a.ClientType, Value: a.Value, Level: a.Level,
				OwnerID: a.OwnerID, ExpiresUnix: a.ExpiresAt.Unix(),
			})
		}
	}
	for name, value := range p.SpecialCoins {
		if value == 0 {
			continue
		}
		if state.SpecialCoins == nil {
			state.SpecialCoins = make(map[string]uint32, len(p.SpecialCoins))
		}
		state.SpecialCoins[name] = value
	}
	return state
}

// applyCharState recompoe no personagem os affects que ainda nao expiraram e
// restaura as moedas. O tempo restante e recalculado da expiracao absoluta.
func (w *World) applyCharState(p *Player, state *model.CharState, now time.Time) {
	if p == nil || p.Char == nil || state == nil {
		return
	}
	slot := 0
	for _, pa := range state.Affects {
		if pa.Type == 0 || pa.ExpiresUnix <= now.Unix() {
			continue
		}
		for slot < len(p.Char.Affects) && p.Char.Affects[slot].Type != 0 {
			slot++
		}
		if slot >= len(p.Char.Affects) {
			break
		}
		clientType := pa.ClientType
		if clientType == 0 {
			clientType = pa.Type
		}
		p.Char.Affects[slot] = model.Affect{
			Type: pa.Type, ClientType: clientType, Value: pa.Value, Level: pa.Level,
			OwnerID: pa.OwnerID, ExpiresAt: time.Unix(pa.ExpiresUnix, 0),
			NextTick: now.Add(8 * time.Second),
		}
		slot++
	}
	// SUBSTITUI, nunca mescla: o Player e reusado no vaivem da selecao, entao
	// mesclar deixaria as moedas do personagem anterior no atual -- e o autosave
	// as gravaria no charstate dele, duplicando-as.
	p.SpecialCoins = nil
	if len(state.SpecialCoins) > 0 {
		p.SpecialCoins = make(map[string]uint32, len(state.SpecialCoins))
		for name, value := range state.SpecialCoins {
			p.SpecialCoins[name] = value
		}
	}
}

// loadCharStateInto restaura buffs e moedas ao entrar no mundo. Chamado ANTES de
// publicar score/affects para que os buffs restaurados ja apareçam no client.
func (w *World) loadCharStateInto(p *Player) {
	store, ok := w.store.(charStateStore)
	if !ok || p == nil || p.Char == nil {
		return
	}
	state, err := store.LoadCharState(p.Char.Name)
	if err != nil {
		log.Printf("[#%d] ERRO ao carregar charstate de %q: %v", p.Session.ID, p.Char.Name, err)
		return
	}
	if state == nil {
		return
	}
	w.applyCharState(p, state, time.Now())
}

// saveCharState persiste buffs e moedas do jogador de forma SINCRONA. Chamado no
// logout e no disconnect (estado que precisa estar em disco antes de largar o
// player). Estado vazio remove o arquivo.
func (w *World) saveCharState(p *Player) {
	if err := w.saveCharStateResult(p); err != nil {
		log.Printf("[#%d] ERRO ao salvar charstate de %q: %v", p.Session.ID, p.Char.Name, err)
	}
}

// saveCharStateResult e a versao que DEVOLVE o erro, para quem precisa desfazer
// o que fez. Usada pelas quests que mexem em contador: o sidecar nao participa
// da transacao da conta, entao quem o grava tem de tratar a falha.
func (w *World) saveCharStateResult(p *Player) error {
	store, ok := w.store.(charStateStore)
	if !ok || p == nil || p.Char == nil {
		return nil
	}
	return store.SaveCharState(p.Char.Name, buildCharState(p, time.Now()))
}

// asyncCharStateStore expoe o save async do charstate (autosave).
type asyncCharStateStore interface {
	SaveCharStateAsync(name string, state *model.CharState) error
}

// saveCharStateAsync persiste buffs/moedas FORA do game-loop (autosave). Store
// sem suporte cai no sincrono.
func (w *World) saveCharStateAsync(p *Player) {
	if p == nil || p.Char == nil {
		return
	}
	state := buildCharState(p, time.Now())
	if as, ok := w.store.(asyncCharStateStore); ok {
		if err := as.SaveCharStateAsync(p.Char.Name, state); err != nil {
			log.Printf("[#%d] ERRO no autosave de charstate %q: %v", p.Session.ID, p.Char.Name, err)
		}
		return
	}
	w.saveCharState(p)
}
