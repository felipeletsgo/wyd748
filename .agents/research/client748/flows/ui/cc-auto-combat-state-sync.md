---
id: cc-auto-combat-state-sync
title: Sincronização dos controles e do estado do C.C
subsystem: ui-gameplay
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-13
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
