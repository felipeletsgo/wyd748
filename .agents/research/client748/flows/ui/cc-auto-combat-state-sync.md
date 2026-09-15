---
id: cc-auto-combat-state-sync
title: Sincronização dos controles e do estado do C.C
subsystem: ui-gameplay
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-15
---

# Sincronização dos controles e do estado do C.C

## Pergunta

Como manter os controles antigos e modernos do C.C coerentes com o mesmo
estado de auto-combate no client 7.48 recompilável, sem transformar UI
posterior em uma alegação de paridade nativa?

## Fronteira de evidência

- Referência funcional oficial: `DOCS/GUIA/GUIAS_JOGABILIDADE.md`, seção 5,
  extraída de `https://www.wyd2.co.kr/guide/firstguide12.asp`.
- Source atual: `TMFieldScene::GameAuto`, `OnControlEvent`, `NewCCMode` e os
  initializers da Field em `TMFieldScene.cpp`.
- Recursos: IDs modernos `B_CCMODE_*`, controles antigos
  `B_CCATTACK`/`B_CCPOTION`/`B_CCMOVE`, seletores nativos `318`/`319`, barra
  nativa `575` e grades de skills `573`/`586`; o layout compatível é carregado
  de `FieldScene2`.
- Servidor: nenhuma regra de combate, packet ou estado autoritativo foi
  alterado por este lote.
- Executável nativo: o hash do frontmatter identifica a baseline. Esta ficha
  não afirma que a topologia moderna de controles existia no binário 7.48.

## Fluxo nativo 7.48

`FUN_00435B13` materializa o fluxo de campo compatível e vincula os controles
`318` (combate físico) e `319` (combate mágico). O dispatcher de teclado
`FUN_00453C59` chama `FUN_004539BC`, que altera o estado global
`DAT_005D03F4` com a seguinte semântica confirmada:

### Callers

- `FUN_00453C59` é o único caller direto de `FUN_004539BC` registrado no
  inventário Ghidra. Ele é o dispatcher de caracteres da Field e encaminha a
  tecla somente dentro do fluxo vivo da cena.
- `FUN_00435B13` não chama a transição: ele é o initializer que resolve os
  controles `318/319` usados para apresentar o estado.

### Função principal

`FUN_004539BC` interpreta `A/a` e `D/d` e escreve diretamente em
`DAT_005D03F4`. Outras teclas saem sem alterar o modo.

| Entrada | Estado anterior | Estado seguinte |
| --- | ---: | ---: |
| `A` | `1` | `0` |
| `A` | qualquer outro | `1` |
| `D` | `2` | `0` |
| `D` | qualquer outro | `2` |

### Callees

O inventário Ghidra registra zero callees diretos para `FUN_004539BC`. A
transição é local: compara a tecla e o estado anterior, grava
`DAT_005D03F4` e retorna ao dispatcher; não envia packet, não aloca objeto e
não inicia callback.

Os dois modos são mutuamente exclusivos. O modo moderno `3` não seleciona
nenhum dos dois indicadores nativos. O dispatcher nativo de clique não possui
cases para `318`/`319`; portanto, oferecer clique equivalente no client
recompilável é `MODERNIZACAO_COMPATIVEL`, não claim de paridade.

## Mapeamento atual

### Source recompilável

`g_GameAuto` possui quatro estados documentados pelo guia e já consumidos por
`TMFieldScene::GameAuto()`:

| Estado | Modo | Efeito local esperado |
| ---: | --- | --- |
| `0` | desligado | não executa auto-combate e oculta os mini-controles |
| `1` | físico | procura e ataca alvos pelo caminho físico |
| `2` | mágico | usa as skills configuradas no atalho |
| `3` | Not attack | mantém automações auxiliares sem iniciar ataques |

Os handlers antigos (`B_CCATTACK`, `B_CCPOTION`, `B_CCMOVE`) e modernos
(`B_CCMODE_DLG_MODE`, `B_CCMODE_DLG_HP`, `B_CCMODE_DLG_MOUNT` e
`P_CCMODE_DLG_PONT`) alteravam os mesmos globais, mas repetiam atualização de
textura, tooltip, visibilidade, thresholds e posição. As cópias divergiam e
algumas desreferenciavam `m_pSGameAutoBtn` sem que ele tivesse sido vinculado.

`NewCCMode(bool resetCombat, bool capturePosition)` agora é o único
sincronizador de apresentação. Ele normaliza modos e percentuais, atualiza os
controles que realmente existirem, protege controle e tooltip nulos e limpa
target/delays quando o modo de combate muda. A posição inicial só é capturada
ao entrar no estado de posição fixa; outros refreshes não deslocam o ponto do
farm.

No caminho `FieldScene2`, `ToggleNativeCCMode()` reproduz a transição de
`FUN_004539BC` tanto para `A`/`D` quanto para os cliques modernos em `318`/`319`.
`NewCCMode()` reflete `g_GameAuto == 1/2` nos dois botões nativos e não consulta
os textos ou texturas modernos ausentes nesse recurso.

Os tooltips de posição seguem a ordem dos handlers antigos, escolhida como
canônica para o layout compatível:

| `m_AutoPostionUse` | Textura | Tooltip |
| ---: | ---: | --- |
| `0` | `463` | `g_UIString[233]` |
| `1` | `464` | `g_UIString[234]` |
| `2` | `465` | `g_UIString[232]` |

Essa escolha ainda requer inspeção visual no client. O handler moderno
anterior possuía uma ordem divergente e não é usado como evidência nativa.

## Correção compatível de skills e auto-ataque mágico

O runtime ativo carrega `config.txt` com `CLASSIC=1`, que projeta `UIVer=1`.
Por isso, o construtor genérico `SGridControlItem` escolhia o atlas antigo
(conjunto `1`) para sprites de skills aprendidas, deixando os ícones sem a
coloração esperada. A correção mantém o comportamento clássico para
inventário/equipamento e usa o atlas colorido `199` (`UI\NewAmul.wyt`) para
os itens aprendidos do painel compatível e os sprites das grades `GRID_SKILLB`.
`SGridControlItem::SetGridControl` converte o atlas `1` em `199` ao inserir um
atalho, cobrindo reconstrução pelo servidor, drag, confirmação e rollback.
Outros atlas, inclusive a seleção `200`, permanecem intactos. O atlas selecionado da barra
continua sendo controlado pelo fluxo existente, incluindo o conjunto `200`
quando o slot está selecionado.

O fluxo nativo também foi fechado para a barra de seleção de auto-skill:

- `T/t` alterna o auto-ataque por `FUN_0044EECF`, usando o controle `312` e o
  estado da Field em `scene + 0x26EB4`.
- `-/_` percorre a quantidade de skills de `1` a `10` por
  `FUN_00452210`/`FUN_00447469`; a source compatível agora usa a mesma faixa.
- O initializer vincula o controle nativo `575`, deixa-o oculto até o
  auto-ataque ser ligado e redimensiona sua largura ancorada à direita da grade,
  preservando o `Y` e a altura nativa de `4` unidades lógicas.

Esse é um `MODERNIZACAO_COMPATIVEL`: os IDs, teclas e geometria seguem a
fronteira nativa 7.48, mas o override do atlas existe apenas para tornar o
painel compatível funcional sob o modo clássico atual. Não há mudança de wire,
ABI, servidor ou autoridade de combate.

### WYD-Go

O servidor não recebe um novo modo, opcode ou layout. Cada intenção produzida
pelo C.C continua entrando nos handlers existentes e sendo revalidada.

## Estado e lifecycle

- Os globais e campos de `TMFieldScene` continuam sendo o estado local; os
  botões apenas o apresentam e encaminham intenção.
- `m_pNativeCCPhysicalBtn` e `m_pNativeCCMagicBtn` começam nulos e são
  vinculados aos IDs `318`/`319` somente no initializer compatível.
  `m_pSGameAutoBtn`, `m_pCCPotionBtn`, `m_pCCFeedBtn`, `m_pccmode`,
  `m_pCCModeHpSte` e `m_pCCModeMountSte` permanecem opcionais.
- Inicialização parcial não derruba a cena: cada atualização tolera ausência
  de painel, botão, texto alternativo ou humano local. `SetAutoSkillNum()`
  também testa individualmente os dez filhos modernos `65775..65784`, e
  `SetAutoTarget()` não desreferencia o painel `65648` quando ele não existe.
- O lote não aloca controles e não cria ownership novo. O teardown dos botões
  continua pertencendo ao container da Field; os membros são descartados junto
  com a cena e não há callback ou buffer adicional em logout, relogin ou
  shutdown. A ação é síncrona e não possui falha parcial a desfazer.
- `B_CCMODE_SYSTEM` não abre um painel ausente. Isso preserva o lifecycle da
  cena em vez de materializar silenciosamente um widget moderno incompleto.

## Wire, ABI e recursos

Não há alteração de opcode, packet, packing, struct, asset ou ABI. As ações
geradas pelo auto-combate continuam passando pelos handlers existentes e são
revalidadas pelo WYD-Go. O client não se torna fonte de verdade de combate,
inventário, skill, alvo ou movimento.

## Matriz de delta

| Claim | Baseline 7.48 | Source antes | Estrutura posterior/manual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| modos `0..2` | `FUN_004539BC` alterna `A: 0↔1` e `D: 0↔2` em `DAT_005D03F4` | `GameAuto()` já consome os modos | controles modernos editam o mesmo global | regras inalteradas | reproduzir no helper compartilhado |
| modo `3` | não seleciona `318` nem `319`; guia o descreve como Not attack | `GameAuto()` já o consome | painel moderno o seleciona | regras inalteradas | preservar como modernização |
| apresentação | `FUN_00435B13` vincula `318`/`319`; click nativo não os despacha | source não os vinculava | painel moderno é opcional | N/A | bind nativo, teclado equivalente e clique moderno |
| thresholds | incrementos de 10% documentados | valores podiam divergir do texto | steppers modernos reutilizados quando existem | N/A | normalizar e refletir |
| posição | caça pode manter posição | captura repetida em branches distintos | handler moderno divergia nos tooltips | movimento continua validado | capturar somente na transição |
| painel moderno | não comprovado como recurso nativo | ponteiros não vinculados | arquitetura útil se materializada | N/A | não alegar paridade; extensão futura explícita |

## Decisões

- Classificar este lote como `MODERNIZACAO_COMPATIVEL`: consolida estado local
  e protege recursos opcionais sem mudar contrato externo.
- Preservar tanto os IDs antigos quanto os modernos quando o recurso os
  materializar, mas manter um único caminho ativo de sincronização.
- Não criar dinamicamente o painel moderno neste lote. Se `FieldScene2` não o
  fornecer, sua criação será uma extensão visual deliberada e documentada.
- Proteger o painel `65648` e cada filho `65775..65784` como opcionais; a
  ausência desses recursos não pode impedir a alternância do estado local.
- Promover a transição de teclado para `TRACED`: entrada, caller, mutação,
  controles, ownership e teardown estão resolvidos. O clique permanece
  explicitamente uma modernização e a validação visual continua pendente.

## Lacunas

- Confirmar em runtime os quatro modos, as quatro opções de poção, as três
  opções de posição e os thresholds de HP/mount.
- Confirmar a ordem visual dos tooltips `232..234`.
- Confirmar em runtime a materialização e o estado visual dos IDs `318`/`319`.
- Exercitar logout/relogin e reconstrução da Field com C.C ligado.

## Validação

- Pesquisa: guia oficial, fluxo source vivo e duplicações de handlers
  comparados; nenhuma alegação de topologia nativa foi promovida.
- Automação: `validate_research.py` e `git diff --check` aprovados depois das
  guardas adicionais.
- Implementação: `IMPLEMENTED / STATICALLY VERIFIED`; a source corrige o atlas
  dos ícones aprendidos, vincula/oculta o controle `575`, usa `T/t` para
  alternar e redimensiona a barra em `1..10` slots.
- Build: `Build-Client.ps1 -Configuration Release -NoDeploy` aprovado em
  Release/Win32 com zero erros; os testes de arquitetura registraram `35278`
  checks PASS. O build emitiu 13 warnings C4018 já existentes em
  `TMFieldScene.cpp`. Artefato daquela etapa: `tmproject/build/TMProject748/Release/WYD.exe`,
  SHA-256 `CD8DB8950E57073216E9F171E03720E12EC88DEC62400BF2F3579838BBBE39B1`.
- Instalação local: `Build-Client.ps1 -Configuration Release` concluído sobre
  `HEAD 77e2628e`, com 35278 checks PASS e build incremental aprovado.
  `tmproject/client748/project.exe` substituído pelo candidato Release;
  o script confirmou igualdade SHA-256 entre artefato e runtime:
  `09F12F0CF8EFC8957EBB721F5D7C9F42DBA696767E811DFA6FF48B0E4BECD4DD`.
  Próximo gate: abrir a janela de skills, conferir cores, alternar `T`,
  percorrer `-` até 10 e voltar a 1, testar a rotação em combate e relogin.
- Client real: não executado; `CLIENT_TESTED` não é alegado.

### Crash ao apertar T / 2026-09-13

- `client-crash-20260912-234954.dmp`, inspecionado com CDB x86 e o PDB
  correspondente ao candidato anterior: `C0000005`, leitura de `0x1E8`,
  `SGridControl::GetAtItem+8 -> TMFieldScene::UseQuickSloat+0x62 -> OnCharEvent`.
  O argumento era `t`; o grid moderno de quick-slot estava nulo. O dump foi
  preservado no runtime, sem copiá-lo para a source ou substituir a evidência.
- `MODERNIZACAO_COMPATIVEL`: `UseQuickSloat` não acessa os cinco widgets modernos
  na Field nativa e valida grid/item no modo moderno. O dispatch nativo já
  comprovado continua até `OnKeyAutoTarget`, sem consumir o T como quick-slot.
  As guardas anteriores de `SetAutoTarget` continuam necessárias, mas não
  alcançavam esse crash anterior no call stack.
- `AUTOMATED TESTED`: dez combinações Q/W/E/R/T maiúsculas/minúsculas na Field
  nativa, mapeamento moderno e tecla desconhecida. Release construído e instalado
  junto ao lote de volatiles, 35439 checks aprovados, SHA-256
  `71CD0587610CFF8EEEBA2D13CC03A15099C70BBE5B17C0ED545EB12897DAE3EA`.
- `CLIENT-TESTED`: pendente; repetir T, `-`, combate e relogin neste candidato.

### Ícones dos atalhos e alinhamento da barra / 2026-09-13

- A imagem enviada pelo usuário confirma que T já exibe a barra, mas ela passa
  da borda dos slots; os atalhos ainda usam os sprites antigos. A correção
  anterior do atlas cobria apenas a janela de skills.
- Reutilizada a evidência nativa de `FUN_00447469` (posição e largura da barra),
  `FUN_004470B9` (reconstrução dos atalhos) e a ficha
  `skill-belt-page-selection.md` (inserção/ownership nas duas páginas).
- O recurso materializado em `client-debug.log` mostra `575`, `573` e `586`
  sob o mesmo pai `5745`. Na escala 1,28, a barra serializada tem X=0 e W=448;
  a grade tem X=25,6 e W=307,2. Usar a borda 448 preservava um excesso de
  115,2 pixels. Agora `SetAutoSkillNum` calcula a borda X+W da própria grade;
  mantém Y, altura e crescimento para a esquerda, sem acumular deslocamentos.
- `MODERNIZACAO_COMPATIVEL`: atlas coerente em todas as inserções `GRID_SKILLB`
  e geometria derivada dos controles materializados. Os conjuntos `199/200`
  estão em `UI/UITextureSetList.txt`, usando texturas `131/132`. Não houve
  alteração de asset, packet, autoridade, ownership ou teardown.
- `STATICALLY VERIFIED`: inserções `AddItem`, `AddSkillItem` e `SetItem` passam
  por `SetGridControl`; a seleção continua usando `200`. Cálculo de geometria
  conferido para 1..10 slots em escalas 1, 1,28, 1,6 e 2,4: borda constante,
  largura limitada à grade e início coincidente com a grade em dez slots.
- `AUTOMATED TESTED`: `Build-Client.ps1 -Configuration Release` aprovado,
  35439 checks de arquitetura PASS; warnings C4018 preexistentes. Executável
  instalado em `tmproject/client748/project.exe`, SHA-256
  `9502CEC318B617D3A0F9F7DECCA353108B58254A4EECA81DF79B91099F4F3C67`.
- `CLIENT_TESTED` pendente para este candidato: conferir cores ao entrar,
  arrastar skills, trocar página e seleção; T e `-` em 1..10 slots; relogin.

### Execução do C.C no ciclo compatível / 2026-09-14

- Entrada em `HEAD 0d1f1fb0f1c1b71ea17d88c973a6976ec86cf3c6`: a árvore já
  continha o painel exclusivo `InitializeCompatCCControls`, o atlas `CC759`,
  `CCModePolicy` e alterações de combate/UI do usuário. Esses trabalhos foram
  preservados; o painel não foi criado por esta correção.
- Causa confirmada na source: o ramo `m_bCompatFieldScene` de `FrameMove`
  retornava depois de `UpdateTeleportPrompt`, antes da única chamada a
  `GameAuto` no ramo moderno. Os botões atualizavam `g_GameAuto`, mas o
  scheduler de consumíveis, buffs e ataques não era executado no 7.48.
- `MODERNIZACAO_COMPATIVEL`: conectada uma chamada por frame compatível após
  `TimeDelay`, `AirMove_Main`, `Affect_Main` e o gate de `UpdateTeleportPrompt`.
  Não se habilitou o restante do tick de HUD moderno. Nenhum packet, regra do
  servidor ou asset foi alterado; esta integração do controlador existente
  não constitui prova de paridade nativa integral do C.C portado do 7.59.
- `GameAuto` agora valida cena ativa, personagem, terreno, container e managers
  antes de usá-los durante reconstrução/teardown. Os dois caminhos de buff
  automático restauram a seleção anterior após `SkillUse`, inclusive quando
  ela rejeita o lançamento, para não substituir a skill ofensiva do modo
  mágico. Os acessos ao painel de portal em `SkillUse`/`AutoSkillUse` toleram
  controle ausente; `AutoSkillUse` rejeita alvo nulo antes de consultá-lo.
- `STATICALLY VERIFIED`: dez verificações focadas da source passaram: chamada
  única no ramo compatível, ordem do countdown, bloqueio por teleporte, modo
  desligado, guardas de personagem/container, restauração nos dois caminhos
  de buff, saída do suporte antes dos ataques, morte e cidade. Essas inspeções
  não simulam combate nem substituem execução real.
- `AUTOMATED TESTED`: `pwsh -NoProfile -ExecutionPolicy Bypass -File
  .\tmproject\Build-Client.ps1 -Configuration Release` aprovado; 35656 checks
  de arquitetura e assertions estáticos PASS, build incremental Release/x86
  concluído, 13 warnings C4018 em trechos não alterados por este ciclo.
  Instalado `tmproject/client748/project.exe`, com SHA-256 igual ao artefato:
  `C16BE8E7930A7B5A3351498C2153C312324271506BB8787BF55123B98BD5AA32`.
- `CLIENT_TESTED` pendente: iniciar o executável novo e testar desligado,
  físico, mágico com buff na barra, suporte, limiares de poção/ração, modos
  de movimento e parada/reentrada após morte, teleporte e relogin. Não houve
  teste in-game nesta sessão. Próximo passo é esse gate, não nova pesquisa
  ampla do painel. Nenhum arquivo foi removido.

### Feedback de combate: framing, cooldown e hit azul / 2026-09-14

- O usuário executou o C.C e trouxe a rejeição `opcode=0x367`, tamanho 96,
  esperado 52, repetida até o disconnect (23:09:38..23:09:52), além da imagem
  `C:/Users/felipe/Desktop/New Bitmap image.bmp`. Isso invalida qualquer
  suposição de funcionamento ponta a ponta do candidato anterior; o
  scheduler passou a rodar, mas expôs defeitos nos caminhos portados.
- Causa do DC: `AutoSkillUse` inicializava `MSG_Attack` com decimal 871
  (`0x367`, ActionStop). Skills com mais de dois alvos conservavam esse
  opcode ao enviar 96 bytes. Agora usa `MSG_Attack_Multi_Opcode` (`0x36C`),
  conservando os ramos compactos One/Two. `MAutoAttack` físico passa a enviar
  `sizeof(MSG_AttackOne)` (48) com One, mantendo Two em 52. A forma estendida
  One/96 já era aceita pelo servidor e não é apresentada como causa do DC.
- Classificação desse delta wire: `PARIDADE_NATIVA`, reutilizando os contratos
  registrados em `DOCS/IMPLEMENTATION_STATUS.md`, `DOCS/protocol/packet-fixtures.md`
  e os asserts de `Basedef.h`: One `0x39D/48`, Two `0x39E/52`, Multi
  `0x36C/96`, ActionStop `0x367/52`. Nenhuma regra de segurança foi relaxada.
- Cooldown: `MODERNIZACAO_COMPATIVEL`. O servidor usa `SkillData.Delay` em
  segundos, sem os descontos herdados de sanc/jewel do TMProject. Manual,
  C.C, suporte pela grade e party agora consultam `IsSkillCoolingDown`;
  `SkillCooldownPolicy` centraliza duração e progresso. O catálogo deixa de
  ser mutado globalmente pelas exceções de equipamento. No modo compatível
  prevalece o Delay do catálogo, com piso local conservador de um segundo.
  O servidor continua autoritativo e pode recusar intenção por cooldown,
  mana, alvo ou outras regras; não foi criado ACK/protocolo novo.
- Evidência da projeção de cooldown: export nativo estudado
  `C:/Users/felipe/Tools/GhidraAnalysis/20260821/decompiled/004776c3_FUN_004776c3.c`,
  linhas 1765..1814, percorre vinte atalhos e projeta o tempo em segundos
  no timer do item. `UpdateSkillCooldownUI` conecta essa projeção ao tick
  compatível, cobre ambas as páginas, limpa o overlay ao terminar, mostra
  o primeiro frame e tolera slots/controles ausentes sem mudar hover.
  Efeitos de personagens remotos não escrevem mais no timer do jogador local;
  índices negativos também não acessam o array de cooldown.
- Hit azul: `MODERNIZACAO_COMPATIVEL`, mantendo os tipos portados 7/8 e
  resolvendo-os para o atlas nativo `Blue_Number` (138, 12 glifos, textura
  115, Y=30). `TMFont3` usava 143/144; 143 no recurso atual é
  `pgt_GDrop_Btn`, com cinco botões, explicando os retângulos da imagem.
  Export nativo estudado `00422930_FUN_00422930.c` confirma os conjuntos
  numéricos 137..141 e o mecanismo de glifos; não há claim de tipos 7/8
  nativos. `UI/UITextureSetList.txt` confirma o atlas azul. `DamageNumberPolicy`
  preserva as demais cores e a escala de crítico. Nenhum asset foi editado;
  as mudanças anteriores do usuário nos efeitos de partículas foram preservadas.
- `STATICALLY VERIFIED`: dez verificações focadas de source/recurso passaram:
  opcode multi, tamanho compacto físico, gate compartilhado manual/C.C,
  atualização nas duas branches de FrameMove, catálogo sem mutações,
  índice validado antes do array, duas páginas sem alteração de hover,
  renderer consumindo a policy testada, identidade do atlas 138 e colisão 143.
- `AUTOMATED TESTED`: `Build-Client.ps1 -Configuration Release` aprovado,
  40683 checks e static assertions PASS. Build integrado incremental e deploy
  verificados, com warnings preexistentes C4018 e C4305/C4309. Candidato final
  `tmproject/client748/project.exe`, SHA-256
  `BEEBDC4A6BD1D37934080D9C05D476845D7F7BDDC1D36390EFDF3B09D349580D`.
  Testes C++ cobrem limites, repetição, wrap do relógio, barra/bloqueio
  concordantes em todo o intervalo, e mapeamento de todos os tipos de dano.
- Go: `go test -count=1 ./internal/game -run
  'Test(CCAttackFramingPreservesConfirmedLayouts|ExactInboundPacketSizeCoversEveryConfirmed748Opcode|ValidateInboundCommandRejectsAttackWithAppendedPayload|SkillDelayUsesSecondsFromCatalog|ActionStopAcceptsNative748FiveTileFinalPlan|ActionStopRejectsRouteLongerThanNativeLimit)$'`
  PASS. O teste novo cobre o erro exato `0x367/96`, ataques válidos, Stop/52
  e a compatibilidade preexistente One/96. A primeira versão do teste assumiu
  incorretamente que One/96 seria rejeitado; a leitura de `security.go`
  confirmou a exceção registrada e a expectativa foi corrigida, sem mudar
  o validador. `git diff --check` aprovado.
- Arquivos deste ciclo: `TMFieldScene.cpp/.h`, `TMHuman.cpp`, `SGrid.cpp`,
  `TMFont3.cpp`, novos `SkillCooldownPolicy.h`/`DamageNumberPolicy.h`, testes
  `CCModePolicyTests.cpp`/`security_test.go`, este registro e o executável
  instalado. Nenhum arquivo removido. Alterações alheias preservadas.
- `CLIENT_TESTED` pendente: reabrir o candidato, usar magia multi-alvo no C.C,
  tentar cast manual durante o cooldown, conferir overlay nas duas páginas
  e números azuis (normal/crítico), depois morte/teleporte/relogin. Não houve
  execução in-game do candidato final nesta sessão; build e testes isolados
  não confirmam o fluxo visual, latência real ou combate ponta a ponta.

### Crash do efeito DoubleSwing / 2026-09-15

- Modo: `MODERNIZACAO_COMPATIVEL`, proteção interna de recurso e lifecycle;
  nenhum opcode, asset, fórmula de combate ou contrato nativo alterado.
- Evidência: `client-crash-20260914-234447.dmp`, aberto com CDB x86 e o PDB
  correspondente, mostra `c0000005` em
  `TMSkillDoubleSwing::FrameMove+0x74`, RVA `0x13BE64` (EIP `0x0016BE64`).
  A instrução `mov eax,[ecx]`, com ECX zero, desreferencia `m_pVB` antes de
  `GetDesc`. O mesh existe, mas seu vertex buffer está nulo. O dump não contém
  o heap necessário para identificar o nível/índice do efeito ou provar a
  origem da falha de carregamento. Isso não é o DC por `0x367/96`.
- `MeshManager::LoadCommonMesh` pode manter um objeto mesmo quando `LoadMsa`
  falha. A correção fica no consumidor DoubleSwing, preservando a semântica
  global do cache e seus outros consumidores.
- `EffectVertexColor.h` valida buffer, stride, FVF, tamanho, `GetDesc`, `Lock`
  e ponteiro retornado; só escreve `diffuse`, equilibra locks bem-sucedidos
  e propaga falha de `Unlock`. O efeito pula a atualização gráfica insegura.
  Os arquivos efetivamente usados por `Mesh/MeshList.txt`, índices 702 e 12,
  são `Effect/FireBall.msa` e `Effect/sphere2.msa`: ambos presentes, com
  FVF 322 e stride 24, compatíveis com o guard de `RDLVERTEX`.
- O relógio e a expiração agora precedem as saídas por recurso/visibilidade;
  efeito inválido ou fora da tela não permanece indefinidamente. A remoção
  diferida retorna imediatamente, inclusive na meia-vida do nível 4.
  Luzes auxiliares opcionais são verificadas antes de uso.
- `STATICALLY VERIFIED`: fluxo de expiração e remoção diferida inspecionados;
  `git diff --check` aprovado. Sem alegação nova de paridade nativa.
- `AUTOMATED TESTED`: `Build-Client.ps1 -Configuration Release` aprovado,
  40721 checks e static assertions PASS. A nova suíte injeta buffer nulo,
  descritor/layout inválido, falhas de GetDesc/Lock/Unlock e Lock sem dados;
  verifica ausência de escrita nas rejeições, balanceamento de lock e
  preservação de geometria/UV no sucesso. Build integrado incremental e
  instalação em `tmproject/client748/project.exe` verificados, SHA-256
  `38CC2632AC847E9419A5346A41A486FFD0C1D9D66572222A4AFFF42FF19F1102`.
- Binário/PDB anteriores preservados em
  `tmproject/build/crash-20260914-234447/` (ignorado), para reabrir o dump.
  Arquivos deste ciclo: `TMSkillDoubleSwing.cpp`, novo `EffectVertexColor.h`,
  novo `EffectVertexColorTests.cpp`, runner `PacketViewTests.cpp`, projeto
  `ArchitectureTests.vcxproj`, este registro e executável local. Nenhuma
  remoção; alterações anteriores de C.C./cooldown/hit azul preservadas.
- `CLIENT_TESTED` pendente: reabrir o executável instalado e repetir combate
  com as skills que antecederam o crash, incluindo fora da tela e fim do
  efeito. Não houve reprodução in-game após este build; a causa específica
  do carregamento falhar continua indeterminada pelo dump disponível.

### Botões Livre/Fixo invertidos / 2026-09-15

- `MODERNIZACAO_COMPATIVEL`: conforme feedback do usuário, invertidos apenas
  os sprites 556/558 em `UpdateCCModeUI` (`positionTextures` agora
  `{556, 557, 558}`). Textos, estado 0=livre/1=cíclico/2=fixo e lógica de
  combate preservados. A skill restringiu o delta ao binding do painel.
- `STATICALLY VERIFIED`: estado 2 continua impedindo perseguição no
  `GameAuto`; cíclico mantém sprite 557. Sem mudança de wire ou assets.
- `AUTOMATED TESTED`: `Build-Client.ps1 -Configuration Release` compilou
  o client e executou 40721 checks PASS. Deploy falhou porque
  `tmproject/client748/project.exe` está em uso; o artefato atualizado está
  em `tmproject/build/TMProject748/Release/WYD.exe`. Não encerrar o jogo
  automaticamente. Fechar o client antes de instalar esse artefato.
- `CLIENT_TESTED` pendente: instalar e conferir os três estados do botão.
  Alterados neste delta somente `TMFieldScene.cpp` e este registro;
  nenhum arquivo removido.
- Instalação concluída após o usuário fechar o client e solicitar o deploy:
  `Copy-Item` do artefato já compilado para `tmproject/client748/project.exe`,
  com igualdade SHA-256 verificada por `Get-FileHash`. Nenhuma recompilação
  necessária. Resta somente conferir os três estados no jogo.

### Painel compacto / 2026-09-15

- `MODERNIZACAO_COMPATIVEL`: a pedido do usuário, o painel exclusivo passou
  de 240x92 para 180x64 unidades lógicas (47,8% menos área). Quatro colunas
  de 44 unidades, ícones 26x26, título e fechamento reposicionados; fonte
  e botão externo de abertura preservados. O rótulo desligado usa `Desl.`
  para caber, mantendo `Desligado` no tooltip.
- Skills de assets/research restringiram a edição a `InitializeCompatCCControls`
  e ao rótulo compacto em `UpdateCCModeUI`. Reutilizada a evidência acima:
  mesmos IDs, atlas, handlers, ownership e lifecycle; sem mudança de wire,
  estado de combate, cooldown, movimento ou servidor. Não houve edição de
  bitmap/atlas nem alegação de layout nativo.
- `STATICALLY VERIFIED`: dez controles dentro do painel, sem interseção dos
  retângulos, nas escalas 1, 1,28, 1,6 e 2,4. A geometria usa a escala já
  aplicada por `SControl`, sem escalonar novamente o input.
- `AUTOMATED TESTED`: `Build-Client.ps1 -Configuration Release -NoDeploy`
  aprovado, 40721 checks PASS e build integrado incremental sem erros;
  warnings C4018 preexistentes. Instalado por `Copy-Item` com igualdade
  SHA-256 conferida: `7385EB3BCB308DDF4412D1721454E1A7A1109822A19F57977D662B85D96508EB`.
  Executável anterior preservado em `tmproject/build/cc-layout-backup-20260915/`.
- Arquivos alterados neste delta: `TMFieldScene.cpp`, este registro e
  `tmproject/client748/project.exe`. Nenhum arquivo removido.
- `CLIENT_TESTED` pendente: abrir C.C., conferir legibilidade, quatro botões,
  tooltips, percentuais e fechamento no client atualizado. Nenhum processo
  do jogo foi iniciado ou encerrado por esta instalação.

### Abertura ancorada ao ícone / 2026-09-15

- `MODERNIZACAO_COMPATIVEL`: removida a posição absoluta antiga do painel.
  A cada abertura, centralizar sobre o botão C.C. e encostar a borda inferior
  na borda superior do botão (gap zero). Ambos são filhos do container raiz;
  usar posições/dimensões já escaladas, sem reaplicar a escala em `SetPos`.
- A skill de assets restringiu este delta à posição: tamanho compacto,
  controles, atlas, ações e contratos anteriores preservados.
- `STATICALLY VERIFIED`: fórmula de centralização e gap zero conferida em
  12 combinações de escala/posição; `git diff --check` aprovado.
- `AUTOMATED TESTED`: `Build-Client.ps1 -Configuration Release -NoDeploy`,
  40721 checks PASS e build incremental aprovado, com warnings C4018
  preexistentes. Instalação via `Copy-Item` e igualdade SHA-256 verificada:
  `78DE3E541E97C7CAE0AD2B089A298C2E4B9E519B4DB088A9DFE7A62EFF98257B`.
  Backup anterior preservado em `tmproject/build/cc-layout-backup-20260915/`.
- Alterados neste delta: `TMFieldScene.cpp`, este registro e
  `tmproject/client748/project.exe`. Nenhum arquivo removido.
- `CLIENT_TESTED` pendente: conferir abertura/reabertura e bordas coladas ao
  ícone no jogo. Nenhum processo do client iniciado ou encerrado.
