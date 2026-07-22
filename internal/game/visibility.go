package game

import "wydgo/internal/wire"

// O grid nativo W2PP usa meia janela 16. Este emulador amplia para 32 para que
// PvP e mapas de guerra conservem entidades ate o limite visual da camera. A
// janela continua local (65x65), sem transmitir o mundo inteiro ao client.
const viewHalfX = 32
const viewHalfY = 32

func inView(ax, ay, bx, by uint16) bool {
	return absDiff(ax, bx) <= viewHalfX && absDiff(ay, by) <= viewHalfY
}

func (p *Player) hasVisible(id uint16) bool {
	if p == nil || p.Visible == nil {
		return false
	}
	_, ok := p.Visible[id]
	return ok
}

func (p *Player) show(id uint16) {
	if p.Visible == nil {
		p.Visible = make(map[uint16]struct{})
	}
	p.Visible[id] = struct{}{}
}

func (p *Player) hide(id uint16) {
	delete(p.Visible, id)
}

func (w *World) showMob(p *Player, m *Mob) {
	if p == nil || !p.InWorld || m == nil || m.Dead || p.hasVisible(m.ID) {
		return
	}
	anct := m.Def.Equip.AncientCodes()
	p.Session.Send(wire.CreateMobVisualExtended(m.ID, m.Def.Name, m.X, m.Y,
		m.Def.Mesh(), anct[:], mobPublicExtended(m), m.Affects[:], 0))
	p.show(m.ID)
}

func (w *World) hideMob(p *Player, m *Mob, removeType uint32) {
	if p == nil || m == nil || !p.hasVisible(m.ID) {
		return
	}
	p.Session.Send(wire.RemoveMob(m.ID, removeType))
	p.hide(m.ID)
}

func (w *World) showGhostShop(p *Player, shop *GhostShop) {
	if p == nil || !p.InWorld || shop == nil || p.hasVisible(shop.ID) {
		return
	}
	p.Session.Send(wire.CreateMobTradeExtended(shop.ID, shop.Name, shop.X, shop.Y,
		shop.Mesh[:], &shop.Extended, shop.Title))
	p.show(shop.ID)
}

func (w *World) hideGhostShop(p *Player, shop *GhostShop) {
	if p == nil || shop == nil || !p.hasVisible(shop.ID) {
		return
	}
	p.Session.Send(wire.RemoveMob(shop.ID, 0))
	p.hide(shop.ID)
}

// playerEnterViewPackets monta a sequencia necessaria para materializar um
// jogador no client 7.48. CreateMob sozinho nao limpa de forma confiavel o
// estado ECMOTION_DEAD mantido para um ID que morreu anteriormente: HP/MP
// confirma que a entidade esta viva e ActionStop fixa a pose e a coordenada
// inicial antes de receber o proximo trajeto de movimento.
func playerEnterViewPackets(subject *Player) [][]byte {
	if subject == nil || subject.Char == nil {
		return nil
	}
	return [][]byte{
		wire.CreateMobExtended(subject.ID, subject.Char.Name, subject.X, subject.Y,
			bodyMesh(subject.Char), bodyAncient(subject.Char), wireExtendedScore(subject.Char), subject.Char.Affects[:], 2, subject.Char.GuildID),
		wire.SetHpMpExtended(subject.ID, wireExtendedScore(subject.Char)),
		wire.ActionStop(subject.ID, subject.X, subject.Y),
	}
}

func sendPlayerEnterView(observer, subject *Player) {
	if observer == nil || observer.Session == nil {
		return
	}
	for _, pkt := range playerEnterViewPackets(subject) {
		observer.Session.Send(pkt)
	}
}

// republishPlayerAppearance reenvia o CreateMob do jogador aos observadores que
// ja o enxergam. O visual do equip (refino, cor, mesh) viaja SO no CreateMob;
// nenhum pacote de update o propaga sozinho, entao uma mudanca como o refino
// ficava visivel apenas para o dono ate o observador cruzar a borda da visao.
// Reconstroi os pacotes por observador (Send cifra in-place).
// refreshAppearance forca a atualizacao visual do avatar SEM troca de equip
// (transformacao de rosto, cor da tintura, refino). Reenvia APENAS o CreateMob
// (que carrega mesh + anct/cor) ao dono e aos observadores, SEM SetHpMp/ActionStop:
// o ActionStop forcava a posicao rastreada (que durante uma caminhada e o INICIO
// do segmento), fazendo o avatar teleportar de volta. O CreateMob sozinho atualiza
// a aparencia sem re-posicionar. O SelfEquip (0x36B) nao carrega o anct, por isso
// a cor precisa vir por aqui.
func (w *World) refreshAppearance(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil {
		return
	}
	mesh, anct := bodyMesh(subject.Char), bodyAncient(subject.Char)
	ext := wireExtendedScore(subject.Char)
	affects, guild := subject.Char.Affects[:], subject.Char.GuildID
	if subject.Session != nil {
		subject.Session.Send(wire.CreateMobExtended(subject.ID, subject.Char.Name,
			subject.X, subject.Y, mesh, anct, ext, affects, 2, guild))
	}
	for _, observer := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if observer == subject || observer.Session == nil ||
			!observer.hasVisible(subject.ID) {
			continue
		}
		observer.Session.Send(wire.CreateMobExtended(subject.ID, subject.Char.Name,
			subject.X, subject.Y, mesh, anct, ext, affects, 0, guild))
	}
}

func (w *World) republishPlayerAppearance(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil {
		return
	}
	for _, observer := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if observer == subject || observer.Session == nil ||
			!observer.hasVisible(subject.ID) {
			continue
		}
		for _, pkt := range playerEnterViewPackets(subject) {
			observer.Session.Send(pkt)
		}
	}
}

func (w *World) showPlayerPair(a, b *Player) {
	if a == nil || b == nil || a == b || !a.InWorld || !b.InWorld ||
		a.Char == nil || b.Char == nil {
		return
	}
	if !a.hasVisible(b.ID) {
		sendPlayerEnterView(a, b)
		a.show(b.ID)
	}
	if !b.hasVisible(a.ID) {
		sendPlayerEnterView(b, a)
		b.show(a.ID)
	}
}

func (w *World) hidePlayerPair(a, b *Player) {
	if a == nil || b == nil || a == b {
		return
	}
	if a.hasVisible(b.ID) {
		a.Session.Send(wire.RemoveMob(b.ID, 0))
		a.hide(b.ID)
	}
	if b.hasVisible(a.ID) {
		b.Session.Send(wire.RemoveMob(a.ID, 0))
		b.hide(a.ID)
	}
}

// rematerializePlayerAfterRevive remove a representacao morta dos outros
// clients antes de aplicar a nova posicao/estado vivo. RemoveType 0 nao apaga
// um TMHuman que ja esta em ECMOTION_DEAD no client 7.48; o tipo 3 executa a
// exclusao imediata. refreshPlayerVisibility recria o personagem somente para
// observadores que continuam dentro da nova janela.
func (w *World) rematerializePlayerAfterRevive(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil || playerCurHP(subject.Char) == 0 {
		return
	}
	for _, observer := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if observer == subject || !observer.hasVisible(subject.ID) {
			continue
		}
		observer.Session.Send(wire.RemoveMob(subject.ID, 3))
		observer.hide(subject.ID)
	}
	w.refreshPlayerVisibility(subject)
	w.syncPlayerVitals(subject)
	w.sendToPlayerView(subject, func() []byte {
		return wire.ActionStop(subject.ID, subject.X, subject.Y)
	})
	w.updatePartyMember(subject)
}

// refreshPlayerVisibility aplica os deltas quando o jogador cruza a borda de
// uma janela: CreateMob ao entrar e RemoveMob tipo 0 ao sair.
func (w *World) refreshPlayerVisibility(p *Player) {
	if p == nil || !p.InWorld {
		return
	}
	w.updatePlayerSpatial(p)
	nearMobs := w.nearbyMobs(p.X, p.Y, viewHalfX)
	nearMobIDs := make(map[uint16]struct{}, len(nearMobs))
	for _, m := range nearMobs {
		nearMobIDs[m.ID] = struct{}{}
		if !p.hasVisible(m.ID) {
			w.showMob(p, m)
		}
	}
	// Visible contem tipos diferentes de entidade. So remova IDs que o indice
	// canonico confirma serem mobs e que deixaram a janela espacial.
	for id := range p.Visible {
		m := w.mobsByID[id]
		if m == nil {
			continue
		}
		if _, nearby := nearMobIDs[id]; !nearby {
			w.hideMob(p, m, 0)
		}
	}
	for _, shop := range w.ghostShops {
		visible := inView(p.X, p.Y, shop.X, shop.Y)
		switch {
		case visible && !p.hasVisible(shop.ID):
			w.showGhostShop(p, shop)
		case !visible && p.hasVisible(shop.ID):
			w.hideGhostShop(p, shop)
		}
	}
	nearPlayers := w.nearbyWorldPlayers(p.X, p.Y, viewHalfX)
	nearPlayerIDs := make(map[uint16]struct{}, len(nearPlayers))
	for _, other := range nearPlayers {
		if other == p {
			continue
		}
		nearPlayerIDs[other.ID] = struct{}{}
		w.showPlayerPair(p, other)
	}
	for id := range p.Visible {
		other := w.playersByID[id]
		if other == nil || other == p {
			continue
		}
		if _, nearby := nearPlayerIDs[id]; !nearby {
			w.hidePlayerPair(p, other)
		}
	}
	for _, g := range w.groundItems {
		visible := inView(p.X, p.Y, g.X, g.Y)
		switch {
		case visible && !p.hasVisible(g.ID):
			p.Session.Send(wire.CreateItem(g.X, g.Y, g.ID, g.Item, 0, 0, 0, 0, 0))
			p.show(g.ID)
		case !visible && p.hasVisible(g.ID):
			p.Session.Send(wire.RemoveItem(uint32(g.ID)))
			p.hide(g.ID)
		}
	}
}

func (w *World) publishGhostShopSpawn(shop *GhostShop) {
	if shop == nil {
		return
	}
	for _, p := range w.players {
		if p.InWorld && inView(p.X, p.Y, shop.X, shop.Y) {
			w.showGhostShop(p, shop)
		}
	}
}

func (w *World) publishGhostShopRemove(shop *GhostShop) {
	if shop == nil {
		return
	}
	for _, p := range w.players {
		if p.InWorld && p.hasVisible(shop.ID) {
			p.Session.Send(wire.RemoveMob(shop.ID, 0))
			p.hide(shop.ID)
		}
	}
}

func (w *World) publishGhostShopItemSold(shop *GhostShop, pos uint32) {
	if shop == nil {
		return
	}
	for _, p := range w.players {
		if p.InWorld && p.hasVisible(shop.ID) {
			// A janela de auto-loja foi aberta para o ID virtual do clone.
			p.Session.Send(wire.ItemSold(shop.ID, pos))
		}
	}
}

// publishMobSpawn materializa uma nova instancia somente para quem esta perto.
func (w *World) publishMobSpawn(m *Mob) {
	w.registerMobSpatial(m)
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		w.showMob(p, m)
	}
}

// publishMobMove envia movimento apenas a quem ja via o mob e atualiza os
// clientes que passaram a entrar/sair da janela por causa do proprio mob.
func (w *World) publishMobMove(m *Mob, oldX, oldY uint16, speed uint32) {
	w.moveMobSpatial(m, oldX, oldY)
	observers := make(map[uint16]*Player)
	for _, p := range w.nearbyWorldPlayers(oldX, oldY, viewHalfX) {
		observers[p.ID] = p
	}
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		observers[p.ID] = p
	}
	for _, p := range observers {
		wasVisible := p.hasVisible(m.ID)
		nowVisible := inView(p.X, p.Y, m.X, m.Y)
		switch {
		case wasVisible && nowVisible:
			p.Session.Send(wire.MobMove(m.ID, oldX, oldY, m.X, m.Y, speed))
		case wasVisible && !nowVisible:
			w.hideMob(p, m, 0)
		case !wasVisible && nowVisible:
			w.showMob(p, m)
		}
	}
}

// publishPlayerMove replica somente uma mudanca real de destino. O client 7.48
// reenvi​a Action durante o mesmo deslocamento; ecoar cada pacote com PosXY
// intermediario reinicia o relogio de caminhada do observador e parece um
// teleporte. PosXY externo e derivado do ultimo destino server-side anunciado.
func (w *World) publishPlayerMove(player *Player, previousX, previousY uint16) {
	if player == nil || !player.InWorld || player.Char == nil {
		return
	}
	if player.MovePublished && player.MovePublishedTargetX == player.X && player.MovePublishedTargetY == player.Y {
		return
	}
	fromX, fromY := previousX, previousY
	if player.MovePublished {
		fromX, fromY = player.MovePublishedTargetX, player.MovePublishedTargetY
	}
	if fromX == player.X && fromY == player.Y {
		return
	}
	// BASE_GetSpeed: nibble baixo de AttackRun, limitado a 1..6. Usar o score
	// impede speed hack e conserva visualmente botas/buffs de corrida.
	speed := uint32(playerAttackRun(player.Char) & 0x0F)
	for _, observer := range w.nearbyWorldPlayers(player.X, player.Y, viewHalfX) {
		if observer != player && observer.hasVisible(player.ID) {
			observer.Session.Send(wire.PlayerMove(player.ID, fromX, fromY, player.X, player.Y, speed))
		}
	}
	player.MovePublished = true
	player.MovePublishedTargetX, player.MovePublishedTargetY = player.X, player.Y
}

func (w *World) publishPlayerStop(player *Player) {
	if player == nil || !player.InWorld {
		return
	}
	for _, observer := range w.nearbyWorldPlayers(player.X, player.Y, viewHalfX) {
		if observer != player && observer.hasVisible(player.ID) {
			observer.Session.Send(wire.ActionStop(player.ID, player.X, player.Y))
		}
	}
	player.MovePublished = false
}

func (w *World) publishMobDeath(m *Mob, killerID uint16, killerExp uint32, expByPlayer map[*Player]uint32) {
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if !p.hasVisible(m.ID) {
			continue
		}
		exp := killerExp
		if personal, ok := expByPlayer[p]; ok {
			exp = personal
		}
		p.Session.Send(wire.CNFMobKill(m.ID, killerID, exp))
		p.Session.Send(wire.RemoveMob(m.ID, 1))
		p.hide(m.ID)
	}
}

func (w *World) sendToMobView(m *Mob, build func() []byte) {
	if m == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if p.hasVisible(m.ID) {
			p.Session.Send(build())
		}
	}
}

func (w *World) sendToPlayerView(subject *Player, build func() []byte) {
	if subject == nil || !subject.InWorld {
		return
	}
	for _, p := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if p == subject || p.hasVisible(subject.ID) {
			p.Session.Send(build())
		}
	}
}

// publishPlayerAffects alimenta os dois canais distintos do client 7.48:
// 0x336 e publico e aciona TMHuman::CheckAffect; 0x3B9 e privado e atualiza
// somente os icones/timers do personagem controlado pela sessao.
func (w *World) publishPlayerAffects(subject *Player) {
	if subject == nil || subject.Char == nil || subject.Session == nil {
		return
	}
	w.sendToPlayerView(subject, func() []byte {
		return wire.UpdateScore(subject.ID, *subject.Char)
	})
	subject.Session.Send(wire.UpdateAffects(subject.ID, *subject.Char))
}

// syncPlayerVitals mantem HP/MP do personagem identicos no proprio client e em
// todos os observadores. Alem das barras, SetHpMp com HP>0 faz o client 7.48
// limpar ECMOTION_DEAD/m_cDie, sendo obrigatorio no fluxo de renascimento.
func (w *World) syncPlayerVitals(subject *Player) {
	if subject == nil || subject.Char == nil {
		return
	}
	w.sendToPlayerView(subject, func() []byte {
		return wire.SetHpMpExtended(subject.ID, wireExtendedScore(subject.Char))
	})
}

// syncPlayerScoreAndVitals encerra uma alteracao de estado que tambem passou
// pelo fluxo de ataque. O 0x181 chama diretamente o handler legado do client e
// redesenha por um frame os WORDs de HP/MP; em casts normais ele nao e preciso.
// O 0x336 estendido atualiza o sidecar uint32 sem esse flicker. Fluxos que
// realmente precisam da barra/pose (dano, cura, morte e revive) usam
// syncPlayerVitals explicitamente.
func (w *World) syncPlayerScoreAndVitals(subject *Player) {
	if subject == nil || subject.Char == nil {
		return
	}
	w.sendToPlayerView(subject, func() []byte {
		return wire.UpdateScore(subject.ID, *subject.Char)
	})
}

func (w *World) publishItemSpawn(g *GroundItem) {
	for _, p := range w.nearbyWorldPlayers(g.X, g.Y, viewHalfX) {
		if !p.hasVisible(g.ID) {
			p.Session.Send(wire.CreateItem(g.X, g.Y, g.ID, g.Item, 0, 0, 0, 0, 0))
			p.show(g.ID)
		}
	}
}

func (w *World) publishItemRemove(g *GroundItem) {
	for _, p := range w.players {
		if p.InWorld && p.hasVisible(g.ID) {
			p.Session.Send(wire.RemoveItem(uint32(g.ID)))
			p.hide(g.ID)
		}
	}
}
