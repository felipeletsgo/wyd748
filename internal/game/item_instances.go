package game

import (
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const instanceSignalID = uint16(0x7530)

type ItemInstance struct {
	Config model.VolatileInstance
	// RuntimeID e a chave efetiva no World. Para execucoes privadas coincide
	// com Config.ID; para zonas compartilhadas usa shared:<SharedGroup>.
	RuntimeID string
	LeaderID  uint16
	MemberIDs []uint16
	// Character UIDs are kept only for private Water reconnects. They are not
	// sent on the wire and are never used as live entity IDs.
	MemberCharacterUIDs []string
	LeaderCharacterUID  string
	MobIDs              map[uint16]struct{}
	Remaining           int
	CurrentStage        int
	Deadline            time.Time
	HardDeadline        time.Time
	ScheduleEnd         time.Time
	TransitionAt        time.Time
	QuizAt              time.Time
	ExitAt              time.Time
	RewardGranted       bool
	// Prazos independentes. Deadline/TransitionAt/QuizAt/ExitAt permanecem
	// como aliases de compatibilidade com fixtures antigas, mas o runtime novo
	// nunca usa o deadline de combate para cancelar uma transicao ou quiz.
	CombatDeadline     time.Time
	TransitionDeadline time.Time
	QuizDeadline       time.Time
	ExitDeadline       time.Time
	BossSpawned        bool
	// Estado runtime de maquinas especiais. O estado nunca e inferido de
	// pacotes do client: cada mob criado pela maquina carrega sua quadrante
	// autoritativa ate a morte.
	State                   string
	HellGateVariant         int
	HellGateClearedMask     uint8
	HellGateLichSpawnedMask uint8
	HellGateValidLichMask   uint8
	HellGateWrongLich       bool
	MobQuadrants            map[uint16]uint8
	HellGateLichIDs         map[uint8]uint16
	NPCIDs                  map[uint16]struct{}
	// UxmalRoom/Slot identificam a sala e a vaga da Pista de Runas. O
	// configurador reduz a execucao a uma unica stage por grupo, mas mantem a
	// origem para recompensas e auditoria.
	UxmalRoom int
	UxmalSlot int
}

const itemInstanceGeneratorReserveRadius = 8

func instanceRuntimeKey(cfg *model.VolatileInstance) string {
	if cfg == nil {
		return ""
	}
	if strings.TrimSpace(cfg.SharedGroup) != "" {
		return "shared:" + strings.TrimSpace(cfg.SharedGroup)
	}
	return cfg.ID
}

// expectedChainItem is the single server-side rule for the ticket accepted
// during exit grace. RewardItem remains the normal room-to-room reward; a
// terminal room can set ChainNextItem to accept a ticket it does not grant.
func expectedChainItem(cfg *model.VolatileInstance) uint16 {
	if cfg == nil {
		return 0
	}
	if cfg.ChainNextItem != 0 {
		return cfg.ChainNextItem
	}
	return cfg.RewardItem
}

// nextItemInstanceRuntimeID keeps the configured instance ID as the readable
// key for the first live execution and allocates a collision-free suffix for
// a new cycle while the previous room is still in exit grace. Config.ID is
// still persisted separately, so restoring a suffixed runtime uses the same
// authoritative data template.
func (w *World) nextItemInstanceRuntimeID(cfg *model.VolatileInstance) string {
	base := instanceRuntimeKey(cfg)
	if base == "" || w == nil {
		return base
	}
	if _, occupied := w.itemInstances[base]; !occupied {
		return base
	}
	for suffix := 1; ; suffix++ {
		candidate := fmt.Sprintf("%s:%d", base, suffix)
		if _, occupied := w.itemInstances[candidate]; !occupied {
			return candidate
		}
	}
}

func instanceMode(cfg model.VolatileInstance) string {
	mode := strings.ToLower(strings.TrimSpace(cfg.Mode))
	if mode != "" {
		return mode
	}
	if cfg.SharedEntry {
		return "private_shared_entry"
	}
	return "private_chain"
}

func sharedTimedInstance(cfg model.VolatileInstance) bool {
	return instanceMode(cfg) == "shared_timed_zone"
}

// instanceAdmissionOnlySchedule means ScheduleEnd closes admission but does
// not end the encounter. Hell Gate has a short entry window followed by a
// separate combat deadline.
func instanceAdmissionOnlySchedule(inst *ItemInstance) bool {
	if inst == nil {
		return false
	}
	return sharedTimedInstance(inst.Config) &&
		strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "hell_gate")
}

func instanceFinishPolicy(cfg model.VolatileInstance) string {
	policy := strings.ToLower(strings.TrimSpace(cfg.FinishPolicy))
	if policy != "" {
		return policy
	}
	if sharedTimedInstance(cfg) {
		return "respawn_until_timeout"
	}
	if len(cfg.Stages) > 1 {
		return "advance_on_clear"
	}
	return "finish_on_clear"
}

// instanceCompletesOnClear keeps policy selection explicit. A template that
// says finish_on_timeout must never inherit the old "Remaining == 0" shortcut,
// while state machines are completed only by their controller.
func instanceCompletesOnClear(inst *ItemInstance) bool {
	if inst == nil {
		return false
	}
	switch instanceFinishPolicy(inst.Config) {
	case "finish_on_timeout", "respawn_until_timeout", "state_machine":
		return false
	default:
		return true
	}
}

func instanceCombatDeadline(inst *ItemInstance) time.Time {
	if inst == nil {
		return time.Time{}
	}
	if !inst.CombatDeadline.IsZero() {
		// Fixtures e mundos migrados ainda podem alterar o alias Deadline
		// diretamente. Honrar o menor prazo evita manter uma sala antiga viva
		// depois de um timeout administrativo.
		if !inst.Deadline.IsZero() && inst.Deadline.Before(inst.CombatDeadline) {
			return inst.Deadline
		}
		return inst.CombatDeadline
	}
	return inst.Deadline
}

func setInstanceCombatDeadline(inst *ItemInstance, deadline time.Time) {
	if inst == nil {
		return
	}
	inst.CombatDeadline = deadline
	inst.Deadline = deadline
}

// generatorReservedForItemInstance identifica os spawns permanentes que
// pertencem a uma sala configurada. Algumas distribuicoes posicionam o
// NPCGener quatro tiles ao lado do centro usado por _MSG_UseItem, por isso a
// tolerancia pequena e intencional. Geradores de respawn normais continuam
// intactos fora dessas areas.
func (w *World) generatorReservedForItemInstance(g model.NPCGener) bool {
	reservedAt := func(rule model.VolatileRule, x, y uint16) bool {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			return false
		}
		cfg := rule.Instance
		for _, stage := range instanceStages(cfg) {
			radius := stage.AreaRadius
			if radius < itemInstanceGeneratorReserveRadius {
				radius = itemInstanceGeneratorReserveRadius
			}
			if chebyshev(x, y, stage.X, stage.Y) <= radius {
				return true
			}
		}
		return false
	}
	intersectsRoute := func(rule model.VolatileRule, x1, y1, x2, y2 uint16) bool {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			return false
		}
		for _, stage := range instanceStages(rule.Instance) {
			radius := stage.AreaRadius
			if radius < itemInstanceGeneratorReserveRadius {
				radius = itemInstanceGeneratorReserveRadius
			}
			minX, maxX := int(x1), int(x2)
			if minX > maxX {
				minX, maxX = maxX, minX
			}
			minY, maxY := int(y1), int(y2)
			if minY > maxY {
				minY, maxY = maxY, minY
			}
			// Os movimentos de NPCGener seguem trechos monotonicos entre
			// pontos. A caixa expandida e uma reserva conservadora: se o
			// trecho puder cruzar a area da sala, o gerador nao pode ser
			// reativado ali.
			if int(stage.X) >= minX-radius && int(stage.X) <= maxX+radius &&
				int(stage.Y) >= minY-radius && int(stage.Y) <= maxY+radius {
				return true
			}
		}
		return false
	}
	// A generator can have up to five route points.  The first point is only
	// the spawn origin; later points can also cross a private room and would
	// otherwise keep repopulating it when a mob follows its route.
	for _, segment := range g.Segments {
		if segment.X == 0 || segment.Y == 0 {
			continue
		}
		for _, rule := range w.volatiles.Rules {
			if reservedAt(rule, segment.X, segment.Y) {
				return true
			}
		}
		for _, rule := range w.volatiles.Items {
			if reservedAt(rule, segment.X, segment.Y) {
				return true
			}
		}
	}
	// Reserve tambem o corredor entre pontos consecutivos. O mob caminha
	// coordenada a coordenada e nao apenas salta entre os endpoints; testar
	// somente os pontos permitia que ele atravessasse uma sala privada.
	var previous *model.GenerSegment
	for index := range g.Segments {
		segment := &g.Segments[index]
		if segment.X == 0 || segment.Y == 0 {
			continue
		}
		if previous != nil {
			for _, rule := range w.volatiles.Rules {
				if intersectsRoute(rule, previous.X, previous.Y, segment.X, segment.Y) {
					return true
				}
			}
			for _, rule := range w.volatiles.Items {
				if intersectsRoute(rule, previous.X, previous.Y, segment.X, segment.Y) {
					return true
				}
			}
		}
		previous = segment
	}
	return false
}

// validateItemInstanceTemplates valida no boot tanto as regras por codigo
// quanto os overrides por item. Antes, apenas os overrides eram percorridos;
// isso deixava Cube/Big Cube/Hell Gate (que nascem em rules) falharem somente
// depois de o jogador consumir o convite.
func (w *World) validateItemInstanceTemplates() error {
	validated := make(map[string]struct{})
	validate := func(source string, rule model.VolatileRule) error {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			return nil
		}
		cfg := rule.Instance
		if _, done := validated[cfg.ID]; done {
			return nil
		}
		validated[cfg.ID] = struct{}{}
		spawns := cfg.Spawns
		if len(cfg.Stages) > 0 {
			spawns = nil
			for _, stage := range cfg.Stages {
				spawns = append(spawns, stage.Spawns...)
				spawns = append(spawns, stage.CompletionSpawns...)
			}
		}
		if cfg.HellGate != nil {
			spawns = append(spawns, cfg.HellGate.FinalSpawns...)
			for _, quadrant := range cfg.HellGate.Quadrants {
				spawns = append(spawns, quadrant.Spawns...)
				spawns = append(spawns, quadrant.Lich)
			}
			for _, spawn := range cfg.HellGate.FinalNPCs {
				def := w.npcDefByName(spawn.NPC)
				if spawn.Count <= 0 || def == nil || def.IsMonster() || def.Extended == nil {
					return fmt.Errorf("volatile %s: template NPC final %q ausente ou hostil", source, spawn.NPC)
				}
			}
		}
		for _, spawn := range spawns {
			def := w.npcDefByName(spawn.NPC)
			if spawn.Count <= 0 || def == nil || !def.IsMonster() ||
				def.Extended == nil {
				return fmt.Errorf("volatile %s: template de instancia %q ausente ou nao-monstro",
					source, spawn.NPC)
			}
		}
		return nil
	}
	for code, rule := range w.volatiles.Rules {
		if err := validate(fmt.Sprintf("code %d", code), rule); err != nil {
			return err
		}
	}
	for itemID, rule := range w.volatiles.Items {
		if err := validate(fmt.Sprintf("item %d", itemID), rule); err != nil {
			return err
		}
	}
	return nil
}

func instancePartyMembers(leader *Player, partyMode string) ([]*Player, bool) {
	if leader == nil {
		return nil, false
	}
	if partyMode == "solo" {
		return []*Player{leader}, true
	}
	if leader.Party == nil {
		if partyMode == "party" {
			return nil, false
		}
		return []*Player{leader}, true
	}
	if len(leader.Party.Members) == 0 || leader.Party.Members[0] != leader {
		return nil, false
	}
	members := make([]*Player, 0, len(leader.Party.Members))
	for _, member := range leader.Party.Members {
		if member != nil && member.InWorld && member.Char != nil && playerCurHP(member.Char) > 0 {
			members = append(members, member)
		}
	}
	return members, len(members) > 0
}

func instanceStages(cfg *model.VolatileInstance) []model.VolatileInstanceStage {
	if cfg == nil {
		return nil
	}
	if len(cfg.Stages) > 0 {
		return cfg.Stages
	}
	return []model.VolatileInstanceStage{{
		Name: cfg.Name, X: cfg.X, Y: cfg.Y, SpawnX: cfg.SpawnX, SpawnY: cfg.SpawnY,
		AreaRadius: cfg.AreaRadius, DurationSeconds: cfg.DurationSeconds, Spawns: cfg.Spawns,
	}}
}

func instanceSpawnCount(cfg *model.VolatileInstance) int {
	stages := instanceStages(cfg)
	if len(stages) == 0 {
		return 0
	}
	total := 0
	for _, spawn := range stages[0].Spawns {
		total += spawn.Count
	}
	return total
}

func instanceAllowsEvolution(ch *model.Char, allowed []string) bool {
	if ch == nil || len(allowed) == 0 {
		return true
	}
	actual := ch.Evolution
	if actual == "" {
		actual = "mortal"
	}
	for _, evolution := range allowed {
		if matchesEvolution(ch, evolution) ||
			(actual == "subcelestial" && evolution == "subcelestial") {
			return true
		}
	}
	return false
}

// instanceAreaOccupied reproduz o GetUserInArea nativo: uma sala fisicamente
// ocupada nao pode ser sobrescrita mesmo se o mapa de instancias estiver vazio
// (por exemplo, depois de reload de dados ou teleporte externo). Os membros
// que estao prestes a entrar sao ignorados: no Water/Cube o ponto de entrada e
// o proprio centro da primeira sala, portanto contar o lider aqui recusaria
// toda abertura legitima.
func (w *World) instanceAreaOccupied(cfg *model.VolatileInstance,
	ignored ...*Player) bool {
	if cfg == nil {
		return true
	}
	ignoredPlayers := make(map[*Player]struct{}, len(ignored))
	for _, player := range ignored {
		if player != nil {
			ignoredPlayers[player] = struct{}{}
		}
	}
	for _, player := range w.playersByID {
		if player == nil || !player.InWorld {
			continue
		}
		// Ignore a member only when the config explicitly declares this tile as
		// an entry point.  For legacy/custom rooms without EntryAreas, a party
		// member already standing on the stage is a real physical occupation and
		// must keep the old anti-overlap behavior.
		if _, isEntering := ignoredPlayers[player]; isEntering &&
			len(cfg.EntryAreas) > 0 && instanceTargetAllowedAt(cfg, player.X, player.Y) {
			continue
		}
		for _, stage := range instanceStages(cfg) {
			if chebyshev(player.X, player.Y, stage.X, stage.Y) <= stage.AreaRadius {
				return true
			}
		}
	}
	return false
}

func (w *World) saveAccountsAtomic(accounts ...*model.Account) error {
	for _, account := range accounts {
		pinAccountEntryPositions(account)
	}
	if batch, ok := w.store.(tradeBatchStore); ok {
		return batch.SaveAccounts(accounts...)
	}
	// SaveGameState also provides one database transaction for all accounts.
	// Some lightweight stores expose only this wider primitive because the same
	// operation can include guild state when needed.  Passing nil deliberately
	// means "accounts only"; guild commands use saveGuildState instead.
	if gameState, ok := w.store.(guildStore); ok {
		return gameState.SaveGameState(nil, accounts...)
	}
	if len(accounts) > 1 {
		return fmt.Errorf("store atual nao suporta transacao multi-conta")
	}
	if len(accounts) == 1 {
		return w.saveAccount(accounts[0])
	}
	return nil
}

func (w *World) planInstancePositions(members []*Player, x, y uint16) ([][2]uint16, bool) {
	return w.planInstancePositionsIgnoring(members, x, y, nil)
}

// planInstancePositionsIgnoring permite que os membros que estao sendo
// movidos em conjunto ignorem as suas posicoes antigas. Sem isso, uma
// transicao para uma sala proxima podia recusar o movimento porque o primeiro
// membro ainda ocupava o tile que o segundo deveria receber.
func (w *World) planInstancePositionsIgnoring(members []*Player, x, y uint16,
	ignored map[*Player]struct{}) ([][2]uint16, bool) {
	return w.planInstancePositionsIgnoringForInstance(members, x, y, ignored, "")
}

func (w *World) planInstancePositionsIgnoringForInstance(members []*Player,
	x, y uint16, ignored map[*Player]struct{}, instanceID string) ([][2]uint16, bool) {
	result := make([][2]uint16, 0, len(members))
	reserved := make(map[uint32]struct{}, len(members))
	for _, member := range members {
		found := false
		for distance := 0; distance <= 4 && !found; distance++ {
			for dy := -distance; dy <= distance && !found; dy++ {
				for dx := -distance; dx <= distance; dx++ {
					if distance > 0 && absInt(dx) != distance && absInt(dy) != distance {
						continue
					}
					nx, ny := int(x)+dx, int(y)+dy
					if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
						continue
					}
					ux, uy := uint16(nx), uint16(ny)
					key := uint32(ux)<<16 | uint32(uy)
					if _, used := reserved[key]; used ||
						w.positionOccupiedExceptPlayersInInstance(ux, uy, nil, member, ignored, instanceID) {
						continue
					}
					reserved[key] = struct{}{}
					result = append(result, [2]uint16{ux, uy})
					found = true
					break
				}
			}
		}
		if !found {
			return nil, false
		}
	}
	return result, true
}

func itemInstanceHasMember(inst *ItemInstance, playerID uint16) bool {
	if inst == nil {
		return false
	}
	for _, id := range inst.MemberIDs {
		if id == playerID {
			return true
		}
	}
	return false
}

// ensureItemInstanceLeader keeps rewards deliverable when the original leader
// leaves during a quiz or is removed by a stale/disconnect cleanup.  Native
// party instances always have one live leader; retaining a departed ID would
// leave the final Water/Cube reward pending forever.
func ensureItemInstanceLeader(inst *ItemInstance) {
	if inst == nil || itemInstanceHasMember(inst, inst.LeaderID) {
		return
	}
	if len(inst.MemberIDs) == 0 {
		inst.LeaderID = 0
		return
	}
	inst.LeaderID = inst.MemberIDs[0]
}

// itemInstanceInExitGrace representa os dez segundos nativos entre a morte
// do ultimo mob e o recall da sala. A recompensa ja foi confirmada, portanto
// a sala nao deve bloquear o lider de consumir o pergaminho seguinte da
// cadeia Water. O cleanup ainda conserva a lista antiga ate observar a nova
// instancia e, nesse caso, nao teleporta o jogador de volta para a cidade.
func itemInstanceInExitGraceAt(inst *ItemInstance, now time.Time) bool {
	return inst != nil && inst.Config.AllowChainDuringExitGrace && expectedChainItem(&inst.Config) != 0 &&
		inst.RewardGranted && !inst.ExitAt.IsZero() && now.Before(inst.ExitAt)
}

// itemInstanceInExitGrace remains a small clock-independent helper for pure
// tests. World paths use itemInstanceInExitGraceAt with World.now(), so a
// ticket cannot be accepted after ExitAt merely because the cleanup tick has
// not run yet.
func itemInstanceInExitGrace(inst *ItemInstance) bool {
	return itemInstanceInExitGraceAt(inst, time.Now())
}

// itemInstanceForPlayer is the authoritative membership lookup.  A player
// must belong to at most one live instance at a time; keeping this invariant
// here prevents the cleanup of an older room from teleporting a player out of
// a newer Water/Cube room.
func (w *World) itemInstanceForPlayer(playerID uint16) *ItemInstance {
	now := w.now()
	for _, inst := range w.itemInstances {
		if inst != nil && !itemInstanceInExitGraceAt(inst, now) &&
			itemInstanceHasMember(inst, playerID) {
			return inst
		}
	}
	return nil
}

func (w *World) itemInstanceExitGraceForPlayer(playerID uint16) *ItemInstance {
	now := w.now()
	var latest *ItemInstance
	for _, inst := range w.itemInstances {
		if inst != nil && itemInstanceInExitGraceAt(inst, now) &&
			itemInstanceHasMember(inst, playerID) {
			if latest == nil || inst.ExitAt.After(latest.ExitAt) ||
				(inst.ExitAt.Equal(latest.ExitAt) && inst.RuntimeID > latest.RuntimeID) {
				latest = inst
			}
		}
	}
	return latest
}

// itemInstanceExitGraceAllowsItem is deliberately narrower than the normal
// membership lookup: during exit grace only the configured successor is
// accepted, and with no Water membership only a data-marked chain root may
// start a new tier. Without this check a client could use a later ticket from
// a platform and bypass the completed rooms.
func (w *World) itemInstanceExitGraceAllowsItem(playerID uint16, itemID uint16) bool {
	inst := w.itemInstanceExitGraceForPlayer(playerID)
	if inst == nil {
		// A completed room remains indexed until the cleanup tick. Once ExitAt
		// passed, the player is still barred by that stale membership; otherwise
		// a ticket could be accepted from the old room during the cleanup gap.
		for _, candidate := range w.itemInstances {
			if candidate != nil && itemInstanceHasMember(candidate, playerID) {
				return false
			}
		}
		// A Water tier has exactly one entry ticket. This is data-driven through
		// ChainStart, so reaching a later platform cannot skip the completed
		// predecessor merely by using a later scroll there.
		if rule, _, ok := w.volatiles.Rule(itemID); ok && rule.Instance != nil &&
			isDurablePrivateWaterConfig(*rule.Instance) {
			return rule.Instance.ChainStart
		}
		return true
	}
	return expectedChainItem(&inst.Config) != 0 &&
		expectedChainItem(&inst.Config) == itemID
}

// detachInstanceMembersAfterCommit removes players from a completed Water
// room only after the next room has been durably accepted. Keeping the old
// membership until that boundary is what makes a failed ticket use reversible:
// the old RuntimeID and the ticket remain valid. The old instance stays
// indexed until exit grace expires so cleanup can remove stale entities
// without recalling a player already transferred to the new room.
func (w *World) detachInstanceMembersAfterCommit(inst *ItemInstance, playerIDs map[uint16]struct{}) {
	if inst == nil || len(playerIDs) == 0 {
		return
	}
	movedUIDs := make(map[string]struct{}, len(playerIDs))
	for playerID := range playerIDs {
		if p := w.playersByID[playerID]; p != nil && p.Char != nil {
			if uid := strings.TrimSpace(p.Char.UID); uid != "" {
				movedUIDs[uid] = struct{}{}
			}
		}
	}
	kept := inst.MemberIDs[:0]
	for _, id := range inst.MemberIDs {
		if _, moved := playerIDs[id]; !moved {
			kept = append(kept, id)
		}
	}
	inst.MemberIDs = kept
	// The old room stays indexed during exit grace, but it no longer owns the
	// characters that were transferred to the next runtime. Preserve detached
	// members that were not part of this transition; otherwise a party member
	// who was temporarily offline could never reattach to the old room.
	if len(movedUIDs) > 0 && len(inst.MemberCharacterUIDs) > 0 {
		keptUIDs := inst.MemberCharacterUIDs[:0]
		for _, uid := range inst.MemberCharacterUIDs {
			if _, moved := movedUIDs[strings.TrimSpace(uid)]; !moved {
				keptUIDs = append(keptUIDs, uid)
			}
		}
		inst.MemberCharacterUIDs = keptUIDs
	}
	if _, moved := movedUIDs[strings.TrimSpace(inst.LeaderCharacterUID)]; moved {
		inst.LeaderCharacterUID = ""
	}
	if pending := w.pendingInstanceMembers[inst.RuntimeID]; pending != nil {
		for uid := range movedUIDs {
			delete(pending, uid)
		}
		if len(pending) == 0 {
			delete(w.pendingInstanceMembers, inst.RuntimeID)
		}
	}
	if _, moved := movedUIDs[strings.TrimSpace(w.pendingInstanceLeaders[inst.RuntimeID])]; moved {
		delete(w.pendingInstanceLeaders, inst.RuntimeID)
	}
	ensureItemInstanceLeader(inst)
}

func (w *World) playerInOtherItemInstance(playerID uint16, current *ItemInstance) bool {
	for _, inst := range w.itemInstances {
		// Exit grace only relaxes admission to the next Water room.  It must
		// never make cleanup blind to a membership that was already transferred
		// (or restored by a reconnect), otherwise an expired older room can
		// teleport the player out of the newer room.
		if inst != nil && inst != current && itemInstanceHasMember(inst, playerID) {
			return true
		}
	}
	return false
}

// instanceForMob devolve a execucao que possui o mob. Mobs de instancia sao
// publicados no mesmo mapa fisico, mas a IA precisa trata-los como entidades
// privadas: somente os membros inscritos podem ser alvos e a sala e o limite
// de movimento. Se a instancia ja foi encerrada, o mob fica adormecido ate a
// limpeza do indice (defesa contra referencias antigas durante um tick).
func (w *World) instanceForMob(m *Mob) *ItemInstance {
	if w == nil || m == nil || m.InstanceID == "" {
		return nil
	}
	return w.itemInstances[m.InstanceID]
}

func instanceStageForMob(inst *ItemInstance) (model.VolatileInstanceStage, bool) {
	if inst == nil {
		return model.VolatileInstanceStage{}, false
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return model.VolatileInstanceStage{}, false
	}
	return stages[inst.CurrentStage], true
}

// instanceMobArea returns the smallest authoritative room for a mob. Hell
// Gate keeps each quadrant inside its own seal instead of letting the generic
// entrance radius (which covers the whole event) turn into a movement leak.
func instanceMobArea(inst *ItemInstance, mob *Mob) (uint16, uint16, int, bool) {
	if inst == nil || mob == nil {
		return 0, 0, 0, false
	}
	if cfg, ok := hellGateInstance(inst); ok && inst.MobQuadrants != nil {
		if id := inst.MobQuadrants[mob.ID]; id != 0 {
			for _, quadrant := range cfg.Quadrants {
				if uint8(quadrant.ID) == id {
					return quadrant.X, quadrant.Y, quadrant.AreaRadius, quadrant.AreaRadius > 0
				}
			}
		}
	}
	stage, ok := instanceStageForMob(inst)
	if !ok || stage.AreaRadius <= 0 {
		return 0, 0, 0, false
	}
	return stage.X, stage.Y, stage.AreaRadius, true
}

func instanceMemberInStage(inst *ItemInstance, p *Player) bool {
	if inst == nil || p == nil || !itemInstanceHasMember(inst, p.ID) {
		return false
	}
	stage, ok := instanceStageForMob(inst)
	if !ok || stage.AreaRadius <= 0 {
		return false
	}
	return chebyshev(p.X, p.Y, stage.X, stage.Y) <= stage.AreaRadius
}

func (w *World) instanceMobInsideArea(m *Mob) bool {
	inst := w.instanceForMob(m)
	x, y, radius, ok := instanceMobArea(inst, m)
	if !ok {
		return false
	}
	return chebyshev(m.X, m.Y, x, y) <= radius
}

func (w *World) instanceMobStepAllowed(m *Mob, x, y uint16) bool {
	inst := w.instanceForMob(m)
	areaX, areaY, radius, ok := instanceMobArea(inst, m)
	if !ok {
		return false
	}
	return chebyshev(x, y, areaX, areaY) <= radius
}

func (w *World) instanceMobTargetAllowed(m *Mob, p *Player) bool {
	if p == nil || !validMobTarget(p) {
		return false
	}
	return instanceMemberInStage(w.instanceForMob(m), p)
}

func (w *World) nearestInstanceMember(m *Mob) *Player {
	inst := w.instanceForMob(m)
	if inst == nil {
		return nil
	}
	var best *Player
	bestDistance := int(^uint(0) >> 1)
	for _, id := range inst.MemberIDs {
		p := w.playersByID[id]
		if !w.instanceMobTargetAllowed(m, p) {
			continue
		}
		distance := chebyshev(m.X, m.Y, p.X, p.Y)
		if distance < bestDistance {
			best, bestDistance = p, distance
		}
	}
	return best
}

func (w *World) instanceMobHasNearbyMember(m *Mob, radius int) bool {
	if m == nil || radius < 0 {
		return false
	}
	inst := w.instanceForMob(m)
	if inst == nil {
		return false
	}
	for _, id := range inst.MemberIDs {
		p := w.playersByID[id]
		if w.instanceMobTargetAllowed(m, p) &&
			chebyshev(m.X, m.Y, p.X, p.Y) <= radius {
			return true
		}
	}
	return false
}

func instanceScheduleEnd(cfg *model.VolatileInstance, now time.Time) (time.Time, bool) {
	if cfg == nil || len(cfg.Schedule) == 0 {
		return time.Time{}, true
	}
	// As janelas se repetem a cada hora. Calcular somente
	// (now.Minute()-StartMinute) falha quando uma janela atravessa 59:xx
	// (por exemplo, start=59, duration=240): depois da virada da hora o
	// valor fica negativo e a sala e recusada enquanto ainda esta aberta.
	secondOfHour := now.Minute()*60 + now.Second()
	var best time.Time
	for _, window := range cfg.Schedule {
		if window.DurationSeconds <= 0 || window.StartMinute < 0 ||
			window.StartMinute > 59 {
			continue
		}
		startSecond := window.StartMinute * 60
		elapsed := secondOfHour - startSecond
		if elapsed < 0 {
			elapsed += 60 * 60
		}
		if elapsed >= window.DurationSeconds {
			continue
		}
		end := now.Add(time.Duration(window.DurationSeconds-elapsed) * time.Second)
		if best.IsZero() || end.Before(best) {
			best = end
		}
	}
	if best.IsZero() {
		return time.Time{}, false
	}
	return best, true
}

func instanceTargetAllowedAt(cfg *model.VolatileInstance, x, y uint16) bool {
	_, ok := instanceEntryAreaAt(cfg, x, y)
	return ok
}

func instanceEntryAreaAt(cfg *model.VolatileInstance, x, y uint16) (model.VolatileInstanceEntryArea, bool) {
	if cfg == nil || len(cfg.EntryAreas) == 0 {
		return model.VolatileInstanceEntryArea{}, true
	}
	if x == 0 || y == 0 {
		return model.VolatileInstanceEntryArea{}, false
	}
	for _, area := range cfg.EntryAreas {
		if x >= area.MinX && x <= area.MaxX &&
			y >= area.MinY && y <= area.MaxY {
			return area, true
		}
	}
	return model.VolatileInstanceEntryArea{}, false
}

// detachPlayerFromItemInstances libera imediatamente a vaga de quem saiu do
// mundo. IDs de entidade mudam no próximo login; mantê-los na sala bloquearia
// uma vaga do Cube e poderia deixar a recompensa presa ao líder desconectado.
func (w *World) detachPlayerFromItemInstances(playerID uint16, now time.Time) {
	w.detachPlayerFromItemInstancesMode(playerID, now, true)
}

func (w *World) detachPlayerFromItemInstancesMode(playerID uint16, now time.Time,
	preservePrivateWater bool) {
	if playerID == 0 {
		return
	}
	for _, inst := range w.itemInstances {
		if inst == nil || !itemInstanceHasMember(inst, playerID) {
			continue
		}
		privateWater := isDurablePrivateWaterInstance(inst)
		member := w.playersByID[playerID]
		if !preservePrivateWater && privateWater && member != nil && member.Char != nil {
			uid := strings.TrimSpace(member.Char.UID)
			if pending := w.pendingInstanceMembers[inst.RuntimeID]; pending != nil {
				delete(pending, uid)
				if len(pending) == 0 {
					delete(w.pendingInstanceMembers, inst.RuntimeID)
				}
			}
			if strings.TrimSpace(w.pendingInstanceLeaders[inst.RuntimeID]) == uid {
				delete(w.pendingInstanceLeaders, inst.RuntimeID)
			}
			if inst.LeaderCharacterUID == uid {
				inst.LeaderCharacterUID = ""
			}
		}
		if preservePrivateWater && privateWater && member != nil && member.Char != nil &&
			strings.TrimSpace(member.Char.UID) != "" {
			uid := strings.TrimSpace(member.Char.UID)
			if w.pendingInstanceMembers == nil {
				w.pendingInstanceMembers = make(map[string]map[string]struct{})
			}
			pending := w.pendingInstanceMembers[inst.RuntimeID]
			if pending == nil {
				pending = make(map[string]struct{})
				w.pendingInstanceMembers[inst.RuntimeID] = pending
			}
			pending[uid] = struct{}{}
			if inst.LeaderID == playerID {
				inst.LeaderCharacterUID = uid
				if w.pendingInstanceLeaders == nil {
					w.pendingInstanceLeaders = make(map[string]string)
				}
				w.pendingInstanceLeaders[inst.RuntimeID] = uid
			}
		}
		members := make([]uint16, 0, len(inst.MemberIDs)-1)
		for _, id := range inst.MemberIDs {
			if id != playerID {
				members = append(members, id)
			}
		}
		inst.MemberIDs = members
		if len(members) == 0 {
			pendingCount := 0
			if w.pendingInstanceMembers != nil {
				pendingCount = len(w.pendingInstanceMembers[inst.RuntimeID])
			}
			keepPrivateWater := preservePrivateWater && privateWater && pendingCount > 0
			if keepPrivateWater {
				// Keep the private room alive while its characters are offline;
				// the durable UID map will reattach them on the next login.
				inst.LeaderID = 0
				w.markInstanceStateDirty()
				continue
			}
			// O tick comum remove os mobs e encerra a instância sem criar um
			// segundo caminho de cleanup durante logout/desconexão.
			inst.TransitionAt = time.Time{}
			inst.QuizAt = time.Time{}
			inst.TransitionDeadline = time.Time{}
			inst.QuizDeadline = time.Time{}
			inst.ExitAt = time.Time{}
			inst.ExitDeadline = time.Time{}
			setInstanceCombatDeadline(inst, now)
			w.markInstanceStateDirty()
			continue
		}
		if inst.LeaderID == playerID {
			inst.LeaderID = members[0]
		}
		ensureItemInstanceLeader(inst)
		w.markInstanceStateDirty()
	}
}

func remainingInstanceSeconds(deadline, now time.Time) int {
	if deadline.IsZero() || !deadline.After(now) {
		return 0
	}
	remaining := int(deadline.Sub(now).Seconds())
	if remaining < 1 {
		return 1
	}
	return remaining
}

// joinSharedItemInstance reproduz o ingresso individual do Cube: cada jogador
// consome o proprio convite e entra numa execucao ainda parada na primeira sala.
// Nenhum ingresso transporta automaticamente os demais membros da party.
func (w *World) joinSharedItemInstance(s *net.Session, p *Player, item *model.Item,
	slot byte, rule model.VolatileRule, inst *ItemInstance, req useItemRequest) {
	cfg := rule.Instance
	now := w.now()
	reject := func(reason string) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel(reason))
	}
	if cfg == nil || inst == nil {
		reject("This instance is not configured.")
		return
	}
	if !w.itemInstanceExitGraceAllowsItem(p.ID, item.Index) {
		reject("Only the next Water Scroll can be used during the exit window.")
		return
	}
	// O alvo do pacote 0x373 e controlado pelo cliente. A fonte nativa decide
	// a entrada pela posição autoritativa do personagem (TargetX/TargetY), não
	// por GridX/GridY forjáveis; usar o campo do pacote permitia abrir Water ou
	// Nightmare a partir de qualquer mapa enviando as coordenadas da sala.
	if !instanceTargetAllowedAt(cfg, p.X, p.Y) {
		reject("This ticket cannot be used from here.")
		return
	}
	if !cfg.SharedEntry {
		reject("This instance does not accept individual entries.")
		return
	}
	if itemInstanceHasMember(inst, p.ID) {
		reject("You are already inside this instance.")
		return
	}
	if active := w.itemInstanceForPlayer(p.ID); active != nil {
		reject("You are already inside another instance.")
		return
	}
	if inst.CurrentStage != 0 || !inst.TransitionAt.IsZero() || !inst.QuizAt.IsZero() ||
		!inst.ExitAt.IsZero() {
		reject("This instance is already in progress.")
		return
	}
	combatDeadline := instanceCombatDeadline(inst)
	if inst.Remaining <= 0 || (!combatDeadline.IsZero() && !now.Before(combatDeadline)) ||
		(!inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)) ||
		(!inst.ScheduleEnd.IsZero() && !now.Before(inst.ScheduleEnd)) {
		reject("This instance is no longer accepting players.")
		return
	}
	if cfg.MaxPlayers > 0 && len(inst.MemberIDs) >= cfg.MaxPlayers {
		reject("This instance is full.")
		return
	}
	if _, err := nightmareEligibleMembers([]*Player{p}, cfg); err != nil {
		reject(err.Error())
		return
	}
	if !instanceAllowsEvolution(p.Char, cfg.AllowedEvolutions) {
		reject("Your evolution cannot enter this instance.")
		return
	}
	stages := instanceStages(&inst.Config)
	if len(stages) == 0 {
		reject("This instance has no configured rooms.")
		return
	}
	destinations, ok := w.planInstancePositions([]*Player{p}, stages[0].X, stages[0].Y)
	if !ok {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("The Cube entrance is blocked."))
		return
	}

	oldItem, oldX, oldY := *item, p.X, p.Y
	nightmareCharges := chargeNightmareEntries([]*Player{p}, cfg)
	oldMembers := append([]uint16(nil), inst.MemberIDs...)
	p.X, p.Y = destinations[0][0], destinations[0][1]
	p.Char.X, p.Char.Y = p.X, p.Y
	if rule.Consume {
		consumeOne(item)
	}
	// Registre a adesao antes do commit. Se o processo cair entre o commit e
	// o append, o item e a posicao persistidos nao podem ficar sem membro na
	// memoria; em falha restauramos tambem a lista de membros.
	inst.MemberIDs = append(inst.MemberIDs, p.ID)
	// A entrada individual altera simultaneamente o convite, a posição e a
	// lista de membros. Usar somente SaveGameState deixava o ticket confirmado
	// sem o membro no agregado durável de uma store que persiste instâncias.
	if err := w.saveAccountsAndInstanceState(p.Account); err != nil {
		restoreNightmareEntries(nightmareCharges)
		*item = oldItem
		p.X, p.Y = oldX, oldY
		p.Char.X, p.Char.Y = oldX, oldY
		inst.MemberIDs = oldMembers
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	w.markInstanceStateDirty()
	w.refreshPlayerVisibility(p)
	w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID,
		uint32(remainingInstanceSeconds(instanceCombatDeadline(inst), now))))
	p.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID,
		uint32(inst.Remaining)))
	name := stages[0].Name
	if name == "" {
		name = inst.Config.Name
	}
	p.Session.Send(wire.MessagePanel(name))
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	log.Printf("[#%d] INSTANCIA %q ingresso individual membros=%d",
		s.ID, cfg.ID, len(inst.MemberIDs))
}

// joinSharedTimedItemInstance admite uma nova party (ou jogador pessoal) na
// mesma zona fisica. Ao contrario do Cube, nenhuma copia de mobs e criada:
// todos os participantes apontam para o mesmo ItemInstance e enxergam a
// mesma onda. O ticket usado pelo solicitante e o unico consumido.
func (w *World) joinSharedTimedItemInstance(s *net.Session, p *Player, item *model.Item,
	slot byte, rule model.VolatileRule, inst *ItemInstance) {
	cfg := rule.Instance
	reject := func(reason string) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel(reason))
	}
	if cfg == nil || inst == nil || !sharedTimedInstance(*cfg) {
		reject("This instance is not configured for shared entry.")
		return
	}
	if !w.itemInstanceExitGraceAllowsItem(p.ID, item.Index) {
		reject("Only the next Water Scroll can be used during the exit window.")
		return
	}
	if !instanceTargetAllowedAt(cfg, p.X, p.Y) {
		reject("This ticket cannot be used from here.")
		return
	}
	now := w.now()
	scheduleEnd, scheduleOK := instanceScheduleEnd(cfg, now)
	if !scheduleOK || (!inst.ScheduleEnd.IsZero() && !now.Before(inst.ScheduleEnd)) ||
		(!inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)) {
		reject("This instance is no longer open.")
		return
	}
	runKey, runAllowed := w.nightmarePartyRunAllowed(cfg, rule.PartyMode, now)
	if !runAllowed {
		reject("The party limit for this Nightmare tier was reached.")
		return
	}
	if rule.PartyMode != "party" && rule.PartyMode != "solo" && rule.PartyMode != "" {
		reject("This entry mode is not configured.")
		return
	}
	members, ok := instancePartyMembers(p, rule.PartyMode)
	if !ok {
		reject("Only the party leader can enter this instance.")
		return
	}
	eligible, nightmareErr := nightmareEligibleMembers(members, cfg)
	if nightmareErr != nil {
		reject(nightmareErr.Error())
		return
	}
	members = eligible
	for _, member := range members {
		if member == nil || member.Char == nil || !member.InWorld ||
			itemInstanceHasMember(inst, member.ID) || w.itemInstanceForPlayer(member.ID) != nil {
			reject("You or a party member is already inside an instance.")
			return
		}
	}
	capacity := instancePlayerCapacity(cfg)
	if capacity > 0 && len(inst.MemberIDs)+len(members) > capacity {
		reject(fmt.Sprintf("This instance accepts at most %d players.", capacity))
		return
	}
	stage, ok := instanceStageForMob(inst)
	if !ok {
		reject("The shared instance has no active room.")
		return
	}
	ignored := make(map[*Player]struct{}, len(members))
	for _, member := range members {
		ignored[member] = struct{}{}
	}
	destinations, positionsOK := w.planInstancePositionsIgnoring(members, stage.X, stage.Y, ignored)
	if !positionsOK {
		reject("The instance entrance is blocked.")
		return
	}
	oldItem := *item
	oldMembers := append([]uint16(nil), inst.MemberIDs...)
	oldScheduleEnd := inst.ScheduleEnd
	type oldPosition struct {
		p    *Player
		x, y uint16
	}
	oldPositions := make([]oldPosition, 0, len(members))
	accounts := make([]*model.Account, 0, len(members))
	seenAccounts := make(map[*model.Account]struct{})
	for index, member := range members {
		oldPositions = append(oldPositions, oldPosition{member, member.X, member.Y})
		member.X, member.Y = destinations[index][0], destinations[index][1]
		member.Char.X, member.Char.Y = member.X, member.Y
		if member.Account != nil {
			if _, seen := seenAccounts[member.Account]; !seen {
				seenAccounts[member.Account] = struct{}{}
				accounts = append(accounts, member.Account)
			}
		}
	}
	if rule.Consume {
		consumeOne(item)
	}
	inst.MemberIDs = append(inst.MemberIDs, func() []uint16 {
		ids := make([]uint16, 0, len(members))
		for _, member := range members {
			ids = append(ids, member.ID)
		}
		return ids
	}()...)
	ensureItemInstanceLeader(inst)
	previousRuns := 0
	if runKey != "" {
		previousRuns = w.nightmarePartyRuns[runKey]
		w.recordNightmarePartyRun(runKey)
	}
	// Include the effective window end in the same durable snapshot as the
	// ticket and the party membership.  Otherwise a restart between this
	// commit and the next autosave could resurrect the zone with the previous
	// deadline.
	if inst.ScheduleEnd.IsZero() || scheduleEnd.After(inst.ScheduleEnd) {
		inst.ScheduleEnd = scheduleEnd
	}
	if err := w.saveAccountsAndInstanceState(accounts...); err != nil {
		*item = oldItem
		inst.MemberIDs = oldMembers
		inst.ScheduleEnd = oldScheduleEnd
		if runKey != "" {
			w.nightmarePartyRuns[runKey] = previousRuns
		}
		for _, old := range oldPositions {
			old.p.X, old.p.Y = old.x, old.y
			old.p.Char.X, old.p.Char.Y = old.x, old.y
		}
		reject("The instance could not be saved.")
		return
	}
	w.markInstanceStateDirty()
	for _, member := range members {
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte { return wire.ActionStop(member.ID, member.X, member.Y) })
	}
	p.Session.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	w.sendItemInstanceStageStatus(inst)
	log.Printf("[#%d] INSTANCIA %q ingresso em zona compartilhada membros=%d",
		s.ID, cfg.ID, len(inst.MemberIDs))
}

func (w *World) useInstanceTicket(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule, req useItemRequest) {
	cfg := rule.Instance
	if cfg == nil {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This instance is not configured."))
		return
	}
	now := w.now()
	if group := strings.TrimSpace(cfg.ExclusiveGroup); group != "" {
		for activeID, active := range w.itemInstances {
			if active == nil ||
				strings.TrimSpace(active.Config.ExclusiveGroup) != group {
				continue
			}
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Another instance of this type is already active."))
			log.Printf("[#%d] instancia recusada exclusiveGroup=%q ativa=%s", s.ID, group, activeID)
			return
		}
	}
	runtimeID := instanceRuntimeKey(cfg)
	// The previous Water room may have a different config ID (the boss is the
	// canonical example: its Room 1 ticket opens a new `water-*-1` runtime).
	// Resolve the player's own grace instance before looking up the next
	// template; otherwise the old boss membership would survive forever.
	graceSource := w.itemInstanceExitGraceForPlayer(p.ID)
	// Private Water executions are independent. The same configured room can
	// therefore have several live RuntimeIDs at once; only shared-timed events
	// use a single shared aggregate.
	if isDurablePrivateWaterConfig(*cfg) {
		runtimeID = w.nextItemInstanceRuntimeID(cfg)
	}
	if inst := w.itemInstances[runtimeID]; inst != nil {
		if sharedTimedInstance(*cfg) {
			w.joinSharedTimedItemInstance(s, p, item, slot, rule, inst)
			return
		}
		// A completed private room remains in the map during exit grace so its
		// cleanup can observe the transfer. It must not block the next room (and
		// especially must not collide with a new Room 1 cycle). The latest grace
		// instance already validated the exact next ticket for this player.
		if itemInstanceInExitGraceAt(inst, now) {
			if !w.itemInstanceExitGraceAllowsItem(p.ID, item.Index) {
				s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
				s.Send(wire.MessagePanel("Only the next Water Scroll can be used during the exit window."))
				return
			}
			if itemInstanceHasMember(inst, p.ID) {
				graceSource = inst
			}
			runtimeID = w.nextItemInstanceRuntimeID(cfg)
		} else {
			if !cfg.SharedEntry {
				s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
				s.Send(wire.MessagePanel("This instance is already occupied."))
				return
			}
			w.joinSharedItemInstance(s, p, item, slot, rule, inst, req)
			return
		}
	}
	if !instanceTargetAllowedAt(cfg, p.X, p.Y) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This ticket cannot be used from here."))
		return
	}
	if isDurablePrivateWaterConfig(*cfg) && cfg.ChainStart {
		if area, _ := instanceEntryAreaAt(cfg, p.X, p.Y); area.RequiresChain && graceSource == nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("This Room 1 Scroll requires a completed Water boss."))
			return
		}
	}
	scheduleEnd, scheduleOK := instanceScheduleEnd(cfg, now)
	if !scheduleOK {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This instance is not open right now."))
		return
	}
	runKey, runAllowed := w.nightmarePartyRunAllowed(cfg, rule.PartyMode, now)
	if !runAllowed {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("The party limit for this Nightmare tier was reached."))
		return
	}
	members, ok := instancePartyMembers(p, rule.PartyMode)
	if !ok {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		if p.Party != nil && len(p.Party.Members) > 0 && p.Party.Members[0] != p {
			s.Send(wire.MessagePanel("Only the party leader can start this instance."))
		} else if rule.PartyMode == "party" {
			s.Send(wire.MessagePanel("A party is required to enter this instance."))
		} else {
			s.Send(wire.MessagePanel("No eligible party member is available."))
		}
		return
	}
	eligibleMembers, nightmareErr := nightmareEligibleMembers(members, cfg)
	if nightmareErr != nil {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel(nightmareErr.Error()))
		return
	}
	members = eligibleMembers
	for _, member := range members {
		if member != nil && !w.itemInstanceExitGraceAllowsItem(member.ID, item.Index) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("Only the next Water Scroll can be used during the exit window."))
			return
		}
	}
	// A character (and every party member moved with it) can only be in one
	// active room.  Previously a player could open Water room 1, then open room
	// 2/3 before room 1 expired; each ItemInstance retained the same ID and its
	// later cleanup teleported the character out of the newer room.
	for _, member := range members {
		if active := w.itemInstanceForPlayer(member.ID); active != nil {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("You or a party member is already inside another instance."))
			return
		}
	}
	if !sharedTimedInstance(*cfg) && !isDurablePrivateWaterConfig(*cfg) &&
		w.instanceAreaOccupied(cfg, members...) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This room is already occupied."))
		return
	}
	stages := instanceStages(cfg)
	totalMobs := instanceSpawnCount(cfg)
	if len(stages) == 0 || (totalMobs <= 0 && !isBigCubeConfig(cfg)) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		if isBigCubeConfig(cfg) {
			s.Send(wire.MessagePanel("This Big Cube has no configured questions."))
		} else {
			s.Send(wire.MessagePanel("This instance has no configured monsters."))
		}
		return
	}
	if cfg.MaxPlayers > 0 && len(members) > cfg.MaxPlayers {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel(fmt.Sprintf("This instance accepts at most %d players.", cfg.MaxPlayers)))
		return
	}
	for _, member := range members {
		if !instanceAllowsEvolution(member.Char, cfg.AllowedEvolutions) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("A party member cannot enter this instance."))
			return
		}
	}
	for _, stage := range stages {
		if !w.terrain.Walkable(stage.SpawnX, stage.SpawnY) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("The instance entrance is not walkable."))
			return
		}
		if stage.Quiz != nil &&
			(!w.terrain.Walkable(stage.Quiz.TrueX, stage.Quiz.TrueY) ||
				!w.terrain.Walkable(stage.Quiz.FalseX, stage.Quiz.FalseY)) {
			s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
			s.Send(wire.MessagePanel("The instance quiz area is not walkable."))
			return
		}
		for _, spawn := range stage.Spawns {
			if spawn.X != 0 && spawn.Y != 0 && !w.terrain.Walkable(spawn.X, spawn.Y) {
				s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
				s.Send(wire.MessagePanel("The instance monster area is not walkable."))
				return
			}
			def := w.npcDefByName(spawn.NPC)
			if def == nil || !def.IsMonster() || def.Extended == nil {
				s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
				s.Send(wire.MessagePanel(fmt.Sprintf("The instance monster %q is not configured.", spawn.NPC)))
				return
			}
		}
	}
	var firstStage model.VolatileInstanceStage
	if len(stages) > 0 {
		firstStage = stages[0]
	}
	placementInstanceID := ""
	if isDurablePrivateWaterConfig(*cfg) {
		placementInstanceID = runtimeID
	}
	destinations, positionsOK := w.planInstancePositionsIgnoringForInstance(
		members, firstStage.X, firstStage.Y, nil, placementInstanceID)
	if !positionsOK {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("The instance entrance is blocked."))
		return
	}

	oldItem := *item
	type oldPosition struct {
		p    *Player
		x, y uint16
	}
	positions := make([]oldPosition, 0, len(members))
	nightmareCharges := chargeNightmareEntries(members, cfg)
	accounts := make([]*model.Account, 0, len(members))
	seenAccounts := make(map[*model.Account]struct{})
	for index, member := range members {
		positions = append(positions, oldPosition{member, member.X, member.Y})
		member.X, member.Y = destinations[index][0], destinations[index][1]
		member.Char.X, member.Char.Y = member.X, member.Y
		if _, seen := seenAccounts[member.Account]; !seen {
			seenAccounts[member.Account] = struct{}{}
			accounts = append(accounts, member.Account)
		}
	}
	if rule.Consume {
		consumeOne(item)
	}
	inst := &ItemInstance{
		Config: cfgCopy(*cfg), RuntimeID: runtimeID, LeaderID: p.ID, MobIDs: make(map[uint16]struct{}),
		Remaining: totalMobs, CurrentStage: 0, ScheduleEnd: scheduleEnd,
		State: "entry", MobQuadrants: make(map[uint16]uint8), HellGateLichIDs: make(map[uint8]uint16),
		NPCIDs: make(map[uint16]struct{}),
	}
	if isBigCubeConfig(cfg) && cfg.DurationSeconds > 0 {
		inst.HardDeadline = now.Add(time.Duration(cfg.DurationSeconds) * time.Second)
	} else if cfg.ActiveDurationSeconds > 0 && !scheduleEnd.IsZero() {
		// State-machine events use ScheduleEnd only for admission. Their
		// absolute cleanup deadline starts after that window closes.
		inst.HardDeadline = scheduleEnd.Add(time.Duration(cfg.ActiveDurationSeconds) * time.Second)
	} else if cfg.TotalDurationSeconds > 0 {
		inst.HardDeadline = now.Add(
			time.Duration(cfg.TotalDurationSeconds) * time.Second)
	}
	for _, member := range members {
		inst.MemberIDs = append(inst.MemberIDs, member.ID)
	}
	// O primeiro conteudo existe antes de confirmar o ticket. Ele ainda nao e
	// publicado; logo qualquer falha pode ser revertida sem deixar sala vazia.
	if ok, _ := w.spawnItemInstanceStage(inst, 0, now, false, false); !ok {
		restoreNightmareEntries(nightmareCharges)
		*item = oldItem
		for _, old := range positions {
			old.p.X, old.p.Y = old.x, old.y
			old.p.Char.X, old.p.Char.Y = old.x, old.y
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("The instance content could not be created."))
		return
	}
	// Publish the runtime record to the World before the atomic commit. It is
	// not visible to clients yet, but the transaction snapshot now contains the
	// exact event state that accompanies the ticket consumption.
	w.itemInstances[runtimeID] = inst
	previousRuns := 0
	if runKey != "" {
		previousRuns = w.nightmarePartyRuns[runKey]
		w.recordNightmarePartyRun(runKey)
	}
	if err := w.saveAccountsAndInstanceState(accounts...); err != nil {
		restoreNightmareEntries(nightmareCharges)
		w.removeUnpublishedItemInstanceMobs(inst)
		delete(w.itemInstances, runtimeID)
		if runKey != "" {
			w.nightmarePartyRuns[runKey] = previousRuns
		}
		*item = oldItem
		for _, old := range positions {
			old.p.X, old.p.Y = old.x, old.y
			old.p.Char.X, old.p.Char.Y = old.x, old.y
		}
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	// The persistence boundary succeeded. Detach the party from the completed
	// room now, before publishing the new room, so the old cleanup can never
	// race the freshly-created membership. If the commit had failed the old
	// membership would still be present and the rollback above would be exact.
	if graceSource != nil {
		moved := make(map[uint16]struct{}, len(members))
		for _, member := range members {
			if member != nil {
				moved[member.ID] = struct{}{}
			}
		}
		w.detachInstanceMembersAfterCommit(graceSource, moved)
	}
	w.markInstanceStateDirty()
	// O primeiro spawn foi montado antes de publicar a execucao para permitir
	// rollback. Recalcule a vigilia agora que o mapa de instancias existe;
	// sem isto os mobs nasciam dormindo ate o proximo movimento de jogador.
	for mobID := range inst.MobIDs {
		if mob := w.mobsByID[mobID]; mob != nil {
			w.recomputeMobActive(mob)
		}
	}
	for _, member := range members {
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte { return wire.ActionStop(member.ID, member.X, member.Y) })
	}
	w.sendItemInstanceStageStatus(inst)
	if isBigCubeInstance(inst) && !w.startBigCubeQuestion(inst, now) {
		// The loader guarantees a valid question list. Keep the defensive path
		// explicit so a hot-reloaded/broken file cannot leave a ticket consumed
		// in an unwinnable room.
		setInstanceCombatDeadline(inst, now)
		log.Printf("INSTANCIA %q: Big Cube sem pergunta inicial valida", cfg.ID)
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	log.Printf("[#%d] INSTANCIA %q aberta membros=%d mobs=%d", s.ID, cfg.ID, len(members), totalMobs)
}

func cfgCopy(cfg model.VolatileInstance) model.VolatileInstance {
	cfg.Spawns = append([]model.VolatileInstanceSpawn(nil), cfg.Spawns...)
	cfg.AllowedEvolutions = append([]string(nil), cfg.AllowedEvolutions...)
	cfg.EntryAreas = append([]model.VolatileInstanceEntryArea(nil), cfg.EntryAreas...)
	cfg.Schedule = append([]model.VolatileInstanceWindow(nil), cfg.Schedule...)
	cfg.Stages = append([]model.VolatileInstanceStage(nil), cfg.Stages...)
	if cfg.HellGate != nil {
		hell := *cfg.HellGate
		hell.Quadrants = append([]model.VolatileHellGateQuadrant(nil), cfg.HellGate.Quadrants...)
		hell.FinalSpawns = append([]model.VolatileInstanceSpawn(nil), cfg.HellGate.FinalSpawns...)
		hell.FinalNPCs = append([]model.VolatileInstanceSpawn(nil), cfg.HellGate.FinalNPCs...)
		for index := range hell.Quadrants {
			hell.Quadrants[index].Spawns = append([]model.VolatileInstanceSpawn(nil), hell.Quadrants[index].Spawns...)
		}
		cfg.HellGate = &hell
	}
	if cfg.Uxmal != nil {
		uxmal := *cfg.Uxmal
		uxmal.EntryAreas = append([]model.VolatileInstanceEntryArea(nil), cfg.Uxmal.EntryAreas...)
		uxmal.RoomPositions = make([][]model.VolatileDestination, len(cfg.Uxmal.RoomPositions))
		for index := range cfg.Uxmal.RoomPositions {
			uxmal.RoomPositions[index] = append([]model.VolatileDestination(nil), cfg.Uxmal.RoomPositions[index]...)
		}
		uxmal.MaxParties = append([]int(nil), cfg.Uxmal.MaxParties...)
		uxmal.Runes = make([][]uint16, len(cfg.Uxmal.Runes))
		for index := range cfg.Uxmal.Runes {
			uxmal.Runes[index] = append([]uint16(nil), cfg.Uxmal.Runes[index]...)
		}
		uxmal.TicketNextSanc = append([]int(nil), cfg.Uxmal.TicketNextSanc...)
		cfg.Uxmal = &uxmal
	}
	for index := range cfg.Stages {
		cfg.Stages[index].Spawns = append(
			[]model.VolatileInstanceSpawn(nil), cfg.Stages[index].Spawns...)
		cfg.Stages[index].CompletionSpawns = append(
			[]model.VolatileInstanceSpawn(nil), cfg.Stages[index].CompletionSpawns...)
		if cfg.Stages[index].Quiz != nil {
			quiz := *cfg.Stages[index].Quiz
			cfg.Stages[index].Quiz = &quiz
		}
	}
	return cfg
}

func stageDuration(cfg model.VolatileInstance, stage model.VolatileInstanceStage) int {
	if cfg.NoCombatTimeout {
		return 0
	}
	if stage.DurationSeconds > 0 {
		return stage.DurationSeconds
	}
	return cfg.DurationSeconds
}

func stageMobCount(stage model.VolatileInstanceStage) int {
	total := 0
	for _, spawn := range stage.Spawns {
		total += spawn.Count
	}
	return total
}

// moveInstanceMembersAtomic troca a sala do grupo em um unico commit. Fazer
// teleportPlayer membro a membro poderia deixar metade do grupo na sala
// anterior quando o PostgreSQL falhasse no meio da transicao.
func (w *World) moveInstanceMembersAtomic(inst *ItemInstance, stage model.VolatileInstanceStage) bool {
	if inst == nil {
		return false
	}
	members := make([]*Player, 0, len(inst.MemberIDs))
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld &&
			member.Char != nil && member.Account != nil {
			members = append(members, member)
		}
	}
	if len(members) == 0 {
		return false
	}
	ignored := make(map[*Player]struct{}, len(members))
	for _, member := range members {
		ignored[member] = struct{}{}
	}
	instanceID := ""
	if isDurablePrivateWaterInstance(inst) {
		instanceID = inst.RuntimeID
	}
	destinations, ok := w.planInstancePositionsIgnoringForInstance(members, stage.X, stage.Y, ignored, instanceID)
	if !ok {
		return false
	}
	type oldPosition struct {
		player *Player
		x, y   uint16
	}
	oldPositions := make([]oldPosition, 0, len(members))
	accounts := make([]*model.Account, 0, len(members))
	seenAccounts := make(map[*model.Account]struct{}, len(members))
	for index, member := range members {
		oldPositions = append(oldPositions, oldPosition{member, member.X, member.Y})
		member.X, member.Y = destinations[index][0], destinations[index][1]
		member.Char.X, member.Char.Y = member.X, member.Y
		if _, seen := seenAccounts[member.Account]; !seen {
			seenAccounts[member.Account] = struct{}{}
			accounts = append(accounts, member.Account)
		}
	}
	if err := w.saveAccountsAndInstanceState(accounts...); err != nil {
		for _, old := range oldPositions {
			old.player.X, old.player.Y = old.x, old.y
			old.player.Char.X, old.player.Char.Y = old.x, old.y
		}
		return false
	}
	for _, member := range members {
		w.refreshPlayerVisibility(member)
		w.sendToPlayerView(member, func() []byte {
			return wire.ActionStop(member.ID, member.X, member.Y)
		})
	}
	return true
}

func (w *World) sendItemInstanceStageStatus(inst *ItemInstance) {
	if inst == nil {
		return
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return
	}
	stage := stages[inst.CurrentStage]
	remaining := remainingInstanceSeconds(instanceCombatDeadline(inst), w.now())
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld {
			member.Session.Send(wire.StandardParm(
				wire.OpInstanceTime, instanceSignalID, uint32(remaining)))
			member.Session.Send(wire.StandardParm(
				wire.OpInstanceMobs, instanceSignalID, uint32(inst.Remaining)))
			name := stage.Name
			if name == "" {
				name = inst.Config.Name
			}
			member.Session.Send(wire.MessagePanel(name))
		}
	}
}

func (w *World) removeUnpublishedItemInstanceMobs(inst *ItemInstance) {
	if inst == nil {
		return
	}
	for mobID := range inst.MobIDs {
		if mob := w.mobsByID[mobID]; mob != nil {
			w.removeMobInstance(mob)
		}
	}
	for npcID := range inst.NPCIDs {
		if npc := w.mobsByID[npcID]; npc != nil {
			w.removeMobInstance(npc)
		}
	}
	inst.MobIDs = make(map[uint16]struct{})
	inst.NPCIDs = make(map[uint16]struct{})
	inst.Remaining = 0
}

// rollbackItemInstanceMobs desfaz somente os mobs criados pela tentativa
// corrente. Quando a sala ja foi publicada, os observadores precisam receber
// o mesmo pacote de morte/remocao que receberiam num encerramento normal;
// remover apenas do indice server-side deixaria um mob fantasma no client.
func (w *World) rollbackItemInstanceMobs(created []*Mob, published bool) {
	for _, mob := range created {
		if mob == nil {
			continue
		}
		if published {
			w.publishMobRemoval(mob)
		}
		w.removeMobInstance(mob)
	}
}

// spawnItemInstanceStage inicia uma sala/onda autoritativa. publish=false e
// usado apenas na abertura: monta todo o primeiro conteudo antes do commit do
// ingresso, sem expor estado que ainda pode sofrer rollback.
// O segundo retorno informa se uma falha é transitória (persistência/movimento)
// e deve ser repetida. Falha de conteúdo, terreno, ocupação ou IDs encerra a
// execução em vez de deixar uma transição infinita prendendo a sala.
func (w *World) spawnItemInstanceStage(inst *ItemInstance, stageIndex int, now time.Time,
	move, publish bool) (bool, bool) {
	if inst == nil {
		return false, false
	}
	stages := instanceStages(&inst.Config)
	if stageIndex < 0 || stageIndex >= len(stages) {
		return false, false
	}
	stage := stages[stageIndex]
	for _, spawn := range stage.Spawns {
		def := w.npcDefByName(spawn.NPC)
		if spawn.Count <= 0 || def == nil || !def.IsMonster() ||
			def.Extended == nil {
			return false, false
		}
	}
	// Construa e reserve a nova onda antes de mover os jogadores. O caminho
	// antigo movia o grupo primeiro e, se faltasse um ID/posição para um mob,
	// deixava a party persistida na sala nova enquanto CurrentStage continuava
	// na sala antiga. A transição precisa ser um commit único: spawn em memória,
	// movimento persistido, publicação e só então troca do estado da instância.
	created := make([]*Mob, 0, stageMobCount(stage))
	reserved := make(map[uint32]struct{}, len(created))
	instanceID := ""
	if isDurablePrivateWaterInstance(inst) {
		instanceID = inst.RuntimeID
	}
	findFree := func(baseX, baseY uint16, radius int) (uint16, uint16, bool) {
		if radius < 4 {
			radius = 4
		}
		for distance := 0; distance <= radius; distance++ {
			for dy := -distance; dy <= distance; dy++ {
				for dx := -distance; dx <= distance; dx++ {
					if distance > 0 && absInt(dx) != distance && absInt(dy) != distance {
						continue
					}
					nx, ny := int(baseX)+dx, int(baseY)+dy
					if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
						continue
					}
					x, y := uint16(nx), uint16(ny)
					key := uint32(x)<<16 | uint32(y)
					if _, used := reserved[key]; used ||
						w.positionOccupiedExceptPlayersInInstance(x, y, nil, nil, nil, instanceID) {
						continue
					}
					return x, y, true
				}
			}
		}
		return 0, 0, false
	}
	rollback := func() {
		// Os mobs ainda não foram publicados nem registrados no spatial index.
		// Se publish=true, rollbackItemInstanceMobs continua sendo seguro para
		// chamadas futuras caso um novo publicador seja introduzido.
		w.rollbackItemInstanceMobs(created, false)
	}
	for _, spawn := range stage.Spawns {
		def := w.npcDefByName(spawn.NPC)
		for n := 0; n < spawn.Count; n++ {
			spawnX, spawnY := stage.SpawnX, stage.SpawnY
			if spawn.X != 0 && spawn.Y != 0 {
				spawnX, spawnY = spawn.X, spawn.Y
			}
			x, y, ok := findFree(spawnX, spawnY, stage.AreaRadius)
			// Instancias nao aceitam fallback para um tile bloqueado: alem de nascer
			// em terreno invalido, mobs poderiam se sobrepor e ficar inalvejaveis.
			if !ok {
				rollback()
				return false, false
			}
			mobID := w.allocMobID()
			if mobID == 0 {
				rollback()
				return false, false
			}
			runtimeID := inst.RuntimeID
			if runtimeID == "" {
				runtimeID = inst.Config.ID
			}
			mob := &Mob{ID: mobID, Def: def, X: x, Y: y,
				HP: def.Extended.MaxHP, GenerIndex: -1, InstanceID: runtimeID}
			w.mobs = append(w.mobs, mob)
			w.mobsByID[mob.ID] = mob
			reserved[uint32(x)<<16|uint32(y)] = struct{}{}
			created = append(created, mob)
		}
	}
	oldInstance := *inst
	// Make the new stage authoritative before movement so the position commit
	// and the instance snapshot are written together.
	inst.CurrentStage = stageIndex
	newMobIDs := make(map[uint16]struct{}, len(created))
	for _, mob := range created {
		newMobIDs[mob.ID] = struct{}{}
	}
	inst.TransitionAt = time.Time{}
	inst.QuizAt = time.Time{}
	inst.TransitionDeadline = time.Time{}
	inst.QuizDeadline = time.Time{}
	inst.BossSpawned = false
	inst.MobIDs = newMobIDs
	inst.Remaining = len(created)
	duration := stageDuration(inst.Config, stage)
	var deadline time.Time
	if _, hellGate := hellGateInstance(inst); hellGate && inst.Config.ActiveDurationSeconds > 0 {
		deadline = inst.HardDeadline
	} else if duration > 0 {
		deadline = now.Add(time.Duration(duration) * time.Second)
	}
	if !instanceAdmissionOnlySchedule(inst) && !inst.ScheduleEnd.IsZero() && inst.ScheduleEnd.Before(deadline) {
		deadline = inst.ScheduleEnd
	}
	if !inst.HardDeadline.IsZero() && inst.HardDeadline.Before(deadline) {
		deadline = inst.HardDeadline
	}
	setInstanceCombatDeadline(inst, deadline)
	if move && !w.moveInstanceMembersAtomic(inst, stage) {
		*inst = oldInstance
		rollback()
		log.Printf("INSTANCIA %q: falha atomica ao mover grupo para sala %d",
			inst.Config.ID, stageIndex+1)
		return false, true
	}
	for _, mob := range created {
		if publish {
			w.publishMobSpawn(mob)
		} else {
			// A primeira sala ainda nao pode ser enviada, mas precisa entrar
			// no indice espacial para o refresh posterior ao commit encontra-la.
			w.registerMobSpatial(mob)
		}
	}
	if publish {
		w.sendItemInstanceStageStatus(inst)
	}
	log.Printf("INSTANCIA %q sala=%d/%d mobs=%d",
		inst.Config.ID, stageIndex+1, len(stages), inst.Remaining)
	return true, false
}

// spawnItemInstanceCompletionWave materializa uma onda especial na mesma
// sala, sem reiniciar CurrentStage ou CombatDeadline. E usado pela Magic
// Chamber para colocar o chefe apos os 100 mobs da quarta sala.
func (w *World) spawnItemInstanceCompletionWave(inst *ItemInstance,
	stage model.VolatileInstanceStage, now time.Time) bool {
	if inst == nil || len(stage.CompletionSpawns) == 0 {
		return false
	}
	if (!inst.ScheduleEnd.IsZero() && !now.Before(inst.ScheduleEnd)) ||
		(!inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)) {
		return false
	}
	for _, spawn := range stage.CompletionSpawns {
		def := w.npcDefByName(spawn.NPC)
		if spawn.Count <= 0 || def == nil || !def.IsMonster() || def.Extended == nil {
			return false
		}
	}
	created := make([]*Mob, 0)
	reserved := make(map[uint32]struct{})
	instanceID := ""
	if isDurablePrivateWaterInstance(inst) {
		instanceID = inst.RuntimeID
	}
	findFree := func(baseX, baseY uint16) (uint16, uint16, bool) {
		radius := stage.AreaRadius
		if radius < 4 {
			radius = 4
		}
		for distance := 0; distance <= radius; distance++ {
			for dy := -distance; dy <= distance; dy++ {
				for dx := -distance; dx <= distance; dx++ {
					if distance > 0 && absInt(dx) != distance && absInt(dy) != distance {
						continue
					}
					nx, ny := int(baseX)+dx, int(baseY)+dy
					if nx <= 0 || ny <= 0 || nx > 65535 || ny > 65535 {
						continue
					}
					x, y := uint16(nx), uint16(ny)
					key := uint32(x)<<16 | uint32(y)
					if _, used := reserved[key]; used ||
						w.positionOccupiedExceptPlayersInInstance(x, y, nil, nil, nil, instanceID) {
						continue
					}
					return x, y, true
				}
			}
		}
		return 0, 0, false
	}
	rollback := func() { w.rollbackItemInstanceMobs(created, false) }
	for _, spawn := range stage.CompletionSpawns {
		def := w.npcDefByName(spawn.NPC)
		for n := 0; n < spawn.Count; n++ {
			baseX, baseY := stage.SpawnX, stage.SpawnY
			if spawn.X != 0 && spawn.Y != 0 {
				baseX, baseY = spawn.X, spawn.Y
			}
			x, y, ok := findFree(baseX, baseY)
			if !ok {
				rollback()
				return false
			}
			mobID := w.allocMobID()
			if mobID == 0 {
				rollback()
				return false
			}
			runtimeID := inst.RuntimeID
			if runtimeID == "" {
				runtimeID = inst.Config.ID
			}
			mob := &Mob{ID: mobID, Def: def, X: x, Y: y, HP: def.Extended.MaxHP,
				GenerIndex: -1, InstanceID: runtimeID}
			w.mobs = append(w.mobs, mob)
			w.mobsByID[mob.ID] = mob
			reserved[uint32(x)<<16|uint32(y)] = struct{}{}
			created = append(created, mob)
		}
	}
	newMobIDs := make(map[uint16]struct{}, len(created))
	for _, mob := range created {
		newMobIDs[mob.ID] = struct{}{}
		w.publishMobSpawn(mob)
	}
	inst.MobIDs = newMobIDs
	inst.Remaining = len(created)
	w.sendItemInstanceStageStatus(inst)
	log.Printf("INSTANCIA %q chefe/onda de conclusao mobs=%d", inst.Config.ID, inst.Remaining)
	return true
}

func firstFreeVisibleInventorySlot(ch *model.Char) int {
	if ch == nil {
		return -1
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		if ch.Inv[slot].Index == 0 {
			return slot
		}
	}
	return -1
}

// grantItemInstanceReward implementa PutItem do Water: a recompensa pertence
// somente ao lider. Inventario cheio usa o mesmo fallback nativo de item no
// chao; persistencia falha mantem a sala pendente para uma nova tentativa.
func (w *World) grantItemInstanceReward(inst *ItemInstance, now time.Time) bool {
	if inst == nil || inst.RewardGranted {
		return true
	}
	oldRewardGranted, oldExitAt, oldExitDeadline := inst.RewardGranted, inst.ExitAt, inst.ExitDeadline
	finish := func() {
		inst.RewardGranted = true
		inst.ExitAt = now.Add(10 * time.Second)
		inst.ExitDeadline = inst.ExitAt
	}
	persistRewardState := func(account *model.Account) error {
		if _, transactional := w.store.(instanceRuntimeTransactionStore); transactional {
			if account != nil {
				return w.saveAccountsAndInstanceState(account)
			}
			return w.saveAccountsAndInstanceState()
		}
		if account != nil {
			if isDurablePrivateWaterInstance(inst) || sharedTimedInstance(inst.Config) ||
				strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "hell_gate") {
				return fmt.Errorf("store does not support atomic account/instance persistence")
			}
			return w.saveAccount(account)
		}
		return w.persistInstanceState()
	}
	if inst.Config.RewardItem == 0 {
		finish()
		if err := persistRewardState(nil); err != nil {
			inst.RewardGranted, inst.ExitAt, inst.ExitDeadline = oldRewardGranted, oldExitAt, oldExitDeadline
			log.Printf("INSTANCIA %q: salvar transicao sem recompensa: %v", inst.Config.ID, err)
			return false
		}
		return true
	}
	leader := w.playersByID[inst.LeaderID]
	if leader == nil || !leader.InWorld || leader.Char == nil {
		return false
	}
	reward, err := materializeItem(model.Item{Index: inst.Config.RewardItem})
	if err != nil {
		log.Printf("INSTANCIA %q: materializar recompensa %d: %v",
			inst.Config.ID, inst.Config.RewardItem, err)
		return false
	}
	slot := firstFreeVisibleInventorySlot(leader.Char)
	if slot < 0 {
		if w.groundItems == nil {
			w.groundItems = make(map[uint16]*GroundItem)
		}
		ground := w.createGroundDropForInstance(leader.X, leader.Y, reward, true, inst.RuntimeID)
		if ground == nil {
			return false
		}
		finish()
		if err := persistRewardState(leader.Account); err != nil {
			w.publishItemRemove(ground)
			delete(w.groundItems, ground.ID)
			inst.RewardGranted, inst.ExitAt, inst.ExitDeadline = oldRewardGranted, oldExitAt, oldExitDeadline
			log.Printf("INSTANCIA %q: salvar recompensa %d no chao: %v",
				inst.Config.ID, inst.Config.RewardItem, err)
			return false
		}
		leader.Session.Send(wire.MessagePanel("Inventory full: the next Water Scroll was dropped."))
		return true
	}

	leader.Char.Inv[slot] = reward
	// Set every authoritative flag before the transaction. If the commit fails,
	// the complete snapshot (item + reward + exit grace) is restored below.
	finish()
	if err := persistRewardState(leader.Account); err != nil {
		leader.Char.Inv[slot] = model.Item{}
		inst.RewardGranted, inst.ExitAt, inst.ExitDeadline = oldRewardGranted, oldExitAt, oldExitDeadline
		log.Printf("INSTANCIA %q: salvar recompensa %d: %v",
			inst.Config.ID, inst.Config.RewardItem, err)
		return false
	}
	leader.Session.Send(wire.SendItem(leader.ID, placeInv, byte(slot), reward))
	leader.Session.Send(wire.MessagePanel("You received the next Water Scroll."))
	log.Printf("[#%d] INSTANCIA %q concluida: item %d -> inv[%d]",
		leader.Session.ID, inst.Config.ID, inst.Config.RewardItem, slot)
	return true
}

func (w *World) completeItemInstance(inst *ItemInstance, now time.Time) {
	if inst == nil || !inst.ExitAt.IsZero() {
		return
	}
	// O ultimo golpe pode chegar no mesmo ciclo em que o prazo vence. O
	// servidor nao deve conceder a recompensa depois do timeout; deixar o
	// ExitAt vazio permite que tickItemInstances execute o cleanup normal.
	combatDeadline := instanceCombatDeadline(inst)
	if (!combatDeadline.IsZero() && !now.Before(combatDeadline)) ||
		(!inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)) ||
		(!instanceAdmissionOnlySchedule(inst) && !inst.ScheduleEnd.IsZero() && !now.Before(inst.ScheduleEnd)) {
		return
	}
	if w.grantItemInstanceReward(inst, now) {
		w.markInstanceStateDirty()
	}
}

func (w *World) onItemInstanceMobKilled(m *Mob, now time.Time) {
	if m == nil || m.InstanceID == "" {
		return
	}
	inst := w.itemInstances[m.InstanceID]
	if inst == nil {
		return
	}
	if strings.EqualFold(strings.TrimSpace(inst.Config.StateMachine), "uxmal") {
		w.onUxmalMobKilled(m, now)
		return
	}
	w.markInstanceStateDirty()
	combatExpired := !instanceCombatDeadline(inst).IsZero() && !now.Before(instanceCombatDeadline(inst))
	if !instanceAdmissionOnlySchedule(inst) &&
		!inst.ScheduleEnd.IsZero() && !now.Before(inst.ScheduleEnd) {
		combatExpired = true
	}
	if combatExpired || (!inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)) {
		// Marca expiracao sem agendar proxima sala/recompensa. O tick seguinte
		// remove os mobs restantes e recolhe os membros.
		inst.TransitionAt = time.Time{}
		inst.QuizAt = time.Time{}
		inst.TransitionDeadline = time.Time{}
		inst.QuizDeadline = time.Time{}
		setInstanceCombatDeadline(inst, now)
		return
	}
	if _, belongs := inst.MobIDs[m.ID]; !belongs {
		return
	}
	if _, special := hellGateInstance(inst); special {
		w.onHellGateMobKilled(inst, m, now)
		for _, id := range inst.MemberIDs {
			if member := w.playersByID[id]; member != nil && member.InWorld {
				member.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, uint32(inst.Remaining)))
			}
		}
		return
	}
	delete(inst.MobIDs, m.ID)
	if inst.Remaining > 0 {
		inst.Remaining--
	}
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld {
			member.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, uint32(inst.Remaining)))
		}
	}
	if inst.Remaining == 0 {
		policy := instanceFinishPolicy(inst.Config)
		if policy == "respawn_until_timeout" {
			if ok, _ := w.spawnItemInstanceStage(inst, inst.CurrentStage, now, false, true); !ok {
				setInstanceCombatDeadline(inst, now)
			}
			w.markInstanceStateDirty()
			return
		}
		if policy == "finish_on_timeout" {
			// The event remains present until its configured deadline. There is
			// intentionally no reward/transition on clear for this policy.
			return
		}
		stages := instanceStages(&inst.Config)
		if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
			setInstanceCombatDeadline(inst, now)
			return
		}
		if stage := stages[inst.CurrentStage]; len(stage.CompletionSpawns) > 0 && !inst.BossSpawned {
			if ok := w.spawnItemInstanceCompletionWave(inst, stage, now); ok {
				inst.BossSpawned = true
				w.markInstanceStateDirty()
				return
			}
			setInstanceCombatDeadline(inst, now)
			return
		}
		if inst.CurrentStage+1 < len(stages) {
			if quiz := stages[inst.CurrentStage].Quiz; quiz != nil {
				deadline := now.Add(time.Duration(quiz.DurationSeconds) * time.Second)
				inst.QuizAt = deadline
				inst.QuizDeadline = deadline
				inst.TransitionAt = time.Time{}
				inst.TransitionDeadline = time.Time{}
				for _, id := range inst.MemberIDs {
					if member := w.playersByID[id]; member != nil && member.InWorld {
						member.Session.Send(wire.StandardParm(
							wire.OpInstanceTime, instanceSignalID, uint32(quiz.DurationSeconds)))
						member.Session.Send(wire.MessagePanel(
							strings.ReplaceAll(quiz.Question, "_", " ")))
					}
				}
				w.markInstanceStateDirty()
				return
			}
			delay := inst.Config.TransitionSeconds
			if delay <= 0 {
				delay = 10
			}
			deadline := now.Add(time.Duration(delay) * time.Second)
			inst.TransitionAt = deadline
			inst.TransitionDeadline = deadline
			for _, id := range inst.MemberIDs {
				if member := w.playersByID[id]; member != nil && member.InWorld {
					member.Session.Send(wire.StandardParm(
						wire.OpInstanceTime, instanceSignalID, uint32(delay)))
					member.Session.Send(wire.MessagePanel("The next room will open shortly."))
				}
			}
			w.markInstanceStateDirty()
		} else {
			w.completeItemInstance(inst, now)
		}
	}
}

func instanceQuizCorrect(p *Player, quiz *model.VolatileInstanceQuiz) bool {
	if p == nil || quiz == nil {
		return false
	}
	x, y := quiz.FalseX, quiz.FalseY
	if quiz.Answer {
		x, y = quiz.TrueX, quiz.TrueY
	}
	minX, minY := uint16(0), uint16(0)
	if x >= 3 {
		minX = x - 3
	}
	if y >= 3 {
		minY = y - 3
	}
	return p.X >= minX && p.X <= x && p.Y >= minY && p.Y <= y
}

// resolveItemInstanceQuiz confirma em um unico commit tanto a EXP dos acertos
// quanto a retirada dos erros. Assim uma falha do banco nao divide a party nem
// concede a recompensa duas vezes.
func (w *World) resolveItemInstanceQuiz(inst *ItemInstance, now time.Time) bool {
	if inst == nil {
		return false
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return false
	}
	quiz := stages[inst.CurrentStage].Quiz
	if quiz == nil {
		return false
	}
	type state struct {
		player         *Player
		char           model.Char
		x, y           uint16
		ok             bool
		levels         int
		exp            uint32
		cytheraChanged bool
	}
	states := make([]state, 0, len(inst.MemberIDs))
	accounts := make([]*model.Account, 0, len(inst.MemberIDs))
	seenAccounts := make(map[*model.Account]struct{}, len(inst.MemberIDs))
	for _, id := range inst.MemberIDs {
		p := w.playersByID[id]
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
			continue
		}
		current := state{player: p, char: cloneCharacterState(p.Char), x: p.X, y: p.Y}
		current.ok = instanceQuizCorrect(p, quiz)
		if current.ok && quiz.RewardExp > 0 {
			oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)
			current.levels, current.exp = grantExp(p.Char, quiz.RewardExp)
			current.cytheraChanged =
				current.levels > 0 && updateCelestialCythera(p.Char)
			w.recalcPlayer(p.Char)
			setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
			setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
		} else if !current.ok {
			p.X, p.Y = w.findFreePlayerPosition(
				inst.Config.ExitX, inst.Config.ExitY, 6, p)
			p.Char.X, p.Char.Y = p.X, p.Y
		}
		states = append(states, current)
		if _, exists := seenAccounts[p.Account]; !exists {
			seenAccounts[p.Account] = struct{}{}
			accounts = append(accounts, p.Account)
		}
	}
	if len(states) == 0 {
		inst.MemberIDs = nil
		inst.QuizAt = time.Time{}
		inst.QuizDeadline = time.Time{}
		setInstanceCombatDeadline(inst, now)
		w.markInstanceStateDirty()
		return true
	}
	// A resposta altera simultaneamente personagens e a maquina da instancia.
	// Prepare ambos antes do commit para que PostgreSQL/JSON possam persistir o
	// snapshot da sala no mesmo commit das EXP/posicoes; salvar somente as
	// contas deixava a pergunta respondida voltar apos um crash.
	oldInstance := *inst
	survivors := make([]uint16, 0, len(states))
	for _, result := range states {
		if result.ok {
			survivors = append(survivors, result.player.ID)
		}
	}
	inst.MemberIDs = survivors
	ensureItemInstanceLeader(inst)
	inst.QuizAt = time.Time{}
	inst.QuizDeadline = time.Time{}
	if len(survivors) == 0 {
		setInstanceCombatDeadline(inst, now)
	} else {
		delay := inst.Config.TransitionSeconds
		if delay < 0 {
			delay = 0
		}
		inst.TransitionAt = now.Add(time.Duration(delay) * time.Second)
		inst.TransitionDeadline = inst.TransitionAt
	}
	if err := w.saveAccountsAndInstanceState(accounts...); err != nil {
		*inst = oldInstance
		for _, old := range states {
			*old.player.Char = old.char
			old.player.X, old.player.Y = old.x, old.y
		}
		return false
	}

	for _, result := range states {
		p := result.player
		if !result.ok {
			w.refreshPlayerVisibility(p)
			w.sendToPlayerView(p, func() []byte {
				return wire.ActionStop(p.ID, p.X, p.Y)
			})
			p.Session.Send(wire.StandardParm(
				wire.OpInstanceTime, instanceSignalID, 0))
			p.Session.Send(wire.StandardParm(
				wire.OpInstanceMobs, instanceSignalID, 0))
			p.Session.Send(wire.MessagePanel("Wrong answer. You left the Cube."))
			continue
		}
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		if result.levels > 0 {
			p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
		}
		if result.cytheraChanged {
			p.Session.Send(wire.SendItem(p.ID, placeEquip, 1, p.Char.Equip[1]))
			w.refreshAppearance(p)
		}
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		p.Session.Send(wire.MessagePanel(fmt.Sprintf(
			"Correct! +%d EXP.", result.exp)))
	}
	if len(survivors) == 0 {
		return true
	}
	return true
}

func (w *World) tickItemInstances(now time.Time) {
	for id, inst := range w.itemInstances {
		hardExpired := !inst.HardDeadline.IsZero() && !now.Before(inst.HardDeadline)
		scheduleExpired := !inst.ScheduleEnd.IsZero() && !now.Before(inst.ScheduleEnd)
		if instanceAdmissionOnlySchedule(inst) {
			// Hell Gate closes admission at ScheduleEnd but remains active until
			// its independent event deadline.
			scheduleExpired = false
		}
		quizDeadline := inst.QuizDeadline
		if quizDeadline.IsZero() {
			quizDeadline = inst.QuizAt
		}
		transitionDeadline := inst.TransitionDeadline
		if transitionDeadline.IsZero() {
			transitionDeadline = inst.TransitionAt
		}
		if isBigCubeInstance(inst) && !hardExpired && !scheduleExpired {
			if inst.QuizAt.IsZero() && inst.TransitionAt.IsZero() &&
				inst.ExitAt.IsZero() && inst.Remaining == 0 {
				if w.startBigCubeQuestion(inst, now) {
					// startBigCubeQuestion published the authoritative timer and
					// question. Re-evaluate it on the next tick, not as an already
					// expired zero-value deadline in the generic path below.
					continue
				}
				setInstanceCombatDeadline(inst, now)
			}
			if !inst.QuizAt.IsZero() && !now.Before(inst.QuizDeadline) {
				if !w.resolveBigCubeQuestion(inst, now) {
					continue
				}
				transitionDeadline = inst.TransitionDeadline
				if transitionDeadline.IsZero() {
					transitionDeadline = inst.TransitionAt
				}
			}
		}
		// A pergunta/transicao e uma nova fase. Ela continua valida mesmo se o
		// prazo de combate da onda anterior ja tiver vencido no mesmo tick.
		if !hardExpired && !scheduleExpired && !inst.QuizAt.IsZero() && !now.Before(quizDeadline) {
			if !w.resolveItemInstanceQuiz(inst, now) {
				continue
			}
			// A resposta cria um novo prazo de transição. Não reutilize o
			// deadline calculado antes de resolver a pergunta: em configurações
			// com atraso positivo isso avançaria a sala no mesmo tick.
			transitionDeadline = inst.TransitionDeadline
			if transitionDeadline.IsZero() {
				transitionDeadline = inst.TransitionAt
			}
		}
		if !hardExpired && !scheduleExpired && !inst.TransitionAt.IsZero() && !now.Before(transitionDeadline) {
			previousStage := inst.CurrentStage
			if ok, retryable := w.spawnItemInstanceStage(inst, previousStage+1, now, true, true); !ok {
				if retryable {
					// Falha de persistencia no movimento e transitoria: mantenha a
					// transicao pendente para o proximo tick.
					continue
				}
				// O grupo ja foi movido, mas o conteudo da nova sala nao pode ser
				// materializado. Nao deixe CurrentStage/Remaining=0 com um prazo
				// futuro: isso seria interpretado como sala concluida e concederia
				// recompensa sem matar a onda. Encerre a execucao neste tick.
				inst.TransitionAt = time.Time{}
				inst.QuizAt = time.Time{}
				inst.TransitionDeadline = time.Time{}
				inst.QuizDeadline = time.Time{}
				setInstanceCombatDeadline(inst, now)
			} else if isBigCubeInstance(inst) && !w.startBigCubeQuestion(inst, now) {
				setInstanceCombatDeadline(inst, now)
			}
		}
		combatDeadline := instanceCombatDeadline(inst)
		combatExpired := !combatDeadline.IsZero() && !now.Before(combatDeadline)
		sharedTimed := sharedTimedInstance(inst.Config) &&
			instanceFinishPolicy(inst.Config) == "respawn_until_timeout"
		linearCompletion := instanceCompletesOnClear(inst)
		if (hellGateAcceptsFinalState(inst) || linearCompletion) && inst.Remaining == 0 && inst.TransitionAt.IsZero() && inst.QuizAt.IsZero() &&
			inst.ExitAt.IsZero() && (hellGateAcceptsFinalState(inst) || now.Before(combatDeadline)) {
			w.completeItemInstance(inst, now)
		}
		expired := hardExpired || scheduleExpired
		if !sharedTimed && inst.TransitionAt.IsZero() && inst.QuizAt.IsZero() && combatExpired {
			expired = true
		}
		completed := !inst.ExitAt.IsZero() && !now.Before(inst.ExitAt)
		if !expired && !completed {
			continue
		}
		for mobID := range inst.MobIDs {
			if mob := w.mobsByID[mobID]; mob != nil {
				mob.Dead = true
				w.publishMobDeath(mob, 0, 0, nil)
				w.removeMobInstance(mob)
			}
		}
		for npcID := range inst.NPCIDs {
			if npc := w.mobsByID[npcID]; npc != nil {
				w.publishMobRemoval(npc)
				w.removeMobInstance(npc)
			}
		}
		allExited := true
		for _, playerID := range inst.MemberIDs {
			if member := w.playersByID[playerID]; member != nil && member.InWorld {
				// Defense in depth for worlds upgraded from the old code: if a
				// duplicate membership was already created, never let an older
				// room's expiry teleport the character out of the newer room.
				if w.playerInOtherItemInstance(playerID, inst) {
					continue
				}
				if !w.teleportPlayer(member, inst.Config.ExitX, inst.Config.ExitY) {
					allExited = false
					continue
				}
				member.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 0))
				member.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, 0))
			}
		}
		if !allExited {
			continue
		}
		// A reward dropped on the floor is private while its Water room exists.
		// Once the room is definitively gone, release it to the public world so
		// an uncollected fallback is not stranded behind a deleted RuntimeID.
		for _, ground := range w.groundItems {
			if ground == nil || ground.InstanceID != id {
				continue
			}
			ground.InstanceID = ""
			w.publishItemSpawn(ground)
		}
		delete(w.itemInstances, id)
		delete(w.pendingInstanceMembers, id)
		delete(w.pendingInstanceLeaders, id)
		w.markInstanceStateDirty()
		log.Printf("INSTANCIA %q encerrada", id)
	}
}
