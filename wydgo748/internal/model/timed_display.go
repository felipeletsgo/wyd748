package model

import "time"

// Expiry display contract (EXTENSAO_COORDENADA over the native 7.48 tooltip).
//
// The authoritative lifetime is ExpiresUnix, which never travels on the 7.48
// wire. The native tooltip (FUN_00418828) instead reads the expiry from the
// item's instance effects, so the wire copy of a timed item carries a
// presentation-only projection of ExpiresUnix in its empty effect pairs:
//   - Angels/familiars 3900..3979: the REMAINING lifetime as EF_WDAY=days,
//     EF_HOUR=hours, EF_MIN=minutes ("2 day 23 hour 59 min");
//   - every other timed item: the expiry DATE as EF_WDAY=day,
//     EF_YEAR=year-2000, EF_WMONTH=month ("expire time 2026 year 10 month 15 day").
//
// The remaining time is a snapshot taken when the item is sent. Dates use the
// server process time zone (TZ in the service unit). Occupied effect pairs are
// never overwritten and the projection is never persisted.
const (
	effectWDay   = 106
	effectHour   = 107
	effectMin    = 108
	effectYear   = 109
	effectWMonth = 110
)

// ExpiryDisplayEffects returns the effect bytes sent to the client for item.
func ExpiryDisplayEffects(item Item, now time.Time) [6]byte {
	effects := item.Eff
	if item.Index == 0 || item.ExpiresUnix <= 0 {
		return effects
	}
	var values [3][2]byte
	if item.Index >= 3900 && item.Index < 3980 {
		if item.PausedUnix > 0 {
			now = time.Unix(item.PausedUnix, 0)
		}
		remaining := time.Unix(item.ExpiresUnix, 0).Sub(now)
		if remaining < 0 {
			remaining = 0
		}
		minutes := int64(remaining / time.Minute)
		days := minutes / (24 * 60)
		if days > 255 {
			days = 255
		}
		values = [3][2]byte{
			{effectWDay, byte(days)},
			{effectHour, byte(minutes / 60 % 24)},
			{effectMin, byte(minutes % 60)},
		}
	} else {
		expires := time.Unix(item.ExpiresUnix, 0).In(time.Local)
		values = [3][2]byte{
			{effectWDay, byte(expires.Day())},
			{effectYear, byte(expires.Year() - 2000)},
			{effectWMonth, byte(expires.Month())},
		}
	}
	next := 0
	for pair := 0; pair < 3 && next < len(values); pair++ {
		if effects[pair*2] != 0 {
			continue
		}
		effects[pair*2], effects[pair*2+1] = values[next][0], values[next][1]
		next++
	}
	return effects
}

// isExpiryDisplayEffect reports whether code is one of the projected fields.
func isExpiryDisplayEffect(code byte) bool {
	return code >= effectWDay && code <= effectWMonth
}
