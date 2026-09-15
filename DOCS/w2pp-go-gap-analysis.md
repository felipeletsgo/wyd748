# Auditoria estática W2PP x WYD-Go

## Escopo e regra de evidência

Este documento consolida diferenças funcionais e arquiteturais observadas entre
o código legado em `W2PP/Code/TMSrv/` e o servidor atual em
`wydgo748/internal/`. O objetivo é identificar lacunas reais, diferenças
intencionais e extensões específicas do W2PP sem transformar o W2PP em fonte de
paridade para o WYD 7.48.

O W2PP é apenas material comparativo. Para afirmar `PARIDADE_NATIVA`, a
autoridade continua sendo o client/runtime 7.48 e a evidência nativa registrada
por decompilação/Ghidra. Uma rotina existente no W2PP pode ser customização,
experimento, código posterior ou comportamento incompatível com o alvo 7.48.

Esta é uma auditoria **estática**. Ela não declara `CLIENT_TESTED` e não implica
que cada fluxo listado foi exercitado em runtime.

### Estados usados

| Estado | Significado |
| --- | --- |
| `EQUIVALENTE` | O comportamento principal está representado nos dois projetos, sem divergência relevante encontrada nesta auditoria. |
| `DIFERENTE` | Ambos cobrem o domínio, mas contrato, lifecycle, persistência ou regras diferem. |
| `AUSENTE_NO_GO` | O fluxo específico do W2PP não tem implementação correspondente encontrada no Go. |
| `W2PP_CUSTOM` | Extensão específica do W2PP; não deve ser tratada automaticamente como lacuna do 7.48. |
| `INTENCIONALMENTE_REMOVIDO` | O comportamento existe ou existiu no legado, mas foi deliberadamente excluído do projeto atual. |
| `GO_ONLY/MODERNIZADO` | O Go possui controle ou arquitetura adicional sem equivalente direto relevante no W2PP auditado. |
| `NAO_VERIFICADO` | A origem/paridade nativa ainda precisa de evidência 7.48 antes de qualquer decisão de implementação. |

## Resumo executivo

A comparação não sustenta a hipótese de que o WYD-Go esteja simplesmente
"faltando o W2PP". Uma parte considerável do que aparece como arquivo/handler no
W2PP foi redistribuída no Go entre `game`, `data`, `store`, `wire`, `control` e
`webadmin`, ou foi substituída por uma arquitetura autoritativa mais explícita.

Os fluxos de composição **Alquimia** e **Extração**, inicialmente ausentes,
foram implementados e testados em 2026-09-15; falta validar a interação no
client. Eventos específicos como **Arena Real**,
**Coliseu N/M/A** e **Castle Zakum** também não têm a mesma máquina de estados no
Go, porém sua necessidade e origem 7.48 ainda devem ser comprovadas antes de
serem promovidas a requisito de paridade.

Não são lacunas: recompra desativada, visibilidade, ChangeCity/home city,
desafios entre jogadores, Cubo, guerras de guilda/torre, mounts básicos,
progressão em geral e os recursos modernos de quiz/global drop/WebAdmin. Vários
módulos do W2PP — donate/PIX, BattlePass, captcha, filtro de drop, teleport custom,
skins de montaria e compositores genéricos — são extensões próprias e opcionais.

## Matriz de protocolo e dispatcher

O `W2PP/Code/TMSrv/ProcessClientMessage.cpp` concentra handlers em um dispatcher
monolítico. O Go distribui o protocolo em handlers por domínio a partir de
`wydgo748/internal/game/world.go` e arquivos especializados.

| Domínio / fluxo | W2PP | WYD-Go | Estado | Impacto / recomendação |
| --- | --- | --- | --- | --- |
| Account secure | `_MSG_AccountSecure` valida modo/pacote e encaminha ao DBServer | autenticação, sessão e persistência são integradas à arquitetura Go | `DIFERENTE` | Não portar o encaminhamento ao DBServer; comparar apenas os efeitos observáveis exigidos pelo 7.48. |
| Entidade fora de visão | `_MSG_NoViewMob` | `internal/game/visibility.go` e testes de visibilidade autoritativa | `GO_ONLY/MODERNIZADO` | Não é handler ausente; comportamento foi realocado. |
| ChangeCity / cidade vinculada | `_MSG_ChangeCity.cpp` deriva a cidade de `TargetX/Y`; `DoRecall` lê os bits `0xC0` de Merchant | `playerHomeCitySpawn`, vínculo por passo confirmado e teleporte, `onChangeCity` idempotente | `GO_ONLY/MODERNIZADO` | Corrigida a dependência exclusiva do aviso do client; ver regressão abaixo. Não promover o destino planejado a posição autoritativa. |
| Desafio entre jogadores | lógica antiga aparece em infraestrutura de ranking/duel; `_MSG_ReqRanking.cpp` ativo hoje só consulta informação | `onPlayerChallenge` em `protocol_misc.go`, modos 0..4, TTL, validação de proximidade/visibilidade e proteção contra replay | `GO_ONLY/MODERNIZADO` | Não confundir `_MSG_ReqRanking` ativo com o lifecycle antigo comentado. |
| Recompra de item | opcode legado `0x3E8` | rejeição explícita testada em `security_test.go` | `INTENCIONALMENTE_REMOVIDO` | Não reintroduzir. A decisão de produto é não ter recompra. |
| Quiz | handlers legados incluem `_MSG_AnswerQuiz` | evento autoritativo em `quiz.go`/`quiz_round.go`, UI coordenada, token por rodada, deadline e entrega server-side | `GO_ONLY/MODERNIZADO` | Manter o servidor como fonte da verdade; respostas forjadas/tardias/repetidas devem continuar inválidas. |
| Integridade do client | sem equivalente direto relevante nesta comparação | challenge/response coordenado e testes em `client_integrity*` | `GO_ONLY/MODERNIZADO` | Recurso de segurança adicional do Go. |

### Correção de vínculo de cidade — 2026-09-15

`MODERNIZACAO_COMPATIVEL`, sem mudança de opcode ou layout. Comparação solicitada
com `W2PP/Code/TMSrv/_MSG_ChangeCity.cpp`, `W2PP/Code/Basedef.cpp`
(`BASE_GetVillage` e `g_pGuildZone`) e `W2PP/Code/TMSrv/Server.cpp` (`DoRecall`):
os limites das quatro cidades e os bits de vínculo já correspondiam à regra
comparada. A lacuna estava na atualização do vínculo, não na escolha do spawn.

O Go descartava `0x291` fora da cidade, mesmo quando o client já havia entrado
visualmente e o movimento autoritativo ainda estava em andamento. O aviso do
client não é repetido a cada passo. Teleportes também gravavam a conta sem
alterar os bits de cidade. Isso preservava Nippleheim após visitar Armia.

- `handlers.go`: estágio dos bits de cidade pela posição confirmada, commit
  com rollback dos dois scores e aviso repetido sem nova gravação.
- `player_movement.go`: vínculo após cada passo aceito, sem antecipar destinos
  bloqueados/cancelados; preserva a cidade atravessada mesmo se o lote termina
  fora dela. Falha de save permite nova tentativa no próximo passo/aviso.
- `teleports.go`: posição e vínculo participam do mesmo save/rollback.
- `quest.go`: teleporte de recompensa inclui o vínculo na transação já existente,
  sem segundo commit independente de itens/contadores.
- `home_city_test.go`: aviso antecipado, quatro cidades, login da conta salva,
  renascimento, repetição, rejeições, passagem pela borda, falha/retry de save
  e commit/rollback de teleporte comum e de quest.

Validação: `STATICALLY VERIFIED` e `AUTOMATED TESTED`. Os testes novos de entrada
e teleporte falharam antes da correção e passaram depois; executados também
`go test -count=1 ./...`, `go vet ./...`,
`go build -o bin/tm-home-city.exe ./cmd/server` e `git diff --check`.
`WYD_TEST_POSTGRES_URL` estava configurada para a suíte; a nova regressão de
quest usa store de teste e não comprova sozinha o fluxo real do jogo. Candidato gerado em
`wydgo748/bin/tm-home-city.exe`, sem substituir/reiniciar o `wydgo748/tm.exe`
em execução. `CLIENT_TESTED` pendente: visitar Armia partindo de Nippleheim,
sair/reentrar e morrer/renascer com o novo servidor instalado.

## Crafting e composição

O Go possui os sete compositores Tiny, Compositor, Agatha, Aylin, Lindy, Ehre
e Odin, além dos handlers de skill Extração e Alquimia. O caminho atual aplica validação
de snapshot/slots, rollback e persistência antes de publicar o resultado.

| Fluxo | W2PP | WYD-Go | Estado | Prioridade / decisão |
| --- | --- | --- | --- | --- |
| Alquimia | `W2PP/Code/TMSrv/_MSG_CombineItemAlquimia.cpp` e `GetMatchCombineAlquimia` | `onCombineAlquimia`: `0x2E1`/84 bytes, dez receitas, aprendizado, snapshot, refinação, consumo e rollback testados | `GO_ONLY/MODERNIZADO` | Implementado; interação in-game pendente. Sem custo de gold: `10 x level` pertence à skill 85. |
| Extração | `W2PP/Code/TMSrv/_MSG_CombineItemExtracao.cpp` | `onCombineExtracao`: `0x2D4`/20 bytes, equipamento, catalisador 1774, transformação e rollback testados | `GO_ONLY/MODERNIZADO` | Implementado; interação in-game pendente. Não confundir com extração Celestial. |
| Dedekinto | `_MSG_CombineItemDedekinto.cpp` | sem peer direto estabelecido | `W2PP_CUSTOM` / `NAO_VERIFICADO` | Não portar até comprovar a origem 7.48; envolve evolução/reset, taxas e transformações de IDs. |
| Shany | `_MSG_CombineItemShany.cpp` | sem peer direto estabelecido | `W2PP_CUSTOM` / `NAO_VERIFICADO` | Fluxo aleatório/configurável; tratar como opcional até evidência nativa. |
| Compositor genérico | `NewCompositor.cpp` | o Go possui compositor 7.48 específico, não o engine arbitrário DB/config-driven | `W2PP_CUSTOM` | Não é substituto obrigatório do compositor atual. |
| Jephi | `NewJephi.cpp` | nenhum peer direto estabelecido | `W2PP_CUSTOM` | Trocas/crafting específicos (peças, gold, pós, soul etc.); avaliar como feature independente. |

### Revisão das skills — 2026-09-15

Modo: `MODERNIZACAO_COMPATIVEL`. A consulta ao W2PP foi solicitada pelo usuário;
é referência semântica, não prova de paridade nativa. Os handlers Go atendem os
packets já emitidos pelo client atual (`TMFieldScene.cpp`, confirmação de
Extração; `MrItemMix.cpp`, Alquimia), sem mudar opcode/layout. O contrato de
crafting não foi promovido a `PARIDADE_NATIVA` nem a `CLIENT_TESTED`.

- `crafting_skills.go`: exige Hunter com skill aprendida, vivo, em mundo e sem
  bloqueio de persistência/loja; valida os 63 slots utilizáveis, snapshot e
  repetição. Alquimia cobre as dez receitas e requisitos +9; Extração usa
  maestria corrente e preserva os adicionais permitidos. Save precede sucesso
  publicado; falha restaura inventário e bloqueia nova mutação da conta.
- A receita zero de Alquimia usa sucesso/validade separados: o W2PP confunde
  esse índice com receita inválida. Não foram copiados o RNG reiniciado por
  pedido nem o retorno sem save do ramo de quantidade bônus.
- `affects.go`: Desintoxicar preserva regeneração positiva e usa aprendizado
  do conjurador para remover o efeito 32; Samaritano redireciona aggro hostil
  apenas no mesmo espaço de jogo. Limites de alvos e Limite da Alma têm
  testes específicos; o cooldown server-side de 102 é de um segundo, mas
  isso não comprova o comportamento da barra/cooldown no client.
- `skill_summons.go`: contagem por maestria, conflito de família e devolução
  de MP na rejeição; summons pertencem somente ao dono e não ocupam party.
  Criação parcial por esgotamento de IDs não devolve MP após criar criaturas.
- PvP: defesa dobrada no cálculo comum de skill, Ethereal com escolha entre
  queima de MP e dispel, e parry de Exterminar cobertos por testes. Propostas
  ofensivas para 79/86 foram descartadas: o catálogo 7.48 as define como buffs
  (`Affect` 37/31), agora exercitados pela rota real de cast.
- `TestAllSkillCatalogEntriesHaveClassifiedRoute` é triagem das entradas
  0..103 (103 reservada), não prova funcional de todas as passivas/affects.

Validação desta revisão: `go test -count=1 ./...`, `go vet ./...`,
`go build -o bin/server-skills-check.exe ./cmd/server` e testes focados com
`-race` de crafting, summons, Samaritano e Desintoxicar passaram. A variável
`WYD_TEST_POSTGRES_URL` estava configurada na suíte; os novos testes de rollback
dos handlers usam store de teste, não uma execução de crafting pelo jogo.
O candidato não foi instalado e nenhum servidor foi reiniciado.

No client, foi revisado o commit `92e16ad7` e reexecutado
`.agents/research/client748/skill-visual-audit.ps1`: 104 entradas, 89 ativas e
86.400 projeções de animação, sem falhas nos checks cobertos. Não houve nova
edição C++/asset nesta revisão, nem novo build C++. A evidência de build anterior
e as lacunas permanecem em
`../.agents/research/client748/flows/combat/skill-visual-dispatch.md`.
Ainda faltam inspeção dinâmica de partículas/poses (incluindo montaria),
efeitos criados fora do dispatcher, revisão semântica restante das passivas
e testes in-game de combate/crafting. `STATICALLY VERIFIED` e `AUTOMATED TESTED`
aplicam-se apenas aos checks descritos; não declarar todas as skills corretas.

## Eventos e instâncias

| Evento | W2PP | WYD-Go | Estado | Impacto / recomendação |
| --- | --- | --- | --- | --- |
| Arena Real | `ArenaReal.cpp` contém scheduler/estado específico | existe infraestrutura genérica de PvP/eventos, mas não a mesma Arena Real | `AUSENTE_NO_GO` + `NAO_VERIFICADO` | Prioridade média/opcional até comprovar que o comportamento pertence ao alvo desejado. |
| Coliseu N/M/A | classes `CColiseu*` com máquinas de estados próprias | engine de instância/waves existe, mas não os eventos específicos | `AUSENTE_NO_GO` + `NAO_VERIFICADO` | Implementar somente após definir regras e proveniência. |
| Castle Zakum | `CCastleZakum.cpp` | há gates, chaves, party, timers e bosses genéricos, não o evento Zakum completo | `AUSENTE_NO_GO` + `NAO_VERIFICADO` | A infraestrutura reduz o custo, mas o evento continua específico e opcional. |
| Cubo | `CCubo.cpp` | `big_cube.go` + `item_instances.go`: estágios, instância isolada, timers, contagem de mobs, quiz, EXP, persistência e recompensas | `DIFERENTE` / `GO_ONLY/MODERNIZADO` | Não tratar como ausência. Comparar regras pontuais somente se houver bug ou requisito concreto. |
| Drop global | extensão atual do projeto Go | `global_drop.go` + WebAdmin e testes | `GO_ONLY/MODERNIZADO` | Feature já implementada no modelo autoritativo do projeto. |

## Guildas, guerras e torre

O W2PP concentra a guerra de torre em `CWarTower.cpp`: scheduler por dia/hora,
countdown, spawn da torre, captura pela guilda que a mata, janela de guerra e
fama ao vencedor.

O Go possui implementação substancial em `internal/game/guild_wars.go` e
arquivos de guilda relacionados, com estado persistido, agenda, notices, spawn e
captura de torre, restrições de proprietário, janela de batalha, recompensa de
fama, recuperação após restart e integração com guerras de cidade. Os testes
`guild_war_test.go`, `guild_scheduled_wars_test.go` e
`guild_city_contract_test.go` exercitam partes importantes desse contrato.

| Fluxo | Estado | Observação |
| --- | --- | --- |
| Declaração de guerra entre guildas | `DIFERENTE` | O Go implementa `0xE0E`, valida autoria/alvo e lifecycle de declaração recíproca. |
| Guerra de torre agendada | `DIFERENTE` / modernizada | Há equivalência semântica ampla, mas scheduler, persistência e lifecycle não foram provados campo a campo idênticos ao W2PP. |
| Guerra de cidade | `GO_ONLY/MODERNIZADO` em relação ao recorte auditado | Integrada ao mesmo estado autoritativo de guerras no Go. |

O texto histórico em `DOCS/IMPLEMENTATION_STATUS.md` que sugere ausência de
guerras de guilda está **obsoleto** para o estado atual do repositório. Para
esta auditoria, o código e os testes atuais prevalecem sobre aquele registro.

## Ranking, player info e duel

Esses itens precisam ser separados porque o W2PP mistura código ativo e
infraestrutura antiga:

- `_MSG_ReqRanking.cpp` ativo apenas chama `ConsultaInfoPlayer(conn, tDuel)` e
  retorna; a lógica antiga de duel no arquivo está comentada.
- `Server.cpp` ainda contém infraestrutura como `ProcessRanking`/`DoRanking`.
- `Ranking.cpp` e `RequestAlias.cpp` mantêm ranking/top-list e integração com DB.
- O Go já tem `onPlayerChallenge` para convite/aceite de desafio, incluindo
  validação de espaço de gameplay, visibilidade, TTL e proteção contra replay.

Portanto, `ReqRanking`, leaderboard/player-info e duelo não devem ser
registrados como uma única feature ausente. Cada consumidor deve ser comparado
separadamente quando houver requisito concreto.

## Extensões específicas do W2PP

Os itens abaixo não são, por si, lacunas do WYD-Go. Eles devem ser tratados como
features opcionais e só entram na trilha de paridade caso apareça evidência
nativa 7.48 independente.

| Extensão W2PP | Fonte principal | Estado | Observação |
| --- | --- | --- | --- |
| Alias multiplexer | `RequestAlias.cpp` | `W2PP_CUSTOM` | Agrupa donate, mail, ranking, captcha, BattlePass, reconnect e outros serviços. |
| Teleporte arbitrário | `RequestTeleport.cpp` / handler correspondente | `W2PP_CUSTOM` | Coordenadas arbitrárias, cooldown e restrições próprias; distinto de portals/teleports/admin teleport do Go. |
| Donate shop / Ruby | `DonateShop.cpp`, `_MSG_ReqShopDonate` | `W2PP_CUSTOM` | Camada de monetização específica. |
| PIX / transferência de conta | `_MSG_SendPix`, `GeradorPix.cpp` | `W2PP_CUSTOM` | Campos SQL e fluxo de pagamento/transferência externos ao core 7.48. |
| reCaptcha | `reCaptcha.cpp` | `W2PP_CUSTOM` | Anti-bot/event-entry próprio. |
| Filtro/lista de drops | `FiltroDrop.cpp` e handlers de filtro/lista | `W2PP_CUSTOM` | UI/DB adicional. |
| Skins/trajes de montaria | `tMontarias.cpp`, `_MSG_SendTraje` | `W2PP_CUSTOM` | Não confundir com suporte básico a montarias, já existente no Go. |
| BattlePass | integração via aliases/handlers custom | `W2PP_CUSTOM` | Sistema de progressão/monetização opcional. |
| `_MSG_ReqNPC` custom | `NewCompositor.cpp` | `W2PP_CUSTOM` | Roteia compositor genérico próprio. |
| Aposta Freyja / `0x3D1` | handler legado | `W2PP_CUSTOM` / stub inativo | O código auditado retorna manutenção em determinada região; não é gap de core significativo. |
| ItemLevel staged rewards | `ItemLevel.cpp` | `W2PP_CUSTOM` / `NAO_VERIFICADO` | Progressão por chave interna, itens/buffs/sets específicos; exigir evidência nativa antes de adotar. |

## Quests e progressão

`_MSG_Quest.cpp` no W2PP é um switch monolítico com muitos caminhos. No Go a
progressão foi dividida entre `ascension.go`, `arch_crystals.go`, `celestial.go`,
`quest.go`, `quest_rewards.go` e respectivos testes.

Por isso, `_MSG_Quest` **não deve ser classificado em bloco como ausente**.
Branches concretos ainda não comparados permanecem `NAO_VERIFICADO`; quando uma
diferença gerar bug/requisito real, a análise deve ser feita branch a branch e
confrontada com evidência nativa 7.48.

## Recursos existentes no Go sem peer direto relevante no W2PP

Estes recursos são parte positiva da arquitetura atual e não devem ser
eliminados para aproximar artificialmente os projetos:

- desafio/resposta de integridade do client (`0x1C1/0x2C2`);
- mensagens coordenadas `0x105/0x106`;
- quiz autoritativo com round token, deadline e proteção contra
  forge/late/replay;
- persistência PostgreSQL transacional/assíncrona com fences de estado;
- WebAdmin como control plane;
- evento de drop global configurável;
- controles administrativos de boss, kick e teleport;
- isolamento moderno e estado durável de instâncias;
- contratos coordenados de score/HP ampliados e outras extensões versionadas.

## Lacunas confirmadas priorizadas

| Prioridade | Lacuna | Estado | Próximo gate correto |
| --- | --- | --- | --- |
| Alta | Alquimia | `GO_ONLY/MODERNIZADO` | Validar UI, receitas, falha e resultado persistido in-game com o candidato. |
| Alta | Extração (crafting W2PP) | `GO_ONLY/MODERNIZADO` | Validar seleção, catalisador, sucesso/falha e resultado persistido in-game. |
| Média/opcional | Arena Real | `AUSENTE_NO_GO` + `NAO_VERIFICADO` | Confirmar se pertence ao produto e ao alvo 7.48; então modelar scheduler/estado. |
| Média/opcional | Coliseu N/M/A | `AUSENTE_NO_GO` + `NAO_VERIFICADO` | Confirmar regras e proveniência; reutilizar engine de instâncias somente depois disso. |
| Média/opcional | Castle Zakum | `AUSENTE_NO_GO` + `NAO_VERIFICADO` | Confirmar requisito/proveniência antes de montar o evento sobre gates/party/timers existentes. |

## Itens que exigem evidência nativa antes de decisão

Não devem ser convertidos diretamente em tarefas de implementação apenas porque
existem no W2PP:

- Dedekinto;
- Shany;
- NewCompositor;
- NewJephi;
- Arena Real;
- Coliseu N/M/A;
- Castle Zakum;
- ItemLevel staged rewards;
- qualquer branch ainda não comparado de `_MSG_Quest.cpp`;
- qualquer extensão de monetização/UI/anti-bot listada como `W2PP_CUSTOM`.

## Matriz consolidada de dispatcher W2PP x Go

Esta matriz registra os pontos de entrada observados nos dispatchers. A ausência
de um nome idêntico não significa ausência funcional: no WYD-Go a separação foi
feita por domínio e handlers menores.

| W2PP | Go / domínio | Estado | Observação |
| --- | --- | --- | --- |
| `_MSG_AccountLogin`, `_MSG_CharacterLogin` | `onLogin`, `onEnterWorld` | `DIFERENTE` | Mesmo domínio de autenticação/entrada, com sessão e persistência próprias. |
| `_MSG_CreateCharacter`, `_MSG_DeleteCharacter`, `_MSG_CharacterLogout` | `onCreateCharacter`, `onDeleteCharacter`, `onCharacterLogout` | `DIFERENTE` | Lifecycle equivalente com validações adicionais. |
| `_MSG_AccountSecure` | autenticação/sessão Go | `DIFERENTE` | W2PP encaminha para DBServer; arquitetura Go não replica o fluxo. |
| `_MSG_MessageChat`, `_MSG_MessageWhisper` | `onMessageChat`, `onMessageWhisper` | `DIFERENTE` | Mesmo recurso com regras de servidor diferentes. |
| `_MSG_Action`, `_MSG_Action2`, `_MSG_Action3`, `_MSG_Illusion` | `onMove` | `DIFERENTE` | Movimento foi reorganizado em comandos autoritativos. |
| `_MSG_ActionStop`, `_MSG_REQMobByID`, `_MSG_Motion` | `onActionStop`, `onREQMobByID`, `onMotion` | `DIFERENTE` | Não são invenções Go; possuem equivalentes de opcode/proveniência. |
| `_MSG_UpdateScore` | descarte de update cliente | `GO_ONLY/MODERNIZADO` | O cliente não atualiza estado autoritativo no Go. |
| `_MSG_NoViewMob` | `visibility.go` | `GO_ONLY/MODERNIZADO` | Lógica redistribuída. |
| `_MSG_Restart`, `_MSG_Deprivate` | `onRestart`, guild handlers | `DIFERENTE` | Mesmo domínio com lifecycle próprio. |
| `_MSG_Challange`, `_MSG_ChallangeConfirm` | `onGuildChallenge` | `DIFERENTE` | Relacionado a guerra/cidade, não ao duelo de jogador. |
| `_MSG_ReqTeleport`, `_MSG_RequestTeleport` | `onReqTeleport` + admin teleport | `DIFERENTE` | Teleportes arbitrários W2PP são extensão própria. |
| `_MSG_REQShopList`, `_MSG_Buy`, `_MSG_Sell` | `onUseNPC`, `onBuyItem`, `onSellItem` | `DIFERENTE` | W2PP mistura shop comum com donate em alguns caminhos. |
| `_MSG_Deposit`, `_MSG_Withdraw` | `onCargoGold` | `DIFERENTE` | Go valida NPC, limites e persistência transacional. |
| `_MSG_SendReqParty`, `_MSG_AcceptParty`, `_MSG_RemoveParty` | party handlers | `DIFERENTE` | Domínio presente com regras próprias. |
| `_MSG_TradingItem`, `_MSG_Trade`, `_MSG_QuitTrade` | trade handlers | `DIFERENTE` | Go possui rollback e fences de estado. |
| `_MSG_UpdateItem`, `_MSG_DropItem`, `_MSG_GetItem`, `_MSG_DeleteItem`, `_MSG_SplitItem` | item handlers | `DIFERENTE` | Mesmo domínio; validação autoritativa modernizada. |
| `_MSG_UseItem`, `_MSG_ApplyBonus` | `onUseItem`, `onApplyBonus` | `DIFERENTE` | Fluxo equivalente com persistência própria. |
| `_MSG_SendAutoTrade`, `_MSG_ReqBuy` | ghost shop | `DIFERENTE` | Compra automática existe com modelo diferente. |
| `_MSG_Rebuy` | rejeição de opcode | `INTENCIONALMENTE_REMOVIDO` | Recompra foi removida do projeto. |
| `_MSG_CombineItem` | `onCombineCompositor` | `DIFERENTE` | Mesmo domínio, regras/configuração diferentes. |
| `_MSG_CombineItemTiny`, `_MSG_CombineItemLindy`, `_MSG_CombineItemAgatha`, `_MSG_CombineItemAilyn`, `_MSG_CombineItemEhre`, `_MSG_CombineItemOdin` | compositores Go | `DIFERENTE` | Fluxos equivalentes modernizados. |
| `_MSG_CombineDedekinto`, `_MSG_CombineDedekinto2`, `_MSG_CombineItemShany` | sem fluxo específico | `NAO_VERIFICADO` | Necessita validar se é produto alvo ou customização. |
| `_MSG_CombineItemAlquimia`, `_MSG_CombineItemExtracao` | `onCombineAlquimia`, `onCombineExtracao` | `GO_ONLY/MODERNIZADO` | Handlers e testes implementados; validação in-game pendente. |
| `_MSG_Quest` | quest/progressão Go | `DIFERENTE` | Comparação deve ser feita por branch. |
| `_MSG_AnswerQuiz` | quiz autoritativo | `GO_ONLY/MODERNIZADO` | Go adiciona proteção contra replay/forgery. |
| `_MSG_InviteGuild`, `_MSG_GuildAlly`, `_MSG_War` | guild handlers | `DIFERENTE` | Guerra de guild possui implementação moderna. |
| `_MSG_ReqRanking` | info/ranking commands | `DIFERENTE` | W2PP usa consulta de informação; não confundir com duelo. |
| `_MSG_PutoutSeal`, `_MSG_CapsuleInfo` | capsule handlers | `DIFERENTE` | Mesmo domínio com persistência server-side. |
| `_MSG_SendFiltro`, `_MSG_ReqDropList`, `_MSG_ReqdListNames` | nenhum equivalente direto | `W2PP_CUSTOM` | Sistema de filtro/lista adicional. |
| `_MSG_SendTraje` | suporte de montaria | `W2PP_CUSTOM` | Camada visual específica. |
| `_MSG_ReqShopDonate`, `_MSG_SendPix`, `_MSG_ReqAlias` | web/admin/serviços | `W2PP_CUSTOM` | Monetização e serviços externos. |
| `_MSG_SendItemLevel`, `_MSG_SendJephi`, `_MSG_ReqNPC` | sem mapeamento direto | `W2PP_CUSTOM` / `NAO_VERIFICADO` | Requer decisão de produto. |

### Casos fora do switch principal W2PP

- Ping (`0x3A0`) e SysQuit (`0x3AE`) são tratados fora da tabela principal; não
  devem ser classificados como invenções Go.
- Firework, TOTO e sistemas de aposta possuem evidência parcial em módulos
  legados; a equivalência exata com os handlers Go ainda deve ser confirmada.

### Casos desativados no W2PP

| Caso | Situação encontrada |
| --- | --- |
| `_MSG_Motion` | Case existente com chamada comentada/desabilitada. |
| `_MSG_UpdateScore` | Case sem atualização ativa de gameplay. |
| `_MSG_MestreGrifo` | Código comentado/desativado. |

## Inventário de módulos W2PP fora do dispatcher

| Módulo | Classificação |
| --- | --- |
| `ArenaReal.cpp` | `AUSENTE_NO_GO` + `NAO_VERIFICADO` |
| `CBatalhaReal.cpp` | Stub sem implementação significativa encontrada. |
| `CCastleZakum.cpp` | `NAO_VERIFICADO` / evento específico. |
| `CColiseuN/M/A.cpp` | `AUSENTE_NO_GO` + `NAO_VERIFICADO` |
| `CCubo.cpp` | `DIFERENTE`; Go possui instância moderna. |
| `CWarTower.cpp` | `DIFERENTE`; guerras de torre existem no Go com outro lifecycle. |
| `BattlePass.cpp` | `W2PP_CUSTOM` |
| `DonateShop.cpp` | `W2PP_CUSTOM` |
| `DropControl.cpp` | `DIFERENTE` no domínio de drop; regras específicas `W2PP_CUSTOM`. |
| `DropList.cpp` | `W2PP_CUSTOM` |
| `EventoClick.cpp` | `W2PP_CUSTOM` / `NAO_VERIFICADO` |
| `Freeze.cpp` | Utilitário de limpeza, não feature de gameplay. |
| `IPControl.cpp` | Utilitário anti-abuso. |
| `ItemLevel.cpp` | `W2PP_CUSTOM` / `NAO_VERIFICADO` |
| `MacroPerga.cpp` | `W2PP_CUSTOM` / `NAO_VERIFICADO` |
| `MailItem.cpp` | `W2PP_CUSTOM` |
| `MobKilled.cpp` | `DIFERENTE`; Go distribui recompensas e drops por domínio. |
| `Ranking.cpp` | Parcialmente coberto; comparar requisitos específicos. |
| `RequestAlias.cpp` | `W2PP_CUSTOM` |

## Cobertura da auditoria

A auditoria cobre os handlers ativos encontrados no dispatcher W2PP, os handlers
principais do dispatcher Go e os módulos externos que alteram gameplay. Itens
marcados como `NAO_VERIFICADO` permanecem deliberadamente fora de uma decisão de
implementação até existir evidência do client nativo 7.48 ou requisito explícito
do produto.

## Conclusão operacional

Para próximas correções de paridade, a ordem recomendada é:

1. escolher uma lacuna funcional concreta desta matriz;
2. localizar o contrato observável correspondente no client nativo 7.48;
3. registrar a evidência necessária até `CONTRACT`;
4. implementar um patch pequeno no servidor/client afetado;
5. executar o gate proporcional e manter `CLIENT_TESTED` separado de build/teste
   automatizado.

Essa ordem evita importar bugs ou customizações do W2PP e preserva o WYD-Go como
servidor autoritativo com contratos explícitos.
