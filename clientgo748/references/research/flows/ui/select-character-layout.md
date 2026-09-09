---
id: ui-select-character-layout
title: Posicionamento do painel de selecao de personagem 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Posicionamento do painel de selecao de personagem 7.48

## Pergunta

Por que o painel de selecao de personagem do TMProject fica parcialmente fora
do viewport em resolucoes superiores, e qual formula o client nativo 7.48 usa
para compor esse painel?

## Fronteira de evidência

- Executavel: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 acima.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr` e
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Inicializacao da cena: `FUN_0049F0E7`.
- Transicao select/create: `FUN_004A250F`.
- Vtable da cena: `0x005A44B4`; slot `+0x68` em `0x005A451C` aponta para
  `FUN_004A250F`.
- Controle afetado: ID `0x502`/`1282`, `TMP_SELECT_CHARWIN` na source.
- Source adaptada: `TMSelectCharScene::VisibleSelectCreate`.
- Captura fornecida em 2026-09-01: o painel do candidato iniciava perto da
  borda direita e era cortado.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0049F0E7` escolhe `UI\\SelCharScene2.txt` no modo responsivo, carrega a
arvore de controles, resolve os controles `0x502` e `0x606` e chama o slot
virtual `+0x68` com argumento `1`. A vtable `0x005A44B4` resolve esse slot para
`FUN_004A250F`, produzindo a entrada visual na seleção de personagens.

### Callers

`CONFIRMED`: `FUN_0049F0E7` chama indiretamente `FUN_004A250F` pelo slot
virtual `+0x68` da vtable `0x005A44B4`, com argumento `1`, depois de carregar a
árvore e resolver os controles da cena. Na source, os callers de
`VisibleSelectCreate` cobrem inicialização, retorno da criação e retornos ao
modo de seleção.

Chamadas posteriores da mesma transicao alternam entre selecao (`1`) e criacao
(`0`) sem recriar o painel. A source possui o mesmo ponto vivo em
`VisibleSelectCreate`, usado na inicializacao, retorno da criacao e demais
retornos ao modo de selecao.

### Função principal

`FUN_004A250F` troca a camera entre `UI\\SelCamAction` e
`UI\\CreCamAction`, localiza os controles `0x501` a `0x606`, alterna sua
visibilidade e chama o slot de posicionamento `+0x70` dos controles. No branch
responsivo (`DAT_005B892C == 2`), o receptor `0x502` recebe a formula abaixo e
depois fica oculto, igual ao fluxo da source.

Os receptores finais sao controles ja pertencentes ao container da cena. Nao
ha alocacao, packet, persistencia ou estado de gameplay nesta transicao.

### Callees

`CONFIRMED`: `FUN_004A250F` chama a troca de câmera `FUN_004A24B4`, a busca de
controle pelo slot `+0x48`, a visibilidade pelo slot `+0x60`, o posicionamento
pelo slot `+0x70` e, ao entrar na criação, o foco do controle `0x1212` pelo
slot `+0x40`. Para o defeito atual, o receptor final relevante é o controle
`0x502`.

### Formula nativa

As constantes recuperadas do executavel sao:

- `_DAT_005A34A0 = 0.5f`;
- `_DAT_005A4150 = 0.25f`.

Para viewport `(W,H)` e dimensoes materializadas do controle `(PW,PH)`:

```text
x = W*0.25 - PW*0.5 + W*0.5
y = H*0.5 - PH*0.5

x = W*0.75 - PW*0.5
y = H*0.50 - PH*0.5
```

Em `1280x960`, com o controle escalado para `398.4x592`, o resultado e
`(760.8,184)`, ocupando aproximadamente `x=761..1159` e `y=184..776` sem
ultrapassar o viewport.

O branch nao responsivo usa literais `(218,547)`, mas nao representa o fluxo
observado com `SelCharScene2` em `1280x960`.

## Causa do delta

A source usava:

```text
x = WidthRatio * 650
y = HeightRatio * 40
```

Com a baseline correta de `800x600`, em `1280x960` isso produz `x=1040`.
Somada a largura escalada de `398.4`, a borda direita chega a `1438.4`,
reproduzindo o corte mostrado na captura. O defeito nao esta na baseline global
nem no asset; e uma coordenada posterior que substituiu a composicao nativa.

`SControl` multiplica posicao e tamanho pela razao do viewport durante a
construcao. Portanto, `m_nWidth` e `m_nHeight` usados na formula ja sao as
dimensoes renderizadas e nao devem receber nova escala.

## Estado e lifecycle

| Evento | Transicao | Resultado | Falha/saida |
| --- | --- | --- | --- |
| inicializar selecao | carrega `SelCharScene2`, resolve `0x502`, chama `+0x68(1)` | painel posicionado no quarto direito | controle ausente e tolerado na source |
| abrir criacao | `+0x68(0)` | oculta selecao e exibe controles de criacao | nao altera ownership |
| retornar da criacao | `+0x68(1)` | reaplica posicao e visibilidade de selecao | usa o mesmo controle materializado |
| entrar no mundo/logout | troca da cena corrente | arvore antiga e marcada para delecao | nenhum ponteiro do layout e retido |
| relogin | nova cena e novo container | formula e reaplicada a nova instancia | segue o mesmo guard de controle |
| teardown | destrutor da cena na vtable `0x005A44B4` | ownership normal da cena e liberado | a rotina de layout nao aloca recursos |

`FUN_0049F053` restaura a vptr `0x005A44B4`, libera o membro opcional em
`0x26E24` e delega ao teardown da base. Na source, `ObjectManager::SetCurrentScene`
marca a cena anterior como deletada e a entrega ao fluxo normal de remocao.

## Wire, ABI e recursos

Nao ha wire, ABI ou mudanca de recurso. A adaptacao transporta somente a
semantica de posicionamento comprovada para os tipos C++ atuais. IDs, arvore
`SelCharScene2`, cameras, visibilidade e ownership permanecem inalterados.

## Mapeamento atual

Classificacao: `PARIDADE_NATIVA`.

`TMSelectCharScene::VisibleSelectCreate` usa o viewport fisico e as dimensoes
ja escaladas de `pSelChar` para aplicar `W*0.75-PW*0.5` e
`H*0.5-PH*0.5`. Os botoes Create/Delete/Esc e o fallback inferior esquerdo nao
foram alterados.

WYD-Go nao participa deste layout local e nao requer mudanca correspondente.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source adaptada | Decisão |
| --- | --- | --- | --- | --- |
| posição responsiva de `0x502` | `W*0.75-PW*0.5`, `H*0.5-PH*0.5` | `ratio*650`, `ratio*40` | fórmula nativa com dimensões materializadas | portar sem mudar baseline |
| visibilidade | oculta `0x502` após posicionar no branch responsivo | ocultava depois de posicionar | preservada | não alterar |
| create/return | mesmo slot alterna controles e câmera | mesmo método alterna controles e câmera | preservado | alterar somente coordenada |
| ownership/teardown | container da cena; sem alocação no layout | container da cena | preservado | não criar estado novo |

## Decisões

- Manter a baseline global `800x600` e o recurso `SelCharScene2`.
- Portar apenas a fórmula nativa do branch responsivo.
- Usar `m_nWidth` e `m_nHeight` sem nova escala.
- Não alterar botões, câmeras, visibilidade, servidor, wire ou ABI.

## Lacunas

A correlação estática está fechada. Falta executar o candidato recompilado em
`1280x960`, inclusive os fluxos Create/return e logout/relogin, para confirmar
apresentação, hitboxes e reconstrução da cena.

## Validação

- `TRACED`: caller, funcao, vtable, slot, receptores, formula e teardown foram
  correlacionados.
- `IMPLEMENTED`: a formula foi adaptada na source ativa.
- `STATICALLY VERIFIED`: o validador de pesquisa e `git diff --check` passaram;
  o build `Release|Win32` v145 terminou com zero erros e zero warnings. O
  pipeline instalou `client748/project.exe` com SHA-256
  `C6184EBD938BC6120539958D049C1538DD3A27A0972108B635531A23E3C19254`.
- Ainda nao e `CLIENT-TESTED`: o fluxo precisa ser executado no candidato
  recompilado.

Teste em jogo obrigatorio:

1. entrar inicialmente na selecao em `1280x960` e confirmar o painel inteiro,
   centralizado no quarto direito;
2. abrir Create e retornar, confirmando posicao e hitboxes;
3. selecionar um personagem e entrar no mundo normalmente;
4. executar logout/relogin e confirmar que a nova cena reaplica o layout.

## Riscos restantes

- Uma alteracao futura das dimensoes do controle deslocara o painel pela mesma
  formula nativa; nao substituir por coordenadas absolutas.
- Promover para `CLIENT-TESTED` somente depois da confirmacao visual e dos
  fluxos Create/return e logout/relogin.
