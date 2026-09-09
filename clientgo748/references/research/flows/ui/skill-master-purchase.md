---
id: skill-master-purchase
title: Abertura, render e compra de skill no mestre
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Abertura, render e compra de skill no mestre

## Pergunta

Como o client nativo 7.48 abre a lista do Skill Master, materializa livros sem
mesh como sprites, confirma a compra por `MSG_ApplyBonus 0x277` e fecha o
painel sem permitir que item, mestre ou custo enviados pelo client sejam
autoritativos?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter, projeto
  `WYD748Native_20260821.gpr`; `FUN_0040D13E`, `FUN_0040DD00`,
  `FUN_00435B13`, `FUN_0044C15C`, `FUN_0044C53F`, `FUN_0044DF53`,
  `FUN_004640E5`, caller `FUN_004662C5`, `FUN_004875C0`, gate
  `FUN_0055890A` e sender `FUN_0055F2DD`.
- Asset/UI: `FieldScene2.bin`, roots `1889/0x761` e `1905/0x771`, grade
  `1894/0x766`, catálogo de itens e atlas de skills `199` para UI2 ou `1` para
  o layout legado.
- Source: `Basedef.h`, `SGrid.cpp`, `TMFieldScene.cpp` e
  `WYD748Compat.cpp` em `client-source/tmproject/Projects/TMProject/`.
- Server: `internal/game/skills.go`, dispatcher em `internal/game/world.go`,
  catálogo de NPC/item/skill e testes em `internal/game`.

## Fluxo nativo 7.48

### Entrada observável

O jogador seleciona um NPC Skill Master e solicita sua lista. Uma resposta de
loja com tipo `3` chega a `FUN_004875C0`, que cria até 27 entradas de item por
`FUN_0040D13E`, organiza a grade e abre o par de painéis pelo toggle
`FUN_0044C15C`.

Ao clicar em um livro, a UI abre a confirmação. Confirmar chega ao dispatcher
de controles `FUN_004662C5`, que chama `FUN_004640E5`. A função constrói e
envia `MSG_ApplyBonus`, com tipo de bônus `2`, item no `Detail` e ID do mestre
em `TargetID`.

### Callers

- `FUN_004662C5` é caller direto confirmado de `FUN_004640E5` no caminho de
  confirmação da MessageBox.
- O dispatcher de packets da Field encaminha a lista tipo `3` para
  `FUN_004875C0`.
- `FUN_00435B13` é a construção/binding da Field que separa o root do Skill
  Master `1889` do root de Skill comum `1905`.
- `FUN_0044DF53` é o fechamento global que percorre os painéis visíveis e
  converge nos toggles de UI.

### Função principal

`FUN_004640E5` inicializa um packet de 20 bytes, grava opcode `0x277`,
`BonusType=2`, copia o índice do livro para `Detail`, copia o mestre para
`TargetID` e chama `FUN_0055F2DD`. O packet carrega uma intenção: o client não
decide classe, pré-requisitos, pontos, custo nem aquisição.

No caminho anterior, `FUN_004875C0` materializa a lista recebida. Para cada
item válido, `FUN_0040D13E` escolhe entre mesh e imagem. Entradas cujo
`nIndexMesh` é negativo permanecem no renderer 2D, usam o atlas `199`/`1` e a
textura `nIndexTexture`; não passam pelo resolvedor de mesh. Na UI2, as
dimensões lógicas são multiplicadas por `23.0f` para itens `5000..5102` e por
`32.0f` para os demais sprites. O caminho legado e os itens com mesh preservam
o multiplicador `24.0f`.

### Callees

- `FUN_0055F2DD`: envio do packet final de compra.
- `FUN_0040D13E`: criação/binding visual de cada item da lista.
- `FUN_0040DD00`: frame virtual do item; sprites recebem origem da caixa e
  meshes convertem a origem para o centro da região.
- `FUN_0044C15C` e `FUN_0044C53F`: visibilidade e lifecycle dos painéis Skill.

`FUN_0055890A` confirma o par `0x277 -> 0x14` no gate nativo. Seu caller geral
continua não resolvido na ficha ampla de tamanho por opcode, mas isso não muda
o tamanho e os campos observados no construtor final `FUN_004640E5`.

### Saídas e erros

O sucesso observável é o envio de uma única intenção e, depois da resposta do
servidor, a atualização pelos snapshots autoritativos de skill, score e gold.
Cancelar a MessageBox não envia `0x277`. Controle ou recurso obrigatório
ausente deve abortar a abertura da lista sem dereferenciar ponteiro nulo.

Item fora da faixa, mestre diferente da loja aberta, item não vendido pelo
mestre, classe incorreta, skill já aprendida, requisito insuficiente, falta de
pontos/gold ou falha de persistência são rejeições server-side; nenhuma delas
autoriza atualização otimista no client.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| selecionar mestre | NPC válido e painel de loja fechado | pedido da lista | mestre e pedido pendentes | envia intenção de lista | NPC/estado inválido não abre |
| receber lista tipo 3 | roots, grade e seções materializados | `FUN_004875C0` | Skill Master visível | esvazia e preenche até 27 itens | recurso incompleto aborta |
| renderizar livro 2D | item válido e `nIndexMesh < 0` | `FUN_0040D13E`/`FUN_0040DD00` | sprite 23×23 por célula na UI2 | atlas e `nIndexTexture`; posição pela origem | índice inválido não acessa catálogo |
| clicar livro | painel visível e célula preenchida | grid Skill Master | MessageBox visível | preserva item e mestre no argumento | célula vazia não envia |
| cancelar | confirmação visível | MessageBox | confirmação fechada | nenhum packet | estado server-side intacto |
| confirmar | item e mestre capturados | `FUN_004662C5` -> `FUN_004640E5` | aguarda resposta/snapshots | envia `0x277/20` | client não publica sucesso local |
| persistência concluída | todas as regras aprovadas | WYD-Go | skill aprendida | persiste e publica skill/score/gold | publicação só após save |
| rejeição/falha de save | qualquer gate falhou | WYD-Go | estado anterior | rollback integral em falha de save | nenhum snapshot de sucesso |
| fechar painel | root visível | toggle/fechamento global | roots ocultos | limpa hover e encerra interação | X/Esc real ainda requer teste |

### Vtables, vptrs e receptores

O receptor do frame visual é o `SGridControlItem` criado para a grade; o
dispatch de frame pode ocorrer virtualmente, razão pela qual
`FUN_0040DD00` não possui caller textual direto no corpus. A adaptação mantém
os tipos C++ recompiláveis e não porta vptrs nem offsets internos do objeto
nativo. Os roots `1889` e `1905` continuam objetos distintos na mesma árvore da
Field.

### Ownership

A Field possui os painéis, a grade e os itens visuais. A grade possui a cópia
de `STRUCT_ITEM` usada na apresentação; o packet copia apenas item e mestre,
sem transferir ownership. No servidor, `World` possui NPCs, catálogo e estado
do jogador e é o único escritor de gameplay.

### Falha parcial

A lista é aceita somente com os controles essenciais materializados. Itens
inválidos não podem alcançar o catálogo nem o renderer. No servidor, todas as
regras são verificadas antes da mutação; uma falha de persistência restaura
skill, score e gold e impede qualquer publicação de sucesso.

### Cleanup e teardown

O native usa `FUN_0044C15C`/`FUN_0044C53F` para visibilidade e
`FUN_0044DF53` no fechamento global. Ao abrir o par, root `1889` fica
centralizado e root `1905` fica à direita em
`(centerX + skillWidth*0.5 + 10, centerY - skillHeight*0.5)`. Ao fechar, ambos
ficam ocultos e root `1905` retorna à posição central de sua abertura comum.
Na source, `SetVisibleSkillMaster` centraliza essa transição, protege controles
opcionais, fecha painéis concorrentes e limpa o hover. Esvaziar ou destruir a
grade libera seus itens visuais pela árvore da Field; o pedido não mantém
callback ou buffer externo vivo.

### Shutdown

Destruir a Field destrói painéis e itens pela árvore de controles. Não existe
compra pendente que deva ser concluída no client durante shutdown. Uma compra
já persistida permanece no servidor; uma intenção ainda não processada não
autoriza reconstrução local.

### Logout e relogin

Logout descarta seleção, confirmação, mestre e itens visuais junto com a
Field. Relogin deve começar com o painel fechado e reconstruir skill, pontos e
gold exclusivamente do estado persistido. A sequência real de fechar por X,
Esc, logout e relogin no `project.exe` ainda é gate de validação, não lacuna do
wire `0x277`.

## Wire, ABI e recursos

### Intenção C->S `MSG_ApplyBonus`

Opcode `0x277`, tamanho 20, little-endian.

| Offset | Tamanho | Campo | Tipo/semântica |
| --- | ---: | --- | --- |
| `0x00` | 12 | `Header` | `MSG_STANDARD`; size 20, type `0x277` |
| `0x0C` | 2 | `BonusType` | `int16`; valor `2` para aprender skill |
| `0x0E` | 2 | `Detail` | `uint16`; item de skill `5000..5095` |
| `0x10` | 2 | `TargetID` | `uint16`; mestre cuja loja está aberta |
| `0x12` | 2 | padding | zerado pelo construtor |

`Basedef.h` preserva esse layout e `WYD748Compat.cpp` exige
`sizeof(MSG_ApplyBonus)==20`. O gate nativo aceita `0x277/20`.

### Recursos e render

`FUN_00435B13` vincula separadamente root `1889/0x761` do Skill Master e root
`1905/0x771` da janela Skill comum. A grade real do mestre no recurso é o
controle `1894/0x766`, `type=16`, filho de `1889`, em `(31,63)`, com
`191x241`, `8` linhas e `4` colunas. O ID `6128` é um controle tipo `6`
(`TML_SKILLM_DESC`), não a grade. O renderer 2D usa atlas `199` em UI2 ou atlas
`1` no layout legado e seleciona `nIndexTexture` do item. Sprites usam origem
da célula; apenas meshes recebem meia largura/altura para converter ao centro.

## Mapeamento atual

### Source recompilável

`MouseClick_SkillMasterNPC` captura no mesmo ponto o mestre da grade e
`m_sShopTarget`. O grid combina mestre e item no argumento da confirmação; o
handler da MessageBox separa os 16 bits altos/baixos e envia
`MSG_ApplyBonus`. `SGridControlItem` agora preserva livros sem mesh no caminho
2D, com atlas/textura e caixa 23×23 nativos na UI2, sem chamar
`BASE_GetMeshIndex`.

`OnPacketShopList` aceita tipo `3`, valida controles essenciais, esvazia a
grade, cria no máximo 27 itens e abre a UI por `SetVisibleSkillMaster`.

### WYD-Go

`onApplyBonus` lê `BonusType`, `Detail` e `TargetID` nos offsets acima e roteia
tipo `2` para `onLearnSkillAtMaster`. O servidor compara `TargetID` com
`Player.ShopNPC`, reobtém o mestre aberto, confirma Merchant e lista vendida,
faixa/classe da skill, catálogos, duplicidade, level/evolução, mastery,
encadeamento, pontos e gold. Depois aplica em cópia, persiste e só então publica
os três estados; falha de save restaura tudo.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject posterior | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| wire da compra | `0x277/20`, tipo 2, item e mestre confirmados | struct e sender compatíveis | nomes ajudam, não decidem ABI | parser/testes nos offsets nativos | `PARIDADE_NATIVA` |
| mestre da confirmação | `TargetID` vem da interação corrente | capturado junto do pedido da lista | estado posterior não decide alvo 7.48 | revalida contra loja aberta | `PARIDADE_NATIVA` |
| livros sem mesh | sprite por atlas/textura; UI2 usa 23× para 5000..5102 | caminho 2D e dimensões restaurados | renderer genérico podia traduzir como mesh | N/A | `PARIDADE_NATIVA` |
| composição Skill Master | 1889 central; 1905 à direita durante o NPC e central ao fechar | toggle simétrico dos dois roots | hardcode posterior não decide posição 7.48 | N/A | `PARIDADE_NATIVA` |
| autoridade da compra | client envia intenção | sem mutação otimista | lógica local posterior não é autoridade | valida, persiste, rollback e publica | manter arquitetura server-authoritative |
| controles auxiliares | somente roots/filhos materializados são válidos | ponteiros opcionais protegidos | pode possuir widgets extras | N/A | manter estrutura superior compatível, sem fabricar recurso |

## Decisões

- Restaurar o wire e o renderer observados no 7.48 como um único caminho
  ativo; não manter branch 7.69 por versão.
- Manter a arquitetura superior server-authoritative: `TargetID` confirma a
  interação, mas nunca escolhe livremente um mestre ou item.
- Preservar widgets/assets posteriores somente quando materializados e
  compatíveis; controles ausentes no recurso 7.48 permanecem opcionais.
- Classificar a entrega como `PARIDADE_NATIVA`, com ficha `CONTRACT`; build e
  testes automatizados não promovem para `CLIENT_TESTED`.

## Lacunas

- executar no `client748/project.exe`: abrir cada mestre, conferir ícones,
  confirmar e cancelar, validar rejeições e observar os snapshots de sucesso;
- validar fechamento por X e Esc e reabertura sem item/mestre stale;
- validar logout/relogin depois de sucesso e durante uma confirmação aberta;
- registrar screenshot na mesma resolução se a geometria do sprite ainda
  divergir;
- resolver o caller indireto de `FUN_0040DD00` apenas se uma próxima mudança
  depender de seu slot virtual exato.

## Validação

- Pesquisa: funções, roots, renderer e wire acima conferidos no projeto Ghidra
  do executável do frontmatter.
- Automação em 2026-09-01: `go test -count=1 ./...`, `go vet ./...`,
  `Build-Client.ps1`, o validador de fichas e `git diff --check` passaram.
  O build Release Win32 v145 terminou com zero erros e 15 warnings C4018
  preexistentes fora das linhas alteradas.
- Artefato: o build oficial instalou `client748/project.exe` com SHA-256
  `85FC6B2541784C4AF83A275B5614FD74B8990A303A6618AEC21DFBB02FE602D2`.
- Follow-up visual em 2026-09-01: `SGridControlItem` passou a aplicar as caixas
  UI2 nativas `23/32`, e `SetVisibleSkillMaster` passou a reproduzir a
  composição e o fechamento simétrico dos roots `1889/1905`. O validador,
  `git diff --check` e `Build-Client.ps1` passaram; Release Win32 v145 terminou
  com zero erros e 15 warnings C4018 preexistentes. O candidato instalado tem
  SHA-256 `9DE77CE65766703FADA3FEDA006388D907E5E53A55D5DFFC90669AB84931004F`.
- Client real: não executado nesta entrega. Estado máximo `CONTRACT`.
