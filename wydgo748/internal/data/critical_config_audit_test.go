package data

import (
	"fmt"
	"strings"
	"testing"
)

func TestCriticalPersistenceConfigReservesPoolConnection(t *testing.T) {
	for pool := 2; pool <= 64; pool++ {
		for _, workers := range []int{1, pool - 1, pool} {
			t.Run(fmt.Sprintf("pool_%d_workers_%d", pool, workers), func(t *testing.T) {
				input := fmt.Sprintf("database_driver=postgres\ndatabase_max_conns=%d\ncritical_persistence_workers=%d\ncritical_persistence_capacity=65536\n", pool, workers)
				cfg, err := LoadServerConfig(writeServerConfig(t, input))
				if workers == pool {
					if err == nil || !strings.Contains(err.Error(), "critical_persistence_workers") {
						t.Fatalf("pool exhaustion must be rejected with a worker diagnostic: %v", err)
					}
					return
				}
				if err != nil {
					t.Fatal(err)
				}
				if cfg.DatabaseMaxConns != uint32(pool) || cfg.CriticalPersistenceWorkers != uint32(workers) || cfg.CriticalPersistenceCapacity != 65536 {
					t.Fatal("validated coordinator configuration was not preserved")
				}
			})
		}
	}
}

func TestCriticalPersistenceConfigRejectsInvalidBounds(t *testing.T) {
	for _, setting := range []string{
		"critical_persistence_workers=0", "critical_persistence_workers=-1",
		"critical_persistence_workers=4294967296", "critical_persistence_workers=abc",
		"critical_persistence_capacity=0", "critical_persistence_capacity=3",
		"critical_persistence_capacity=65537", "critical_persistence_capacity=-1",
		"critical_persistence_timeout_ms=99", "critical_persistence_timeout_ms=10001",
	} {
		t.Run(setting, func(t *testing.T) {
			if _, err := LoadServerConfig(writeServerConfig(t, setting+"\n")); err == nil {
				t.Fatal("invalid coordinator configuration was accepted")
			}
		})
	}
	for _, timeout := range []int{100, 10000} {
		input := fmt.Sprintf("critical_persistence_workers=1\ncritical_persistence_capacity=1\ncritical_persistence_timeout_ms=%d\n", timeout)
		if _, err := LoadServerConfig(writeServerConfig(t, input)); err != nil {
			t.Fatalf("inclusive configuration boundary rejected: %v", err)
		}
	}
	defaults := DefaultServerConfig()
	if defaults.DatabaseDriver != "postgres" || defaults.CriticalPersistenceWorkers == 0 || defaults.CriticalPersistenceWorkers >= defaults.DatabaseMaxConns || defaults.CriticalPersistenceCapacity < defaults.CriticalPersistenceWorkers {
		t.Fatal("production defaults cannot admit the critical coordinator safely")
	}
}
