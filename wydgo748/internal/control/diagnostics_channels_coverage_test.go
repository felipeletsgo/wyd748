package control

import (
	"expvar"
	"testing"
)

func TestReadChannelsDecodesPublishedMetricsAndRejectsWrongShape(t *testing.T) {
	if expvar.Get(ChannelMetricsName) != nil {
		t.Skip("channel metrics are already registered")
	}
	want := map[string]ChannelReading{"1": {Players: 7, HeartbeatAgeMicros: 1200}}
	var published any = want
	expvar.Publish(ChannelMetricsName, expvar.Func(func() any { return published }))
	defer func() { published = nil }()

	if got := readChannels(); len(got) != 1 || got["1"] != want["1"] {
		t.Fatalf("channel readings=%v, want %v", got, want)
	}
	published = "invalid channel metric shape"
	if got := readChannels(); got != nil {
		t.Fatalf("invalid channel metrics returned %v", got)
	}
}
