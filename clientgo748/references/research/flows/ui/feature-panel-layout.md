---
id: ui-feature-panel-layout
title: Composição lado a lado de Character, Skill e Inventory 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Composição lado a lado de Character, Skill e Inventory 7.48

## Pergunta

Por que os painéis Character, Skill e Inventory carregam sobrepostos a partir
de `FieldScene2.bin`, e como o client nativo 7.48 recompõe as três janelas lado
a lado depois do carregamento do recurso?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 acima.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr` e
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Inicializador nativo: `FUN_00435b13`, export auxiliar
  `00435b13_FUN_00435b13.c`.
- Recurso ativo: `client748/UI/FieldScene2.bin`.
- Source adaptada: `TMFieldScene::InitializeCompatFieldScene` e
  `TMFieldScene::PositionCompatFeaturePanels`.
- Capturas fornecidas em 2026-09-01: o candidato TMProject abriu as três
  janelas na mesma posição; o client original mostrou a composição
  `Character -> Skill -> Inventory`.

O recurso e o export textual são aceleradores. Os valores usados na fórmula
foram confirmados no executável correspondente ao hash registrado, não
inferidos apenas pela aparência da captura.

## Estado serializado

`CONFIRMED`: os três roots possuem a mesma posição em `FieldScene2.bin`; logo,
o arquivo sozinho não contém a composição final observada em jogo.

| Janela | Control ID | Posição no recurso | Tamanho |
| --- | ---: | ---: | ---: |
| Character | `513` | `(530, 0)` | `227 x 421` |
| Skill | `1905` | `(530, 0)` | `227 x 421` |
| Inventory | `257` | `(530, 0)` | `227 x 421` |

Tratar `(530,0)` como posição final reproduz exatamente a sobreposição do
candidato. Editar os três roots no asset descartaria o comportamento dinâmico
do nativo e fixaria a solução em uma resolução.

## Fluxo nativo 7.48

### Entrada observável

`FUN_00435b13` roda durante a construção da FieldScene, depois que a árvore de
controles foi materializada. A função encontra os roots `513`, `1905` e `257`
e substitui suas posições serializadas por coordenadas calculadas com o
viewport e as dimensões de cada painel.

### Callers

`CONFIRMED`: o caller pertence ao inicializador da FieldScene que executa
`FUN_00435b13` após carregar a árvore de `FieldScene2`. Não há caller de tecla
ou abertura individual reposicionando os roots; a composição pertence à
construção da cena.

### Função principal

`FUN_00435b13` obtém as dimensões do viewport e dos três roots, calcula suas
coordenadas e grava a composição inicial. As constantes recuperadas do
executável são:

- `_DAT_005A34A0 = 0.5f`;
- `_DAT_005A430C = 1.5f`;
- `_DAT_005A3670 = 10.0f`.

### Callees

`CONFIRMED`: os receptores finais são os roots de `SPanel` correspondentes aos
IDs `513`, `1905` e `257`. Os callees relevantes consultam largura/altura do
viewport e do controle e atualizam `x/y` pela operação equivalente a `SetPos`.
Não existe callee de rede, persistência ou gameplay.

### Saídas e erros

A saída observável é a composição horizontal inicial das três janelas. Root
ausente não recebe escrita; os demais continuam posicionados. O cálculo não
aloca memória, não produz resposta de packet e não cria estado assíncrono.

Não há packet, estado de gameplay ou dependência do servidor nesta transição.
O client apenas apresenta painéis locais já pertencentes à cena corrente.

### Fórmulas

Para viewport `(W,H)` e dimensões do painel `(PW,PH)`, o nativo aplica:

```text
Character.x = W*0.5 - PW*1.5 - 10
Character.y = H*0.5 - PH*0.5

Skill.x = W*0.5 - PW*0.5
Skill.y = H*0.5 - PH*0.5

Inventory.x = W*0.5 + PW*0.5 + 10
Inventory.y = H*0.5 - PH*0.5
```

Com `800x600` e painéis de `227x421`, o resultado é:

```text
Character = (49.5, 89.5)
Skill     = (286.5, 89.5)
Inventory = (523.5, 89.5)
```

Os intervalos horizontais são de dez pixels e as três janelas compartilham o
mesmo centro vertical, correspondendo à captura do client original.

## Estado e lifecycle

| Evento | Precondição | Transição | Resultado | Falha/saída |
| --- | --- | --- | --- | --- |
| construção da FieldScene | recurso e container carregados | vincula 513/1905/257 e calcula posições | roots lado a lado | root ausente é ignorado |
| abrir C, S ou I | root já posicionado na cena | alterna visibilidade | posição calculada é preservada | painel ausente não é aberto |
| fechar e reabrir | mesma cena | alterna visibilidade novamente | não força posição inicial outra vez | sem estado adicional |
| troca de cena/logout | árvore antiga destruída | teardown normal do container | roots antigos deixam de existir | nenhum callback retém os painéis |
| relogin | nova FieldScene construída | fórmula é reaplicada aos novos roots | layout limpo para a nova cena | segue os mesmos guards de construção |

O posicionamento ocorre uma vez por construção da cena, não a cada atalho de
abertura. Isso reproduz a composição inicial nativa sem apagar uma eventual
movimentação manual durante a vida da mesma cena.

## Wire, ABI e recursos

Não há wire nem alteração de ABI. O recurso ativo permanece
`FieldScene2.bin`, inclusive suas três posições serializadas `(530,0)` e os
IDs nativos `513`, `1905` e `257`. A adaptação transporta somente a semântica
de layout dinâmico; não transporta offsets, packing ou vtables do executável.

## Mapeamento atual

### Source recompilável

Classificação: `PARIDADE_NATIVA`.

`TMFieldScene::PositionCompatFeaturePanels()` aplica as três fórmulas usando
`g_pDevice->m_dwScreenWidth`, `g_pDevice->m_dwScreenHeight` e as dimensões dos
roots materializados. A chamada ocorre em `InitializeCompatFieldScene()` após
os bindings de Inventory (`257`), Character (`513`) e Skill (`1905`).

Os guards limitam a adaptação ao caminho `m_bCompatFieldScene`, exigem device
válido e ignoram individualmente roots nulos. Nenhum offset, vtable ou layout
de objeto do executável foi transportado; somente a semântica comprovada de
composição foi adaptada aos tipos C++ atuais.

### WYD-Go

Não aplicável: esta é uma composição visual local. O servidor não recebe nem
persiste posição ou visibilidade desses painéis.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| posição serializada | três roots em `(530,0)` | preserva o asset | asset importado coincide | N/A | não editar o recurso |
| composição inicial | runtime posiciona C/S/I lado a lado | helper após bindings | caminho 7.59 não recompunha roots 7.48 | N/A | portar fórmula nativa |
| reabertura | preserva posição da cena | helper roda uma vez por cena | não usar reposição por atalho | N/A | preservar lifecycle nativo |

## Decisões

- Manter os roots e IDs nativos no recurso.
- Reaplicar a fórmula em runtime somente no inicializador compatível.
- Usar viewport e dimensões materializadas, sem coordenadas absolutas.
- Não alterar servidor, wire ou ABI para um comportamento estritamente visual.

## Lacunas

A correlação estática está fechada. Falta somente validar visualmente o
candidato recompilado ao abrir, fechar/reabrir e reconstruir a cena por
logout/relogin, inclusive em resolução superior.

## Validação

- `TRACED`: recurso, função nativa, constantes, receptores e lifecycle foram
  correlacionados.
- `IMPLEMENTED`: a composição foi adicionada à source ativa.
- `STATICALLY VERIFIED`: o validador de pesquisa e `git diff --check` passaram;
  o build `Release|Win32` v145 terminou com zero erros e 21 warnings
  preexistentes. O pipeline instalou `client748/project.exe` com SHA-256
  `CD92A005EBDAB0DF21D9BF5B1CB1C1FC593F048BB9011802B67D2F71539BF40B`.
- Não é `CLIENT-TESTED`: build não comprova apresentação ou interação.

Teste em jogo obrigatório:

1. abrir C, S e I simultaneamente e confirmar a ordem
   `Character -> Skill -> Inventory`, sem sobreposição;
2. fechar e reabrir cada janela na mesma cena;
3. repetir após logout/relogin;
4. repetir em resolução superior para confirmar centralização dinâmica.

## Riscos restantes

- Um root ausente é tolerado para preservar a inicialização parcial, mas deixa
  a janela correspondente indisponível; isso deve aparecer como outro defeito
  de binding, não ser mascarado por um controle inventado.
- Alterações futuras no tamanho real dos roots mudam a composição pelo mesmo
  cálculo nativo; não substituir por coordenadas absolutas de `800x600`.
- Promover para `CLIENT-TESTED` somente com evidência visual do candidato
  recompilado e do fluxo de relogin.
