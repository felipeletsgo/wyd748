package game

import (
	"encoding/binary"
	"errors"
	"log"
	"strconv"
	"strings"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// tipos de "lugar" do SwapItem (decompilado 7.54).
const (
	placeEquip   = 0
	placeInv     = 1
	placeStorage = 2
)

// shopMerchant = valor do nibble baixo de Merchant que o client 7.48 reconhece
// como loja de gold (m_stScore.Reserved & 0xF == 3 no click handler do client).
const shopMerchant = 3
const skillMasterMerchant = 19
const nativeShopMerchant = 1
const craftingMerchant = 8

// clientShopSlots e o limite REAL de exibicao da loja no client 7.48: o
// OnPacketShopList percorre `for (i = 0; i < 27; ++i)` e monta a grade com
// (i%5, i/5). Item alem da 27a posicao do pacote simplesmente nunca e desenhado.
const clientShopSlots = 27

// shopDisplayList devolve a lista de venda como o client vai enxerga-la.
//
// Os NPCs convertidos do Micronics espalham o estoque pelas 64 posicoes do
// Inventory original, com buracos: o Aki, por exemplo, tem itens em 1..7, em
// 27..31 e em 33..35. Como o client so le as 27 primeiras entradas, tudo que
// estiver depois some da loja. Compactar resolve, preservando a ordem.
//
// A loja de SKILL e a excecao: ali os vazios em 8, 17 e 26 sao separadores dos
// tres ramos e precisam ser mantidos, senao a grade do mestre embaralha.
//
// ESTA FUNCAO E A FONTE UNICA: o mesmo resultado alimenta o pacote 0x17C e a
// resolucao do slot na compra (0x379). Compactar so na exibicao faria o client
// comprar o item errado.
func shopDisplayList(vende []model.Item, shopType uint32) []model.Item {
	if shopType == wire.ShopSkill {
		return vende
	}
	out := make([]model.Item, 0, clientShopSlots)
	for _, item := range vende {
		if item.Index == 0 {
			continue
		}
		if len(out) == clientShopSlots {
			break
		}
		out = append(out, item)
	}
	return out
}

// shopSlotFromClient converte a posicao que o client manda no 0x379 para o
// indice da lista que enviamos no 0x17C.
//
// O client NAO envia o indice da grade: ele remapeia antes (SGrid.cpp,
// SGridControl::BuyItem):
//
//	SourPos = cellX + 5*cellY          // = i, o indice da lista exibida
//	SourPos = SourPos % 9 + 27 * (SourPos / 9)
//
// Isso revela o layout nativo da loja: BLOCOS DE 9 espacados de 27 (0..8,
// 27..35, 54..62) -- a mesma estrutura dos 27 slots do mestre de skill, e
// exatamente onde os NPCs do Micronics guardam o estoque. Os indices 0..8
// mapeiam para si mesmos, o que fazia as primeiras compras funcionarem e
// mascarava o problema.
//
// A inversa: bloco = S/27, posicao = S%27 (precisa ser < 9), indice = pos + 9*bloco.
func shopSlotFromClient(sourPos uint16) (int, bool) {
	block := int(sourPos) / 27
	pos := int(sourPos) % 27
	if pos >= 9 {
		// Regiao morta entre os blocos: nao corresponde a nenhuma celula.
		return 0, false
	}
	return pos + 9*block, true
}

// countShopItems conta apenas as posicoes ocupadas: a lista crua do Micronics
// tem buracos, entao len() nao diz quantos itens a loja realmente oferece.
func countShopItems(vende []model.Item) int {
	count := 0
	for _, item := range vende {
		if item.Index != 0 {
			count++
		}
	}
	return count
}

func shopTypeForMerchant(merchant byte) (uint32, bool) {
	if merchant == skillMasterMerchant {
		return wire.ShopSkill, true
	}
	if merchant == nativeShopMerchant || merchant&0xF == shopMerchant {
		return wire.ShopNormal, true
	}
	return 0, false
}

// onLogin: 0x20D. Carrega a conta do store e manda a char-list.
func (w *World) onLogin(s *net.Session, pkt []byte) {
	if len(pkt) < 44 || w.authPending[s] || w.players[s] != nil {
		return
	}
	// Desligando: o snapshot final ja foi gravado, entao aceitar a entrada agora
	// criaria estado que ninguem persistiria.
	if w.shuttingDown {
		s.Send(wire.MessagePanel("Server going into maintenance. Try again shortly."))
		return
	}
	accountName := cstr(pkt[12:28])
	password := cstr(pkt[28:40])
	cliver := binary.LittleEndian.Uint32(pkt[40:44])
	if !w.allowLoginAttempt(s.RemoteIP(), accountName, w.now()) {
		log.Printf("[#%d] LOGIN limitado conta=%q ip=%q", s.ID, accountName, s.RemoteIP())
		s.Send(wire.MessagePanel("Too many login attempts. Try again later."))
		return
	}
	log.Printf("[#%d] LOGIN conta=%q cliver=%d", s.ID, accountName, cliver)
	select {
	case w.authSlots <- struct{}{}:
	default:
		s.Send(wire.MessagePanel("Login server busy. Try again."))
		return
	}
	w.authPending[s] = true
	go func() {
		defer func() { <-w.authSlots }()
		acc, err := account.Authenticate(w.store, accountName, password)
		w.commands <- command{s: s, queuedAt: time.Now(), login: &loginResult{
			accountName: accountName, account: acc, err: err,
		}}
	}()
}

func (w *World) onLoginResult(s *net.Session, result *loginResult) {
	if !w.authPending[s] {
		return
	}
	delete(w.authPending, s)
	if result.err != nil {
		if errors.Is(result.err, account.ErrInvalidCredentials) {
			log.Printf("[#%d] LOGIN recusado conta=%q", s.ID, result.accountName)
		} else {
			log.Printf("[#%d] LOGIN erro conta=%q: %v", s.ID, result.accountName, result.err)
		}
		s.Send(wire.MessagePanel("Wrong account or password."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	acc := result.account
	if !w.claimAccountSession(s, acc.Name) {
		log.Printf("[#%d] LOGIN recusado conta=%q: conta ja conectada", s.ID, acc.Name)
		s.Send(wire.AlreadyPlaying())
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	// ANTI-DUPE: recarrega a conta do DISCO agora que a sessao ja esta reservada.
	// O snapshot que a goroutine de auth leu antes do PBKDF2 pode estar velho -- se
	// a sessao viva mexeu no inventario/gold e salvou (ou desconectou) durante a
	// verificacao de senha, admitir o snapshot restauraria o estado pre-troca e
	// duplicaria itens/gold. A senha ja foi conferida contra o mesmo hash; aqui so
	// interessa o estado de jogo, que passa a vir do arquivo mais recente.
	if fresh, err := w.store.LoadAccount(acc.Name); err != nil {
		w.releaseAccountSession(s, acc)
		log.Printf("[#%d] LOGIN erro ao recarregar conta %q: %v", s.ID, acc.Name, err)
		s.Send(wire.MessagePanel("Error loading the account. Try again."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	} else {
		acc = fresh
	}
	pinAccountEntryPositions(acc)
	// O bonus de status do Arch acompanha o nivel ATUAL do Mortal de origem;
	// atualizar aqui, antes do syncProgression abaixo, faz o saldo de pontos ja
	// nascer certo nesta sessao.
	refreshArchMortalLevel(acc)
	guildRepaired := false
	for i := range acc.Chars {
		if acc.Chars[i].Name == "" {
			continue
		}
		syncProgression(&acc.Chars[i])
		w.recalcPlayer(&acc.Chars[i])
		filterShortSkills(&acc.Chars[i])
		// GuildID/GuildRank sao copias desnormalizadas: a guild pode ter sido
		// dissolvida ou o rank mudado enquanto este personagem estava offline.
		if w.repairGuildState(&acc.Chars[i]) {
			guildRepaired = true
		}
	}
	if guildRepaired {
		// Persistir o reparo agora evita reencenar a mesma correcao a cada login.
		if err := w.saveAccount(acc); err != nil {
			log.Printf("[#%d] salvar reparo de guild da conta %q: %v", s.ID, acc.Name, err)
		}
	}
	p := &Player{Session: s, Account: acc, CharSlot: -1}
	w.players[s] = p
	s.Send(wire.CharList(acc.Name, acc.Chars, acc.Cargo[:], acc.CargoGold))
	log.Printf("[#%d] char-list enviada (%d personagem(ns))", s.ID, characterCount(acc.Chars))
}

const maxCharactersPerAccount = 4

var reservedCharacterNames = map[string]struct{}{
	"ADMIN": {}, "CARALHO": {}, "PINTO": {}, "PORRA": {}, "BUCETA": {},
	"CUZAO": {}, "GRITAR": {}, "CRIAR": {}, "SAIR": {}, "DONATE": {},
	"REINO": {}, "KINGDOM": {}, "KING": {}, "KICKPARTY": {}, "GTAX": {},
	"TRANSFERIR": {}, "CLEARINV": {}, "CHANGEINV": {},
}

func validCharacterName(name string) bool {
	if len(name) < 4 || len(name) > 13 {
		return false
	}
	if _, reserved := reservedCharacterNames[strings.ToUpper(name)]; reserved {
		return false
	}
	if _, command := chatCommandAliases[strings.ToLower(name)]; command {
		return false
	}
	for _, c := range []byte(name) {
		if (c < 'A' || c > 'Z') && (c < 'a' || c > 'z') {
			return false
		}
	}
	return true
}

func characterCount(chars []model.Char) int {
	count := 0
	for _, ch := range chars {
		if ch.Name != "" {
			count++
		}
	}
	return count
}

func (w *World) newCharacterFromTemplate(name string, class int) (model.Char, bool) {
	if class < 0 || class >= len(w.charTemplates) {
		return model.Char{}, false
	}
	template := w.charTemplates[class]
	if template.Name == "" {
		return model.Char{}, false
	}
	created := template.NewCharacter(name, w.charSpawn)
	uid, err := model.NewCharacterUID()
	if err != nil {
		log.Printf("gerar UID do personagem %q: %v", name, err)
		return model.Char{}, false
	}
	created.UID = uid
	syncProgression(&created)
	w.recalcPlayer(&created)
	setPlayerCurHP(&created, playerMaxHP(&created))
	setPlayerCurMP(&created, playerMaxMP(&created))
	return created, true
}

// onCreateCharacter trata o MSG_NewCharacter 0x20F integralmente no servidor:
// valida, instancia o template da classe, persiste e atualiza a selecao.
func (w *World) onCreateCharacter(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || p.InWorld || len(pkt) < 36 {
		s.Send(wire.NewCharacterFail())
		return
	}
	slot := int(int32(binary.LittleEndian.Uint32(pkt[12:16])))
	name := cstr(pkt[16:32])
	class := int(int32(binary.LittleEndian.Uint32(pkt[32:36])))
	if slot < 0 || slot >= maxCharactersPerAccount || class < 0 || class >= 4 ||
		!validCharacterName(name) || characterCount(p.Account.Chars) >= maxCharactersPerAccount {
		log.Printf("[#%d] criacao recusada slot=%d nome=%q classe=%d", s.ID, slot, name, class)
		s.Send(wire.NewCharacterFail())
		return
	}
	if slot < len(p.Account.Chars) && p.Account.Chars[slot].Name != "" {
		s.Send(wire.NewCharacterFail())
		return
	}
	exists, err := w.characterNameTaken(name)
	if err != nil || exists {
		if err != nil {
			log.Printf("[#%d] verificar nome %q: %v", s.ID, name, err)
		}
		s.Send(wire.NewCharacterFail())
		return
	}
	created, ok := w.newCharacterFromTemplate(name, class)
	if !ok {
		log.Printf("[#%d] layout ausente para classe %d", s.ID, class)
		s.Send(wire.NewCharacterFail())
		return
	}
	previousChars := append([]model.Char(nil), p.Account.Chars...)
	for len(p.Account.Chars) <= slot {
		p.Account.Chars = append(p.Account.Chars, model.Char{})
	}
	p.Account.Chars[slot] = created
	if err := w.saveAccount(p.Account); err != nil {
		p.Account.Chars = previousChars
		log.Printf("[#%d] salvar personagem novo %q: %v", s.ID, name, err)
		s.Send(wire.NewCharacterFail())
		return
	}
	if w.charNames != nil {
		w.charNames[strings.ToLower(name)] = struct{}{}
	}
	s.Send(wire.CNFNewCharacter(uint16(s.ID), p.Account.Chars))
	log.Printf("[#%d] personagem criado: %q slot=%d classe=%d nascimento=(%d,%d)",
		s.ID, name, slot, class, created.X, created.Y)
}

// onEnterWorld: 0x213. Materializa o char no mundo e dispara a sequencia de login
// na ordem exata do Micronics (sem ela os campos do client ficam nao-inicializados).
func (w *World) onEnterWorld(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || len(pkt) != characterLoginPacketSize {
		log.Printf("[#%d] enter-world sem player/char", s.ID)
		return
	}
	slot := int(int32(binary.LittleEndian.Uint32(pkt[12:16])))
	if slot < 0 || slot >= len(p.Account.Chars) || slot >= maxCharactersPerAccount || p.Account.Chars[slot].Name == "" {
		log.Printf("[#%d] enter-world com slot invalido %d", s.ID, slot)
		return
	}
	p.CharSlot = slot
	p.Char = &p.Account.Chars[slot]
	ch := p.Char
	entryX, entryY := playerEntryX, playerEntryY
	if isLoadtestAccountName(p.Account.Name, w.loadtestAccountPrefix) {
		entryX, entryY = w.loadtestSpawn.X, w.loadtestSpawn.Y
		// Bots continuam na area Tauron, mas nao ocupam todos a mesma celula:
		// isso exercita movimento/colisao e evita que o simulador pare por uma
		// rejeicao de rota causada pelo proprio grupo de carga.
		entryX, entryY = w.findFreePlayerPosition(entryX, entryY, 32, p)
	}
	ch.X, ch.Y = entryX, entryY
	// O loader primeiro obtem o sidecar autoritativo e somente depois substitui
	// affects/moedas em RAM. Assim uma falha de PostgreSQL nao apaga o estado
	// atual nem deixa o personagem entrar com um agregado parcial.
	if err := w.loadCharStateInto(p); err != nil {
		log.Printf("[#%d] ERRO ao carregar charstate de %q: %v", s.ID, ch.Name, err)
		p.CharSlot = -1
		p.Char = nil
		s.Send(wire.MessagePanel("The character state could not be loaded. Try again."))
		return
	}
	w.recalcPlayer(ch)
	// Quem morreu e saiu tem CurHP=0 persistido. Entrar assim TRAVA o jogador:
	// o client desenha a pose de morte e nao responde a nada -- ele nem pode
	// pedir o /restart. Devolver o minimo de vida deixa o personagem jogavel
	// para andar ate um curandeiro ou usar uma pocao.
	//
	// Vem DEPOIS do recalc para que MaxHP ja esteja correto, e ANTES de qualquer
	// pacote: EnterWorld, CreateMob, UpdateScore e SetHpMp levam o HP corrigido.
	if playerMaxHP(ch) > 0 && playerCurHP(ch) == 0 {
		setPlayerCurHP(ch, 1)
		p.DeadAt = time.Time{}
		log.Printf("[#%d] %s entrou morto; revivido com 1 de HP", s.ID, ch.Name)
	}
	// ClientId = MENOR slot livre (comportamento do TMSrv nativo, que usa o indice
	// da conexao). Um contador so-crescente dava id novo a cada relog e o client
	// 7.48 (mesmo processo) mantem estado atrelado ao id antigo -> chaos com lixo
	// gigante e HP/MP travados na segunda entrada. Nunca sobrescrever um jogador
	// quando os 999 slots estao ocupados.
	playerID, ok := w.allocPlayerID()
	if !ok {
		p.CharSlot = -1
		p.Char = nil
		p.InWorld = false
		p.ID = 0
		s.Send(wire.MessagePanel("The world is full. Please try again later."))
		log.Printf("[#%d] ENTER-WORLD recusado: limite de 999 jogadores atingido", s.ID)
		return
	}
	p.ID = playerID
	p.InWorld = true
	p.X, p.Y = entryX, entryY
	p.NextCPRecovery = w.now().Add(chaosRecoveryInterval)
	w.playersByID[p.ID] = p
	w.indexPlayerCharacter(p)
	// A private Water instance is keyed by Character UID, not the process-local
	// player ID. Reattach before building EnterWorld/CreateMob so reconnecting
	// at a saved room is a single authoritative position, never a correction
	// teleport visible to other clients.
	w.attachRestoredInstanceMember(p)
	log.Printf("[#%d] ENTER-WORLD %s id=%d @(%d,%d)", s.ID, ch.Name, p.ID, ch.X, ch.Y)

	// 1) enter-world (STRUCT_MOB completo)
	s.Send(wire.EnterWorld(p.ID, *ch))
	// 2) self-CreateMob (spawn=2): materializa o proprio player. Parte da sequencia
	// COMPROVADA in-game; sem ele o re-enter (2o login do mesmo client) reconstroi o
	// self com estado velho (HP/MP travados). ActionStop vem depois, senao reseta a pose.
	s.Send(wire.CreateMobExtendedWithGuildRank(p.ID, ch.Name, ch.X, ch.Y, bodyMesh(ch),
		bodyAncient(ch), wireExtendedScore(ch), ch.Affects[:], 2, ch.GuildID, ch.GuildRank, ch.CP))
	// 3) sequencia de login (ordem Micronics): 3A8 -> 336 -> 185 -> 337 -> 36B -> 181 -> 366
	s.Send(wire.WarInfo())
	s.Send(wire.UpdateScore(p.ID, *ch))
	s.Send(wire.UpdateAffects(p.ID, *ch))
	s.Send(wire.UpdateCarry(p.ID, ch.Inv[:], ch.Gold))
	s.Send(wire.UpdateEtc(p.ID, *ch))
	s.Send(wire.SelfEquip(p.ID, ch.Equip[:]))
	s.Send(wire.SetHpMpExtended(p.ID, wireExtendedScore(ch)))
	s.Send(wire.ActionStop(p.ID, ch.X, ch.Y))
	s.Send(wire.SetShortSkill(p.ID, ch.ShortSkill))

	// 4) materializa mobs/outros players/itens DENTRO do raio e popula p.Visible.
	// Antes mandava CreateMob de todos os mobs sem filtro (e sem semear p.Visible),
	// deixando o sistema de visibilidade inconsistente e sem escalar pra milhares.
	w.refreshPlayerVisibility(p)
	log.Printf("[#%d] visibilidade inicial: %d entidades", s.ID, len(p.Visible))
}

// isLoadtestAccountName restringe o nascimento alternativo ao conjunto que o
// provisionador realmente cria. Um nome arbitrario que apenas comece por
// "bot" nao deve ser movido para Noatum por acidente.
func isLoadtestAccountName(name, prefix string) bool {
	prefix = strings.ToLower(strings.TrimSpace(prefix))
	name = strings.ToLower(strings.TrimSpace(name))
	if prefix == "" || !strings.HasPrefix(name, prefix) {
		return false
	}
	suffix := name[len(prefix):]
	if len(suffix) != 4 {
		return false
	}
	for _, ch := range suffix {
		if ch < '0' || ch > '9' {
			return false
		}
	}
	n, err := strconv.Atoi(suffix)
	return err == nil && n >= 1 && n <= 990
}

// onApplyBonus trata MSG_ApplyBonus 0x277: BonusType int16@12, Detail int16@14.
func (w *World) onApplyBonus(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) != applyBonusPacketSize || playerCurHP(p.Char) == 0 {
		return
	}
	bonusType := int(int16(binary.LittleEndian.Uint16(pkt[12:14])))
	detail := int(int16(binary.LittleEndian.Uint16(pkt[14:16])))
	// No 7.48 a compra no mestre de habilidades nao e MSG_BuyItem: o client
	// envia MSG_ApplyBonus tipo 2 com o item de skill em Detail e o mestre em
	// TargetID. Tratar como bonus comum descartava toda tentativa de aprender.
	if bonusType == 2 {
		itemIndex := int(binary.LittleEndian.Uint16(pkt[14:16]))
		masterID := binary.LittleEndian.Uint16(pkt[16:18])
		w.onLearnSkillAtMaster(s, p, itemIndex, masterID)
		return
	}
	// Mesmo cuidado do killMobState: o recalc cru do applyBonus clampa o CurHP
	// buffado; captura antes e restaura depois do recalcPlayer.
	oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)
	if !applyBonus(p.Char, bonusType, detail) {
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		return
	}
	w.recalcPlayer(p.Char) // reaplica equip+affects (applyBonus recalcula cru)
	if oldHP > 0 {
		setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
	}
	if oldMP > 0 {
		setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
	}
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.syncPlayerVitalsToObservers(p)
	w.updatePartyMember(p)
	log.Printf("[#%d] bonus type=%d detail=%d STR=%d INT=%d DEX=%d CON=%d ATK=%d MATK=%d DEF=%d HP=%d/%d MP=%d/%d special=%v",
		s.ID, bonusType, detail, playerStr(p.Char), playerInt(p.Char), playerDex(p.Char),
		playerCon(p.Char),
		playerAttack(p.Char), playerMagicAttack(p.Char), playerDefense(p.Char),
		playerCurHP(p.Char), playerMaxHP(p.Char), playerCurMP(p.Char), playerMaxMP(p.Char),
		effectiveExtended(p.Char).Mastery)
}

// onSwapItem: 0x376. Move/equipa item de forma AUTORITATIVA -- troca no estado do
// servidor e confirma com um 0x182 por slot afetado (o client so aplica ao receber).
func (w *World) onSwapItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || len(pkt) != 20 {
		return
	}
	st, sp, dt, dp := pkt[12], pkt[13], pkt[14], pkt[15]
	src := playerSlotOf(p, st, sp)
	dst := playerSlotOf(p, dt, dp)
	if src == nil || dst == nil {
		log.Printf("[#%d] SWAP invalido %d/%d -> %d/%d", s.ID, st, sp, dt, dp)
		return
	}
	if (st == placeStorage && p.ghostShopLocksCargoSlot(int(sp))) ||
		(dt == placeStorage && p.ghostShopLocksCargoSlot(int(dp))) {
		s.Send(wire.MessagePanel("The item is locked while it is listed on Auto Trade."))
		s.Send(wire.SendItem(p.ID, st, sp, *src))
		s.Send(wire.SendItem(p.ID, dt, dp, *dst))
		return
	}
	if (st == placeStorage || dt == placeStorage) && !w.validCargoAccess(p, pkt) {
		s.Send(wire.MessagePanel("Move closer to the Warehouse to move Cargo."))
		s.Send(wire.SendItem(p.ID, st, sp, *src))
		s.Send(wire.SendItem(p.ID, dt, dp, *dst))
		return
	}
	if (st == placeStorage && dt == placeEquip) || (st == placeEquip && dt == placeStorage) {
		s.Send(wire.MessagePanel("Move the item through your inventory before equipping it."))
		s.Send(wire.SendItem(p.ID, st, sp, *src))
		s.Send(wire.SendItem(p.ID, dt, dp, *dst))
		return
	}
	w.cancelTrade(p, "inventario alterado")
	oldSrc, oldDst := *src, *dst
	*src, *dst = *dst, *src
	// O client nunca decide se um item pode ocupar um slot de equipamento. Faz
	// a troca provisoria, valida todos os slots de equip afetados e reverte se
	// classe, requisitos ou nPos nao forem satisfeitos.
	valid := true
	if st == placeEquip {
		valid = w.canEquip(p.Char, *src, sp)
	}
	if valid && dt == placeEquip {
		valid = w.canEquip(p.Char, *dst, dp)
	}
	if !valid {
		*src, *dst = oldSrc, oldDst
		s.Send(wire.SendItem(p.ID, st, sp, *src))
		s.Send(wire.SendItem(p.ID, dt, dp, *dst))
		log.Printf("[#%d] SWAP equip rejeitado %d/%d -> %d/%d", s.ID, st, sp, dt, dp)
		return
	}
	if st == placeStorage || dt == placeStorage {
		if err := w.saveAccount(p.Account); err != nil {
			*src, *dst = oldSrc, oldDst
			s.Send(wire.SendItem(p.ID, st, sp, *src))
			s.Send(wire.SendItem(p.ID, dt, dp, *dst))
			s.Send(wire.MessagePanel("Save failed. The Cargo move was cancelled."))
			log.Printf("[#%d] salvar movimento do Cargo: %v", s.ID, err)
			return
		}
	}
	s.Send(wire.SendItem(p.ID, st, sp, *src))
	s.Send(wire.SendItem(p.ID, dt, dp, *dst))
	if st == placeEquip || dt == placeEquip {
		if (st == placeEquip && sp == mountSlot) || (dt == placeEquip && dp == mountSlot) {
			w.resetEggIncubationClock(p)
		}
		// Montaria recem-equipada com estado zerado nasce viva (HP/comida/longev).
		if mount, _ := equippedMount(p.Char); mount != nil {
			w.initFreshMount(mount)
		}
		w.recalcPlayer(p.Char)
		s.Send(wire.UpdateScore(p.ID, *p.Char))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.syncPlayerVitalsToObservers(p)
		w.syncCriaPet(p) // cria equipada nasce como pet; desequipada some
		// O 0x36B incremental atualiza dono e observadores sem carregar posicao.
		w.refreshAppearance(p)
	}
	log.Printf("[#%d] SWAP ok %d/%d <-> %d/%d", s.ID, st, sp, dt, dp)
}

func (w *World) validCargoAccess(p *Player, pkt []byte) bool {
	if p == nil || len(pkt) < 20 {
		return false
	}
	rawNPCID := binary.LittleEndian.Uint32(pkt[16:20])
	if rawNPCID > 0xFFFF {
		return false
	}
	npcID := uint16(rawNPCID)
	if npcID == 0 || npcID != p.CargoNPC {
		return false
	}
	m, err := w.resolveNPCInteraction(p, npcID)
	return err == nil && m.Def.Extended != nil && m.Def.Extended.Merchant&0xF == cargoMerchantType
}

func (w *World) nearCargoNPC(p *Player) bool {
	if p == nil || p.CargoNPC == 0 {
		return false
	}
	m, err := w.resolveNPCInteraction(p, p.CargoNPC)
	return err == nil && m.Def.Extended != nil && m.Def.Extended.Merchant&0xF == cargoMerchantType
}

func (w *World) onCargoGold(s *net.Session, pkt []byte, deposit bool) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld ||
		playerCurHP(p.Char) == 0 || len(pkt) != 16 || !w.nearCargoNPC(p) {
		return
	}
	amount := binary.LittleEndian.Uint32(pkt[12:16])
	if amount == 0 || amount > maxCharacterGold {
		return
	}
	if deposit {
		if p.Char.Gold < amount || p.Account.CargoGold > maxCharacterGold-amount {
			s.Send(wire.MessagePanel("That amount could not be deposited."))
			return
		}
		p.Char.Gold -= amount
		p.Account.CargoGold += amount
	} else {
		if p.Account.CargoGold < amount || p.Char.Gold > maxCharacterGold-amount {
			s.Send(wire.MessagePanel("That amount could not be withdrawn."))
			return
		}
		p.Account.CargoGold -= amount
		p.Char.Gold += amount
	}
	if err := w.saveAccount(p.Account); err != nil {
		if deposit {
			p.Char.Gold += amount
			p.Account.CargoGold -= amount
		} else {
			p.Account.CargoGold += amount
			p.Char.Gold -= amount
		}
		log.Printf("[#%d] salvar transferencia do Cargo: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. The transfer was not applied."))
		return
	}
	op := uint16(wire.OpWithdraw)
	if deposit {
		op = wire.OpDeposit
	}
	s.Send(wire.CargoGoldTransfer(op, wire.SceneField, amount))
	s.Send(wire.UpdateCargoGold(wire.SceneField, p.Account.CargoGold))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
}

// onUseNPC: 0x28B. Cliente clicou num NPC. Se for mercador, abre a loja mandando
// a lista (0x17C). NPC comum (Merchant==0) por ora nao faz nada.
// clickOkAbsent marca que o pacote 0x28B chegou curto demais para conter o campo
// ClickOk. Serve so para o log de instrumentacao nao mentir dizendo "0".
const clickOkAbsent = -1

// parseUseNPCRequest le o 0x28B, que e o mesmo pacote do _MSG_Quest nativo
// (139|FLAG_CLIENT2GAME). O npcID@12 ja era usado; o ClickOk@16 e o "confirm" do
// fluxo de quest. O campo so e lido quando o pacote realmente o traz (20 bytes),
// para nao regredir a abertura de loja, que hoje funciona com o layout curto.
func parseUseNPCRequest(pkt []byte) (npcID uint16, clickOk int32, ok bool) {
	if len(pkt) < 14 {
		return 0, clickOkAbsent, false
	}
	npcID = binary.LittleEndian.Uint16(pkt[12:14])
	clickOk = clickOkAbsent
	if len(pkt) >= 20 {
		clickOk = int32(binary.LittleEndian.Uint32(pkt[16:20]))
	}
	return npcID, clickOk, true
}

func (w *World) onUseNPC(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil {
		return
	}
	npcID, clickOk, ok := parseUseNPCRequest(pkt)
	if !ok {
		log.Printf("[#%d] 0x28B curto demais (%d bytes)", s.ID, len(pkt))
		return
	}
	w.cancelTrade(p, "interacao com NPC")
	// Ponto unico de validacao: existencia, visibilidade e alcance. Vale para
	// loja e quest -- antes disso nem a loja checava distancia.
	// Este handler atende DOIS opcodes com layouts diferentes, confirmado em
	// captura: 0x27B REQShopList tem 16 bytes (so TargetID@12) e 0x28B
	// UseNPC/Quest tem 20 (ClickOk@16). Logar o opcode real evita a confusao
	// de atribuir um tamanho ao pacote errado.
	opcode := int(wire.ParseHeader(pkt).Type)
	m, err := w.resolveNPCInteraction(p, npcID)
	if err != nil {
		log.Printf("[#%d] 0x%X recusado id=%d clickOk=%d: %v", s.ID, opcode, npcID, clickOk, err)
		s.Send(wire.MessagePanel(npcInteractionMessage(err)))
		return
	}
	log.Printf("[#%d] 0x%X npc=%q id=%d clickOk=%d dist=%d size=%d",
		s.ID, opcode, m.Def.Name, m.ID, clickOk, chebyshev(p.X, p.Y, m.X, m.Y), len(pkt))
	// Toda interacao abre exatamente um contexto autoritativo. Cargo nao pode
	// reutilizar um banqueiro antigo depois que o jogador clicou em outro NPC.
	p.ShopNPC, p.CraftNPC, p.CargoNPC = 0, 0, 0

	// Roteamento por ALLOWLIST, nesta ordem:
	//   1. tipos com handler proprio (loja, mestre, cargo);
	//   2. NPC explicitamente configurado em quests.json;
	//   3. qualquer outro -> resposta neutra.
	// "O resto vira quest" seria perigoso: capturaria crafting, ferreiro,
	// mestre de montaria e os tipos ainda nao classificados.
	//
	// 0x13 tambem possui low nibble 3. O mestre precisa ser testado antes da
	// loja generica, senao o client abre a janela normal e arrasta a skill para
	// o inventario em vez de aprende-la.
	// Composicao do Sefirot: o compositor E o mestre de skill da classe, entao
	// precisa ser testado ANTES da loja -- senao o clique sempre abre a janela
	// de skills e a composicao nunca acontece.
	//
	// Dois gates para nao sequestrar o clique de quem so quer comprar skill:
	// ClickOk=1 (o client so envia depois do "Sim" na janela de confirmacao) e
	// posse das oito pedras. Sem qualquer um deles, segue para a loja.
	if class, isMaster := sefirotClassForNPC(m.Def); isMaster && clickOk == 1 && hasSephiraStones(p.Char) {
		w.craftSefirot(s, p, class)
		return
	}
	if w.handleUxmalNPC(s, p, m) {
		return
	}
	if w.handleMountMasterNPC(s, p, m, clickOk) {
		return
	}
	if w.handleAbilityResetMasterNPC(s, p, m, clickOk) {
		return
	}
	if w.handleCarbMasterNPC(s, p, m) {
		return
	}
	if shopType, isShop := shopTypeForMerchant(m.Def.Extended.Merchant); isShop {
		p.ShopNPC = m.ID // lembra a loja aberta pro buy (server-authoritative)
		display := shopDisplayList(m.Def.Vende, shopType)
		s.Send(wire.ShopList(display, 0, shopType))
		if dropped := countShopItems(m.Def.Vende) - countShopItems(display); dropped > 0 {
			log.Printf("[#%d] loja %q: %d item(ns) alem do limite de %d do client",
				s.ID, m.Def.Name, dropped, clientShopSlots)
		}
		log.Printf("[#%d] loja aberta: %q (%d itens exibidos)", s.ID, m.Def.Name, countShopItems(display))
		return
	}
	if m.Def.Extended.Merchant&0x0F == craftingMerchant {
		// A janela e escolhida localmente pelo client a partir do rosto/posicao do
		// NPC. Guardamos apenas o contexto autoritativo para validar o opcode de
		// composicao que chegar depois.
		p.CraftNPC = m.ID
		log.Printf("[#%d] compositor aberto: %q", s.ID, m.Def.Name)
		return
	}
	if m.Def.Extended.Merchant&0x0F == cargoMerchantType {
		p.CargoNPC = m.ID
		log.Printf("[#%d] cargo aberto: %q", s.ID, m.Def.Name)
		return
	}
	if w.handleAscensionNPC(s, p, m) {
		return
	}
	// A ascensao vem ANTES do reino: com a Pedra e o Sefirot equipados o clique
	// no rei e pedido de Arch, nao de capa.
	if kingdomForNPC(m.Def) != model.KingdomNeutral && w.createArch(s, p) {
		return
	}
	if w.handleKingdomNPC(s, p, m) {
		return
	}
	if quest := w.questForNPC(m.Def); quest != nil {
		w.onQuestInteraction(s, p, m, quest, clickOk)
		return
	}
	if kind, reserved := reservedNPCKind(m.Def); reserved {
		// Tipo conhecido cuja janela o proprio client abre (cargo): nada a
		// fazer aqui, e sem mensagem para nao poluir a tela.
		log.Printf("[#%d] click em %q (%s, sem acao server-side)", s.ID, m.Def.Name, kind)
		return
	}
	log.Printf("[#%d] click em %q sem interacao configurada", s.ID, m.Def.Name)
	s.Send(wire.MessagePanel("This character has nothing for you right now."))
}

// onBuyItem: 0x379. Cliente comprou o item Vende[sellSlot] do mercador. Cobra o
// preco (do itemlist), adiciona no primeiro slot livre do inventario, confirma com
// 0x182 e atualiza o gold no client (0x337).
func (w *World) onBuyItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	// len(pkt) >= 16: le sellSlot@14 abaixo. Sem esta checagem um 0x379 curto
	// (forjado) causaria panic ao fatiar pkt[14:16].
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 || len(pkt) != 24 {
		return
	}
	w.cancelTrade(p, "compra em loja")
	// O client manda TargetID=0 no buy; a loja aberta vem do estado do servidor.
	// A excecao nativa e a janela de recompra: nela TargetID e o proprio
	// ClientID. O alvo/slot continuam sendo validados no fluxo separado.
	targetID := binary.LittleEndian.Uint16(pkt[12:14])
	sellSlot := binary.LittleEndian.Uint16(pkt[14:16]) // TargetCarryPos@14
	if targetID == p.ID {
		if p.ShopNPC == 0 {
			s.Send(wire.MessagePanel("Open a merchant before buying back an item."))
			return
		}
		if _, err := w.resolveNPCInteraction(p, p.ShopNPC); err != nil {
			s.Send(wire.MessagePanel("The merchant is no longer available."))
			return
		}
		w.onRebuyPurchase(s, p, pkt, sellSlot)
		return
	}
	if p.ShopNPC == 0 {
		log.Printf("[#%d] buy sem loja aberta", s.ID)
		return
	}
	m, interactionErr := w.resolveNPCInteraction(p, p.ShopNPC)
	if interactionErr != nil {
		log.Printf("[#%d] compra invalida loja=%d slot=%d", s.ID, p.ShopNPC, sellSlot)
		p.ShopNPC = 0
		return
	}
	// O slot vem da grade que o CLIENT desenhou, entao precisa ser resolvido
	// contra a MESMA lista enviada no 0x17C -- que e compactada para caber nas
	// 27 posicoes exibiveis. Indexar o Vende cru aqui compraria outro item.
	shopType, isShop := shopTypeForMerchant(m.Def.Extended.Merchant)
	if !isShop {
		log.Printf("[#%d] compra em npc que nao e loja: %q", s.ID, m.Def.Name)
		return
	}
	display := shopDisplayList(m.Def.Vende, shopType)
	index, ok := shopSlotFromClient(sellSlot)
	if !ok || index >= len(display) {
		log.Printf("[#%d] compra invalida loja=%d slot=%d -> indice=%d (lista tem %d)",
			s.ID, p.ShopNPC, sellSlot, index, len(display))
		return
	}
	it := display[index]
	if it.Index == 0 {
		return
	}
	// A compra no mestre e uma requisicao de aprendizado. O item 5000..5095
	// nunca existe fisicamente no inventario; onLearnSkill valida classe,
	// requisitos, pontos, gold e persiste a conta.
	if m.Def.Extended.Merchant == skillMasterMerchant {
		w.onLearnSkill(s, p, int(it.Index))
		return
	}
	def, exists := w.items[it.Index]
	if !exists {
		log.Printf("[#%d] compra rejeitada: item %d ausente do catalogo server-side", s.ID, it.Index)
		return
	}
	price := def.Price
	if p.Char.Gold < price {
		log.Printf("[#%d] gold insuficiente: item %d custa %d, tem %d", s.ID, it.Index, price, p.Char.Gold)
		return
	}
	dst := addToInv(p.Char, it)
	if dst < 0 {
		log.Printf("[#%d] inventario cheio, compra abortada", s.ID)
		return
	}
	oldGold := p.Char.Gold
	p.Char.Gold -= price
	// Persist-before-confirm: a compra move gold+item juntos; grava antes de
	// confirmar e reverte os dois se o disco falhar (disciplina do codebase).
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[dst] = model.Item{}
		p.Char.Gold = oldGold
		log.Printf("[#%d] ERRO ao salvar compra item=%d: %v", s.ID, it.Index, err)
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, byte(dst), p.Char.Inv[dst]))
	s.Send(wire.UpdateEtc(p.ID, *p.Char)) // refaz o display de gold
	log.Printf("[#%d] comprou item %d por %d gold -> inv[%d] (gold restante=%d)", s.ID, it.Index, price, dst, p.Char.Gold)
}

// onSellItem: 0x37A. Vende um item do inventario pro mercador aberto por
// 25% do preco de compra. Server-authoritative (usa p.ShopNPC, nao o TargetID).
func (w *World) onSellItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	// len(pkt) >= 17: le MyType@14 e MyPos@16. Sem isto um 0x37A curto forjado
	// causaria panic no acesso a pkt[16].
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 || len(pkt) != 20 {
		return
	}
	w.cancelTrade(p, "venda em loja")
	if p.ShopNPC == 0 {
		log.Printf("[#%d] venda sem loja aberta", s.ID)
		return
	}
	m, interactionErr := w.resolveNPCInteraction(p, p.ShopNPC)
	if interactionErr != nil {
		p.ShopNPC = 0
		return
	}
	if _, isShop := shopTypeForMerchant(m.Def.Extended.Merchant); !isShop {
		p.ShopNPC = 0
		return
	}
	myType := pkt[14] // MyType@14 (0=equip,1=inv)
	myPos := pkt[16]  // MyPos@16
	// A UI normal vende a partir do Carry. Aceitar Equip permitia forjar o
	// MyType e vender rosto/capa/montaria ou outros slots estruturais.
	if myType != placeInv {
		log.Printf("[#%d] venda rejeitada fora do inventario type=%d pos=%d", s.ID, myType, myPos)
		return
	}
	src := slotOf(p.Char, myType, myPos)
	if src == nil || src.Index == 0 {
		log.Printf("[#%d] venda invalida type=%d pos=%d", s.ID, myType, myPos)
		return
	}
	if _, filled := model.CelestialSealID(*src); filled {
		s.Send(wire.MessagePanel("A filled Spirit's Seal cannot be sold."))
		s.Send(wire.SendItem(p.ID, myType, myPos, *src))
		return
	}
	sold := src.Index
	def, exists := w.items[sold]
	price := uint32(0)
	if exists {
		price = def.Price / 4 // padrao WYD: vende por 25% do preco de compra
	}
	// Teto de gold: a venda credita gold e nao pode ultrapassar 2 bilhoes, como
	// toda entrada de gold no servidor. Sem isto era o unico credito sem teto.
	if p.Char.Gold > maxCharacterGold || price > maxCharacterGold-p.Char.Gold {
		s.Send(wire.MessagePanel("You cannot carry any more gold."))
		s.Send(wire.SendItem(p.ID, myType, myPos, *src)) // item intacto
		return
	}
	oldItem, oldGold, oldRebuy := *src, p.Char.Gold, p.Rebuy
	// O item vendido entra na lixeira somente depois de ter uma identidade
	// server-side. Saves antigos sem UID sao migrados no proprio movimento.
	if oldItem.UID == "" {
		var err error
		oldItem, err = materializeItem(oldItem)
		if err != nil {
			s.Send(wire.SendItem(p.ID, myType, myPos, *src))
			log.Printf("[#%d] venda rejeitada: UID do item %d nao pode ser materializado: %v", s.ID, sold, err)
			return
		}
	}
	*src = model.Item{} // esvazia o slot
	p.Char.Gold += price
	p.addRebuy(oldItem, price)
	// Persist-before-confirm: reverte item+gold se o disco falhar.
	if err := w.saveAccount(p.Account); err != nil {
		*src, p.Char.Gold, p.Rebuy = oldItem, oldGold, oldRebuy
		log.Printf("[#%d] ERRO ao salvar venda item=%d: %v", s.ID, sold, err)
		return
	}
	s.Send(wire.SendItem(p.ID, myType, myPos, *src)) // slot agora vazio
	s.Send(wire.UpdateEtc(p.ID, *p.Char))            // atualiza gold
	w.sendRebuyList(p)
	log.Printf("[#%d] vendeu item %d por %d gold (gold=%d)", s.ID, sold, price, p.Char.Gold)
}

// onMove: 0x366. PosX/Y@12 e Route[24] descrevem uma intencao de caminhada;
// TargetX/Y@24 e o destino visual. O pacote e replicado para a interpolacao do
// client, mas player_movement.go so promove os passos vencidos a autoridade.
func (w *World) onMove(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || playerCurHP(p.Char) == 0 || len(pkt) < 52 {
		return
	}
	w.cancelTrade(p, "jogador se movimentou")
	if binary.LittleEndian.Uint32(pkt[20:24]) == 6 {
		w.onIllusionMove(p, pkt)
		return
	}
	startX, startY, wireRoute, authorityRoute, ok := w.validatedPlayerMoveRoute(p, pkt)
	if !ok {
		w.recordSecurityViolation(s, wire.OpAction, "rota/destino de movimento invalido")
		return
	}
	x, y := actionTarget748(pkt)
	if x == 0 || y == 0 || len(authorityRoute) == 0 || samePlayerMovementDestination(p, x, y) {
		return
	}
	w.beginPlayerMovement(p, startX, startY, x, y, wireRoute, authorityRoute, w.now())
}

func actionTarget748(pkt []byte) (uint16, uint16) {
	if len(pkt) < 28 {
		return 0, 0
	}
	return binary.LittleEndian.Uint16(pkt[24:26]), binary.LittleEndian.Uint16(pkt[26:28])
}

func (w *World) onIllusionMove(p *Player, pkt []byte) {
	const illusionSkill = 73
	if p.Char.Class != 3 || p.Char.LearnedSkill&(1<<1) == 0 {
		return
	}
	skill, ok := w.skills[illusionSkill]
	if !ok {
		return
	}
	now := w.now()
	if p.SkillReady == nil {
		p.SkillReady = make(map[int]time.Time)
	}
	if now.Before(p.SkillReady[illusionSkill]) {
		return
	}
	targetX := binary.LittleEndian.Uint16(pkt[24:26])
	targetY := binary.LittleEndian.Uint16(pkt[26:28])
	if targetX == 0 || targetY == 0 || chebyshev(p.X, p.Y, targetX, targetY) > 8 ||
		!w.terrain.Walkable(targetX, targetY) {
		return
	}
	mastery := int(playerMastery(p.Char, 1))
	mana := skillManaCost(skill, mastery, int(effectiveExtended(p.Char).SaveMana))
	if playerCurMP(p.Char) < uint32(mana) {
		return
	}
	fromX, fromY := p.X, p.Y
	targetX, targetY = w.findFreePlayerPosition(targetX, targetY, 2, p)
	spendPlayerMP(p.Char, uint32(mana))
	p.SkillReady[illusionSkill] = now.Add(900 * time.Millisecond)
	p.X, p.Y = targetX, targetY
	p.Char.X, p.Char.Y = targetX, targetY
	w.refreshPlayerVisibility(p)
	speed := uint32(playerAttackRun(p.Char) & 0x0f)
	w.sendToPlayerView(p, func() []byte {
		return wire.IllusionMove(p.ID, fromX, fromY, targetX, targetY, speed)
	})
	w.syncPlayerVitals(p)
	log.Printf("[#%d] usou Illusion @(%d,%d)->(%d,%d) mp=-%d", p.Session.ID,
		fromX, fromY, targetX, targetY, mana)
}

// onActionStop trata MSG_Stop (0x367, 52B). Ele usa o mesmo prefixo de
// coordenadas do MSG_Action: PosX@12 e PosY@14.
func (w *World) onActionStop(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) < 52 {
		return
	}
	x := binary.LittleEndian.Uint16(pkt[12:14])
	y := binary.LittleEndian.Uint16(pkt[14:16])
	if !w.validReportedStop(p, x, y) {
		w.recordSecurityViolation(s, wire.OpActionStop, "Stop tentou reposicionar o personagem")
		return
	}
	w.publishPlayerStop(p)
	w.refreshPlayerVisibility(p)
}

// onMoveStop: MSG_MOVESTOP 0x2CB (36B). CurrentX/Y ficam em @20/@24; o client
// envia este pacote ao interromper uma caminhada para atacar.
func (w *World) onMoveStop(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 36 {
		return
	}
	x := binary.LittleEndian.Uint32(pkt[20:24])
	y := binary.LittleEndian.Uint32(pkt[24:28])
	if x == 0 || y == 0 || x > 65535 || y > 65535 ||
		!w.validReportedStop(p, uint16(x), uint16(y)) {
		w.recordSecurityViolation(s, wire.OpMoveStop, "MoveStop tentou reposicionar o personagem")
		return
	}
	w.publishPlayerStop(p)
	w.refreshPlayerVisibility(p)
}

// recallX/recallY sao o ponto seguro de Armia usado pelo renascimento e pelo
// reset de area de quest.
const recallX, recallY = uint16(2112), uint16(2088)

// recallPlayer recolhe o jogador para a cidade. E o servico UNICO do
// renascimento (onRestart) e do reset de area de quest, para nao divergirem.
//
// Ordem segura (o ponto sutil): o cadaver e visto pelos observadores da
// posicao ANTIGA; ele precisa ser descartado (RemoveMob type 3) ANTES de
// mover, senao a re-materializacao (que consulta a posicao NOVA) deixa o corpo
// orfao no ponto de morte. Depois revive, teleporta e recria vivo no destino.
func (w *World) recallPlayer(p *Player, reason string) bool {
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld {
		return false
	}
	// Um restart/recall tambem e uma saida da instancia. Sem esta liberacao,
	// o jogador morto continuava em MemberIDs e a limpeza posterior podia
	// teleporta-lo novamente para a sala antiga ou bloquear uma nova entrada.
	now := w.now()
	// Recall/restart is a definitive exit from a private Water room. Unlike a
	// socket logout, it must not leave a UID pending that would reattach the
	// character to the old room on the next login.
	w.detachPlayerFromItemInstancesMode(p.ID, now, false)
	dead := playerCurHP(p.Char) == 0
	if dead {
		for _, observer := range w.nearbyWorldPlayers(p.X, p.Y, viewHalfX) {
			if observer == p || observer.Session == nil || !observer.hasVisible(p.ID) {
				continue
			}
			observer.Session.Send(wire.RemoveMob(p.ID, 3))
			observer.hide(p.ID)
		}
		hp := uint32(200)
		if playerMaxHP(p.Char) < hp {
			hp = playerMaxHP(p.Char)
		}
		setPlayerCurHP(p.Char, hp)
		p.DeadAt = time.Time{}
	}
	p.X, p.Y = w.findFreePlayerPosition(recallX, recallY, 8, p)
	p.Char.X, p.Char.Y = p.X, p.Y
	clearPublishedPlayerMove(p)
	// A posicao segura persiste; falha de disco nao aborta o recall (a posicao
	// em RAM ja esta correta e o autosave a cobre em segundos).
	if err := w.saveAccount(p.Account); err != nil {
		log.Printf("[#%d] recall (%s): salvar posicao: %v", p.ID, reason, err)
	}
	if p.Session != nil {
		p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	}
	w.refreshPlayerVisibility(p)
	w.syncPlayerVitals(p)
	w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
	w.updatePartyMember(p)
	log.Printf("[#%d] recall (%s) -> Armia @(%d,%d) revivido=%v", p.ID, reason, p.X, p.Y, dead)
	return true
}

// onRestart implementa _MSG_Restart (0x289). O fluxo nativo repoe HP e executa
// DoRecall; delega ao recallPlayer, que centraliza o renascimento seguro.
func (w *World) onRestart(s *net.Session) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) != 0 {
		return
	}
	// O TMSrv bloqueia pedidos repetidos durante quatro segundos apos a morte.
	if !p.DeadAt.IsZero() && time.Since(p.DeadAt) < 4*time.Second {
		return
	}
	w.recallPlayer(p, "restart")
}

func (w *World) onSetShortSkill(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || len(pkt) < 32 {
		return
	}
	copy(p.Char.ShortSkill[:], pkt[12:32])
	filterShortSkills(p.Char)
	s.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
	log.Printf("[#%d] barra de skills atualizada: %v", s.ID, p.Char.ShortSkill)
}

// filterShortSkills separa a mascara autoritativa de skills aprendidas dos
// atalhos visuais. O client pode mandar indices globais (classe*24 + skill) ou
// locais; qualquer atalho sem bit correspondente em LearnedSkill e removido.
func filterShortSkills(ch *model.Char) {
	for i, value := range ch.ShortSkill {
		if value == 0xFF {
			continue
		}
		skill := int(value)
		classBase := int(ch.Class) * 24
		if skill >= classBase && skill < classBase+24 {
			skill -= classBase
		}
		if specialSkillLearned(ch, skill) {
			continue
		}
		if skill < 0 || skill >= 24 || ch.LearnedSkill&(1<<skill) == 0 {
			ch.ShortSkill[i] = 0xFF
		}
	}
}

// ChangeCity informa o village em @12. No modelo atual basta registrar a posicao
// autoritativa como ponto persistido; regras de reino/cidade entram depois.
func (w *World) onChangeCity(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || len(pkt) < 16 {
		return
	}
	village := binary.LittleEndian.Uint32(pkt[12:16])
	if village > 4 {
		return
	}
	p.Char.X, p.Char.Y = p.X, p.Y
	log.Printf("[#%d] ChangeCity village=%d home=(%d,%d)", s.ID, village, p.X, p.Y)
}

// onSysQuit trata o 0x3AE. ATENCAO: no client 7.48 esse opcode e SOBRECARREGADO
// -- e MSG_DelayStart E MSG_SysQuit (Basedef.h). O client manda em TELEPORTE e
// MORTE (DelayStart de carregar a cena), nao so ao sair. Por isso NAO tratamos
// como saida: nao fechamos loja fantasma nem cancelamos trade (a loja so fecha
// ao abrir outra ou desconectar; o quit real e o socket caindo -> onDisconnect).
// So sincronizamos/gravamos a posicao e devolvemos o ack.
func (w *World) onSysQuit(s *net.Session) {
	p := w.players[s]
	if p == nil || p.Char == nil {
		return
	}
	p.Char.X, p.Char.Y = p.X, p.Y
	if err := w.saveAccount(p.Account); err != nil {
		log.Printf("[#%d] erro ao salvar no DelayStart(0x3AE): %v", s.ID, err)
		return
	}
	s.Send(wire.SysQuit(p.ID))
	log.Printf("[#%d] DelayStart(0x3AE) ack (teleporte/morte/saida)", s.ID)
}

// attackRange = alcance de melee em tiles (distancia de Chebyshev).
const attackRange = 3

// physicalAttackRange porta o alcance de ataque normal do _MSG_Attack: armas
// de alcance usam exclusivamente o EF_RANGE da arma principal (slot 6), com
// os dois tiles de base e a margem nativa de tres. Um arco EF_RANGE=5 atinge
// ate 10 tiles; ataque sem arma de alcance permanece melee (3).
func (w *World) physicalAttackRange(ch *model.Char) int {
	if ch == nil || len(ch.Equip) <= 6 {
		return attackRange
	}
	weapon := ch.Equip[6]
	def, ok := w.items[weapon.Index]
	if !ok || weapon.Index == 0 {
		return attackRange
	}
	weaponRange := itemAbility(weapon, def, "EF_RANGE")
	if weaponRange <= 0 {
		return attackRange
	}
	extra := 0
	if specialSkillLearned(ch, 101) {
		extra = 1
	}
	return minInt(23, 2+weaponRange+3+extra)
}

// isLearnedClassSkill separa uma execucao real de skill de um ataque normal.
// No wire do 7.48 o campo SkillId e sempre transmitido. Ataques de arco podem
// deixa-lo com um valor nao-negativo embora nao representem uma skill; enviar
// todo valor >= 0 para onSkillAttack faria o golpe ser descartado como "skill
// nao aprendida". A unica forma de um pacote entrar no caminho de skills e o
// indice pertencer a classe do personagem e estar aprendido no servidor.
func isLearnedClassSkill(ch *model.Char, skillIndex int) bool {
	if ch == nil || skillIndex < 0 {
		return false
	}
	if specialSkillLearned(ch, skillIndex) {
		return true
	}
	local := skillIndex - int(ch.Class)*24
	return local >= 0 && local < 24 && ch.LearnedSkill&(uint32(1)<<local) != 0
}

func specialSkillBit(skillIndex int) (uint, bool) {
	if skillIndex < 97 || skillIndex > 102 {
		return 0, false
	}
	return uint(skillIndex - 72), true
}

func specialSkillLearned(ch *model.Char, skillIndex int) bool {
	bit, ok := specialSkillBit(skillIndex)
	return ch != nil && ok && ch.LearnedSkill&(uint32(1)<<bit) != 0
}

func spectralPacket(ch *model.Char, packet []byte) []byte {
	if specialSkillLearned(ch, 101) {
		return wire.SpectralVisual(packet)
	}
	return packet
}

// attackIntervalFor deriva o intervalo minimo entre golpes da VELOCIDADE DE
// ATAQUE server-side (nibble alto de AttackRun, 0..15). Char mais rapido ataca
// mais vezes; como o piso usa a velocidade REAL do servidor, um speedhack no
// client nao consegue baixa-lo. Numeros abaixo sao para ajuste fino in-game:
// ~900ms na velocidade 0 caindo ate ~450ms na 15 (~2.2 golpes/s no maximo).
func attackIntervalFor(ch *model.Char) time.Duration {
	speed := 0
	if e := effectiveExtended(ch); e != nil {
		speed = int(e.AttackRun >> 4)
	}
	speed = clampInt(speed, 0, 15)
	const slowMs, fastMs = 900, 450
	return time.Duration(slowMs-(slowMs-fastMs)*speed/15) * time.Millisecond
}

func acceptClientAttack(p *Player, pkt []byte, now time.Time) bool {
	if p == nil || len(pkt) < 12 {
		return false
	}
	tick := binary.LittleEndian.Uint32(pkt[8:12])
	// SKIPCHECKTICK pertence exclusivamente a ataques gerados pelo servidor.
	if tick == 0 || tick == 0x0E0A1ACA {
		return false
	}
	interval := attackIntervalFor(p.Char)
	if p.LastAttackTick != 0 {
		// Subtracao unsigned preserva o wrap natural de GetTickCount, mas um tick
		// simplesmente retrocedendo nao pode virar um delta gigantesco valido.
		if tick < p.LastAttackTick && p.LastAttackTick-tick < 0x80000000 {
			return false
		}
		if tick-p.LastAttackTick < uint32(interval/time.Millisecond) {
			return false
		}
	}
	if !p.LastAttackAt.IsZero() && now.Sub(p.LastAttackAt) < interval {
		return false
	}
	p.LastAttackTick = tick
	p.LastAttackAt = now
	return true
}

// onAttack: 0x39D/0x367/0x39E. O pacote fornece apenas a intenção de alvo por
// TargetID. O servidor resolve esse ID e revalida existência, gameplay space,
// distância, terreno/linha de visão e demais requisitos antes de aplicar o dano.
// Dano vem de combat.go; em HP 0 o mob morre e agenda respawn.
func (w *World) onAttack(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld {
		return
	}
	req := parseAttackSkill(pkt)
	if playerCurHP(p.Char) == 0 && !(req.Skill == 99 && specialSkillLearned(p.Char, 99)) {
		return
	}
	w.cancelTrade(p, "jogador atacou")
	// Anti-speed compartilhado: ataque fisico basico E skills respeitam o MESMO
	// limite de ataques por periodo (acceptClientAttack, por velocidade de ataque
	// do servidor). Por isso ele vem ANTES do dispatch de skill.
	now := w.now()
	if !acceptClientAttack(p, pkt, now) {
		return
	}
	// SkillId@24 so identifica uma skill se ela foi realmente aprendida pela
	// classe. O cliente 7.48 tambem preenche esse campo em variantes do ataque
	// fisico de alcance; nesses casos o golpe precisa continuar aqui, nao virar
	// uma tentativa silenciosamente rejeitada de usar skill.
	if isLearnedClassSkill(p.Char, req.Skill) {
		w.onSkillAttack(p, req)
		return
	}
	// O alvo vem em @44 no layout compacto 7.48. Dano, posicao do atacante e
	// demais valores enviados pelo client nunca sao aceitos como autoridade.
	maxRange := w.physicalAttackRange(p.Char)
	m := w.mobByID(req.TargetID)
	if m == nil {
		target := w.playerByID(req.TargetID)
		if target == nil || target == p || !target.InWorld || target.Char == nil ||
			playerCurHP(target.Char) == 0 || sameSupportGroup(p, target) ||
			!w.playersShareGameplaySpace(p, target) ||
			chebyshev(p.X, p.Y, target.X, target.Y) > maxRange ||
			!w.combatLineOfSight(p.X, p.Y, target.X, target.Y) {
			return
		}
		if req.TargetX != 0 && req.TargetY != 0 &&
			chebyshev(req.TargetX, req.TargetY, target.X, target.Y) > 1 {
			return
		}
		w.breakHideOnAttack(p)
		w.cancelTrade(target, "personagem foi atacado")
		p.CombatTargetID = target.ID
		target.LastAttackerID = p.ID

		calculated := w.playerHitsPlayer(p, target)
		if calculated == 0 {
			log.Printf("[#%d] errou ataque PvP em %q", s.ID, target.Char.Name)
			return
		}
		calculated = addFlatDamage(calculated, w.equipmentGemBonuses(p.Char).forceDamage)
		calculated = absorbFlatDamage(calculated, w.equipmentGemBonuses(target.Char).absorbDamage)
		// Montaria adulta viva do alvo absorve 25% do dano no proprio HP.
		calculated = uint32(w.absorbMountDamage(target, int(calculated)))
		applied := minU32(calculated, playerCurHP(target.Char))
		setPlayerCurHP(target.Char, playerCurHP(target.Char)-applied)
		lethal := playerCurHP(target.Char) == 0
		if lethal {
			w.cancelTrade(target, "personagem morreu")
			w.mountRiderDied(target)
			target.DeadAt = now
			w.receiveDeathLetter(target, p.Char.Name, "jogador")
			w.applyPvPKills(p, target)
		}

		// O numero flutuante recebe o dano calculado integral, inclusive
		// overkill. A vida autoritativa continua reduzida somente pelo HP real.
		w.sendToPlayerView(target, func() []byte {
			return spectralPacket(p.Char, wire.AttackHitExtended(p.ID, target.ID, p.X, p.Y, target.X, target.Y,
				calculated, playerMaxHP(target.Char), p.Char.Exp, playerCombatMP(p.Char)))
		})
		w.syncPlayerVitals(target)
		w.updatePartyMember(target)
		if lethal {
			w.publishPlayerDeath(target, p.ID)
		}
		log.Printf("[#%d] ataque PvP em %q dmg_calculado=%d aplicado=%d hp=%d",
			s.ID, target.Char.Name, calculated, applied, playerCurHP(target.Char))
		return
	}
	if m == nil || !w.playerCanInteractWithMob(p, m) ||
		chebyshev(p.X, p.Y, m.X, m.Y) > maxRange ||
		!w.combatLineOfSight(p.X, p.Y, m.X, m.Y) {
		return
	}
	if req.TargetX != 0 && req.TargetY != 0 && chebyshev(req.TargetX, req.TargetY, m.X, m.Y) > 1 {
		return
	}
	w.breakHideOnAttack(p)
	p.CombatTargetID = m.ID
	// Assim como SetBattle no TMSrv, ser atacado coloca o mob imediatamente em
	// combate mesmo antes de sua proxima varredura de inimigos visiveis.
	m.TargetID = p.ID
	dmg := w.playerHitsMob(p, m)
	if dmg == 0 {
		log.Printf("[#%d] errou ataque no mob id=%d %q (accuracy=%d)", s.ID, m.ID, m.Def.Name, effectiveExtended(p.Char).Accuracy)
		return
	}
	dmg = uint32(applyCouragePvEDamageAt(p.Char, int(dmg), false, now))
	dmg = addFlatDamage(dmg, w.equipmentGemBonuses(p.Char).forceDamage)
	// Escudo de boss absorve ANTES de aplicar, para que o numero flutuante do
	// client mostre o dano real (zero quando imune).
	dmg = w.bossMitigateDamage(m, dmg)
	oldHP := m.HP
	if dmg >= m.HP {
		m.HP = 0
	} else {
		m.HP -= dmg
	}
	// Gancho do subsistema de boss: para mob comum e so uma consulta de mapa.
	w.notifyMobDamaged(m, oldHP, p.ID, dmg)
	// O 0x181 atualiza a barra, mas somente o resultado 0x39D produz animacao e
	// o numero flutuante do dano no client 7.48.
	// Instance mobs share the map coordinates with the public world, but their
	// hit animation and damage must remain private to the current stage members.
	// Sending this through broadcast leaked combat packets to outsiders (and
	// let a client observe an encounter it could not target).
	w.sendToMobView(m, func() []byte {
		return spectralPacket(p.Char, wire.AttackHitExtended(p.ID, m.ID, p.X, p.Y, m.X, m.Y,
			dmg, m.Def.Extended.MaxHP, p.Char.Exp, playerCombatMP(p.Char)))
	})
	if m.HP == 0 {
		w.killMobState(p, m, dmg, minU32(dmg, m.Def.Extended.MaxHP))
	} else {
		// golpe nao-fatal: so baixa a barra de HP.
		w.sendToMobView(m, func() []byte {
			return wire.SetMobHpMp(m.ID, m.HP, m.Def.Extended.MaxHP,
				m.Def.Extended.MaxMP, m.Def.Extended.MaxMP)
		})
		log.Printf("[#%d] atacou mob id=%d %q dmg=%d hp=%d/%d",
			s.ID, m.ID, m.Def.Name, dmg, m.HP, m.Def.Extended.MaxHP)
	}
}

// killMobState processa a morte de um monstro, xp, respawn e pacotes de morte.
// chebyshev = max(|dx|,|dy|), a metrica de distancia do WYD.
func chebyshev(ax, ay, bx, by uint16) int {
	dx, dy := absDiff(ax, bx), absDiff(ay, by)
	if dy > dx {
		return dy
	}
	return dx
}

func absDiff(a, b uint16) int {
	if a > b {
		return int(a - b)
	}
	return int(b - a)
}

// pickupRange = alcance (Chebyshev) pra pegar item do chao.
const pickupRange = 4

type dropItemRequest struct {
	srcType, srcPos byte
	rotate          uint32
}

func parseDropItemRequest(pkt []byte) (dropItemRequest, bool) {
	if len(pkt) < 32 {
		return dropItemRequest{}, false
	}
	srcType := binary.LittleEndian.Uint32(pkt[12:16])
	srcPos := binary.LittleEndian.Uint32(pkt[16:20])
	// O servidor nativo nao permite jogar equipamento diretamente no chao.
	// Cargo ainda nao existe no wyd-go; por enquanto somente Carry/Inv e valido.
	if srcType != placeInv || srcPos >= model.PlayerCarrySlots {
		return dropItemRequest{}, false
	}
	return dropItemRequest{srcType: byte(srcType), srcPos: byte(srcPos),
		rotate: binary.LittleEndian.Uint32(pkt[20:24])}, true
}

type getItemRequest struct {
	destType, destPos byte
	itemID            uint16
}

func parseGetItemRequest(pkt []byte) (getItemRequest, bool) {
	if len(pkt) < 28 {
		return getItemRequest{}, false
	}
	destType := binary.LittleEndian.Uint32(pkt[12:16])
	destPos := binary.LittleEndian.Uint32(pkt[16:20])
	if destType != placeInv || destPos >= model.PlayerCarrySlots {
		return getItemRequest{}, false
	}
	return getItemRequest{destType: byte(destType), destPos: byte(destPos),
		itemID: binary.LittleEndian.Uint16(pkt[20:22])}, true
}

// onDropItem: 0x272 MSG_DropItem (SourType i32@12, SourPos i32@16, Rotate@20,
// GridXY@24/26). Jogar item no chao: remove do slot (0x182 confirma o vazio) e
// materializa um GroundItem na posicao do player, com os EFEITOS preservados.
func (w *World) onDropItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	w.cancelTrade(p, "item jogado no chao")
	req, ok := parseDropItemRequest(pkt)
	if !ok {
		log.Printf("[#%d] pacote de drop invalido", s.ID)
		return
	}
	src := slotOf(p.Char, req.srcType, req.srcPos)
	if src == nil || src.Index == 0 {
		log.Printf("[#%d] drop invalido type=%d pos=%d", s.ID, req.srcType, req.srcPos)
		return
	}
	item := *src
	if _, filled := model.CelestialSealID(item); filled {
		s.Send(wire.MessagePanel("A filled Spirit's Seal cannot be dropped."))
		return
	}
	// Reserva no mundo sem publicar. O item somente aparece aos clientes depois
	// que a remocao autoritativa do inventario estiver persistida.
	g := w.createGroundDropForInstance(p.X, p.Y, item, false, w.gameplaySpaceForPlayer(p))
	if g == nil {
		return
	}
	*src = model.Item{}
	// Fecha a janela cross-player do autosave: outro jogador pode coletar e
	// persistir o item antes de a conta do dono salvar a remocao.
	if err := w.saveAccount(p.Account); err != nil {
		*src = item
		w.unregisterGroundItem(g)
		log.Printf("[#%d] drop cancelado por falha ao salvar: %v", s.ID, err)
		return
	}
	w.publishItemSpawn(g)
	// 0x175 faz o client destacar o item do cursor e limpar o slot. O 0x182
	// confirma o mesmo estado autoritativo, como nas demais mutacoes de Carry.
	s.Send(wire.CNFDropItem(uint32(req.srcType), uint32(req.srcPos), req.rotate, g.X, g.Y))
	s.Send(wire.SendItem(p.ID, req.srcType, req.srcPos, *src))
	log.Printf("[#%d] jogou item %d no chao @(%d,%d)", s.ID, item.Index, g.X, g.Y)
}

// onGetItem: 0x270 MSG_GetItem (DestType i32@12, DestPos i32@16, ItemID u16@20,
// GridXY@22/24). Pegar item do chao: valida alcance, tenta o slot de destino que o
// client pediu (se livre), senao o primeiro livre; confirma com 0x182 e remove o
// GroundItem pra todos que o viam.
func (w *World) onGetItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 {
		return
	}
	w.cancelTrade(p, "item coletado")
	req, ok := parseGetItemRequest(pkt)
	if !ok {
		log.Printf("[#%d] pacote de coleta invalido", s.ID)
		return
	}
	g := w.groundItems[req.itemID]
	if g == nil {
		return // ja pego por outro / expirou
	}
	if !w.groundItemVisibleToPlayer(p, g) {
		log.Printf("[#%d] tentou recolher item %d de outra instancia", s.ID, req.itemID)
		return
	}
	// Objeto de mundo e mobilia do mapa, nao loot: sem esta guarda o jogador
	// caminha ate um portao de castelo e o poe no inventario.
	if g.Permanent {
		log.Printf("[#%d] tentou recolher o objeto de mundo %d (item %d)",
			s.ID, g.ID, g.Item.Index)
		return
	}
	if chebyshev(p.X, p.Y, g.X, g.Y) > pickupRange {
		log.Printf("[#%d] pegar item %d fora de alcance", s.ID, req.itemID)
		return
	}
	if def, exists := w.items[g.Item.Index]; exists {
		if rule, code, registered := w.volatiles.Rule(g.Item.Index); registered && rule.Action == "ground_gold" {
			value := volatileGoldValue(rule, g.Item, def)
			if value == 0 || p.Char.Gold > maxCharacterGold || value > maxCharacterGold-p.Char.Gold {
				log.Printf("[#%d] moeda item=%d volatile=%d rejeitada: valor=%d gold=%d",
					s.ID, g.Item.Index, code, value, p.Char.Gold)
				return
			}
			oldGold := p.Char.Gold
			p.Char.Gold += value
			if err := w.saveAccount(p.Account); err != nil {
				p.Char.Gold = oldGold
				log.Printf("[#%d] coleta de moeda cancelada por falha ao salvar: %v", s.ID, err)
				return
			}
			w.publishItemRemove(g)
			w.unregisterGroundItem(g)
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			log.Printf("[#%d] coletou moeda item=%d gold=+%d total=%d", s.ID, g.Item.Index, value, p.Char.Gold)
			return
		}
	}
	slot := -1
	if req.destType == placeInv { // honra o slot pedido pelo client, se livre
		if dst := slotOf(p.Char, placeInv, req.destPos); dst != nil && dst.Index == 0 {
			*dst = g.Item
			slot = int(req.destPos)
		}
	}
	if slot < 0 {
		slot = addToInv(p.Char, g.Item)
	}
	if slot < 0 {
		log.Printf("[#%d] pegar item %d: inventario cheio", s.ID, req.itemID)
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[slot] = model.Item{}
		log.Printf("[#%d] coleta cancelada por falha ao salvar: %v", s.ID, err)
		return
	}
	w.publishItemRemove(g)
	w.unregisterGroundItem(g)
	// 0x171 e a confirmacao nativa da coleta; 0x182 fixa o slot autoritativo.
	s.Send(wire.CNFGetItem(uint32(placeInv), uint32(slot)))
	s.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
	log.Printf("[#%d] pegou item %d do chao -> inv[%d]", s.ID, g.Item.Index, slot)
}

// onDisconnect: salva a conta e remove o player do mundo. So o loop mexe no mapa
// (zero mutex). O save e sincrono aqui -- OK porque desconexao e raro e o arquivo
// e pequeno; se virar multiplayer, mover pra fora do loop.
func (w *World) onDisconnect(s *net.Session) {
	delete(w.authPending, s)
	delete(w.security, s)
	if p, ok := w.players[s]; ok {
		// Conta e charstate formam um unico aggregate no PostgreSQL. No
		// disconnect fisico nao ha resposta a segurar, mas uma falha precisa
		// envenenar a RAM para impedir um save posterior incompleto.
		if p.InWorld && p.Account != nil && p.Char != nil && !p.PersistencePoisoned {
			if err := w.saveAccountAndCharStateResult(p); err != nil {
				p.PersistencePoisoned = true
				log.Printf("[#%d] ERRO ao salvar estado completo de %q: %v", s.ID, p.Account.Name, err)
			}
		} else if p.Account != nil && !p.PersistencePoisoned {
			if err := w.saveAccount(p.Account); err != nil {
				p.PersistencePoisoned = true
				log.Printf("[#%d] ERRO ao salvar conta %q: %v", s.ID, p.Account.Name, err)
			}
		}
		w.detachPlayerFromItemInstances(p.ID, w.now())
		// Persist the UID-based Water membership immediately. Waiting for the
		// next autosave would make a clean disconnect look like a lost room after
		// a process restart.
		w.flushInstanceStateIfDirty()
		w.unregisterPlayerSpatial(p)
		w.closeGhostShop(p, "desconexao")
		w.cancelTrade(p, "desconexao")
		// Remove primeiro o grupo e a entidade visivel. O client nao pode manter
		// personagem/party fantasma depois que o socket caiu.
		w.removePartyPlayer(p)
		// Evocacoes pertencem ao dono: despawnam na desconexao, senao ficariam
		// orfas e seguiriam o proximo player a reusar este ID.
		w.removePlayerSummons(p.ID)
		for _, other := range w.players {
			if other != p && other.InWorld && other.hasVisible(p.ID) {
				other.Session.Send(wire.RemoveMob(p.ID, 0))
				other.hide(p.ID)
			}
		}
		w.releaseAccountSession(s, p.Account)
		w.unindexPlayerCharacter(p)
		delete(w.playersByID, p.ID)
		delete(w.players, s)
	}
	log.Printf("[#%d] player removido", s.ID)
}

func accountSessionKey(name string) string {
	return strings.ToLower(strings.TrimSpace(name))
}

// claimAccountSession roda apenas no loop do World, portanto a verificacao e a
// reserva sao atomicas sem mutex. So e chamada depois da senha correta para nao
// permitir enumerar externamente quais contas estao online.
func (w *World) claimAccountSession(s *net.Session, name string) bool {
	if w.accountSessions == nil {
		w.accountSessions = make(map[string]*net.Session)
	}
	key := accountSessionKey(name)
	if active := w.accountSessions[key]; active != nil && active != s {
		// Em carga alta, o socket pode terminar antes de o comando nil de
		// desconexão chegar ao loop do World. Não mantenha a conta presa até o
		// backlog ser drenado: a reserva do socket morto já não protege ninguém.
		if active.IsClosed() {
			delete(w.accountSessions, key)
		} else {
			return false
		}
	}
	w.accountSessions[key] = s
	return true
}

func (w *World) releaseAccountSession(s *net.Session, acc *model.Account) {
	if acc == nil || w.accountSessions == nil {
		return
	}
	key := accountSessionKey(acc.Name)
	if w.accountSessions[key] == s {
		delete(w.accountSessions, key)
	}
}

// characterNameTaken checa a unicidade do nome pelo indice em memoria quando ele
// existe (O(1), sem tocar o disco). Sem o indice, cai no scan do store legado.
func (w *World) characterNameTaken(name string) (bool, error) {
	if w.charNames != nil {
		_, taken := w.charNames[strings.ToLower(name)]
		return taken, nil
	}
	return w.store.CharacterNameExists(name)
}

// mobByID acha o mob (NPC/monstro) vivo pelo ClientId; nil se nao existir.
func (w *World) mobByID(id uint16) *Mob {
	m := w.mobsByID[id]
	if m == nil || m.Dead {
		return nil
	}
	return m
}

// addToInv poe o item no primeiro slot VISIVEL do inventario. O array/wire tem
// 64 entradas, mas o indice 63 nao possui celula na UI do client 7.48.
func addToInv(ch *model.Char, it model.Item) int {
	it, err := materializeItem(it)
	if err != nil {
		return -1
	}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		if ch.Inv[i].Index == 0 {
			ch.Inv[i] = it
			return i
		}
	}
	return -1
}

// slotOf resolve (type,pos) -> ponteiro pro item no char; nil se invalido.
func slotOf(ch *model.Char, typ, pos byte) *model.Item {
	switch typ {
	case placeEquip:
		if int(pos) < len(ch.Equip) {
			return &ch.Equip[pos]
		}
	case placeInv:
		if int(pos) < model.PlayerCarrySlots {
			return &ch.Inv[pos]
		}
	}
	return nil
}

func playerSlotOf(p *Player, typ, pos byte) *model.Item {
	if p == nil || p.Char == nil {
		return nil
	}
	if typ == placeStorage {
		if p.Account != nil && int(pos) < model.PlayerCargoSlots {
			return &p.Account.Cargo[pos]
		}
		return nil
	}
	return slotOf(p.Char, typ, pos)
}

// affectFaceTransform e o affect COSMETICO da transformacao de rosto (volatiles
// 70-77). Value = mesh do monstro; sem efeito de combate (diferente do affect 16
// da transformacao BM). face_restore (89) remove.
const affectFaceTransform = 40

// bodyMesh extrai os 16 indices visuais dos slots de equip (ItemEff do CreateMob).
// bodyAncient devolve o AnctCode (cor/refino antigo) de cada slot de equip, para
// o campo anct@130 do CreateMob. Sem ele a cor da tintura nao viaja para o mundo
// (o mesh so leva o refino). Montaria nao entra (seus bytes de estado nao sao
// codigo de sanc).
func bodyAncient(ch *model.Char) []byte {
	if ch == nil {
		return nil
	}
	a := make([]byte, len(ch.Equip))
	for i, it := range ch.Equip {
		if !model.IsMount(it.Index) {
			a[i] = model.AncientCode(it)
		}
	}
	return a
}

func bodyMesh(ch *model.Char) []uint16 {
	return bodyMeshAt(ch, time.Now())
}

func bodyMeshAt(ch *model.Char, now time.Time) []uint16 {
	if ch == nil {
		return nil
	}
	m := make([]uint16, len(ch.Equip))
	for i, it := range ch.Equip {
		m[i] = model.VisualItemCode(it, model.IsMount(it.Index))
	}
	faces := [...]uint16{22, 23, 24, 25, 32}
	for i := range ch.Affects {
		a := &ch.Affects[i]
		// Transformacao de rosto de monstro (cosmetica): o mesh vem no Value.
		if a.Type == affectFaceTransform && a.ExpiresAt.After(now) && a.Value > 0 {
			m[0] = uint16(a.Value)
			break
		}
		if a.Type == 16 && a.ExpiresAt.After(now) && a.Value >= 1 && a.Value <= len(faces) {
			m[0] = faces[a.Value-1]
			break
		}
	}
	return m
}

// cstr le uma C-string (ate o primeiro NUL) de um slice.
func cstr(b []byte) string {
	for i := range b {
		if b[i] == 0 {
			return string(b[:i])
		}
	}
	return string(b)
}
