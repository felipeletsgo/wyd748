package playerportal

import (
	"net/http"
	"net/netip"
	"strconv"
	"sync"
	"time"
)

const registrationBucketCapacity = 4096

type registrationBucket struct {
	expires time.Time
	count   int
}

type registrationRule struct {
	key    string
	limit  int
	window time.Duration
}

// Quotas count admitted attempts, including malformed JSON, validation failures,
// duplicates and backend failures. Windows start at the first admitted attempt,
// not at a wall-clock boundary; they are not rolling windows. State is bounded,
// process-local and resets on restart. This is not volumetric DDoS protection.
type registrationLimiter struct {
	mu        sync.Mutex
	buckets   map[string]registrationBucket
	nextSweep time.Time
}

func registrationRules(network string) [5]registrationRule {
	return [5]registrationRule{
		{"network:" + network + ":hour", 5, time.Hour},
		{"network:" + network + ":day", 10, 24 * time.Hour},
		{"global:minute", 30, time.Minute},
		{"global:hour", 300, time.Hour},
		{"global:day", 1000, 24 * time.Hour},
	}
}

// admit checks and charges all quotas atomically. Rejected attempts do not
// extend their lockout or charge other quotas. Sweeping is amortized rather
// than scanning the entire bounded map for every request under a flood.
func (l *registrationLimiter) admit(network string, now time.Time) time.Duration {
	if network == "unknown" {
		return time.Minute
	}
	l.mu.Lock()
	defer l.mu.Unlock()
	if l.buckets == nil {
		l.buckets = make(map[string]registrationBucket)
	}
	if !now.Before(l.nextSweep) {
		for key, bucket := range l.buckets {
			if !now.Before(bucket.expires) {
				delete(l.buckets, key)
			}
		}
		l.nextSweep = now.Add(time.Minute)
	}
	rules := registrationRules(network)
	var pending [5]registrationBucket
	var wait time.Duration
	missing := 0
	for i, rule := range rules {
		bucket, exists := l.buckets[rule.key]
		if !exists {
			missing++
		}
		if !exists || !now.Before(bucket.expires) {
			bucket = registrationBucket{expires: now.Add(rule.window)}
		}
		if bucket.count >= rule.limit && bucket.expires.Sub(now) > wait {
			wait = bucket.expires.Sub(now)
		}
		pending[i] = bucket
	}
	if wait > 0 {
		return wait
	}
	if len(l.buckets)+missing > registrationBucketCapacity {
		return time.Minute // Fail closed; never evict active quotas to admit traffic.
	}
	for i, rule := range rules {
		pending[i].count++
		l.buckets[rule.key] = pending[i]
	}
	return 0
}

func limitNetwork(peer string) string {
	ip, err := netip.ParseAddr(peer)
	if err != nil {
		return "unknown"
	}
	ip = ip.WithZone("").Unmap()
	if ip.Is4() {
		return ip.String()
	}
	return netip.PrefixFrom(ip, 64).Masked().String()
}

func localHealthProbe(r *http.Request) bool {
	if r.Method != http.MethodGet || (r.URL.Path != "/healthz" && r.URL.Path != "/readyz") || r.Header.Get("X-Forwarded-For") != "" {
		return false
	}
	addr, err := netip.ParseAddrPort(r.RemoteAddr)
	return err == nil && addr.Addr().Unmap().IsLoopback()
}

func retrySeconds(wait time.Duration) string {
	return strconv.FormatInt(max(1, int64((wait+time.Second-1)/time.Second)), 10)
}
