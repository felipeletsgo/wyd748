package game

import (
	"strings"

	"wydgo/internal/wire"
)

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
	if p == nil || !p.InWorld || m == nil || m.Dead ||
		!w.mobVisibleToPlayer(p, m) || p.hasVisible(m.ID) {
		return
	}
	anct := m.Def.Equip.AncientCodes()
	p.Session.Send(wire.CreateMobVisualExtended(m.ID, m.Def.Name, m.X, m.Y,
		m.Def.Mesh(), anct[:], mobPublicExtendedAt(m, w.now()), m.Affects[:], 0))
	p.show(m.ID)
}

// mobVisibleToPlayer is the visibility boundary for private instance mobs.
// They share the physical map with the rest of the world, but their packets
// must only reach members currently registered in the same stage.  Keeping
// this check in the publication layer prevents a caller that forgets the
// instance context from leaking CreateMob, movement, damage or death packets.
func (w *World) mobVisibleToPlayer(p *Player, m *Mob) bool {
	if p == nil || m == nil {
		return true
	}
	if m.InstanceID != "" {
		return instanceMemberInStage(w.instanceForMob(m), p)
	}
	// Global NPCs/merchants remain public. Hostile public monsters, however,
	// belong to the public gameplay space and must not leak into an event
	// runtime that happens to overlap the same physical coordinates.
	if m.Def != nil && m.Def.IsMonster() && m.SummonerID == 0 {
		return w.gameplaySpaceForPlayer(p) == ""
	}
	if m.SummonerID != 0 {
		owner := w.playerByID(m.SummonerID)
		if owner != nil {
			return w.gameplaySpaceForPlayer(p) == w.gameplaySpaceForPlayer(owner)
		}
	}
	return true
}

// gameplaySpaceForPlayer returns the authoritative runtime that owns a player,
// or the empty public-world space. A shared event still has a private
// interaction space: its RuntimeID is shared by all admitted participants,
// not by unrelated players who happen to occupy the same physical map tile.
// This distinction is shared by publication, combat, support, summons, trade
// and party admission.
func (w *World) gameplaySpaceForPlayer(p *Player) string {
	if w == nil || p == nil {
		return ""
	}
	runtimeID := w.playerRuntimeInstanceID(p.ID)
	if runtimeID == "" {
		return ""
	}
	return runtimeID
}

func (w *World) playersShareGameplaySpace(a, b *Player) bool {
	if a == nil || b == nil {
		return false
	}
	aSpace := w.gameplaySpaceForPlayer(a)
	bSpace := w.gameplaySpaceForPlayer(b)
	if aSpace == "" && bSpace == "" {
		return true
	}
	return aSpace != "" && aSpace == bSpace
}

// privateWaterRuntimeIDForPlayer remains as a compatibility-named wrapper for
// ground-item and legacy visibility code. The underlying lookup is O(1) and
// now uses the same authoritative gameplay-space index as combat.
func (w *World) privateWaterRuntimeIDForPlayer(playerID uint16) string {
	if w == nil || playerID == 0 {
		return ""
	}
	p := w.playersByID[playerID]
	if p == nil {
		// Tests and restore paths may not have a live Player object yet. The
		// runtime index still gives a safe answer for a water-owned ID.
		runtimeID := w.playerRuntimeInstanceID(playerID)
		if runtimeID != "" && isDurablePrivateWaterInstance(w.itemInstances[runtimeID]) {
			return runtimeID
		}
		return ""
	}
	runtimeID := w.gameplaySpaceForPlayer(p)
	if runtimeID != "" && isDurablePrivateWaterInstance(w.itemInstances[runtimeID]) {
		return runtimeID
	}
	return ""
}

// playersVisibleTogether applies the interaction-space boundary
// symmetrically. Public players continue to see one another normally; members
// of the same event runtime can see one another; a runtime member never sees
// or leaks to another runtime/public player merely because coordinates match.
func (w *World) playersVisibleTogether(a, b *Player) bool {
	if a == nil || b == nil || a == b {
		return a != nil && b != nil && a == b
	}
	return w.playersShareGameplaySpace(a, b)
}

func (w *World) groundItemVisibleToPlayer(p *Player, g *GroundItem) bool {
	if p == nil || g == nil {
		return false
	}
	if g.Permanent {
		return true
	}
	// Every non-permanent ground item belongs to exactly one gameplay space.
	// Empty means public; a runtime id means only that runtime. This is generic
	// for Water, Cube, Nightmare, Hell Gate, Uxmal and player drops alike.
	return w.gameplaySpaceForPlayer(p) == strings.TrimSpace(g.InstanceID)
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
		wire.CreateMobExtendedWithGuildRank(subject.ID, subject.Char.Name, subject.X, subject.Y,
			bodyMesh(subject.Char), bodyAncient(subject.Char), wireExtendedScore(subject.Char), subject.Char.Affects[:], 2, subject.Char.GuildID, subject.Char.GuildRank, subject.Char.CP),
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

// refreshAppearance publica uma mudanca REAL de aparencia do avatar
// (equipamento, transformacao de rosto, cor, refino, capa ou montaria) pelo
// MSG_UpdateEquip 0x36B. Esse e o SendEquip/GridMulticast nativo: nao contem
// coordenadas e portanto nao interrompe nem encaixa uma caminhada em andamento.
// CreateMob fica reservado exclusivamente para entrada/reentrada na visao.
func (w *World) refreshAppearance(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil {
		return
	}
	w.sendToPlayerView(subject, func() []byte {
		return playerAppearancePacket(subject)
	})
}

func playerAppearancePacket(subject *Player) []byte {
	if subject == nil || subject.Char == nil {
		return nil
	}
	return wire.VisualEquip(subject.ID, bodyMesh(subject.Char), bodyAncient(subject.Char))
}

func (w *World) showPlayerPair(a, b *Player) {
	if a == nil || b == nil || a == b || !a.InWorld || !b.InWorld ||
		a.Char == nil || b.Char == nil || !w.playersVisibleTogether(a, b) {
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
		if !w.mobVisibleToPlayer(p, m) {
			continue
		}
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
		if other == p || !w.playersVisibleTogether(p, other) {
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
		if _, nearby := nearPlayerIDs[id]; !nearby || !w.playersVisibleTogether(p, other) {
			w.hidePlayerPair(p, other)
		}
	}
	for _, g := range w.groundItems {
		visible := w.groundItemVisibleToPlayer(p, g) && inView(p.X, p.Y, g.X, g.Y)
		switch {
		case visible && !p.hasVisible(g.ID):
			p.Session.Send(wire.CreateItem(g.X, g.Y, g.ID, g.Item, g.Rotate, g.State, 0, 0, 0))
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
	w.publishRegisteredMobSpawn(m)
}

// publishRegisteredMobSpawn publishes an entity that has already crossed all
// fallible registration steps. Boss spawn uses this to avoid exposing an
// orphan when RegisterBoss rejects its profile.
func (w *World) publishRegisteredMobSpawn(m *Mob) {
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if w.mobVisibleToPlayer(p, m) {
			w.showMob(p, m)
		}
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
		if !w.mobVisibleToPlayer(p, m) {
			if p.hasVisible(m.ID) {
				w.hideMob(p, m, 0)
			}
			continue
		}
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

// publishPlayerMove replica somente uma mudanca real de destino. O TMSrv nativo
// conserva Route[24] e a publica junto da origem informada no segmento. Descartar
// essa rota obrigava cada observador a recalcular o caminho e causava pequenas
// correcoes em curvas, desniveis ou mudancas de destino.
func (w *World) publishPlayerMove(player *Player, fromX, fromY uint16, route []byte) {
	if player == nil || !player.InWorld || player.Char == nil {
		return
	}
	if player.MovePublished && player.MovePublishedTargetX == player.X && player.MovePublishedTargetY == player.Y {
		return
	}
	if fromX == player.X && fromY == player.Y {
		return
	}
	// BASE_GetSpeed: nibble baixo de AttackRun, limitado a 1..6. Usar o score
	// impede speed hack e conserva visualmente botas/buffs de corrida.
	speed := uint32(playerAttackRun(player.Char) & 0x0F)
	// O jogador ainda esta indexado na celula anterior neste ponto. Consulte a
	// uniao das duas janelas para que quem ja o via na origem receba o segmento;
	// jogadores novos so serao materializados pelo refresh posterior.
	observers := make(map[uint16]*Player)
	if player.MovePublished {
		for _, observer := range w.nearbyWorldPlayers(
			player.MovePublishedTargetX, player.MovePublishedTargetY, viewHalfX) {
			observers[observer.ID] = observer
		}
	}
	for _, observer := range w.nearbyWorldPlayers(fromX, fromY, viewHalfX) {
		observers[observer.ID] = observer
	}
	for _, observer := range w.nearbyWorldPlayers(player.X, player.Y, viewHalfX) {
		observers[observer.ID] = observer
	}
	for _, observer := range observers {
		if observer != player && !w.playersVisibleTogether(observer, player) {
			w.hidePlayerPair(observer, player)
			continue
		}
		if observer != player && observer.hasVisible(player.ID) {
			observer.Session.Send(wire.PlayerMove(player.ID, fromX, fromY, player.X, player.Y, speed, route))
		}
	}
	player.MovePublished = true
	player.MovePublishedStartX, player.MovePublishedStartY = fromX, fromY
	player.MovePublishedTargetX, player.MovePublishedTargetY = player.X, player.Y
	player.MovePublishedRoute = [maxMovementRouteBytes]byte{}
	for index, step := range route {
		if index >= len(player.MovePublishedRoute) || step == 0 {
			break
		}
		player.MovePublishedRoute[index] = step
	}
}

// publishPlayerStop encerra uma rota sem usar ActionStop/Effect=1. Esse efeito
// pertence a spawn, teleporte e correcao dura; envia-lo em toda parada encaixava
// o avatar remoto instantaneamente. Ao chegar ao destino, a rota ja termina
// sozinha. Uma parada intermediaria recebe uma unica reorientacao Effect=0.
func (w *World) publishPlayerStop(player *Player) {
	if player == nil || !player.InWorld || player.Char == nil || !player.MovePublished {
		return
	}
	startX, startY := player.MovePublishedStartX, player.MovePublishedStartY
	plannedX, plannedY := player.MovePublishedTargetX, player.MovePublishedTargetY
	if player.X != plannedX || player.Y != plannedY {
		speed := uint32(playerAttackRun(player.Char) & 0x0F)
		observers := make(map[uint16]*Player)
		for _, point := range [][2]uint16{{startX, startY}, {plannedX, plannedY}, {player.X, player.Y}} {
			for _, observer := range w.nearbyWorldPlayers(point[0], point[1], viewHalfX) {
				observers[observer.ID] = observer
			}
		}
		for _, observer := range observers {
			if observer != player && !w.playersVisibleTogether(observer, player) {
				w.hidePlayerPair(observer, player)
				continue
			}
			if observer != player && observer.hasVisible(player.ID) {
				observer.Session.Send(wire.PlayerMove(
					player.ID, startX, startY, player.X, player.Y, speed, nil))
			}
		}
	}
	clearPublishedPlayerMove(player)
}

func clearPublishedPlayerMove(player *Player) {
	if player == nil {
		return
	}
	player.MovePublished = false
	player.MovePublishedStartX = 0
	player.MovePublishedStartY = 0
	player.MovePublishedTargetX = 0
	player.MovePublishedTargetY = 0
	player.MovePublishedRoute = [maxMovementRouteBytes]byte{}
}

func (w *World) publishMobDeath(m *Mob, killerID uint16, killerExp uint32, expByPlayer map[*Player]uint32) {
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if !w.mobVisibleToPlayer(p, m) || !p.hasVisible(m.ID) {
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

// publishPlayerDeath envia o total de EXP de CADA destinatario. O client chama
// SetMyHumanExp quando o killer e ele proprio ou um membro da party; reutilizar
// a EXP da vitima aqui fazia o client do killer receber visualmente o total da
// pessoa morta. PvP nao concede EXP nem gold.
func (w *World) publishPlayerDeath(victim *Player, killerID uint16) {
	if victim == nil || !victim.InWorld {
		return
	}
	for _, recipient := range w.nearbyWorldPlayers(victim.X, victim.Y, viewHalfX) {
		if !w.playersVisibleTogether(recipient, victim) {
			continue
		}
		if recipient != victim && !recipient.hasVisible(victim.ID) {
			continue
		}
		recipient.Session.Send(playerDeathPacket(recipient, victim, killerID))
	}
}

func playerDeathPacket(recipient, victim *Player, killerID uint16) []byte {
	var exp uint32
	if recipient != nil && recipient.Char != nil {
		exp = recipient.Char.Exp
	}
	return wire.CNFMobKill(victim.ID, killerID, exp)
}

// publishMobRemoval retira um mob que nunca morreu em combate (rollback de
// spawn, reload ou cleanup administrativo). Diferente de publishMobDeath, nao
// envia CNFMobKill nem cria um hit visual falso no client.
func (w *World) publishMobRemoval(m *Mob) {
	if m == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if !w.mobVisibleToPlayer(p, m) || !p.hasVisible(m.ID) {
			continue
		}
		p.Session.Send(wire.RemoveMob(m.ID, 0))
		p.hide(m.ID)
	}
}

func (w *World) sendToMobView(m *Mob, build func() []byte) {
	if m == nil {
		return
	}
	for _, p := range w.nearbyWorldPlayers(m.X, m.Y, viewHalfX) {
		if w.mobVisibleToPlayer(p, m) && p.hasVisible(m.ID) {
			p.Session.Send(build())
		}
	}
}

func (w *World) sendToPlayerView(subject *Player, build func() []byte) {
	if subject == nil || !subject.InWorld {
		return
	}
	for _, p := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if !w.playersVisibleTogether(p, subject) {
			continue
		}
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

// syncPlayerVitalsToObservers manda o 0x181 para quem VE o jogador, menos ele
// proprio. Serve aos fluxos que ja enviaram um 0x336 privado ao dono.
//
// O motivo e o flicker da barra: o wrapper do patch wide copia a cauda uint32
// para o sidecar e DEPOIS chama o handler nativo, sempre (o comentario do
// Patch-WYD748-ExtendedStats.ps1 afirma que as sincronizacoes seguintes
// retornam antes dele, mas os quatro desvios e o fall-through convergem no
// mesmo `call`). Cada pacote de vitals custa um redesenho nativo; mandar 0x336
// e 0x181 em sequencia ao mesmo jogador custa DOIS, e a barra pisca.
//
// O 0x336 ja carrega HP/MP nos WORDs legados e na cauda wide, entao o dono nao
// perde nada. Os observadores continuam recebendo, porque para eles o 0x336
// privado nunca chegou.
func (w *World) syncPlayerVitalsToObservers(subject *Player) {
	if subject == nil || subject.Char == nil || !subject.InWorld {
		return
	}
	for _, p := range w.nearbyWorldPlayers(subject.X, subject.Y, viewHalfX) {
		if p != subject && w.playersVisibleTogether(p, subject) && p.hasVisible(subject.ID) {
			p.Session.Send(wire.SetHpMpExtended(subject.ID, wireExtendedScore(subject.Char)))
		}
	}
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

// syncPlayerChaos publica a mudança do byte de PK/chaos que fica dentro do
// CreateMob. UpdateEtc não possui campo de CP no protocolo 7.54; enviar apenas
// 0x337 deixaria o nome dos observadores com a cor anterior. O pacote mantém a
// mesma posição e não inclui Action, portanto não reinicia a caminhada.
func (w *World) syncPlayerChaos(subject *Player) {
	if subject == nil || !subject.InWorld || subject.Char == nil {
		return
	}
	w.sendToPlayerView(subject, func() []byte {
		return wire.CreateMobExtendedWithGuildRank(subject.ID, subject.Char.Name, subject.X, subject.Y,
			bodyMesh(subject.Char), bodyAncient(subject.Char), wireExtendedScore(subject.Char),
			subject.Char.Affects[:], 2, subject.Char.GuildID, subject.Char.GuildRank, subject.Char.CP)
	})
}

func (w *World) publishItemSpawn(g *GroundItem) {
	for _, p := range w.nearbyWorldPlayers(g.X, g.Y, viewHalfX) {
		if w.groundItemVisibleToPlayer(p, g) && !p.hasVisible(g.ID) {
			p.Session.Send(wire.CreateItem(g.X, g.Y, g.ID, g.Item, g.Rotate, g.State, 0, 0, 0))
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
