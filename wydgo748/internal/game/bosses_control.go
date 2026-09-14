package game

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"log"
	"time"
	"wydgo/internal/control"
)

// MODERNIZACAO_COMPATIVEL: administrative access to the existing boss lifecycle.
type bossesReceipt struct {
	command control.BossesCommand
	result  control.BossesResult
}
type bossesRequest struct {
	ctx       context.Context
	actor     string
	command   *control.BossesCommand
	authorize func() bool
	reply     chan control.BossesResult
}

func (w *World) BossesStatus(ctx context.Context) (control.BossesStatus, error) {
	r, err := w.enqueueBosses(&bossesRequest{ctx: ctx})
	return r.Status, err
}
func (w *World) BossesCommand(ctx context.Context, actor string, cmd control.BossesCommand, authorize func() bool) (control.BossesResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.BossesResult{}, err
	}
	return w.enqueueBosses(&bossesRequest{ctx: ctx, actor: actor, command: &cmd, authorize: authorize})
}
func (w *World) enqueueBosses(req *bossesRequest) (control.BossesResult, error) {
	if err := req.ctx.Err(); err != nil {
		return control.BossesResult{}, err
	}
	if !w.bossesPending.CompareAndSwap(false, true) {
		return control.BossesResult{}, control.ErrBusy
	}
	req.reply = make(chan control.BossesResult, 1)
	select {
	case w.commands <- command{bosses: req, queuedAt: time.Now()}:
	default:
		w.bossesPending.Store(false)
		return control.BossesResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-req.ctx.Done():
		return control.BossesResult{}, req.ctx.Err()
	}
}

func (w *World) bossesSnapshot() control.BossesStatus {
	if w.bossesEpoch == "" {
		var b [16]byte
		if _, err := rand.Read(b[:]); err != nil {
			panic(err)
		}
		w.bossesEpoch = hex.EncodeToString(b[:])
	}
	s := control.BossesStatus{Version: 1, Epoch: w.bossesEpoch, AsOf: w.now().UTC(), Bosses: make([]control.BossStatus, 0, len(w.bossSpawns))}
	for _, state := range w.bossSpawns {
		b := control.BossStatus{ID: state.config.ID, Name: state.def.Name, Revision: state.revision,
			Alive: state.mobID != 0, MobID: state.mobID, MaxHP: state.def.Score.MaxHP,
			X: state.config.Spawn.X, Y: state.config.Spawn.Y, SpawnX: state.config.Spawn.X, SpawnY: state.config.Spawn.Y}
		if mob := w.mobsByID[state.mobID]; mob != nil {
			b.HP, b.X, b.Y = mob.HP, mob.X, mob.Y
		}
		if !state.respawnAt.IsZero() {
			at := state.respawnAt.UTC()
			b.RespawnAt = &at
		}
		s.Bosses = append(s.Bosses, b)
	}
	return s
}

func (w *World) handleBosses(req *bossesRequest) {
	defer w.bossesPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.BossesResult{Code: "ok", Status: w.bossesSnapshot()}
	if req.command == nil {
		req.reply <- result
		return
	}
	c := *req.command
	result.OperationID = c.OperationID
	finish := func(code string) { result.Code = code; req.reply <- result }
	if c.Validate() != nil {
		finish("invalid_command")
		return
	}
	if req.actor == "" || req.authorize == nil || !req.authorize() {
		finish("capability_required")
		return
	}
	if req.ctx.Err() != nil {
		return
	}
	if w.shuttingDown {
		finish("server_maintenance")
		return
	}
	if c.Epoch != w.bossesEpoch {
		finish("stale_boss")
		return
	}
	key := req.actor + ":" + c.OperationID
	if receipt, exists := w.bossesReceipts[key]; exists {
		if receipt.command != c {
			finish("operation_conflict")
			return
		}
		result = receipt.result
		result.Replayed = true
		result.Status = w.bossesSnapshot()
		req.reply <- result
		return
	}
	// Never evict receipts during this epoch: delayed retries must not summon again.
	if len(w.bossesReceipts) >= 1024 {
		finish("operation_capacity")
		return
	}
	code := "unknown_boss"
	for _, state := range w.bossSpawns {
		if state.config.ID != c.BossID {
			continue
		}
		switch {
		case state.revision != c.ExpectedRevision:
			code = "stale_boss"
		case state.mobID != 0:
			code = "boss_alive"
		default:
			code = "ok"
			if err := w.spawnBoss(state); err != nil {
				log.Printf("[web-admin] boss spawn failed boss=%q operation=%q: %v", c.BossID, c.OperationID, err)
				code = "spawn_failed"
			}
		}
		break
	}
	result.Code, result.Status = code, w.bossesSnapshot()
	if w.bossesReceipts == nil {
		w.bossesReceipts = make(map[string]bossesReceipt)
	}
	// Only retain the receipt, not a full catalog per operation.
	receipt := result
	receipt.Status = control.BossesStatus{}
	w.bossesReceipts[key] = bossesReceipt{command: c, result: receipt}
	log.Printf("[web-admin] bosses actor=%q operation=%q action=%q reason=%q result=%q boss=%q revision=%d",
		req.actor, c.OperationID, c.Action, c.Reason, code, c.BossID, c.ExpectedRevision)
	req.reply <- result
}
