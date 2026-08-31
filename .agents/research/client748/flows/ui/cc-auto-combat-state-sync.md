---
id: cc-auto-combat-state-sync
title: Sincronização dos controles e do estado do C.C
subsystem: ui-gameplay
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
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
  `B_CCATTACK`/`B_CCPOTION`/`B_CCMOVE` e seletores nativos `318`/`319`; o
  layout compatível é carregado de `FieldScene2`.
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
  de painel, botão, texto alternativo ou humano local.
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
- Automação: `git diff --check` aprovado antes desta ficha.
- Build: `Build-Client.ps1` aprovado em Release/Win32 com zero erros e 21
  warnings preexistentes; candidato instalado em `client748/project.exe`,
  SHA-256 `CE4F2775B382200601EAF59ABA4271D4EDC271C2A072EE29915AFB1B0E525F94`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
