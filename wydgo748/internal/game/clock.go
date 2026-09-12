package game

import (
	"math/rand"
	"time"
)

// clock.go -- fontes de tempo e aleatoriedade do World. Existem para tornar o
// gameplay TESTAVEL de forma determinista: reset de area, cooldown de mob,
// respawn e drop dependiam de time.Now()/rand globais, o que so podia ser
// testado com time.Sleep (lento e instavel). Producao continua usando o relogio
// e o RNG reais; os testes injetam implementacoes controladas.
//
// Regra de uso: dentro do game loop prefira w.now() a time.Now(). A migracao e
// incremental -- codigo ainda nao migrado continua correto, porque o default e
// exatamente o comportamento anterior.

// Clock e a fonte de tempo do mundo.
type Clock interface {
	Now() time.Time
}

// RNG e a fonte de aleatoriedade do mundo. Intn(n) devolve [0,n).
type RNG interface {
	Intn(n int) int
}

// realClock e o relogio de producao.
type realClock struct{}

func (realClock) Now() time.Time { return time.Now() }

// realRNG usa o gerador global de math/rand, o mesmo comportamento que o codigo
// tinha antes da injecao.
type realRNG struct{}

func (realRNG) Intn(n int) int { return rand.Intn(n) }

// now devolve o instante atual pelo relogio do mundo. Tolera World com clock
// zerado (construido em teste sem NewWorld) caindo no relogio real.
func (w *World) now() time.Time {
	if w.clock == nil {
		return time.Now()
	}
	return w.clock.Now()
}

// intn devolve um inteiro em [0,n) pelo RNG do mundo. n <= 0 devolve 0, imitando
// o cuidado que os chamadores ja tomam antes de sortear.
func (w *World) intn(n int) int {
	if n <= 0 {
		return 0
	}
	if w.rng == nil {
		return rand.Intn(n)
	}
	return w.rng.Intn(n)
}
