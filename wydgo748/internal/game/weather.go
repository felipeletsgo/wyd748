package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// World weather values of 0x18B and MSG_CNFCharacterLogin.Weather.
const (
	weatherClear     = 0
	weatherRain      = 1
	weatherSnow      = 2
	weatherHeavySnow = 3
)

// Weather schedule (server design, not native parity). Each period lasts a
// random duration and draws the next weather from the weights; the client
// adapts the visuals per area (always snow in the 27-30 x 21-24 fields, rain
// instead of snow in some dungeons, clear in fields 29-30/22).
const (
	weatherMinDuration = 10 * time.Minute
	weatherMaxDuration = 25 * time.Minute
)

// weatherWeights are percentages for clear, rain, snow and heavy snow.
var weatherWeights = [4]int{60, 25, 10, 5}

// pickWeather draws a weather value from weatherWeights.
func pickWeather(roll int) int {
	for weather, weight := range weatherWeights {
		if roll < weight {
			return weather
		}
		roll -= weight
	}
	return weatherClear
}

// weatherSyncInterval bounds how often players are brought up to date.
const weatherSyncInterval = time.Second

// tickWeather changes the world weather when its period ends and sends the
// current value to every player whose client still shows another one. The
// schedule uses its own RNG so gameplay rolls keep their sequences, and a
// player whose save or teleport is still committing is updated on a later
// sync instead of receiving a packet mid-transaction.
func (w *World) tickWeather(now time.Time) {
	if w.weatherRNG != nil && !now.Before(w.nextWeatherChange) {
		// The world starts clear; the first period only schedules a change.
		first := w.nextWeatherChange.IsZero()
		span := int((weatherMaxDuration - weatherMinDuration) / time.Second)
		w.nextWeatherChange = now.Add(weatherMinDuration +
			time.Duration(w.weatherRNG.Intn(span+1))*time.Second)
		if !first {
			total := 0
			for _, weight := range weatherWeights {
				total += weight
			}
			w.weather = pickWeather(w.weatherRNG.Intn(total))
		}
	}
	if now.Before(w.nextWeatherSync) {
		return
	}
	w.nextWeatherSync = now.Add(weatherSyncInterval)
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.SentWeather == w.weather || w.criticalPlayerReserved(p) {
			continue
		}
		p.SentWeather = w.weather
		p.Session.Send(wire.Weather(0, uint32(w.weather)))
	}
}

// skillWeatherAt is the weather BASE_GetSkillDamage receives for a caster at
// (x, y), mirroring the client (SkillDamageWeather in WeatherPacket.h): heavy
// snow counts as snow, and the 27-30 x 21-24 fields always count as snow.
func skillWeatherAt(weather int, x, y uint16) int {
	if weather == weatherHeavySnow {
		weather = weatherSnow
	}
	fieldX, fieldY := int(x)>>7, int(y)>>7
	if fieldX > 26 && fieldX < 31 && fieldY > 20 && fieldY < 25 {
		weather = weatherSnow
	}
	return weather
}

// applyWeatherDamage applies the native weather modifiers of
// BASE_GetSkillDamage: rain weakens fire (-10%) and strengthens thunder
// (+30%); snow strengthens ice (+20%).
func applyWeatherDamage(damage, weather int, skill model.SkillDef) int {
	switch {
	case weather == weatherRain && skill.InstanceType == 2:
		return 90 * damage / 100
	case weather == weatherRain && skill.InstanceType == 5:
		return 130 * damage / 100
	case weather == weatherSnow && skill.InstanceType == 3:
		return 120 * damage / 100
	}
	return damage
}
