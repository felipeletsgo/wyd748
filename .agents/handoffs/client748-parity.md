# Handoff: paridade visual e funcional do client 7.48

Atualizado em: 2026-08-27
Estado geral: `STATICALLY VERIFIED`

## Objetivo e limites

Tornar `client-source/tmproject` compatível apenas com o WYD 7.48, portando para
source os comportamentos comprovados pela referência histórica. Os scripts e
binários em `client748/wyd.exe nativo+patches/` são somente material de estudo e
não participam mais do build nem da instalação. O escopo atual concentra HUD compacto, render contextual de itens,
input/câmera e lifecycle de fechamento; paridade completa da UI ainda não foi
testada.

## Fontes e artefatos

```text
client748/wyd.exe nativo+patches/WYDoriginal.exe | stock histórico | B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C
client748/wyd.exe nativo+patches/WYD.exe         | referência Ghidra | 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
client748/project.exe                            | candidato source | 7BA846DAE559464491739B104903CE4E843BC57C8AED57EF00E368D5F7E27171
```

Hashes acima foram reverificados em 2026-08-27. O hash de `project.exe` é
volátil e deve ser recalculado depois de qualquer build.

## Evidência confirmada

- A política documental foi auditada em 2026-08-25: instruções e skills ativas
  não chamam scripts nem manifestos da cadeia histórica.
  `client748/project.exe` é o único executável aceito para validação; a pasta
  histórica permanece somente evidência estática.
- `Build-Client.ps1` instala automaticamente todo build bem-sucedido em
  `client748/project.exe` e compara o SHA-256 do output transitório com o
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
- `FUN_0040e817` comprova a redução uniforme da malha e
  `TMMesh::RenderForUI` comprova a centralização pelo AABB. A correção 1×1
  deliberadamente adapta a redução para a diagonal completa do AABB, com
  limite conservador de `0.27`, margem de 10% dentro da célula nativa de
  `0.3` e teto de escala `1.0`; itens pequenos nunca são ampliados.
- A comparação histórica entre os binários 7.48 altera somente
  `g_pItemGridXY` e a máscara de ocupação; ela não corrige escala nem posição de
  render. Essa evidência foi portada para a source, sem executar ou reutilizar
  os patches, e o candidato mantém a fórmula stock de `FUN_0040e817`.
- Inventário, Cargo, loja, trade/mix e item arrastado compartilham o helper de
  contenção por AABB. O painel irregular de equipamento continua centralizado
  em sua própria região e recebe escala explícita `1.0`, sem a contenção 1×1.
- Os meshes exclusivos `egg001..egg014` (`300..303` e `937..946`) são uma
  exceção comprovada: `FUN_0040e817` usa `min(1, 0.3 / MaxZ)`. Como todos têm
  `MaxZ = 0.48`, a escala visual passa de aproximadamente `0.191` para `0.625`,
  sem alterar centralização, footprint lógico 1×1 ou os demais itens.
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
- `client748/client-debug.log`: os seis controles existem sob parent 5716 nas
  posições registradas em `references/client-ui-748.md`.
- `client748/screenshot/Capture0084.jpg`: referência original de HUD e grid.
- O 7.48 materializa todos os ItemMix 1–6. A correlação confirmada é:
  Compositor/1/`0x3A6`, Aylin/2/`0x3B5`, Agatha/3/`0x3BA`,
  Tiny/4/`0x3C0`, Lindy e Odin/5/`0x2C3` e `0x2D2`, e
  Ehre/6/`0x2D3`. O modo alternativo `0x2C4` compartilha ItemMix5, mas está
  dormente: não há handler/receita autoritativa atual no servidor.
- Os controles 65857/65861+/81923/86019 são deriva da source mais nova; não são
  painéis ItemMix 7.48 disponíveis para novos sistemas.
- O candidato `B424BE8D...` caiu ao entrar no mundo. O dump
  `client748/client-crash-20260824-213257.dmp` registra AV de escrita em
  `0x0066C5B3`; removendo ASLR, `0x004BC5B3` resolve pelo PDB para
  `TMFieldScene::SetEquipGridState`, na escrita em `m_pGridNewSlot1 + 0x404`.
  O ponteiro é nulo porque os NewSlot 16/17 são controles posteriores ausentes
  do FieldScene2 7.48. O caller novo era o reset interativo de ItemMix durante
  `InitializeCompatFieldScene`.
- O candidato `2CAEA7C5...` passou da inicialização anterior, mas caiu ao receber
  um notice `!`. O dump `client748/client-crash-20260824-220158.dmp` registra AV
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
  `client748/client-crash-20260824-225343.dmp` registra leitura inválida em RVA
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
- O dump `client748/client-crash-20260825-151016.dmp` registra leitura nula em
  RVA `0xCC643`, resolvida pelo PDB para `TMFieldScene.cpp:23080` ao abrir o NPC
  Skill Apprentice. O bootstrap 7.48 não ligava `m_pSkillMSec1`; a árvore real
  `FieldScene2.bin` e `FUN_00435b13` confirmam root 1889, child 1890, lista 6128
  e textos 6049–6052. O candidato atual liga esses controles, desabilita a
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
  efeitos a partir de `0x50`. `client748/ItemList.bin` é o asset canônico 1×1:
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
contenção visual em célula 1x1      | AUTOMATED TESTED      | AABB completo; limite 0.27; escala nunca acima de 1
inventário/cargo/loja/trade/drag    | AUTOMATED TESTED      | helper compartilhado compilado em Release Win32
equipamento sem contenção 1x1       | AUTOMATED TESTED      | centro do controle + escala explícita 1.0
centralização candidato 2C92F658    | CLIENT-TESTED/FALHOU  | meia célula adicionada duas vezes em grades regulares
correção de posição 1EEA1FC1        | CLIENT-TESTED/FALHOU  | inventário subiu meia célula; equipamento ficou 12 px à direita
correção de posição F8DA50A0        | CLIENT-TESTED/FALHOU  | meia largura omitida deslocou todos os itens para a esquerda
correção de posição 5A4AEC0A        | CLIENT-TESTED/PASSOU  | captura confirmou grades e equipamento centralizados
notificação translúcida 7.48        | STATICALLY VERIFIED  | recursos 178/259/260
Skill 1905 separada do NPC 1889     | STATICALLY VERIFIED  | TMFieldScene.cpp + Ghidra
crash NPC Skill Apprentice 20260825 | STATICALLY VERIFIED  | dump/PDB + controles 1889/1890/6049–6052/6128
crash NPC Skill Apprentice 20260827 | STATICALLY VERIFIED  | m_pHellgateStore nulo; lifecycle 1889/1905 protegido
layout/valores do HUD compacto      | STATICALLY VERIFIED  | texto sem stretch + IDs nativos
EXP nativa em quatro quartos        | STATICALLY VERIFIED  | controles 1171–1174
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
escala dos ovos 30DA37B9            | STATICALLY VERIFIED    | fórmula nativa por MaxZ; Release e assets passaram
paridade visual dentro do mundo     | NÃO TESTADO          | autenticação não automatizada
fechamento X/Esc e modais real      | NÃO TESTADO          | checklist client-ui-748.md
```

O rebuild Release Win32 com toolset v145 terminou sem erros para o candidato
atual; a compilação completa registrou 6 advertências preexistentes.
O artefato instalado e o output de build possuem o mesmo SHA-256
`E077183B...F3B4F9`; `Build-Client.ps1` instalou e verificou o candidato
automaticamente, sem patch. Os candidatos
`4BE5943C...`, `44677EF8...`, `41669031...`, `1EEA1FC1...` e `F8DA50A0...` foram
testados pelo usuário e reprovaram no grid/mesh; não atribuir essas falhas ao
novo binário antes do teste real.

## Worktree e arquivos ativos

- `client-source/tmproject/Projects/TMProject/SGrid.cpp` — grid ordinário usa
  `BASE_GetMeshIndex`, caixa local fixa 24×24, origem da célula e contenção
  compartilhada pela diagonal completa do AABB. Inventário, Cargo, loja,
  trade/mix e drag usam o helper; os meshes `egg001..egg014` usam a escala
  nativa por `MaxZ`; equipamento preserva escala `1.0`.
- `client-source/tmproject/Projects/TMProject/TMMesh.cpp` — sentinelas nativas
  `FLT_MIN`/`FLT_MAX` para AABB e pivô estáveis.
- `client-source/tmproject/Projects/TMProject/WYD748Assets.cpp` — o adapter do
  catálogo 7.48 preserva todos os efeitos exatamente como estão no asset; a
  normalização runtime de `EF_GRID` foi removida. O mesmo adapter traduz o
  painel nativo 626 para o prompt semântico 65885 usado pelo AutoTrade.
- `client-source/tmproject/Update-Client748ItemList.ps1` — ferramenta ativa e
  reproduzível que gera/verifica o catálogo canônico a partir da referência e
  permite mudanças somente nos bytes de valor de `EF_GRID`.
- `client748/ItemList.bin` — asset canônico 1×1, SHA-256
  `112C6CFA...AE8627E3`; todos os 1.980 `EF_GRID` têm valor zero.
- `client748/wyd.exe nativo+patches/` — material histórico somente para estudo;
  não editar nem executar os scripts desse diretório.
- `client-source/tmproject/Projects/TMProject/TMFieldScene.cpp` — HUD 7.48,
  seis ItemMix nativos, controles Help/affect/Skill Apprentice 7.48, linha de
  buffs no FieldScene2 clássico e autoaproximação de ataque.
- `client-source/tmproject/Projects/TMProject/TMHuman.cpp` — efeitos nativos
  Lighten, Magic Shield e Skill Amp restaurados conforme `FUN_00506f9d`.
- `client-source/tmproject/Projects/TMProject/EventTranslator.cpp` — wheel e
  sentido do zoom 7.48.
- `client-source/tmproject/Projects/TMProject/SControl.cpp` — cursor preserva o
  `GeomControl`, `SMessagePanel` usa recursos translúcidos e o input de texto
  não consulta seletores de chat exclusivos da 7.59.
- `client-source/tmproject/Projects/TMProject/TMScene.cpp` — painel compartilhado
  de login/notice/saída com altura nativa de 24 pixels.
- `client-source/AGENTS.md` e `references/client-ui-748.md` — gates de regressão
  para grid, transformação de UI e lifecycle de painéis.
- `internal/game/handlers.go` — resolução autoritativa do banqueiro para Cargo.
- `internal/game/coverage_more_test.go` — cobertura de bind, fallback, distância,
  visibilidade, DWORD alto e packets inválidos de Cargo.
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
  erros; output e `client748/project.exe` são idênticos com SHA-256
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
  erros; output e `client748/project.exe` são idênticos com SHA-256
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
  erros; output e `client748/project.exe` são idênticos com SHA-256
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
  erros; output e `client748/project.exe` =
  `A63DD267B9149B06F3CCFA0086893A45CADB5DB8EDEA0BE2814C3BF068922ABA`
- `Test-Client748Assets.ps1` passou com 6.500 itens, 3.584 texturas, 104 skills
  e 18 shaders; `git diff --check` passou, restando só avisos LF/CRLF
- correção da AutoTrade está `STATICALLY VERIFIED`; promover para
  `CLIENT-TESTED` somente após adicionar item, testar preço válido/inválido e
  confirmar a abertura da loja no client real
- `FUN_0040e817` reaberta para a escala de itens: a família exclusiva
  `egg001..egg014` agora usa `min(1, 0.3 / MaxZ)`, enquanto os demais itens
  preservam a contenção conservadora pela diagonal completa do AABB
- build Release Win32 v145 passou com duas advertências preexistentes e zero
  erros; output e `client748/project.exe` = `30DA37B9...92044`
- `Test-Client748Assets.ps1` passou com perfil 7.48, 6.500 itens, 3.584 texturas,
  104 skills e 18 shaders; `git diff --check` encontrou apenas avisos LF/CRLF
- `FUN_004110f5` reaberta para o clique de AutoTrade: grid type 10, painel 626,
  edit 627, caption 630 e botão 667 confirmados contra callers da source
- alias `626 -> 65885` compilado em Release Win32 v145 com zero warnings e zero
  erros; output e `client748/project.exe` = `35DA2FB6...FB176`
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
  contenção visual por AABB completo
- build Release Win32 v145 com 21 advertências preexistentes e zero erros depois
  da correção assimétrica dos anchors de grid/equipamento
- hashes do output e `client748/project.exe` = `5A4AEC0A...E48F6`, idênticos
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
  asset e contém meshes por AABB completo em todos os grids regulares; as
  capturas do usuário reprovaram sua posição por deslocamento de meia célula
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
  `2C9323E0...A18F0DC5`; `client748/mesh/ItemList.bin` permaneceu intacto
- `Test-Client748Assets.ps1` passou com o catálogo canônico 1×1; nenhum script
  do diretório histórico foi executado
- `MSG_UseItem` agora republica o slot fonte em recusas e rollbacks diretos de
  catálogo, Magical Pill, skill especial, summon, barra de gold e posição;
  tint/untint/Repliction também republicam o alvo restaurado e, quando vestido,
  o estado visual autoritativo. Nenhum arquivo de client/layout foi alterado.
- `go test ./internal/game -run 'TestOnUseItem'`, `go test ./internal/game`,
  `go test ./internal/wire ./internal/game` e `go vet ./...` passaram após o
  resync. `go test ./...` ficou bloqueado somente pela ausência local de
  `client748/Mounts-KR.json` e `client748/Costumes-KR.json`; `internal/game`
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
  `client748/project.exe`, SHA-256
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
- `client748/UI/UITextureSetList.txt` confirma que o set `446` é
  `NewUI_AutoTrade_BG`, com um item `143x50`. O índice anterior `512` estava
  fora do intervalo válido `0..511` e por isso não produzia imagem.
- As duas criações de `m_pAutoTradePanel` em `TMHuman.cpp` agora usam o asset e
  as dimensões nativas. Como o renderer percorre a lista em ordem inversa,
  construtor e `CreateControl()` inserem o texto antes do painel; assim o painel
  permanece como fundo e o título fica visível após recriação dos controles.
- `Build-Client.ps1` passou com 0 erros e 4 warnings C4018 preexistentes,
  instalou output idêntico em `client748/project.exe`, SHA-256
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
  `Build-Client.ps1` instalou output idêntico em `client748/project.exe`,
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
  `client748/Mounts-KR.json` e `client748/Costumes-KR.json` não existem.
- O build oficial Release Win32 v145 passou; após fechar o `project.exe` que
  bloqueava a cópia, instalou output idêntico em `client748/project.exe`,
  SHA-256 `93DA0B486941CF8B381068F1D87BE3320EECA830EAB8EE9E637B4E86AEDB684A`.
  `Test-Client748Assets.ps1` e `git diff --check` passaram. UI está
  `STATICALLY VERIFIED`; compra do servidor está `AUTOMATED TESTED`.
- Após cobrir a segunda rota de labels e o dispatch do clique esquerdo, o build
  oficial Release Win32 v145 passou com 0 erros e 6 warnings preexistentes. O
  output foi instalado em `client748/project.exe`, SHA-256
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
  `client748/project.exe`, SHA-256
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
  preexistentes. `build/Release/WYD.exe` e `client748/project.exe` são idênticos,
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
  em `client748/project.exe`, SHA-256
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
  e instalou output idêntico em `client748/project.exe`, SHA-256
  `CAF93919F4B1CBC9CE8EED10B4BFE56E860FF78978F20A31473EDDF35F79B8C4`.
  `Test-Client748Assets.ps1`, `go vet ./...`, `gofmt -l` e
  `git diff --check` passaram. `internal/game`, `internal/wire` e
  `internal/store` passaram com `-count=1`; `internal/data` conserva somente
  as duas falhas conhecidas pelos assets ausentes `Mounts-KR.json` e
  `Costumes-KR.json`. A correção permanece `STATICALLY VERIFIED` até o teste
  visual no candidato instalado.

## Crash ao abrir o NPC de habilidades em 2026-08-27

- O fluxo real de abrir o NPC de habilidades falhou no client e gerou
  `client748/client-crash-20260827-140525.dmp`, com 62.464.379 bytes e horário
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
  instalando output idêntico em `client748/project.exe`, SHA-256
  `F8251714775601720307940598522E6D2924E5C61DAB300728F949FE0C8A380B`.
  `Test-Client748Assets.ps1` passou com 6.500 itens, 3.584 texturas, 104 skills
  e 18 shaders; `git diff --check` não encontrou erro de whitespace.
- Estado: `STATICALLY VERIFIED`. Ainda é obrigatório testar no client abertura,
  X, reabertura, Esc, nova interação e clique nas habilidades.

## Paridade estática: `SetMyHumanMagic`

- `TMFieldScene::SetMyHumanMagic()` permanece vazio por paridade com
  `FUN_004431D9`, no-op nativo de 11 bytes sem callees.
- Os callers diretos são `FUN_004431E4`, `FUN_004528C5` e `FUN_00454763`.
- Não restaurar o sidecar `STRUCT_MOB::Magician`; o cálculo ativo usa
  `CurrentScore.MagicAmp` como representação canônica.
- Evidência: `inventory/set-my-human-magic-noop.md`. Estado do símbolo:
  `STATICALLY_EVIDENCED`/`PARITY_STATIC_EVIDENCE`, sem claim runtime.

## Pendências e riscos

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
- Executar em jogo os fluxos de inventário, cargo, NPC shop, equipamento e
  drag válido/inválido, comparando com screenshots 7.48.
- Confirmar no candidato `30DA37B9...` que os ovos ficaram legíveis na loja,
  inventário, Cargo e durante o drag, sem invadir células vizinhas.
- A entrada no mundo e a centralização de inventário/loja/equipamento já foram
  confirmadas no candidato `5A4AEC0A...`; ainda validar notices `!`, Kibita e
  digitação/backspace. A abertura do NPC Skill Apprentice falhou no candidato
  anterior; a correção `F8251714...A380B` ainda requer o fluxo real completo.
- Ao lado de um banqueiro, mover item inventário↔Cargo e conferir persistência,
  rejeição fora de alcance e atualização de gold/slots.
- Clicar para atacar um inimigo fora do alcance e confirmar aproximação até o
  range da arma, ataque e cancelamento seguro se o alvo morrer/desaparecer.
- Confirmar que o catálogo canônico e a contenção AABB fazem `Steel_Pants`, uma
  armadura larga, uma arma longa e um item pequeno ocuparem uma célula sem
  invadir vizinhas em inventário, Cargo, loja, trade/mix e drag; equipamento
  deve manter escala e posicionamento próprios sem regressão.
- Aplicar Armadura Crítica e buffs representativos com flags visuais (Haste,
  Magic Weapon, Shield, Critical e Poison); conferir efeito no avatar/observers,
  ícone 23×23 no topo, contagem de duração, hover e desaparecimento ao expirar.
- Confirmar que login, notices e a contagem após X usam o painel translúcido
  estreito, sem o fundo marrom alternativo.
- Confirmar highlight azul/vermelho completo em grid e equipamento.
- Validar EXP/ATT/DEF visualmente no HUD e após mudanças de score/skill/equip.
- Validar wheel, botão do meio, Alt+direito e contagem de cinco segundos após X.
- Percorrer o checklist de login, notice, servidor/canal, Character, venda,
  gold/preço, system menu, `X`, `Esc` e montarias KR.
- Testar cada artesão (Compositor, Aylin, Agatha, Tiny, Lindy, Odin e Ehre),
  incluindo staging, remoção, rejeição, sucesso, rollback e relogin.
- Não implementar `0x2C4` até recuperar uma receita e resposta autoritativas;
  ele é um modo dormente do ItemMix5, não uma UI livre.
- Testar em jogo uma recusa/rollback de `MSG_UseItem` e confirmar que origem e
  alvo voltam imediatamente; até isso, o conserto está `AUTOMATED TESTED`, não
  `CLIENT-TESTED`.
- Se houver crash, gerar minidump e aplicar o procedimento ASLR de
  `ghidra-client748.md`; não mascarar ponteiro nulo com widget 7.59.

## Próximo passo executável

1. No candidato `F8251714...A380B`, abrir o NPC Skill Apprentice, fechar pelo X,
   reabrir, fechar por Esc, interagir novamente e clicar nas habilidades.
2. Testar a placa sem hover e o título dentro da faixa escura no mesmo
   candidato `F8251714...A380B`.
3. Testar Cargo no novo candidato, abrindo antes e
   depois de AutoTrade/loja para excluir posições residuais, em 1024x768 e
   1280x960.
4. Testar a AutoTrade no mesmo candidato: adicionar o
   mesmo item, informar preço válido, repetir com preço inválido, conferir a
   placa `NewUI_AutoTrade_BG` junto do título e, ao abrir por outro personagem,
   conferir fundo/preço, confirmação `Não`/`Sim`, avisos, remoção imediata da
   placa após o último item e alinhamento do Carry.
5. No mesmo candidato, testar loja/inventário/Cargo/drag usando um ovo da
   família `egg001..egg014`; depois repetir `Steel_Pants`, armadura larga, arma
   longa e item pequeno para excluir regressão nos demais itens.
6. Depois do teste do NPC no novo candidato, repetir entrada no mundo,
   digitação/backspace, notice e Kibita; depois testar Cargo, autoaproximação,
   HUD, câmera e fechamento em 1280×960.
7. Atualizar esta matriz item a item; usar `CLIENT-TESTED` somente após o fluxo
   real correspondente.
