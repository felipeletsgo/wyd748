# Handoff: paridade visual e funcional do client 7.48

Atualizado em: 2026-08-26
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
client748/project.exe                            | candidato source | 30DA37B94389AB6BF6EBBBE831278FFB62902591C4EEE035C3CDBB1340992044
```

Hashes acima foram reverificados em 2026-08-26. O hash de `project.exe` é
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
- `FUN_004110f5` confirma que a seleção de item para AutoTrade no Cargo de tipo
  10 abre o painel 626, foca o edit 627 e usa caption 630, enquanto o botão 667
  precisa estar visível. O adapter já traduzia os filhos, mas omitia o painel;
  `WYD748_TranslateControlID` agora mapeia `626 -> 65885`, eliminando o retorno
  silencioso de `SGridControl::TradeItem` sem alterar grid ou posicionamento.

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
crash NPC Skill Apprentice          | STATICALLY VERIFIED  | dump/PDB + controles 1889/1890/6049–6052/6128
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
Armadura Crítica tipo 50            | AUTOMATED TESTED       | semântica 31 / ícone 24 inclusive relog
grid/buffs do candidato 4BE5943C    | CLIENT-TESTED/FALHOU   | itens multisslot e nenhum ícone de buff
grid do candidato 44677EF8          | CLIENT-TESTED/FALHOU   | itens ainda invadiam células vizinhas
grid do candidato 41669031          | CLIENT-TESTED/FALHOU   | índice bruto exibiu esferas/pedras, mesh correta sumiu
build/instalação 5A4AEC0A           | AUTOMATED TESTED       | Release + assets + cópia/hash automáticos passaram
startup do candidato 1EEA1FC1       | CLIENT-TESTED/PASSOU   | captura em jogo confirmou inventário e equipamento renderizados
startup do candidato F8DA50A0       | CLIENT-TESTED/PASSOU   | captura em jogo confirmou entrada no mundo
startup do candidato 5A4AEC0A       | CLIENT-TESTED/PASSOU   | captura confirmou entrada e render no mundo
seleção AutoTrade 35DA2FB6          | STATICALLY VERIFIED    | painel nativo 626 traduzido para 65885; build/assets passaram
escala dos ovos 30DA37B9            | STATICALLY VERIFIED    | fórmula nativa por MaxZ; Release e assets passaram
paridade visual dentro do mundo     | NÃO TESTADO          | autenticação não automatizada
fechamento X/Esc e modais real      | NÃO TESTADO          | checklist client-ui-748.md
```

O rebuild Release Win32 com toolset v145 terminou com duas advertências
preexistentes em `SGrid.cpp` e zero erros para o candidato atual.
O artefato instalado e o output de build possuem o mesmo SHA-256
`30DA37B9...92044`; `Build-Client.ps1` instalou e verificou o candidato
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
```

Repetir build, instalação e hash se o código mudar.

## Pendências e riscos

- No candidato `A63DD267...922ABA`, abrir AutoTrade, adicionar o mesmo item que
  causou o dump, informar preço válido e confirmar que ele entra na loja sem
  crash nem mensagem indevida de item não vendável; repetir com preço inválido.
- Executar em jogo os fluxos de inventário, cargo, NPC shop, equipamento e
  drag válido/inválido, comparando com screenshots 7.48.
- Confirmar no candidato `30DA37B9...` que os ovos ficaram legíveis na loja,
  inventário, Cargo e durante o drag, sem invadir células vizinhas.
- A entrada no mundo e a centralização de inventário/loja/equipamento já foram
  confirmadas no candidato `5A4AEC0A...`; ainda validar notices `!`, Kibita, NPC
  Skill Apprentice e digitação/backspace no chat sem crash.
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

1. Ler `wyd-go-feature/SKILL.md`, `ghidra-client748.md` e
   `client-ui-748.md`.
2. Inspecionar `git diff` dos arquivos ativos sem reverter a worktree do usuário.
3. Testar primeiro a AutoTrade no candidato `A63DD267...922ABA`: adicionar o
   mesmo item, informar preço válido, repetir com preço inválido e abrir a loja.
4. No mesmo candidato, testar loja/inventário/Cargo/drag usando um ovo da
   família `egg001..egg014`; depois repetir `Steel_Pants`, armadura larga, arma
   longa e item pequeno para excluir regressão nos demais itens.
5. Testar entrada no mundo, digitação/backspace, notice e Kibita no candidato
   `5A4AEC0A...`, incluindo a abertura do NPC Skill Apprentice; depois testar
   Cargo, autoaproximação, HUD, câmera e fechamento em 1280×960.
6. Atualizar esta matriz item a item; usar `CLIENT-TESTED` somente após o fluxo
   real correspondente.
