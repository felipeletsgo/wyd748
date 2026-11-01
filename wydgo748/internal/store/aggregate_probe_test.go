package store

import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"slices"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

// P10 measurement probe: size and write duration of the singleton instance
// aggregate, which holds the instance reservation while it is written. Run
// with WYD_AGGREGATE_PROBE=1 against the local PostgreSQL test database.
func TestInstanceAggregateWriteProbe(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" || os.Getenv("WYD_AGGREGATE_PROBE") != "1" {
		t.Skip("set WYD_AGGREGATE_PROBE=1 and WYD_TEST_POSTGRES_URL")
	}
	ctx := context.Background()
	st, err := NewPostgresStore(ctx, PostgresConfig{URL: url, MaxConns: 4, OperationTimeout: 500 * time.Millisecond})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		_, _ = conn.Exec(ctx, `DELETE FROM instance_state WHERE singleton=true`)
		_ = conn.Close(ctx)
	})
	now := time.Now().UTC()
	for _, runtimes := range []int{10, 50, 100} {
		snapshot := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, Channels: map[byte]*model.InstanceStateSnapshot{}}
		for channel := byte(1); channel <= 2; channel++ {
			state := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, NightmarePartyRuns: map[string]int{}}
			for i := range runtimes / 2 {
				members := make([]string, 8)
				for m := range members {
					members[m] = fmt.Sprintf("c%02d-%04d-%02d-0000-0000-000000000000", channel, i, m)
				}
				state.Instances = append(state.Instances, model.InstanceRuntimeState{
					RuntimeID: fmt.Sprintf("nightmare-%d-%04d", channel, i), ConfigID: "nightmare_normal", State: "running",
					MemberCharacterUIDs: members, LeaderCharacterUID: members[0], CurrentStage: 2,
					ScheduleEnd: now.Add(time.Hour), HardDeadline: now.Add(time.Hour), CombatDeadline: now.Add(30 * time.Minute),
				})
				state.NightmarePartyRuns[members[0]] = 1
			}
			snapshot.Channels[channel] = state
		}
		payload, _ := json.Marshal(snapshot)
		var durations []time.Duration
		for range 40 {
			started := time.Now()
			if err := st.SaveCriticalInstanceState(snapshot); err != nil {
				t.Fatal(err)
			}
			durations = append(durations, time.Since(started))
		}
		slices.Sort(durations)
		t.Logf("P10 instance aggregate: %d runtimes, %d bytes JSON, write p50=%s p95=%s max=%s",
			runtimes, len(payload), durations[len(durations)/2].Round(10*time.Microsecond),
			durations[len(durations)*95/100].Round(10*time.Microsecond), durations[len(durations)-1].Round(10*time.Microsecond))
	}
}
