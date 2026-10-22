package game

import (
	"context"
	"sort"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
)

// MODERNIZACAO_COMPATIVEL: bounded staff reads of live World state. They share
// the overview admission gate (one read at a time), use the spatial index for
// the map and the instance table for instances, and copy values out.

// worldRead admits one read through the overview gate.
func (w *World) worldRead(ctx context.Context, label string, fn func()) error {
	if err := ctx.Err(); err != nil {
		return err
	}
	if !w.controlPending.CompareAndSwap(false, true) {
		return control.ErrBusy
	}
	return w.queueStaff(ctx, label, fn, func() { w.controlPending.Store(false) })
}

func (w *World) MapSnapshot(ctx context.Context, x, y uint16, radius int) (control.MapSnapshot, error) {
	if radius < 1 || radius > control.MaxMapRadius || int(x) >= model.TerrainWidth || int(y) >= model.TerrainHeight {
		return control.MapSnapshot{}, control.ErrBusy
	}
	var snapshot control.MapSnapshot
	err := w.worldRead(ctx, "control.map", func() { snapshot = w.mapSnapshot(x, y, radius) })
	return snapshot, err
}

func (w *World) mapSnapshot(x, y uint16, radius int) control.MapSnapshot {
	out := control.MapSnapshot{Version: 1, AsOf: w.now().UTC(), X: x, Y: y, Radius: radius,
		Players: []control.MapPlayer{}, Mobs: []control.MapMob{}}
	for _, p := range w.nearbyWorldPlayers(x, y, radius) {
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
			continue
		}
		if len(out.Players) == control.MaxMapPlayers {
			out.Truncated = true
			break
		}
		row := control.MapPlayer{UID: p.Char.UID, Name: p.Char.Name, Account: p.Account.Name, X: p.X, Y: p.Y}
		if score := p.Char.RuntimeScore; score != nil {
			row.Level = score.Level
		} else if p.Char.Score != nil {
			row.Level = p.Char.Score.Level
		}
		if p.Session != nil {
			row.SessionID = p.Session.ID
		}
		out.Players = append(out.Players, row)
	}
	for _, m := range w.nearbyMobs(x, y, radius) {
		if m == nil || m.Def == nil {
			continue
		}
		if len(out.Mobs) == control.MaxMapMobs {
			out.Truncated = true
			break
		}
		row := control.MapMob{ID: m.ID, Name: m.Def.Name, X: m.X, Y: m.Y, HP: m.HP,
			Boss: w.bossFor(m.ID) != nil, NPC: m.Def.Tipo != model.TipoMonstro}
		if m.Def.Score != nil {
			row.MaxHP = m.Def.Score.MaxHP
		}
		out.Mobs = append(out.Mobs, row)
	}
	sort.Slice(out.Players, func(i, j int) bool { return out.Players[i].UID < out.Players[j].UID })
	sort.Slice(out.Mobs, func(i, j int) bool { return out.Mobs[i].ID < out.Mobs[j].ID })
	return out
}

func (w *World) Instances(ctx context.Context) (control.InstancesView, error) {
	var view control.InstancesView
	err := w.worldRead(ctx, "control.instances", func() { view = w.instancesView() })
	return view, err
}

func optionalTime(t time.Time) *time.Time {
	if t.IsZero() {
		return nil
	}
	utc := t.UTC()
	return &utc
}

func (w *World) instancesView() control.InstancesView {
	view := control.InstancesView{Version: 1, AsOf: w.now().UTC(), Instances: []control.InstanceStatus{}}
	for _, inst := range w.itemInstances {
		if inst == nil {
			continue
		}
		status := control.InstanceStatus{RuntimeID: inst.RuntimeID, ConfigID: inst.Config.ID, Name: inst.Config.Name,
			X: inst.Config.X, Y: inst.Config.Y, AreaRadius: inst.Config.AreaRadius, State: inst.State, Stage: inst.CurrentStage,
			Members: []control.InstanceMember{}, Mobs: len(inst.MobIDs), Remaining: inst.Remaining,
			BossSpawned: inst.BossSpawned, RewardGranted: inst.RewardGranted, MaxPlayers: inst.Config.MaxPlayers,
			CombatEnds: optionalTime(inst.CombatDeadline), TransitionAt: optionalTime(inst.TransitionDeadline),
			QuizEnds: optionalTime(inst.QuizDeadline), ExitAt: optionalTime(inst.ExitDeadline),
			HardDeadline: optionalTime(inst.HardDeadline), ScheduleEnd: optionalTime(inst.ScheduleEnd)}
		for _, id := range inst.MemberIDs {
			name := "(offline)"
			if p := w.playersByID[id]; p != nil && p.Char != nil {
				name = p.Char.Name
			}
			status.Members = append(status.Members, control.InstanceMember{Name: name, Leader: id == inst.LeaderID})
		}
		view.Instances = append(view.Instances, status)
	}
	sort.Slice(view.Instances, func(i, j int) bool { return view.Instances[i].RuntimeID < view.Instances[j].RuntimeID })
	return view
}

// ReservedGenerators returns the enabled NPCGener entries NewWorld skipped
// before the template check, keyed by generator index, with the owning
// feature ("item_instance" or "colosseum"). The set never changes after
// construction, so it is safe to read without the World goroutine.
func (w *World) ReservedGenerators() map[int]string {
	out := make(map[int]string, len(w.reservedGenerators))
	for index, reason := range w.reservedGenerators {
		out[index] = reason
	}
	return out
}
