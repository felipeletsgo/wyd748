// Package control defines the private, read-only v1 web/server contract.
package control

import (
	"context"
	"crypto/sha256"
	"crypto/subtle"
	"encoding/json"
	"errors"
	"expvar"
	"fmt"
	"io"
	"net"
	"net/http"
	"net/url"
	"strconv"
	"strings"
	"sync"
	"time"
	"unicode/utf8"
)

var ErrBusy = errors.New("control: busy")

type Query struct {
	Search string
	Offset int
}

const PageSize = 50

type Player struct {
	UID       string `json:"uid"`
	Name      string `json:"name"`
	Account   string `json:"account"`
	Class     uint8  `json:"class"`
	Level     uint32 `json:"level"`
	HP        uint32 `json:"hp"`
	MaxHP     uint32 `json:"maxHp"`
	MP        uint32 `json:"mp"`
	MaxMP     uint32 `json:"maxMp"`
	GuildID   uint16 `json:"guildId"`
	GuildRank uint8  `json:"guildRank"`
	SessionID int64  `json:"sessionId"`
	PartySize int    `json:"partySize"`
	X         uint16 `json:"x"`
	Y         uint16 `json:"y"`
}

type Overview struct {
	ModerationEpoch string      `json:"moderationEpoch"`
	Version         int         `json:"version"`
	AsOf            time.Time   `json:"asOf"`
	State           string      `json:"state"`
	UptimeSeconds   int64       `json:"uptimeSeconds"`
	Online          int         `json:"online"`
	Authenticated   int         `json:"authenticated"`
	Mobs            int         `json:"mobs"`
	QueueDepth      int         `json:"queueDepth"`
	TickMicros      int64       `json:"tickMicros"`
	LagMicros       int64       `json:"lagMicros"`
	Diagnostics     Diagnostics `json:"diagnostics"`
	Players         []Player    `json:"players"`
	Matched         int         `json:"matched"`
	Offset          int         `json:"offset"`
	HasMore         bool        `json:"hasMore"`
}

type Diagnostics struct {
	Version             int       `json:"version"`
	AsOf                time.Time `json:"asOf"`
	PostgresQueueDepth  int64     `json:"postgresQueueDepth"`
	PersistenceFailures int64     `json:"persistenceFailures"`
	Panics              int64     `json:"panics"`
}

type Source interface {
	Overview(context.Context, Query) (Overview, error)
}

// AccountPresenceSource is an optional embedded-control capability. It checks
// a bounded set of persisted account names in one World command, avoiding a
// full or paginated player snapshot for the administrative directory.
type AccountPresenceSource interface {
	AccountPresence(context.Context, []string) (map[string]bool, error)
}

// LoopbackAddress rejects wildcards, DNS names, and port zero outside tests.
func LoopbackAddress(address string) error {
	host, port, err := net.SplitHostPort(address)
	if err != nil {
		return fmt.Errorf("expected literal loopback host:port")
	}
	ip := net.ParseIP(host)
	n, err := strconv.Atoi(port)
	if ip == nil || !ip.IsLoopback() || err != nil || n < 1 || n > 65535 {
		return fmt.Errorf("expected literal loopback host and port 1..65535")
	}
	return nil
}

func ParseQuery(values url.Values) (Query, error) {
	q := Query{Search: strings.TrimSpace(values.Get("search"))}
	if utf8.RuneCountInString(q.Search) > 48 {
		return q, errors.New("search too long")
	}
	if raw := values.Get("offset"); raw != "" {
		v, err := strconv.Atoi(raw)
		if err != nil || v < 0 || v > 10000 {
			return q, errors.New("invalid offset")
		}
		q.Offset = v
	}
	return q, nil
}

func NewHandler(source Source, token string) (http.Handler, error) {
	if source == nil || len(token) < 32 {
		return nil, errors.New("control: source and 32+ byte token required")
	}
	want := sha256.Sum256([]byte("Bearer " + token))
	var mu sync.Mutex
	var next time.Time
	started := time.Now()
	mux := http.NewServeMux()
	mux.HandleFunc("GET /v1/overview", func(w http.ResponseWriter, r *http.Request) {
		got := sha256.Sum256([]byte(r.Header.Get("Authorization")))
		if subtle.ConstantTimeCompare(got[:], want[:]) != 1 || r.Header.Get("X-WYD-Actor") == "" {
			http.Error(w, "unauthorized", http.StatusUnauthorized)
			return
		}
		q, err := ParseQuery(r.URL.Query())
		if err != nil {
			http.Error(w, "invalid_query", 400)
			return
		}
		mu.Lock()
		if time.Now().Before(next) {
			mu.Unlock()
			w.Header().Set("Retry-After", "1")
			http.Error(w, "busy", 429)
			return
		}
		next = time.Now().Add(time.Second)
		mu.Unlock()
		ctx, cancel := context.WithTimeout(r.Context(), 2*time.Second)
		defer cancel()
		result, err := source.Overview(ctx, q)
		if err != nil {
			http.Error(w, "world_unavailable", 503)
			return
		}
		result = WithProcessDiagnostics(result, started)
		w.Header().Set("Content-Type", "application/json")
		_ = json.NewEncoder(w).Encode(result)
	})
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Cache-Control", "no-store")
		w.Header().Set("X-Content-Type-Options", "nosniff")
		mux.ServeHTTP(w, r)
	}), nil
}

// WithProcessDiagnostics completes a World snapshot for either control transport.
// Process metrics are sampled independently and do not change the World timestamp.
func WithProcessDiagnostics(result Overview, started time.Time) Overview {
	result.UptimeSeconds = int64(time.Since(started).Seconds())
	result.Diagnostics = Diagnostics{
		Version:             1,
		AsOf:                time.Now().UTC(),
		PostgresQueueDepth:  expvarInt("postgres_async_queue_depth"),
		PersistenceFailures: expvarInt("postgres_async_failures_total"),
		Panics:              expvarInt("world_panics_total"),
	}
	return result
}

func expvarInt(name string) int64 {
	v := expvar.Get(name)
	if metric, ok := v.(*expvar.Int); ok {
		return metric.Value()
	}
	return 0
}

type Client struct {
	base, token string
	http        *http.Client
}

func NewClient(base, token string) (*Client, error) {
	u, err := url.Parse(base)
	if err != nil || u.Scheme != "http" || u.User != nil || u.RawQuery != "" || u.Fragment != "" || (u.Path != "" && u.Path != "/") {
		return nil, errors.New("control: expected http loopback origin")
	}
	if err := LoopbackAddress(u.Host); err != nil {
		return nil, err
	}
	if len(token) < 32 {
		return nil, errors.New("control: token too short")
	}
	return &Client{base: strings.TrimRight(base, "/"), token: token, http: &http.Client{
		Timeout:       3 * time.Second,
		Transport:     &http.Transport{Proxy: nil, DialContext: (&net.Dialer{Timeout: time.Second}).DialContext, MaxConnsPerHost: 2, MaxIdleConnsPerHost: 2, IdleConnTimeout: 30 * time.Second, ResponseHeaderTimeout: 3 * time.Second},
		CheckRedirect: func(*http.Request, []*http.Request) error { return http.ErrUseLastResponse },
	}}, nil
}

func (c *Client) Overview(ctx context.Context, actor string, q Query) (Overview, error) {
	var result Overview
	v := url.Values{"search": {q.Search}, "offset": {strconv.Itoa(q.Offset)}}
	r, err := http.NewRequestWithContext(ctx, "GET", c.base+"/v1/overview?"+v.Encode(), nil)
	if err != nil {
		return result, err
	}
	r.Header.Set("Authorization", "Bearer "+c.token)
	r.Header.Set("X-WYD-Actor", actor)
	resp, err := c.http.Do(r)
	if err != nil {
		return result, errors.New("control unavailable")
	}
	defer resp.Body.Close()
	if resp.StatusCode == 429 {
		return result, ErrBusy
	}
	if resp.StatusCode != 200 {
		return result, errors.New("control unavailable")
	}
	data, err := io.ReadAll(io.LimitReader(resp.Body, 128*1024+1))
	if err != nil || len(data) > 128*1024 {
		return result, errors.New("invalid control response")
	}
	if err = json.Unmarshal(data, &result); err != nil || result.Version != 1 || result.AsOf.IsZero() || result.Diagnostics.Version != 1 || result.Diagnostics.AsOf.IsZero() || len(result.Players) > PageSize {
		return Overview{}, errors.New("invalid control contract")
	}
	return result, nil
}
