package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// sequenceRNG returns the queued values in order, then zero.
type sequenceRNG struct{ values []int }

func (r *sequenceRNG) Intn(n int) int {
	if len(r.values) == 0 {
		return 0
	}
	v := r.values[0]
	r.values = r.values[1:]
	if v >= n {
		return n - 1
	}
	return v
}

func TestPickWeatherFollowsWeights(t *testing.T) {
	cases := map[int]int{0: weatherClear, 59: weatherClear, 60: weatherRain, 84: weatherRain,
		85: weatherSnow, 94: weatherSnow, 95: weatherHeavySnow, 99: weatherHeavySnow}
	for roll, want := range cases {
		if got := pickWeather(roll); got != want {
			t.Fatalf("roll %d gave weather %d, want %d", roll, got, want)
		}
	}
}

func TestTickWeatherBroadcastsOnlyChanges(t *testing.T) {
	w := newZoneTestWorld()
	p := addZonePlayer(w, 1, 2100, 2100, 500)
	// First period (duration only), then duration + weather rolls: 70 = rain.
	w.weatherRNG = &sequenceRNG{values: []int{0, 0, 70, 0, 70, 0, 10}}
	origin := time.Unix(1_000_000, 0)
	w.tickWeather(origin)
	if w.weather != weatherClear || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("the first period changed the starting clear weather")
	}
	start := origin.Add(weatherMinDuration)

	w.tickWeather(start)
	if w.weather != weatherRain {
		t.Fatalf("weather %d, want rain", w.weather)
	}
	packet, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || len(packet) != 16 ||
		wire.ParseHeader(packet).Type != wire.OpWeather ||
		binary.LittleEndian.Uint32(packet[12:]) != weatherRain {
		t.Fatalf("rain was not broadcast as a 16-byte 0x18B packet")
	}
	if !w.nextWeatherChange.Equal(start.Add(weatherMinDuration)) {
		t.Fatalf("next change %v, want %v", w.nextWeatherChange, start.Add(weatherMinDuration))
	}

	w.tickWeather(start.Add(time.Minute))
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("weather changed before its period ended")
	}

	// A player whose client shows another weather is updated on the next sync.
	late := addZonePlayer(w, 2, 2100, 2100, 500)
	w.tickWeather(start.Add(2 * time.Minute))
	if late.SentWeather != weatherRain || late.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("a player showing old weather was not brought up to date")
	}
	late.Session.DequeuePacketForTest()

	w.tickWeather(start.Add(weatherMinDuration))
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("an unchanged weather was broadcast again")
	}

	w.tickWeather(start.Add(2 * weatherMinDuration))
	if w.weather != weatherClear || p.Session.QueuedPacketsForTest() != 1 || p.SentWeather != weatherClear {
		t.Fatalf("return to clear weather %d was not broadcast once", w.weather)
	}
}

func TestEnterWorldCarriesWeather(t *testing.T) {
	b := wire.EnterWorld(7, 0, model.Char{Name: "p", Score: &model.Score{}})
	wire.PutEnterWorldWeather(b, weatherSnow)
	if got := binary.LittleEndian.Uint16(b[1244:]); got != weatherSnow {
		t.Fatalf("MSG_CNFCharacterLogin.Weather = %d, want %d", got, weatherSnow)
	}
	if got := binary.LittleEndian.Uint16(b[1242:]); got != 7 {
		t.Fatalf("weather overwrote ClientID: %d", got)
	}
}

func TestSkillWeatherMirrorsClient(t *testing.T) {
	if got := skillWeatherAt(weatherHeavySnow, 2100, 2100); got != weatherSnow {
		t.Fatalf("heavy snow counts as %d, want snow", got)
	}
	// Field 28/22 always counts as snow, even in clear or rainy weather.
	if got := skillWeatherAt(weatherRain, 28*128+5, 22*128+5); got != weatherSnow {
		t.Fatalf("snow field counts as %d, want snow", got)
	}
	if got := skillWeatherAt(weatherRain, 26*128+5, 22*128+5); got != weatherRain {
		t.Fatalf("field 26/22 counts as %d, want rain", got)
	}
}

func TestWeatherModifiesSkillDamage(t *testing.T) {
	cases := []struct {
		weather, instance, want int
	}{
		{weatherRain, 2, 90},
		{weatherRain, 5, 130},
		{weatherRain, 3, 100},
		{weatherSnow, 3, 120},
		{weatherSnow, 5, 100},
		{weatherClear, 2, 100},
	}
	for _, c := range cases {
		got := applyWeatherDamage(100, c.weather, model.SkillDef{InstanceType: c.instance})
		if got != c.want {
			t.Fatalf("weather %d instance %d: %d, want %d", c.weather, c.instance, got, c.want)
		}
	}

	// End to end: a thunder skill hits harder in rain than in clear weather.
	w := &World{}
	ch := &model.Char{Class: 1, X: 2100, Y: 2100, Score: testScore(model.Score{Level: 100, Int: 500})}
	skill := model.SkillDef{Index: 10, InstanceType: 5, InstanceValue: 50}
	clear := w.baseSkillDamage(ch, skill)
	w.weather = weatherRain
	if rain := w.baseSkillDamage(ch, skill); rain <= clear {
		t.Fatalf("thunder in rain %d is not above clear %d", rain, clear)
	}
}
