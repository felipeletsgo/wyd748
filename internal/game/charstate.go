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
	LoadCharState(characterUID string) (*model.CharState, error)
	SaveCharState(characterUID string, state *model.CharState) error
}

type playerStateStore interface {
	SavePlayerState(guilds *model.GuildRegistry, account *model.Account,
		characterUID string, state *model.CharState) error
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
				OwnerID: a.OwnerID, OwnerCharacterUID: a.OwnerCharacterUID,
				ExpiresUnix: a.ExpiresAt.Unix(),
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
		// A player-owned affect without a stable character identity belongs to
		// an older sidecar format. Restoring it by recycled OwnerID could bind a
		// debuff to another character, so discard it rather than resurrecting a
		// stale effect. OwnerID-only affects with no source are still valid
		// server buffs (OwnerID=0) and remain loadable.
		if pa.OwnerID != 0 && pa.OwnerCharacterUID == "" {
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
			OwnerID: pa.OwnerID, OwnerCharacterUID: pa.OwnerCharacterUID,
			ExpiresAt: time.Unix(pa.ExpiresUnix, 0),
			NextTick:  now.Add(8 * time.Second),
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
func (w *World) loadCharStateInto(p *Player) error {
	if p == nil || p.Char == nil {
		return nil
	}
	var state *model.CharState
	if stateStore, ok := w.store.(charStateStore); ok {
		loaded, err := stateStore.LoadCharState(p.Char.UID)
		if err != nil {
			return err
		}
		state = loaded
	}
	// Limpe somente DEPOIS de uma leitura bem-sucedida. Limpar antes e seguir
	// com erro permitia entrar sem buffs/contadores autoritativos e o autosave
	// consolidava essa perda no banco.
	p.Char.Affects = [16]model.Affect{}
	p.SpecialCoins = nil
	if state != nil {
		w.applyCharState(p, state, w.now())
	}
	return nil
}

// saveCharState persiste buffs e moedas do jogador de forma SINCRONA. Chamado no
// logout e no disconnect (estado que precisa estar em disco antes de largar o
// player). Estado vazio remove o arquivo.
func (w *World) saveCharState(p *Player) {
	if err := w.saveCharStateResult(p); err != nil {
		log.Printf("[#%d] ERRO ao salvar charstate de %q: %v", p.Session.ID, p.Char.Name, err)
	}
}

// saveCharStateResult e a versao isolada que DEVOLVE o erro. Operacoes que
// alteram conta e charstate juntas devem usar saveAccountAndCharStateResult,
// que aproveita a transacao unica oferecida pelo PostgreSQL.
func (w *World) saveCharStateResult(p *Player) error {
	store, ok := w.store.(charStateStore)
	if !ok || p == nil || p.Char == nil {
		return nil
	}
	return store.SaveCharState(p.Char.UID, buildCharState(p, w.now()))
}

// saveAccountAndCharStateResult usa uma unica transacao quando o store suporta.
// O fallback preserva a ordem segura do adaptador JSON de desenvolvimento.
func (w *World) saveAccountAndCharStateResult(p *Player) error {
	if p == nil || p.Account == nil || p.Char == nil {
		return nil
	}
	state := buildCharState(p, w.now())
	return w.saveAccountAndCharStateForUID(p.Account, p.Char.UID, state)
}

// saveAccountAndCharStateForUID é a variante usada quando uma operação troca
// ou remove a ficha apontada por Player.Char (por exemplo, encapsulamento e
// extração Celestial). O UID fica explícito mesmo que o slot já tenha sido
// zerado; PostgreSQL consegue excluir o charstate na mesma transação.
func (w *World) saveAccountAndCharStateForUID(account *model.Account, uid string, state *model.CharState) error {
	if account == nil {
		return nil
	}
	snapshot := accountPersistenceSnapshot(account)
	if transactional, ok := w.store.(playerStateStore); ok {
		return transactional.SavePlayerState(nil, snapshot, uid, state)
	}
	if stateStore, ok := w.store.(charStateStore); ok && uid != "" {
		if err := stateStore.SaveCharState(uid, state); err != nil {
			return err
		}
	}
	return w.store.SaveAccount(snapshot)
}

// asyncCharStateStore expoe o save async do charstate (autosave).
type asyncCharStateStore interface {
	SaveCharStateAsync(characterUID string, state *model.CharState) error
}

// saveCharStateAsync persiste buffs/moedas FORA do game-loop (autosave). Store
// sem suporte cai no sincrono.
func (w *World) saveCharStateAsync(p *Player) {
	if p == nil || p.Char == nil {
		return
	}
	state := buildCharState(p, w.now())
	if as, ok := w.store.(asyncCharStateStore); ok {
		if err := as.SaveCharStateAsync(p.Char.UID, state); err != nil {
			log.Printf("[#%d] ERRO no autosave de charstate %q: %v", p.Session.ID, p.Char.Name, err)
		}
		return
	}
	w.saveCharState(p)
}
