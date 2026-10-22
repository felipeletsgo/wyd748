package control

import (
	"context"
	"time"
)

// Bounded live World reads for the staff map and instance monitor
// (DOCS/server/staff-operations.md). Both share the overview admission gate.
const (
	MaxMapRadius  = 128
	MaxMapPlayers = 200
	MaxMapMobs    = 400
)

type MapPlayer struct {
	UID       string `json:"uid"`
	Name      string `json:"name"`
	Account   string `json:"account"`
	Level     uint32 `json:"level"`
	X         uint16 `json:"x"`
	Y         uint16 `json:"y"`
	SessionID int64  `json:"sessionId"`
}

type MapMob struct {
	ID    uint16 `json:"id"`
	Name  string `json:"name"`
	X     uint16 `json:"x"`
	Y     uint16 `json:"y"`
	HP    uint32 `json:"hp"`
	MaxHP uint32 `json:"maxHp"`
	Boss  bool   `json:"boss"`
	NPC   bool   `json:"npc"`
}

// MapSnapshot covers a square of side 2*Radius+1 cells around X/Y. Truncated
// reports that a cap cut the lists.
type MapSnapshot struct {
	Version   int         `json:"version"`
	AsOf      time.Time   `json:"asOf"`
	X         uint16      `json:"x"`
	Y         uint16      `json:"y"`
	Radius    int         `json:"radius"`
	Players   []MapPlayer `json:"players"`
	Mobs      []MapMob    `json:"mobs"`
	Truncated bool        `json:"truncated"`
}

type InstanceMember struct {
	Name   string `json:"name"`
	Leader bool   `json:"leader"`
}

type InstanceStatus struct {
	RuntimeID     string           `json:"runtimeId"`
	ConfigID      string           `json:"configId"`
	Name          string           `json:"name"`
	X             uint16           `json:"x"`
	Y             uint16           `json:"y"`
	AreaRadius    int              `json:"areaRadius"`
	State         string           `json:"state"`
	Stage         int              `json:"stage"`
	Members       []InstanceMember `json:"members"`
	Mobs          int              `json:"mobs"`
	Remaining     int              `json:"remaining"`
	BossSpawned   bool             `json:"bossSpawned"`
	RewardGranted bool             `json:"rewardGranted"`
	CombatEnds    *time.Time       `json:"combatEnds"`
	TransitionAt  *time.Time       `json:"transitionAt"`
	QuizEnds      *time.Time       `json:"quizEnds"`
	ExitAt        *time.Time       `json:"exitAt"`
	HardDeadline  *time.Time       `json:"hardDeadline"`
	ScheduleEnd   *time.Time       `json:"scheduleEnd"`
	MaxPlayers    int              `json:"maxPlayers"`
}

type InstancesView struct {
	Version   int              `json:"version"`
	AsOf      time.Time        `json:"asOf"`
	Instances []InstanceStatus `json:"instances"`
}

type WorldReadSource interface {
	MapSnapshot(ctx context.Context, x, y uint16, radius int) (MapSnapshot, error)
	Instances(ctx context.Context) (InstancesView, error)
}
