---
id: equipped-item-matched-consumable-shortcut
title: Use equipped-item matched consumable through F shortcut
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Use equipped-item matched consumable through F shortcut

## Pergunta

Como o client nativo 7.48 trata `F/f`, escolhe no Carry um consumível
compatível com o item equipado no slot 12 e publica a intenção de uso sem
consumir estado autoritativo localmente?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; exports focados
  `0044fc4b_FUN_0044fc4b.c`, `004525a1_FUN_004525a1.c`,
  `00453c59_FUN_00453c59.c`, `004541f3_FUN_004541f3.c` e
  `004662c5_FUN_004662c5.c`.
- Assets: nenhum asset é lido ou alterado por esta transição; o controle
  `0x9C99` é uma segunda entrada observável para a mesma ação.
- Source atual: `TMFieldScene::OnCharEvent`, `OnKeyAuto` e o layout
  `MSG_UseItem` em `client-source/tmproject/Projects/TMProject/`.
- Servidor: handler autoritativo de `MSG_UseItem` no WYD-Go; esta ficha limita
  o claim ao contrato publicado pelo client.

## Fluxo nativo 7.48

### Entrada observável

- Evento: tecla `F/f`, controle `0x9C99` ou comando textual reconhecido pelo
  handler da Field.
- Precondições e estado inicial: personagem vivo para o dispatcher de teclado,
  item presente em `Equip[12]`, grid de Carry disponível e cooldown compartilhado
  de uso de item livre.
- Saída observável: quando a primeira entrada de Carry com ability `38 == 17`
  possui a mesma chave de abilities `56/57` do item equipado, o client envia
  `MSG_UseItem` `0x373/0x24`, sinaliza o botão direito e toca o som 54.

### Callers

- `FUN_00453C59` é o dispatcher de caracteres da Field; chama
  `FUN_004525A1` na cadeia de atalhos e encerra o dispatch quando ela retorna
  consumido.
- `FUN_004541F3` trata o controle `0x9C99` e força a mesma função com
  caractere `0x66` (`f`).
- `FUN_004525A1` aceita somente `F/f`, chama `FUN_0044FC4B` e retorna `1`
  mesmo quando a ação não encontra ou não envia um item.
- `FUN_004662C5` é um segundo caller direto de `FUN_0044FC4B`: compara o
  conteúdo de um controle textual com a string de comando interna e, na
  igualdade, executa a ação, limpa/oculta o controle e consome o evento.

### Função principal

`FUN_0044FC4B` primeiro exige `Equip[12].sIndex != 0`. Ela consulta o grid por
chamadas virtuais, com `X = 8..0` no laço externo e `Y = 6..0` no interno, e
para na primeira célula cujo item existe e cuja ability `0x26` (`38`) é
`0x11` (`17`). Não procura uma segunda entrada caso a chave posterior não
combine.

Com um candidato, consulta o tempo do servidor e aceita o uso quando o
timestamp anterior é zero ou a diferença é maior que 199 ms. Em seguida
constrói duas chaves de 16 bits como `(ability[56] << 8) | ability[57]`, uma
para `Equip[12]` e outra para o candidato. Somente chaves iguais alcançam o
envio.

A posição do Carry vem da chamada virtual de normalização após `CheckType`; o
fallback nativo é `x + 9*y`. O packet zerado recebe ID do personagem, opcode
`0x373`, `SourType = 1`, `SourPos`, posição atual e `ItemID = 0`, e é enviado
com 36 bytes. Depois do envio a função atualiza o cooldown, marca o estado de
botão direito e solicita o som 54.

### Callees

- `FUN_0054CD07`: lê as abilities `38`, `56` e `57` dos itens.
- Slots virtuais `+0xB4`, `+0xC0` e `+0xC4` do receptor do grid: recuperam a
  célula, normalizam tipo/grid e resolvem a posição do Carry.
- Método virtual `+0x08` de `DAT_0092E654`: fornece o tempo do servidor usado
  pelo cooldown.
- `FUN_0055F2DD`: publica os 36 bytes de `MSG_UseItem`.
- `FUN_00429A6D` e `FUN_0042AD2B`: resolvem e reproduzem o som 54 quando o
  subsistema de áudio está disponível.

### Saídas e erros

- Tecla diferente retorna não consumida no dispatcher específico.
- `F/f` retorna consumida mesmo sem equipamento, candidato, chave igual ou
  cooldown livre; nesses casos não há packet, mutação do item, som nem avanço
  do timestamp.
- O timestamp só avança depois de `FUN_0055F2DD` ser chamado.
- O nativo não reduz quantidade, remove grid item nem zera Carry neste fluxo.
  Confirmações e ressincronização permanecem responsabilidade do servidor.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| tecla não `F/f` | Field recebe caractere | `FUN_004525A1` | inalterado | nenhum | retorna `0` |
| `F/f` sem slot 12 | personagem vivo | `FUN_004525A1 -> FUN_0044FC4B` | inalterado | nenhum | evento consumido |
| `F/f` sem ability 17 | slot 12 presente | busca reversa no grid | inalterado | nenhum | evento consumido |
| candidato em cooldown | chaves ainda não avaliadas | timer, diferença `< 200` ms | inalterado | nenhum | evento consumido |
| primeira ability 17 incompatível | cooldown livre | abilities `56/57` | inalterado | nenhum; não busca próxima | evento consumido |
| candidato compatível | cooldown livre; chaves iguais | `FUN_0055F2DD` | aguarda servidor | envia `0x373`; atualiza timestamp; botão/som | evento consumido |
| controle `0x9C99` | modo de controles ativo | `FUN_004541F3` | igual a `F` | delega caractere `0x66` | retorna resultado |
| comando textual | string reconhecida | `FUN_004662C5 -> FUN_0044FC4B` | igual à ação principal | limpa/oculta controle textual | evento consumido |

### Vtables, vptrs e receptores

`FUN_0044FC4B` recebe a instância da Field. O ponteiro em `Field + 0x2873C`
é o receptor do grid e fornece os slots virtuais `+0xB4/+0xC0/+0xC4`; a
implementação não importa os offsets ou vptrs nativos para a source, apenas
reutiliza `SGridControl::GetItem`, `CheckType` e `CheckPos`. O timer e o
publicador são singletons externos ao objeto. Não há callback assíncrono ou
receptor persistente criado por esta ação.

### Ownership

Field e grid apenas observam `Equip[12]`, o candidato e a posição do jogador.
O packet é local à pilha. O item continua pertencendo ao agregado
server-authoritative de personagem/Carry; o client não assume ownership de uma
remoção antecipada.

### Falha parcial

Todas as recusas anteriores ao envio são sem mutação. Após o envio, somente o
cooldown e os sinais audiovisuais locais mudam; quantidade e slot permanecem
intactos para que rejeição, rollback ou atualização autoritativa não deixem o
Carry divergente.

### Cleanup e teardown

A função não aloca memória, não anexa cursor, não cria controle e não mantém
callback. O packet morre ao retornar e nenhum cleanup adicional é necessário.
O grid e a Field seguem seus owners existentes.

### Shutdown

Não há trabalho de shutdown próprio. Se a cena ou o socket forem desmontados,
nenhum estado desta ação precisa ser drenado além do transporte já existente.

### Logout e relogin

O único estado local durável é o timestamp compartilhado da instância atual de
Field. A reconstrução da cena elimina esse estado; equipamento e Carry são
materializados novamente a partir do servidor. Não há token, ponteiro de item
ou seleção que sobreviva ao logout/relogin.

## Wire, ABI e recursos

- Direção: client -> servidor.
- Opcode/tamanho: `0x373`, 36 bytes.
- Layout vigente: header `0..11`; `SourType @12`; `SourPos @16`;
  `DestType @20`; `DestPos @24`; `GridX @28`; `GridY @30`; `ItemID @32`.
- Valores desta transição: `SourType = 1`, `SourPos` normalizado/fallback,
  destinos zerados, coordenadas do personagem convertidas para 16 bits e
  `ItemID = 0` pelo zero-fill.
- O contrato de seleção depende das abilities de item `38`, `56` e `57`; não
  há asset visual novo nem ID de recurso importado de outra versão.

## Mapeamento atual

### Source recompilável

`TMFieldScene::OnCharEvent` agora inclui `OnKeyAuto` dentro do gate de
personagem vivo, na mesma cadeia dos atalhos de consumível. `OnKeyAuto`
reproduz a busca, o match de abilities, o cooldown, a resolução de posição e o
envio autoritativo. O layout de `MSG_UseItem` já é protegido por
`static_assert` em `WYD748Compat.cpp`.

### WYD-Go

O servidor continua sendo a única autoridade para validar e aplicar o uso. A
source não faz consumo otimista neste novo caminho; portanto uma recusa não
exige reconstrução artificial de item pelo atalho.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject posterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| dispatch `F/f` | vivo e consome evento | handler existia como stub | nome/estrutura eram apenas pista | N/A | portar paridade nativa |
| busca do candidato | Carry 9x7 reverso, ability 38 = 17 | reproduz ordem e primeira ocorrência | não decide regra 7.48 | valida intenção | preservar regra nativa |
| vínculo com slot 12 | abilities 56/57 iguais | reproduz chave de 16 bits | não decide layout | item real é autoritativo | preservar contrato |
| cooldown | 200 ms compartilhados | reutiliza `m_dwUseItemTime` | auxilia arquitetura | servidor também revalida | preservar limite local |
| consumo local | nenhum | nenhum no novo handler | outros potions removem otimisticamente | aplica/publica estado real | escolher fluxo server-authoritative |
| wire `0x373` | 36 bytes, campos confirmados | struct e asserts compatíveis | layouts posteriores não decidem ABI | handler legado existente | manter ABI 7.48 |

## Decisões

- Classificar o delta como `PARIDADE_NATIVA`.
- Implementar a função viva da source a partir de `FUN_0044FC4B`, sem portar
  offsets de objeto nem substituir as abstrações superiores do grid atual.
- Preservar o comportamento de primeira ability 17 encontrada, inclusive a
  ausência de segunda busca quando a chave não combina.
- Não copiar o consumo otimista de outros atalhos; ele não existe nesta
  transição nativa e conflita com o servidor autoritativo do projeto.
- Manter a ficha em `CONTRACT`; build não é execução real do atalho.

## Lacunas

- Executar `F/f`, o controle `0x9C99` e o comando textual no
  `client748/project.exe` com combinação válida, inválida e cooldown ativo.
- Capturar o packet e a resposta autoritativa em uma sessão real.
- Confirmar visualmente que uma recusa do servidor mantém Carry e equipamento
  sincronizados. Essas lacunas bloqueiam somente `CLIENT_TESTED`.

## Validação

- Pesquisa: callers diretos e dispatchers de `FUN_0044FC4B` conferidos no
  corpus Ghidra do hash registrado; fluxo, falhas e teardown estão fechados
  para esta transição.
- Automação: `validate_research.py --repo .` aprovado com `CONTRACT=5`,
  `LOCATED=3` e `TRACED=2`.
- Client source: `Build-Client.ps1` aprovado com 13 warnings preexistentes e
  zero erros; candidato instalado em `client748/project.exe`, SHA-256
  `A9C2233C26957ED2415A796AB55034DBC2A6A0B600A38570F35F0CCD8846FF24`.
- Higiene: `git diff --check` aprovado; somente avisos informativos de
  LF/CRLF do Git no Windows.
- Client real: não executado; `CLIENT_TESTED` não é alegado.
