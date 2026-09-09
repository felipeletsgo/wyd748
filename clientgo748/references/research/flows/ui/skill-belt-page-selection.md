---
id: skill-belt-page-selection
title: Selecionar a pagina ao soltar uma skill na barra
subsystem: ui
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-06
---

## Pergunta

Qual pagina recebe o atalho quando uma skill e solta na segunda barra?
Escopo estreito: escolha do deslocamento 0 ou 10, sem alterar wire ou skills.

## Fronteira de evidência

- UTILIZADA: projeto Ghidra WYD748Native_20260821 read-only/noanalysis,
  export `exports/skill-belt-page-flow.tsv`, SHA-256 conferido na linha program;
  execucao sem SCRIPT ERROR. Descompilacao estudada de FUN_00416196,
  FUN_004209FC, FUN_00435B13 e FUN_004470B9.
- UTILIZADA: source atual SGrid::SellItem/SellItem2 e
  TMFieldScene::InitializeCompatSkillBelts/UpdateSkillBelt, ResourceControl.h.
- UTILIZADA: recurso FieldScene2.bin materializado no log client-debug.log:
  grids 573/586 sob root 5745; grid 571 sob root 1905. O log comprova IDs,
  nao valida o executavel desta alteracao.
- UTILIZADA: WYD-Go onSetShortSkill/filterShortSkills e wire.SetShortSkill.
- NAO APLICAVEL: TMProject posterior/guias como prova nativa. W2PP, Secrets
  e Micronics excluidos. Nenhum asset ou campo de protocolo novo.

## Fluxo nativo 7.48

### Callers

FUN_004209FC chama FUN_00416196 em 00420EBD, apos hit-test e verificacao de
cursor pickup com item anexado, evento 0x202. O receptor e a grade atingida.

### Callees

FUN_00416196, ramo grid type 5, testa control ID 0x24A (586) e usa deslocamento
10, ou 0 nas outras grades. Retira o visual anterior via slot +0xA4, constroi
por FUN_0040D13E e adiciona via +0x8C. Depois de desanexar o cursor e destruir
o visual anterior, grava o indice em ShortSkill[x+deslocamento], envia por
FUN_0055F2DD e atualiza por FUN_004431E4/FUN_004470B9.
FUN_004470B9 reconstroi separadamente os dez slots de cada pagina, lendo
ShortSkill[0..9] e [10..19]. FUN_00435B13 vincula o controle 586 da segunda.

## Estado e lifecycle

A escolha de pagina e calculada por evento, sem memoria ou ownership novos.
O container e dono das grades; a FieldScene guarda bindings emprestados,
recriados por InitializeCompatSkillBelts a cada cena/relogin. A grade e dona
dos visuais, que sao retirados antes da substituicao e destruidos pelo caller.
Fechamento/shutdown continuam no teardown existente da cena; o delta nao
retém ponteiros, nao cria timers nem muda alocacao/falha parcial. A qualidade
do rollback de substituicao e uma frente separada, nao um claim desta ficha.

## Wire, ABI e recursos

Nenhuma alteracao. O emissor existente envia 0x378/32 com vinte atalhos; o
servidor filtra skills nao aprendidas e devolve o estado autoritativo. Esta
ficha rastreia a escolha de pagina, nao promove o contrato wire integral.
IDs nativos ja vinculados: 573 (primeira), 586 (segunda), botoes 587/588.

## Mapeamento atual

SellItem e SellItem2 comparavam m_dwControlID com 65645, ID posterior ausente
nas grades materializadas do 7.48. InitializeCompatSkillBelts ja ligava
m_pGridSkillBelt3 a 586, mas esses consumidores ignoravam o binding.

## Matriz de delta

| Claim | Nativo | Source anterior | Decisao |
| --- | --- | --- | --- |
| Segunda pagina | ID 586 seleciona +10 | teste 65645 selecionava +0 | usar identidade do binding m_pGridSkillBelt3 |
| Primeira pagina | +0 | +0 | manter |
| Ownership e envio | substituicao e array de 20 | mesmo fluxo | manter neste corte |

## Decisões

MODERNIZACAO_COMPATIVEL de binding local: usar a instancia ja resolvida pela
cena nos dois callers, evitando uma segunda tabela de IDs. O comportamento
de pagina resultante coincide com a transicao nativa rastreada. Sem alterar
classe, layout, vtable, recurso, emissor ou autoridade server-side.

## Lacunas

Testar drag nas duas paginas, selecao, troca por Z/587/588 e relogin no
project.exe final. Tratar separadamente rollback quando AddItem rejeitar.

## Validação

Pesquisa e inspeção da source confirmam a causa; build e testes do consumidor
server-side devem ser registrados no handoff. Nao CLIENT-TESTED.
