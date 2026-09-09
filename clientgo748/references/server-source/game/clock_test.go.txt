package game

import (
	"testing"
	"time"
)

func TestWorldClockFallsBackToRealWhenUnset(t *testing.T) {
	// World construido direto em teste (sem NewWorld) nao tem clock; now() deve
	// funcionar mesmo assim, senao qualquer teste antigo passaria a panicar.
	w := &World{}
	if w.now().IsZero() {
		t.Fatal("now() sem clock deveria cair no relogio real")
	}
	if got := w.intn(0); got != 0 {
		t.Fatalf("intn(0)=%d, quer 0", got)
	}
	if got := w.intn(5); got < 0 || got >= 5 {
		t.Fatalf("intn(5)=%d fora de [0,5)", got)
	}
}

func TestWorldUsesInjectedClockAndRNG(t *testing.T) {
	start := time.Date(2026, 7, 24, 12, 0, 0, 0, time.UTC)
	clock := newFakeClock(start)
	w := &World{clock: clock, rng: fixedRNG{value: 3}}

	if !w.now().Equal(start) {
		t.Fatalf("now()=%v, quer %v", w.now(), start)
	}
	clock.Advance(90 * time.Second)
	if want := start.Add(90 * time.Second); !w.now().Equal(want) {
		t.Fatalf("apos Advance now()=%v, quer %v", w.now(), want)
	}
	if got := w.intn(10); got != 3 {
		t.Fatalf("intn(10)=%d, quer 3 (RNG fixo)", got)
	}
	// O RNG fixo nunca pode devolver um valor fora do intervalo pedido.
	if got := w.intn(2); got != 1 {
		t.Fatalf("intn(2)=%d, quer 1 (clamp em n-1)", got)
	}
}

// TestQuestZoneResetHonorsDeadlineWithFakeClock prova o ganho real da injecao:
// a janela de 10 minutos e verificavel sem time.Sleep. Antes o unico jeito de
// exercitar o deadline era esperar de verdade.
func TestQuestZoneResetHonorsDeadlineWithFakeClock(t *testing.T) {
	start := time.Date(2026, 7, 24, 12, 0, 0, 0, time.UTC)
	clock := newFakeClock(start)
	w := newZoneTestWorld()
	w.clock = clock
	w.nextQuestZoneReset = start.Add(questZoneResetInterval)
	// Mantem o autosave fora deste teste: aqui so o reset de zona importa.
	w.nextAutoSave = start.Add(time.Hour)

	inside := addZonePlayer(w, 1, 2400, 2100, 500)

	// Um minuto depois o deadline ainda nao venceu.
	clock.Advance(time.Minute)
	w.tick()
	if inside.X != 2400 || inside.Y != 2100 {
		t.Fatalf("recolhido antes do deadline: (%d,%d)", inside.X, inside.Y)
	}

	// Passados os 10 minutos, o proximo tick recolhe.
	clock.Advance(questZoneResetInterval)
	w.tick()
	if !nearRecall(inside) {
		t.Fatalf("nao foi recolhido apos o deadline: (%d,%d)", inside.X, inside.Y)
	}
	if want := clock.Now().Add(questZoneResetInterval); !w.nextQuestZoneReset.Equal(want) {
		t.Fatalf("proximo reset=%v, quer %v", w.nextQuestZoneReset, want)
	}
}

// TestWorldOptionsInjectClockAndRNG cobre as opcoes publicas de injecao. Elas
// nao eram exercitadas por nenhum teste (os demais setam os campos direto), o
// que significa que uma quebra nelas passaria despercebida.
func TestWorldOptionsInjectClockAndRNG(t *testing.T) {
	start := time.Date(2026, 1, 1, 0, 0, 0, 0, time.UTC)
	fake := newFakeClock(start)
	w := &World{clock: realClock{}, rng: realRNG{}}

	WithClock(fake)(w)
	WithRNG(fixedRNG{value: 7})(w)
	if !w.now().Equal(start) {
		t.Errorf("WithClock nao aplicou: now()=%v", w.now())
	}
	if got := w.intn(100); got != 7 {
		t.Errorf("WithRNG nao aplicou: intn=%d", got)
	}

	// nil e ignorado de proposito: uma opcao vazia nao pode zerar o relogio e
	// fazer todo deadline do mundo disparar de uma vez.
	WithClock(nil)(w)
	WithRNG(nil)(w)
	if !w.now().Equal(start) {
		t.Error("WithClock(nil) substituiu o relogio ja injetado")
	}
	if got := w.intn(100); got != 7 {
		t.Errorf("WithRNG(nil) substituiu o RNG ja injetado: %d", got)
	}
}
