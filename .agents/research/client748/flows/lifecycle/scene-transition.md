---
id: scene-transition
title: Criar e substituir a cena ativa no lifecycle 7.48
subsystem: lifecycle
status: LOCATED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-28
---

# Criar e substituir a cena ativa no lifecycle 7.48

## Pergunta

Como o client nativo WYD 7.48 converte o estado solicitado em uma cena,
seleciona e inicializa a cena nova, preserva a mensagem entre cenas e agenda a
cena anterior para destruição? Quais partes da implementação do TMProject
7.69+ podem ser aproveitadas sem importar contrato ou layout de versão nova?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`.
- SHA-256: `8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593`.
- Projeto Ghidra: `WYD748Native_20260821.gpr`; corpus auxiliar com 4.146
  funções em `GhidraAnalysis/20260821/decompiled`.
- Exports focados versionados:
  `.agents/research/client748/exports/scene-transition-focused-next.tsv`,
  `scene-lifecycle-network-focused.tsv`, `scene-lifecycle-helpers.tsv` e
  `scene-lifecycle-missing-helpers.tsv`.
- Source recompilável consultada: `client-source/tmproject/Projects/TMProject/`
  (`ObjectManager`, `TMScene`, `TreeNode` e `NewApp`).
- Servidor: não há regra server-side direta para troca de cena local; qualquer
  intenção posterior continua sujeita à autoridade do WYD-Go.

Os exports são aceleradores de busca. Vtables, xrefs indiretos, receptores,
teardown e transições de logout/relogin ainda precisam ser reabertos e fechados
no projeto Ghidra antes de transformar esta ficha em contrato de implementação.

## Fluxo nativo 7.48

### Entrada observável

`LOCATED`: o lifecycle da aplicação passa pela vtable em `0x005A6104` e pelo
loop de mensagens/input/rede em `FUN_0055DAB8`. A inicialização geral de
subsistemas está localizada em `FUN_0055CA18`, o bootstrap/configuração em
`FUN_0055BC0A` e o shutdown em `FUN_0055D066`. A criação/troca de cena converge
para `FUN_004B3500` e a substituição da cena ativa para `FUN_004B37C9`.

### Callers

`LOCATED`: `FUN_004B3500` é exposta pelo slot `+0x54` da vtable efetiva do
`ObjectManager` em `0x005A45FC`; `FUN_004B37C9` ocupa o slot `+0x58`. A
instância instala essa vtable por `FUN_004AFAC0` e `FUN_004AFBA0`. A entrada
externa que fornece cada estado e a cadeia completa de chamadas indiretas ainda
não foram fechadas no projeto Ghidra. O endereço `0x005A45F0` é um preâmbulo
adjacente, não a base da vtable.

### Função principal: `FUN_004B3500`

`CONFIRMED`: o estado solicitado `9` é convertido para o estado interno `0`.
Depois da conversão, somente os estados internos `0`, `5`, `7` e `8` entram na
seleção de construtor observada. Os quatro construtores nativos localizados são:

| Estado interno | Construtor observado | Claim |
| --- | --- | --- |
| `0` | `FUN_004A8CCF` | `CONFIRMED` |
| `5` | `FUN_00431D00` | `CONFIRMED` |
| `7` | `FUN_0049EE30` | `CONFIRMED` |
| `8` | `FUN_004343A4` | `CONFIRMED` |

Cada construtor instala seu próprio vptr/vtable. A cena nova é selecionada por
`FUN_004B37C9` e inicializada por uma chamada virtual no slot `+0x4C` do
receptor. Em sucesso, a cena é anexada à árvore pelo helper `FUN_0054AC09`,
usando a raiz localizada em `manager + 0x1B07C`.

`CONFIRMED`: quando a criação não produz uma cena, o nativo mostra uma mensagem
de erro e agenda o fechamento da janela por `PostMessageA`. Quando a cena existe,
mas sua inicialização falha, a cena parcialmente criada é destruída quando
aplicável, uma mensagem de erro é exibida e o mesmo fechamento é agendado.

### Substituição: `FUN_004B37C9`

`CONFIRMED`: a função salva a cena atual, grava a cena nova em
`DAT_0067CF38`, marca a cena anterior para deleção e transfere para a cena nova
o estado de controles da mensagem em `scene + 0x84`. Ela também copia estado e
tempo relacionados à mensagem e encaminha a cena anterior ao slot
`ObjectManager + 0x64`. A identidade do receptor final foi resolvida pelas
instruções como o próprio `ObjectManager`.

`LOCATED`: a cadeia posterior que consome a marca de deleção, a ordem completa
de destruição e a relação entre a cena global e a árvore ainda não estão
fechadas. Portanto, a função não autoriza uma alteração equivalente na source
enquanto a ficha permanecer neste estado.

### Callees e helpers

- `FUN_004A8CCF`, `FUN_00431D00`, `FUN_0049EE30` e `FUN_004343A4`: construtores
  das quatro cenas selecionadas; cada receptor precisa ser reaberto para fechar
  sua inicialização e destrutor.
- Virtual `+0x4C`: inicialização da cena nova; o retorno e todos os efeitos de
  falha ainda precisam ser confirmados no receptor de cada vtable.
- `FUN_0054AC09`: anexa um objeto/cena à árvore de ownership.
- Slot `ObjectManager + 0x64`: recebe a cena anterior no caminho de limpeza;
  a implementação, o consumidor de `m_cDeleted` e a ordem de teardown ainda
  precisam ser confirmados diretamente.
- `FUN_004B16C0` e `FUN_00494C00`: localizados na cadeia de consumo/limpeza e
  destruição-base, mas ainda não fechados para todos os tipos de cena e todos os
  caminhos de saída.
- `FUN_00423CA0`, `FUN_00423DD8`, `FUN_004BAC70` e `FUN_004BACFF`: helpers de
  transporte/socket e timer localizados no lifecycle adjacente; sua interação
  completa com logout, relogin e troca de cena permanece pendente.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| Estado solicitado `9` | dispatcher/bootstrap fornece o estado | `FUN_004B3500` | estado interno `0` | câmera e cena são preparados | cadeia de entrada ainda `LOCATED` |
| Estado interno `0`, `5`, `7` ou `8` | estado aceito | construtor específico; virtual `+0x4C` | cena nova criada/inicializada | vptr próprio, seleção da cena | falha parcial destrói cena e fecha janela |
| Cena criada com sucesso | cena inicializada | `FUN_0054AC09` | cena sob a raiz `manager+0x1B07C` | ownership passa à árvore | consumidor de deleção pendente |
| Cena anterior existente | cena atual não nula | `FUN_004B37C9` | nova cena global; anterior marcada | mensagem/tempo são copiados | teardown posterior não fechado |
| Loop de aplicação | janela e subsistemas ativos | `FUN_0055DAB8` | mensagens/input/rede processados | dispatch Win32 e timer | branches `0x464/0x465` ainda parcialmente resolvidos |
| Shutdown | aplicação em encerramento | `FUN_0055D066` | subsistemas em teardown | janela, socket e objetos são finalizados | ordem completa ainda não confirmada |
| Logout/relogin | transição de sessão | não resolvido | não resolvido | não resolvido | bloqueia `TRACED`/`CONTRACT` |

### Vtables, vptrs e receptores

Vtable da aplicação em `0x005A6104`:

```text
+0x00 FUN_0055F3E0
+0x04 FUN_0055BC0A
+0x08 FUN_0055D066
+0x0C FUN_0055D345
+0x10 FUN_0055EDF7
+0x14 FUN_0055D6E6
+0x18 FUN_0055EE1E
+0x1C FUN_0055EE45
```

Vtable efetiva do `ObjectManager` em `0x005A45FC`:

```text
+0x54 FUN_004B3500
+0x58 FUN_004B37C9
+0x5C FUN_004B3952
+0x60 FUN_004B21F0
+0x64 FUN_004B21C9
+0x68 FUN_004B27DA
```

`FUN_004AFAC0` e `FUN_004AFBA0` instalam o vptr efetivo do `ObjectManager`.
Os quatro construtores de cena acima instalam vtables próprias; os endereços
completos dos respectivos receptores e a correspondência de cada slot ainda
precisam ser confirmados no projeto Ghidra.

### Ownership

`CONFIRMED`: após inicialização bem-sucedida, `FUN_0054AC09` anexa a cena à
árvore cujo root é derivado de `manager + 0x1B07C`. A cena anterior não é
liberada imediatamente durante a substituição: ela é marcada para deleção e
encaminhada ao caminho do `ObjectManager`. A relação entre `DAT_0067CF38`, a
cena global, a raiz e o owner de cada filho permanece parcialmente resolvida.

### Falha parcial

`CONFIRMED`: criação nula e falha de inicialização têm diagnóstico visível e
solicitam fechamento da janela. Falha de inicialização também contempla a
destruição da cena parcial quando aplicável. Ainda não foi fechado se efeitos,
controles ou filhos alocados antes da falha exigem uma limpeza adicional por
tipo de cena.

### Cleanup e teardown

`LOCATED`: `FUN_004B16C0`, `FUN_00494C00` e o slot `+0x64` formam a cadeia de
limpeza localizada. Ainda faltam o consumidor integral de `m_cDeleted`, a
ordem entre remoção da árvore e destrutor-base, os quatro destrutores, a
desativação de input/foco e os caminhos de teardown após erro, troca, logout e
relogin. Essa lacuna impede promover a ficha a `TRACED`.

### Shutdown

`LOCATED`: `FUN_0055D066` é a entrada de shutdown na vtable da aplicação;
`FUN_0055DAB8` mantém o loop de mensagens, input, timer e rede; e
`FUN_0055CA18` inicializa os subsistemas antes desse loop. Socket e timer foram
localizados em `FUN_00423CA0`/`FUN_00423DD8` e
`FUN_004BAC70`/`FUN_004BACFF`, mas a ordem de desmontagem e a convergência com
as cenas ainda não foram demonstradas.

### Logout e relogin

`UNRESOLVED`: não foi fechada uma transição observável de logout/relogin que
prove a limpeza e a reconstrução da árvore, do estado global da cena, dos
controles e dos recursos de transporte. Não portar comportamento moderno para
preencher essa lacuna.

## Wire, ABI e recursos

Esta transição não estabelece, por si só, um packet ou contrato wire. Os
endereços, vptrs, slots e offsets registrados são layout/ABI interno do
executável nativo 7.48 e não devem ser copiados para structs da source ou para
o servidor. O recurso relevante é a árvore de cenas/objetos; nenhum ID de UI,
loader ou layout de recurso novo foi inferido a partir do TMProject.

## Mapeamento atual

### Source recompilável

O equivalente semântico atual está em:

- `client-source/tmproject/Projects/TMProject/ObjectManager.cpp:701-764`:
  `SetCurrentState` converte `TM_FIELD2_STATE` para `TM_NONE_STATE`, cria
  `TMFieldScene`, `TMSelectCharScene`, `TMSelectServerScene` ou `TMDemoScene`,
  inicializa, anexa ao root e reporta falhas.
- `client-source/tmproject/Projects/TMProject/ObjectManager.cpp:766-783`:
  `SetCurrentScene` troca a cena global, marca a anterior, copia o estado do
  painel de mensagens e chama `DeleteObject`.
- `client-source/tmproject/Projects/TMProject/ObjectManager.cpp:924-963`:
  `CleanUp` percorre a árvore, consome `m_cDeleted` e libera
  `m_pPreviousScene`.
- `TMScene.cpp` e `TreeNode.cpp`: constroem containers/filhos e mantêm a lista
  intrusiva de ownership. Esses arquivos são a implementação atual, não prova
  do layout nativo 7.48.

### WYD-Go

Não existe equivalente server-side direto para a troca de cena local. O Go é
autoritativo apenas para as intenções de gameplay que surgem depois da entrada
no mundo; conexão, login, sessão, permissões, estado de personagem,
inventário, observers e persistência continuam sendo validados no servidor.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject 7.69+ | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Conversão de estado | `9 -> 0` | `TM_FIELD2_STATE -> TM_NONE_STATE` | semântica homônima sugerida | não aplicável | confirmar enum/entrada 7.48 antes de portar |
| Estados que criam cena | `0`, `5`, `7`, `8` | quatro classes de cena | referência semântica posterior | não aplicável | manter no estudo; não alterar enquanto `LOCATED` |
| Inicialização | virtual `+0x4C` | `InitializeScene()` | pode ter outra ABI | não aplicável | adaptar só com receptor/retorno 7.48 fechados |
| Ownership | root `manager+0x1B07C` e `FUN_0054AC09` | `m_pRoot->AddChild` | árvore moderna não decide offset | não aplicável | preservar decisão nativa após fechar teardown |
| Troca/limpeza | marca anterior e slot `+0x64` | `m_cDeleted`, `DeleteObject`, `CleanUp` | lifecycle posterior é pista | não aplicável | fechar consumidor e destrutores antes de editar |
| Falha | mensagem, destruição parcial e `PostMessageA` | `MessageBox`/`PostMessage` equivalentes | não decide erro 7.48 | não aplicável | manter como claim nativa; testar somente após contrato |

## Decisões

- Manter esta ficha em `LOCATED`. A criação e a substituição foram localizadas,
  mas caller externo completo, receptores virtuais, teardown, shutdown e
  logout/relogin ainda não estão fechados.
- Não alterar `ObjectManager`, `TMScene`, `TreeNode` ou `NewApp` por causa da
  semelhança observada. A implementação do TMProject 7.69+ é referência
  semântica secundária, não contrato 7.48.
- Não importar endereço, offset, vtable, enum, recurso ou ABI do TMProject para
  o client 7.48. O delta `9 -> 0` só poderá ser adaptado depois de a entrada e o
  consumidor da transição estarem comprovados.
- Não declarar `TRACED`, `CONTRACT`, `IMPLEMENTED` ou `CLIENT_TESTED` por causa
  de export, build ou correspondência estrutural.

## Lacunas

- callers reais e transições que fornecem os estados a `FUN_004B3500`;
- receptores/retornos completos do slot virtual `+0x4C` para as quatro cenas;
- destrutores específicos das quatro classes e consumidor de `m_cDeleted`;
- ordem de detach, remoção da árvore, destrutor-base e liberação de
  `m_pPreviousScene`;
- atribuição, consulta, invalidação e destruição de `app+0xF4`, `app+0xF8` e
  `app+0xFC` no loop de mensagens;
- ordem de shutdown entre janela, socket, timer, cenas, controles e foco;
- logout/relogin e reconstrução completa da sessão/cena;
- comparação campo a campo com o recurso e a source 7.48, após o fluxo nativo
  estar fechado.

## Validação

- Pesquisa: SHA-256 da referência conferido em 2026-08-28; funções, vtables,
  instruções e helpers foram exportados em modo headless/read-only pelo Ghidra.
- Automação: `validate_research.py --repo .` passou com `LOCATED=3` após a
  inclusão desta ficha; `triage_catalog.py --repo . --format summary` passou
  com `functions=4146`, `UNMAPPED=4084`, `LOCATED=23` e
  `STATICALLY_EVIDENCED=39`. O corpus permanece com 4.146 funções; a triagem
  não é contagem de funções compreendidas.
- Integridade: `git diff --check` passou nesta documentação; os avisos exibidos
  referem-se somente à conversão normal de LF/CRLF pelo Git no Windows.
- Source: inspeção estática de `ObjectManager.cpp`, `TMScene.cpp` e
  `TreeNode.cpp`; nenhum arquivo ativo foi alterado nesta etapa.
- Client real: não executado; não há build, startup, screenshot, dump ou fluxo
  real em `client748/project.exe`. Portanto, nenhuma alegação `CLIENT_TESTED` é
  permitida.
