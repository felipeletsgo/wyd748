package game

import (
	"context"
	"expvar"
	"log"
	"time"
)

// MODERNIZACAO_COMPATIVEL: admitted logins are recorded for the staff account
// page. The World only enqueues a record; one goroutine writes it, so login
// latency and the tick never wait for PostgreSQL.

const loginHistoryQueue = 256

var metricLoginHistoryDropped = expvar.NewInt("world_login_history_dropped_total")

type loginRecord struct {
	account  string
	clientIP string
}

type loginHistoryStore interface {
	RecordAccountLogin(context.Context, string, string, int) error
}

// startLoginHistory runs only for stores that keep login history.
func (w *World) startLoginHistory() {
	backend, ok := w.store.(loginHistoryStore)
	if !ok {
		return
	}
	queue := make(chan loginRecord, loginHistoryQueue)
	w.loginHistory = queue
	channel := int(w.operational.ChannelID)
	if channel == 0 {
		channel = 1
	}
	go func() {
		for record := range queue {
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			if err := backend.RecordAccountLogin(ctx, record.account, record.clientIP, channel); err != nil {
				log.Printf("login history account=%q: %v", record.account, err)
			}
			cancel()
		}
	}()
}

// recordLogin runs on the World goroutine and never blocks.
func (w *World) recordLogin(account, clientIP string) {
	if w.loginHistory == nil {
		return
	}
	select {
	case w.loginHistory <- loginRecord{account: account, clientIP: clientIP}:
	default:
		metricLoginHistoryDropped.Add(1)
	}
}
