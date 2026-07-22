# CLAUDE.md — WYD-Go

Guia operacional do emulador WYD 7.48 escrito em Go. A implementação é
server-authoritative: o client apresenta o estado e envia intenções; inventário,
atributos, combate, skills, progressão e persistência são decididos pelo servidor.

## Comandos obrigatórios

Execute a partir de `wyd-go`:

```powershell
go test ./...
go vet ./...
go build -o tm.exe ./cmd/server
go build -o account-api.exe ./cmd/account-api
go build -o account-create.exe ./cmd/account-create
```

O servidor aceita configuração central em `data/server.txt` e flags de linha de
comando como sobrescritas. Inicie na raiz do módulo para que os caminhos
`data/...` sejam encontrados. Pare qualquer `tm.exe` antigo antes de substituir
o binário.

## Arquitetura

Uma goroutine de `World` é dona de todo estado mutável. As sessões decodificam o
socket e enviam comandos ao game loop; o loop processa movimento, combate, itens,
party e ticks em ordem. Escritas de rede passam pela fila de cada sessão.

Regras de dependência:

- `model`: tipos puros de domínio;
- `wire`: framing, criptografia e builders byte a byte; não importa `game`;
- `net`: socket e sessão; não contém regra de jogo;
- `store`: interface de persistência e implementação JSON atômica;
- `data`: loaders de NPCGener, catálogos, terreno, drops e templates;
- `game`: único lugar que coordena o estado e chama as demais camadas;
- `account`: validação e hash compartilhados pela API, CLI e login.

O código de game é fragmentado por responsabilidade. Sessão, visibilidade,
equipamentos, skills, affects, party, movimento, terreno, drops, combate, trade,
teleportes e Loja Fantasma possuem arquivos próprios. Não volte a concentrar
features em um handler monolítico.

## Contrato único de atributos

`ExtendedScore` v2 é a única estrutura autoritativa:

- `Char.Extended`: base persistida do personagem;
- `Char.ExtendedRuntime`: cópia efetiva recalculada com equipamento e affects;
- `NPCDef.Extended`: atributos persistidos de NPCs e monstros;
- `Mob.HP`: recurso atual do monstro em `uint32`.

Arquivos de conta, NPC e template aceitam somente o formato atual, rejeitam
campos desconhecidos e exigem `extendedScore.version = 2`. Os campos `score`,
`baseScore`, atributos planos e senhas em texto são inválidos.

`model.WireScore` não é estado de domínio. Ele representa exclusivamente o
`STRUCT_SCORE` fixo de 28 bytes que o client 7.48 exige no protocolo. A projeção
é criada por `ExtendedScore.CompatibilityScore()` no momento do envio e nunca
volta aos cálculos nem aos JSONs.

## Fatos de protocolo que não podem regredir

- Header `_MSG` tem 12 bytes, little-endian. `FinishPacket` preenche Size/Tick e
  cifra o buffer; nunca reutilize o mesmo `[]byte` em broadcast.
- `0x364 CreateMob` tem 176 bytes: mesh@34, Affect[16]@66, Score@100 e Spawn@128.
- `0x336 UpdateScore` possui prefixo nativo de 92 bytes e cauda XSC2; o pacote
  patched tem 236 bytes. Score@12, affects compactos @42..73 e campos `uint32`
  do ExtendedScore @92..228, com assinatura XSC2@232.
- O affect compacto é `(tipoVisual << 8) | tempo`; tempo usa blocos de 8 s,
  arredondados para cima. `0x336` é público e atualiza o efeito visual.
- `0x3B9 UpdateAffect` tem 140 bytes e leva 16 estruturas completas. É enviado
  apenas ao dono para ícones, texto e timer.
- Ao entrar na visão, `CreateMob` já deve carregar os affects. Aplicação ou
  expiração posterior usa `publishPlayerAffects` ou `publishMobAffects`.
- Enter-world usa o ClientID real e a ordem: 0x114, self-0x364, 0x3A8, 0x336,
  0x3B9, 0x185, 0x337, 0x36B, 0x378, 0x181, 0x366 e sync regional ±32.
- Inventário possui 64 slots estruturais no wire e JSON, mas somente 0..62 são
  jogáveis. Cargo possui 128 estruturais e 120 visíveis. Slots sem UI devem
  permanecer zerados.
- Morte usa `0x165 RemoveMob(type=1)`; saída de visão usa 0; descarte do cadáver
  antes de recriar um player usa 3.
- Skills usam 0x39D/48B para 1 alvo, 0x39E/52B para 2 e 0x36C/96B para até 13.
  Os pares `{TargetID,Damage}` começam em @44.
- Movimento de mob usa 0x366 com `Speed@16`, `Effect=0@20` e destino de até
  `2×Speed` tiles. HeightMap é `int8`; rota exige delta `<8`, enquanto busca de
  spawn usa `<50`.
- Aprendizado de skill ocorre pela loja de skill (`0x379`, ShopType 3).
  `0x277` distribui somente stats (tipo 0) ou mastery (tipo 1).
- **Clique em NPC usa DOIS opcodes com layouts diferentes**, ambos no mesmo
  handler: `0x27B REQShopList` tem 16 bytes (só `TargetID@12`) e `0x28B` tem 20
  (`npcID@12`, `ClickOk@16`). O `0x28B` é o `_MSG_Quest` nativo (`139|C2G`) — o
  clique genérico e a quest são o mesmo pacote.
- **A confirmação de NPC é CLIENT-side.** O client decide sozinho se abre a
  janela "tem certeza?", olhando o tipo do NPC e o mesh de rosto: tipo 11 +
  rosto 58, além dos tipos 10/13/14/15. NPC com o gatilho **não envia pacote no
  primeiro clique** — só o "Sim" envia, já com `ClickOk=1`. NPC sem o gatilho
  envia `ClickOk=0` direto. Nos dois casos o servidor recebe UM pacote por
  interação; não existe pacote de "oferta" para gatear recompensa.
- Interação com NPC exige existência, visibilidade e **Chebyshev ≤ 8**. O raio
  veio de captura: o client envia o clique a pelo menos 6 tiles.
- **Guild no wire é um WORD `(canal << 12) | guildID`**, com o id nos 12 bits
  baixos — daí o teto de 4095. Fica em `CreateMob@98` (os 2 bytes entre
  `Affect[16]` e `Status@100`) e em `0x336@74`. O `@74` é um campo de 16 bits,
  **não** dois bytes separados. Guild vive no prefixo nativo, não na cauda XSC2.
- Canais de chat, todos no `0x334` e separados pelo prefixo do texto:
  `-` guild, `--` global, `=` party, `!` carta (painel H), sem prefixo local.
  A ordem do teste importa: `--` precisa ser reconhecido **antes** de `-`.
  `/spk` é anúncio de servidor (`Color=7`, consome o Shout 3330).
- Comandos de chat chegam por `0x333` **ou** por `0x334` conforme o foco da
  caixa; ambos passam pelo mesmo dispatcher, que aceita comando sem argumento.

## Dados autoritativos

- `data/character_templates.json`: nascimento, ExtendedScore v2, visual e itens;
- `data/server.txt`: caminhos e regras globais (`exp_minimum`, `exp_rate` e
  `party_exp_bonus`);
- `data/NPCGener.txt`: posição, população e rota; o template não guarda X/Y;
- `data/npcs/*.json`: ExtendedScore v2, aparência, EXP, carry, gold e loja;
- `data/itemlist.csv` + `Itemname.csv`: definição, preço, requisito e efeitos;
- `data/SkillData.csv`: custo, delay, alvo, alcance e parâmetros das skills;
- `data/droprate.json`: chance por slot do carry;
- `data/volatiles.json`: comportamento server-side de consumíveis;
- `data/guilds.json`: registro canônico de guilds. **Fica FORA do diretório de
  contas** — lá dentro os varredores de conta tentariam parseá-lo como conta e
  quebrariam o boot. `Char.GuildID/GuildRank` são cópias desnormalizadas,
  reparadas no login contra este arquivo;
- `data/quests.json`: allowlist de quest. NPC ausente daqui **nunca** vira
  quest; NPC inexistente ou de tipo reservado derruba o boot;
- HeightMap/AttributeMap: altura, bloqueio e reposicionamento no terreno.

Guild e contas afetadas são gravadas na MESMA transação (`SaveGameState`): o
journal carrega um tipo de destino explícito por entrada (`account`/`guilds`) e
o apply resolve o caminho pelo tipo, nunca pelo nome do arquivo.

Equip é serializado como objeto nomeado de 16 slots. Inventário é sempre uma
lista de 64 itens. Slots vazios de personagem são `null`, preservando sua
posição sem criar objetos incompletos. Contas possuem apenas `passwordHash`
PBKDF2; `pass` é inválido.

## Persistência e segurança

O autosave roda a cada 3 segundos. `JSONStore.SaveAccount` grava em temporário,
faz `Sync` e `rename`. Operações sensíveis podem salvar antes da confirmação ao
client. O game loop nunca aceita preço, efeito, dano, skill aprendida ou posição
de inventário sem validar contra o estado server-side. Uma conta não pode entrar
duas vezes simultaneamente.

## Como implementar uma feature

1. Confirme o pacote no log e o layout no source do client/W2PP.
2. Adicione opcode ou builder em `internal/wire` e teste os offsets.
3. Implemente a regra no arquivo de domínio correto em `internal/game`.
4. Registre a entrada no switch de `world.go`, se houver pacote C→S.
5. Atualize visibilidade, ExtendedRuntime e persistência quando necessário.
6. Rode testes, vet e builds; depois valide visualmente no client 7.48.

Porte algoritmos das sources 7.59/W2PP/Secrets, nunca structs, offsets ou
endereços. A especificação extensa fica em `DOCS/EMULATOR.md`; o contrato wide
em `DOCS/EXTENDED_SCORE.md`; o estado entregue em
`DOCS/IMPLEMENTATION_STATUS.md`; o roadmap em `../plan.md`.
