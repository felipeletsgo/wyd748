package game

import (
	"fmt"
	"log"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// dumpHPProjection mostra, lado a lado, os TRES numeros que decidem a barra de
// vida do proprio jogador:
//
//   - base      -- ExtendedScore persistido, sem buff;
//   - efetivo   -- ExtendedRuntime, com equipamento e affects (o que vai na
//     cauda wide dos pacotes, e o que o HUD patcheado deveria ler);
//   - projetado -- os WORDs de 16 bits do STRUCT_SCORE nativo, que e o que o
//     handler legado do client escreve no TMHuman.
//
// Serve para separar erro de servidor de erro de client sem depurador: se
// `efetivo` e `projetado` mantiverem a MESMA proporcao antes e depois da aura,
// o servidor mandou o certo e a divergencia esta em quem desenha.
//
// Comando de beta. Some quando a barra estiver resolvida.
func (w *World) dumpHPProjection(s *net.Session, p *Player) {
	if p == nil || p.Char == nil {
		return
	}
	base := p.Char.Extended
	efetivo := effectiveExtended(p.Char)
	if base == nil || efetivo == nil {
		s.Send(wire.MessagePanel("No extended score."))
		return
	}
	escala := model.CompatibilityHPScale(efetivo.MaxHP)
	projetado := efetivo.CompatibilityScore()

	razao := func(cur, max uint32) float64 {
		if max == 0 {
			return 0
		}
		return 100 * float64(cur) / float64(max)
	}
	efetivoPct := razao(efetivo.CurHP, efetivo.MaxHP)
	projetadoPct := razao(uint32(projetado.CurHP), uint32(projetado.MaxHP))

	linhas := []string{
		fmt.Sprintf("base  HP %d/%d", base.CurHP, base.MaxHP),
		fmt.Sprintf("wide  HP %d/%d (%.1f%%)", efetivo.CurHP, efetivo.MaxHP, efetivoPct),
		fmt.Sprintf("word  HP %d/%d (%.1f%%) escala=%d",
			projetado.CurHP, projetado.MaxHP, projetadoPct, escala),
	}
	for _, linha := range linhas {
		s.Send(wire.MessagePanel(linha))
	}
	// A divergencia entre as duas proporcoes e o veredito. Acima de 1 ponto
	// percentual o servidor projetou errado; abaixo disso, ele mandou coerente
	// e a barra torta e do client.
	divergencia := efetivoPct - projetadoPct
	if divergencia < -1 || divergencia > 1 {
		s.Send(wire.MessagePanel(fmt.Sprintf("DIVERGE %.1f pontos: erro no servidor", divergencia)))
	} else {
		s.Send(wire.MessagePanel("wide e word coerentes: barra torta e do client"))
	}
	afetos := 0
	for _, a := range p.Char.Affects {
		if a.Type != 0 {
			afetos++
		}
	}
	log.Printf("[#%d] HPDEBUG base=%d/%d wide=%d/%d(%.1f%%) word=%d/%d(%.1f%%) escala=%d affects=%d",
		s.ID, base.CurHP, base.MaxHP, efetivo.CurHP, efetivo.MaxHP, efetivoPct,
		projetado.CurHP, projetado.MaxHP, projetadoPct, escala, afetos)
}
