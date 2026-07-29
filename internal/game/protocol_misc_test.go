package game

import (
	"encoding/binary"
	"testing"
)

func TestPingIsNativeNoResponseKeepalive(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	before := p.Session.QueuedPacketsForTest()

	w.onPing(p.Session, make([]byte, 12))

	if got := p.Session.QueuedPacketsForTest(); got != before {
		t.Fatalf("ping gerou resposta: fila %d -> %d", before, got)
	}
}

func TestReqRankingReturnsNearbyPlayerInfo(t *testing.T) {
	requester, _ := networkedTestPlayer(1, "Requester", 2100, 2100)
	target, _ := networkedTestPlayer(2, "Target", 2101, 2100)
	w := worldWithNetworkedPlayers(requester, target)
	requester.show(target.ID)
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(target.ID))
	beforeRequester := requester.Session.QueuedPacketsForTest()
	beforeTarget := target.Session.QueuedPacketsForTest()

	w.onReqRanking(requester.Session, pkt)

	if got := requester.Session.QueuedPacketsForTest(); got != beforeRequester+1 {
		t.Fatalf("consulta nao respondeu ao solicitante: %d -> %d", beforeRequester, got)
	}
	if got := target.Session.QueuedPacketsForTest(); got != beforeTarget {
		t.Fatalf("consulta vazou pacote ao alvo: %d -> %d", beforeTarget, got)
	}
}
