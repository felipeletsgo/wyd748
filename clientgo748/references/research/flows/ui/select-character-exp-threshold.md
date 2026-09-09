---
id: select-character-exp-threshold
title: EXP atual e limiar do próximo nível na seleção de personagem
subsystem: ui-lifecycle
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# EXP atual e limiar do próximo nível na seleção de personagem

## Pergunta

Como o client nativo 7.48 preenche os controles de EXP atual e EXP necessária
para o próximo nível na tela de seleção de personagem, inclusive para
personagens de segunda classe?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`, com o SHA-256 do
  frontmatter.
- Projeto Ghidra: `WYD748Native_20260821.gpr`.
- Decompilação auxiliar:
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled\004a4582_FUN_004a4582.c`.
- Source atual:
  `client-source/tmproject/Projects/TMProject/TMSelectCharScene.cpp` e as
  tabelas em `Basedef.h`.
- Escopo: mutação textual local dos controles `1314` e `1315`; não há packet,
  ABI, loader, persistência ou regra server-side neste fluxo.

## Fluxo nativo 7.48

### Entrada observável

`FUN_004A4582` é alcançada pelo callback da cena de seleção registrado na
entrada de vtable/data `0x005A44BC`. A entrada observada no executável ocorre
com `0x201` (`WM_LBUTTONDOWN`) sobre um personagem selecionável. O receptor
obtém o slot selecionado e atualiza o painel com os dados desse personagem.

A source atual executa sua atualização correspondente quando `dwFlags == 514`
(`WM_LBUTTONUP`). Essa divergência de fase do clique fica registrada, mas não
integra o delta desta correção: mudar o evento exigiria uma ficha própria que
cobrisse os demais efeitos da seleção.

### Callers

- A entrada indireta da cena é materializada pela referência `0x005A44BC`.
- O callback recebe o evento do controle de personagem, resolve o slot e
  encaminha a atualização para `FUN_004A4582`.
- Não há caller de rede, timer ou worker; a transição é síncrona na thread da
  UI.

### Função principal

`FUN_004A4582` preenche separadamente os dois textos:

| Controle | Valor nativo |
| --- | --- |
| `0x522` / `1314` | EXP atual do personagem selecionado |
| `0x523` / `1315` | valor absoluto da tabela para o próximo nível |

O índice de nível armazenado seleciona o elemento seguinte da tabela. Na
source recompilável, a equivalência é
`g_pNextLevel[Score.Level + 1]` ou
`g_pNextLevel_G2[Score.Level + 1]`.

### Callees

- `FUN_0049AB73` fornece o comportamento base consumido pela atualização da
  cena.
- As operações de controle resolvem os IDs `1314` e `1315`, formatam os
  inteiros de 64 bits e publicam os textos de forma independente.
- A tabela G2 é selecionada quando a capa no equipamento 15 está entre
  `3197..3199`; os demais personagens usam a tabela normal.

### Saídas e erros

- Slot válido: ambos os controles são atualizados no mesmo ciclo.
- Seleção inválida ou branch que não resolve personagem: o callback não deve
  consumir Score, Equip ou EXP de um slot inexistente.
- O executável não usa o texto de EXP atual como fallback para `1315`; manter
  o buffer anterior produz informação incorreta, não um estado nativo válido.
- A source limita o índice ao último elemento disponível para que dados de
  nível inválidos ou máximos não causem leitura fora da tabela.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| clique em personagem | slot selecionável | callback -> `FUN_004A4582` | painel representa o slot | escreve `1314` e `1315` | slot inválido não é consumido |
| personagem comum | Equip 15 fora de `3197..3199` | tabela normal | limiar normal exibido | somente texto local | índice limitado ao array |
| segunda classe | Equip 15 em `3197..3199` | tabela G2 | limiar G2 exibido | somente texto local | índice limitado ao array |

### Vtables, vptrs e receptores

A entrada indireta foi fechada na referência `0x005A44BC`; `FUN_004A4582` é o
receptor específico da atualização e `FUN_0049AB73` é seu callee base. Nenhum
vptr ou offset de objeto é transportado para a source como ABI.

### Ownership

O agregado de seleção pertence ao `ObjectManager`; a cena apenas lê `Exp`,
`Score.Level` e `Equip[15]` do slot e altera os textos dos controles. Não há
alocação, transferência de ownership ou mutação do personagem.

### Falha parcial

Os dois textos são independentes. A source deve formatar o limiar antes de
chamar `SetText` em `1315`; reutilizar o buffer de `1314` deixa o painel
parcialmente atualizado. O clamp local evita que um nível fora da faixa
propague uma leitura inválida.

### Cleanup e teardown

Não aplicável: o fluxo não cria controles, buffers persistentes, callbacks ou
recursos. Ele apenas altera texto em controles já pertencentes à cena.

### Shutdown

Não aplicável pelo mesmo motivo; o shutdown dos controles pertence ao
lifecycle geral da cena de seleção.

### Logout e relogin

Não aplicável à mutação textual. Quando o agregado da seleção for novamente
materializado após logout/relogin, o mesmo callback lê os dados do novo slot;
nenhum estado desta atualização sobrevive à cena.

## Wire, ABI e recursos

Não há packet ou contrato wire neste fluxo. Os IDs `1314` e `1315` são
recursos existentes do layout 7.48. As tabelas C++ são dados locais da source;
nenhum endereço, packing ou layout de objeto do executável nativo é copiado.

## Mapeamento atual

### Source recompilável

`TMSelectCharScene.cpp` já preenchia `1314` com `m_stSelCharData.Exp[i]`, mas
os dois branches destinados a `1315` estavam vazios e reutilizavam o mesmo
`szValue`. A adaptação seleciona a tabela conforme Equip 15, usa
`Score.Level + 1`, limita o índice e formata o limiar antes de atualizar o
controle.

### WYD-Go

Não há mudança server-side. O servidor continua autoritativo sobre EXP e nível;
a tela apenas apresenta o agregado recebido e já armazenado para seleção.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject posterior/manual | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| EXP atual | `1314` recebe EXP | já implementado | compatível | fornece estado autoritativo | manter |
| próximo nível | `1315` recebe tabela `[Level+1]` | branches vazios | tabelas existentes | N/A | portar paridade nativa |
| segunda classe | capas `3197..3199`, tabela G2 | condição já reservava essas capas | inclui `573`, `1767`, `1770` | N/A | preservar extensões compatíveis |
| fase do clique | `WM_LBUTTONDOWN` observado | `WM_LBUTTONUP` | divergência preexistente | N/A | documentar; não alterar neste delta |

## Decisões

- Classificar o preenchimento de `1315` como `PARIDADE_NATIVA`.
- Preservar `573`, `1767` e `1770` como `MODERNIZACAO_COMPATIVEL`: elas usam
  a mesma tabela G2 sem alterar recurso, ABI ou wire do 7.48.
- Limitar o índice ao último elemento da tabela para não ler fora do array em
  nível máximo ou entrada inconsistente.
- Manter o evento atual da source; esta ficha não autoriza mudar toda a
  semântica de seleção de `WM_LBUTTONUP` para `WM_LBUTTONDOWN`.

## Lacunas

- Executar no `client748/project.exe` a seleção de um personagem comum e um
  personagem de segunda classe.
- Confirmar visualmente que `1314` contém a EXP atual e `1315` o limiar
  absoluto correto para cada tabela.
- A divergência de fase do clique deve ser investigada somente se produzir
  efeito observável incompatível nos demais campos ou animações da seleção.

## Validação

- Pesquisa: `FUN_004A4582`, entrada indireta `0x005A44BC`, callee
  `FUN_0049AB73`, IDs `1314/1315`, seleção normal/G2 e branches inválidos
  fechados no projeto Ghidra do hash registrado.
- Automação: `validate_research.py --repo .` aprovado com `CONTRACT=4`,
  `LOCATED=3` e `TRACED=1`.
- Client source: `Build-Client.ps1` aprovado com zero erros e zero avisos;
  candidato instalado em `client748/project.exe`, SHA-256
  `51AFF48D55D475E510A083CE8B469776E657BDBEA61E200BFA81FAD26E3C76CE`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
