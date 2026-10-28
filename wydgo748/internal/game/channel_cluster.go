package game

import (
	"errors"
	"fmt"
	"reflect"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// ChannelCluster owns shared state on one actor. Worlds never run independent
// goroutines against the same account/guild aggregates. Persistence workers
// receive detached snapshots through one reservation coordinator.
type ChannelCluster struct {
	store     store.Store
	worlds    []*World
	active    *World
	owner     *criticalPersistenceOwner
	instances *model.InstanceStateSnapshot
	wake      chan struct{}
	tickets   map[string]*channelTicket
	transfers map[string]*channelTicket
}

func NewChannelCluster(backend store.Store) (*ChannelCluster, error) {
	if backend == nil {
		return nil, errors.New("channel cluster: missing store")
	}
	c := &ChannelCluster{store: backend, wake: make(chan struct{}, 1)}
	if saved, ok := backend.(instanceRuntimeStore); ok {
		state, err := saved.LoadInstanceState()
		if err != nil {
			return nil, fmt.Errorf("channel cluster: load instance state: %w", err)
		}
		c.instances = state.Clone()
	}
	return c, nil
}

func WithChannelCluster(cluster *ChannelCluster) WorldOption {
	return func(w *World) { w.cluster = cluster }
}

func (c *ChannelCluster) attach(w *World) error {
	if w.store != c.store {
		return errors.New("channel cluster: worlds must use one store")
	}
	if w.channel == 0 {
		return errors.New("channel cluster: channel zero is reserved")
	}
	for _, other := range c.worlds {
		if other.channel == w.channel {
			return fmt.Errorf("channel cluster: duplicate channel %d", w.channel)
		}
	}
	if len(c.worlds) == 0 && w.guilds != nil {
		w.guilds.UpgradeChannelCitizenship()
	}
	if len(c.worlds) > 0 {
		first := c.worlds[0]
		w.guilds, w.accountSessions, w.charNames = first.guilds, first.accountSessions, first.charNames
		w.authSlots = first.authSlots
		w.authClientsByIP, w.authClientsByNetwork = first.authClientsByIP, first.authClientsByNetwork
		w.authRateByIP, w.authRateByAccount = first.authRateByIP, first.authRateByAccount
		w.chatRateByAccount = first.chatRateByAccount
		if first.staffOps.sanctionVersions == nil {
			first.staffOps.sanctionVersions = make(map[string]uint64)
		}
		w.staffOps.sanctionVersions = first.staffOps.sanctionVersions
		if first.staffOps.bans == nil {
			first.staffOps.bans = make(map[string]time.Time)
		}
		if first.staffOps.mutes == nil {
			first.staffOps.mutes = make(map[string]time.Time)
		}
		w.staffOps.bans, w.staffOps.mutes = first.staffOps.bans, first.staffOps.mutes
	}
	c.worlds = append(c.worlds, w)
	w.cluster = c
	c.enter(w)
	return nil
}

func (c *ChannelCluster) enter(w *World) {
	c.active = w
	if w.guilds != nil {
		w.guilds.SelectWarChannel(w.channel)
	}
	if c.owner != nil {
		c.owner.activeWorld = w
	}
}

func (c *ChannelCluster) inWorld(w *World, action func()) {
	previous := c.active
	c.enter(w)
	defer func() {
		if previous != nil {
			c.enter(previous)
		}
	}()
	action()
}

func (c *ChannelCluster) Run() {
	ticker := time.NewTicker(worldTickInterval)
	defer ticker.Stop()
	c.run(ticker.C, nil)
}

// run rotates bounded command batches. No lock is held across blocking I/O or
// select; one busy channel cannot starve another channel's commands or ticks.
func (c *ChannelCluster) run(ticks <-chan time.Time, stop <-chan struct{}) {
	for {
		select {
		case <-stop:
			return
		default:
		}
		progress := false
		for _, w := range c.worlds {
			c.enter(w)
			w.processCriticalCompletions(criticalCompletionBatchLimit)
			w.resumeCriticalDeferredCommands(criticalDeferredReplayLimit)
			cmd, ok := w.popPendingCommand()
			if !ok {
				select {
				case cmd = <-w.commands:
					ok = true
				default:
				}
			}
			if ok {
				w.processCommandBatch(cmd, nil)
				progress = true
			}
			w.finishShutdownIfReady()
		}
		select {
		case <-stop:
			return
		case <-ticks:
			for _, w := range c.worlds {
				c.enter(w)
				w.tick()
			}
			continue
		default:
		}
		if progress {
			continue
		}
		var results <-chan criticalPersistenceResult
		if c.owner != nil {
			results = c.owner.queue.results
		}
		// Internal authentication and control producers also send directly to
		// commands. Select on every mailbox instead of relying on Enqueue's wake.
		cases := []reflect.SelectCase{
			{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(stop)},
			{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(c.wake)},
			{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(results)},
			{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(ticks)},
		}
		for _, w := range c.worlds {
			cases = append(cases, reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(w.commands)})
		}
		chosen, value, _ := reflect.Select(cases)
		switch chosen {
		case 0:
			return
		case 2:
			c.enter(c.worlds[0])
			c.worlds[0].applyCriticalCompletion(value.Interface().(criticalPersistenceResult))
		case 3:
			for _, w := range c.worlds {
				c.enter(w)
				w.tick()
			}
		default:
			if chosen >= 4 {
				w := c.worlds[chosen-4]
				c.enter(w)
				w.processCommandBatch(value.Interface().(command), nil)
			}
		}
	}
}

func (c *ChannelCluster) notify() {
	select {
	case c.wake <- struct{}{}:
	default:
	}
}

func (c *ChannelCluster) world(channel byte) *World {
	for _, w := range c.worlds {
		if w.channel == channel {
			return w
		}
	}
	return nil
}

func (w *World) restoredChannelInstances(state *model.InstanceStateSnapshot) *model.InstanceStateSnapshot {
	if w.cluster == nil || state == nil {
		return state
	}
	if w.channel == 1 {
		copy := state.Clone()
		copy.Channels = nil
		return copy
	}
	return state.Channels[w.channel].Clone()
}

func (w *World) persistedChannelInstances(local *model.InstanceStateSnapshot) *model.InstanceStateSnapshot {
	if w.cluster == nil || local == nil {
		return local
	}
	root := w.cluster.instances.Clone()
	if root == nil {
		root = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
	}
	channels := root.Channels
	if channels == nil {
		channels = make(map[byte]*model.InstanceStateSnapshot)
	}
	if w.channel == 1 {
		root = local.Clone()
	} else {
		channels[w.channel] = local.Clone()
	}
	root.Channels = channels
	return root
}

func (w *World) committedChannelInstances(state *model.InstanceStateSnapshot) {
	if w.cluster != nil && state != nil {
		w.cluster.instances = state.Clone()
	}
}

// Shutdown closes every world's admission before the first terminal save.
// Each World then drains the same accepted queue and saves only its local state.
func (c *ChannelCluster) Shutdown(timeout time.Duration) bool {
	deadline := time.Now().Add(timeout)
	done := make([]chan error, len(c.worlds))
	for i, w := range c.worlds {
		done[i] = make(chan error, 1)
		select {
		case w.commands <- command{queuedAt: time.Now(), shutdown: done[i]}:
			c.notify()
		case <-time.After(time.Until(deadline)):
			return false
		}
	}
	result := true
	for _, reply := range done {
		select {
		case err := <-reply:
			result = result && err == nil
		case <-time.After(time.Until(deadline)):
			return false
		}
	}
	return result
}
