package control

import (
	"net/url"
	"testing"
)

func TestParseQueryFilters(t *testing.T) {
	q, err := ParseQuery(url.Values{"minLevel": {"10"}, "maxLevel": {"20"}, "class": {"0"}, "guild": {"5"}, "x": {"2100"}, "y": {"2100"}, "radius": {"16"}})
	if err != nil || q.MinLevel != 10 || q.MaxLevel != 20 || !q.HasClass || q.Class != 0 || q.GuildID != 5 || q.Radius != 16 || !q.Optional() {
		t.Fatal(q, err)
	}
	if q, err := ParseQuery(url.Values{}); err != nil || q.Optional() || q.HasClass {
		t.Fatal("default query must be the non-optional first page", q, err)
	}
	for _, bad := range []url.Values{
		{"minLevel": {"0"}}, {"class": {"256"}}, {"guild": {"x"}}, {"radius": {"5"}},
		{"x": {"1"}, "radius": {"5"}}, {"x": {"1"}, "y": {"1"}}, {"minLevel": {"30"}, "maxLevel": {"20"}}, {"radius": {"257"}, "x": {"1"}, "y": {"1"}},
	} {
		if _, err := ParseQuery(bad); err == nil {
			t.Fatal("accepted", bad)
		}
	}
}

func TestBucketP99UsesIntervalDeltas(t *testing.T) {
	var before, after [len(DurationBucketLabels)]int64
	before[7] = 50 // old slow ticks must not leak into the next interval
	after = before
	after[0] += 99
	after[3] += 1
	if got := BucketP99(before, after); got != 1000 {
		t.Fatal(got)
	}
	after[8] += 5
	if got := BucketP99(before, after); got != OverflowMicros {
		t.Fatal(got)
	}
	if got := BucketP99(after, after); got != 0 {
		t.Fatal(got)
	}
}
