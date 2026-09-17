# Skills: buffs e passivas, revisão individual

Atualizado: 2026-09-17. HEAD observado: `34ad1c22fc809f87f8db773f6cba0f0ddc80e076`.
Estado: lote servidor `AUTOMATED TESTED`; auditoria global **incompleta**.

## Escopo e autoridade

O lote inicial iniciou a revisão individual de buffs e passivas, consultando W2PP. O pedido mais
recente foi revisar todo o diff sem commit, incluindo os ajustes posteriores
de UI/chat; essa revisão está registrada abaixo. A árvore atual e o pedido
mais recente prevalecem sobre este registro. Não declarar todos os
buffs/visuais validados.

- Go: `wydgo748/internal/game/{affects,skill_passives,crafting_skills,equipment}.go`.
- Catálogo de IDs/nomes: `tmproject/client748/SkillData.csv` (nomes nem sempre
  correspondem à semântica histórica; identificar por ID e classe).
- Referência secundária local: `W2PP/Code/Basedef.cpp`,
  `W2PP/Code/TMSrv/{GetFunc,_MSG_Sell,_MSG_CombineItemAlquimia,_MSG_CombineItemExtracao}.cpp`.
- Visual/animação: ficha existente
  [skill-visual-dispatch](../research/client748/flows/combat/skill-visual-dispatch.md).
- Grafo extraído neste lote:
  [passive-score-flow.tsv](../research/client748/exports/passive-score-flow.tsv).
  Ghidra 12.1.3, projeto `C:/Users/felipe/Tools/GhidraProjects/WYD748Native_20260821`,
  programa `WYD.exe`, somente leitura, sem reanálise. SHA256 do programa:
  `8aa2f918844bce3afe21f1204f69757a443e32eb2f2f616936b1d9bfe215f593`.

## Correções deste lote

Todas classificadas `MODERNIZACAO_COMPATIVEL`: cálculo/validação interna no
servidor, sem novo opcode, layout ou modificação do client.

1. **81 Ligação Espectral**: aplica bônus de venda ao NPC pelo aprendizado HT
   bit 9, usando especialização runtime 2. W2PP `_MSG_Sell.cpp:168`:
   `bonus=min(mastery/10+6,26)/2`, preço multiplicado por `(50+bonus)/50`.
   Exige preço anterior ao imposto >1000; exclui 693/694/695. Mantidos o preço
   base e a política de impostos do Go, que NÃO são idênticos ao W2PP.
   Testes de classe, bit, arredondamento, teto, venda, replay, limite de gold,
   ausência/distância do NPC e rollback de inventário/UID/gold/tesouro.
2. **84 Alquimia**: recusa ingrediente empilhado em qualquer um dos três pares
   de efeitos, inclusive efeito de quantidade duplicado. Antes só verificava
   o primeiro par, permitindo consumir uma pilha inteira como unidade.
   Testes dos três pares e preservação do inventário/ausência de save na recusa.
3. **45 Toque da Athena**: respeita o teto de especialização 320 que o próprio
   recálculo Go e W2PP Type 15 usam. Regressão demonstrou `[339,358,359,139]`
   antes do patch; agora `[320,320,320,139]`. Testa aplicação pela skill,
   recálculo repetido, base intacta e expiração no relógio injetado.

Arquivos deste lote: `skill_passives.go`, `skill_passives_test.go` (novo),
`handlers.go` (uma chamada no preço de venda), `crafting_skills.go`,
`crafting_skills_test.go`, `affects.go`, `affects_test.go`, este registro e o TSV.
Demais deltas já estavam na árvore; foram preservados. Nenhuma remoção.

## Fila individual

IDs abaixo são zero-based. Inclui cada registro `Passive=1` e cada affect
não agressivo do CSV, mais Alquimia. `Anterior` significa implementação/teste
presente na árvore, NÃO revisão individual encerrada neste lote. `Pendente`
significa que falta fechar fórmula, entrada ou lifecycle; não significa ausência
de implementação. Efeitos ofensivos/debuffs e curas continuam fora deste lote.

| ID | Nome no catálogo | Situação / próxima verificação |
| --- | --- | --- |
| 3 | Samaritano | Pendente: affect 24, defesa e transferência de aggro |
| 9 | Mestre das Armas | Divergência de duas armas; ver seção abaixo |
| 11 | Assalto | Anterior: affect 13; revisar multiplicador final |
| 13 | Possuído | Anterior: affect 14; W2PP multiplica bônus em evoluções superiores, comparar contrato |
| 14 | Noção de Combate | Anterior: hook de combate; falta fechar evidência individual |
| 15 | Armadura Crítica | Anterior: affect 50 normalizado para 31, visual 24; bônus aprendido separado |
| 41 | Velocidade | AUTOMATED TESTED: affect 2, velocidade consumida pelo movimento, grupo/limite, custo, cooldown, recast e expiração; visual pendente |
| 43 | Escudo Mágico | AUTOMATED TESTED: fórmula base affect 11 e lifecycle; interação do aprendizado de 43 com 44 ainda pendente |
| 44 | Arma Mágica | AUTOMATED TESTED: fórmula base affect 9 e lifecycle/grupo; W2PP triplica se Foema bit 19, ainda não portado nem validado no 7.48 |
| 45 | Toque da Athena | Corrigido/testado neste lote: teto, base, repetição, expiração |
| 46 | Controle de Mana | AUTOMATED TESTED: regra expressa do usuário 90% MP / 10% HP, sem reserva; insuficiência transborda para HP no mesmo golpe; removido SaveMana indevido; teste in-game pendente |
| 47 | Cancelamento | Anterior: remove só primeira imunidade, cooldown 1s; teste passou |
| 53 | Proteção Elemental | Pendente: affect 25, defesa e resistências |
| 64 | Lobisomem | Anterior: transformação; não contar teste isolado como todos os modelos validados |
| 65 | Armadura Elemental | Anterior: redução com escudo + bônus do lobo; consumidores físico/skill/mob |
| 66 | Homem Urso | Anterior: transformação; teste preserva HP cheio em recálculo posterior |
| 67 | Escudo do Tormento | Anterior: escudo equipado, `(AC+1)/7` e bônus do urso |
| 68 | Astaroth | Pendente: transformação e interação com 69 |
| 69 | Metamorfose Superior | Anterior: bônus da transformação; fechar evidência individual |
| 70 | Titã | Pendente: transformação completa |
| 71 | Éden | Anterior: bypass do requisito de escudo na absorção; transformação pendente |
| 74 | Agressividade | Pendente: affect 27 versus bônus permanente W2PP bit 2 |
| 75 | Encantar Gelo | Pendente: ID tratado como invisibilidade/affect 28, não confiar no nome |
| 76 | Imunidade | Pendente: affect 19 e consumidores de resistência/cancelamento |
| 77 | Meditação | Pendente: affect 21, ataque e penalidade de defesa |
| 78 | Lança de Ferro | Corrigido/testado na revisão: segundo alvo independente do MISS primário, preservando crítico/golpe duplo e lifecycle de morte |
| 79 | Tempestade de Raios | Anterior: affect 37; W2PP bônus aprendido +200 diverge, não portar cegamente |
| 81 | Ligação Espectral | Venda corrigida/testada; precisão não comprovada |
| 82 | Perícia de Caçador | Divergência de duas armas; sem conclusão nativa, ver abaixo |
| 83 | Extração | Revisados autorização, receita, dado estrito, runtime mastery, replay e rollback; testes passaram |
| 84 | Alquimia | Dez receitas testadas; corrigida rejeição de pilhas; runtime/replay/rollback passaram |
| 85 | Escudo Dourado | Anterior: custo gold persistido antes da publicação; revisar fórmula |
| 86 | Explosão Etérea | Anterior: affect 31, visual 31 distinto da skill 15; revisar fórmula |
| 87 | Troca de Espírito | Pendente: affect 38 e arredondamento MP para HP |
| 89 | Evasão Aprimorada | Pendente: affect 26 e escala no cálculo de acerto |
| 90 | Visão de Caçadora | Fórmula crítica W2PP confirmada no código Go: bit 18, mínimo 4; teste de bits passou |
| 91 | Olhos de Aguia | Sem efeito de aprendizado comprovado; não atribuir o bônus de 90 nem o alcance de 92 |
| 92 | Toxina de Serpente | Alcance mínimo 2, bit 20 confirmado em FUN_0054eb20; teste passou |
| 93 | Lamina Aérea | Anterior: proc 1/4, bit 21, 0x39E/DMGX; teste passou, visual pendente |
| 96 | Poder Superior | Affect 4 tem customização explícita de dano percentual; preservar, revisar aplicação |
| 101 | Força Espectral | Anterior: +1 alcance inclusive desarmado; teste passou |
| 102 | Limite da Alma | Anterior: cooldown 1s, slot 15, duração por evolução; testes passaram |

## Lacunas concretas, não repetir pesquisa sem mudar a pergunta

- **Duas armas 9/82**: árvore anterior aplica 100% de offhand por W2PP
  (iguais 50%, diferentes 30% sem passiva). Nativo `FUN_0054eb20` usa 30%/20%,
  não possui esse bônus e trata Unique 47+45 como mão principal apenas.
  `FUN_0054fdb9` tem bônus HT bit 10 para Unique 42/43: especialização 2/5+10,
  ou +120 com bit 15. O grafo confirmou callers de 54eb20 em vários fluxos,
  inclusive ataque `0052ee94`, MAS não mostrou callers/incoming de 54fdb9.
  Portanto não afirmar que esse cálculo de score é executado pelo client.
  Não alterar todo o balanço de armas só por esse pseudocódigo. Falta resolver
  o consumidor vivo/contrato antes de substituir o patch anterior.
- **81 precisão**: `GetFunc.cpp::GetParryRate` lê `Rsv & 0x200`, não
  `LearnedSkill`. Comentário "Ligação espectral" não prova correspondência.
  Subtração 20 no dado 1..1000 não é bônus de 20 pontos percentuais.
- **Extração**: comparação `< chance`; **Alquimia**: `<= chance`.
  Não uniformizar os dois. Extração consome todo slot 1774, como W2PP;
  preserva UID/efeitos do alvo no sucesso; Go usa clamp em vez de wrap byte.
- **Athena**: o teto 320 é o contrato interno adotado pelo Go/W2PP; não afirmar
  que 320 é o teto do client nativo 7.48. Nenhum wire foi alterado neste lote.
- W2PP tem diferenças/bugs próprios (inclusive `if (!Type == 1)` na sequência
  de affects); usar fórmulas como referência, não copiar controle de fluxo.
- A animação montada human1/HOLY slot 24 e a inspeção visual real continuam
  pendentes na ficha visual existente. O lote inicial não compilou/instalou
  client; a revisão posterior abaixo compilou e instalou o candidato.

## Validação

Dentro de `wydgo748`:

```text
go test -count=1 ./internal/game ./internal/wire                PASS (entrada)
go test -count=1 ./internal/game -run 'TestPassiveMerchantSale|TestCityTax|TestShopOpenBuyAndSell' PASS
go test -count=1 ./internal/game -run 'Test(Extracao|CombineExtracao|CombineAlquimia|SkillCraft|PassiveMerchant)' PASS
go test -count=1 ./internal/game -run '^TestAthenaMasteryCapAndLifecycle$' FAIL antes da correção
go test -count=1 ./internal/game -run 'TestAthena|Test.*(Affect|Buff|Carbuncle)' PASS após correção
go test -count=1 ./...                                         PASS
go vet ./...                                                 PASS
```

Testes de persistência das correções usam stores de teste com falhas injetadas;
isso não certifica PostgreSQL real. Nenhum teste visual/in-game realizado.
Ghidra 12.1.2 falhou por versão de linguagem; 12.1.3 exportou corretamente o
grafo. Não repetir com 12.1.2.

## Revisão do diff sem commit — 2026-09-16

Escopo: deltas Go de buffs/passivas/crafting/venda/combate e deltas C++ de
tooltip da barra, redimensionamento/filtros do chat e renderização HP/MP.
Preservadas as alterações anteriores, sem commit/push e sem remoções.
Correções desta revisão classificadas como `MODERNIZACAO_COMPATIVEL`:

- `TMFieldScene.cpp`: preservada a seleção inversa dos controles alternativos
  de chat no layout moderno; o layout compat mantém seleções espelhadas.
  As duas inicializações são alternativas, não uma sobrescrita sequencial.
- `TMHuman.cpp`: mensagens de personagem visível também respeitam os filtros
  de whisper/grupo. Whisper comum agora possui formatação explícita, evitando
  linha vazia quando o handler da cena delega ao personagem.
- `combat.go` e `handlers.go`: MISS primário mantém as flags da ação e resolve
  o alvo secundário da Lança de Ferro. Sem nova rolagem da progressão de golpe
  duplo; mantém validações de classe, aprendizado, geometria e PvP existentes.
  `handlers_lifecycle_integration_test.go` ganhou quatro regressões: primário
  mob/jogador e secundário mob sobrevivente/morto, com inspeção do 0x39E/DMGX,
  MISS, flags de crítico/golpe duplo e progressão única. Antes do patch os
  quatro cenários falharam por HP secundário intacto; após patch/fixture completa
  todos passaram. A fixture registra visibilidade e store para testar publicação
  e persistência do lifecycle, não apenas mutação de HP.

Validação final:

```text
go test -count=1 ./internal/game -run 'TestIronSpear|TestHuntress|TestPhysical|TestPlayerPhysical' PASS
go test -count=1 ./...      PASS
go vet ./...               PASS
go build ./...             PASS
git diff --check           PASS (avisos LF/CRLF, sem erro de whitespace)
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 PASS
ArchitectureTests: 40721 checks PASS; static assertions PASS
```

Client compilado/instalado em `tmproject/client748/project.exe`, SHA256
`8C80A35A6A2B486C112F321D54201A1339C31025EC5716797270CCDE4ACAC146`.
Source/build: `STATICALLY VERIFIED`; regressões: `AUTOMATED TESTED`.
Não `CLIENT-TESTED`: tooltip, filtros, resize e barras HP/MP ainda exigem
interação/inspeção visual no jogo. Servidor validado em build/testes, não
reiniciado/instalado em serviço ativo. Divergência nativa de duas armas 9/82
continua aberta; não foi alterada sem evidência suficiente.

## Continuação Foema — 2026-09-16

Delta `MODERNIZACAO_COMPATIVEL`, interno do servidor, sem alterar wire/ABI:

- `affects.go`: membros mortos não ocupam vagas do buff coletivo; alvos
  explícitos mortos são recusados, preservadas as exceções 31/99 de
  ressurreição. A seleção validada é reaproveitada na aplicação.
- `skills.go`: seleção/validação do suporte ocorre antes de cobrar MP e
  registrar cooldown. Um alvo inválido não produz mais animação de autocast.
- `skills_test.go`: Velocidade/Arma Mágica com membro morto intermediário e
  limites por mastery 0/24/25/75/274/275/320, até 13 alvos.
- `affects_test.go`: fixture dos quatro buffs usando os valores do SkillData
  do runtime; não declara paridade nativa de fórmulas ainda sem evidência.
- `skills_integration_test.go`: 20 casos de rejeição (41/43/44/46 × alvo
  inexistente, outro grupo, distante, offline, morto), conferindo MP, cooldown,
  affects e ausência de publicação. Lifecycle de 41/43/44 verifica custo,
  cooldown, renovação, ausência de acúmulo, base preservada, expiração antiga
  versus renovada e remoção persistida. Velocidade exercita também
  `movementTilesPerSecond`, consumidor do limite autoritativo de movimento.

Os 20 casos inválidos e dois casos de grupo falharam antes do patch e passaram
depois. Fórmulas base de 43/44 confrontadas com `W2PP/Code/Basedef.cpp`, affects
11/9, sem portar o multiplicador de aprendizado nem afirmar paridade nativa.

Naquele lote, Controle de Mana permaneceu **não resolvido**: `affects.go` somava
`Level/10 + Value` em SaveMana (Value=100 no catálogo, sempre limitado a 99).
W2PP `GetFunc.cpp:2683` usa metade do dano com reserva de 300 MP;
`_MSG_Attack.cpp:2433` usa outra fórmula, reserva de 5% e condicionais
inconsistentes entre débito/absorção. Nenhuma delas foi copiada. A decisão
posterior do usuário e a implementação constam abaixo; não retomar essa
comparação como bloqueio da regra expressamente escolhida.

Validação deste lote:

```text
go test -count=1 ./internal/game -run 'TestPartyBuffDeadMember|TestFoemaBuffInvalidTarget' FAIL antes do patch
go test -count=1 ./internal/game -run 'TestPartyBuffDeadMember|TestFoemaBuffInvalidTarget|Test.*(Support|Revive|Resurrect|Summon|Sephira)' PASS
go test -count=1 ./internal/game -run 'TestFoemaStatBuff|TestPartyBuffDeadMember|TestFoemaBuffInvalidTarget' PASS
go test -count=1 ./... PASS
go vet ./... PASS
go build ./... PASS
```

Sem alterações C++/assets neste lote, sem instalação/reinício do servidor,
sem commit/push e sem remoções. `STATICALLY VERIFIED` e `AUTOMATED TESTED`,
não `CLIENT-TESTED`. Expiração persistida exercitada com store de teste, não
validação PostgreSQL real.

## Controle de Mana — decisão e implementação 2026-09-17

O usuário definiu absorção **90% no MP / 10% no HP**, até esgotar MP, quando
todo o dano passa ao HP. Delta `MODERNIZACAO_COMPATIVEL`, regra interna
explicitamente solicitada, sem novo wire/ABI e sem alegação de paridade W2PP.

- `affects.go`: `absorbManaControlDamageAt` aplica a divisão 1:1 por golpe,
  após mitigações existentes/montaria. Usa uint64 no produto; arredondamento
  e déficit de MP vão para HP, sem reserva nem desconto gratuito. Sem buff,
  expirado, morto ou MP zero não absorve. Não altera MaxHP/cura. Removida a
  contribuição indevida de affect 18 em SaveMana; custo normal continua válido.
- `handlers.go`: PvP físico primário e secundário da Lança de Ferro.
- `skill_pvp.go`: cada golpe das skills, inclusive esgotamento no multigolpe.
- `mob_combat.go`: dano compartilhado de mobs e magia de bosses.
- `skill_summons.go`: dano de invocação contra jogador.
- `affects.go`: veneno preserva seu limite não letal antes da divisão.
  Sacrifício de HP da skill 30, morte e operações administrativas não passam
  por essa absorção. Buff com MP zero pode voltar a absorver após recuperar MP.
- `mana_control_test.go` (novo): fórmula, extremos uint32, arredondamento,
  slots duplicados, expiração, esgotamento, recuperação, custo de cast e
  preservação do desconto legítimo; oito caminhos reais de combate com MP
  suficiente, insuficiente e zero. Decodifica SetHpMp/28B no alvo e observador,
  comparando HP/MP autoritativos. Montaria absorve antes; testes de morte com
  falta de MP e também pelos 10% de HP mesmo com MP sobrando.
- `DOCS/EMULATOR.md`: contrato da regra atualizado, substituindo SaveMana.

Regressão de custo inicialmente falhou: SaveMana=99 em vez de 25. Após remover
o bônus indevido, todos os testes abaixo passaram:

```text
go test -count=1 ./internal/game -run '^TestManaControl' PASS
go test -count=1 ./... PASS
go vet ./... PASS
go build ./... PASS
```

`STATICALLY VERIFIED` e `AUTOMATED TESTED`, não `CLIENT-TESTED`.
Sem instalação/reinício do servidor, sem alterações C++/assets neste lote,
sem commit/push e sem remoções. Demais alterações anteriores preservadas.

## Próximo passo da auditoria individual (se retomada)

Fechar a interação **43 → 44**; não repetir os testes/inventário de 41 ou a
regra escolhida de 46 sem mudança. Validar 46 no jogo após instalar/reiniciar
o candidato autorizado; os testes acima não comprovam apresentação visual.
Continuam abertas também a animação montada HOLY, duas armas 9/82 e skill 91.
Não declarar todos os buffs/passivas concluídos a partir deste lote.

## Ícones de alvo e grupo — 2026-09-17

Pedido: affects/buffs/debuffs sob a barra de HP do alvo apontado e ao lado do
nome na lista de grupo. Correção explícita do usuário: **somente enquanto o
membro estiver no campo de visão**. Não manter snapshots de membros distantes.

Delta `MODERNIZACAO_COMPATIVEL`, preservando os contratos existentes de
CreateMob/UpdateScore e o lifecycle da ficha `ui/party-panel-layout-lifecycle`.
Não é alegação de paridade visual nativa. A tentativa inicial de cache remoto
e fanout Party foi retirada integralmente; nenhum delta Go deste lote permanece.

- `TMFieldScene.cpp/.h`: cria painéis compat ausentes, projeta alvo e membros
  encontrados por ID na visão local, antes do retorno antecipado da rotina do
  próprio personagem. Âncora do alvo é sua própria barra superior, não a do dono.
  Painéis pertencem ao container e não interceptam clicks.
- `ObservedAffectProjection.h` (novo): 32 slots públicos, tipo no byte alto,
  duração no baixo; compacta, quebra linhas e oculta inválidos/inativos.
  `InViewAffects` rejeita entidade ausente, morta, removida e em DelayDelete;
  `m_nWillDie` fecha o intervalo entre RemoveMob e exclusão física da entidade.
- `ObservedAffectProjectionTests.cpp` (novo), `ArchitectureTests.vcxproj`,
  `PacketViewTests.cpp` e `TMProject748.vcxproj`: testes e inclusão no build.
- Ficha existente de Party atualizada; nenhuma remoção de arquivo. Demais
  alterações prévias do worktree preservadas, sem commit/push.

Validação final:

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\tmproject\Build-Client.ps1 -NoDeploy PASS
ArchitectureTests: 40775 checks PASS; static assertions PASS
python .agents/skills/wyd-client748-research/scripts/validate_research.py PASS
git diff --check PASS
```

Release gerado em `tmproject/build/TMProject748/Release/WYD.exe`, SHA-256
`718690934994DE6A8BE1E87AC8635CA556EC1E38427E772D5095840E97135FC6`.
Warnings anteriores de signedness permanecem fora do trecho alterado.
`STATICALLY VERIFIED` e `AUTOMATED TESTED`, sem sobrescrever
`client748/project.exe`. Validação in-game e instalação pendentes;
não declarar todos os buffs de Foema corretos. Permanece aberta a interação
43 → 44 descrita acima. Nenhum novo teste Go necessário neste lote sem delta Go.
