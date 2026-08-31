---
id: cc-auto-combat-state-sync
title: Sincronização dos controles e do estado do C.C
subsystem: ui-gameplay
status: UNMAPPED
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
- Recursos: IDs modernos `B_CCMODE_*` e controles antigos
  `B_CCATTACK`/`B_CCPOTION`/`B_CCMOVE`; o layout compatível é carregado de
  `FieldScene2`.
- Servidor: nenhuma regra de combate, packet ou estado autoritativo foi
  alterado por este lote.
- Executável nativo: o hash do frontmatter identifica a baseline. Esta ficha
  não afirma que a topologia moderna de controles existia no binário 7.48.

## Fluxo nativo 7.48

O guia oficial confirma a funcionalidade e seus quatro modos, mas esta ficha
ainda não atribui endereço ou função nativa ao sincronizador de apresentação.
Consequentemente, nenhuma textura, ID de controle ou topologia moderna abaixo
é apresentada como claim do executável 7.48. O estado documental permanece
`UNMAPPED`; isso não bloqueia a modernização interna que preserva os contratos
externos já observados.

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
- `m_pSGameAutoBtn`, `m_pCCPotionBtn` e `m_pCCFeedBtn` são vinculados quando o
  recurso os fornece. `m_pccmode`, `m_pCCModeHpSte` e
  `m_pCCModeMountSte` começam nulos e permanecem opcionais.
- Inicialização parcial não derruba a cena: cada atualização tolera ausência
  de painel, botão, texto alternativo ou humano local.
- O lote não aloca controles e não cria ownership novo. O teardown continua
  pertencendo ao container da Field; não há callback ou buffer persistente
  adicional para limpar em logout, relogin ou shutdown.
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
| modos `0..3` | guia oficial descreve off/físico/mágico/Not attack | `GameAuto()` já consome os quatro | controles modernos editam o mesmo global | regras inalteradas | preservar |
| apresentação | layout compatível pode ter só controles antigos | handlers duplicados e divergentes | painel moderno é opcional | N/A | sincronizador único e null-safe |
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
- Manter esta ficha em `UNMAPPED`: a funcionalidade source foi fechada, mas a
  topologia nativa ainda não foi atribuída a funções/endereços e os controles
  não foram testados visualmente.

## Lacunas

- Confirmar em runtime os quatro modos, as quatro opções de poção, as três
  opções de posição e os thresholds de HP/mount.
- Confirmar a ordem visual dos tooltips `232..234`.
- Verificar quais IDs C.C são materializados por `FieldScene2`; criar apenas
  os controles mínimos ausentes, como extensão, se o painel for desejado.
- Exercitar logout/relogin e reconstrução da Field com C.C ligado.

## Validação

- Pesquisa: guia oficial, fluxo source vivo e duplicações de handlers
  comparados; nenhuma alegação de topologia nativa foi promovida.
- Automação: `git diff --check` aprovado antes desta ficha.
- Build: `Build-Client.ps1` aprovado em Release/Win32 com zero erros e 21
  warnings preexistentes; candidato instalado em `client748/project.exe`,
  SHA-256 `8CD73ED35D59482C27EC3760C59D05EC44B5DAB4ACE39DFFAD540AF9D4A28002`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
