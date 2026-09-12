package loadtest

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"strings"

	"wydgo/internal/account"
	"wydgo/internal/model"
)

// AccountStore e o subconjunto de persistencia usado pelo provisionador. A
// conta de origem nunca e mutada; cada bot e uma conta nova com UIDs novos.
type AccountStore interface {
	LoadAccount(name string) (*model.Account, error)
	CreateAccount(acc *model.Account) error
}

// ProvisionConfig define a clonagem de contas para um ambiente descartavel.
type ProvisionConfig struct {
	SourceAccount   string
	SourceCharacter string
	// SourceSlot, quando informado, seleciona a posição exata no array de
	// personagens. Isso é necessário quando Mortal e Arch compartilham o
	// mesmo nickname (o caso da conta felipe). Nil preserva a seleção legada:
	// nome, ou primeiro personagem não vazio.
	SourceSlot      *int
	BotCount        int
	AccountPrefix   string
	CharacterPrefix string
	BotPassword     string
}

type ProvisionResult struct {
	Created       int
	Skipped       int
	SourceCreated bool
}

func (c ProvisionConfig) normalize() (ProvisionConfig, error) {
	if c.SourceAccount == "" {
		c.SourceAccount = "felipe"
	}
	if c.BotCount == 0 {
		c.BotCount = 990
	}
	if c.BotCount < 1 || c.BotCount > 990 {
		return c, fmt.Errorf("bot_count deve ficar entre 1 e 990")
	}
	if c.AccountPrefix == "" {
		c.AccountPrefix = "bot"
	}
	if c.CharacterPrefix == "" {
		c.CharacterPrefix = "Bot"
	}
	if len(c.AccountPrefix) < 1 || len(c.AccountPrefix)+4 > 12 {
		return c, fmt.Errorf("account_prefix deve gerar contas de 4 a 12 caracteres")
	}
	for _, ch := range c.AccountPrefix {
		if (ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') {
			return c, fmt.Errorf("account_prefix deve conter somente letras")
		}
	}
	if !strings.HasPrefix(strings.ToLower(c.AccountPrefix), "bot") {
		return c, fmt.Errorf("account_prefix deve comecar por bot")
	}
	if len(c.CharacterPrefix)+5 > 13 {
		return c, fmt.Errorf("character_prefix deve caber no limite de 13 caracteres")
	}
	for _, ch := range c.CharacterPrefix {
		if (ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') {
			return c, fmt.Errorf("character_prefix deve conter somente letras")
		}
	}
	if !account.ValidPassword(c.BotPassword) {
		return c, fmt.Errorf("senha dos bots deve ter 4 a 12 caracteres ASCII visiveis")
	}
	return c, nil
}

// ProvisionBotsFromSource materializa o modelo em um banco descartavel e
// depois cria os bots. Quando target ainda nao possui SourceAccount, a conta
// e copiada do source sem alterar o ponteiro carregado da base principal. Isso
// permite que o observador felipe entre no mesmo ambiente dos bots sem exigir
// que a base de carga seja um clone completo do PostgreSQL de producao.
//
// Se a conta modelo ja existir no target, ela e preservada: a ferramenta nunca
// sobrescreve uma conta existente, mesmo quando source e target sao bancos
// diferentes. Para atualizar o modelo, recrie a base descartavel ou remova a
// conta manualmente antes da rodada.
func ProvisionBotsFromSource(target, source AccountStore, cfg ProvisionConfig) (ProvisionResult, error) {
	cfg, err := cfg.normalize()
	if err != nil {
		return ProvisionResult{}, err
	}
	if target == nil || source == nil {
		return ProvisionResult{}, errors.New("store de carga/modelo ausente")
	}
	sourceAccount, err := source.LoadAccount(cfg.SourceAccount)
	if err != nil {
		return ProvisionResult{}, fmt.Errorf("carregar conta fonte %q: %w", cfg.SourceAccount, err)
	}
	if sourceAccount == nil {
		return ProvisionResult{}, fmt.Errorf("conta fonte %q ausente", cfg.SourceAccount)
	}
	result := ProvisionResult{}
	// Um banco descartavel novo nao contem nem o personagem de observacao. A
	// copia preserva a identidade da conta modelo neste banco isolado; os bots
	// abaixo recebem identidades novas por snapshot/persistencia.
	if existing, loadErr := target.LoadAccount(cfg.SourceAccount); loadErr != nil {
		if !errors.Is(loadErr, os.ErrNotExist) {
			return result, fmt.Errorf("verificar conta fonte no destino: %w", loadErr)
		}
		copyOfSource, copyErr := copyAccount(sourceAccount)
		if copyErr != nil {
			return result, fmt.Errorf("copiar conta fonte: %w", copyErr)
		}
		if createErr := target.CreateAccount(copyOfSource); createErr != nil {
			if !isAlreadyExists(createErr) {
				return result, fmt.Errorf("materializar conta fonte no destino: %w", createErr)
			}
		} else {
			result.SourceCreated = true
		}
	} else if existing == nil {
		copyOfSource, copyErr := copyAccount(sourceAccount)
		if copyErr != nil {
			return result, fmt.Errorf("copiar conta fonte: %w", copyErr)
		}
		if createErr := target.CreateAccount(copyOfSource); createErr != nil {
			if !isAlreadyExists(createErr) {
				return result, fmt.Errorf("materializar conta fonte no destino: %w", createErr)
			}
		} else {
			result.SourceCreated = true
		}
	}
	origin, err := selectCharacter(sourceAccount, cfg.SourceCharacter, cfg.SourceSlot)
	if err != nil {
		return result, err
	}
	hash, err := account.HashPassword(cfg.BotPassword)
	if err != nil {
		return result, fmt.Errorf("gerar hash dos bots: %w", err)
	}
	for i := 1; i <= cfg.BotCount; i++ {
		accountName := fmt.Sprintf("%s%04d", strings.ToLower(cfg.AccountPrefix), i)
		charName := generatedCharacterName(cfg.CharacterPrefix, i-1)
		clone, err := cloneForLoadtest(origin, accountName, charName, hash)
		if err != nil {
			return result, fmt.Errorf("clonar bot %s: %w", accountName, err)
		}
		if err := target.CreateAccount(clone); err != nil {
			if isAlreadyExists(err) {
				result.Skipped++
				continue
			}
			return result, fmt.Errorf("criar conta %s: %w", accountName, err)
		}
		result.Created++
	}
	return result, nil
}

func isAlreadyExists(err error) bool {
	if err == nil {
		return false
	}
	message := strings.ToLower(err.Error())
	return strings.Contains(message, "ja existe") ||
		strings.Contains(message, "already exists") ||
		strings.Contains(message, "duplicate")
}

func copyAccount(source *model.Account) (*model.Account, error) {
	if source == nil {
		return nil, errors.New("conta fonte ausente")
	}
	payload, err := json.Marshal(source)
	if err != nil {
		return nil, err
	}
	var clone model.Account
	if err := json.Unmarshal(payload, &clone); err != nil {
		return nil, err
	}
	return &clone, nil
}

func selectCharacter(acc *model.Account, name string, slot *int) (*model.Char, error) {
	if acc == nil {
		return nil, fmt.Errorf("conta fonte ausente")
	}
	if slot != nil && *slot < -1 {
		return nil, fmt.Errorf("slot de personagem fonte %d invalido", *slot)
	}
	if slot != nil && *slot >= 0 {
		if *slot >= len(acc.Chars) {
			return nil, fmt.Errorf("slot de personagem fonte %d fora do limite", *slot)
		}
		character := &acc.Chars[*slot]
		if character.Name == "" {
			return nil, fmt.Errorf("slot de personagem fonte %d vazio", *slot)
		}
		if name != "" && !strings.EqualFold(character.Name, name) {
			return nil, fmt.Errorf("slot de personagem fonte %d nao corresponde a %q", *slot, name)
		}
		return character, nil
	}
	for i := range acc.Chars {
		if name != "" && strings.EqualFold(acc.Chars[i].Name, name) {
			return &acc.Chars[i], nil
		}
	}
	if name != "" {
		return nil, fmt.Errorf("personagem fonte %q nao encontrado", name)
	}
	for i := range acc.Chars {
		if acc.Chars[i].Name != "" {
			return &acc.Chars[i], nil
		}
	}
	return nil, fmt.Errorf("conta fonte nao possui personagem")
}

func cloneForLoadtest(origin *model.Char, accountName, charName, passwordHash string) (*model.Account, error) {
	if origin == nil || origin.Score == nil {
		return nil, fmt.Errorf("personagem fonte sem Score")
	}
	clone := *origin
	clone.Name = charName
	clone.UID = ""
	clone.X, clone.Y = 1162, 1700
	clone.RuntimeScore = nil
	clone.ArchMortalUID = "" // o Mortal de origem nao e clonado para cada bot
	clone.GuildID, clone.GuildRank = 0, 0
	clone.QuestsDone = append([]int32(nil), origin.QuestsDone...)
	extended := *origin.Score
	extended.CurHP, extended.CurMP = extended.MaxHP, extended.MaxMP
	clone.Score = &extended
	if origin.AlternateCelestial != nil {
		alternate := *origin.AlternateCelestial
		if alternate.Score != nil {
			ext := *alternate.Score
			ext.CurHP, ext.CurMP = ext.MaxHP, ext.MaxMP
			alternate.Score = &ext
		}
		clone.AlternateCelestial = &alternate
	}
	// Itens sao copias visuais/estatisticas, nunca identidades. O Postgres
	// materializa UIDs novos no CreateAccount.
	for i := range clone.Equip {
		clone.Equip[i].UID = ""
	}
	for i := range clone.Inv {
		clone.Inv[i].UID = ""
	}
	clone.NextExp = 0
	acc := &model.Account{
		Name:         accountName,
		PasswordHash: passwordHash,
		Chars:        []model.Char{clone},
	}
	return acc, nil
}

// generatedCharacterName gera somente A-Z, pois o client 7.48 rejeita digitos
// no nickname. O prefixo curto + cinco letras suporta dezenas de milhoes.
func generatedCharacterName(prefix string, n int) string {
	prefix = strings.ToUpper(prefix)
	var suffix [5]byte
	for i := len(suffix) - 1; i >= 0; i-- {
		suffix[i] = byte('A' + n%26)
		n /= 26
	}
	return prefix + string(suffix[:])
}
