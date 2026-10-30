package game

import (
	"context"
	"encoding/binary"
	"errors"
	"log"
	"math/rand/v2"
	"strconv"
	"strings"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// SwapItem storage-location types (7.54 decompilation reference).
const (
	placeEquip   = 0
	placeInv     = 1
	placeStorage = 2
)

// shopMerchant is the low Merchant nibble recognized as a gold shop by the
// 7.48 client (m_stScore.Reserved & 0xF == 3 in the click handler).
const shopMerchant = 3
const skillMasterMerchant = 19
const nativeShopMerchant = 1
const craftingMerchant = 8

// clientShopSlots is the 7.48 shop display limit: OnPacketShopList iterates
// `for (i = 0; i < 27; ++i)` and builds the grid at (i%5, i/5).
// Packet entries beyond the first 27 are never drawn.
const clientShopSlots = 27

// shopDisplayList returns the sale list as displayed by the client.
//
// Converted WYD 7.48 NPCs spread stock across the original 64-slot Inventory
// with gaps: Aki has items in 1..7, 27..31, and 33..35. The client reads only
// the first 27 entries. Compact the list while preserving order so later
// entries remain visible.
//
// Skill shops are the exception: empty entries 8, 17, and 26 separate the
// three branches and must be retained to preserve the master's grid.
//
// This is the single source for both 0x17C and purchase-slot resolution
// (0x379). Compacting only the display would purchase the wrong item.
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

// shopSlotFromClient maps the position sent in 0x379 to the list index sent
// in 0x17C.
//
// The client remaps the grid index before sending (SGrid.cpp,
// SGridControl::BuyItem):
//
//	SourPos = cellX + 5*cellY          // = i, the displayed list index
//	SourPos = SourPos % 9 + 27 * (SourPos / 9)
//
// The native layout has nine-entry blocks spaced by 27 (0..8, 27..35,
// 54..62), matching the skill master's 27 slots and WYD 7.48 NPC stock.
// Indices 0..8 map to themselves, so early successful purchases hid the bug.
//
// Inverse: block = S/27, position = S%27 (must be < 9), index = position + 9*block.
func shopSlotFromClient(sourPos uint16) (int, bool) {
	block := int(sourPos) / 27
	pos := int(sourPos) % 27
	if pos >= 9 {
		// The gap between blocks does not correspond to a grid cell.
		return 0, false
	}
	return pos + 9*block, true
}

// countShopItems counts occupied entries; the raw WYD 7.48 list has gaps,
// so len() does not measure the number of items actually offered.
func countShopItems(vende []model.Item) int {
	count := 0
	for _, item := range vende {
		if item.Index != 0 {
			count++
		}
	}
	return count
}

func shopTypeForMerchant(merchant uint32) (uint32, bool) {
	if merchant == skillMasterMerchant {
		return wire.ShopSkill, true
	}
	if merchant == nativeShopMerchant || merchant&0xF == shopMerchant {
		return wire.ShopNormal, true
	}
	return 0, false
}

// onLogin: 0x20D. Load the account from storage and send character selection.
func (w *World) onLogin(s *net.Session, pkt []byte) {
	if len(pkt) < 44 || s.IsClosed() || s.AdmissionRejected() || w.authPending[s] || w.players[s] != nil {
		return
	}
	// After the final shutdown snapshot, admitting a login would create state
	// that cannot be persisted.
	if w.shuttingDown || w.shutdownLoginsBlocked(w.now()) {
		s.Send(wire.MessagePanel("Server going into maintenance. Try again shortly."))
		return
	}
	accountName := cstr(pkt[12:28])
	password := cstr(pkt[28:40])
	cliver := binary.LittleEndian.Uint32(pkt[40:44])
	var clientBuild uint32
	if len(pkt) >= 48 {
		clientBuild = binary.LittleEndian.Uint32(pkt[44:48])
	}
	if reason, rejected := w.networkRejected(s); rejected {
		log.Printf("[#%d] LOGIN rejected before authentication ip=%q: network policy (%s)",
			s.ID, s.RemoteIP(), reason)
		s.Send(wire.MessagePanel("Connections from this network are not allowed."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	if !wire.CompatibleClientLogin(pkt) {
		log.Printf("[#%d] LOGIN rejected before authentication: incompatible client protocol=%d build=0x%08X minimum_build_revision=%d",
			s.ID, cliver, clientBuild, wire.MinimumClientBuildRevision)
		s.RejectAdmission(wire.MessagePanel("Client update required. Close the game and run the launcher to update."))
		return
	}
	if !w.allowLoginAttempt(s.RemoteIP(), accountName, w.now()) {
		log.Printf("[#%d] LOGIN rate limited account=%q ip=%q", s.ID, accountName, s.RemoteIP())
		s.Send(wire.MessagePanel("Too many login attempts. Try again later."))
		return
	}
	log.Printf("[#%d] LOGIN account=%q cliver=%d build_revision=%d password_len=%d", s.ID, accountName, cliver,
		clientBuild&0xFFFF, len(password))
	select {
	case w.authSlots <- struct{}{}:
	default:
		s.Send(wire.MessagePanel("Login server busy. Try again."))
		return
	}
	w.authPending[s] = true
	var transfer *channelTicket
	if len(pkt) >= 100 && pkt[48] != 0 {
		var valid bool
		transfer, valid = w.consumeTicket(cstr(pkt[48:100]), accountName, s)
		if !valid {
			delete(w.authPending, s)
			<-w.authSlots
			s.RejectAdmission(wire.MessagePanel("Channel transfer expired. Sign in again."))
			return
		}
	}
	sanctionVersion := w.staffOps.sanctionVersions[accountSessionKey(accountName)]
	go func() {
		defer func() { <-w.authSlots }()
		var acc *model.Account
		var err error
		if transfer != nil {
			acc, err = w.store.LoadAccount(accountName)
		} else {
			acc, err = account.Authenticate(w.store, accountName, password)
		}
		result := &loginResult{accountName: accountName, account: acc, err: err, sanctionVersion: sanctionVersion, transfer: transfer}
		if backend, ok := w.sanctionBackend(); ok && err == nil {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			result.sanctions, result.sanctionErr = backend.ActiveSanctionState(ctx, acc.Name)
			cancel()
		}
		w.commands <- command{s: s, queuedAt: time.Now(), login: result}
	}()
}

func (w *World) onLoginResult(s *net.Session, result *loginResult) {
	if !w.authPending[s] {
		return
	}
	delete(w.authPending, s)
	if s.IsClosed() || s.AdmissionRejected() {
		return
	}
	// Authentication runs outside the World loop and may finish after the
	// maintenance cutoff or the final persistence snapshot.
	if w.shuttingDown || w.shutdownLoginsBlocked(w.now()) {
		s.Send(wire.MessagePanel("Server going into maintenance. Try again shortly."))
		return
	}
	if result.err != nil {
		if errors.Is(result.err, account.ErrInvalidCredentials) {
			log.Printf("[#%d] LOGIN rejected account=%q", s.ID, result.accountName)
		} else {
			log.Printf("[#%d] LOGIN error account=%q: %v", s.ID, result.accountName, result.err)
		}
		s.Send(wire.MessagePanel("Wrong account or password."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	acc := result.account
	sanctionErr := result.sanctionErr
	if result.sanctionVersion != w.staffOps.sanctionVersions[accountSessionKey(acc.Name)] {
		// An old authentication snapshot must not restore a revoked mute or
		// bypass a sanction refresh that is still in flight. Retry a fresh read.
		sanctionErr = errors.New("sanction state changed during authentication")
	}
	if sanctionErr != nil {
		log.Printf("[#%d] LOGIN sanction check failed account=%q: %v", s.ID, acc.Name, sanctionErr)
		s.Send(wire.MessagePanel("Login temporarily unavailable. Try again shortly."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	if message, banned := w.admitSanctions(acc.Name, result.sanctions, w.now()); banned {
		log.Printf("[#%d] LOGIN rejected account=%q: account suspended", s.ID, acc.Name)
		s.Send(wire.MessagePanel(message))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	if reason, rejected := w.networkRejected(s); rejected {
		log.Printf("[#%d] LOGIN rejected account=%q ip=%q: network policy (%s)",
			s.ID, acc.Name, s.RemoteIP(), reason)
		s.Send(wire.MessagePanel("Connections from this network are not allowed."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	if result.transfer != nil && (w.cluster == nil || !w.now().Before(result.transfer.expires) ||
		result.transfer.session != s || result.transfer.channel != w.channel ||
		result.transfer.account != accountSessionKey(acc.Name) || w.cluster.transfers[result.transfer.account] != result.transfer) {
		s.RejectAdmission(wire.MessagePanel("Channel transfer expired. Sign in again."))
		return
	}
	if !w.claimAccountSession(s, acc.Name) {
		log.Printf("[#%d] LOGIN rejected account=%q: account already connected", s.ID, acc.Name)
		s.Send(wire.AlreadyPlaying())
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	if !w.claimAuthenticatedClientSlot(s) {
		w.releaseAccountSession(s, acc)
		log.Printf("[#%d] LOGIN rejected account=%q ip=%q: limit of %d authenticated clients",
			s.ID, acc.Name, s.RemoteIP(), w.authenticatedClientLimitFor(s))
		s.Send(wire.MessagePanel("Only " + strconv.Itoa(w.authenticatedClientLimitFor(s)) +
			" game clients are allowed from the same network."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	}
	// Reload storage after reserving the session. The auth goroutine's snapshot
	// predates PBKDF2 and may be stale if the previous owner changed inventory
	// or gold and saved/disconnected during verification. Admitting that copy
	// could restore pre-trade state and duplicate items or gold. Authentication
	// already checked the hash; gameplay comes from the latest stored snapshot.
	if fresh, err := w.store.LoadAccount(acc.Name); err != nil {
		w.releaseAccountSession(s, acc)
		w.releaseAuthenticatedClientSlot(s)
		log.Printf("[#%d] LOGIN failed to reload account %q: %v", s.ID, acc.Name, err)
		s.Send(wire.MessagePanel("Error loading the account. Try again."))
		time.AfterFunc(300*time.Millisecond, s.Close)
		return
	} else {
		acc = fresh
	}
	pinAccountEntryPositions(acc)
	// Arch stat bonuses follow the source Mortal's current level. Refresh them
	// before syncProgression so this session starts with the correct budget.
	refreshArchMortalLevel(acc)
	accountRepaired := false
	for i := range acc.Chars {
		if acc.Chars[i].Name == "" {
			continue
		}
		syncProgression(&acc.Chars[i])
		w.recalcPlayer(&acc.Chars[i])
		filterShortSkills(&acc.Chars[i])
		// GuildID/GuildRank are denormalized copies: the guild may have dissolved
		// or the rank changed while this character was offline.
		if w.repairGuildState(&acc.Chars[i]) {
			accountRepaired = true
		}
	}
	if accountRepaired && w.criticalWrites == nil {
		// Persist the repair now rather than repeating it on every login. With
		// the coordinator, the idempotent repair rides the next account write.
		if err := w.saveAccount(acc); err != nil {
			log.Printf("[#%d] save account guild repair %q: %v", s.ID, acc.Name, err)
		}
	}
	p := &Player{Session: s, Account: acc, CharSlot: -1}
	w.players[s] = p
	s.MarkAuthenticated()
	w.sendLoginCharacterList(s, p)
	w.recordLogin(acc.Name, s.RemoteIP())
	log.Printf("[#%d] character list sent (%d characters)", s.ID, characterCount(acc.Chars))
	if result.transfer != nil {
		delete(w.cluster.transfers, result.transfer.account)
		entry := make([]byte, characterLoginPacketSize)
		binary.LittleEndian.PutUint32(entry[12:16], uint32(result.transfer.slot))
		w.onEnterWorld(s, entry)
	}
}

const maxCharactersPerAccount = 4

// Preserve historical denylist tokens; translation would change accepted names.
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
		log.Printf("generate character UID %q: %v", name, err)
		return model.Char{}, false
	}
	created.UID = uid
	syncProgression(&created)
	w.recalcPlayer(&created)
	setPlayerCurHP(&created, playerMaxHP(&created))
	setPlayerCurMP(&created, playerMaxMP(&created))
	return created, true
}

// onCreateCharacter handles MSG_NewCharacter 0x20F authoritatively: validate,
// instantiate the class template, persist, and update character selection.
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
		log.Printf("[#%d] creation rejected slot=%d name=%q class=%d", s.ID, slot, name, class)
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
			log.Printf("[#%d] check name %q: %v", s.ID, name, err)
		}
		s.Send(wire.NewCharacterFail())
		return
	}
	created, ok := w.newCharacterFromTemplate(name, class)
	if !ok {
		log.Printf("[#%d] missing class layout %d", s.ID, class)
		s.Send(wire.NewCharacterFail())
		return
	}
	if w.criticalWrites != nil {
		// Character selection holds no Char pointer; growing empty slots first
		// keeps the pre-image and proposal the same length.
		for len(p.Account.Chars) <= slot {
			p.Account.Chars = append(p.Account.Chars, model.Char{})
		}
		images := criticalPreimages(p)
		p.Account.Chars[slot] = created
		nameKey := strings.ToLower(name)
		if w.charNames != nil {
			// Reserve the name while the write is pending; another account
			// cannot claim it before the outcome is known.
			w.charNames[nameKey] = struct{}{}
		}
		err := w.queueCriticalLiveMutation("character creation", images, func() {
			if !s.IsClosed() {
				s.Send(selectionUpdatePacket(s, wire.OpCNFNewCharacter, uint16(s.ID), p))
			}
			log.Printf("[#%d] character created: %q slot=%d class=%d spawn=(%d,%d)",
				s.ID, name, slot, class, created.X, created.Y)
		}, criticalLiveMutationOptions{reject: func() {
			if w.charNames != nil && !accountUsesName(p.Account, name) {
				delete(w.charNames, nameKey)
			}
			if !s.IsClosed() {
				s.Send(wire.NewCharacterFail())
			}
		}})
		if err != nil {
			if w.charNames != nil && !accountUsesName(p.Account, name) {
				delete(w.charNames, nameKey)
			}
			log.Printf("[#%d] new character %q admission rejected: %v", s.ID, name, err)
			s.Send(wire.NewCharacterFail())
		}
		return
	}
	previousChars := append([]model.Char(nil), p.Account.Chars...)
	for len(p.Account.Chars) <= slot {
		p.Account.Chars = append(p.Account.Chars, model.Char{})
	}
	p.Account.Chars[slot] = created
	if err := w.saveAccount(p.Account); err != nil {
		p.Account.Chars = previousChars
		log.Printf("[#%d] save new character %q: %v", s.ID, name, err)
		s.Send(wire.NewCharacterFail())
		return
	}
	if w.charNames != nil {
		w.charNames[strings.ToLower(name)] = struct{}{}
	}
	// Character creation must return the selection aggregate in the ABI chosen
	// by this session's login packet; stock and source structs are not isomorphic.
	s.Send(selectionUpdatePacket(s, wire.OpCNFNewCharacter, uint16(s.ID), p))
	log.Printf("[#%d] character created: %q slot=%d class=%d spawn=(%d,%d)",
		s.ID, name, slot, class, created.X, created.Y)
}

// onEnterWorld: 0x213. Place the character in the world and send the login
// sequence in WYD 7.48 order so the client initializes every required field.
func (w *World) onEnterWorld(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || len(pkt) != characterLoginPacketSize {
		log.Printf("[#%d] enter-world missing player or character", s.ID)
		return
	}
	if p.InWorld {
		return // A repeated entry must not reset an authoritative live character.
	}
	if w.shuttingDown || s.IsClosed() || p.PersistencePoisoned || w.criticalPlayerReserved(p) {
		return
	}
	slot := int(int32(binary.LittleEndian.Uint32(pkt[12:16])))
	if slot < 0 || slot >= len(p.Account.Chars) || slot >= maxCharactersPerAccount || p.Account.Chars[slot].Name == "" {
		log.Printf("[#%d] enter-world invalid character slot %d", s.ID, slot)
		return
	}
	if owner := w.playerByCharacterUID(p.Account.Chars[slot].UID); owner != nil && owner != p {
		s.Send(wire.MessagePanel("This character is already connected. Try again after it disconnects."))
		return
	}
	if w.criticalWrites != nil {
		if err := w.prepareCriticalCharacterEntry(p, slot); err != nil {
			log.Printf("[#%d] failed to prepare character entry: %v", s.ID, err)
			s.Send(wire.MessagePanel("The character could not be prepared. Try again."))
		}
		return
	}
	p.CharSlot = slot
	p.Char = &p.Account.Chars[slot]
	ch := p.Char
	_, entryX, entryY := w.homeCitySpawn(ch)
	entryX, entryY = w.findFreePlayerPosition(entryX, entryY, 8, p)
	if isLoadtestAccountName(p.Account.Name, w.loadtestAccountPrefix) {
		entryX, entryY = w.loadtestSpawn.X, w.loadtestSpawn.Y
		// Keep load-test bots in Tauron without stacking them on one tile.
		// This exercises movement and collision without self-blocking the group.
		entryX, entryY = w.findFreePlayerPosition(entryX, entryY, 32, p)
	}
	ch.X, ch.Y = entryX, entryY
	// Load authoritative sidecar state before replacing affects and coins in
	// memory. A database failure must not erase or partially restore the state.
	if err := w.loadCharStateInto(p); err != nil {
		log.Printf("[#%d] failed to load character state for %q: %v", s.ID, ch.Name, err)
		p.CharSlot = -1
		p.Char = nil
		s.Send(wire.MessagePanel("The character state could not be loaded. Try again."))
		return
	}
	w.migrateUnsupportedEquipment(p)
	// First equip is the only activation boundary for costumes, premium mounts
	// and fairies. Existing deadlines are checked before any score or appearance
	// reaches the client; expired equipment is durably removed here.
	if err := w.prepareTimedEquipmentForEnter(p, w.now()); err != nil {
		log.Printf("[#%d] failed to prepare timed equipment for %q: %v", s.ID, ch.Name, err)
		p.CharSlot = -1
		p.Char = nil
		s.Send(wire.MessagePanel("Timed equipment could not be validated. Try again."))
		return
	}
	previousEmblem := ch.CapeEmblem
	if w.ensureCapeEmblem(ch) {
		if err := w.saveAccount(p.Account); err != nil {
			ch.CapeEmblem = previousEmblem
			p.CharSlot, p.Char = -1, nil
			s.Send(wire.MessagePanel("The cape emblem could not be saved. Try again."))
			return
		}
	}
	w.publishCharacterEntry(p, slot)
}

// publishCharacterEntry runs on World after preparation has committed. Position
// and client IDs are allocated at publication, not before an asynchronous save:
// unrelated players may have entered or moved while that save was pending.
func (w *World) publishCharacterEntry(p *Player, slot int) {
	s, ch := p.Session, p.Char
	// Selection may outlive an offline guild kingdom/rank change. Reconcile
	// this derived projection again before any appearance reaches the client.
	w.repairGuildState(ch)
	_, entryX, entryY := w.homeCitySpawn(ch)
	entryX, entryY = w.findFreePlayerPosition(entryX, entryY, 8, p)
	if isLoadtestAccountName(p.Account.Name, w.loadtestAccountPrefix) {
		entryX, entryY = w.loadtestSpawn.X, w.loadtestSpawn.Y
		entryX, entryY = w.findFreePlayerPosition(entryX, entryY, 32, p)
	}
	ch.X, ch.Y = entryX, entryY
	w.recalcPlayer(ch)
	// A character who logs out dead retains CurHP=0. Entering with zero HP
	// locks the client in the death pose, unable even to request /restart.
	// Restore one HP so the player can reach a healer or use a potion.
	//
	// Do this after recalculation sets MaxHP and before any packet carrying HP.
	if playerMaxHP(ch) > 0 && playerCurHP(ch) == 0 {
		setPlayerCurHP(ch, 1)
		p.DeadAt = time.Time{}
		log.Printf("[#%d] %s entered dead; restored to 1 HP", s.ID, ch.Name)
	}
	// Use the lowest free ClientId, matching the native server's connection
	// index. A growing counter gives the same 7.48 client process a new ID on
	// each relog and leaves stale state behind. Never replace an active player
	// when all 999 slots are occupied.
	playerID, ok := w.allocPlayerID()
	if !ok {
		p.CharSlot = -1
		p.Char = nil
		p.InWorld = false
		p.ID = 0
		s.Send(wire.MessagePanel("The world is full. Please try again later."))
		log.Printf("[#%d] ENTER-WORLD rejected: 999-player limit reached", s.ID)
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

	// 1) Full STRUCT_MOB. Recalculation already subtracted purchased skills
	// from SkillPts; do not restore the entire budget on each relog.
	clientChar := clientGuildProjection(p, *ch)
	clientChar.Equip = clientEquipProjection(ch)
	enter := wire.EnterWorld(p.ID, uint16(slot), clientChar)
	wire.PutEnterWorldWeather(enter, uint16(w.weather))
	p.SentWeather = w.weather
	s.Send(enter)
	// 2) Self-CreateMob (spawn=2), observed in-game. Without it, a second login
	// in the same client process retains stale HP/MP. ActionStop follows it.
	s.Send(playerCreatePacket(p))
	// 3) Login sequence in WYD 7.48 order: 3A8 -> 336 -> 185 -> 337 -> 36B -> 181 -> 366.
	s.Send(wire.WarInfo())
	s.Send(playerScorePacket(p))
	s.Send(playerAffectsPacket(p))
	s.Send(wire.UpdateCarry(p.ID, ch.Inv[:], ch.Gold))
	s.Send(wire.UpdateEtc(p.ID, *ch))
	s.Send(wire.SelfEquip(p.ID, clientChar.Equip[:]))
	// The source client consumes the wide 36-byte resource ABI; stock 7.48
	// must receive the original 20-byte projection for the same live state.
	s.Send(wire.HpMp(p.ID, wireScoreState(ch)))
	s.Send(wire.ActionStop(p.ID, ch.X, ch.Y))
	s.Send(wire.SetShortSkill(p.ID, ch.ShortSkill))
	w.sendKefraPortalState(p)

	// 4) Publish nearby mobs, players, and items, then populate p.Visible.
	w.refreshPlayerVisibility(p)
	w.restoreNightmareSession(p)
	w.issueClientIntegrityChallenge(p)
	log.Printf("[#%d] initial visibility: %d entities", s.ID, len(p.Visible))
}

// isLoadtestAccountName restricts alternate spawning to the provisioned set.
// An arbitrary account starting with "bot" must not be moved accidentally.
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

// onApplyBonus handles MSG_ApplyBonus 0x277: BonusType int16@12, Detail int16@14.
func (w *World) onApplyBonus(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || len(pkt) != applyBonusPacketSize || playerCurHP(p.Char) == 0 {
		return
	}
	bonusType := int(int16(binary.LittleEndian.Uint16(pkt[12:14])))
	detail := int(int16(binary.LittleEndian.Uint16(pkt[14:16])))
	// A 7.48 skill-master purchase uses MSG_ApplyBonus type 2, not MSG_BuyItem:
	// Detail contains the skill item and TargetID the master. Treating this as
	// a regular bonus discarded every learning attempt.
	if bonusType == 2 {
		itemIndex := int(binary.LittleEndian.Uint16(pkt[14:16]))
		masterID := binary.LittleEndian.Uint16(pkt[16:18])
		w.onLearnSkillAtMaster(s, p, itemIndex, masterID)
		return
	}
	// As in killMobState, raw applyBonus recalculation clamps buffed CurHP.
	// Capture it before recalculation and restore it after recalcPlayer.
	oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)
	if !applyBonus(p.Char, bonusType, detail) {
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		return
	}
	w.recalcPlayer(p.Char) // Reapply equipment and affects after raw applyBonus.
	if oldHP > 0 {
		setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
	}
	if oldMP > 0 {
		setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
	}
	s.Send(playerScorePacket(p))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	w.syncPlayerVitalsToObservers(p)
	w.updatePartyMember(p)
	log.Printf("[#%d] bonus type=%d detail=%d STR=%d INT=%d DEX=%d CON=%d ATK=%d MATK=%d DEF=%d HP=%d/%d MP=%d/%d special=%v",
		s.ID, bonusType, detail, playerStr(p.Char), playerInt(p.Char), playerDex(p.Char),
		playerCon(p.Char),
		playerAttack(p.Char), playerMagicAttack(p.Char), playerDefense(p.Char),
		playerCurHP(p.Char), playerMaxHP(p.Char), playerCurMP(p.Char), playerMaxMP(p.Char),
		effectiveScore(p.Char).Mastery)
}

// onSwapItem: 0x376. Move/equip in authoritative server state, then echo the
// successful 0x376 to finish the native drag lifecycle.
func (w *World) onSwapItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || len(pkt) != 20 ||
		w.shuttingDown || p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) {
		return
	}
	if p.Trade != nil && w.criticalPlayerReserved(w.playerByID(p.Trade.OpponentID)) {
		return
	}
	st, sp, dt, dp := pkt[12], pkt[13], pkt[14], pkt[15]
	src := playerSlotOf(p, st, sp)
	dst := playerSlotOf(p, dt, dp)
	if src == nil || dst == nil {
		log.Printf("[#%d] invalid SWAP %d/%d -> %d/%d", s.ID, st, sp, dt, dp)
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
	if w.tryStackMerge(p, st, sp, dt, dp, binary.LittleEndian.Uint16(pkt[16:18])) {
		return
	}
	w.cancelTrade(p, "inventory changed")
	if w.criticalWrites != nil && (st == placeStorage || dt == placeStorage) {
		targetID := binary.LittleEndian.Uint16(pkt[16:18])
		if err := w.queueCargoItemSwap(p, st, sp, dt, dp, targetID); err != nil {
			s.Send(wire.SendItem(p.ID, st, sp, *src))
			s.Send(wire.SendItem(p.ID, dt, dp, *dst))
			s.Send(wire.MessagePanel("The item move could not be submitted. Please try again."))
			log.Printf("[#%d] submit Cargo item move: %v", s.ID, err)
		}
		return
	}
	if w.criticalWrites != nil && (st == placeEquip || dt == placeEquip) {
		targetID := binary.LittleEndian.Uint16(pkt[16:18])
		if err := w.queueEquipmentSwap(p, st, sp, dt, dp, targetID); err != nil {
			s.Send(wire.SendItem(p.ID, st, sp, *src))
			s.Send(wire.SendItem(p.ID, dt, dp, *dst))
			if errors.Is(err, errTimedItemExpired) {
				s.Send(wire.MessagePanel("This timed item has expired."))
			} else if !errors.Is(err, errEquipmentSwapEligibility) {
				s.Send(wire.MessagePanel("The item move could not be submitted. Please try again."))
			}
			log.Printf("[#%d] submit equipment move: %v", s.ID, err)
		}
		return
	}
	oldSrc, oldDst := *src, *dst
	*src, *dst = *dst, *src
	// The client cannot decide equipment eligibility. Stage the swap, validate
	// every affected slot, and roll back if class, requirements, or nPos fail.
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
		log.Printf("[#%d] SWAP equipment rejected %d/%d -> %d/%d", s.ID, st, sp, dt, dp)
		return
	}
	// A lone off-hand weapon becomes the main weapon (W2PP _MSG_TradingItem).
	oldMain, oldOff := p.Char.Equip[6], p.Char.Equip[7]
	promoted := (st == placeEquip || dt == placeEquip) && w.promoteOffhandWeapon(p.Char)
	restore := func() {
		p.Char.Equip[6], p.Char.Equip[7] = oldMain, oldOff
		*src, *dst = oldSrc, oldDst
	}
	// A first equip starts the server-owned deadline. The activation and the
	// slot move are committed together before either is confirmed to the client.
	timedActivated := false
	activationNow := w.now()
	for _, target := range []struct {
		typ, pos byte
		item     *model.Item
	}{{st, sp, src}, {dt, dp, dst}} {
		if target.typ != placeEquip {
			timedActivated = pauseUnequippedFairy(target.item, activationNow) || timedActivated
			continue
		}
		activated, err := w.activateTimedItem(target.item, target.pos, activationNow)
		if err != nil {
			restore()
			s.Send(wire.SendItem(p.ID, st, sp, *src))
			s.Send(wire.SendItem(p.ID, dt, dp, *dst))
			if errors.Is(err, errTimedItemExpired) {
				s.Send(wire.MessagePanel("This timed item has expired."))
			} else {
				s.Send(wire.MessagePanel("This timed item could not be activated."))
			}
			log.Printf("[#%d] timed equip rejected %d: %v", s.ID, target.item.Index, err)
			return
		}
		timedActivated = timedActivated || activated
	}
	if st == placeStorage || dt == placeStorage || timedActivated || promoted {
		if err := w.saveAccount(p.Account); err != nil {
			restore()
			s.Send(wire.SendItem(p.ID, st, sp, *src))
			s.Send(wire.SendItem(p.ID, dt, dp, *dst))
			s.Send(wire.MessagePanel("Save failed. The item move was cancelled."))
			log.Printf("[#%d] save item move: %v", s.ID, err)
			return
		}
	}
	// FUN_00486808 and source OnPacketSwapItem consume a single 0x376 result.
	// Two 0x182 snapshots update cells but do not finish cursor state.
	targetID := binary.LittleEndian.Uint16(pkt[16:18])
	s.Send(wire.SwapItem(p.ID, st, sp, dt, dp, targetID))
	// Refresh the remaining-time projection after freezing or resuming a fairy.
	for _, target := range []struct {
		typ, pos byte
		item     model.Item
	}{{st, sp, *src}, {dt, dp, *dst}} {
		if isFairyIndex(target.item.Index) {
			s.Send(wire.SendItem(p.ID, target.typ, target.pos, target.item))
		}
	}
	if promoted {
		s.Send(wire.SendItem(p.ID, placeEquip, 6, p.Char.Equip[6]))
		s.Send(wire.SendItem(p.ID, placeEquip, 7, p.Char.Equip[7]))
	}
	if st == placeEquip || dt == placeEquip {
		if (st == placeEquip && sp == mountSlot) || (dt == placeEquip && dp == mountSlot) {
			w.resetEggIncubationClock(p)
		}
		// A newly equipped zero-state mount starts alive (HP/food/longevity).
		if mount, _ := equippedMount(p.Char); mount != nil {
			w.initFreshMount(mount)
		}
		w.recalcPlayer(p.Char)
		s.Send(playerScorePacket(p))
		s.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.syncPlayerVitalsToObservers(p)
		w.syncCriaPet(p) // Equipped offspring spawns a pet; unequipping removes it.
		// Incremental 0x36B updates owner and observers without position data.
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
	return npcID != 0 && w.resolveCargoNPC(p, npcID) != nil || hasBackpack(p)
}

func (w *World) nearCargoNPC(p *Player) bool {
	return w.resolveCargoNPC(p, 0) != nil || hasBackpack(p)
}

// hasBackpack reports a living Huntress who learned Backpack (skill 85):
// WYD 7.48 skill help 5085 opens Cargo from anywhere, without a banker.
func hasBackpack(p *Player) bool {
	return p != nil && p.Char != nil && playerCurHP(p.Char) > 0 &&
		isLearnedClassSkill(p.Char, backpackSkill)
}

func (w *World) resolveCargoNPC(p *Player, requestedID uint16) *Mob {
	if p == nil || p.Char == nil {
		return nil
	}
	// The native 7.48 Cargo branch opens locally (FUN_0045ee28) and only puts
	// the banker ID in MSG_SwapItem.TargetID; it does not send the newer 0x28B
	// context packet. Validate that packet target authoritatively, then bind it
	// for cargo-gold and cargo-item operations that carry no NPC ID themselves.
	if requestedID != 0 {
		m, err := w.resolveNPCInteraction(p, requestedID)
		if err != nil || !cargoNPCMatches(m) {
			return nil
		}
		p.CargoNPC = m.ID
		return m
	}
	if p.CargoNPC != 0 {
		m, err := w.resolveNPCInteraction(p, p.CargoNPC)
		if err == nil && cargoNPCMatches(m) {
			return m
		}
	}

	// Deposit/withdraw packets do not carry TargetID. Resolve the nearest
	// visible banker from the spatial index so a locally opened native Cargo
	// window remains usable before the first item swap.
	var selected *Mob
	bestDistance := npcInteractionRange + 1
	for _, m := range w.nearbyMobs(p.X, p.Y, npcInteractionRange) {
		if !p.hasVisible(m.ID) || !cargoNPCMatches(m) {
			continue
		}
		distance := chebyshev(p.X, p.Y, m.X, m.Y)
		if selected == nil || distance < bestDistance ||
			(distance == bestDistance && m.ID < selected.ID) {
			selected, bestDistance = m, distance
		}
	}
	if selected == nil {
		p.CargoNPC = 0
		return nil
	}
	p.CargoNPC = selected.ID
	return selected
}

func cargoNPCMatches(m *Mob) bool {
	return m != nil && !m.Dead && m.Def != nil && m.Def.Score != nil &&
		m.Def.Score.Merchant&0x0F == cargoMerchantType
}

func (w *World) onCargoGold(s *net.Session, pkt []byte, deposit bool) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld ||
		w.shuttingDown || p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) ||
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
	} else {
		if p.Account.CargoGold < amount || p.Char.Gold > maxCharacterGold-amount {
			s.Send(wire.MessagePanel("That amount could not be withdrawn."))
			return
		}
	}
	if w.criticalWrites != nil {
		if err := w.queueCargoGoldTransfer(p, deposit, amount); err != nil {
			log.Printf("[#%d] submit cargo transfer: %v", s.ID, err)
			s.Send(wire.MessagePanel("The transfer could not be submitted. Please try again."))
		}
		return
	}
	if deposit {
		p.Char.Gold -= amount
		p.Account.CargoGold += amount
	} else {
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
		log.Printf("[#%d] save Cargo transfer: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. The transfer was not applied."))
		return
	}
	publishCargoGoldTransfer(p, deposit, amount)
}

// onUseNPC: 0x28B. A merchant click sends the shop list (0x17C).
// clickOkAbsent distinguishes a short packet without ClickOk from a real zero
// in instrumentation logs. Ordinary NPCs require a configured interaction.
const clickOkAbsent = -1

// parseUseNPCRequest reads 0x28B, the native _MSG_Quest packet
// (139|FLAG_CLIENT2GAME). npcID is at 12; ClickOk at 16 confirms a quest.
// Read ClickOk only in a full 20-byte packet, preserving short shop requests.
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
		log.Printf("[#%d] 0x28B too short (%d bytes)", s.ID, len(pkt))
		return
	}
	w.cancelTrade(p, "NPC interaction")
	// Validate existence, visibility, and range for both shops and quests.
	// Captures show two layouts: 0x27B REQShopList has 16 bytes (TargetID@12),
	// while 0x28B UseNPC/Quest has 20 (ClickOk@16). Log the actual opcode to
	// avoid attributing a size to the wrong packet.
	opcode := int(wire.ParseHeader(pkt).Type)
	m, err := w.resolveNPCInteraction(p, npcID)
	if err != nil {
		log.Printf("[#%d] 0x%X rejected id=%d clickOk=%d: %v", s.ID, opcode, npcID, clickOk, err)
		s.Send(wire.MessagePanel(npcInteractionMessage(err)))
		return
	}
	log.Printf("[#%d] 0x%X npc=%q id=%d clickOk=%d dist=%d size=%d",
		s.ID, opcode, m.Def.Name, m.ID, clickOk, chebyshev(p.X, p.Y, m.X, m.Y), len(pkt))
	// Each interaction opens exactly one authoritative context. Cargo must not
	// reuse a stale banker after the player clicks another NPC.
	p.ShopNPC, p.ShopTax, p.CraftNPC, p.CargoNPC = 0, 0, 0, 0
	p.PremiumNPC, p.PremiumVersion = 0, 0
	clearCityWarContext(p)

	// Allowlist routing: specialized handlers (shop/master/cargo), explicitly
	// configured quests.json NPCs, then a neutral response. Treating every
	// remaining NPC as a quest would capture crafting, blacksmiths, mount
	// masters, and unclassified types.
	//
	// 0x13 also has low nibble 3. Test the skill master before generic shops,
	// or the client drags the skill into inventory instead of learning it.
	// The Sefirot composer is also the class skill master; test composition
	// before shops or every click opens skills instead of composition.
	//
	// Require both ClickOk=1 (sent after confirmation) and all eight stones
	// to avoid intercepting ordinary skill purchases. Otherwise open the shop.
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
	if w.handleApprenticeCape(s, p, m, clickOk) {
		return
	}
	if w.handleMysticLandNPC(s, p, m, clickOk) {
		return
	}
	if w.handleCircleServiceNPC(s, p, m) {
		return
	}
	if w.handleOfficialNPCService(s, p, m) {
		return
	}
	if w.handleCarbMasterNPC(s, p, m) {
		return
	}
	if isPremiumMerchant(m.Def) {
		w.openPremiumShop(s, p, m)
		return
	}
	if city, isCollector := cityCollectorCity(m); isCollector {
		owner := uint16(0)
		if w.guilds != nil {
			owner = w.guilds.Wars.Cities.Territories[city].Owner
		}
		setCityWarContext(p, m.ID, city, owner, w.now())
		log.Printf("[#%d] tax collector opened: %q city=%s", s.ID, m.Def.Name, cityWarZones[city].name)
		return
	}
	if shopType, isShop := shopTypeForMerchant(m.Def.Score.Merchant); isShop {
		p.ShopNPC = m.ID // Bind the authoritative merchant for purchases.
		display := shopDisplayList(m.Def.Vende, shopType)
		// TMProject renders 27 shop entries; stock 7.48 keeps its 64-entry ABI.
		_, tax, taxed := w.cityShopTax(m)
		if taxed {
			p.ShopTax = tax
		}
		s.Send(wire.ShopList(display, p.ShopTax, shopType))
		if dropped := countShopItems(m.Def.Vende) - countShopItems(display); dropped > 0 {
			log.Printf("[#%d] shop %q: %d items beyond the client limit of %d",
				s.ID, m.Def.Name, dropped, clientShopSlots)
		}
		log.Printf("[#%d] shop opened: %q (%d displayed items)", s.ID, m.Def.Name, countShopItems(display))
		return
	}
	if m.Def.Score.Merchant == craftingMerchant {
		// The client selects the window from the NPC's face/position. Retain
		// authoritative context for the subsequent composition request.
		p.CraftNPC = m.ID
		log.Printf("[#%d] composer opened: %q", s.ID, m.Def.Name)
		return
	}
	if m.Def.Score.Merchant&0x0F == cargoMerchantType {
		p.CargoNPC = m.ID
		log.Printf("[#%d] Cargo opened: %q", s.ID, m.Def.Name)
		return
	}
	if w.handleAscensionNPC(s, p, m) {
		return
	}
	// Ascension precedes kingdom selection: with the Stone and Sefirot
	// equipped, clicking the king requests Arch ascension rather than a cape.
	if kingdomForNPC(m.Def) != model.KingdomNeutral && w.createArch(s, p) {
		return
	}
	if w.handleKingdomNPC(s, p, m) {
		return
	}
	if quest := w.questForNPC(m.Def); quest != nil {
		if w.handleKibitaSoul(s, p, m, quest) {
			return
		}
		w.onQuestInteraction(s, p, m, quest, clickOk)
		return
	}
	if kind, reserved := reservedNPCKind(m.Def); reserved {
		// Known locally opened window (cargo): no additional action or message.
		log.Printf("[#%d] click on %q (%s, no server-side action)", s.ID, m.Def.Name, kind)
		return
	}
	log.Printf("[#%d] click on %q without configured interaction", s.ID, m.Def.Name)
	s.Send(wire.MessagePanel("This character has nothing for you right now."))
}

// onBuyItem: 0x379. Charge the catalog price for Vende[sellSlot], add the item
// to Carry, and confirm with the native 0x379 response.
func (w *World) onBuyItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	// Require 16 bytes before reading sellSlot@14; a forged short 0x379 would
	// otherwise panic when slicing pkt[14:16].
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld || w.shuttingDown ||
		p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) || playerCurHP(p.Char) == 0 || len(pkt) != 24 {
		return
	}
	if p.Trade != nil && w.criticalPlayerReserved(w.playerByID(p.Trade.OpponentID)) {
		return
	}
	w.cancelTrade(p, "shop purchase")
	// The 7.48 grid sends the displayed MerchantID.
	targetID := binary.LittleEndian.Uint16(pkt[12:14])
	sellSlot := binary.LittleEndian.Uint16(pkt[14:16]) // TargetCarryPos@14
	if p.ShopNPC == 0 {
		log.Printf("[#%d] purchase without an open shop", s.ID)
		s.Send(wire.MessagePanel("Open a merchant before buying an item."))
		return
	}
	if targetID != p.ShopNPC {
		// Do not trust the client target. Return a panel to finish the click
		// visually instead of failing silently.
		log.Printf("[#%d] purchase merchant mismatch packet=%d open=%d", s.ID, targetID, p.ShopNPC)
		s.Send(wire.MessagePanel("That merchant is no longer available."))
		return
	}
	m, interactionErr := w.resolveNPCInteraction(p, p.ShopNPC)
	if interactionErr != nil {
		log.Printf("[#%d] invalid purchase shop=%d slot=%d", s.ID, p.ShopNPC, sellSlot)
		p.ShopNPC = 0
		// The 7.48 UI has no local fallback for a stale merchant interaction.
		s.Send(wire.MessagePanel("The merchant is no longer available."))
		return
	}
	// Resolve the client's grid slot against the same compacted 27-entry
	// list sent in 0x17C. Indexing raw Vende would purchase a different item.
	shopType, isShop := shopTypeForMerchant(m.Def.Score.Merchant)
	if !isShop {
		log.Printf("[#%d] purchase from a non-shop NPC: %q", s.ID, m.Def.Name)
		s.Send(wire.MessagePanel("That character is not a merchant."))
		return
	}
	display := shopDisplayList(m.Def.Vende, shopType)
	index, ok := shopSlotFromClient(sellSlot)
	if !ok || index >= len(display) {
		log.Printf("[#%d] invalid purchase shop=%d slot=%d -> index=%d (list has %d)",
			s.ID, p.ShopNPC, sellSlot, index, len(display))
		s.Send(wire.MessagePanel("That shop item is no longer available."))
		return
	}
	it := display[index]
	if it.Index == 0 {
		// Empty visual cells are not purchasable and need a terminal UI response.
		s.Send(wire.MessagePanel("That shop item is no longer available."))
		return
	}
	// A skill-master purchase is a learning request. Items 5000..5095 never
	// enter inventory; onLearnSkill validates class, requirements, points,
	// and gold, then persists the account.
	if m.Def.Score.Merchant == skillMasterMerchant {
		w.onLearnSkill(s, p, int(it.Index))
		return
	}
	def, exists := w.items[it.Index]
	if !exists {
		log.Printf("[#%d] purchase rejected: item %d absent from the server catalog", s.ID, it.Index)
		s.Send(wire.MessagePanel("That item is unavailable on this server."))
		return
	}
	basePrice := uint64(def.Price)
	totalPrice, treasury, city, _ := w.cityPriceForNPC(m, basePrice)
	if totalPrice > uint64(maxCharacterGold) || uint64(p.Char.Gold) < totalPrice {
		log.Printf("[#%d] insufficient gold: item %d costs %d (base=%d), available=%d", s.ID, it.Index, totalPrice, basePrice, p.Char.Gold)
		// The 7.48 client displays the server's 0x101 panel rather than
		// synthesizing this error. Every expected rejection needs a response.
		s.Send(wire.MessagePanel("You do not have enough gold."))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueShopPurchase(p, it, totalPrice, treasury, city, targetID, sellSlot); err != nil {
			log.Printf("[#%d] submit merchant purchase item=%d: %v", s.ID, it.Index, err)
			if errors.Is(err, errShopInventoryFull) {
				s.Send(wire.MessagePanel("Your inventory is full."))
			} else {
				s.Send(wire.MessagePanel("The purchase could not be submitted. Please try again."))
			}
		}
		return
	}
	dst, err := w.commitShopPurchase(p, it, totalPrice, city, treasury)
	if err != nil {
		if errors.Is(err, errShopInventoryFull) {
			log.Printf("[#%d] inventory full, purchase aborted", s.ID)
			s.Send(wire.MessagePanel("Your inventory is full."))
			return
		}
		log.Printf("[#%d] ERROR saving purchase item=%d: %v", s.ID, it.Index, err)
		// After rollback, send a terminal result so the 7.48 UI does not appear
		// to have ignored the purchase button.
		s.Send(wire.MessagePanel("The purchase could not be completed."))
		return
	}
	if dst < 0 {
		log.Printf("[#%d] inventory full, purchase aborted", s.ID)
		// Explain the rejection without changing authoritative gold.
		s.Send(wire.MessagePanel("Your inventory is full."))
		return
	}
	// FUN_00487b92 copies the shop item to MyCarryPos and updates Coin from
	// this response. SendItem+UpdateEtc alone does not complete the native
	// merchant purchase lifecycle.
	s.Send(wire.BuyItem(p.ID, p.ShopNPC, sellSlot, uint16(dst), p.Char.Gold))
	log.Printf("[#%d] bought item %d for %d gold (base=%d) -> inv[%d] (remaining gold=%d)", s.ID, it.Index, totalPrice, basePrice, dst, p.Char.Gold)
}

// onBuyToto: 0x3CE. The wire/window flow belongs to 7.48; ticket identity and
// the gold/inventory transaction are authoritative extensions. Never trust
// Coin@20: the price comes from ItemList.
func (w *World) onBuyToto(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld || w.shuttingDown ||
		p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) || playerCurHP(p.Char) == 0 || len(pkt) != 36 {
		return
	}
	if p.Trade != nil && w.criticalPlayerReserved(w.playerByID(p.Trade.OpponentID)) {
		return
	}
	w.cancelTrade(p, "TOTO ticket purchase")

	targetID := binary.LittleEndian.Uint16(pkt[12:14])
	shopSlot := int16(binary.LittleEndian.Uint16(pkt[14:16]))
	dst := int16(binary.LittleEndian.Uint16(pkt[16:18]))
	match := int32(binary.LittleEndian.Uint32(pkt[24:28]))
	scoreA := int32(binary.LittleEndian.Uint32(pkt[28:32]))
	scoreB := int32(binary.LittleEndian.Uint32(pkt[32:36]))

	if p.ShopNPC == 0 {
		s.Send(wire.MessagePanel("Open a merchant before buying a TOTO ticket."))
		return
	}
	if targetID != p.ShopNPC {
		s.Send(wire.MessagePanel("That merchant is no longer available."))
		return
	}
	m, err := w.resolveNPCInteraction(p, p.ShopNPC)
	if err != nil {
		p.ShopNPC = 0
		s.Send(wire.MessagePanel("The merchant is no longer available."))
		return
	}
	shopType, isShop := shopTypeForMerchant(m.Def.Score.Merchant)
	if !isShop {
		s.Send(wire.MessagePanel("That character is not a merchant."))
		return
	}
	if shopSlot < 0 {
		s.Send(wire.MessagePanel("That shop item is no longer available."))
		return
	}
	display := shopDisplayList(m.Def.Vende, shopType)
	index, ok := shopSlotFromClient(uint16(shopSlot))
	if !ok || index >= len(display) || display[index].Index != 4147 {
		s.Send(wire.MessagePanel("That shop item is not a TOTO ticket."))
		return
	}
	if dst < 0 || dst >= model.PlayerCarrySlots || p.Char.Inv[dst].Index != 0 {
		s.Send(wire.MessagePanel("Choose an empty inventory slot."))
		return
	}
	if match < 1 || match > 80 || scoreA < 0 || scoreA > 127 || scoreB < 0 || scoreB > 127 {
		s.Send(wire.MessagePanel("Invalid TOTO match or score."))
		return
	}
	def, exists := w.items[4147]
	if !exists {
		s.Send(wire.MessagePanel("TOTO tickets are unavailable on this server."))
		return
	}
	basePrice := uint64(def.Price)
	totalPrice, treasury, city, _ := w.cityPriceForNPC(m, basePrice)
	if totalPrice > uint64(maxCharacterGold) || uint64(p.Char.Gold) < totalPrice {
		s.Send(wire.MessagePanel("You do not have enough gold."))
		return
	}

	ticket, err := materializeItem(model.Item{
		Index: 4147,
		Eff:   [6]byte{64, byte(match), 65, byte(scoreA), 66, byte(scoreB)},
	})
	if err != nil {
		log.Printf("[#%d] ERROR materializing TOTO ticket: %v", s.ID, err)
		s.Send(wire.MessagePanel("The TOTO ticket could not be created."))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueTicketPurchase(p, ticket, int(dst), totalPrice, treasury, city); err != nil {
			log.Printf("[#%d] submit TOTO purchase match=%d: %v", s.ID, match, err)
			s.Send(wire.MessagePanel("The TOTO purchase could not be submitted. Please try again."))
		}
		return
	}
	if err := w.commitTicketPurchase(p, ticket, int(dst), totalPrice, city, treasury); err != nil {
		log.Printf("[#%d] ERROR saving TOTO ticket match=%d: %v", s.ID, match, err)
		s.Send(wire.MessagePanel("The TOTO purchase could not be completed."))
		return
	}

	// 0x3CE has no server response. Publish only the two native snapshots
	// needed to materialize the ticket and updated balance.
	s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	log.Printf("[#%d] bought TOTO match=%d score=%d-%d for %d gold (base=%d) -> inv[%d]",
		s.ID, match, scoreA, scoreB, totalPrice, basePrice, dst)
}

// onSellItem: 0x37A. Ordinary sales require the open shop context. The native
// Hell Store panel opens locally, so its target is validated independently.
func (w *World) onSellItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld || w.shuttingDown ||
		p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) || playerCurHP(p.Char) == 0 || len(pkt) != 20 {
		return
	}
	if p.Trade != nil && w.criticalPlayerReserved(w.playerByID(p.Trade.OpponentID)) {
		return
	}
	w.cancelTrade(p, "shop sale")
	targetID := binary.LittleEndian.Uint16(pkt[12:14])
	m, interactionErr := w.resolveNPCInteraction(p, targetID)
	if interactionErr != nil {
		if p.ShopNPC == targetID {
			p.ShopNPC = 0
		}
		return
	}
	hellStore := isHellStoreNPC(m)
	if hellStore && !w.hellStoreAvailable(p, m) {
		return
	}
	if !hellStore && p.ShopNPC == 0 {
		log.Printf("[#%d] sale without an open shop", s.ID)
		return
	}
	if !hellStore && targetID != p.ShopNPC {
		log.Printf("[#%d] sale merchant mismatch packet=%d open=%d", s.ID, targetID, p.ShopNPC)
		s.Send(wire.MessagePanel("That merchant is no longer available."))
		return
	}
	if _, isShop := shopTypeForMerchant(m.Def.Score.Merchant); !hellStore && !isShop {
		p.ShopNPC = 0
		return
	}
	myTypeWord := binary.LittleEndian.Uint16(pkt[14:16])
	myPosWord := binary.LittleEndian.Uint16(pkt[16:18])
	if myTypeWord != uint16(placeInv) || myPosWord >= uint16(model.PlayerCarrySlots) {
		return
	}
	myType, myPos := byte(myTypeWord), byte(myPosWord)
	// Normal UI sells from Carry. Accepting Equip would allow forged MyType
	// to sell faces, capes, mounts, or other structural slots.
	if myType != placeInv {
		log.Printf("[#%d] sale rejected outside inventory type=%d pos=%d", s.ID, myType, myPos)
		return
	}
	src := slotOf(p.Char, myType, myPos)
	if src == nil || src.Index == 0 {
		log.Printf("[#%d] invalid sale type=%d pos=%d", s.ID, myType, myPos)
		return
	}
	if _, filled := model.CelestialSealID(*src); filled {
		s.Send(wire.MessagePanel("A filled Spirit's Seal cannot be sold."))
		s.Send(wire.SendItem(p.ID, myType, myPos, *src))
		return
	}
	sold := src.Index
	def, exists := w.items[sold]
	basePrice := uint64(0)
	if exists {
		basePrice = uint64(def.Price) / 4 // WYD sale price is 25% of purchase price.
	}
	if m.Def.Name == "HG_Dionys" {
		basePrice = dionysSalePrice(basePrice)
	}
	basePrice = passiveMerchantSalePrice(p.Char, sold, basePrice)
	city, taxRate, taxed := w.cityShopTax(m)
	netPrice := basePrice
	treasury := uint64(0)
	if taxed {
		tax := basePrice * uint64(taxRate) / 100
		netPrice -= tax
		treasury = tax / 4
	}
	// Sale credits must respect the two-billion gold cap like every other
	// server-side gold source.
	if uint64(p.Char.Gold) > uint64(maxCharacterGold) || netPrice > uint64(maxCharacterGold)-uint64(p.Char.Gold) {
		s.Send(wire.MessagePanel("You cannot carry any more gold."))
		s.Send(wire.SendItem(p.ID, myType, myPos, *src)) // Item unchanged.
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueShopSale(p, int(myPos), netPrice, treasury, city); err != nil {
			log.Printf("[#%d] submit merchant sale item=%d: %v", s.ID, sold, err)
			// The item is unchanged on admission rejection, not optimistically sold.
			s.Send(wire.MessagePanel("The sale could not be submitted. Please try again."))
			s.Send(wire.SendItem(p.ID, myType, myPos, *src))
		}
		return
	}
	if err := w.commitShopSale(p, src, netPrice, city, treasury); err != nil {
		log.Printf("[#%d] ERROR saving sale item=%d: %v", s.ID, sold, err)
		return
	}
	s.Send(wire.SendItem(p.ID, myType, myPos, *src)) // Slot now empty.
	s.Send(wire.UpdateEtc(p.ID, *p.Char))            // Update gold.
	log.Printf("[#%d] sold item %d for %d gold (base=%d; city=%d; gold=%d)", s.ID, sold, netPrice, basePrice, city, p.Char.Gold)
}

// onMove: 0x366. PosX/Y@12 and Route[24] describe movement intent;
// TargetX/Y@24 is the visual destination. Replicate for client interpolation,
// but player_movement.go promotes only elapsed steps to authoritative state.
func (w *World) onMove(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || playerCurHP(p.Char) == 0 || len(pkt) < 52 {
		return
	}
	w.cancelTrade(p, "player moved")
	if binary.LittleEndian.Uint32(pkt[20:24]) == 6 {
		w.onIllusionMove(p, pkt)
		return
	}
	startX, startY, wireRoute, authorityRoute, ok := w.validatedPlayerMoveRoute(p, pkt)
	if !ok {
		reason := movementPacketRejectionSummary(p, pkt)
		// The client already animates its requested route. Stop the previous
		// authoritative plan and correct the owner without using the teleport
		// action, which would clear the source client's death state.
		w.publishPlayerStop(p)
		s.Send(wire.ActionRouteCorrection(p.ID, p.X, p.Y))
		w.recordSecurityViolation(s, wire.OpAction, reason)
		return
	}
	x, y := actionTarget748(pkt)
	if x == 0 || y == 0 || len(authorityRoute) == 0 || samePlayerMovementDestination(p, x, y) {
		return
	}
	catchupSteps := 0
	if len(wireRoute) > 0 && len(authorityRoute) > len(wireRoute) {
		catchupSteps = len(authorityRoute) - len(wireRoute)
	}
	publishX, publishY, publishRoute := startX, startY, wireRoute
	if catchupSteps > 0 && len(authorityRoute) <= maxMovementRouteBytes {
		// Observers know the last authoritative position. Publish the entire
		// bridge when it fits the wire rather than snapping the avatar to the
		// sender's visually advanced PosX/Y.
		publishX, publishY = p.X, p.Y
		publishRoute = authorityRoute
	}
	w.beginPlayerMovement(p, publishX, publishY, x, y, publishRoute, authorityRoute,
		catchupSteps, w.now())
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
		!w.terrain.Walkable(targetX, targetY) ||
		!w.oxStepAllowed(p, targetX, targetY) ||
		w.isOXMember(p) && w.ox.phase == "question" && !now.Before(w.ox.deadline) {
		return
	}
	mastery := int(playerMastery(p.Char, 1))
	mana := skillManaCost(skill, mastery, int(effectiveScore(p.Char).SaveMana))
	if playerCurMP(p.Char) < uint32(mana) {
		return
	}
	fromX, fromY := p.X, p.Y
	targetX, targetY = w.findFreePlayerPosition(targetX, targetY, 2, p)
	if !w.oxStepAllowed(p, targetX, targetY) {
		return
	}
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
	log.Printf("[#%d] used Illusion @(%d,%d)->(%d,%d) mp=-%d", p.Session.ID,
		fromX, fromY, targetX, targetY, mana)
}

// onActionStop handles MSG_Stop (0x367, 52B). PosX/Y@12 and
// TargetX/Y@24 describe a short final plan; Route[24] is zeroed.
func (w *World) onActionStop(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 52 {
		return
	}
	targetX, targetY, route, ok := w.validatedActionStopRoute(p, pkt)
	if !ok {
		w.recordSecurityViolation(s, wire.OpActionStop, "Stop attempted to reposition the character")
		return
	}
	if len(route) == 0 {
		w.publishPlayerStop(p)
		w.refreshPlayerVisibility(p)
		return
	}
	// The owner already animates its stop locally. For observers,
	// publish the reconstructed route from the authoritative position.
	w.beginPlayerMovement(p, p.X, p.Y, targetX, targetY, route, route, 0, w.now())
}

// onMoveStop: MSG_MOVESTOP 0x2CB (36B). CurrentX/Y are at @20/@24.
// The client sends it when interrupting walking to attack.
func (w *World) onMoveStop(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || !p.InWorld || p.Char == nil || len(pkt) != 36 {
		return
	}
	x := binary.LittleEndian.Uint32(pkt[20:24])
	y := binary.LittleEndian.Uint32(pkt[24:28])
	if x == 0 || y == 0 || x > 65535 || y > 65535 ||
		!w.validReportedStop(p, uint16(x), uint16(y)) {
		w.recordSecurityViolation(s, wire.OpMoveStop, "MoveStop attempted to reposition the character")
		return
	}
	w.publishPlayerStop(p)
	w.refreshPlayerVisibility(p)
}

const (
	playerHomeCityMask       = uint32(0xC0)
	playerHomeCityShift      = 6
	mortalBeginnerLevelLimit = 35
	mortalBeginnerSpawnX     = 2112
	mortalBeginnerSpawnY     = 2042
)

// playerHomeCity preserves the legacy contract: Merchant bits 7:6 store the
// bound city. Zero bits keep older characters bound to Armia (city 0).
func playerHomeCity(ch *model.Char) int {
	if ch == nil || ch.Score == nil {
		return 0
	}
	city := int((ch.Score.Merchant & playerHomeCityMask) >> playerHomeCityShift)
	if city < 0 || city >= len(cityWarZones) {
		return 0
	}
	return city
}

func playerHomeCitySpawn(ch *model.Char) (int, uint16, uint16) {
	city := playerHomeCity(ch)
	// Keep the project's beginner login policy separate from native DoRecall.
	// The beginner field overrides the login destination, not the bound city.
	if newbieZoneEligible(ch) {
		return city, mortalBeginnerSpawnX, mortalBeginnerSpawnY
	}
	zone := cityWarZones[city]
	return city, zone.exitX, zone.exitY
}

// armiaSpawnSpread is the ±tile spread around the Armia birth point.
const armiaSpawnSpread = 5

// homeCitySpawn is playerHomeCitySpawn with the Armia birth spread: players
// bound to Armia appear at a random walkable tile within ±5 of 2091,2100.
func (w *World) homeCitySpawn(ch *model.Char) (int, uint16, uint16) {
	city, x, y := playerHomeCitySpawn(ch)
	if city != 0 || x != cityWarZones[0].exitX || y != cityWarZones[0].exitY {
		return city, x, y
	}
	// A private source: the spread must not consume draws of the gameplay RNG.
	nx := uint16(int(x) + rand.IntN(2*armiaSpawnSpread+1) - armiaSpawnSpread)
	ny := uint16(int(y) + rand.IntN(2*armiaSpawnSpread+1) - armiaSpawnSpread)
	if w.terrain.Walkable(nx, ny) {
		return city, nx, ny
	}
	return city, x, y
}

// bindableCityAt follows BASE_GetVillage for the four cities available through
// the 7.48 ChangeCity packet. Server position, not the reported Village, wins.
func bindableCityAt(x, y uint16) (int, bool) {
	for city := 0; city < 4; city++ {
		if homeCityZones[city].area.contains(x, y) {
			return city, true
		}
	}
	return 0, false
}

// MODERNIZACAO_COMPATIVEL: keep the existing Merchant hometown bits, but bind
// from confirmed server position rather than relying on a one-shot UI notice.
// W2PP's ChangeCity uses TargetX/Y; Go must wait for authoritative movement.
// Teleports use this same commit so their position and city roll back together.
func stagePlayerHomeCity(p *Player) {
	if p.Char.Score == nil {
		return
	}
	if city, ok := bindableCityAt(p.X, p.Y); ok {
		stageHomeCity(p.Char, city)
	}
}

// The caller owns position rollback; this commit restores both Merchant fields
// on failure. Quest teleports stage the same change in their wider transaction.
func (w *World) savePlayerLocation(p *Player) error {
	if p.Char.Score == nil {
		return w.saveAccount(p.Account)
	}
	previousMerchant := p.Char.Score.Merchant
	previousRuntimeMerchant := uint32(0)
	if p.Char.RuntimeScore != nil {
		previousRuntimeMerchant = p.Char.RuntimeScore.Merchant
	}
	stagePlayerHomeCity(p)
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Score.Merchant = previousMerchant
		if p.Char.RuntimeScore != nil {
			p.Char.RuntimeScore.Merchant = previousRuntimeMerchant
		}
		return err
	}
	return nil
}

func (w *World) bindPlayerHomeCity(p *Player) {
	if p == nil {
		return
	}
	if city, ok := bindableCityAt(p.X, p.Y); ok {
		w.bindPlayerHomeCityIndex(p, city)
	}
}

// Bind the last city actually reached, even if a movement batch leaves it.
// Call only after position, visibility and the account snapshot are stable.
func (w *World) bindPlayerHomeCityIndex(p *Player, city int) {
	if p == nil || !p.InWorld || p.Char == nil || p.Char.Score == nil || p.Account == nil {
		return
	}
	if city < 0 || city >= 4 || playerHomeCity(p.Char) == city {
		return
	}
	if w.criticalWrites != nil {
		if w.criticalTradeReserved(p) {
			return
		}
		proposed := accountStateSnapshot(p.Account)
		ch := &proposed.Chars[p.CharSlot]
		oldX, oldY := p.X, p.Y
		stageHomeCity(ch, city)
		err := w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, "hometown binding",
			"Your return city could not be saved.", func() {
				p.Char.Score.Merchant = ch.Score.Merchant
				if p.Char.RuntimeScore != nil && ch.RuntimeScore != nil {
					p.Char.RuntimeScore.Merchant = ch.RuntimeScore.Merchant
				}
			}, nil, false, criticalAccountMutationHooks{check: func() error {
				if p.X != oldX || p.Y != oldY {
					return errCriticalPersistenceIdentity
				}
				return nil
			}})
		if err != nil {
			log.Printf("[#%d] hometown binding admission rejected: %v", p.Session.ID, err)
		}
		return
	}
	previousMerchant := p.Char.Score.Merchant
	previousRuntimeMerchant := uint32(0)
	if p.Char.RuntimeScore != nil {
		previousRuntimeMerchant = p.Char.RuntimeScore.Merchant
	}
	stageHomeCity(p.Char, city)
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Score.Merchant = previousMerchant
		if p.Char.RuntimeScore != nil {
			p.Char.RuntimeScore.Merchant = previousRuntimeMerchant
		}
		log.Printf("[#%d] ChangeCity failed to persist %s[%d]: %v",
			p.Session.ID, cityWarZones[city].name, city, err)
	}
}

// recallPlayer handles both restart and quest-zone reset through DoRecall's
// city and guild destination rules. Login keeps its separate beginner policy.
//
// Observers see a corpse at its old position. RemoveMob type 3 must clear it
// before movement; otherwise rematerialization uses the new position and
// leaves an orphaned corpse at the death site. Then revive and publish.
func (w *World) recallPlayer(p *Player, reason string) bool {
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld {
		return false
	}
	if w.criticalWrites != nil {
		return w.queueCriticalRecall(p, reason)
	}
	// Restart or recall also exits the instance. Without detaching the member,
	// later cleanup could teleport them to the old room or block re-entry.
	now := w.now()
	wasInInstance := w.playerRuntimeInstanceID(p.ID) != ""
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
	city, spawnX, spawnY := w.recallSpawn(p.Char)
	p.X, p.Y = w.findFreePlayerPosition(spawnX, spawnY, 3, p)
	p.Char.X, p.Char.Y = p.X, p.Y
	clearPublishedPlayerMove(p)
	// Persist the safe position. A disk failure does not abort recall because
	// the in-memory position is correct and autosave retries shortly.
	if err := w.saveAccount(p.Account); err != nil {
		log.Printf("[#%d] recall (%s): failed to save position: %v", p.ID, reason, err)
	}
	if p.Session != nil {
		if wasInInstance {
			clearItemInstanceHUD(p)
		}
		p.Session.Send(playerScorePacket(p))
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	}
	w.refreshPlayerVisibility(p)
	w.syncPlayerVitals(p)
	w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
	w.updatePartyMember(p)
	log.Printf("[#%d] recall (%s) -> (%d,%d), bound city %s[%d], revived=%v",
		p.ID, reason, p.X, p.Y, cityWarZones[city].name, city, dead)
	return true
}

// onRestart implements _MSG_Restart (0x289). The native flow restores HP and
// calls DoRecall; recallPlayer centralizes the safe respawn here.
func (w *World) onRestart(s *net.Session) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) != 0 {
		return
	}
	// TMSrv rejects repeated requests during the first four seconds after death.
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
	log.Printf("[#%d] skill bar updated: %v", s.ID, p.Char.ShortSkill)
}

// filterShortSkills separates the learned-skill mask from visual shortcuts.
// Clients may send global (class*24 + skill) or local indices;
// remove shortcuts without a corresponding LearnedSkill bit.
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

// ChangeCity reports village@12, but legacy TMSrv derives the city
// from the authoritative position and stores hometown in Merchant[7:6].
// A forged packet cannot remotely select another city.
func (w *World) onChangeCity(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || p.Char.Score == nil || p.Account == nil || !p.InWorld || len(pkt) != 16 {
		return
	}
	reportedVillage := binary.LittleEndian.Uint32(pkt[12:16])
	city, ok := bindableCityAt(p.X, p.Y)
	if !ok {
		log.Printf("[#%d] ChangeCity ignored outside a city @(%d,%d) village=%d",
			s.ID, p.X, p.Y, reportedVillage)
		return
	}

	w.bindPlayerHomeCity(p)
	if playerHomeCity(p.Char) != city {
		return
	}
	if reportedVillage != uint32(city) {
		log.Printf("[#%d] ChangeCity village mismatch reported=%d; using authoritative position %s[%d] @(%d,%d)",
			s.ID, reportedVillage, cityWarZones[city].name, city, p.X, p.Y)
		return
	}
	log.Printf("[#%d] ChangeCity -> %s[%d] @(%d,%d)",
		s.ID, cityWarZones[city].name, city, p.X, p.Y)
}

// onSysQuit handles overloaded 0x3AE: MSG_DelayStart and MSG_SysQuit.
// The 7.48 client sends it on teleport and death (scene loading),
// not just logout. Do not close ghost shops or cancel trades here.
// Shops close on replacement or disconnect; the actual logout is
// transport closure, processed by onDisconnect.
// Synchronize/persist position and return the acknowledgment.
func (w *World) onSysQuit(s *net.Session) {
	p := w.players[s]
	if p == nil || p.Char == nil {
		return
	}
	if w.criticalWrites != nil {
		images := criticalPreimages(p)
		p.Char.X, p.Char.Y = p.X, p.Y
		err := w.queueCriticalLiveMutation("DelayStart", images, func() {
			if !s.IsClosed() {
				s.Send(wire.SysQuit(p.ID))
			}
		}, criticalLiveMutationOptions{})
		if err != nil {
			log.Printf("[#%d] DelayStart(0x3AE) admission rejected: %v", s.ID, err)
		}
		return
	}
	p.Char.X, p.Char.Y = p.X, p.Y
	if err := w.saveAccount(p.Account); err != nil {
		log.Printf("[#%d] error saving during DelayStart(0x3AE): %v", s.ID, err)
		return
	}
	s.Send(wire.SysQuit(p.ID))
	log.Printf("[#%d] DelayStart(0x3AE) ack (teleport/death/exit)", s.ID)
}

// attackRange is melee reach in tiles (Chebyshev distance).
const attackRange = 3

// physicalAttackRange ports normal _MSG_Attack reach. Ranged weapons
// use main-hand (slot 6) EF_RANGE, two base tiles and
// the native three-tile margin. EF_RANGE=5 bows reach ten tiles;
// attacks without a ranged weapon remain melee (three tiles).
func (w *World) physicalAttackRange(ch *model.Char) int {
	extra := 0
	if specialSkillLearned(ch, 101) {
		extra = 1
	}
	if ch == nil || len(ch.Equip) <= 6 {
		return attackRange + extra
	}
	weapon := ch.Equip[6]
	def, ok := w.items[weapon.Index]
	if !ok || weapon.Index == 0 {
		return attackRange + extra
	}
	weaponRange := itemAbility(weapon, def, "EF_RANGE")
	if weaponRange <= 0 {
		return attackRange + extra
	}
	return minInt(23, 2+weaponRange+3+extra)
}

// isLearnedClassSkill distinguishes actual skills from normal attacks.
// The 7.48 wire always sends SkillId. Bow attacks can leave it
// nonnegative without representing a skill. Routing every such value
// to onSkillAttack would reject normal hits as unlearned skills.
// Only a learned index belonging to this character's class
// may enter the skill path.
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

// One physical attack action per 900 ms for every character. Attack Speed
// never shortens this interval. A double hit is two hits within that action;
// the native bonus-nibble thresholds are defined by doubleHitChance.
const physicalAttackInterval = 900 * time.Millisecond

func attackIntervalFor(_ *model.Char) time.Duration { return physicalAttackInterval }

type clientAttackClock struct {
	tick uint32
}

// validateClientAttackClock is deliberately pure. A stale/dead/out-of-range
// target must not consume the next legitimate physical attack on a new target.
func validateClientAttackClock(p *Player, pkt []byte, now time.Time) (clientAttackClock, bool) {
	if p == nil || len(pkt) < 12 {
		return clientAttackClock{}, false
	}
	tick := binary.LittleEndian.Uint32(pkt[8:12])
	// SKIPCHECKTICK is reserved for server-generated attacks.
	if tick == 0 || tick == 0x0E0A1ACA {
		return clientAttackClock{}, false
	}
	interval := attackIntervalFor(p.Char)
	if p.LastAttackTick != 0 {
		// Unsigned subtraction preserves GetTickCount wrap, but a genuinely
		// backward tick must not become a huge valid delta.
		if tick < p.LastAttackTick && p.LastAttackTick-tick < 0x80000000 {
			return clientAttackClock{}, false
		}
		if tick-p.LastAttackTick < uint32(interval/time.Millisecond) {
			return clientAttackClock{}, false
		}
	}
	if !p.LastAttackAt.IsZero() && now.Sub(p.LastAttackAt) < interval {
		return clientAttackClock{}, false
	}
	return clientAttackClock{tick: tick}, true
}

func commitClientAttackClock(p *Player, clock clientAttackClock, now time.Time) {
	p.LastAttackTick = clock.tick
	p.LastAttackAt = now
}

func acceptClientAttack(p *Player, pkt []byte, now time.Time) bool {
	clock, ok := validateClientAttackClock(p, pkt, now)
	if ok {
		commitClientAttackClock(p, clock, now)
	}
	return ok
}

// skillPacketInterval is only a flood floor.  The authoritative cadence of a
// spell is SkillData.Delay, enforced by onSkillAttack/SkillReady.  Keeping a
// separate, short floor prevents a modified client from turning the attack
// opcode into a busy loop without making a normal swing suppress a spell cast.
// The old implementation shared LastAttackAt/LastAttackTick between physical
// attacks and skills; an auto-attack arriving a few milliseconds before a
// valid spell caused exactly the "three of four hits" symptom seen in-game.
const skillPacketInterval = 200 * time.Millisecond

func acceptClientSkill(p *Player, pkt []byte, skillIndex int, now time.Time) bool {
	if p == nil || len(pkt) < 12 || skillIndex < 0 {
		return false
	}
	tick := binary.LittleEndian.Uint32(pkt[8:12])
	if tick == 0 || tick == 0x0E0A1ACA {
		return false
	}
	if p.LastSkillTicks == nil {
		p.LastSkillTicks = make(map[int]uint32)
	}
	if previous, exists := p.LastSkillTicks[skillIndex]; exists {
		// Same/retrograde client ticks are replays.  The unsigned comparison
		// preserves GetTickCount wrap while rejecting a genuine rewind.
		if tick == previous || (tick < previous && previous-tick < 0x80000000) {
			return false
		}
	}
	if !p.LastSkillAt.IsZero() && now.Sub(p.LastSkillAt) < skillPacketInterval {
		return false
	}
	p.LastSkillTicks[skillIndex] = tick
	p.LastSkillAt = now
	return true
}

// onAttack: 0x39D/0x367/0x39E. The packet nominates TargetID only.
// The server resolves existence, gameplay space, distance, terrain,
// line of sight, and other requirements before applying damage.
// combat.go computes damage; zero HP kills the mob and schedules respawn.
func (w *World) onAttack(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || w.criticalPlayerReserved(p) {
		return
	}
	req := parseAttackSkill(pkt)
	// Reserve the entire reward set before consuming attack/skill clocks or
	// mutating either hit of Iron Spear. A later save guard is too late.
	if (w.mobByID(req.TargetID) != nil || w.mobByID(req.SecondaryTargetID) != nil) &&
		w.criticalKillParticipantsReserved(p) {
		return
	}
	if playerCurHP(p.Char) == 0 && !(req.Skill == 99 && specialSkillLearned(p.Char, 99)) {
		return
	}
	now := w.now()
	// SkillId@24 denotes a skill only if learned by this class.
	// The 7.48 client also fills it for ranged physical attack variants;
	// those hits must remain here instead of silently becoming
	// rejected attempts to use a skill.
	if isLearnedClassSkill(p.Char, req.Skill) {
		// Skills have their own native SkillData.Delay.  Do not apply the
		// physical swing limiter here: auto-attacks and spells are independent
		// actions in the client and may legitimately alternate in one second.
		if !acceptClientSkill(p, pkt, req.Skill, now) {
			metricSkillPacketRejected.Add(1)
			return
		}
		metricSkillPacketAccepted.Add(1)
		w.onSkillAttack(p, req)
		return
	}
	// Normal attacks remain protected by the fixed, server-owned cadence.
	attackClock, clockOK := validateClientAttackClock(p, pkt, now)
	if !clockOK {
		metricPhysicalAttackRejected.Add(1)
		return
	}
	// TargetID is at @44 in compact 7.48. Client-supplied damage,
	// attacker position, and other values are not authoritative.
	attackTwo := len(pkt) == 52 && wire.ParseHeader(pkt).Type == wire.OpAttackTwo
	airBladeEligible := attackTwo
	ironSpearEligible := attackTwo && p.Char.Class == 3 && p.Char.LearnedSkill&0x40 != 0
	maxRange := w.physicalAttackRange(p.Char)
	m := w.mobByID(req.TargetID)
	if m == nil {
		target := w.playerByID(req.TargetID)
		if !w.canInitiatePvP(p, target) || target == p || !target.InWorld || target.Char == nil ||
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
		commitClientAttackClock(p, attackClock, now)
		w.cancelTrade(p, "player attacked")
		w.breakHideOnAttack(p)
		w.cancelTrade(target, "character was attacked")
		p.CombatTargetID = target.ID
		target.LastAttackerID = p.ID

		hit := playerPhysicalHitPlayerWithRNG(p, target, w.intn)
		if !hit.Hit {
			secondary := ironSpearSecondaryResult{}
			if ironSpearEligible {
				secondary = w.applyHuntressIronSpearSecondary(p, req, target.ID, target.X, target.Y,
					hit.Double, hit.Critical, now)
			}
			w.sendToPlayerView(target, func() []byte {
				if secondary.ok {
					return spectralPacket(p.Char, wire.AttackHitsWideTwoResult(p.ID, p.X, p.Y, target.X, target.Y,
						p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), []wire.SkillTarget{
							{ID: target.ID, Miss: true, MaxHP: playerMaxHP(target.Char)}, secondary.target,
						}))
				}
				return spectralPacket(p.Char, wire.AttackHitWideResult(p.ID, target.ID,
					p.X, p.Y, target.X, target.Y, 0, playerMaxHP(target.Char), p.Char.Exp,
					playerCombatMP(p.Char), 0, true))
			})
			w.finalizeIronSpearSecondary(p, secondary)
			log.Printf("[#%d] missed PvP attack against %q", s.ID, target.Char.Name)
			return
		}
		hit = applyHuntressAirBlade(hit, p.Char, playerDefense(target.Char), airBladeEligible, w.intn)
		calculated := hit.Damage
		calculated = addFlatDamage(calculated, w.equipmentGemBonuses(p.Char).forceDamage)
		calculated = absorbFlatDamage(calculated, w.playerFlatDamageAbsorb(target.Char))
		// A living adult mount absorbs 25% of damage into its own HP.
		calculated = uint32(w.absorbMountDamage(target, int(calculated)))
		calculated = absorbManaControlDamageAt(target.Char, calculated, now)
		applied := minU32(calculated, playerCurHP(target.Char))
		setPlayerCurHP(target.Char, playerCurHP(target.Char)-applied)
		lethal := playerCurHP(target.Char) == 0
		if lethal {
			w.cancelTrade(target, "character died")
			w.mountRiderDied(target)
			target.DeadAt = now
			w.receiveDeathLetter(target, p.Char.Name, "player")
			w.applyPvPKills(p, target)
		}
		secondary := ironSpearSecondaryResult{}
		if ironSpearEligible && hit.Flank == 0 {
			secondary = w.applyHuntressIronSpearSecondary(p, req, target.ID, target.X, target.Y,
				hit.Double, hit.Critical, now)
		}

		// Floating numbers show full calculated damage, including overkill;
		// authoritative health decreases only by actual available HP.
		w.sendToPlayerView(target, func() []byte {
			if secondary.ok {
				return spectralPacket(p.Char, wire.AttackHitsWideTwoResult(p.ID, p.X, p.Y, target.X, target.Y,
					p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), []wire.SkillTarget{
						{ID: target.ID, Damage: calculated, MaxHP: playerMaxHP(target.Char)}, secondary.target,
					}))
			}
			if hit.Flank > 0 {
				return spectralPacket(p.Char, wire.AttackHitWideFlankResult(p.ID, target.ID, p.X, p.Y, target.X, target.Y,
					calculated, playerMaxHP(target.Char), p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), hit.Flank))
			}
			return spectralPacket(p.Char, wire.AttackHitWideResult(p.ID, target.ID, p.X, p.Y, target.X, target.Y,
				calculated, playerMaxHP(target.Char), p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), false))
		})
		w.syncPlayerVitals(target)
		w.updatePartyMember(target)
		w.finalizeIronSpearSecondary(p, secondary)
		if lethal {
			w.publishPlayerDeath(target, p.ID)
		}
		log.Printf("[#%d] PvP attack against %q calculated_damage=%d applied=%d hp=%d",
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
	commitClientAttackClock(p, attackClock, now)
	w.cancelTrade(p, "player attacked")
	w.breakHideOnAttack(p)
	p.CombatTargetID = m.ID
	// Like TMSrv SetBattle, being attacked immediately puts a mob
	// into combat, before its next scan for visible enemies.
	m.TargetID = p.ID
	hit := playerPhysicalHitMobAt(p, m, w.intn, now)
	if !hit.Hit {
		secondary := ironSpearSecondaryResult{}
		if ironSpearEligible {
			secondary = w.applyHuntressIronSpearSecondary(p, req, m.ID, m.X, m.Y,
				hit.Double, hit.Critical, now)
		}
		w.sendToMobView(m, func() []byte {
			if secondary.ok {
				return spectralPacket(p.Char, wire.AttackHitsWideTwoResult(p.ID, p.X, p.Y, m.X, m.Y,
					p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), []wire.SkillTarget{
						{ID: m.ID, Miss: true, MaxHP: m.Def.Score.MaxHP}, secondary.target,
					}))
			}
			return spectralPacket(p.Char, wire.AttackHitWideResult(p.ID, m.ID, p.X, p.Y, m.X, m.Y,
				0, m.Def.Score.MaxHP, p.Char.Exp, playerCombatMP(p.Char), 0, true))
		})
		w.finalizeIronSpearSecondary(p, secondary)
		log.Printf("[#%d] missed attack on mob id=%d %q (accuracy=%d%%)", s.ID, m.ID, m.Def.Name,
			playerVersusMobAccuracy(p.Char, m.Def))
		return
	}
	hit = applyHuntressAirBlade(hit, p.Char, effectiveMobDefenseAt(m, now), airBladeEligible, w.intn)
	dmg := hit.Damage
	dmg = uint32(applyCouragePvEDamageAt(p.Char, int(dmg), false, now))
	dmg = addFlatDamage(dmg, w.equipmentGemBonuses(p.Char).forceDamage)
	// Boss shields absorb before applying damage so floating numbers
	// show actual damage (zero while immune).
	dmg = w.bossMitigateDamage(m, dmg)
	oldHP := m.HP
	if dmg >= m.HP {
		m.HP = 0
	} else {
		m.HP -= dmg
	}
	// Boss subsystem hook: ordinary mobs incur only a map lookup.
	w.notifyMobDamaged(m, oldHP, p.ID, dmg)
	secondary := ironSpearSecondaryResult{}
	if ironSpearEligible && hit.Flank == 0 {
		secondary = w.applyHuntressIronSpearSecondary(p, req, m.ID, m.X, m.Y,
			hit.Double, hit.Critical, now)
	}
	// 0x181 updates the bar; only 0x39D triggers animation and
	// floating damage numbers in the 7.48 client.
	// Instance mobs share the map coordinates with the public world, but their
	// hit animation and damage must remain private to the current stage members.
	// Sending this through broadcast leaked combat packets to outsiders (and
	// let a client observe an encounter it could not target).
	w.sendToMobView(m, func() []byte {
		if secondary.ok {
			return spectralPacket(p.Char, wire.AttackHitsWideTwoResult(p.ID, p.X, p.Y, m.X, m.Y,
				p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), []wire.SkillTarget{
					{ID: m.ID, Damage: dmg, MaxHP: m.Def.Score.MaxHP}, secondary.target,
				}))
		}
		if hit.Flank > 0 {
			return spectralPacket(p.Char, wire.AttackHitWideFlankResult(p.ID, m.ID, p.X, p.Y, m.X, m.Y,
				dmg, m.Def.Score.MaxHP, p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), hit.Flank))
		}
		return spectralPacket(p.Char, wire.AttackHitWideResult(p.ID, m.ID, p.X, p.Y, m.X, m.Y,
			dmg, m.Def.Score.MaxHP, p.Char.Exp, playerCombatMP(p.Char), hit.visualFlags(), false))
	})
	w.finalizeIronSpearSecondary(p, secondary)
	if m.HP == 0 {
		w.killMobState(p, m, dmg, minU32(dmg, oldHP))
	} else {
		// Nonfatal hit: update only the HP bar.
		w.sendToMobViewProtocol(m, func(observer *Player) []byte {
			// Source observers own a uint32 resource handler; stock observers
			// keep the proportional WORD projection used by the original 7.48.
			return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP,
				m.Def.Score.MaxMP, m.Def.Score.MaxMP)
		})
		w.gameplayLogf("attack", "[#%d] attacked mob id=%d %q dmg=%d hp=%d/%d",
			s.ID, m.ID, m.Def.Name, dmg, m.HP, m.Def.Score.MaxHP)
	}
}

type ironSpearSecondaryResult struct {
	target   wire.SkillTarget
	player   *Player
	mob      *Mob
	oldMobHP uint32
	damage   uint32
	lethal   bool
	ok       bool
}

func ironSpearBehindTile(attackerX, attackerY, primaryX, primaryY, x, y uint16) bool {
	dx, dy := 0, 0
	if primaryX > attackerX {
		dx = 1
	} else if primaryX < attackerX {
		dx = -1
	}
	if primaryY > attackerY {
		dy = 1
	} else if primaryY < attackerY {
		dy = -1
	}
	return int(x) == int(primaryX)+dx && int(y) == int(primaryY)+dy
}

// applyHuntressIronSpearSecondary implements Iron Spear (skill 78). The
// client only nominates the entity occupying the tile immediately behind the
// primary target; the server revalidates that geometry and recalculates damage.
func (w *World) applyHuntressIronSpearSecondary(attacker *Player, req skillCastRequest,
	primaryID, primaryX, primaryY uint16, double, critical bool, now time.Time) ironSpearSecondaryResult {
	id := req.SecondaryTargetID
	if attacker == nil || attacker.Char == nil || w.criticalPlayerReserved(attacker) ||
		id == 0 || id == attacker.ID || id == primaryID {
		return ironSpearSecondaryResult{}
	}

	if m := w.mobByID(id); m != nil {
		if w.criticalKillParticipantsReserved(attacker) || m.Dead || m.HP == 0 || !w.playerCanInteractWithMob(attacker, m) ||
			!ironSpearBehindTile(attacker.X, attacker.Y, primaryX, primaryY, m.X, m.Y) ||
			!w.combatLineOfSight(attacker.X, attacker.Y, m.X, m.Y) {
			return ironSpearSecondaryResult{}
		}
		m.TargetID = attacker.ID
		hit := playerPhysicalFollowupHitMobAt(attacker, m, double, critical, w.intn, now)
		result := ironSpearSecondaryResult{
			target: wire.SkillTarget{ID: m.ID, MaxHP: m.Def.Score.MaxHP}, mob: m, oldMobHP: m.HP, ok: true,
		}
		if !hit.Hit {
			result.target.Miss = true
			return result
		}
		dmg := uint32(applyCouragePvEDamageAt(attacker.Char, int(hit.Damage), false, now))
		dmg = addFlatDamage(dmg, w.equipmentGemBonuses(attacker.Char).forceDamage)
		dmg = w.bossMitigateDamage(m, dmg)
		result.damage, result.target.Damage = dmg, dmg
		if dmg >= m.HP {
			m.HP = 0
		} else {
			m.HP -= dmg
		}
		result.lethal = m.HP == 0
		return result
	}

	target := w.playerByID(id)
	if target == nil || target == attacker || !target.InWorld || target.Char == nil ||
		playerCurHP(target.Char) == 0 || sameSupportGroup(attacker, target) ||
		!w.canInitiatePvP(attacker, target) || !w.playersShareGameplaySpace(attacker, target) ||
		!ironSpearBehindTile(attacker.X, attacker.Y, primaryX, primaryY, target.X, target.Y) ||
		!w.combatLineOfSight(attacker.X, attacker.Y, target.X, target.Y) {
		return ironSpearSecondaryResult{}
	}
	target.LastAttackerID = attacker.ID
	w.cancelTrade(target, "character was attacked")
	hit := playerPhysicalFollowupHitPlayerWithRNG(attacker, target, double, critical, w.intn)
	result := ironSpearSecondaryResult{
		target: wire.SkillTarget{ID: target.ID, MaxHP: playerMaxHP(target.Char)}, player: target, ok: true,
	}
	if !hit.Hit {
		result.target.Miss = true
		return result
	}
	dmg := addFlatDamage(hit.Damage, w.equipmentGemBonuses(attacker.Char).forceDamage)
	dmg = absorbFlatDamage(dmg, w.playerFlatDamageAbsorb(target.Char))
	dmg = uint32(w.absorbMountDamage(target, int(dmg)))
	dmg = absorbManaControlDamageAt(target.Char, dmg, now)
	result.damage, result.target.Damage = dmg, dmg
	applied := minU32(dmg, playerCurHP(target.Char))
	setPlayerCurHP(target.Char, playerCurHP(target.Char)-applied)
	result.lethal = playerCurHP(target.Char) == 0
	return result
}

func (w *World) finalizeIronSpearSecondary(attacker *Player, result ironSpearSecondaryResult) {
	if !result.ok || result.target.Miss {
		return
	}
	if result.mob != nil {
		w.notifyMobDamaged(result.mob, result.oldMobHP, attacker.ID, result.damage)
		if result.lethal {
			w.killMobState(attacker, result.mob, result.damage, minU32(result.damage, result.oldMobHP))
			return
		}
		w.sendToMobViewProtocol(result.mob, func(observer *Player) []byte {
			return wire.MobHpMp(result.mob.ID, result.mob.HP, result.mob.Def.Score.MaxHP,
				result.mob.Def.Score.MaxMP, result.mob.Def.Score.MaxMP)
		})
		return
	}
	if result.player == nil {
		return
	}
	w.syncPlayerVitals(result.player)
	w.updatePartyMember(result.player)
	if result.lethal {
		w.cancelTrade(result.player, "character died")
		w.mountRiderDied(result.player)
		result.player.DeadAt = w.now()
		w.receiveDeathLetter(result.player, attacker.Char.Name, "player")
		w.applyPvPKills(attacker, result.player)
		w.publishPlayerDeath(result.player, attacker.ID)
	}
}

// killMobState handles monster death, XP, respawn, and death packets.
// chebyshev = max(|dx|,|dy|), WYD's distance metric.
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

// pickupRange is ground-item pickup reach (Chebyshev distance).
const pickupRange = 4

type dropItemRequest struct {
	srcType, srcPos byte
	rotate          uint32
	// gridX/gridY are the client's own cell (native FUN_0046247E converts the
	// character position into GridX@24 and GridY@26).
	gridX, gridY uint16
}

// dropCellTolerance bounds how far the client cell may be from the server
// position, which trails the visual route by up to a few steps.
const dropCellTolerance = 2

func parseDropItemRequest(pkt []byte) (dropItemRequest, bool) {
	if len(pkt) != 32 {
		return dropItemRequest{}, false
	}
	srcType := binary.LittleEndian.Uint32(pkt[12:16])
	srcPos := binary.LittleEndian.Uint32(pkt[16:20])
	// Native 0x175 removes Carry (1) and Cargo (2). Equip (0) remains
	// prohibited; each source has its own authoritative slot limit.
	validInventory := srcType == placeInv && srcPos < model.PlayerCarrySlots
	validCargo := srcType == placeStorage && srcPos < model.PlayerCargoSlots
	if !validInventory && !validCargo {
		return dropItemRequest{}, false
	}
	return dropItemRequest{srcType: byte(srcType), srcPos: byte(srcPos),
		rotate: binary.LittleEndian.Uint32(pkt[20:24]),
		gridX:  binary.LittleEndian.Uint16(pkt[24:26]), gridY: binary.LittleEndian.Uint16(pkt[26:28])}, true
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
// GridXY@24/26). Remove the source (0x182 confirms emptiness) and
// create a GroundItem at the player's position, preserving effects.
func (w *World) onDropItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	// Carry and Cargo are persisted through the owning account.  A session that
	// has not completed account binding must never publish a non-durable drop.
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld || playerCurHP(p.Char) == 0 || w.criticalTradeReserved(p) {
		return
	}
	w.cancelTrade(p, "item dropped on the ground")
	req, ok := parseDropItemRequest(pkt)
	if !ok {
		log.Printf("[#%d] invalid drop packet", s.ID)
		return
	}
	if req.srcType == placeStorage {
		if p.Account == nil || !w.nearCargoNPC(p) {
			s.Send(wire.MessagePanel("Move closer to the Warehouse to drop Cargo items."))
			return
		}
		if p.ghostShopLocksCargoSlot(int(req.srcPos)) {
			s.Send(wire.MessagePanel("The item is locked while it is listed on Auto Trade."))
			return
		}
	}
	// playerSlotOf resolves both Character Carry and account-owned Cargo; slotOf
	// only knows Character storage and made every valid type-2 drop impossible.
	src := playerSlotOf(p, req.srcType, req.srcPos)
	if src == nil || src.Index == 0 {
		log.Printf("[#%d] invalid drop type=%d pos=%d", s.ID, req.srcType, req.srcPos)
		s.Send(wire.MessagePanel("That item can no longer be dropped."))
		return
	}
	item := *src
	if _, filled := model.CelestialSealID(item); filled {
		s.Send(wire.MessagePanel("A filled Spirit's Seal cannot be dropped."))
		return
	}
	// Reserve without publishing. Clients see the item only after
	// authoritative inventory removal has been persisted. Like native 7.48 the
	// item lands on the character's own cell, never on a random neighbor.
	dropX, dropY := w.playerDropCell(p, req)
	g := w.createGroundDropCell(dropX, dropY, item, false, w.gameplaySpaceForPlayer(p), ownerOf(p))
	if g == nil {
		s.Send(wire.MessagePanel("There is no room to drop that item here."))
		return
	}
	if w.criticalWrites != nil {
		if err := w.queueGroundDrop(p, g, req, item); err != nil {
			w.unregisterGroundItem(g)
			log.Printf("[#%d] item drop admission rejected: %v", s.ID, err)
		}
		return
	}
	*src = model.Item{}
	// Close the cross-player autosave window: another player must not
	// collect and persist the item before its owner's removal is saved.
	if err := w.saveAccount(p.Account); err != nil {
		*src = item
		w.unregisterGroundItem(g)
		log.Printf("[#%d] drop canceled after save failure: %v", s.ID, err)
		s.Send(wire.MessagePanel("Save failed. The item was not dropped."))
		return
	}
	w.publishItemSpawn(g)
	// 0x175 detaches the cursor item and clears the slot. 0x182 confirms
	// the same authoritative state as other Carry mutations.
	s.Send(wire.CNFDropItem(uint32(req.srcType), uint32(req.srcPos), req.rotate, g.X, g.Y))
	s.Send(wire.SendItem(p.ID, req.srcType, req.srcPos, *src))
	log.Printf("[#%d] dropped item %d on the ground @(%d,%d)", s.ID, item.Index, g.X, g.Y)
}

// playerDropCell returns the cell the client reported under the character when
// it is walkable and within dropCellTolerance of the server position; otherwise
// the server position.
func (w *World) playerDropCell(p *Player, req dropItemRequest) (uint16, uint16) {
	if req.gridX == 0 || req.gridY == 0 || chebyshev(p.X, p.Y, req.gridX, req.gridY) > dropCellTolerance ||
		!w.terrain.Walkable(req.gridX, req.gridY) {
		return p.X, p.Y
	}
	return req.gridX, req.gridY
}

// onGetItem: 0x270 MSG_GetItem (DestType i32@12, DestPos i32@16, ItemID u16@20,
// GridXY@22/24). Validate reach and try the requested free slot;
// otherwise use the first free slot. Confirm with 0x182 and remove
// the GroundItem for every observer who could see it.
func (w *World) onGetItem(s *net.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Account == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 || w.criticalTradeReserved(p) {
		return
	}
	w.cancelTrade(p, "item picked up")
	req, ok := parseGetItemRequest(pkt)
	if !ok {
		log.Printf("[#%d] invalid pickup packet", s.ID)
		return
	}
	g := w.groundItems[req.itemID]
	if g == nil || g.criticalReserved() {
		return // already picked up by another player / expired
	}
	if !w.groundItemVisibleToPlayer(p, g) {
		log.Printf("[#%d] attempted to pick up item %d from another instance", s.ID, req.itemID)
		return
	}
	// Permanent world objects are map furniture, not loot. Without
	// this guard a player could put a castle gate into inventory.
	if g.Permanent {
		log.Printf("[#%d] attempted to pick up world object %d (item %d)",
			s.ID, g.ID, g.Item.Index)
		return
	}
	if chebyshev(p.X, p.Y, g.X, g.Y) > pickupRange {
		log.Printf("[#%d] pickup item %d out of range", s.ID, req.itemID)
		return
	}
	if !groundItemClaimable(p, g, w.now()) {
		s.Send(wire.MessagePanel(groundProtectedMessage))
		return
	}
	if def, exists := w.items[g.Item.Index]; exists {
		if rule, code, registered := w.volatiles.Rule(g.Item.Index); registered && rule.Action == "ground_gold" {
			value := volatileGoldValue(rule, g.Item, def)
			if value == 0 || p.Char.Gold > maxCharacterGold || value > maxCharacterGold-p.Char.Gold {
				log.Printf("[#%d] currency item=%d volatile=%d rejected: value=%d gold=%d",
					s.ID, g.Item.Index, code, value, p.Char.Gold)
				return
			}
			oldGold := p.Char.Gold
			if w.criticalWrites != nil {
				if err := w.queueGroundPickup(p, g, int(req.destPos), value); err != nil {
					log.Printf("[#%d] currency pickup admission rejected: %v", s.ID, err)
				}
				return
			}
			p.Char.Gold += value
			if err := w.saveAccount(p.Account); err != nil {
				p.Char.Gold = oldGold
				log.Printf("[#%d] currency pickup canceled after save failure: %v", s.ID, err)
				return
			}
			w.publishItemRemove(g)
			w.unregisterGroundItem(g)
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			log.Printf("[#%d] picked up currency item=%d gold=+%d total=%d", s.ID, g.Item.Index, value, p.Char.Gold)
			return
		}
	}
	if w.criticalWrites != nil {
		if err := w.queueGroundPickup(p, g, int(req.destPos), 0); err != nil {
			log.Printf("[#%d] item pickup admission rejected: %v", s.ID, err)
		}
		return
	}
	slot := -1
	if req.destType == placeInv { // honor the client's requested slot if free
		if dst := slotOf(p.Char, placeInv, req.destPos); dst != nil && dst.Index == 0 {
			*dst = g.Item
			slot = int(req.destPos)
		}
	}
	if slot < 0 {
		slot = addToInv(p.Char, g.Item)
	}
	if slot < 0 {
		log.Printf("[#%d] pickup item %d: inventory full", s.ID, req.itemID)
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.Inv[slot] = model.Item{}
		log.Printf("[#%d] pickup canceled after save failure: %v", s.ID, err)
		return
	}
	w.publishItemRemove(g)
	w.unregisterGroundItem(g)
	// 0x171 acknowledges pickup; 0x182 fixes the authoritative slot.
	s.Send(wire.CNFGetItem(uint32(placeInv), uint32(slot)))
	s.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
	log.Printf("[#%d] picked up item %d from the ground -> inv[%d]", s.ID, g.Item.Index, slot)
}

// onDisconnect persists and removes players on the World loop.
// With the staged owner, final account/sidecar persistence retains identity
// until actor acknowledgement, followed by serialized instance persistence.
// The default production path remains synchronous until S2 activation gates.
func (w *World) onDisconnect(s *net.Session) {
	delete(w.authPending, s)
	delete(w.security, s)
	w.releaseAuthenticatedClientSlot(s)
	if w.deferCriticalDisconnect(s) {
		w.resumeCriticalDisconnects(criticalCompletionBatchLimit)
		return
	}
	if p, ok := w.players[s]; ok {
		// Account and character state form one PostgreSQL aggregate.
		// After physical disconnect there is no reply to defer, but failure
		// must poison RAM to prevent saving a later incomplete snapshot.
		if p.InWorld && p.Account != nil && p.Char != nil && !p.PersistencePoisoned {
			if err := w.saveAccountAndCharStateResult(p); err != nil {
				p.PersistencePoisoned = true
				log.Printf("[#%d] ERROR saving full state for %q: %v", s.ID, p.Account.Name, err)
			}
		} else if p.Account != nil && !p.PersistencePoisoned {
			if err := w.saveAccount(p.Account); err != nil {
				p.PersistencePoisoned = true
				log.Printf("[#%d] ERROR saving account %q: %v", s.ID, p.Account.Name, err)
			}
		}
		w.finishDisconnect(s, p)
	}
}

// The legacy path persists instance membership inline. The staged lifecycle
// performs these steps separately and calls finishDisconnectAfterInstances only
// after its detached instance snapshot has been acknowledged on World.
func (w *World) finishDisconnect(s *net.Session, p *Player) {
	w.detachPlayerFromItemInstances(p.ID, w.now())
	// Persist the UID-based Water membership immediately. Waiting for the
	// next autosave would make a clean disconnect look like a lost room after
	// a process restart.
	w.flushInstanceStateIfDirty()
	w.finishDisconnectAfterInstances(s, p)
}

func (w *World) finishDisconnectAfterInstances(s *net.Session, p *Player) {
	w.unregisterPlayerSpatial(p)
	w.closeGhostShop(p, "disconnect")
	w.cancelTrade(p, "disconnect")
	// Remove party membership and visibility first so clients cannot
	// retain ghost characters or party members after socket closure.
	w.removePartyPlayerMode(p, w.rememberNightmareParty(p))
	// Despawn owner summons on disconnect; orphans must not follow
	// the next player to reuse this ID.
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
	log.Printf("[#%d] player removed", s.ID)
}

func accountSessionKey(name string) string {
	return strings.ToLower(strings.TrimSpace(name))
}

// claimAccountSession runs only on the World loop. Authenticate first to avoid
// exposing account presence. Ownership ends after persistence and cleanup, not
// when the transport closes: a queued disconnect still owns live mutable state.
func (w *World) claimAccountSession(s *net.Session, name string) bool {
	if !w.transferAdmissionAllowed(name, s) {
		return false
	}
	// Authentication may have read an old snapshot before a critical commit.
	// Do not reach the gameplay reload until World has applied its completion.
	if w.criticalAccountReserved(name) {
		return false
	}
	if w.accountSessions == nil {
		w.accountSessions = make(map[string]*net.Session)
	}
	key := accountSessionKey(name)
	if active := w.accountSessions[key]; active != nil && active != s {
		return false
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

// characterNameTaken uses the in-memory index if available (O(1),
// no disk access); otherwise fall back to the legacy store scan.
func (w *World) characterNameTaken(name string) (bool, error) {
	if w.charNames != nil {
		_, taken := w.charNames[strings.ToLower(name)]
		return taken, nil
	}
	return w.store.CharacterNameExists(name)
}

// mobByID finds a living NPC/monster by ClientId; nil if absent.
func (w *World) mobByID(id uint16) *Mob {
	m := w.mobsByID[id]
	if m == nil || m.Dead {
		return nil
	}
	return m
}

// addToInv uses the first visible slot. The wire array has 64 entries,
// but index 63 has no cell in the 7.48 client UI.
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

// slotOf resolves (type,pos) to an item pointer; nil if invalid.
func slotOf(ch *model.Char, typ, pos byte) *model.Item {
	switch typ {
	case placeEquip:
		if clientEquipSlotSupported(pos) && int(pos) < len(ch.Equip) {
			return &ch.Equip[pos]
		}
	case placeInv:
		if int(pos) < model.PlayerCarrySlots {
			return &ch.Inv[pos]
		}
	}
	return nil
}

// clientEquipSlotSupported maintains the WYD-Go 7.48 projection.
// Equip retains 16 positions for historical data and wire compatibility.
// Native necklace slot 9 is supported; later NewSlot/Belt entries are not.
func clientEquipSlotSupported(pos byte) bool {
	return pos < model.MaxEquipSlots
}

func clientEquipProjection(ch *model.Char) [model.MaxEquipSlots]model.Item {
	var equip [model.MaxEquipSlots]model.Item
	if ch == nil {
		return equip
	}
	equip = ch.Equip
	equip[0] = wire.CharacterFaceItem(*ch)
	for slot := range equip {
		if !clientEquipSlotSupported(byte(slot)) {
			equip[slot] = model.Item{}
		}
	}
	return equip
}

// migrateUnsupportedEquipment makes old saves usable without deletion.
// Move items incorrectly stored in Necklace to visible Carry, then Cargo;
// persistence failure rolls back the entire migration.
func (w *World) migrateUnsupportedEquipment(p *Player) {
	if p == nil || p.Char == nil || p.Account == nil || p.Char.Equip[9].Index == 0 {
		return
	}
	sessionID := int64(0)
	if p.Session != nil {
		sessionID = p.Session.ID
	}
	item := p.Char.Equip[9]
	if def, ok := w.items[item.Index]; ok && def.Pos&(1<<9) != 0 {
		return
	}
	snapshot := snapshotTimedAccountItems(p.Account)
	if !w.planUnsupportedEquipment(p) {
		log.Printf("[#%d] incompatible equipment %d retained in Equip[9]: Carry and Cargo full",
			sessionID, item.Index)
		return
	}
	if err := w.saveAccount(p.Account); err != nil {
		restoreTimedAccountItems(p.Account, snapshot)
		log.Printf("[#%d] failed to migrate incompatible equipment %d: %v", sessionID, item.Index, err)
		return
	}
	log.Printf("[#%d] incompatible equipment %d migrated from Equip[9] into storage", sessionID, item.Index)
}

// planUnsupportedEquipment changes only the supplied account. A staged entry
// uses a detached account and persists this move with timed-item preparation.
func (w *World) planUnsupportedEquipment(p *Player) bool {
	if p == nil || p.Char == nil || p.Account == nil || p.Char.Equip[9].Index == 0 {
		return false
	}
	item := p.Char.Equip[9]
	// Keep valid necklaces equipped. The old blanket slot migration made even
	// native Mystic Amulets disappear from equipment on every login. Unknown
	// or wrong-position legacy items still use the lossless storage migration.
	if def, ok := w.items[item.Index]; ok && def.Pos&(1<<9) != 0 {
		return false
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		if p.Char.Inv[slot].Index != 0 {
			continue
		}
		p.Char.Equip[9], p.Char.Inv[slot] = model.Item{}, item
		return true
	}
	for slot := 0; slot < model.PlayerCargoSlots; slot++ {
		if p.Account.Cargo[slot].Index != 0 {
			continue
		}
		p.Char.Equip[9], p.Account.Cargo[slot] = model.Item{}, item
		return true
	}
	return false
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

// affectBMTransform is the BeastMaster form (skills 64/66/68/70/71). Value is
// the form ID (Wolf 1 to Eden 5), not a potency.
const affectBMTransform = 16

// affectFaceTransform is cosmetic face transformation (volatiles 70-77).
// Value is the monster mesh; unlike BM affect 16, there is no combat effect.
// face_restore (89) removes it.
const affectFaceTransform = 40

// affectFaceTransformClientType is the visual slot sent for the face
// transformation. The client reads affect word 40 as a stun (it sets
// m_cCantMove, m_cCantAttk and the skill block), which froze a transformed
// player; slot 16 is the native Transformation icon and sets no state flag.
const affectFaceTransformClientType = 16

// bodyMesh extracts 16 visual equipment indices (CreateMob ItemEff).
// bodyAncient returns each slot's AnctCode (ancient color/refinement)
// for CreateMob anct@130; without it dye color cannot reach the world.
// The mesh carries only refinement. Mount state bytes are excluded:
// they are not refinement codes.
func bodyAncient(ch *model.Char) []byte {
	if ch == nil {
		return nil
	}
	a := make([]byte, len(ch.Equip))
	for i, it := range ch.Equip {
		if clientEquipSlotSupported(byte(i)) && !model.IsMount(it.Index) {
			a[i] = model.AncientCode(it)
		}
	}
	faces := [...]uint16{22, 23, 24, 25, 32}
	now := time.Now()
	for i := range ch.Affects {
		if af := &ch.Affects[i]; af.Type == 16 && af.ExpiresAt.After(now) && af.Value >= 1 && af.Value <= len(faces) {
			a[0] = model.AncientCode(transformFace(ch, af.Value, faces[af.Value-1]))
			break
		}
	}
	return a
}

// transformBaseSanc is pTransBonus[].BaseSanc (W2PP Basedef.cpp): Wolf,
// Bear, Astaroth, Titan and Eden.
var transformBaseSanc = [...]int{15, 60, 115, 155, 155}

// transformFace builds the transformed face the client draws. W2PP
// GetCurrentScore Type 16 refines it from the fourth mastery (Nature) and the
// level, so the beast glows brighter as Nature grows (0..+9).
func transformFace(ch *model.Char, value int, face uint16) model.Item {
	item := model.Item{Index: face}
	if ch == nil || value < 1 || value > len(transformBaseSanc) {
		return item
	}
	nature := minInt(320, int(playerMastery(ch, 3)))
	level := int(playerLevel(ch))
	if isCelestialEvolution(ch) {
		level += 399 // W2PP MAX_LEVEL for every evolution above Arch.
	}
	sanc := ((nature+level*2)/3 - transformBaseSanc[value-1]) / 12
	sanc = clampInt(sanc, 0, 9)
	if sanc > 0 {
		item.Eff[0], item.Eff[1] = 43, byte(sanc)
	}
	return item
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
		if clientEquipSlotSupported(byte(i)) {
			m[i] = model.VisualItemCode(it, model.IsMount(it.Index))
		}
	}
	faces := [...]uint16{22, 23, 24, 25, 32}
	for i := range ch.Affects {
		a := &ch.Affects[i]
		// Cosmetic monster-face transformation takes its mesh from Value.
		if a.Type == affectFaceTransform && a.ExpiresAt.After(now) && a.Value > 0 {
			m[0] = uint16(a.Value)
			break
		}
		if a.Type == 16 && a.ExpiresAt.After(now) && a.Value >= 1 && a.Value <= len(faces) {
			m[0] = model.VisualItemCode(transformFace(ch, a.Value, faces[a.Value-1]), false)
			break
		}
	}
	return m
}

// cstr reads a byte-slice C-string through the first NUL.
func cstr(b []byte) string {
	for i := range b {
		if b[i] == 0 {
			return string(b[:i])
		}
	}
	return string(b)
}
