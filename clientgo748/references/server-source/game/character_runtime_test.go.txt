package game

import (
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

// Esta bateria cobre a classe de bug "estado que atravessa uma fronteira sem
// ser saneado" -- a mesma do personagem que entrava morto.
//
// Sao duas fronteiras:
//   - tela de selecao -> mundo: o Player e REUSADO, entao o que nao for zerado
//     vaza para o proximo personagem;
//   - ponteiro para dentro de uma fatia -> mutacao da fatia: p.Char aponta para
//     Account.Chars[slot], e um append pode realocar o array.

// --- fronteira 1: ponteiro para dentro da fatia ------------------------------

// TestArchCreationKeepsCharPointerValid cobre o pior dos bugs encontrados: o
// append que abre o slot do Arch pode REALOCAR Account.Chars e deixar p.Char
// apontando para o array antigo. O consumo dos itens ia para memoria orfa e o
// save gravava o array novo, onde a Pedra e o Sefirot continuavam intactos --
// ascensao repetivel.
//
// O cenario e o mais comum: conta com um Mortal vinda do JSON tem len==cap==1.
func TestArchCreationKeepsCharPointerValid(t *testing.T) {
	w, p, _ := newAscensionWorld(t)
	// Forca len == cap == 1, que e o que o append precisa para realocar.
	exato := make([]model.Char, 1)
	exato[0] = p.Account.Chars[0]
	p.Account.Chars = exato
	p.CharSlot = 0
	p.Char = &p.Account.Chars[0]
	prepareArchCandidate(p, 0)

	if !w.createArch(p.Session, p) {
		t.Fatal("a ascensao deveria ser tratada")
	}

	// A criacao agora encerra o personagem obrigatoriamente; portanto nenhum
	// ponteiro para o array antigo pode sobreviver na sessao.
	if p.Char != nil || p.CharSlot != -1 || p.InWorld {
		t.Error("runtime do Mortal permaneceu ativo depois da criacao do Arch")
	}
	// O consumo precisa estar no array QUE FOI PERSISTIDO, mesmo depois do
	// append ter realocado a fatia.
	mortal := p.Account.Chars[0]
	if mortal.Equip[eternalStoneSlot].Index != 0 {
		t.Errorf("a Pedra sobreviveu no array persistido: %d", mortal.Equip[eternalStoneSlot].Index)
	}
	if mortal.Equip[sefirotSlot].Index != 0 {
		t.Errorf("o Sefirot sobreviveu no array persistido: %d", mortal.Equip[sefirotSlot].Index)
	}
}

// --- fronteira 2: selecao -> mundo -------------------------------------------

// TestCharacterRuntimeIsFullyReset e a GUARDA DA CLASSE INTEIRA: por reflexao,
// exige que todo campo de escopo-personagem esteja zerado depois de sair do
// mundo. Um campo novo adicionado ao Player sem reset quebra este teste.
func TestCharacterRuntimeIsFullyReset(t *testing.T) {
	// Unicos campos de escopo-SESSAO: sobrevivem de proposito, porque a conta
	// continua autenticada na volta a selecao.
	sessionScoped := map[string]bool{
		"Session": true, "Account": true,
		// Trava de integridade da sessao: um character-logout jamais pode
		// limpar o estado que proibe persistir RAM parcial apos panic.
		"PersistencePoisoned": true,
	}

	session := net.NewTestSession(1, 64)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{
		Name:  "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100},
	}}}
	p := &Player{Session: session, Account: acc}

	// Suja TODO campo de personagem com valor nao-zero, por reflexao: assim o
	// teste nao depende de eu lembrar de preencher cada um.
	value := reflect.ValueOf(p).Elem()
	typ := value.Type()
	for i := 0; i < value.NumField(); i++ {
		name := typ.Field(i).Name
		if sessionScoped[name] {
			continue
		}
		field := value.Field(i)
		if !field.CanSet() {
			continue // campo nao-exportado: nao faz parte do contrato
		}
		switch field.Kind() {
		case reflect.Bool:
			field.SetBool(true)
		case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
			field.SetUint(7)
		case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
			field.SetInt(7)
		case reflect.Map:
			field.Set(reflect.MakeMap(field.Type()))
		case reflect.Ptr:
			field.Set(reflect.New(field.Type().Elem()))
		case reflect.Struct:
			if field.Type() == reflect.TypeOf(time.Time{}) {
				field.Set(reflect.ValueOf(time.Unix(1, 0)))
			}
		}
	}
	p.Char = &acc.Chars[0]
	p.InWorld = false // removePlayerFromWorld nao deve depender disso

	w := newZoneTestWorld()
	w.players[session] = p
	w.removePlayerFromWorld(p, "teste")

	for i := 0; i < value.NumField(); i++ {
		name := typ.Field(i).Name
		if sessionScoped[name] || name == "CharSlot" {
			continue // CharSlot vira -1 de proposito (ver abaixo)
		}
		field := value.Field(i)
		if !field.CanSet() {
			continue
		}
		if !field.IsZero() {
			t.Errorf("campo %q sobreviveu a saida do mundo (%v) -- zere-o em "+
				"resetCharacterRuntime, senao vaza para o proximo personagem",
				name, field.Interface())
		}
	}
	if p.CharSlot != -1 {
		t.Errorf("CharSlot=%d, quer -1 (nenhum personagem selecionado)", p.CharSlot)
	}
}

// TestSpecialCoinsDoNotLeakBetweenCharacters: as moedas sao do PERSONAGEM. Sem
// o reset, entrar com B depois de A deixava B com as moedas de A -- e o
// autosave as gravava no charstate de B, duplicando-as.
func TestSpecialCoinsDoNotLeakBetweenCharacters(t *testing.T) {
	session := net.NewTestSession(1, 64)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{
		Name:  "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100},
	}}}
	p := &Player{Session: session, Account: acc, Char: &acc.Chars[0],
		SpecialCoins: map[string]uint32{"evento": 500}}

	w := newZoneTestWorld()
	w.players[session] = p
	w.removePlayerFromWorld(p, "volta a selecao")

	if len(p.SpecialCoins) != 0 {
		t.Fatalf("as moedas de A sobreviveram para o proximo personagem: %v", p.SpecialCoins)
	}
}

// TestApplyCharStateReplacesCoins: mesmo que o reset falhe, applyCharState nao
// pode ACUMULAR -- ele reconstroi as moedas a partir do estado lido.
func TestApplyCharStateReplacesCoins(t *testing.T) {
	w := newZoneTestWorld()
	p := &Player{
		Char:         &model.Char{Name: "B", Score: &model.Score{Version: model.ScoreVersion}},
		SpecialCoins: map[string]uint32{"do_personagem_anterior": 999},
	}

	w.applyCharState(p, &model.CharState{
		Version:      model.CharStateVersion,
		SpecialCoins: map[string]uint32{"deste": 10},
	}, time.Now())

	if _, vazou := p.SpecialCoins["do_personagem_anterior"]; vazou {
		t.Errorf("applyCharState mesclou em vez de substituir: %v", p.SpecialCoins)
	}
	if p.SpecialCoins["deste"] != 10 {
		t.Errorf("as moedas do proprio personagem nao foram aplicadas: %v", p.SpecialCoins)
	}

	// Estado sem moedas precisa ZERAR, nao manter o que estava.
	w.applyCharState(p, &model.CharState{Version: model.CharStateVersion}, time.Now())
	if len(p.SpecialCoins) != 0 {
		t.Errorf("estado sem moedas deveria zerar: %v", p.SpecialCoins)
	}
}

// TestGuildInviteDoesNotSurviveCharacterSwitch: o convite e do PERSONAGEM. Sem
// o reset, dentro dos 30s do TTL dava para ser convidado com A, voltar a
// selecao, entrar com B e o B entrar na guild -- guildCommandAccept nao valida
// para quem era o convite.
func TestGuildInviteDoesNotSurviveCharacterSwitch(t *testing.T) {
	session := net.NewTestSession(1, 64)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{
		Name:  "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100},
	}}}
	p := &Player{Session: session, Account: acc, Char: &acc.Chars[0],
		GuildInviteFrom: 3, GuildInviteUntil: time.Now().Add(guildInviteTTL)}

	w := newZoneTestWorld()
	w.players[session] = p
	w.removePlayerFromWorld(p, "volta a selecao")

	if p.GuildInviteFrom != 0 || !p.GuildInviteUntil.IsZero() {
		t.Fatalf("convite de guild sobreviveu: from=%d ate=%v", p.GuildInviteFrom, p.GuildInviteUntil)
	}
}

// TestSkillCooldownsDoNotLeakBetweenCharacters.
func TestSkillCooldownsDoNotLeakBetweenCharacters(t *testing.T) {
	session := net.NewTestSession(1, 64)
	acc := &model.Account{Name: "conta", Chars: []model.Char{{
		Name:  "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100},
	}}}
	p := &Player{Session: session, Account: acc, Char: &acc.Chars[0],
		SkillReady: map[int]time.Time{26: time.Now().Add(time.Hour)}}

	w := newZoneTestWorld()
	w.players[session] = p
	w.removePlayerFromWorld(p, "volta a selecao")

	if len(p.SkillReady) != 0 {
		t.Fatalf("cooldowns sobreviveram para o proximo personagem: %v", p.SkillReady)
	}
}
