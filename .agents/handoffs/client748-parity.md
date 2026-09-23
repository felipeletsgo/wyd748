# Handoff: paridade visual e funcional do client 7.48

> Registro histórico de continuidade. Comandos, scripts e resultados abaixo pertencem às rodadas registradas; não indicam disponibilidade nem validação no HEAD atual. Para operação atual, consulte `DOCS/build-and-integration.md` a partir da raiz.

Atualizado em: 2026-09-03
Estado geral: `STATICALLY VERIFIED`

## Objetivo e limites

Tornar `tmproject` compatível apenas com o WYD 7.48, portando para
source os comportamentos comprovados pela referência histórica. Os scripts e
binários em `references/client748/` são somente material de estudo e
não participam mais do build nem da instalação. O escopo atual concentra HUD compacto, render contextual de itens,
input/câmera e lifecycle de fechamento; paridade completa da UI ainda não foi
testada.

## Fontes e artefatos

```text
references/client748/WYDoriginal.exe | stock histórico | B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C
references/client748/WYD.exe         | referência Ghidra | 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
tmproject/client748/project.exe                            | candidato source | BA85D6CB23D88E3C56DE7A996D0A782E0888CE8819CAE23448A16334510D2277
```

Os hashes históricos permanecem os fingerprints imutáveis registrados; o
hash de `project.exe` foi atualizado pelo build de 2026-09-02. Esse hash é
volátil e deve ser recalculado depois de qualquer build.

O build `Release|Win32` deste candidato concluiu com zero erros e 13 warnings
C4018 preexistentes; o warning C4805 introduzido no helper Quest foi eliminado;
`validate_research.py` e `git diff --check` também passaram.

## Evidência confirmada

- A política documental foi auditada em 2026-08-25: instruções e skills ativas
  não chamam scripts nem manifestos da cadeia histórica.
  `tmproject/client748/project.exe` é o único executável aceito para validação; a pasta
  histórica permanece somente evidência estática.
- `Build-Client.ps1` instala automaticamente todo build bem-sucedido em
  `tmproject/client748/project.exe` e compara o SHA-256 do output transitório com o
  candidato instalado. Divergência de hash ou ausência de `client748` falha o
  build. `Run-Client748.ps1` executa exclusivamente esse candidato instalado;
  não existe etapa manual de cópia nem fallback para `build/.../WYD.exe`.
- `FUN_0040d13e` lê o registro do catálogo, chama `FUN_0040cea0` para resolver o
  índice e só então liga a mesh por `FUN_00401bdc`; na source esse resolvedor é
  `BASE_GetMeshIndex`. O candidato `41669031...` ignorou essa etapa e passou o
  `nIndexMesh` bruto, produzindo esferas/pedras genéricas no lugar das meshes dos
  itens. A caixa local nativa continua `24 × dimensão`, sem ratio de viewport.
- `FUN_0040fc3e` passa a origem da célula ao item ordinário e `FUN_0040dd00`
  aplica sozinho a correção de meia largura/altura. A source deve preservar a
  mesma conversão nos dois eixos: o candidato `F8DA50A0...` omitiu meia largura
  e foi reprovado no mundo com todos os itens deslocados para a esquerda.
  Equipamento primeiro centraliza a origem da caixa dentro do receptáculo e
  reutiliza esse frame; trade/mix continua no helper que recebe o centro final.
- As capturas do candidato `2C92F658...` confirmaram o sintoma dessa soma
  duplicada: itens de grades regulares apareciam aproximadamente meia célula à
  direita, enquanto os slots irregulares de equipamento continuavam centrados.
- `FUN_0040e6aa` e `FUN_0040e817` ocupam os slots `+0x8C/+0x90` da vtable em
  `0x005A4024` e leem exclusivamente `mesh+0x354` (`MaxZ`). A fórmula nativa é
  `targetHeight = itemCellHeight * 0.3` e
  `scale = MaxZ > targetHeight ? targetHeight / MaxZ : 1.0`; não há leitura da
  diagonal do AABB, limite `0.27` ou ampliação acima de `1.0`.
- A comparação histórica entre os binários 7.48 altera somente
  `g_pItemGridXY` e a máscara de ocupação; ela não corrige escala nem posição de
  render. Essa evidência foi portada para a source, sem executar ou reutilizar
  os patches, e o candidato mantém a fórmula stock de `FUN_0040e817`.
- Inventário, Cargo, loja, trade/mix e item arrastado compartilham o helper de
  escala por `MaxZ`. O painel irregular de equipamento continua centralizado
  em sua própria região e recebe escala explícita `1.0`.
- Não existe exceção nativa por nome, tipo ou família: `egg001..egg014` segue
  exatamente a mesma fórmula por `MaxZ` dos demais meshes. As caixas lógicas
  são `24x24` para mesh/UI legada, `23x23` para sprites UI2 `5000..5102` e
  `32x32` para os demais sprites UI2.
- `SCursor::FrameMove2` copia o `GeomControl` do item, incluindo `fScale`, antes
  de ancorá-lo ao cursor; assim o drag preserva a escala do contexto de origem.
- `FUN_00435b13`: HUD compacto usa IDs 1029, 1030, 1031, 1032, 1033 e 1040;
  Skill comum 1905 e Skill Apprentice 1889 são painéis independentes.
- `FUN_004431e4`: EXP é par atual/limiar, ATT e DEF possuem valores companions e
  1171–1174 implementam a progressão nativa de quatro quartos.
- `FUN_00403924`: a notificação translúcida 7.48 usa `MessagePanel2` 178 com
  bordas 259/260 e altura 24. Login, notice e contagem de saída compartilham
  `SMessagePanel`; o modo CLASSIC global não deve reativar o painel marrom 45.
- `FUN_004776c3`: o frame nativo do mundo chama base, `TimeDelay` e `AirMove_Main`.
  O caminho compatível agora segue essa ordem; `TimeDelay` publica a contagem de
  cinco segundos e envia o segundo `WM_CLOSE`.
- `FUN_004aec3d` e `EventTranslator`: rotação requer `QuarterView == 0`; com
  `[CAMERAROTATE] 1`, o 7.48 usa arraste do botão do meio ou Alt+botão direito.
  Com `[CAMERAVIEW] 0`, o sentido vem de `viewchange` e a magnitude do wheel.
- `tmproject/client748/client-debug.log`: os seis controles existem sob parent 5716 nas
  posições registradas em `references/client-ui-748.md`.
- `tmproject/client748/screenshot/Capture0084.jpg`: referência original de HUD e grid.
- O 7.48 materializa todos os ItemMix 1–6. A correlação confirmada é:
  Compositor/1/`0x3A6`, Aylin/2/`0x3B5`, Agatha/3/`0x3BA`,
  Tiny/4/`0x3C0`, Lindy e Odin/5/`0x2C3` e `0x2D2`, e
  Ehre/6/`0x2D3`. O modo alternativo `0x2C4` compartilha ItemMix5, mas está
  dormente: não há handler/receita autoritativa atual no servidor.
- Os controles 65857/65861+/81923/86019 são deriva da source mais nova; não são
  painéis ItemMix 7.48 disponíveis para novos sistemas.
- O candidato `B424BE8D...` caiu ao entrar no mundo. O dump
  `tmproject/client748/client-crash-20260824-213257.dmp` registra AV de escrita em
  `0x0066C5B3`; removendo ASLR, `0x004BC5B3` resolve pelo PDB para
  `TMFieldScene::SetEquipGridState`, na escrita em `m_pGridNewSlot1 + 0x404`.
  O ponteiro é nulo porque os NewSlot 16/17 são controles posteriores ausentes
  do FieldScene2 7.48. O caller novo era o reset interativo de ItemMix durante
  `InitializeCompatFieldScene`.
- O candidato `2CAEA7C5...` passou da inicialização anterior, mas caiu ao receber
  um notice `!`. O dump `tmproject/client748/client-crash-20260824-220158.dmp` registra AV
  em VA carregado `0x0064BD66`; removendo ASLR, `0x0046BD66` resolve para
  `SListBox::AddItem` (`SControl.cpp:2400`) com `this == nullptr`. O caller
  `0x004C8551` é o ramo `!` de `TMFieldScene::OnPacketMessageWhisper`, que usa
  `m_pHelpList[3]`. O bootstrap compatível não ligava os controles Help nativos.
- `FUN_0052d2c8` confirma que o 7.48 escreve notices `!` diretamente na lista
  Help 874. O candidato atual liga painel/texto 864/865, botões 867/869/871/873,
  listas 868/870/872/874, memo 875 e summon 878, sem fabricar controles 7.59.
- `FUN_0045ee28` abre Cargo localmente, preserva o ID do banqueiro e não envia
  `0x28B`; o `SGrid` inclui esse ID em `MSG_SwapItem.TargetID`. O servidor agora
  valida o alvo por visibilidade, alcance e `Merchant` tipo 2, liga o contexto
  no primeiro swap e aceita o fallback determinístico ao banqueiro visível mais
  próximo para packets legados sem TargetID.
- `FUN_0051a939` mostra que um ataque fora do alcance calcula a célula mais
  próxima e chama imediatamente `FUN_00520216` para construir a rota. A source
  agora chama `GetRoute` no mesmo ramo, em vez de apenas guardar `m_vecMyNext`.
- O candidato `D510CBA8...` caiu após clicar na Kibita. O dump
  `tmproject/client748/client-crash-20260824-225343.dmp` registra leitura inválida em RVA
  `0xAB409` (`TMFieldScene::OnPacketEvent`) ao tratar o opcode `0x333`. A
  instrução lia `m_pPartyList + 0x218` com `m_pPartyList == nullptr`.
  `FUN_00481dd6`, handler nativo 7.48 alcançado pelo dispatcher
  `FUN_00492e7d`, verifica chat list e party list antes desse fluxo. Os handlers
  `OnPacketMessageChat` e `OnPacketMessageChat_Index` agora reproduzem esse
  lifecycle e descartam com segurança mensagens recebidas sem esses controles.
- Os dumps `client-crash-20260825-134231.dmp` e
  `client-crash-20260825-134443.dmp` registram AV em RVA `0x68A9D`, resolvida
  pelo PDB para `SEditableText::OnCharEvent` (`SControl.cpp:1297`). O caminho
  herdado da 7.59 desreferenciava o seletor de chat `B_CHAT_SELECT` 90114, que
  legitimamente não existe no 7.48. `FUN_00406bd7` confirma que o handler
  nativo não consulta esse controle; o caminho moderno e os IDs 90129–90136
  foram removidos e o listener de backspace vazio agora é protegido.
- O dump `tmproject/client748/client-crash-20260825-151016.dmp` registra leitura nula em
  RVA `0xCC643`, resolvida pelo PDB para `TMFieldScene.cpp:23080` ao abrir o NPC
  Skill Apprentice. O bootstrap 7.48 não ligava `m_pSkillMSec1`; a árvore real
  `FieldScene2.bin` e `FUN_00435b13` confirmam root 1889, child 1890, grade 1894
  e textos 6049–6052. O ID 6128 é `TML_SKILLM_DESC`, não a grade. O candidato
  atual liga esses controles, desabilita a
  seleção do child como o nativo e protege os dois handlers de ShopType 3.
- `FUN_004baf10` inicializa máximos da malha com `FLT_MIN` e mínimos com
  `FLT_MAX`. Os decimais recuperados na source quebravam o pivô/AABB e
  contribuíam para o deslocamento. `FUN_0040e817`, `FUN_0040dd00` e
  `FUN_0040fc3e` confirmam que inventário/cargo recebem a origem da célula e
  aplicam meia largura/altura no item, enquanto equipamento centraliza na
  região inteira. A escala nativa é uniforme, limitada por `MaxZ`, sem esticar
  e sem ampliar modelos pequenos.
- O grid lógico e visual usa uma única definição 1×1 em `g_pItemGridXY` e
  `g_pItemGrid`. A tentativa de recuperar uma segunda tabela de footprints para
  render foi removida; escala e mesh voltaram ao fluxo nativo comum.
- O catálogo ativo usa 6500 registros XOR `0x5A` de 140 bytes e 12 pares de
  efeitos a partir de `0x50`. `tmproject/client748/ItemList.bin` é o asset canônico 1×1:
  os 1.980 pares de efeito 33 (`EF_GRID`) têm valor zero e todo o restante do
  arquivo foi preservado. SHA-256 `112C6CFA...AE8627E3`, stamp `8F268603`.
  `CopyLegacyItem` apenas carrega os efeitos; não existe normalização runtime.
  A referência anterior foi preservada como
  `ItemList.pre-grid1x1.2C9323E0374F.bin`, sem tocar no catálogo sob `mesh/`.
- `FUN_0052b72a`, `FUN_004770ad` e `FUN_00435b13` confirmam os dois canais de
  affect 7.48: `0x336` atualiza flags/efeitos do avatar para dono e observers;
  `0x3B9` atualiza os 16 affects completos do dono e seus ícones 23×23 no topo.
  O bootstrap compatível agora liga o mini painel 292, descrição 12834 e cria
  os painéis 12806–12821 com textura 200 e o lifecycle nativo de duração. Como
  o pacote instalado usa `[CLASSIC] 1`, `Affect_Main` recria essa linha stock
  também no FieldScene2 clássico, em vez de ocultar todos os ícones pelo gate
  de UI2 presente na source herdada.
- O wire anterior emitia `0x3B9` com 268 bytes e 32 posições herdadas. O gate
  nativo de `FUN_0052b72a` aceita 140 bytes: header de 12 mais 16 affects de 8.
  Source client, servidor, asserts e testes agora compartilham esse ABI 7.48.
- `FUN_00506f9d` confirma o lifecycle visual exato de Lighten, Magic Shield e
  Skill Amp. Os três blocos estavam comentados na source e foram restaurados:
  Lighten mantém os dois billboards 109, Shield reinicia `m_pMagicShield` a
  cada segundo e Skill Amp usa billboard 93 com escala/cor/fade nativos.
- A auditoria de `SkillData` encontrou como única projeção fora da faixa visual
  nativa o tipo 50 de Armadura Crítica. Aplicação, persistência/relogin e wire
  agora o projetam para semântica 31 e ícone 24; os demais tipos permanecem
  conforme `CheckAffect` do executável 7.48.
- `FUN_004431e4` indexa `DAT_005b77c0[Type]` para os tipos de affect `0..39`.
  A source herdada misturava 17 índices de textura da 7.59, inclusive EXP
  `39 -> 162`, que não existe no atlas clássico de 128 células. Os primeiros
  40 valores de `g_AffectSkillType` agora são byte-idênticos à tabela nativa;
  EXP usa a célula `85` e o maior índice dessa faixa é `125`.
- `FUN_004110f5` confirma que a seleção de item para AutoTrade no Cargo de tipo
  10 abre o painel 626, foca o edit 627 e usa caption 630, enquanto o botão 667
  precisa estar visível. O adapter já traduzia os filhos, mas omitia o painel;
  `WYD748_TranslateControlID` agora mapeia `626 -> 65885`, eliminando o retorno
  silencioso de `SGridControl::TradeItem` sem alterar grid ou posicionamento.
- O dump `client-crash-20260826-162021.dmp` mapeia a nova falha em
  `TMFieldScene::OnControlEvent`, RVA `0x000A9570`: após publicar AutoTrade, a
  source ocultava diretamente `m_pCargoPanel1`, controle herdado da 7.59 que
  não existe no FieldScene2 do 7.48. `FUN_004662c5` confirma que o evento 667
  envia `919 / 0x397` e testa cada superfície de Cargo antes de ocultá-la;
  o cancelamento 668 delega a `FUN_0044ae38`. O ponteiro agora nasce nulo e é
  guardado no botão compartilhado de dinheiro, na confirmação da AutoTrade,
  no bloqueio de movimento e no fechamento das superfícies após a publicação.
- `FUN_004662c5` também confirma que o botão nativo 313 chama
  `FUN_004656af` quando a AutoTrade está fechada e `FUN_0044ae38` quando está
  visível. O dispatch compatível agora consome o alias 65794 imediatamente;
  isso impede handlers intermediários de engolirem a abertura do prompt 626,
  que publica o título usado no render e no clique de consulta da loja.

Reabrir funções e callers/callees antes de uma nova edição; os endereços valem
para o hash legado acima.

## Estado atual

```text
grid lógico 1x1 no asset            | AUTOMATED TESTED      | 1980 EF_GRID=0; XOR/stamp/diff seletivo validados
escala visual em célula 1x1         | STATICALLY VERIFIED   | MaxZ e altura 0.3; escala nunca acima de 1
inventário/cargo/loja/trade/drag    | STATICALLY VERIFIED   | helper por MaxZ; Release v145 passou
equipamento sem contenção 1x1       | AUTOMATED TESTED      | centro do controle + escala explícita 1.0
centralização candidato 2C92F658    | CLIENT-TESTED/FALHOU  | meia célula adicionada duas vezes em grades regulares
correção de posição 1EEA1FC1        | CLIENT-TESTED/FALHOU  | inventário subiu meia célula; equipamento ficou 12 px à direita
correção de posição F8DA50A0        | CLIENT-TESTED/FALHOU  | meia largura omitida deslocou todos os itens para a esquerda
correção de posição 5A4AEC0A        | CLIENT-TESTED/PASSOU  | captura confirmou grades e equipamento centralizados
notificação translúcida 7.48        | STATICALLY VERIFIED  | recursos 178/259/260
Skill 1905 separada do NPC 1889     | STATICALLY VERIFIED  | TMFieldScene.cpp + Ghidra
crash NPC Skill Apprentice 20260825 | STATICALLY VERIFIED  | dump/PDB + controles 1889/1890/6049–6052/6128
crash NPC Skill Apprentice 20260827 | STATICALLY VERIFIED  | m_pHellgateStore nulo; lifecycle 1889/1905 protegido
compra de skill no mestre 85FC6B25  | CONTRACT              | renderer 2D + TargetID 0x277 + testes byte-level
composição e candidatos IME 1DF5956A | TRACED/STATIC VERIFIED | dispatcher, foco, candidatos e teardown adaptados
Premium Firework 76B3E66E             | IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED TESTED | contrato 0x3C9/0x3CA; efeito nativo restaurado
array de animação 0x1C1/0x2C2 DA9F578E | CONTRACT / STATICALLY VERIFIED / AUTOMATED TESTED | wire nativo; probe server-side fail-closed
layout/valores do HUD compacto      | STATICALLY VERIFIED  | texto sem stretch + IDs nativos
EXP nativa em quatro quartos        | STATICALLY VERIFIED  | controles 1171–1174
layout C/S/I lado a lado CD92A005   | STATICALLY VERIFIED  | FUN_00435b13 + roots 513/1905/257
campos Character 1110/1168/1376/1377 | IMPLEMENTED / STATICALLY VERIFIED | Att Speed/C.POINT/HOLD/Kingdom nativos
layout Gamble 3EDB5818              | STATICALLY VERIFIED  | root 6400; centro vertical e +10 X reaplicados no open
layout/lifecycle Party              | IMPLEMENTED / STATICALLY VERIFIED | roots 1857/1863/5742; toggle e handlers nativos restaurados
layout/lifecycle Quest              | IMPLEMENTED / STATICALLY VERIFIED | toggle 315; root 320; quatro abas/listas e teardown restaurados
wheel/rotação de câmera             | STATICALLY VERIFIED  | input/lifecycle nativo
contagem de 5 s após X              | STATICALLY VERIFIED  | TimeDelay no frame compat
atalhos/chat/minimap clássicos      | STATICALLY VERIFIED  | somente controles 7.48
ItemMix 1–6 e roteamento por NPC    | STATICALLY VERIFIED  | IDs/opcodes/Ghidra + build D510CBA8
handlers ItemMix ativos no servidor | AUTOMATED TESTED     | game/wire sem cache
modo ItemMix5 alternativo 0x2C4     | DORMANT               | sem handler/receita autoritativa
entrada no mundo B424BE8D           | CLIENT-TESTED/FALHOU   | AV em NewSlot1 nulo, dump preservado
entrada no mundo 2CAEA7C5           | CLIENT-TESTED/FALHOU   | AV em m_pHelpList[3] nulo
correção do crash D510CBA8          | STATICALLY VERIFIED    | bindings Help nativos 865/867–875/878
clique na Kibita D510CBA8           | CLIENT-TESTED/FALHOU   | AV em m_pPartyList nulo, dump preservado
correção do crash 5CB1E7EC          | STATICALLY VERIFIED    | guards nativos nos handlers 0x333/0x105
digitação no mundo 5CB1E7EC         | CLIENT-TESTED/FALHOU   | AV no seletor de chat 7.59 ausente
correção do crash D28E4F25          | STATICALLY VERIFIED    | OnCharEvent conforme FUN_00406bd7
Cargo próximo/TargetID legado       | AUTOMATED TESTED       | validação autoritativa + fallback espacial
autoaproximação para atacar         | STATICALLY VERIFIED    | GetRoute conforme FUN_0051a939
efeitos públicos de buffs           | AUTOMATED TESTED       | 0x336 para dono e observers
efeitos Lighten/Shield/Skill Amp    | STATICALLY VERIFIED    | FUN_00506f9d restaurada na source
ícones/duração de buffs no topo     | AUTOMATED TESTED       | ABI 0x3B9 140B + 16 painéis 12806–12821
atlas dos buffs 0..39 C3108E5A      | STATICALLY VERIFIED    | DAT_005b77c0 exata; EXP 39 -> textura 85
Armadura Crítica tipo 50            | AUTOMATED TESTED       | semântica 31 / ícone 24 inclusive relog
grid/buffs do candidato 4BE5943C    | CLIENT-TESTED/FALHOU   | itens multisslot e nenhum ícone de buff
grid do candidato 44677EF8          | CLIENT-TESTED/FALHOU   | itens ainda invadiam células vizinhas
grid do candidato 41669031          | CLIENT-TESTED/FALHOU   | índice bruto exibiu esferas/pedras, mesh correta sumiu
build/instalação 5A4AEC0A           | AUTOMATED TESTED       | Release + assets + cópia/hash automáticos passaram
startup do candidato 1EEA1FC1       | CLIENT-TESTED/PASSOU   | captura em jogo confirmou inventário e equipamento renderizados
startup do candidato F8DA50A0       | CLIENT-TESTED/PASSOU   | captura em jogo confirmou entrada no mundo
startup do candidato 5A4AEC0A       | CLIENT-TESTED/PASSOU   | captura confirmou entrada e render no mundo
seleção AutoTrade 35DA2FB6          | STATICALLY VERIFIED    | painel nativo 626 traduzido para 65885; build/assets passaram
abertura AutoTrade 9290A758          | STATICALLY VERIFIED    | dump 162021 corrigido; Release/assets/hash passaram
prompt/nome AutoTrade 0A5AD930        | STATICALLY VERIFIED    | botão 313 roteado conforme FUN_004662c5; build/assets passaram
título/clique da loja 1AA86EE3        | STATICALLY VERIFIED    | Desc 0x364 preservado; build/wire/assets/hash passaram
placa/preços AutoTrade 73CABC41       | STATICALLY VERIFIED    | lifecycle por TradeDesc e labels 800..811 restaurados
placa/clique AutoTrade E077183B       | STATICALLY VERIFIED    | LabelPosition2 + left-click GRID_TRADEMY2 restaurados
confirmação/título AutoTrade 7BA846DA | STATICALLY VERIFIED    | message box NewUI e ordem nativa texto/painel restauradas
posição/persistência AutoTrade CAF93919 | IMPLEMENTED            | build passou; instalação bloqueada pelo project.exe PID 11008
escala por MaxZ 30DA37B9            | HISTÓRICO/REFUTADO     | aplicava fórmula nativa só aos ovos; trace posterior removeu a exceção
paridade visual dentro do mundo     | NÃO TESTADO          | autenticação não automatizada
fechamento X/Esc e modais real      | NÃO TESTADO          | checklist client-ui-748.md
```

O rebuild Release Win32 com toolset v145 terminou sem erros para o candidato
atual; a compilação registrou 21 advertências preexistentes.
O artefato instalado e o output de build possuem o mesmo SHA-256
`3EDB5818...64774`; `Build-Client.ps1` instalou e verificou o candidato
automaticamente, sem patch. Os candidatos
`4BE5943C...`, `44677EF8...`, `41669031...`, `1EEA1FC1...` e `F8DA50A0...` foram
testados pelo usuário e reprovaram no grid/mesh; não atribuir essas falhas ao
novo binário antes do teste real.

## Worktree e arquivos ativos

- `tmproject/TMProject748/internal/ui/SGrid.cpp` — grid ordinário usa
  `BASE_GetMeshIndex` para itens com mesh e o renderer 2D nativo para entradas
  com `nIndexMesh < 0`, incluindo os livros de skill no atlas 199/1. Sprites
  usam a origem da célula; meshes mantêm a centralização. Inventário, Cargo,
  loja, trade/mix e drag usam a fórmula nativa compartilhada por `MaxZ`, sem
  exceção por família; equipamento preserva escala `1.0`. As caixas lógicas
  `23/24/32` acompanham o ratio do viewport e sincronizam item e `GeomControl`.
- `tmproject/TMProject748/internal/render/mesh/TMMesh.cpp` — sentinelas nativas
  `FLT_MIN`/`FLT_MAX` para AABB e pivô estáveis.
- `tmproject/TMProject748/internal/core/WYD748Assets.cpp` — o adapter do
  catálogo 7.48 preserva todos os efeitos exatamente como estão no asset; a
  normalização runtime de `EF_GRID` foi removida. O mesmo adapter traduz o
  painel nativo 626 para o prompt semântico 65885 usado pelo AutoTrade.
- A rodada usou uma ferramenta de geração/verificação de ItemList hoje ausente
  da árvore. O registro não constitui um comando operacional atual; a evidência
  registrada abaixo deve ser preservada e confrontada com o asset atual.
- `tmproject/client748/ItemList.bin` — asset canônico 1×1, SHA-256
  `112C6CFA...AE8627E3`; todos os 1.980 `EF_GRID` têm valor zero.
- `references/client748/` — material histórico somente para estudo;
  não editar nem executar os scripts desse diretório.
- `tmproject/TMProject748/internal/app/scenes/TMFieldScene.cpp` — HUD 7.48,
  seis ItemMix nativos, controles Help/affect/Skill Apprentice 7.48, linha de
  buffs no FieldScene2 clássico, autoaproximação de ataque e resposta nativa
  ao request de array de animação `0x1C1`.
- `tmproject/TMProject748/internal/core/Basedef.h` e
  `WYD748Compat.cpp` — contrato Win32 de 24 bytes de `MSG_REQArray`, opcodes
  `0x1C1/0x2C2` e asserts dos offsets `0x0C`, `0x10` e `0x14`.
- `tmproject/TMProject748/internal/render/mesh/MeshManager.h/.cpp` — conserva o
  tamanho bruto exato dos buffers `.bon` sem alterar os offsets herdados de
  `stBoneAni`.
- `.agents/research/client748/flows/transport/bone-animation-array-probe.md` —
  ficha `CONTRACT` do dispatcher, gate, handler, loader, sender, wire,
  ownership e teardown do fluxo.
- `internal/wire/client_integrity.go`, `internal/game/client_integrity.go` e
  `data/client_integrity.json` — contrato de 24 bytes, manifesto server-side,
  seleção, pending, timeout, fail-closed e cleanup de sessão.
- `tmproject/TMProject748/internal/game/entities/TMHuman.cpp` — efeitos nativos
  Lighten, Magic Shield e Skill Amp restaurados conforme `FUN_00506f9d`.
- `tmproject/TMProject748/internal/core/EventTranslator.cpp` — wheel e
  sentido do zoom 7.48; composição `GCS_COMPSTR` e página agregada de
  candidatos IME com validação integral do bloco `CANDIDATELIST`.
- `tmproject/TMProject748/internal/app/scenes/NewApp.cpp` — dispatch nativo de
  `WM_IME_COMPOSITION`, `WM_IME_ENDCOMPOSITION` e `WM_IME_NOTIFY`, protegendo
  cena e controles opcionais.
- `tmproject/TMProject748/internal/ui/SControl.cpp` — cursor preserva o
  `GeomControl`, `SMessagePanel` usa recursos translúcidos, o input de texto
  não consulta seletores de chat exclusivos da 7.59 e `SEditableText` recebe
  composição IME somente com foco.
- `tmproject/TMProject748/internal/ui/SControlContainer.cpp` — foco,
  pick e modais são desligados antes da destruição da árvore de controles.
- `tmproject/TMProject748/internal/app/scenes/TMScene.cpp` — painel compartilhado
  de login/notice/saída com altura nativa de 24 pixels.
- `AGENTS.md` e `references/client-ui-748.md` — gates de regressão
  para grid, transformação de UI e lifecycle de painéis.
- `internal/game/handlers.go` — resolução autoritativa do banqueiro para Cargo.
- `internal/game/coverage_more_test.go` — cobertura de bind, fallback, distância,
  visibilidade, DWORD alto e packets inválidos de Cargo.
- `internal/game/skills_integration_test.go` — contrato byte-level de
  `MSG_ApplyBonus` para o Skill Master, incluindo TargetID, catálogo vendido,
  persistência, publicação e rollback.
- `internal/game/affects.go`, `charstate.go` e `internal/wire/codec.go` — projeção
  7.48 de Armadura Crítica em aplicação, relogin e serialização.
- A worktree contém muitas mudanças de sessões anteriores; inspecionar o diff
  de cada arquivo e não revertê-las.

## Última validação

```text
verificado em 2026-08-26:
- a tabela nativa `DAT_005b77c0` de `FUN_004431e4` foi comparada com os 40
  tipos visuais da source: as 17 divergências herdadas foram removidas; EXP
  agora traduz `Type 39` para textura `85`, dentro do atlas clássico 0..127
- build oficial Release Win32 v145 passou com 31 warnings preexistentes e zero
  erros; output e `tmproject/client748/project.exe` são idênticos com SHA-256
  `C3108E5A3806539BD3C95E63ABFF92F436745B2CD83A0EE188F04AAFF4065001`
- `git diff --check` passou, restando somente avisos LF/CRLF; a correção dos
  ícones está `STATICALLY VERIFIED` até o teste visual no client real
- `Test-Client748Assets.ps1` passou para o pacote instalado: 3.584 texturas,
  104 skills, 6.500 itens e 18 shaders no perfil WYD 7.48
- `FUN_004662c5` reconfirmou o toggle do botão 313: `FUN_004656af` abre o
  prompt nativo 626/627/630/574 e `FUN_0044ae38` fecha a AutoTrade visível
- o alias 313 -> 65794 agora é consumido no primeiro switch compatível de
  `TMFieldScene::OnControlEvent`, antes dos handlers genéricos intermediários
- build oficial Release Win32 v145 passou com 13 warnings preexistentes e zero
  erros; output e `tmproject/client748/project.exe` são idênticos com SHA-256
  `0A5AD9303F8435AE4A1DB45C0528B7C9B13971A88ED3F6F52008404154F1D71D`
- `Test-Client748Assets.ps1` e
  `go test -count=1 ./internal/game ./internal/wire` passaram
- dump `client-crash-20260826-162021.dmp` mapeado até
  `TMFieldScene::OnControlEvent`, RVA `0x000A9570`: a confirmação da AutoTrade
  desreferenciava `m_pCargoPanel1`, página 7.59 ausente no FieldScene2 7.48
- `FUN_004662c5` confirmou o envio `919 / 0x397` pelo botão 667 com testes
  individuais das superfícies de Cargo; o botão 668 chama `FUN_0044ae38`
- `m_pCargoPanel1` agora é inicializado deterministicamente e possui guardas no
  botão de dinheiro, na confirmação da loja, no bloqueio de movimento e ao
  ocultar Cargo depois da publicação
- build oficial Release Win32 v145 passou com 13 warnings preexistentes e zero
  erros; output e `tmproject/client748/project.exe` são idênticos com SHA-256
  `9290A75860833401AF2C7A539DDEF532F3414EC1283ABB6FF89E464A6D3E7C1D`
- `Test-Client748Assets.ps1` passou com 6.500 itens, 3.584 texturas, 104 skills
  e 18 shaders; `git diff --check` passou, restando só avisos LF/CRLF
- dump `client-crash-20260826-154302.dmp` do candidato
  `30DA37B9...92044` mapeado por ASLR/PDB até RVA `0x718AC`,
  `SGridControl::TradeItem`: o crash era a desreferência de
  `m_pChatSelectPanel`, controle 7.59 ausente no layout 7.48
- Ghidra 7.48 reconfirmou `FUN_004110f5` com painel 626, edit 627, caption 630
  e botão 667, sem seletor de chat; callers/fechamento relevantes:
  `FUN_004209fc`, `FUN_00447594` e `FUN_004662c5`
- `SGridControl::TradeItem` e o retorno de preço inválido em
  `TMFieldScene::OnControlEvent` agora toleram o seletor 7.59 nulo; a blacklist
  voltou ao contrato nativo (`508`, `509`, `522`, `526..537`, `747` e
  `3200..3299`), permitindo novamente o item `4905`
- build oficial Release Win32 v145 passou com 15 warnings preexistentes e zero
  erros; output e `tmproject/client748/project.exe` =
  `A63DD267B9149B06F3CCFA0086893A45CADB5DB8EDEA0BE2814C3BF068922ABA`
- `Test-Client748Assets.ps1` passou com 6.500 itens, 3.584 texturas, 104 skills
  e 18 shaders; `git diff --check` passou, restando só avisos LF/CRLF
- correção da AutoTrade está `STATICALLY VERIFIED`; promover para
  `CLIENT-TESTED` somente após adicionar item, testar preço válido/inválido e
  confirmar a abertura da loja no client real
- `FUN_0040e817` foi reaberta em uma etapa intermediária para aplicar
  `min(1, 0.3 / MaxZ)` somente a `egg001..egg014`; a investigação posterior de
  `FUN_0040e6aa`, da vtable e dos dispatches indiretos refutou essa exceção e a
  contenção pela diagonal do AABB
- build Release Win32 v145 passou com duas advertências preexistentes e zero
  erros; output e `tmproject/client748/project.exe` = `30DA37B9...92044`
- `Test-Client748Assets.ps1` passou com perfil 7.48, 6.500 itens, 3.584 texturas,
  104 skills e 18 shaders; `git diff --check` encontrou apenas avisos LF/CRLF
- `FUN_004110f5` reaberta para o clique de AutoTrade: grid type 10, painel 626,
  edit 627, caption 630 e botão 667 confirmados contra callers da source
- alias `626 -> 65885` compilado em Release Win32 v145 com zero warnings e zero
  erros; output e `tmproject/client748/project.exe` = `35DA2FB6...FB176`
- `Test-Client748Assets.ps1` passou após o rebuild e `git diff --check` não
  encontrou erros, somente avisos de conversão LF/CRLF da worktree existente
- dois dumps reais mapeados por ASLR/PDB até `SEditableText::OnCharEvent` e a
  desreferência do seletor de chat 7.59 ausente
- dump `client-crash-20260825-151016.dmp` mapeado por ASLR/PDB até o acesso
  nulo a `m_pSkillMSec1` no NPC Skill Apprentice
- Ghidra 7.48 reaberto para construtor de malha, grid ordinário, receptáculo de
  equipamento, initializer/handlers Skill Apprentice, dispatcher de affects,
  `CheckAffect`, painel de ícones e `TMHuman::FrameMoveEffect`
- auditoria de todos os tipos de affect usados por `SkillData`; somente o tipo
  50 exigia projeção para a faixa visual nativa
- build Release Win32 v145 com 31 warnings preexistentes e zero erros depois da
  contenção visual por AABB completo, hipótese intermediária posteriormente
  refutada pelo trace nativo por `MaxZ`
- build Release Win32 v145 com 21 advertências preexistentes e zero erros depois
  da correção assimétrica dos anchors de grid/equipamento
- hashes do output e `tmproject/client748/project.exe` = `5A4AEC0A...E48F6`, idênticos
  após instalação e verificação automáticas sem patch binário
- a busca final não encontrou instrução ativa de cópia manual; `git diff
  --check` passou, restando somente avisos de conversão LF/CRLF
- `go test ./internal/wire ./internal/game`, `go test ./...` e `go vet ./...`
  passaram; `0x3B9` possui teste explícito de 140 bytes/16 affects
- o teste de schema agora pula corretamente `data/accounts` opcional vazio,
  assim como já pulava o diretório ausente
- `Test-Client748Assets.ps1` passou: perfil 7.48, 3584 registros de textura,
  104 skills e 18 shaders com tamanhos/hashes esperados
- `git diff --check` passou; restaram apenas avisos de conversão LF/CRLF
- o candidato `4BE5943C...` foi reprovado pelo usuário: calça ocupava cerca de
  seis células e os ícones de buffs não apareciam
- o candidato `44677EF8...` foi reprovado pelo usuário: o inventário continuou
  com itens invadindo células vizinhas; a análise posterior encontrou mesh,
  caixa local e centralização divergentes de `FUN_0040d13e/0040fc3e/0040dd00`
- o candidato `41669031...` foi reprovado pelo usuário: a troca para
  `nIndexMesh` bruto removeu as meshes corretas e exibiu esferas/pedras genéricas
- o candidato `2C92F658...` mantém `BASE_GetMeshIndex`, usa o footprint zero do
  asset e aplicava a hipótese posteriormente refutada de contenção por AABB em
  todos os grids regulares; as capturas do usuário reprovaram sua posição por
  deslocamento de meia célula
- o candidato `1EEA1FC1...` foi reprovado pela captura do usuário: os itens das
  grades regulares ficaram centralizados horizontalmente, mas meia célula acima;
  os itens equipados continuaram meia célula à direita
- o candidato `F8DA50A0...` foi reprovado pela captura no mundo: omitir meia
  largura no frame comum deslocou uniformemente inventário e equipamento para a
  esquerda
- o candidato `5A4AEC0A...` restaura meia largura e meia altura conforme
  `FUN_0040dd00`; equipamento continua preparando a origem centralizada no
  receptáculo e trade/mix preserva o helper de centro final. A captura do
  usuário confirmou visualmente inventário/loja e equipamento centralizados e
  foi classificada como `CLIENT-TESTED/PASSOU`
- `Update-Client748ItemList.ps1 -VerifyOnly` confirmou 6.500 registros, XOR
  `0x5A`, stamp `8F268603`, 1.980 `EF_GRID` zerados e somente 1.721 bytes de
  valor diferentes da referência preservada
- `ItemList.bin` ativo = `112C6CFA...AE8627E3`; referência anterior =
  `2C9323E0...A18F0DC5`; `tmproject/client748/mesh/ItemList.bin` permaneceu intacto
- `Test-Client748Assets.ps1` passou com o catálogo canônico 1×1; nenhum script
  do diretório histórico foi executado
- `MSG_UseItem` agora republica o slot fonte em recusas e rollbacks diretos de
  catálogo, Magical Pill, skill especial, summon, barra de gold e posição;
  tint/untint/Repliction também republicam o alvo restaurado e, quando vestido,
  o estado visual autoritativo. Nenhum arquivo de client/layout foi alterado.
- `go test ./internal/game -run 'TestOnUseItem'`, `go test ./internal/game`,
  `go test ./internal/wire ./internal/game` e `go vet ./...` passaram após o
  resync. `go test ./...` ficou bloqueado somente pela ausência local de
  `tmproject/client748/Mounts-KR.json` e `tmproject/client748/Costumes-KR.json`; `internal/game`
  permaneceu verde nessa execução.
- Ghidra 7.48 reconfirmou que `FUN_00492e7d` despacha `0x363/0x364` para
  `FUN_004829f2`; no spawn de loja, a descrição de 24 bytes no offset wire 326
  alimenta o campo equivalente a `TMHuman::m_TradeDesc` e seu controle de texto.
- `OnPacketCreateMobCompat` agora preserva esse título após `InitObject`; o
  mesmo `m_TradeDesc[0]` governa tanto o label visual quanto o envio `0x39A` ao
  clicar na loja. Spawn comum limpa explicitamente o campo e o controle.
- `offsetof(MSG_CreateMobTrade, Desc) == 326` e o tamanho total de 352 bytes
  ficaram protegidos por `static_assert`; a cópia termina o buffer sem usar o
  `sprintf` inseguro do handler legado.
- build oficial Release Win32 v145 passou e instalou output idêntico ao
  `tmproject/client748/project.exe`, SHA-256
  `1AA86EE3CC292C0BD0FF429F83145D874867A13C7CF258649CAC66084BD2B17B`.
- `go test -count=1 ./internal/wire`, `Test-Client748Assets.ps1` (6.500 itens,
  3.584 texturas, 104 skills e 18 shaders) e `git diff --check` passaram; este
  ajuste está `STATICALLY VERIFIED` até confirmar título e abertura no client.
```

Repetir build, instalação e hash se o código mudar.

## Placa persistente da AutoTrade

- A captura do usuário confirmou que o título textual já era publicado, mas o
  fundo visual persistente da loja permanecia invisível.
- Ghidra 7.48 (`FUN_004f7ea6`, `FUN_004ff400` e `FUN_00504a80`) confirmou o
  contrato nativo: texture set `446`, posição inicial `(-10, 635)`, dimensões
  `143x50`, cor `0x77777777`, `IMAGE_STRETCH` e seleção desabilitada.
- `tmproject/client748/UI/UITextureSetList.txt` confirma que o set `446` é
  `NewUI_AutoTrade_BG`, com um item `143x50`. O índice anterior `512` estava
  fora do intervalo válido `0..511` e por isso não produzia imagem.
- As duas criações de `m_pAutoTradePanel` em `TMHuman.cpp` agora usam o asset e
  as dimensões nativas. Como o renderer percorre a lista em ordem inversa,
  construtor e `CreateControl()` inserem o texto antes do painel; assim o painel
  permanece como fundo e o título fica visível após recriação dos controles.
- `Build-Client.ps1` passou com 0 erros e 4 warnings C4018 preexistentes,
  instalou output idêntico em `tmproject/client748/project.exe`, SHA-256
  `24451257F36DEAE9A103C0578E9E6B0204D55003B3B39F6AA9F71A32321173FC`.
- `Test-Client748Assets.ps1` passou com 6.500 itens, 3.584 texturas, 104 skills
  e 18 shaders; `git diff --check` não encontrou erro de whitespace. A placa
  está `STATICALLY VERIFIED` até confirmação no fluxo real.
- `TMHuman::FrameMove` não apaga mais a placa a cada quadro; texto e painel
  seguem `m_bVisible && m_TradeDesc[0]`, e o toggle global de nomes preserva a
  placa ativa fora do hover, conforme o lifecycle de `FUN_00504a80`.
- `FUN_00486424` e `FieldScene2.bin` confirmaram os labels nativos `800..811`
  na faixa escura sob os grids `653..664`. `OnPacketAutoTrade` agora formata,
  exibe e habilita comma mode nesses controles; slots vazios e o fechamento da
  janela limpam e ocultam os labels para impedir preço residual.
- A compilação completa Release Win32 v145 passou com 17 warnings C4018
  preexistentes e zero erros. Depois de fechar o client que bloqueava a cópia,
  `Build-Client.ps1` instalou output idêntico em `tmproject/client748/project.exe`,
  SHA-256 `73CABC41CD7573E90A8CB42D8003572728E7E42921C06244132B1B575BB4A5CE`.
- `Test-Client748Assets.ps1` passou com 6.500 itens, 3.584 texturas, 104 skills
  e 18 shaders; `git diff --check` passou, restando apenas avisos LF/CRLF. Esta
  etapa permanece `STATICALLY VERIFIED` até o teste visual no client real.

## Compra e composição da AutoTrade do comprador

- Os ramos de Carbunkle em `TMHuman::LabelPosition` e `LabelPosition2` só
  aplicam a ocultação por hover quando `m_TradeDesc` está vazio; clones de loja
  preservam painel e título continuamente em ambas as rotas de posicionamento.
- `GRID_TRADEMY2` não desenha mais a máscara preta usada pelos grids do
  vendedor. O receptáculo nativo de `FieldScene2.bin` continua intacto.
- O clique esquerdo ordinário em `GRID_TRADEMY2` agora entra em `TradeItem()` e
  abre a confirmação nativa 7.48 de controle `646`; somente o callback positivo
  chama `SendReqBuy`. O servidor ainda retorna `Not enough gold.` nas rejeições
  e confirma sucesso somente depois da persistência atômica das duas contas.
- A janela Carry do comprador recebe a âncora `(530,35)`, na mesma baseline da
  AutoTrade `(280,35)`, seguindo a composição lateral 7.48.
- Testes focados das compras, `go test -count=1 ./internal/game
  ./internal/wire` e `go vet ./...` passaram. `go test -count=1 ./...` só
  falhou em dois testes paralelos de `internal/data` porque
  `tmproject/client748/Mounts-KR.json` e `tmproject/client748/Costumes-KR.json` não existem.
- O build oficial Release Win32 v145 passou; após fechar o `project.exe` que
  bloqueava a cópia, instalou output idêntico em `tmproject/client748/project.exe`,
  SHA-256 `93DA0B486941CF8B381068F1D87BE3320EECA830EAB8EE9E637B4E86AEDB684A`.
  `Test-Client748Assets.ps1` e `git diff --check` passaram. UI está
  `STATICALLY VERIFIED`; compra do servidor está `AUTOMATED TESTED`.
- Após cobrir a segunda rota de labels e o dispatch do clique esquerdo, o build
  oficial Release Win32 v145 passou com 0 erros e 6 warnings preexistentes. O
  output foi instalado em `tmproject/client748/project.exe`, SHA-256
  `E077183BA1E2110005B8E5190EAA188BBF48FF7893C92D66BC9BFA0941F3B4F9`.
  `go test -count=1 ./internal/game ./internal/wire`, `go vet ./...`,
  `Test-Client748Assets.ps1` e `git diff --check` passaram. Este fluxo segue
  `STATICALLY VERIFIED` até o teste real de placa e compra.
- O usuário confirmou no candidato `E077183B...F3B4F9` que a compra funciona,
  mas mostrou duas regressões: ausência da confirmação e placa vazia congelada
  após comprar o último item. `TradeItem()` voltou a abrir o message box nativo
  `646`, cujo callback existente envia a compra. `OnPacketRemoveMob` agora limpa
  e oculta imediatamente o título/painel no `RemoveType 0`, e `FrameMove` não
  reexibe overlays de atores em `DelayDelete` ou já deletados.
- O build Release Win32 v145 passou e instalou o novo candidato em
  `tmproject/client748/project.exe`, SHA-256
  `4BC9C8EC95DEBF66D751F5F29C4B8F2401133E5C4FAC66C708ADCA46A67E1472`.
  `Test-Client748Assets.ps1`, `go test -count=1 ./internal/game
  ./internal/wire`, `go vet ./...` e `git diff --check` passaram. As duas
  correções estão `STATICALLY VERIFIED` e aguardam o fluxo real.
- As capturas seguintes mostraram que a compra ainda usava a caixa clássica de
  madeira e que `NewUI_AutoTrade_BG` aparecia sem o título. Ghidra 7.48
  (`FUN_00403eb8`) confirma a composição NewUI com texture sets `164/165`, e
  `FUN_004f7ea6` confirma a inserção do texto antes do painel `446`.
- `SMessageBox` agora seleciona a composição NewUI para o tipo
  `TMC_MESSAGEBOX_MESSAGE` usado pela confirmação Sim/Não da AutoTrade, sem
  mudar `g_UIVer` global nem a geometria/hit-test do HUD clássico.
- `TMHuman::CreateControl()` voltou a adicionar `m_pAutoTradeDesc` ao container
  e mantém a ordem nativa texto/painel também após recriar os controles. O
  construtor principal já usava essa ordem e foi documentado junto ao código.
- O build oficial Release Win32 v145 passou com 0 erros e 6 warnings
  preexistentes. `build/Release/WYD.exe` e `tmproject/client748/project.exe` são idênticos,
  SHA-256 `7BA846DAE559464491739B104903CE4E843BC57C8AED57EF00E368D5F7E27171`.
  `Test-Client748Assets.ps1`, `go test -count=1 ./internal/game
  ./internal/wire`, `go vet ./...` e `git diff --check` passaram. O ajuste está
  `STATICALLY VERIFIED` até a confirmação visual no jogo.

## Geometria estável do Cargo 7.48

- O usuário mostrou que Cargo e Carry abriam em posições diferentes conforme a
  janela usada anteriormente. No ramo compatível, `SetVisibleCargo` apenas
  alterava visibilidade e conservava coordenadas residuais de AutoTrade/loja.
- Ghidra 7.48 reconfirmou `FUN_004484f3` como o toggle nativo de Cargo/Carry;
  o stock depende da geometria original do recurso, enquanto a source precisa
  reaplicá-la porque compartilha os painéis com outros fluxos.
- Ao abrir, o Cargo agora é centralizado pela largura real da tela e do próprio
  painel. O Carry recebe a mesma baseline `35` e abre imediatamente à direita,
  separado pelo gap nativo escalado de aproximadamente `24.4`; as dimensões dos
  painéis já estão escaladas e não são multiplicadas novamente.
- O build oficial Release Win32 v145 passou com zero erros e instalou o output
  em `tmproject/client748/project.exe`, SHA-256
  `7D203FE6A1B8DB99BF320BC3D2DA020435BB83E537C052EC374A57F09A2BB71D`.
  `Test-Client748Assets.ps1`, `go test -count=1 ./internal/game
  ./internal/wire`, `go vet ./...` e `git diff --check` passaram. A correção
  está `STATICALLY VERIFIED` e aguarda confirmação visual em jogo.

## Posição e persistência finais da placa AutoTrade

- As capturas de 2026-08-27 confirmaram dois desvios no candidato instalado
  `7BA846DA...27171`: o título ficava acima da faixa escura e o conjunto só
  aparecia durante hover.
- Ghidra 7.48 reconfirmou `FUN_004f7ea6` para criação/inserção e
  `FUN_00504a80` para visibilidade/posição. No hash histórico
  `8AA2F918...5F593`, o texto usa âncora horizontal `140` em `baseY`, enquanto
  `NewUI_AutoTrade_BG` usa `150` em `baseY - 13 * heightRatio`; a visibilidade
  é governada por `TradeDesc[0]`.
- `TMHuman::LabelPosition()` e `LabelPosition2()` agora preservam atores com
  `TradeDesc` no cull sem hover e usam as duas âncoras nativas separadas. As
  ocultações legítimas por ator removido, profundidade, sombra e fora da tela
  permanecem intactas.
- O build oficial Release Win32 v145 foi repetido após o encerramento do client
  e instalou output idêntico em `tmproject/client748/project.exe`, SHA-256
  `CAF93919F4B1CBC9CE8EED10B4BFE56E860FF78978F20A31473EDDF35F79B8C4`.
  `Test-Client748Assets.ps1`, `go vet ./...`, `gofmt -l` e
  `git diff --check` passaram. `internal/game`, `internal/wire` e
  `internal/store` passaram com `-count=1`; `internal/data` conserva somente
  as duas falhas conhecidas pelos assets ausentes `Mounts-KR.json` e
  `Costumes-KR.json`. A correção permanece `STATICALLY VERIFIED` até o teste
  visual no candidato instalado.

## Crash ao abrir o NPC de habilidades em 2026-08-27

- O fluxo real de abrir o NPC de habilidades falhou no client e gerou
  `tmproject/client748/client-crash-20260827-140525.dmp`, com 62.464.379 bytes e horário
  local 2026-08-27 14:05:25. O dump não integra o Git.
- O dump/PDB resolve a falha para `TMFieldScene::SetVisibleSkillMaster`, no
  acesso `m_pHellgateStore->SetVisible(0)`: `m_pHellgateStore == nullptr`. O
  acesso seguinte a `m_pGambleStore` também dependia de um controle opcional.
- `FUN_00435b13`, `FUN_004875c0`, `FUN_0044c15c`, `FUN_0044c53f` e
  `FUN_0044df53` confirmam que o 7.48 abre conjuntamente os roots 1889 e 1905,
  enquanto Hellgate/Gamble herdados podem não ser materializados pelo layout.
- `SetVisibleSkillMaster` agora exige os dois roots nativos, protege todos os
  painéis concorrentes opcionais e preserva posição, som e o fechamento por
  X/Esc. Nenhum widget 7.59 foi fabricado.
- `Build-Client.ps1` passou com 13 warnings C4018 preexistentes e zero erros,
  instalando output idêntico em `tmproject/client748/project.exe`, SHA-256
  `F8251714775601720307940598522E6D2924E5C61DAB300728F949FE0C8A380B`.
  `Test-Client748Assets.ps1` passou com 6.500 itens, 3.584 texturas, 104 skills
  e 18 shaders; `git diff --check` não encontrou erro de whitespace.
- Estado: `STATICALLY VERIFIED`. Ainda é obrigatório testar no client abertura,
  X, reabertura, Esc, nova interação e clique nas habilidades.

## Compra de skill no mestre em 2026-09-01

- A ficha `flows/ui/skill-master-purchase.md` fechou em `CONTRACT` o fluxo
  nativo desde a abertura dos roots 1889/1905 até `MSG_ApplyBonus` (`0x277`,
  20 bytes), incluindo callers/callees, ownership, teardown e relogin.
- `FUN_0040D13E` e `FUN_0040DD00` confirmam que itens sem mesh usam renderer
  2D, atlas 199 na UI2 ou 1 no legado, textura do catálogo e origem da célula.
  Na UI2, itens 5000–5102 usam caixa 23×23 por dimensão lógica e os demais
  sprites usam 32×32; meshes e UI legada preservam 24×24. `SGridControlItem`
  agora preserva esse caminho sem alterar a centralização dos itens 3D.
- `FieldScene2.bin` confirma que a grade é 1894 (`type=16`, parent 1889,
  `31,63`, `191x241`, `8x4`); 6128 é o texto `TML_SKILLM_DESC`.
- `FUN_0044C15C` centraliza root 1889 e posiciona root 1905 à direita em
  `centerX + skillWidth*0.5 + 10`. O toggle recompilável agora abre e oculta os
  dois roots simetricamente e restaura 1905 ao centro no fechamento.
- `FUN_004640E5`, via caller `FUN_004662C5`, confirma `BonusType=2`, o item em
  `Detail` e o mestre em `TargetID`. `MouseClick_SkillMasterNPC` agora conserva
  esse mestre em `m_sShopTarget` para o pacote de confirmação.
- O teste byte-level cobre sucesso, target divergente, faixa e catálogo de
  venda, persistência, rollback e ausência de publicação em falha.
- `go test -count=1 ./...`, `go vet ./...`, o validador de pesquisa,
  `git diff --check` e `Build-Client.ps1` passaram. O build Win32 terminou com
  zero erros e 15 warnings C4018 preexistentes, instalando
  `tmproject/client748/project.exe` com SHA-256
  `85FC6B2541784C4AF83A275B5614FD74B8990A303A6618AEC21DFBB02FE602D2`.
- Estado máximo: `CONTRACT`. O fluxo real de abrir, renderizar, cancelar,
  confirmar, rejeitar, fechar, reabrir e relogar ainda não foi executado.
- Follow-up visual: caixas UI2 `23×23` para itens 5000–5102, `32×32` para os
  demais sprites e composição/fechamento nativos dos roots 1889/1905 foram
  recompilados. Validador, `git diff --check` e build Win32 v145 passaram; zero
  erros, 15 warnings C4018 preexistentes. Candidato instalado:
  `9DE77CE65766703FADA3FEDA006388D907E5E53A55D5DFFC90669AB84931004F`.
  Continua `CONTRACT`, não `CLIENT-TESTED`.

## Paridade IME em 2026-09-01

- A ficha `flows/ui/control-focus-ime-lifecycle.md` avançou para `TRACED` após
  fechar o dispatcher `FUN_0055DAB8`, os handlers `FUN_004AF545`,
  `FUN_004AF550`, `FUN_004AF5EB` e `FUN_004AF5F6`, o receptor editável
  `FUN_00406F5E`, ownership e teardown de troca de cena/logout.
- A source lê composição com `GCS_COMPSTR`, entrega texto limitado ao
  `SEditableText` focado e reproduz a página candidata agregada no primeiro
  controle. Tamanhos, tabela de offsets, strings terminadas e alocação da
  `CANDIDATELIST` são validados antes da publicação.
- `WM_IME_NOTIFY` agora abre, muda e fecha candidatos; fim de composição,
  teardown da árvore e controles opcionais são protegidos sem fabricar widgets
  posteriores ausentes do recurso 7.48.
- `validate_research.py`, `git diff --check` e o build oficial Release Win32
  v145 passaram. O build terminou com zero erros e dois warnings C4305/C4309
  preexistentes, instalando `tmproject/client748/project.exe` com SHA-256
  `1DF5956AC134BCAEB5C072E84B77EF9BBDFF6EDE30DAC8ACBE8616375CED6082`.
- Estado: `TRACED` e `STATICALLY VERIFIED`; composição, candidatos, foco,
  troca de cena e relogin ainda não foram executados no client real.

## Premium Firework em 2026-09-01

- A ficha `flows/ui/premium-firework-display.md` fechou em `CONTRACT` a
  apresentação nativa do desenho Premium Firework, incluindo dispatcher,
  callers/callees, bitmap, render, ownership, expiração, teardown, shutdown e
  relogin.
- O fluxo começa na intenção C->S `0x3C9` de 52 bytes, com bitmap no offset
  34. O servidor valida item, posição, catálogo, bits reservados e cooldown,
  persiste o consumo e só então publica `0x3CA` ao dono e observadores visíveis.
- O pacote S->C `0x3CA` possui 36 bytes e bitmap no offset 20. A source agora
  despacha o pacote tipado e cria o efeito `6` sobre o humano indicado pelo
  header, usando a posição corrente dessa entidade.
- `TMEffectFirework` restaura a grade 10x10 LSB-first, a forma mínima para
  bitmap vazio, cem partículas, sons `315/316`, textura `(7, 360000)`, duração
  de 10 segundos e as duas passagens de render nativas.
- O effect container/ObjectManager possui o nó depois da inserção; a expiração
  normal, troca de cena, logout e shutdown eliminam o efeito sem estado
  persistente entre cenas.
- Asserts Win32 verificam tamanho/offset do pacote e layout usado pelo efeito.
  Os testes Go cobrem wire byte-level, consumo, persistência, rollback,
  validações, cooldown, dono/observers e exclusão de outsiders.
- `Build-Client.ps1` passou em Release Win32 v145 com zero erros e instalou
  `tmproject/client748/project.exe` com SHA-256
  `76B3E66EAC6E17EB615B80B7CAE7F900BA8D836CC89A8874BDD3C9D59A088ECF`.
- Estado: `IMPLEMENTED`, `STATICALLY VERIFIED` e `AUTOMATED TESTED`. O fluxo
  ainda não foi executado no client real e não é `CLIENT-TESTED`.

## Paridade estática: `SetMyHumanMagic`

- `TMFieldScene::SetMyHumanMagic()` permanece vazio por paridade com
  `FUN_004431D9`, no-op nativo de 11 bytes sem callees.
- Os callers diretos são `FUN_004431E4`, `FUN_004528C5` e `FUN_00454763`.
- Não restaurar o sidecar `STRUCT_MOB::Magician`; o cálculo ativo usa
  `CurrentScore.MagicAmp` como representação canônica.
- Evidência: `inventory/set-my-human-magic-noop.md`. Estado do símbolo:
  `STATICALLY_EVIDENCED`/`PARITY_STATIC_EVIDENCE`, sem claim runtime.

## Input e extensões dormentes em 2026-09-01

- `FUN_004AF910` confirma que a liberação do botão esquerdo é no-op; a source
  preserva o corpo vazio e não inventa `WM_LBUTTONUP`.
- Mission foi preservada como extensão manual, mas o botão `86019` não simula
  mais sucesso. `MSG_Mission` tem 88 bytes e permanece sem opcode; `0x3C0`
  continua exclusivo ao Tiny nativo de 84 bytes.
- `SetAutoSkillNum()` e `SetAutoTarget()` toleram o painel moderno `65648` e
  seus dez filhos ausentes, sem materializar widgets posteriores.
- Cabeça `57` continua classificada e protegida como merchant. O stub Premium
  não envia packet até existir evidência nativa ou contrato coordenado.
- Fichas: `left-mouse-release-noop.md`, `mission-panel-dormant.md` e
  `premium-npc-click-dormant.md`. Validador e `git diff --check` passaram.
- Build Release/Win32: zero erros, 13 warnings C4018 preexistentes; candidato
  `87431F0B066FD782CE1231F1E76C1905671E8C0D2C23E9CA75EDD4DC351F9979`.

## Consulta do array de animação em 2026-09-01

- A ficha `flows/transport/bone-animation-array-probe.md` fechou em `CONTRACT`
  o fluxo nativo `0x1C1` S->C para `0x2C2` C->S, ambos com 24 bytes.
- `FUN_00492E7D`, `FUN_0055890A`, `FUN_004927DD`, `FUN_004B00E4` e
  `FUN_0055F2DD` confirmam dispatcher, gate de tamanho, handler, loader `.bon`
  e sender. Categoria e offset inválidos são clampados somente para a leitura;
  o eco preserva os campos originais e promove o byte como `int8_t` assinado.
- A source tipou o packet e seus offsets, restaurou a resposta com o ID do
  humano local e passou a conservar o tamanho bruto exato do `.bon`. Isso evita
  truncar `ed.bon` e `tn.bon`, cujos tamanhos não são múltiplos de oito.
- O servidor agora carrega um manifesto versionado, escolhe um probe depois da
  entrada no mundo, emite `0x1C1` e consome `0x2C2`. Mantém um único pending
  efêmero por jogador e fecha imediatamente em replay, framing/fase inválidos,
  ID/conteúdo divergente ou resposta no/após o deadline; o tick cobre ausência.
- Wire/layout permanecem `PARIDADE_NATIVA`; manifesto, seleção, prazo e política
  fail-closed são `EXTENSAO_COORDENADA`, sem claim criptográfico do client.
- `Build-Client.ps1` passou em Release Win32 v145 com zero erros e 31 warnings
  preexistentes. `validate_research.py` e `git diff --check` passaram; o
  candidato instalado tem SHA-256
  `DA9F578E6AEF2A6F2ED923E893F412717F7966AC861A21F3A17D939EDF70EE3F`.
- Testes Go cobrem layout/parser, manifesto, cópia/seleção, pending único,
  sucesso, replay, mismatch, timeout, gate, fase e cleanup no reset.
- Estado: `CONTRACT`, `STATICALLY VERIFIED` e `AUTOMATED TESTED`; o fluxo ainda
  não foi executado no client real e não é `CLIENT-TESTED`.

## Extensão de avisos indexados em 2026-09-01

- `0x105/0x106` não existem nos dispatchers nativos 7.48 focalizados;
  `FUN_0049889A` trata somente `0x101`, `0x102` e `0x104` para `ID=0`.
- O lote foi classificado como `EXTENSAO_COORDENADA`, não paridade. A ficha
  `flows/ui/indexed-parameterized-message-extension.md` registra o frame de
  108 bytes, índice `int16` relativo a `+1000`, CSV de até seis parâmetros,
  fallback, ownership e teardown.
- `TMScene` agora exige tamanho exato, seletor e painel válidos, faz bounds
  check da tabela e substitui somente `%s`/`%%` sem usar o asset como format
  string. Os stubs por entidade permanecem deliberadamente fora do contrato.
- O WYD-Go possui builders byte-level para os dois opcodes, com `ID=0`, CSV de
  91 bytes e vírgula/NUL interno normalizado. O convite de grupo usa
  `MessageParameterized(-938, nome do líder)` e gold insuficiente na loja
  fantasma usa `MessageIndexed(-845)`; outros erros preservam o painel legado.
- `go test -count=1 ./internal/wire ./internal/game`, o validador de pesquisa e
  `Build-Client.ps1` passaram. O build `Release|Win32` v145 terminou com zero
  erros e 31 warnings preexistentes, instalando `tmproject/client748/project.exe` com
  SHA-256
  `9E225456063C5DC77917C007FDCA9ECD05DDC9312FD25D9AB28FE55F334B5BF4`.
- Estado: `IMPLEMENTED`, `AUTOMATED TESTED` no wire/gameplay e
  `STATICALLY VERIFIED` no client. Sem execução real, não é `CLIENT-TESTED`.

## Baseline visual 800x600 da UI em 2026-09-01

- A comparação do TMProject em aproximadamente 1280x960 com o client original
  em 800x600 mostrou compressão horizontal sistemática no HUD superior,
  buffs, textos, controles, HUD inferior e placas sobre personagens.
- A causa era uma baseline dividida: `BASE_ScreenResize()` e os quatro roots
  inferiores de `FieldScene2.bin` já fechavam exatamente 800 pixels, mas
  `RenderDevice`, `SControl`, clipping e `SProgressBar` calculavam a razão
  horizontal contra 965. Em 1024 pixels, os roots 5739/5716/5745/5744 medem na
  origem 311/185/264/40 e totalizam 800, confirmando a largura do recurso.
- `Basedef.h` agora centraliza `WYD748_UI_BASE_WIDTH=800` e
  `WYD748_UI_BASE_HEIGHT=600`; criação, autosize, centralização, clipping e
  progress bars usam a mesma base. `m_dwCurrScreenX=965` foi preservado porque
  guarda/restaura o modo físico e é sobrescrito por `GetDeviceCaps`, não define
  coordenadas da UI.
- O remendo que esticava o root 5716, fundo 5717, labels EXP/ATT/DEF, barra EXP,
  HOLD e ornamentos foi removido. Os roots inferiores preservam dimensões do
  recurso e recebem somente as ancoragens de viewport: chat à esquerda,
  status ao rodapé, atalhos antes do menu e menu à direita.
- As barras sobre personagens continuam na ordem correta
  `barras -> nome -> personagem`. A cor superior roxa/azulada ainda difere do
  vermelho/laranja original, mas não foi alterada neste lote porque os dois
  caminhos de `TMHuman::LabelPosition` usam paletas diferentes e a evidência
  nativa da seleção ainda não está fechada.
- Classificação: `PARIDADE_NATIVA`; validação estática/build não equivalem a
  teste visual. Gerar novo print em 1280x960 com o candidato recompilado antes
  de promover qualquer parte deste ajuste para `CLIENT-TESTED`.
- `Build-Client.ps1` passou em `Release|Win32` v145 com zero erros e 31 warnings
  preexistentes. O candidato foi instalado em `tmproject/client748/project.exe` com
  SHA-256 `66F17B2CE195035122CBAC8C07C5907E5CAF88B4102CE82E22DE005835824FB0`.

## Composição lado a lado de Character, Skill e Inventory em 2026-09-01

- As capturas do candidato mostraram os painéis Character, Skill e Inventory
  sobrepostos; a referência original mostrou a ordem lado a lado
  `Character -> Skill -> Inventory`.
- `FieldScene2.bin` serializa intencionalmente os três roots na mesma posição
  `(530,0)`, todos com `227x421`: Character `513`, Skill `1905` e Inventory
  `257`. Portanto, mover roots no asset não reproduziria o lifecycle nativo.
- `FUN_00435b13` recompõe as posições depois do carregamento. As constantes do
  executável são `0.5f`, `1.5f` e `10.0f`; em `800x600`, os resultados exatos
  são Character `(49.5,89.5)`, Skill `(286.5,89.5)` e Inventory
  `(523.5,89.5)`.
- `TMFieldScene::PositionCompatFeaturePanels()` aplica essas fórmulas usando o
  viewport atual e as dimensões materializadas. A chamada ocorre uma vez na
  construção compatível, após os três bindings, para não apagar movimento
  manual ao fechar/reabrir uma janela.
- A ficha completa está em
  `.agents/research/client748/flows/ui/feature-panel-layout.md`.
- Classificação: `PARIDADE_NATIVA`. O validador de pesquisa, `git diff --check`
  e o build `Release|Win32` v145 passaram; foram zero erros e 21 warnings
  preexistentes. O pipeline instalou `tmproject/client748/project.exe` com SHA-256
  `CD92A005EBDAB0DF21D9BF5B1CB1C1FC593F048BB9011802B67D2F71539BF40B`.
  Estado: `STATICALLY VERIFIED`; promover para `CLIENT-TESTED` somente após
  teste visual, reabertura e relogin.

## Posição do painel de seleção de personagem em 2026-09-01

- A captura em `1280x960` mostrou o controle de seleção iniciado perto de
  `x=1040` e cortado pela borda direita. A causa era
  `VisibleSelectCreate()` multiplicar os literais `(650,40)` pela nova razão
  correta de `800x600`; com largura renderizada `398.4`, o limite direito
  chegava a `1438.4`.
- `FUN_0049F0E7` carrega `SelCharScene2`, resolve `0x502`/`1282` e chama o slot
  virtual `+0x68`. A vtable `0x005A44B4`, slot `0x005A451C`, resolve para
  `FUN_004A250F`.
- No branch responsivo, `FUN_004A250F` usa
  `x=W*0.75-PW*0.5` e `y=H*0.5-PH*0.5`. Em `1280x960`, com controle
  `398.4x592`, o resultado é `(760.8,184)`, inteiramente dentro do viewport.
- `SControl` já materializa `m_nWidth/m_nHeight` escalados. A source agora usa
  essas dimensões diretamente; botões, câmeras, visibilidade, asset, wire e
  servidor não foram alterados.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/select-character-layout.md` e cobre
  callers, callees, vtable, create/return, teardown e logout/relogin.
- O validador, `git diff --check` e o build `Release|Win32` v145 passaram com
  zero erros e zero warnings. O candidato instalado tem SHA-256
  `C6184EBD938BC6120539958D049C1538DD3A27A0972108B635531A23E3C19254`.
  Estado: `STATICALLY VERIFIED`, ainda não `CLIENT-TESTED`.

## Caixa visual e escala de itens em grids em 2026-09-02

- As capturas mostraram itens pequenos dentro da loja e do inventário. A
  hipótese anterior de contenção pela diagonal do AABB, com limite `0.27` e
  exceção para `egg001..egg014`, foi refutada pela reabertura do fluxo nativo.
- `FUN_0040d13e` materializa caixa lógica `24x24` para mesh/UI legada,
  `23x23` para sprites UI2 `5000..5102` e `32x32` para os demais sprites UI2.
  `FUN_0040e6aa` e `FUN_0040e817`, slots `+0x8C/+0x90` da vtable
  `0x005A4024`, leem somente `mesh+0x354` (`MaxZ`) e aplicam
  `min(1, itemCellHeight * 0.3 / MaxZ)`.
- `SGrid.cpp` agora aplica essa fórmula a todos os grids e ao drag, sem branch
  por família, preserva equipamento em `1.0` e materializa as caixas
  `23/24/32` no mesmo espaço escalado dos slots. Item e `GeomControl` recebem
  a mesma geometria, inclusive no Skill Master.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/grid-item-mesh-scale.md`, com xrefs,
  dispatches virtuais, lifecycle, teardown e relogin. Estado de entrega:
  `STATICALLY VERIFIED`; build passou, mas o teste visual segue pendente.

## Composição lado a lado de Shop e Inventory em 2026-09-02

- A captura do candidato mostrou a loja de NPC deslocada e o Inventory fora da
  composição; a referência original mostra Shop `1793` centralizada e
  Inventory `257` à direita.
- `FUN_00435b13` usa
  `Shop.x=W*0.5-Shop.width*0.5`, `Shop.y=H*0.5-Shop.height*0.5`,
  `Inventory.x=W*0.5+Inventory.width*0.5+10` e
  `Inventory.y=H*0.5-Inventory.height*0.5`. `FUN_004481c5` abre a dupla,
  fecha AutoTrade e Trade concorrentes e restaura o modo do grid no fechamento.
- `TMFieldScene::PositionCompatShopPanels()` reaplica a composição na abertura,
  pois AutoTrade e outras features movem o mesmo root de Inventory.
  `SetVisibleShop()` também fecha AutoTrade antes de expor Shop + Inventory.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/shop-inventory-layout.md`. Estado de
  entrega: `STATICALLY VERIFIED`; falta validação visual em três resoluções.

## Composição lado a lado de Trade e Inventory em 2026-09-02

- O root Trade `576` corresponde ao membro nativo `this+0x285F0`; Inventory
  `257` corresponde a `this+0x27B34`. `FUN_00435b13` centraliza Trade e coloca
  Inventory à direita com o mesmo gap lógico de dez pixels das outras duplas.
- `FUN_0044b890` é o toggle de Trade: possui treze callers únicos, fecha
  AutoTrade e painéis concorrentes, mostra Trade + Inventory, usa modo de grid
  `7` durante a negociação e limpa highlights, buffers e controles temporários
  ao fechar.
- `TMFieldScene::PositionCompatTradePanels()` reaplica a fórmula ao abrir,
  impedindo que o Inventory compartilhado herde a posição de Shop, Cargo ou
  AutoTrade. A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/trade-inventory-layout.md`.
- Estado da source: `IMPLEMENTED`; validação estática e novo build do lote ainda
  estão pendentes, portanto não é `CLIENT-TESTED`.

## Posição inicial e toggle do menu inferior direito em 2026-09-02

- O root do menu `292` não usa a posição serializada como posição final.
  `FUN_00435b13` o coloca em `x=resourceX-2` e
  `y=button5744.y-menu.height`, iniciando oculto.
- `FUN_004662c5`, branch `0x1670`, alterna a visibilidade e mantém sincronizada
  a seleção do botão. A source aplica os mesmos anchors depois de materializar
  o HUD e preserva o lifecycle de toggle, fechamento de cena e relogin.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/system-menu-initial-layout.md`. Estado
  de entrega: `STATICALLY VERIFIED`; falta validação visual/click em jogo.

## Posição residual do Inventory após AutoTrade em 2026-09-02

- `FUN_0044AE38` posiciona explicitamente Cargo no branch vendedor, mas no
  branch cliente apenas oculta Cargo e mostra Inventory. Não há `SetPos` do
  root Inventory nesse caminho nem no toggle normal `FUN_00447691`.
- A source movia Inventory para `530x35` ao abrir a loja de outro personagem.
  Como o fechamento e o toggle normal alteram somente visibilidade, essa
  coordenada sobrevivia e deslocava as aberturas seguintes.
- `TMFieldScene::SetVisibleAutoTrade(1,0)` agora preserva a composição
  responsiva criada por `PositionCompatFeaturePanels()`. O branch vendedor
  continua posicionando Cargo como no nativo; nenhuma restauração foi colocada
  no fechamento para não sobrescrever a composição seguinte.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/auto-trade-inventory-layout.md`. Estado
  de entrega: `IMPLEMENTED`; validação estática, build e teste visual ainda
  precisam ser atualizados para o novo candidato.

## Composição dos seis ItemMix com Inventory em 2026-09-02

- `FUN_00435B13` vincula os roots ItemMix `1360`, `6110`, `6145`, `6432`,
  `6481` e `6512` em seis membros distintos, centraliza cada um pelo viewport
  e posiciona Inventory `257` à direita com gap lógico de dez pixels.
- Os toggles `FUN_00449384`, `FUN_00449632`, `FUN_004498E0`, `FUN_00449B8E`,
  `FUN_00449E3C` e `FUN_0044A0FB` convergem na cascata `FUN_00447691` e nos
  cleanups específicos `FUN_004487E2`, `FUN_004489C5`, `FUN_00448C38`,
  `FUN_00448E0B`, `FUN_00448FDE` e `FUN_004491B1`.
- A source chamava o compositor de painéis antes de vincular os seis ItemMix e
  não reaplicava posição no open. `PositionCompatNativeMixPanels()` agora roda
  após os bindings e em `SetVisibleNativeMix(1)`, eliminando a coordenada
  herdada de Shop, Trade ou AutoTrade sem alterar staging, receita ou wire.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/native-mix-inventory-layout.md`. Estado
  de entrega: `STATICALLY VERIFIED` no candidato `3EDB5818...64774`; falta
  apenas o teste visual e funcional dentro do jogo.

## Posição nativa do Gamble em 2026-09-02

- `FUN_00435B13` resolve o root `6400` e o posiciona em
  `centerX-panelWidth/2+10`, `centerY-panelHeight/2` antes de ocultá-lo.
- `TMFieldScene::PositionCompatGamblePanel()` aplica essa fórmula depois de
  materializar os reels e novamente em `SetVisibleGamble(1)`, descartando
  coordenada serializada ou residual sem alterar aposta, animação ou wire.
- A ficha `CONTRACT` está em
  `.agents/research/client748/flows/ui/gamble-jackpot.md`. Estado de entrega:
  `STATICALLY VERIFIED` no candidato `3EDB5818...64774`; falta abertura dos
  dois tipos, rolagem, fechamento e relogin dentro do jogo.

## Painel Party nativo em 2026-09-02

- `FUN_00435B13` vincula root `1857`, lista `1863` e botão `5742`, colocando
  root e botão em `x=0` e `y=viewportHeight-root.height-165`.
- `FUN_0044DA6F` centraliza o toggle e a seleção inversa do botão. Click
  `5742`, tecla `P`, packets, `Esc` e UI concorrente convergem nesse lifecycle.
- `FUN_004883BF`, `FUN_00488879` e `FUN_00488B52`, via dispatcher
  `FUN_00492E7D`, cobrem convite `0x37F`, inclusão `0x37D` e remoção `0x37E`;
  a lista `1863` mantém aceite `0x3AB`, `Ctrl`+click e click direito.
- A source agora vincula os controles nativos, reaplica a posição no open,
  aceita a lista nativa no callback e protege payload, lista, chat e AutoParty
  opcional. A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/party-panel-layout-lifecycle.md`.
- Estado de entrega: `IMPLEMENTED / STATICALLY VERIFIED`; o build oficial
  passou e instalou o candidato `B51D48AC...9AE6BC`. Teste real ainda
  pendente, sem claim `CLIENT-TESTED`.

## Painel Quest nativo em 2026-09-02

- `FUN_00435B13` chama `FUN_00441823`, que vincula toggle `315`, root `320`,
  os quatro pares lista/conteúdo `321/322`, `325/326`, `327/328`, `335/334`,
  fechar `323`, título `324`, abas `329/330/331/333` e memo `332`.
- O root recebe centro exato em X/Y. `FUN_004662C5` centraliza toggle, abas,
  seleções, memo e fechamento; `FUN_0049E50F` carrega conteúdo somente quando
  o destino é válido.
- A source agora vincula o grupo nativo completo, reaplica a posição no open,
  recarrega as quatro listas, compartilha helpers entre click, tecla `X`,
  fechar, `Esc` e AirMove e protege controles opcionais. O callback aceita
  tanto `65793` quanto o ID nativo `315`; isso corrige o atalho `X` no modo
  compatível.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/quest-panel-layout-lifecycle.md`.
- Estado de entrega: `IMPLEMENTED / STATICALLY VERIFIED`; o build oficial
  passou sem o warning C4805 e instalou o candidato
  `B51D48AC...9AE6BC`. Teste real ainda pendente, sem claim
  `CLIENT-TESTED`.

## Campos Character nativos em 2026-09-02

- `FUN_00435B13` vincula o grupo Character e `FUN_004431E4` atualiza os
  controles em construção, Score, equipamento e skill. Os IDs relevantes são
  Att Speed `1110`, HOLD `1168`, C.POINT `1376` e Kingdom `1377`.
- A source agora publica Att Speed em `1110`, mantém o texto C.POINT em `1376`,
  oculta HOLD quando `m_nFakeExp == 0` e oculta Kingdom quando não existe manto
  de reino válido. O binding moderno `65768` não sobrescreve mais o emblema
  nativo no modo 7.48.
- O servidor agora mantém `Char.Hold` como dívida persistente de EXP, publica
  o valor em `MSG_UpdateEtc::Hold`, `MSG_CNFMobKill::FakeExp` e
  `Ext1.Data[0]`. Morte PvP cria a dívida, EXP de combate a paga primeiro e
  CP/Chaos continua completamente separado.
- O lifecycle e o contrato wire estão registrados em
  `.agents/research/client748/flows/combat/pvp-death-held-exp-lifecycle.md`.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/character-stat-fields-update.md`.
- Estado de entrega: `IMPLEMENTED / STATICALLY VERIFIED`; o build oficial
  passou com zero erros e 17 warnings C4018 e instalou o candidato
  `A09722FF...5377BE`. Teste real ainda pendente, sem claim
  `CLIENT-TESTED`.

## Tela inicial e seleção de servidor em 2026-09-02

- `FUN_004A8F14`, initializer virtual `+0x4C` da cena `7`, centraliza o root
  nativo `4622` exatamente em X/Y. A source o traduz para `P_SERVER_SEL`
  (`65537`).
- A source já aplicava essa fórmula, mas adicionava `75.0f` a `m_nPosY`; esse
  offset ausente no nativo deslocava Server/Channel e todos os seus filhos para
  baixo.
- `TMSelectServerScene::InitializeUI` agora conserva somente a centralização
  nativa. Largura de canal, offsets dos títulos, branches dos logos, listas,
  callbacks e wire permaneceram inalterados.
- A ficha `TRACED` está em
  `.agents/research/client748/flows/ui/server-selection-layout-lifecycle.md`.
- Estado de entrega: `IMPLEMENTED / STATICALLY VERIFIED`; o validador, o
  `diff --check` e o build oficial passaram com zero erros e zero warnings. O
  candidato instalado é `6E6AF9A8...9A380BC`; falta o teste real, sem claim
  `CLIENT-TESTED`.

## PK Mode autoritativo em 2026-09-02

- O contrato nativo `0x399`, o input `K` e o lifecycle completo estão na ficha
  `.agents/research/client748/flows/ui/pk-mode-toggle-lifecycle.md`, validada em
  `CONTRACT`.
- O client protege os controles PK moderno e legado opcionais sem interromper
  o toggle nem o envio. O WYD-Go agora exige `Player.PKMode` para ataque físico,
  skill e summon contra jogador, antes de consumir efeitos; PvE permanece livre.
- `validate_research.py`, `go test -count=1 ./...`, `gofmt -d` e
  `git diff --check` passaram. O build oficial `Release|Win32` concluiu com zero
  erros e 13 warnings C4018 e instalou o candidato
  `68844150...3F7457C`.
- Estado: `CONTRACT / IMPLEMENTED / AUTOMATED TESTED / STATICALLY VERIFIED`.
  Falta executar o fluxo real; não é `CLIENT_TESTED`.

## Held EXP por morte PvP em 2026-09-03

- `Char.Hold` agora e persistente; morte PvP cria a divida sem retirar EXP ou
  gold, e EXP de combate a paga antes da progressao. Quest/item nao amortizam.
- O limite e 10% do intervalo do nivel atual. Em 80%, somente o MaxHP efetivo
  cai pela metade; o recalc o restaura quando Hold volta abaixo do limiar.
- `UpdateEtc@12`, `CNFMobKill@12` e `EnterWorld Ext1.Data[0]@1264` transportam
  Hold. CP/Chaos permanece independente.
- A ficha `CONTRACT` esta em
  `.agents/research/client748/flows/combat/pvp-death-held-exp-lifecycle.md`.
- `go test -count=1 ./...`, o validador e `git diff --check` passaram. O build
  oficial instalou `A09722FF...5377BE` com zero erros e 17 warnings C4018.
  Falta o fluxo real; nao e `CLIENT_TESTED`.

## Pendências e riscos

- No candidato `A09722FF...5377BE`, pressionar `K`, confirmar ausência de
  crash, bloqueio de PvP desligado, liberação ligado, PvE preservado e reset
  para desligado depois de logout/relogin.
- No mesmo candidato, morrer em PvP, confirmar Hold sem perda imediata de
  EXP/gold, pagar a divida matando mobs e validar o limiar/restauracao de MaxHP,
  CP independente e persistencia depois de logout/relogin.

- No candidato `DA9F578E...EE3F`, enviar desenhos Premium Firework com mais
  de um padrão e o fallback vazio; confirmar a mesma forma para dono e
  observador, nenhuma publicação para outsider, sons `315/316`, as duas
  passagens visuais, expiração após 10 segundos e ausência de efeito stale.
  Repetir durante troca de cena e depois de logout/relogin; até concluir esse
  fluxo, não promover a implementação para `CLIENT-TESTED`.
- No candidato `7D203FE6...A2BB71D`, abrir primeiro AutoTrade/loja e depois o
  banqueiro para comprovar que o Cargo sempre volta ao centro e o Carry abre à
  direita, com topo e espaçamento simétricos. Repetir em 1024x768 e 1280x960.
- No candidato `4BC9C8E...E1472`, criar uma loja com título e confirmar que
  `NewUI_AutoTrade_BG` fica visível junto do texto, acompanha o Carbunkle e
  desaparece em personagens sem loja. Em outro personagem, abrir os itens,
  confirmar fundo normal e preço abaixo de cada item; clicar, escolher `Não` e
  comprovar que nada foi comprado; depois escolher `Sim` e conferir os avisos
  de sucesso/gold insuficiente. Ao comprar o último item, o clone e a placa
  devem sumir sem congelar na tela nem acompanhar o comprador. Repetir depois
  de uma recriação de cena/relogin e validar o alinhamento lateral do Carry.
- Executar em jogo os fluxos de inventário, Cargo, NPC shop, equipamento e
  drag válido/inválido, comparando com screenshots 7.48 e verificando as caixas
  `23/24/32` e a fórmula uniforme por `MaxZ`.
- A entrada no mundo e a centralização de inventário/loja/equipamento já foram
  confirmadas no candidato `5A4AEC0A...`; ainda validar notices `!`, Kibita e
  digitação/backspace. A abertura do NPC Skill Apprentice falhou no candidato
  anterior; a correção e a compra no Skill Master agora devem ser testadas no
  candidato `1DF5956A...CED6082`.
- No candidato `1DF5956A...CED6082`, testar composição real, abertura/mudança/
  fechamento de candidatos, troca de foco e fim de composição. Repetir após
  troca de cena e logout/relogin; até isso, IME não é `CLIENT-TESTED`.
- Ao lado de um banqueiro, mover item inventário↔Cargo e conferir persistência,
  rejeição fora de alcance e atualização de gold/slots.
- Clicar para atacar um inimigo fora do alcance e confirmar aproximação até o
  range da arma, ataque e cancelamento seguro se o alvo morrer/desaparecer.
- Confirmar que `Steel_Pants`, uma armadura larga, uma arma longa, um ovo e um
  sprite pequeno ocupam a célula sem invadir vizinhas em inventário, Cargo,
  loja, trade/mix e drag; equipamento deve manter escala e posicionamento
  próprios sem regressão.
- Aplicar Armadura Crítica e buffs representativos com flags visuais (Haste,
  Magic Weapon, Shield, Critical e Poison); conferir efeito no avatar/observers,
  ícone 23×23 no topo, contagem de duração, hover e desaparecimento ao expirar.
- Confirmar que login, notices e a contagem após X usam o painel translúcido
  estreito, sem o fundo marrom alternativo.
- Confirmar highlight azul/vermelho completo em grid e equipamento.
- Validar EXP/ATT/DEF visualmente no HUD e após mudanças de score/skill/equip.
- Abrir C, S e I simultaneamente e confirmar a ordem lado a lado
  Character/Skill/Inventory, fechar/reabrir e repetir após logout/relogin e em
  resolução superior.
- Validar wheel, botão do meio, Alt+direito e contagem de cinco segundos após X.
- Percorrer o checklist de login, notice, servidor/canal, Character, venda,
  gold/preço, system menu, `X`, `Esc` e montarias KR.
- Testar cada artesão (Compositor, Aylin, Agatha, Tiny, Lindy, Odin e Ehre),
  incluindo staging, remoção, rejeição, sucesso, rollback e relogin.
- Não implementar `0x2C4` até recuperar uma receita e resposta autoritativas;
  ele é um modo dormente do ItemMix5, não uma UI livre.
- Testar em jogo o probe `0x1C1/0x2C2` com o manifesto ativo: sucesso deve
  preservar a sessão; byte divergente e timeout devem fechá-la. Repetir após
  logout/relogin; até lá, a consulta permanece sem `CLIENT_TESTED`.
- Testar em jogo uma recusa/rollback de `MSG_UseItem` e confirmar que origem e
  alvo voltam imediatamente; até isso, o conserto está `AUTOMATED TESTED`, não
  `CLIENT-TESTED`.
- Se houver crash, gerar minidump e aplicar o procedimento ASLR de
  `ghidra-client748.md`; não mascarar ponteiro nulo com widget 7.59.

## Próximo passo executável

Prioridade imediata: testar PK Mode e Held EXP no candidato
`A09722FFFE127DB9BC95132F1DA323D55794BFD0A3270FE40ACE0CCA735377BE` com dois
jogadores e um mob, incluindo morte PvP, pagamento da divida e logout/relogin.
Registrar o resultado nas duas fichas antes de promover para `CLIENT_TESTED`.

1. No candidato `DA9F578E...EE3F`, entrar no mundo com o manifesto de
   integridade ativo e confirmar sucesso do probe. Em ambiente de teste,
   alterar o byte esperado e reduzir o timeout para confirmar o fail-closed;
   repetir depois de logout/relogin e restaurar o manifesto versionado.
2. No mesmo candidato, convidar outro personagem para grupo e confirmar o aviso
   `-938` com o nome do líder; tentar comprar em loja fantasma sem gold e
   confirmar `-845`, sem compra ou estado stale após relogin.
3. No candidato `DA9F578E...EE3F`, executar o fluxo Premium Firework com
   dois padrões e o fallback vazio; observar dono, um observer e um outsider,
   sons, render, expiração, troca de cena e logout/relogin.
4. No candidato `DA9F578E...EE3F`, testar composição IME real, página de
   candidatos, troca de foco, fim da composição, troca de cena e relogin.
5. No mesmo candidato, abrir o NPC Skill Master e conferir os ícones dos
   livros; cancelar uma compra, confirmar outra e validar uma rejeição sem
   estado visual stale. Fechar por X/Esc e repetir depois de relogin.
6. Testar a placa sem hover e o título dentro da faixa escura no mesmo
   candidato.
7. Testar Cargo no mesmo candidato, abrindo antes e
   depois de AutoTrade/loja para excluir posições residuais, em 1024x768 e
   1280x960.
8. Testar a AutoTrade no mesmo candidato: adicionar o
   mesmo item, informar preço válido, repetir com preço inválido, conferir a
   placa `NewUI_AutoTrade_BG` junto do título e, ao abrir por outro personagem,
   conferir fundo/preço, confirmação `Não`/`Sim`, avisos, remoção imediata da
   placa após o último item e alinhamento do Carry.
9. No novo candidato, testar loja/inventário/Cargo/drag com `Steel_Pants`, uma
   armadura larga, uma arma longa, um ovo e um sprite pequeno. Confirmar a
   fórmula única por `MaxZ`, sem tratamento visual especial por família.
10. Depois do teste do NPC no novo candidato, repetir entrada no mundo,
   digitação/backspace, notice e Kibita; depois testar Cargo, autoaproximação,
   HUD, câmera e fechamento em 1280×960.
11. Atualizar esta matriz item a item; usar `CLIENT-TESTED` somente após o fluxo
   real correspondente.
12. No candidato `C6184EBD...C19254`, entrar na seleção em `1280x960`, abrir
    Create e retornar, confirmar posição/hitboxes, entrar no mundo e repetir
    depois de logout/relogin. O painel deve permanecer inteiro e centralizado
    no quarto direito.
13. No candidato `3EDB5818...64774`, abrir uma loja de NPC depois de AutoTrade,
    confirmar Shop centralizada e Inventory à direita com gap proporcional;
    abrir o menu inferior direito, conferir posição, seleção do botão, X/Esc e
    repetir após troca de cena e logout/relogin em `800x600`, `1024x768` e
    `1280x960`.
14. No mesmo candidato, testar Compositor, Aylin, Agatha, Tiny, Lindy, Odin e
    Ehre; em cada caso confirmar ItemMix centralizado, Inventory à direita,
    troca com Shop/Trade/AutoTrade e fechamento por botão e Esc. Repetir após
    logout/relogin nas três resoluções acima.
15. No mesmo candidato, abrir os dois tipos de Gamble depois de outras janelas
    terem sido movidas, confirmar centro vertical e deslocamento de dez pixels
    à direita, fechar por `Esc`, executar uma rolagem e repetir após relogin.
16. No candidato `B51D48AC...9AE6BC`, abrir Party pelo botão `5742` e pela tecla `P`, aceitar
    convite, adicionar/remover membro e testar click esquerdo, `Ctrl`+click e
    direito na lista. Fechar por `Esc` e UI concorrente; repetir após troca de
    cena e logout/relogin nas três resoluções de referência.
17. No candidato `B51D48AC...9AE6BC`, abrir Quest pelo botão `315` e pela tecla `X`, testar as
    quatro abas/listas, conteúdos e memo, fechar por `323`, `Esc` e AirMove e
    repetir após troca de cena e logout/relogin em `800x600`, `1024x768` e
    `1280x960`.
18. No candidato `ADFA0B99...10795`, abrir Character e confirmar Att Speed em
    `1110`, HOLD oculto com Hold zero, Held EXP com valor/percentual e Kingdom
    oculto para personagem sem reino. Repetir após atualização de Score,
    troca de equipamento e logout/relogin; testar também um personagem com
    manto de reino válido.

### Lote em andamento em 2026-09-03: chat não funcional no FieldScene compatível

- Causa localizada na source: o campo nativo 7.48 é `TME_CHAT=5123`, mas
  `OnControlEvent` aceitava somente `E_CHAT=65671`; envio, histórico, limpeza e
  IME nunca eram processados para a árvore compatível.
- Causa complementar na recepção: `TMHuman::OnPacketMessageChat` exige
  `m_pChatGeneral`; `InitializeCompatFieldScene` não o ligava, descartando o
  `0x333` antes da atualização da lista/balão.
- `TMFieldScene.cpp` agora liga painel `5739`, canais `5697..5704`, seleciona o
  geral e aceita `5123` apenas sob `m_bCompatFieldScene`. O wire `0x333/108`,
  servidor e fallback de party permanecem inalterados.
- Cliques nos controles `5697..5704` agora alternam juntos o botão do filtro e
  seu estado visual pareado para geral, grupo, private e guild.
- O mail `0x334` prefixado por `!` permanece no grupo Help nativo
  `864/867..875`: a lista `874` recebe remetente, horário e texto, o indicador
  `875` abre a quarta aba e o handler protege a chegada antecipada quando a
  lista ainda não existe. Nenhum controle posterior foi fabricado.
- `go test ./internal/game ./internal/wire` passou; `Build-Client.ps1
  -Configuration Release` passou com zero erros e instalou `tmproject/client748/project.exe`
  com SHA-256 `BA85D6CB23D88E3C56DE7A996D0A782E0888CE8819CAE23448A16334510D2277`.
- O teste com dois clients revelou outra causa independente: `onMessageChat`
  exigia o cache transitório `observer.Visible` e descartava a mensagem para um
  jogador próximo ainda não registrado nesse cache. A publicação agora usa
  proximidade espacial + sessão/em-mundo + `playersVisibleTogether`; um teste
  cobre cache vazio e impede vazamento para outro runtime nas mesmas coordenadas.
- Estado: `IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED TESTED`; ainda não há
  `CLIENT-TESTED`. Próximo passo é testar Enter, os quatro filtros, chat comum,
  comandos (`=`, `-`, `--`, `@`), whisper, mensagem recebida, party fora da
  tela, mail/quarta aba/fechamento e relogin.
- Ajuste posterior para o defeito específico de canal/cidadania: na
  inicialização compatível, os primários `5697..5700` agora começam ligados e
  os pares `5701..5704` começam desligados; o clique mantém `paired = !primary`.
  Isso é necessário porque `-`/`--` usam a gate `m_pChatGuild` no receptor e o
  estado duplicado anterior podia ocultar a mensagem do segundo client. A
  ficha `flows/ui/local-chat-message.md` registra a distinção entre os vários
  canais. Build e reteste de dois clients ainda pendentes.
- O roteamento server-side dos modos do `0x334` foi concluído: `--` global,
  `@@` cidadania, `@` reino, `-` guild e `=` party. `@@` compara
  `Char.Citizenship` não nulo; `@` compara o reino derivado da capa e rejeita
  reino neutro. O teste `TestChatHandlersRouteCitizenshipAndKingdomChannels`
  passou após corrigir o fixture: `545` e `545` são Hekalotia, enquanto `546`
  é Akelonia e `548` é neutra.
- Validação desta continuação: `go test -count=1 ./internal/game ./internal/wire`,
  `go test -count=1 ./...` e `git diff --check` passaram. A recepção visual
  com dois clients ainda precisa ser executada; portanto o estado permanece
  `IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED TESTED`, sem claim
  `CLIENT-TESTED`.

### Correção visual do chat — 2026-09-04

- O build oficial de Release foi concluído com zero erros e instalou
  `tmproject/client748/project.exe`; SHA-256:
  `2B6E9635F9B10A2BA1E0231D2FDCFA0120DA5A3A5753BDFF51A5C51EAD163A0F`.
- Em mensagens curtas, `OnPacketMessageWhisper` passou a inserir a linha
  normalizada `szMsg` com `dwColor`, preservando remetente/cor e ocultando os
  prefixos de roteamento.
- Em mensagens longas, `InsertInChatList` passou a iniciar a continuação em
  `maxLen - 1`, usar `dest[startId]` para a primeira linha e inserir a segunda
  linha somente quando `dest2` não estiver vazia.
- `go test -count=1 ./internal/game ./internal/wire` passou e `git diff
  --check` não encontrou erro. Warnings do compilador são preexistentes.
- Estado honesto: `IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED TESTED`;
  ainda não `CLIENT-TESTED`. É necessário testar com dois clients as mensagens
  curta/longa, local, grupo, guild, global/canal, cidadania, reino, whisper e
  mail, incluindo troca de cena, fechamento e relogin.

### Correção complementar do receptor de mensagens longas — 2026-09-04

- O caminho de recepção `OnPacketMessageWhisper` ainda possuía uma leitura
  incorreta de continuação em `MobName[maxLen + 15]`; ela foi substituída por
  `String[maxLen - 1]`, alinhada ao limite de quebra usado pelo texto.
- A continuação agora adiciona `ipNewItem2` somente quando não está vazia; o
  primeiro item não é mais duplicado.
- Essa é uma `MODERNIZACAO_COMPATIVEL`: somente a apresentação da mensagem
  foi corrigida. O frame `0x334/128`, prefixos, filtros, cor e ownership
  permanecem iguais.
- O build foi repetido após este ajuste, com zero erros e 13 warnings
  preexistentes. O estado permanece sem `CLIENT-TESTED` até o teste entre
  dois clients, incluindo mensagem longa.

### Correção do menu de interação Ctrl+clique direito — 2026-09-04

- Defeito confirmado no caminho compatível: `InitializeScene` retorna depois de
  `InitializeCompatFieldScene` quando falta o controle moderno `66817`; por
  isso os bindings nativos do menu root `640` não eram executados.
- O evento nativo `WM_RBUTTONUP` (`517`) com `MK_CONTROL` (`wParam & 8`) também
  não era tratado por `OnMouseEventCompat`, embora exista em
  `FUN_0055DAB8`. O evento agora valida AutoTrade/alvo e chama `PGTVisible`.
- `TMFieldScene.cpp` agora liga `640/645`, Party `641`, Guild `642`, Trade
  `643`, Challenge `620`, variantes `639/621/622/623`, opções Guild
  `816/817/862/863` e subopções `912..915`; o root é modal no slot 2 e
  centralizado no viewport.
- `PGTVisible` recebeu uma guarda de árvore incompleta para evitar crash em
  falha parcial do recurso. A alteração é `PARIDADE_NATIVA`, sem mudança de
  wire ou servidor. Ficha: `flows/ui/player-interaction-menu-lifecycle.md`.
- Estado desta correção: `IMPLEMENTED / STATICALLY VERIFIED`; build, testes e
  teste real com dois clients ainda pendentes. Não promover a `CLIENT-TESTED`
  antes de testar Party, Guild, Trade, Challenge, Cancel/Esc, troca de cena e
  logout/relogin.
- Continuação em 2026-09-04: corrigido o despacho do evento. O adaptador agora
  não entrega `WM_RBUTTONUP + MK_CONTROL` ao `SControlContainer` antes de
  tentar abrir o PGT oculto; quando o menu já está visível, a UI modal mantém
  prioridade. Build Release|Win32 passou (0 erros, 13 warnings preexistentes)
  e instalou `tmproject/client748/project.exe`, SHA-256
  `FA90D368E8EDE5F9D9E986263F13FAA0163A10DE8190C7B54C210AC59245B9C8`.
  Estado: `IMPLEMENTED / STATICALLY VERIFIED / AUTOMATED BUILD`; falta apenas
  o teste real com dois clients para `CLIENT-TESTED`.

### Continuação server-side do menu de interação — 2026-09-04

- O lote coordenado das ações Party/Trade/Challenge/Guild foi concluído no
  servidor. `0x39F` agora trata desafio com convite, aceitação, expiração,
  consumo único e validação de identidade/espaço; `0xE0E` trata declaração
  unilateral, resposta recíproca, cancelamento, líder, alvo e rollback de
  persistência. O client envia apenas intenção; `World` permanece autoritativo.
- A guerra de guild usa somente `Guild.WarTarget`, persistido no estado da
  guild. Não foram inventados neste lote zonas, torres ou cerco; esses fluxos
  exigem evidência própria.
- Regras, skill e protocolo de continuidade receberam uma guarda anti-loop:
  uma checagem curta por retomada, seguida imediatamente de patch/teste;
  duas chamadas sem alteração, teste ou evidência nova encerram o ciclo.
- Validação: `go test ./internal/wire`, `go test -count=1 ./...` e
  `git diff --check` passaram. Estado: `IMPLEMENTED / AUTOMATED TESTED` no
  servidor; o menu ainda não é `CLIENT-TESTED`.
- Próximo passo executável: teste manual com dois clients das ações Party,
  Guild, Trade e Challenge, incluindo cancelamento, troca de cena e
  logout/relogin. Não reabrir triagem ou listar chats para essa retomada.

### Cartas de guerra entre canais 4030/4031 — 2026-09-21

- Evidência nativa do `WYD.exe` canônico confirmou que 4030 abre o modal no
  modo 9 e 4031 no modo 10; `0xED7/0xED8` só são enviados depois da confirmação,
  em `MSG_STANDARDPARM/16`, com canal em `Parm/+12`.
- A source anterior enviava imediatamente `0xED7` com canal zero para 4030,
  roteava 4031 pelo uso genérico e procurava apenas os IDs modernos do modal.
  `SGrid` e `TMFieldScene` agora restauram os dois prompts e os IDs compatíveis
  627/630, além de OK/Enter e cancel do `FieldScene2.bin`.
- `ServerWarLetterContract.h`, asserts e testes fixam itens, modos, opcodes,
  tamanho e offset. `Build-Client.ps1` passou 51.684 checks, zero erros e
  instalou `project.exe` SHA-256
  `C3BF528A2C9BD9DB6719604BC567C752162FFAF5C2DD6ECD9E704FB1F18984ED`.
- Estado: client `PARIDADE_NATIVA / CONTRACT / STATICALLY VERIFIED / AUTOMATED
  TESTED`; não `CLIENT-TESTED`. O servidor não possui handler nem coordenação
  entre Worlds/canais, então o suporte integrado permanece explicitamente
  bloqueado em vez de consumir a carta sem efeito autoritativo.

### AirMove 0xAD9 — 2026-09-22

- O `WYD.exe` nativo confirma `MSG_STANDARDPARM2/20`, rota `0..4` e modos 1
  (start) e 2 (end); a UI depende de NPC visível face 63/Merchant baixo 7 no
  chunk 16. A ficha é `flows/transport/airmove-contract.md`.
- Client: limite de rota, contrato C++ testável e fim idempotente. Servidor:
  valida contexto, duração e replay, bloqueia gameplay durante voo e decide
  o destino entre os cinco terminais nativos; morte/logout/teleporte cancelam
  o estado. Testes Go focados passaram, incluindo 19/21 bytes.
- A base atual não tem NPC de transporte correspondente. Não criar um NPC
  fictício para satisfazer teste; o gate de execução real permanece pendente.
- Gate do lote: `go test -count=1 ./...`, `go vet ./...`, validador de fichas,
  layout/links e build integrado passaram; 51.693 checks C++ e `project.exe`
  SHA-256 `1F8157D57A3583605CE52923C6D09662535AE6D8B39E90FEBF10972479FF57DB`.
  Estado `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT-TESTED`.
- Continuação da UI: o bootstrap compacto 7.48 agora vincula título `12549`
  e colunas `12550..12552` do painel `12544`, presentes no dump real de
  `FieldScene2.bin`. `AirMove_ShowUI` valida dependências e nomes antes de exibir
  a janela, oculta/limpa estado em falha parcial e permite fechamento sem lista.
  É `MODERNIZACAO_COMPATIVEL`, sem alteração de wire ou servidor. Novo build
  passou 51.693 checks e instalou `project.exe` SHA-256
  `A29A600F5DF1B54ADFC352D8E02584185856C33F670D11B84F558444C7A4890A`;
  ficha, layout e diff passaram. Pendente: NPC real e abertura/viagem no client.
- Continuação da UI: o título agora é aplicado diretamente ao controle e os
  cinco nomes de rota são copiados com limite de 18 caracteres, sem usar texto
  de recurso como formato `printf` nem exceder os buffers locais. É
  `MODERNIZACAO_COMPATIVEL`, sem mudança de wire/servidor. Build Release
  integrado passou com 51.693 checks; candidato instalado SHA-256
  `10791835D8098F3F6D08D7951568C4EE5535BC5375F3375E2ABEAAAF2736B1FA`.
  A abertura e a viagem com NPC real continuam pendentes de `CLIENT_TESTED`.
- Continuação do voo: `TMHuman::Update` e `AirMove_End` agora consomem e zeram
  o delta pendente uma vez. Antes, `SetPosition` já alterava `m_vecPosition`,
  mas o offset acumulado voltava a ser somado a cada frame e no encerramento.
  O teste C++ cobre consumo único; build Release integrado passou 51.695 checks
  e instalou `project.exe` SHA-256
  `2140019A43B8B46D3739B6EA12375549CEFDFB2ACF98C0A55EF3141469A402C6`.
  É `MODERNIZACAO_COMPATIVEL`; sem mudança no wire ou no servidor. A viagem
  real com NPC continua pendente de `CLIENT_TESTED`.
- No pouso, o tipo antigo era escrito no mesh do corpo e o mount de voo `40`
  continuava ativo, com o look original zerado. O client agora salva e
  restaura tipo e look da montaria antes de `UpdateMount()`;
  `MODERNIZACAO_COMPATIVEL`, sem wire/servidor novos. Build Release passou
  51.696 checks e instalou `project.exe` SHA-256
  `381273DFD218716139F1E9DC3000C9547CE4666C9ED3243125FFA88C959087A0`.
  Pendente validar montaria original e corpo em viagem real (`CLIENT_TESTED`).
- A transicao de waypoint podia ler alem do array `[5][10]` caso uma rota
  visual ocupasse a ultima posicao. O indice agora so avanca apos verificar
  limite e sentinela `(0,0)`, sem alterar rotas, wire ou servidor. Testes C++
  cobrem sentinela, ultima posicao e indice invalido. Build Release passou
  51.698 checks, ficha e layout passaram, e instalou `project.exe` SHA-256
  `0E8F52AFE6EB5FEFD0BE837EA5B88692DCC944E9C470FBF3ED7EA4637DAA4D3E`.
  Estado `MODERNIZACAO_COMPATIVEL / AUTOMATED TESTED`; viagem real pendente.

### Recepção de chat de grupo `0x334` durante bootstrap parcial — 2026-09-22

- O WYD-Go entrega mensagens de grupo com `String[0]='='` em `0x334/128`.
  `TMFieldScene::OnPacketMessageWhisper` e `TMHuman::OnPacketMessageWhisper`
  liam `m_pPartyList->m_nNumItem` sem verificar se a lista `1863` já havia
  sido vinculada. Essa janela já é protegida no receptor de chat local `0x333`.
- Os dois receptores agora consomem o packet sem tocar a UI quando falta a lista;
  o caminho do humano também verifica a cena e os filtros opcionais antes de
  acessá-los. Decisão: `MODERNIZACAO_COMPATIVEL`; wire, entrega autoritativa,
  prefixos e comportamento com UI íntegra permanecem inalterados.
- `go test -count=1 ./internal/game -run 'TestChatHandlersRouteLocalWhisperAndChannels|TestChatHandlersRouteCitizenshipAndKingdomChannels'`
  passou. `Build-Client.ps1` passou 51.698 checks, compilou os dois arquivos e
  instalou `project.exe` SHA-256
  `A73406F414D3BD92A776D2F197684BC204C11D1D52A662FBAF62DDF3162FB45A`.
  Estado `STATICALLY VERIFIED / AUTOMATED TESTED` para build e envelope; a
  chegada durante bootstrap parcial não foi exercida pelo teste automatizado.
  Não é `CLIENT-TESTED`: testar com dois clients recebimento de `=` antes/depois do painel Party,
  incluindo troca de cena e relogin.

### Entrada das cartas de guerra — 2026-09-22

- O modal 4030/4031 convertia decimal em `long long`, mas reduzia para o
  `int32` de `Parm/+12` sem faixa. Entradas como `4294967297` podiam enviar
  canal 1. A guarda agora exige `1..INT32_MAX` e mantém modal/texto/foco no erro;
  decisão `MODERNIZACAO_COMPATIVEL`, sem alteração de wire ou de servidor.
- `Build-Client.ps1` passou 51.699 checks e instalou `project.exe` SHA-256
  `5B908910F0388A71834B620BF65D52C7B90DE93EA7C5809BAAD5FFCE718E041A`.
  Ainda não é `CLIENT_TESTED`. `0xED7/0xED8` permanecem sem handler nem
  coordenador entre canais no WYD-Go; não consumir carta nem anunciar suporte
  integrado até esse estado existir.

### Desconexão na seleção de servidor — 2026-09-22

- `NewApp` já encaminhava `FD_CLOSE` como `OnPacketEvent(0, nullptr)` e o
  handler base já implementava mensagem, limpeza da migração e retorno seguro.
  Porém, `TMSelectServerScene` retornava antes da chamada base quando o payload
  era nulo, tornando a queda TCP silenciosa justamente na tela de login.
- O override agora chama `TMScene::OnPacketEvent` antes da guarda nula e só faz
  cast para `MSG_STANDARD` com payload presente. Decisão: `PARIDADE_NATIVA`,
  baseada na ficha `socket-disconnect-return-selectserver`; wire e servidor não
  mudaram.
- Um teste de contrato protege a ordem base -> guarda nula -> cast. O build
  Release integrado passou 51.705 checks e instalou `project.exe` SHA-256
  `4610A0C73E978284EFB81D6F43DBE03C291EF5259398EE4B27E803C84C91CA97`.
  Estado `STATICALLY VERIFIED / AUTOMATED TESTED`; ainda falta provocar uma
  queda TCP real na seleção, confirmar a mensagem e conectar novamente para
  promover a `CLIENT_TESTED`.

### Limite nativo da senha de login — 2026-09-22

- O `WYD.exe` canônico rejeita conta acima de 12, senha abaixo de 4 e senha
  acima de 10 antes de conectar. Os branches reproduzidos em `FUN_004AC985`
  são `cmp eax,0Ch` em `0x004ADC2A`, mínimo 4 em `0x004ADC95` e
  `cmp eax,0Ah` em `0x004ADD00`; o `strdef.bin` confirma as mensagens 5 e 6.
- A source tinha somente o mínimo e podia aceitar uma entrada longa para depois
  truncá-la no campo `AccountPassword[12]`. `TMSelectServerScene` agora mostra
  a mensagem 6 e retorna antes de conexão e serialização. O packet permanece
  `0x20D/0x74`, versão 748, sem mudança de ABI.
- O WYD-Go, cadastro público, autenticação administrativa, loadtest, portal e
  OpenAPI convergem no máximo 10. Contas antigas do emulador com senha de 11 ou
  12 caracteres precisam de recuperação operacional; não houve migração
  automática de hashes.
- A ficha `flows/transport/account-login-credential-bounds.md` está em
  `CONTRACT`. `go test -count=1 ./...`, `go vet ./...`, geração/check/build do
  portal e o
  validador de fichas passaram. `Build-Client.ps1` passou 51.711 checks e
  instalou `project.exe` SHA-256
  `24B8A2B08E93BFF1F6B9AA9F72DF9C0283B751F12DAB44535AB3BA25A5EF77C3`.
- Estado `PARIDADE_NATIVA / STATICALLY VERIFIED / AUTOMATED TESTED`; ainda não
  `CLIENT-TESTED`. Próximo passo: no candidato atual, confirmar que 11
  caracteres exibem a mensagem 6 sem conexão e que exatamente 10 autenticam;
  aproveitar a sessão para provocar uma queda TCP na seleção e validar o lote
  anterior de disconnect.

### Limite da tabela de grupos na selecao e troca de servidor — 2026-09-22

- A tabela `g_pServerList` tem 10 grupos; a selecao e o menu de troca em Field
  procuravam uma sentinela em ate 11 entradas. Ambos usam agora uma descoberta
  limitada ao tamanho da tabela; Field ignora uma tabela vazia. Decisao:
  `MODERNIZACAO_COMPATIVEL`, sem mudanca de wire ou servidor.
- `Build-Client.ps1 -NoDeploy` passou 51.723 checks (vazio, parcial e 10 grupos),
  compilou Release|x86 e produziu SHA-256
  `C1B4379FEC7C2DB660F25204273A07215FD8BAF67723D2EB7517DDCE998BEDA7`.
  O runtime nao foi substituido. Estado `STATICALLY VERIFIED / AUTOMATED TESTED`,
  nao `CLIENT_TESTED`; validar selecao e troca com tabela cheia no client real.
- `0xFAA` segue sem handler no Go: a resposta nativa distingue renomeacao e
  transferencia, e sucesso limpa o slot. Nao implementar mutacao de conta sem
  fechar origem, persistencia e semantica autoritativa da operacao.

### Leitura unica do `sn.bin` binario — 2026-09-22

- `NewApp::InitServerName` ainda lia o `sn.bin` de 143 bytes como texto na
  inicializacao e escrevia nomes fixos `Meu Canal`/`STAFF`; a cena de selecao
  ja recarregava o mesmo asset corretamente com
  `WYD748_LoadServerNameList` antes de montar os controles. Removidos chamada,
  metodo e declaracao textuais; a cena usa `ServerName_Path` no loader binario.
  `MODERNIZACAO_COMPATIVEL`, sem alteracao de asset, wire ou servidor.
- `Build-Client.ps1 -NoDeploy` passou 51.723 checks, compilou Release|x86 e
  produziu SHA-256
  `28F1764724CCF964C88852D3B8A1035B669591CB8D516C56E7E41007572F4980`.
  O asset local mede 143 bytes e suas ordens estao em `0..10`; ainda nao houve
  teste direto do loader nem execucao in-game do candidato. O runtime nao foi
  substituido. Proximo gate: instalar e abrir o candidato quando houver sessao
  de client, selecionar `Canal`/`VPS`, conectar e exercitar retorno/relogin.

### Limites locais da selecao e troca de canais — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: a tabela de endpoints possui 10 grupos x 11
  canais (status no zero), enquanto nomes opcionais possuem 10 x 10. O fluxo
  importado consultava nome no indice 10 e a origem agregada no indice 11
  quando o canal diario chegava a 10. `ServerChannelNameAt` exige indice e
  terminador validos; `CopyServerEndpointAt` descarta origem fora da tabela.
  A selecao limita o maior grupo visivel e valida o grupo mapeado antes de
  ler `m_nDay`; a troca rejeita indice de grupo corrente invalido.
- Teste focado cobre nome 9/10, nome sem NUL e endpoint 10/11, alem de grupos
  fora do intervalo. `Build-Client.ps1 -NoDeploy` passou 51.727 checks e
  compilou Release|x86; SHA-256 do candidato:
  `4333D4357DDEFDC5321756777C9D278BA35CD26FD430493B409A99A16C488B88`.
  `git diff --check` dos arquivos tocados passou. O runtime nao foi substituido.
  Pendente: abrir o candidato real, selecionar o ultimo canal configurado,
  conectar, trocar de canal e verificar retorno/relogin. Sem `CLIENT_TESTED`.

### Teste direto do carregador `sn.bin` — 2026-09-22

- O `ArchitectureTests` agora compila o `WYD748Assets.cpp` real e chama
  `WYD748_LoadServerNameList` sobre `client748/sn.bin`, sem alterar o asset.
  Confirma o layout de 143 bytes, `__VPS`/`Canal`, as ordens e o grupo sem nome;
  rejeita caminho nulo, buffers curtos, arquivo truncado e ordem fora de
  `0..10`, preservando o estado carregado na rejeicao.
- Compilacao focada `ArchitectureTests.vcxproj` Release|Win32 e execucao passaram
  com 51.742 checks. Apenas o projeto de testes mudou; o executavel do client
  nao foi recompilado nem instalado neste lote. `AUTOMATED TESTED` para o
  carregador, ainda nao `CLIENT_TESTED` para selecao/conexao/retorno. A regra
  de nomes opcionais de canais permanece sem mudanca por faltar evidencia
  nativa especifica para seu mapeamento.

### Duracao da resposta `0xFAA` — 2026-09-22

- `PARIDADE_NATIVA` localizada: o receptor nativo usa 3500 ms (`0xdac`) em
  todos os resultados de `0xFAA`; a source usava 2000 ms nos erros 2, 3 e
  default. Ajustada somente a duracao naquele corte; nenhum handler Go criado.
- `Build-Client.ps1 -NoDeploy` compilou Release|x86 e passou 51.742 checks.
  Sem instalacao ou exercicio da resposta no jogo: `STATICALLY VERIFIED` para
  esta correção, não `CLIENT_TESTED`.

### Mensagem de erro da transferencia `0xFAA` — 2026-09-22

- `PARIDADE_NATIVA` localizada: `client748/UI/strdef.bin` contem apenas 440
  entradas; 1131 nao existe e deixava Result 4 sem texto. O receptor nativo
  usa erro generico. A cena agora usa entrada 204 (`Unknown error.`) por 3500 ms.
  Teste focado cruza o asset real com o ramo de resposta da source.
- Os textos 201-206 descrevem migracao ao `Integrated server`. Go ainda nao
  despacha `0xFAA`; nao implementar rename/delete local sem contrato de destino,
  identidade e transacao. O fluxo permanece LOCATED, sem `CLIENT_TESTED`.
- `Build-Client.ps1 -NoDeploy`: Release|x86 compilado, 51.751 checks PASS;
  SHA-256 `F3B09027AFAAB2D44C7CF76C92F0CEF3EBB2329B67EEA31DEBAFFCA0928192F5`.
  Validador das fichas e `git diff --check` PASS. Candidato nao instalado.
- Teste complementar do loader real: `ArchitectureTests.vcxproj` Release|Win32
  passou 51.757 checks. `strdef.bin` carrega com checksum valido, linha 204
  materializada e 1131 vazia; arquivo corrompido e rejeitado sem apagar a
  tabela anterior. Somente testes mudaram, sem novo build/deploy do client.

### Seleção estável de grupos visíveis — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: a UI cria grupos em ordem inversa, inclusive o
  último slot, mas os eventos contavam apenas os slots anteriores e podiam
  associar uma linha esparsa a outro endpoint. `VisibleServerGroupSlots` define
  as linhas válidas e a cena guarda seus slots ao construí-las.
- O snapshot é necessário porque selecionar o grupo agregado zera endpoints de
  status na tabela global sem remover as linhas existentes; recalcular os
  slots no próximo evento mudaria a identidade da linha. Testes cobrem dez
  grupos, lacunas, tabela vazia e a mutação após a construção.
- `Build-Client.ps1 -NoDeploy`: 51.769 checks PASS, Release|x86, candidato
  `E3CA0135E84E4DA74B29A1528D247F4B5978CA151D05890E6D91C7941BB1418A`.
  Não instalado nem `CLIENT_TESTED`. O candidato anterior foi instalado e
  abriu a janela, mas a captura de tela falhou com `0x80004002`; nenhum clique
  foi validado. O mapeamento interno do grupo agregado continua pendente de
  evidência nativa antes de alteração.

### Reabertura da lista de canais em Field — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `SListBox::Empty` não apaga a seleção. Field
  reabria a lista de troca com o índice antigo e dereferenciava sem guarda o
  item recebido pelo callback. Agora limpa a seleção antes de inserir linhas
  e ignora evento sem item; o comando de troca `srv` permanece intacto.
- `Build-Client.ps1 -NoDeploy` passou 51.772 checks e recompilou Field em
  Release|x86. Ainda sem teste real de reabertura/troca. A conversão de linha
  esparsa por `idwEvent + 1` e o grupo agregado aguardam evidência nativa e
  contrato integrado antes de qualquer mudança semântica.

### Pedido `srv` no emulador monocanal — 2026-09-22

- O client Field envia whisper `srv` após a espera visual de cinco segundos,
  mas WYD-Go não possui coordenador de canais nem resposta de migração `0x52A`.
  O handler agora consome o comando reservado e envia aviso `0x101` de
  indisponibilidade, sem alterar personagem/sessão nem simular transferência.
- Teste focado `go test ./internal/game -run
  'TestServerSwitchRequestReportsUnavailableWithoutMigration|TestWhisperDayRequestReturnsHiddenCalendarSync|TestWhisperHandlerReportsOfflineAndCharacterInfo'
  -count=1` passou. O mapeamento `idwEvent + 1` permanece intocado e sem
  evidência nativa suficiente; nenhuma troca real foi `CLIENT_TESTED`.

### Leitura da população na seleção de canal — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `BASE_GetHttpRequest` podia consumir todo o buffer
  antes de inserir NUL e retornava sucesso mesmo se `InternetReadFile`
  falhasse. A leitura agora reserva o terminador, rejeita capacidade inválida
  e limpa a resposta na falha. O status continua apenas informativo; endpoint,
  seleção, wire e emulador não foram alterados.
- `Build-Client.ps1 -NoDeploy`: 51.774 checks PASS, Release|x86 recompilado;
  candidato SHA-256
  `CAE948D565C85EA1F048800DE849BC2AC40E7CC02D89F14F24FD6908B7E2C819`.
  Teste de contrato estático cobre o limite/NUL/falha; não houve simulação de
  falha WinINet nem clique no client. O runtime não foi substituído.

### Identidade do adaptador no login e reconexão — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: login inicial e reconexão `0x20D/0x74` montavam
  `AdapterName[4]` duplicadamente, sem validar o segundo `GetAdaptersInfo`,
  podendo desreferenciar alocação nula ou exceder `temp[256]`. Ambos usam agora
  uma rotina comum que consulta o primeiro adaptador, valida GUID completo e
  preserva os mesmos quatro words para a entrada válida. Falha de API, falta
  de NUL e GUID inválido deixam o campo zerado pelo packet inicializado.
- Teste focado cobre GUID válido, palavras wire, caractere inválido, ausência
  de NUL, truncamento e uso da rotina nas duas cenas. `Build-Client.ps1
  -NoDeploy` passou 51.781 checks e compilou Release|x86. Wire e servidor não
  mudaram; executável de runtime não foi substituído. Estado
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`. Pendente:
  exercitar login e reconexão no client real com o emulador.

### Canais esparsos na seleção inicial — 2026-09-22

- A coluna Channel vazia na captura antes de escolher `Canal`/`VPS` é o
  lifecycle esperado; a captura não comprova defeito de carregamento.
- `MODERNIZACAO_COMPATIVEL`: linhas de canais configurados eram compactadas,
  mas `Connect` usava `selectedChannel + 1` como índice físico. A cena agora
  guarda o canal de cada linha criada e usa o snapshot para o endpoint normal;
  rejeita índice obsoleto. Wire, servidor e mapeamento especial do agregado
  permanecem inalterados.
- `Build-Client.ps1 -NoDeploy`: 51.785 checks PASS, Release|x86, candidato
  SHA-256 `67B64372442E871601FE9A5006E79DC8097212C469EEB7CF8D7DABB4F9074237`.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não instalado nem `CLIENT_TESTED`.
  Próximo gate: clique e conexão reais no client; grupo agregado e `srv` em
  Field continuam dependentes de evidência nativa própria.

### Falha parcial ao abrir a seleção de servidores — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `LoadRC` e `sn.bin` podem falhar antes do antigo
  `memset(m_pCheckHumanList)`, mas `ObjectManager` destrói a cena nesse caminho
  e o destrutor examina os 50 ponteiros. O array agora nasce nulo no objeto;
  fluxo de sucesso, recursos e wire permanecem iguais.
- `Build-Client.ps1 -NoDeploy`: 51.787 checks PASS, Release|x86, SHA-256
  `22A0B984E9A50144B40827FA4BAAF41A6C326985AFA2485B22CBC9F534B3AE38`.
  Guarda estática e compilação passaram; falha injetada e client real não foram
  exercitados. Candidato não instalado, estado não `CLIENT_TESTED`.

### Destino de endpoint invalido na selecao/migracao — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `CopyServerEndpoint` mantinha o conteudo anterior
  do destino quando a celula de `serverlist.bin` era vazia ou nao tinha NUL.
  Agora limpa o primeiro byte na rejeicao; endereco valido, asset, indice e
  wire nao mudam. Testes cobrem ambos os tipos de falha apos IP anterior.
- `Build-Client.ps1 -NoDeploy`: 51.787 checks PASS, Release|x86 recompilado,
  candidato SHA-256
  `0595B7DD797EF394CC7DFD7944B48EDDC1C74B4C1BD7B0DD9D097325EA30934B`.
  Nao instalado nem `CLIENT_TESTED`. Grupo agregado segue pendente: corpus e
  projeto Ghidra historicos nao existem no perfil local atual; nao mudar o
  mapeamento sem resolver o clique/endpoint no binario 7.48.

### Controles obrigatorios na abertura de Scene2 — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `LoadRC` podia encerrar a leitura de um
  `SelServerScene2.bin` parcial sem detectar a falta de controles usados sem
  guarda por `InitializeUI` e login. A cena agora verifica root, listas,
  painel/botoes, logos e campos de edicao apos `sn.bin` e antes de montar UI;
  falha com log e usa o teardown parcial protegido no lote anterior.
- `Build-Client.ps1 -NoDeploy`: 51.788 checks PASS, Release|x86,
  `WYD.exe` SHA-256
  `60E3831218A033646C1FE0367A481A7B0F1CB9FCF7124D7DDC532D947E769135`.
  Teste de contrato estatico; recurso truncado e client real nao exercitados.
  Candidato nao instalado, nao `CLIENT_TESTED`. Seguem pendentes o clique/
  conexao visual e a evidencia nativa do grupo agregado.

### Tag de controle truncada no carregador RC — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `TMScene::ReadRCBin` aceitava 1–3 bytes finais de
  uma tag de tipo como EOF limpo e retornava sem fechar o arquivo quando o tipo
  era desconhecido. A leitura agora distingue EOF entre registros de tag
  incompleta; ambas as rejeicoes fecham o arquivo. Formato valido e wire
  permanecem iguais.
- `Build-Client.ps1 -NoDeploy`: 51.792 checks PASS, recompilacao Release|x86,
  candidato SHA-256
  `E1646D6AFC0406F518477C9D4A1A4635790C7C09BB1A48CC10C9068976E36138`.
  Teste focado atualizado depois do build passou 51.793 checks, incluindo tag
  valida seguida de tag parcial. `STATICALLY VERIFIED / AUTOMATED TESTED` para
  essa guarda; arquivo RC completo corrompido e fluxo visual nao foram testados
  no client. Nao instalado, nao `CLIENT_TESTED`.

### Falha de RC na seleção de personagem e no Field — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `TMSelectCharScene` e `TMFieldScene` ignoravam o
  retorno de `LoadRC`. Agora interrompem a inicialização se o RC falhar; no
  Field, isso impede que recurso ausente/corrompido entre no fallback reservado
  para o `FieldScene2.bin` 7.48 válido sem o controle moderno `66817`.
- `Build-Client.ps1 -NoDeploy`: 51.795 checks PASS, cenas recompiladas em
  Release|x86, candidato SHA-256
  `9F3C1DA5B193D7F5D364AEB16F33758938537A1A6F6441F6F5F1D0575AE08F91`.
  As guardas têm teste de ordem estático; não houve injeção do arquivo RC no
  client executável. Candidato não instalado, não `CLIENT_TESTED`.

### Alinhamento de grupos esparsos na seleção — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: o painel decorativo dos grupos agora usa o índice
  da linha visível, como o texto e a área clicável. O slot físico ainda
  identifica o painel e o endpoint; configuração densa e wire não mudam.
- `Build-Client.ps1 -NoDeploy`: 51.796 checks PASS e compilação incremental
  Release|x86, candidato SHA-256
  `E491C5C4C4011BED8105DE5CFD95FF17CDEAAC3C03C32B989CFBEBC5922F1B2D`.
  Guarda geométrica estática; candidato não instalado, sem clique em grupo
  esparso no client. `STATICALLY VERIFIED / AUTOMATED TESTED`, não
  `CLIENT_TESTED`.

### Conclusão do fade dos grupos — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: o ramo final de `SetAlphaServer` agora cobre todos
  os painéis de grupo existentes, como o ramo parcial já fazia; antes só os
  slots 0 e 1 recebiam a cor final. Sem mudança de duração, asset ou wire.
- `Build-Client.ps1 -NoDeploy`: 51.797 checks PASS, cena recompilada em
  Release|x86, candidato SHA-256
  `8D139C4EF065F7AFDD65C6DEFC77D6ECC3887A6689BB92C1E414D9BF38D0C975`.
  A guarda ampliada passou 51.798 checks sem recompilar o produto.
  Houve uma falha inicial na guarda estática por CRLF, corrigida antes do build
  aprovado. O frame não foi visto no client; candidato não instalado,
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`.

### Transferência de personagem sem serviço de destino — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: o WYD-Go agora aceita `0xFAA/52` somente na
  seleção autenticada e responde `Result=4` no mesmo layout. O client 7.48
  exibe erro genérico e libera a espera; nenhum slot, nome ou conta é alterado.
  Não há confirmação de transferência: destino, identidade e persistência
  atômica ainda precisam de contrato próprio. `0x52A` não substitui `0xFAA`.
- `go test -count=1 ./internal/game ./internal/wire` PASS. Cobertos tamanho,
  fase, Result/Slot, resposta desencriptada e imutabilidade da conta.
  `AUTOMATED TESTED` no servidor; sem clique/resposta real no client, não
  `CLIENT_TESTED`. A janela de alerta de segurança do Windows bloqueou o
  teste visual; cabe ao usuário resolver esse alerta antes de retomá-lo.

### Falha ao carregar o terreno da seleção — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: a cena de seleção agora encerra a inicialização
  quando `Env\\Field1616.trn` não carrega. Antes marcava erro crítico e seguia
  montando a cena com terreno indisponível. O caminho de asset válido e o wire
  permanecem iguais.
- `Build-Client.ps1 -NoDeploy`: 51.814 checks PASS e build Release|x86 PASS.
  Guarda estática confirma o retorno antes do uso da máscara do terreno; não
  houve injeção de asset ausente no executável. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`.
- O client foi aberto com autorização do usuário, mas duas capturas da janela
  falharam com `0x80004002`; não houve clique nem validação visual. O grupo
  agregado segue sem contrato nativo suficiente para mudar seu mapeamento.

### Leitura segura do terreno 7.48 — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: o leitor `.trn` mantém o layout nativo de nome
  prefixado por byte, dois bytes de coordenada e 4096 tiles de 12 bytes.
  Rejeita nome sem espaço para terminador e qualquer leitura parcial, antes
  de derivar posição e máscaras. A seleção de personagem e a inicialização
  principal do campo agora encerram a carga se o terreno falhar.
- `Build-Client.ps1 -NoDeploy`: 51.824 checks PASS, incluindo o
  `client748/Env/Field1616.trn` real, nome-limite e truncamentos; compilação
  Release|x86 PASS. SHA-256 do candidato:
  `90431DEE41C41F5CAA9BC10A255F78C5345BE9D250172CA97518CC2519B69773`.
  `git diff --check` sem erro de whitespace. A primeira tentativa de build
  falhou por redeclaração de variável apenas no teste, corrigida e aprovada.
- O processo `client748/project.exe` existente não foi substituído. A tentativa
  visual autorizada não obteve uma janela estável: `sky.get_window` respondeu
  `window id 11535688 was not found` mesmo após nova seleção. Sem clique ou
  validação do candidato no runtime. `STATICALLY VERIFIED / AUTOMATED TESTED`,
  não `CLIENT_TESTED`. Próximo gate: instalar o candidato quando o client
  estiver fechado e exercer seleção, criação de personagem e entrada no campo.

### Liberação do terreno recusado durante warp — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `TMScene::Warp2` agora libera o `TMGround` recém-criado
  quando `LoadTileMap` falha, antes da transferência de propriedade para a cena.
  O terreno atual, o asset válido e o contrato de rede não mudam.
- `Build-Client.ps1 -NoDeploy`: 51.825 checks PASS e compilação incremental
  Release|x86 PASS. SHA-256 do candidato:
  `451BCB1EAD70A776C02A26DE5DB86C7C7AF9B17DDB0FF15F284EAA3B005ADC49`.
  `git diff --check` sem erro. Guarda de regressão é estática; falha de mapa não
  foi injetada no runtime. `STATICALLY VERIFIED / AUTOMATED TESTED`, não
  `CLIENT_TESTED`.
- O processo `client748/project.exe` permanece em uso e não foi substituído.
  A tentativa visual autorizada falhou novamente: a janela listada como
  `11535688` deixou de ser encontrada no instante da captura (a ferramenta
  informou `34800848`); a recuperação única não estabilizou o identificador.
  Nenhum clique foi feito. Instalação e fluxo visual do candidato pendentes.

### Leitura segura dos objetos de campo 7.48 — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `TMObjectContainer::Load` valida a leitura completa
  e os limites dos registros de 28/36 bytes antes de criar objetos; a cópia
  de cada registro deixa de ler os 8 bytes opcionais após o fim do arquivo.
  O layout e os tipos aceitos permanecem os do runtime 7.48. A inicialização
  do campo e `Warp2` encerram a transição e liberam objetos/terreno ainda não
  anexados quando o `.dat` falha.
- `Build-Client.ps1 -NoDeploy`: 51.830 checks PASS, incluindo os 96 arquivos
  reais `client748/Env/Field*.dat`, limites de tipos e truncamentos; build
  incremental Release|x86 PASS. SHA-256 do candidato:
  `E42E96E99D7507313B7BF981AD8C5261E2074A3A8D28DC0C219650360AE1D9E0`.
  `git diff --check` sem erro. A primeira tentativa de teste falhou por
  conferir `ifstream::eof()` após leitura por iteradores; corrigido para
  `!bad()` e aprovado.
- O candidato não foi instalado. A tentativa de abrir `build/.../WYD.exe`
  com o diretório de trabalho do runtime saiu antes de criar janela porque
  `project.exe` (PID 8144) já estava aberto e `NewApp.cpp` impede uma segunda
  instância pelo título. Nenhum clique foi feito. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`; teste visual do candidato aguarda
  fechamento combinado da sessão atual.

### Rollback do terreno vizinho sem objetos — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `GroundNewAttach` agora libera primeiro o container
  de objetos e depois o terreno secundário antigo. Se a carga de `Field*.dat`
  falhar após `Attach`, remove os links direcionais que apontam para o candidato
  e libera container e terreno antes de marcar o erro crítico. O caminho de
  sucesso, os assets e o wire 7.48 permanecem inalterados.
- `Build-Client.ps1 -NoDeploy`: 51.833 checks PASS e compilação incremental
  Release|x86 PASS. SHA-256 do candidato:
  `01E43147C447FC3B4A9312160A61C0A79F365CF9E639FBE3DF89AC10D89ECCBE`.
  Guarda de regressão cobre ordem de liberação e rollback; `git diff --check`
  sem erro de whitespace. `STATICALLY VERIFIED / AUTOMATED TESTED` somente.
- O usuário autorizou o teste visual, mas a ferramenta de janelas perdeu o
  identificador da janela do `project.exe` antes da captura (`11535688` foi
  substituído por `34800848`; a segunda tentativa falhou por ausência de PID
  na janela de primeiro plano). Nenhum clique foi feito. O candidato não foi
  instalado e a falha de `.dat` não foi provocada no runtime. O gate
  `CLIENT_TESTED` segue pendente; retomar quando a janela puder ser capturada
  e a sessão atual estiver fechada para permitir a instalação do candidato.
- Atualização `MODERNIZACAO_COMPATIVEL`: o rollback acima removia links, mas
  não restaurava a borda de tiles/normais já copiada por `TMGround::Attach`.
  A primeira tentativa deslocou `Attach` para depois de `Field*.dat`; ela foi
  **substituída** pela correção abaixo porque alterava o fluxo válido. O
  vizinho antigo continua desligado antes de ser liberado. Binário/Ghidra e
  servidor: `NÃO APLICÁVEL` à ordem interna sem mudança de asset ou wire.
  Guarda de ordem/cleanup em
  `SceneDisconnectContractTests.cpp`; `Build-Client.ps1 -NoDeploy`:
  51.844 checks PASS, compilação incremental Release|x86 PASS, SHA-256
  `7B231A2CCA8134473C6366A4D51885FFC969C399FBF8474E453976AC2CCC1269`.
  `STATICALLY VERIFIED / AUTOMATED TESTED`; injeção de falha no executável e
  fluxo visual continuam pendentes.
- Correção da regressão de ordem (`MODERNIZACAO_COMPATIVEL`, 2026-09-23):
  `TMObjectContainer::Load` consulta `GroundGetColor` e chama `GroundSetColor`
  pela cena para efeitos 501..503 e 506. Os `Field*.dat` reais contêm 3.186
  registros 501, 116 de 502, 452 de 503 e 2.905 de 506; a ordem anterior deixava o
  vizinho fora dos links da cena durante a carga e poderia suprimir a cor
  desses efeitos. `GroundNewAttach` voltou a anexar antes de carregar os
  objetos; guarda 64 tiles/normais da borda ativa e o estado do minimapa e,
  em falha de `.dat`, desliga os links, restaura a borda e libera o candidato.
  `Attach` inválido também libera o candidato. Asset/wire/servidor não mudam;
  binário/Ghidra `NÃO APLICÁVEL` à proteção interna do rollback. Guarda
  estática atualizada; `Build-Client.ps1 -NoDeploy`: 51.844 checks PASS,
  Release|x86 PASS, SHA-256 do candidato
  `95D94D8D82DF90BDDE1ACECB7F9C8C43DC40A2792A7BA289BBA100336B95F76C`.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`. A sessão
  antiga `project.exe` (PID 8144) ainda estava aberta; candidato não instalado.

### Limpeza e limites da máscara de terreno — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `GroundNewAttach` e `Warp2` agora zeram os 65.536
  bytes de `m_HeightMapData` antes de reconstruir o mapa, em vez de apenas os
  quatro primeiros bytes. As duas sobrecargas de `GroundGetMask` limitam o
  índice 256 ao último índice válido, 255. O layout do terreno, os assets e o
  wire não mudam; evita atributos antigos no espaço não copiado e acesso fora
  do array exatamente na borda superior.
- `Build-Client.ps1 -NoDeploy`: 51.837 checks PASS e compilação incremental
  Release|x86 PASS. Candidato SHA-256
  `4DE6FA216E178D8464DC7CAEE014D70EEB9B01968F1737AA490BB0298ED887E8`.
  Guardas estáticas cobrem os dois caminhos de reconstrução e as duas consultas;
  `git diff --check` sem erro de whitespace. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`. Candidato não instalado; a validação
  visual aguarda a janela do client ficar estável e a sessão atual ser fechada.

### Falha de terreno vizinho sem recarga por quadro — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `GroundNewAttach` agora sinaliza como erro crítico a
  falha de leitura ou validação de `Field*.trn`, com o código 10 já usado em
  `Warp2`, libera o candidato e preserva o terreno ativo antes de qualquer
  substituição. A guarda inicial impede nova tentativa por quadro (inclusive
  uma segunda direção no mesmo quadro) enquanto a cena está em erro. O caminho
  de carga bem-sucedida, os assets e o wire permanecem inalterados.
- Guarda estática adicionada a `SceneDisconnectContractTests.cpp` (arquivo de
  testes já presente como untracked no worktree). `Build-Client.ps1 -NoDeploy`:
  51.838 checks PASS e compilação incremental Release|x86 PASS. Candidato
  SHA-256 `B6FECE8A767F058A9A962FD3456537EDB776D0B91E45C86CCF4B6A2AA123E454`.
  `git diff --check` sem erro de whitespace. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`; o candidato não foi instalado no
  runtime. A captura visual autorizada não pôde prosseguir: `list_windows`
  retornou a janela `WYDESTINY MMORPG`, mas `get_window` rejeitou o ID como
  inexistente mesmo após atualizar a lista. Nenhuma entrada foi enviada ao
  client, e a sessão aberta foi preservada.

### Coordenadas internas do terreno antes da troca — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `GroundNewAttach` e `Warp2` agora comparam as
  coordenadas carregadas do registro `.trn` com as coordenadas solicitadas
  pelo nome `FieldXXYY.trn`. Um arquivo estruturalmente válido, mas nomeado
  para outra posição, é descartado antes de anexar/substituir o terreno;
  recebe o mesmo erro crítico 10 já usado para falha de `.trn`. Isto evita
  links direcionais errados e desreferência nula na montagem da máscara.
  A source `TMGround::Attach` compara apenas um eixo ao escolher o vizinho;
  por isso a verificação completa fica antes da chamada. Asset 7.48:
  96/96 arquivos `client748/Env/Field*.trn` têm coordenadas internas iguais
  às do nome, conferidas diretamente no cabeçalho dos registros. O caminho
  válido e o wire não mudam; binário/Ghidra: `NÃO APLICÁVEL` para esta guarda
  defensiva sem alteração do contrato válido; servidor: `NÃO APLICÁVEL`.
- Guardas em `SceneDisconnectContractTests.cpp`; `Build-Client.ps1 -NoDeploy`:
  51.840 checks PASS, build incremental Release|x86 PASS. Candidato SHA-256
  `2ABE6D3084588607677C12FF33EAA2AFBE1A2F4A82BF8BB514AFB4E272DF2CB1`.
  `git diff --check` sem erro de whitespace. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`; o executável novo não foi instalado.
- Fechamento da mesma fronteira: `TMGround::Attach` agora exige deslocamento
  unitário em exatamente um eixo e não limpa os links existentes se o terreno
  não for adjacente. Único caller ativo: `GroundNewAttach`, já protegido pela
  comparação completa das coordenadas. Guarda estática adicionada ao teste;
  `Build-Client.ps1 -NoDeploy`: 51.841 checks PASS e build incremental
  Release|x86 PASS, SHA-256
  `2FE9C8D877F2FE041E9AF433BD6458FD3CC18DEB8277C0590F9383CE18B22856`.
  O `project.exe` antigo (PID 8144) segue em execução; candidato não instalado
  nem `CLIENT_TESTED`.

### Limite da máscara dos objetos de campo — 2026-09-22

- `MODERNIZACAO_COMPATIVEL`: `TMObject::RegisterMask` rejeita índices de
  máscara fora de `0..2047` e agora escreve em
  `m_pMaskData[maskY][maskX]` somente com ambos os índices em `0..127`.
  O endereçamento anterior achatava `nBaseY` na segunda dimensão e aceitava
  128, que fica fora do array `128x128` de `TMGround`. Para posições internas,
  o endereço e o valor da máscara permanecem iguais; os 96 `Field*.dat` do
  runtime 7.48 têm 124.004 registros, todos com índice de máscara `3..2035`.
  Não muda asset, wire ou
  autoridade do servidor. Evidência: dimensões reais do array e a source do
  fluxo de carga de objetos; binário/Ghidra `NÃO APLICÁVEL` à guarda de limite
  sem alteração do contrato válido; servidor `NÃO APLICÁVEL`.
- Guarda estática em `SceneDisconnectContractTests.cpp`; `Build-Client.ps1
  -NoDeploy`: 51.842 checks PASS e compilação incremental Release|x86 PASS,
  após a guarda de índice. Candidato SHA-256
  `ED427AB5059D418AC66E9E089CDB0DB2B19638BF718C52C32ABFCFB8CE9E05D6`.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`: o executável
  instalado não foi substituído enquanto o `project.exe` anterior segue aberto.

### Capacidade dos objetos de água e luz — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: `ValidateObjectFileRecords` rejeita, antes de
  criar qualquer objeto, um `Field*.dat` com mais de 10 registros de água
  (tipo 2) ou mais de 6 luzes (tipos 511..518), que excederiam os arrays fixos
  de `TMGround` e `TMObjectContainer`. O loader passa as capacidades reais dos
  arrays, além do limite geral de 4.096 registros. Todos os 96 arquivos de
  campo do runtime 7.48 passam. Os limites exatos e os casos excedentes são
  exercidos por testes de bytes sintéticos. Binário/Ghidra: `NÃO APLICÁVEL` à
  rejeição de arquivo inválido sem mudança no caminho válido; assets 7.48:
  `UTILIZADOS`; source: `UTILIZADA`; servidor: `NÃO APLICÁVEL` (arquivo local).
- `Build-Client.ps1 -NoDeploy`: 51.847 checks PASS e Release|x86 PASS.
  `Build-Client.ps1`: mesmos checks PASS, candidato instalado e SHA-256
  `9DE2AAAB34E21D1A4ECFE9321F6ECC8F88F256D5E07DBCCFA248A384CB0BC215`
  conferido entre build e `client748/project.exe`. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`: o controle visual foi interrompido
  pelo Esc do usuário durante a tentativa de abrir o client. Houve autorização
  posterior para retomar, mas a captura da janela falhou (ver abaixo).

### Coordenadas de folhas na máscara de campo — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: a validação prévia de `Field*.dat` agora rejeita
  coordenadas de folhas (tipos 311..322) que produziriam índice negativo na
  máscara `m_pVAttrData[128][128]` ou conversão `float`→`int` indefinida. O
  cálculo usado pelo loader não foi alterado. Frações reais como `-0.6` e
  `127.3` continuam aceitas porque a conversão C++ trunca antes de `% 128`;
  uma guarda ingênua de `0..127` quebraria os assets 7.48. O offset de
  `vecPosition` no registro é verificado por `static_assert`. Binário/Ghidra:
  `NÃO APLICÁVEL` à rejeição de arquivo inválido sem mudança no caminho válido;
  assets 7.48: `UTILIZADOS`; source: `UTILIZADA`; servidor: `NÃO APLICÁVEL`.
- Testes sintéticos cobrem as frações válidas, `-1`, NaN, infinito e o limite
  inteiro superior; os 96 `Field*.dat` originais continuam passando.
  `Build-Client.ps1 -NoDeploy`: 51.849 checks PASS e Release|x86 PASS;
  `Build-Client.ps1`: mesmos checks PASS, candidato instalado e SHA-256
  `25498FB3767C1CA59AADF19CEEBA4E65C2285967A45E7857C150618E7FC05EED`
  conferido entre o build e `client748/project.exe`.
- Com nova autorização do usuário, a janela única `WYDESTINY MMORPG` foi
  localizada, mas a captura falhou duas vezes com `SetIsBorderRequired failed:
  No such interface supported (0x80004002)`. Nenhum clique foi enviado.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, ainda não `CLIENT-TESTED`.

### Conversões dos registros de objetos de campo — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: o preflight de `Field*.dat` também rejeita
  coordenadas/altura não finitas ou fora da faixa conversível para `int`,
  incluindo a soma das coordenadas com o offset efetivo do campo. Isso cobre
  todos os tipos de registro antes das conversões comuns do loader, sem mudar
  os valores válidos. Testes sintéticos exercem NaN, infinito, limite inteiro
  e overflow por offset; os 96 arquivos de campo 7.48 passam. Binário/Ghidra:
  `NÃO APLICÁVEL` à rejeição de arquivo inválido; servidor: `NÃO APLICÁVEL`.
- `Build-Client.ps1 -NoDeploy`: 51.851 checks PASS e Release|x86 PASS;
  candidato gerado em `build/`, sem instalar no runtime. Estado:
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`.
- Restrição explícita do usuário: não tentar teste visual ou captura da janela
  nesta máquina. Nenhuma nova tentativa foi feita neste lote. Validação do
  fluxo no client executável permanece pendente por essa limitação.
- No mesmo loader, o checksum acumulado não tinha consumidor ativo (o único
  uso estava em bloco comentado após `return`) e foi removido. A chave espacial
  mantém os mesmos quadrantes da grade com deslocamento em inteiro sem sinal,
  evitando comportamento indefinido no deslocamento com sinal. A busca de
  referências encontrou o checksum apenas nesse bloco morto; testes de chaves
  e dos 96 assets passaram. Novo gate após essa edição: 51.852 checks PASS e
  Release|x86 PASS (`Build-Client.ps1 -NoDeploy`), ainda não `CLIENT_TESTED`.
- O comentário que questionava a liberação de `m_pObjectList` foi removido
  após conferir `TreeNode::~TreeNode` e `AddChildWithKey`: os objetos são
  filhos proprietários da árvore; o array é índice não proprietário. Apagá-los
  novamente no destrutor causaria dupla liberação. As seis luzes permanecem
  liberadas separadamente pelo destrutor do container.

### Confirmação de descarte de item — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: o `case 740` do `TMFieldScene` não chama mais
  `PickupAtItem` antes de enviar `MSG_DeleteItem`. Essa chamada transferia o
  ownership para um retorno ignorado, vazando o `SGridControlItem`; também
  ocultava o item antes de uma possível rejeição. O grid agora mantém o item
  até `SendItem` autoritativo (0x182), cujo handler existente retira, limpa
  aliases e libera o visual tanto em sucesso quanto na ressincronização.
  Guardas de ponteiros impedem desreferência após fechamento da UI. Não houve
  alteração no wire ou no servidor. A ficha `send-item-local-update.md` cobre
  o receptor 0x182 e seu ownership; não se afirma paridade nativa integral do
  fluxo de descarte 0x2E4 sem pesquisa adicional.
- Guarda estática de ordem/ownership em `SceneDisconnectContractTests.cpp`;
  `go test -count=1 ./internal/game -run '^TestDeleteItem'` PASS;
  `Build-Client.ps1 -NoDeploy`: 51.853 checks PASS e Release|x86 PASS,
  SHA-256 `0D30F2222CDC1E0925B937C0DC67E25325F1BA5F0FF354E9C7F465F743428D00`.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT-TESTED`; nenhum
  executável foi instalado e nenhuma captura/tentativa visual foi realizada.
  O usuário explicitou que não se deve repetir tentativas de visualizar a
  tela nesta sessão. O fluxo real de confirmação permanece pendente.
- No mesmo fluxo, `SGridControl::SellItem` agora rejeita item/índice inválido
  antes de acessar `ItemList` e monta o texto da confirmação como dado limitado
  aos 64 bytes de `Name`, sem interpretar `%` como formato nem ler além de um
  registro sem terminador. Testes de contrato cobrem a ordem das guardas e um
  nome de 64 bytes não terminado contendo `%s`. Sem mudança no wire ou servidor.
  `Build-Client.ps1 -NoDeploy`: 51.855 checks PASS, Release|x86 PASS,
  SHA-256 `E9784EC0EBDA6CBF0855FD0D686C359390F28EA0FF4B00FD32DBBDA6FF1D5866`;
  `git diff --check` PASS. `STATICALLY VERIFIED / AUTOMATED TESTED`, não
  `CLIENT-TESTED`. Nenhum executável instalado e nenhuma tentativa de tela.

### Confirmação de venda com nome de item — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: os três caminhos de venda em `SGrid` (duplo clique,
  drag em Shop e `SellItem2`) usam um formatter comum que valida o item/índice,
  lê `ItemList.Name` até 64 bytes e substitui literalmente o único `%s` do
  molde 342. O `strdef.bin` 7.48 consumido contém `'%s'`; saída válida e
  envelope `MSG_Sell` ficam inalterados. Molde inválido/truncado é rejeitado
  antes de mostrar o modal ou emitir packet. Binário/Ghidra: `NÃO APLICÁVEL`
  à guarda interna de limites sem mudar o fluxo válido; asset 7.48 e source:
  `UTILIZADOS`; servidor: `NÃO APLICÁVEL` (wire/autoridade inalterados).
- Testes C++ exercitam nome de 64 bytes sem terminador contendo `%s`, overflow,
  molde ambíguo e ausência dos três `sprintf` antigos. `Build-Client.ps1
  -NoDeploy`: 51.858 checks PASS, Release|x86 PASS, SHA-256
  `C76F01EF9B3BD9AB36ABC5762027A11473A68067BA8D8AA4A0806F9CF0F2FE25`;
  `git diff --check` PASS. `STATICALLY VERIFIED / AUTOMATED TESTED`, não
  `CLIENT-TESTED`; nenhuma instalação, abertura ou captura do client.

### Venda em loja: endereço do slot e vínculo ao mercador — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: a confirmação `case 890` agora obtém o slot de
  Carry pela mesma projeção nativa de nove colunas usada no atalho de venda,
  com guardas para controles/objeto ausentes. O `MSG_Sell` continua com 20
  bytes, `TargetID` do mercador visível, `MyType` e `MyPos` nas posições
  existentes. A evidência da projeção 7.48 já está registrada em
  `GetCarrySlotForCell`/`FUN_0052a737`; nenhuma pesquisa nativa nova foi
  necessária para a guarda interna.
- No emulador, `onSellItem` passou a exigir `TargetID == ShopNPC`, como já
  faz `onBuyItem`. O NPC, preço e
  item seguem resolvidos pelo estado autoritativo, não pelo ID do pacote.
  Os testes de venda existentes passaram a enviar o mercador aberto; o
  lifecycle cobre rejeição de ID divergente sem alterar item, gold ou save,
  seguida de venda válida. Não houve remoção de arquivos nem alteração de
  formato wire.
- Gate: testes Go focados de loja, segurança, imposto, rollback e passivo
  PASS; `Build-Client.ps1 -NoDeploy` 51.859 checks PASS e Release|x86 PASS
  (candidato somente em `build/`); `git diff --check` PASS.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT-TESTED`.
  Por restrição explícita do usuário, não instalar, abrir ou capturar o client
  nesta sessão; o fluxo visual permanece pendente.

### Slot estrutural 63 fora da grade Carry — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: `GetCarryGridForSlot` agora limita a projeção
  visual a `MAX_VISIBLE_CARRY` (0..62). Antes aceitava o slot estrutural 63,
  que virava célula (0,7) fora da grade nativa 9x7. O array e o snapshot wire
  de 64 itens não mudaram; `TMHuman::OnPacketSendItem` ainda atualiza o estado
  estrutural antes de consultar a grade. A evidência de geometria 7.48 já
  constava da ficha `send-item-local-update.md` e dos contratos de Carry.
- Teste estático protege o limite da grade; teste de snapshot existente
  preserva o item estrutural 63 no wire. `Build-Client.ps1 -NoDeploy`:
  51.860 checks PASS e Release|x86 PASS; `git diff --check` scoped PASS.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT-TESTED`.
  Nenhuma instalação, abertura ou captura do client; teste visual pendente.

### Swap confirmado com grade visual ausente — 2026-09-23

- `MODERNIZACAO_COMPATIVEL`: `OnPacketSwapItem` agora resolve os dois slots
  lógicos validados e aplica a confirmação `0x376` diretamente ao cache, sem
  depender de `PickupItem`/`AddItem` ou da existência do grid de equipamento.
  Antes, um grid ausente causava desreferência nula; uma falha de visual podia
  zerar um item confirmado pelo servidor. A transação visual continua usando
  as regras de ownership existentes, mas não determina mais o item lógico.
  Também se restringiu o reset da montaria a saída de `Equip[14]`: Carry/Cargo
  posição 14 não é slot de montaria. Wire, servidor e arrays 7.48 inalterados.
- Reutilizada a ficha `inventory-transaction-confirmations.md` (`CONTRACT`):
  binário/Ghidra `NÃO APLICÁVEL` à guarda interna sem alteração do fluxo válido;
  source e teste C++ `UTILIZADOS`; servidor `NÃO ALTERADO` (confirmação já
  autoritativa). Os testes cobrem cópia integral de item, slot vazio, alias
  origem/destino e ordem do commit independente das grades.
- `Build-Client.ps1 -NoDeploy`: 51.865 checks PASS e Release|x86 PASS;
  candidato permaneceu em `build/`. `STATICALLY VERIFIED / AUTOMATED TESTED`,
  não `CLIENT-TESTED`. Nenhuma instalação, abertura ou captura do client foi
  feita; o teste real está vedado pelo usuário nesta sessão.
- Continuação do mesmo lote: depois do commit lógico, se uma grade existente
  estiver sem o ícone do item confirmado, o handler o recria da cópia do cache
  usando `SetItemOnGrid`. Não duplica o visual já presente e preserva o cache
  caso a inserção seja rejeitada. Se o controle inteiro não existir, nenhuma
  UI pode ser atualizada nessa posição; o item lógico continua preservado.
  Teste estático protege a ordem commit/restauração e ambos os lados da troca.
  `Build-Client.ps1 -NoDeploy`: 51.865 checks PASS e Release|x86 PASS;
  `git diff --check` scoped PASS. `STATICALLY VERIFIED / AUTOMATED TESTED`,
  não `CLIENT-TESTED`; nenhuma tentativa visual nesta continuação.
- Ownership do mesmo handler: ao descartar um visual rejeitado, inclusive no
  caso de item abaixo do índice 41, `releaseRejectedVisual` agora limpa os
  aliases globais de hover, último anexo, venda e cursor antes da liberação.
  Segue o cleanup já aplicado por `SGridControl::Empty`; evita ponteiros
  pendentes sem mudar o wire nem o estado autoritativo. Teste estático cobre
  os quatro aliases e a ausência de deleção direta nos dois lados da troca.
  O diálogo de divisão de item (`case 12`) agora verifica a existência do
  item/controle antes de consultá-los: ele pode permanecer aberto após essa
  liberação e fecha pelo caminho comum sem enviar `MSG_SplitItem` inválido.
  Teste estático cobre a guarda anterior ao acesso. Gate final
  `Build-Client.ps1 -NoDeploy`: 51.867 checks PASS e Release|x86 PASS.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT-TESTED`; nenhuma
  instalação, abertura ou captura do client.

### SendItem recebido por humano remoto — 2026-09-23

- `MODERNIZACAO_COMPATIVEL` no filtro do receptor: a ficha existente
  `send-item-local-update.md` (`LOCATED`) registra uma comparação do receptor
  com o humano local antes das cópias, mas não prova paridade integral.
  `TMHuman::OnPacketSendItem` agora
  consome `0x182` destinado a outro humano antes de consultar a grade,
  escrever no cache local ou recalcular sua aparência com o equipamento
  do jogador. O emulador envia `SendItem` à sessão do próprio personagem;
  wire, servidor, assets e fluxo local não foram alterados.
- O teste de contrato verifica que a guarda precede `Bag_View`, a primeira
  escrita de equipamento e `SetPacketMOBItem`. `Build-Client.ps1 -NoDeploy`:
  51.868 checks PASS e Release|x86 PASS, com quatro avisos C4018 preexistentes
  em trechos não alterados de `TMHuman.cpp`. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT-TESTED`. Não instalar, abrir, capturar ou
  automatizar a tela do client nesta máquina; teste visual permanece pendente.
- `MODERNIZACAO_COMPATIVEL` adjacente: antes de qualquer atualização local,
  `OnPacketSendItem` também rejeita `sIndex` negativo ou maior que o último
  índice do `ItemList.bin` 7.48 (6.500 entradas). Zero segue como slot vazio.
  Evita acesso fora de `g_pItemList` ao recalcular equipamento; frame e itens
  válidos continuam iguais. Teste de contrato protege a ordem da guarda;
  `Build-Client.ps1 -NoDeploy`: 51.869 checks PASS e Release|x86 PASS.
  Validador das fichas e `git diff --check` scoped PASS.
  `STATICALLY VERIFIED / AUTOMATED TESTED`; `CLIENT-TESTED` segue vedado
  nesta máquina.
