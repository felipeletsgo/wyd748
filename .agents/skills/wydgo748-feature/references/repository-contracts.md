# Contratos técnicos transversais

As regras de trabalho e o roteamento vivem apenas no `AGENTS.md` da raiz.
Esta referência mantém os invariantes do produto; não comprova que todos os
fluxos já passaram por validação integrada. Caminhos de packages e dados abaixo
são relativos a `wydgo748/`. Para o client, consulte `client-ui-748.md` e
`ghidra-client748.md`; para build e artefatos, `DOCS/build-and-integration.md`.

# Contratos arquiteturais

## World

`World` é o único dono do estado mutável de gameplay.

Nenhum outro package pode manter uma segunda fonte de verdade para estado de
jogo.

## Score

`Score` v2 é autoritativo.

O wire ativo usa `STRUCT_SCORE` de 140 bytes, equivalente a `model.Score`.
O contrato coordenado está em `DOCS/SCORE.md`; não há projeção legada ativa.

Nunca derive estado autoritativo de valores recebidos do client.

## Validação server-side

Validar no servidor, quando aplicável:

- packet;
- tamanho;
- opcode;
- fase;
- estado da sessão;
- alvo;
- identidade;
- distância;
- terreno;
- gameplay space;
- cooldown;
- item;
- UID;
- slot;
- preço;
- requisito;
- saldo;
- inventário;
- capacidade;
- membership;
- sequência;
- deadline.

Não confiar em campos do pacote apenas porque o client normalmente os envia
corretamente.

---

# Persistência e anti-dupe

PostgreSQL é a persistência autoritativa.

JSON existe somente como adaptador explícito de desenvolvimento:

```text
database_driver=json
```

Não existe fallback automático.

Se PostgreSQL estiver configurado e indisponível, o servidor deve falhar no
boot em vez de iniciar com outro estado.

Toda operação capaz de criar dupe segue:

```text
validar
→ criar snapshot/cópia
→ mutar estado
→ persistir
→ publicar
```

Se a persistência falhar:

```text
restaurar TODO o snapshot
```

Não fazer rollback parcial.

Especialmente revisar essa ordem em:

- craft;
- compra/venda;
- trade;
- ticket;
- instância;
- reward;
- ground drop;
- ascensão;
- counters;
- guild;
- inventário;
- equipamento.

Persistência e publicação nunca devem ficar em ordem inversa quando isso puder
confirmar ao client uma operação ainda não persistida.

---

# Identidades

Distinguir sempre:

```text
ClientID       = identidade efêmera da entidade live / wire
CharacterUID   = identidade estável do personagem
Item UID       = identidade estável server-side do item materializado
RuntimeID      = identidade server-side do gameplay space
```

`ClientID` pode ser reutilizado após disconnect.

Portanto `ClientID` não deve ser usado como identidade persistente ou durável
para:

- ownership;
- affects persistidos;
- reconnect;
- rewards;
- membership resumível;
- delayed actions long-lived;
- estado salvo no banco.

Sempre verificar risco de ID reuse em qualquer referência que sobreviva ao
tick ou à sessão que a criou.

UID de item nunca vai ao wire.

---

# Gameplay space

`RuntimeID` é uma fronteira autoritativa de gameplay.

```text
RuntimeID == ""  → mundo público
RuntimeID != ""  → runtime privado/compartilhado
```

Para entidades dinâmicas, dois participantes pertencem ao mesmo espaço somente
quando os RuntimeIDs são exatamente iguais.

A regra deve valer, conforme aplicável, para:

- Player ↔ Player;
- Player ↔ Mob;
- Mob ↔ Player;
- Mob ↔ Mob;
- summons;
- boss;
- skills;
- AoE;
- affects;
- AI;
- aggro;
- party EXP;
- drops;
- ground items;
- rewards;
- collision;
- movement;
- spawn;
- teleport;
- reconnect.

Nunca inferir gameplay space por coordenadas.

Entidades em runtimes diferentes podem ocupar exatamente as mesmas coordenadas.

NPCs globais, merchants e objetos permanentes devem possuir exceções explícitas
quando necessário. Não implementar exceção implícita apenas porque
`RuntimeID == ""`.

`Merchant != 0` nunca é hostil nem atacável.

---

# Visibilidade e atualização

Não usar `CreateMob` para atualizar jogador que já está visível.

Para entidade já materializada, usar packets incrementais apropriados:

- score;
- affects;
- equipamento;
- movimento;
- HP;
- estado.

`CreateMob` é para materialização inicial.

---

# Índice espacial

Usar o índice espacial para consultas locais.

Não varrer todos os mobs ou todos os players em ticks frequentes quando a
consulta é espacial.

Ao adicionar uma nova mecânica de tick, verificar explicitamente sua
complexidade.

---

# Inventário

Inventário:

```text
64 slots estruturais
63 slots visíveis
```

Cargo:

```text
128 slots estruturais
120 slots visíveis
```

Não confundir índice estrutural com célula visível do client.

---

# Dados autoritativos

Arquivos sob `data/` são a fonte autoritativa de conteúdo.

## Itens

`itemlist.csv` define:

- itens;
- efeitos estáticos;
- valores estáticos do item.

## Nomes

`Itemname.csv` substitui somente nomes.

Não deve alterar semântica.

## Efeitos

`ItemEffect.h` define a relação:

```text
ID ↔ EF_*
```

dos efeitos persistidos.

## Skills

`SkillData.csv` define todos os parâmetros das skills.

Não duplicar valores desses arquivos em:

- maps hardcoded;
- tabelas Go;
- switches de correção;
- overrides pós-load;
- constantes duplicadas.

Se um valor existe em arquivo autoritativo, o código deve carregá-lo.

---

# Organização

## model

Domínio puro.

Sem protocolo, store ou sessão.

## wire

Somente:

- protocolo;
- criptografia;
- serialização;
- builders.

Não colocar regra de gameplay.

## net

Somente:

- conexão;
- sessão;
- transporte.

Não colocar regra de jogo.

## store

Persistência.

## data

Loaders e validação dos arquivos autoritativos.

## game

Coordenação e regras de gameplay.

Fragmentar por feature.

Handlers devem:

```text
decodificar
→ validar envelope básico
→ rotear
```

Não colocar lógica extensa em:

```text
world.go
handlers.go
```

Criar arquivo específico da feature quando a regra crescer.

---

# Texto

Texto enviado ao client deve ser em inglês.

Logs e comentários internos podem ser em português.

---

# Protocolo WYD 7.48

O protocolo é byte-exato.

Nunca considerar um packet correto apenas porque os campos aparentam estar
certos.

Ao criar ou alterar packet:

1. confirmar opcode;
2. confirmar tamanho;
3. confirmar offsets;
4. confirmar tipos;
5. confirmar signed/unsigned;
6. confirmar padding;
7. confirmar ID do header;
8. confirmar comportamento do client;
9. comparar com packet nativo quando possível.

Um byte extra ou ausente pode fazer o client ou servidor rejeitar o packet.

Não portar layouts diretamente de 7.54/7.59 para 7.48.

---
