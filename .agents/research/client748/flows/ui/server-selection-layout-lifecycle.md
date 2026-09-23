---
id: ui-server-selection-layout-lifecycle
title: Posicionamento e lifecycle da selecao de servidor 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-22
---

# Posicionamento e lifecycle da selecao de servidor 7.48

## Pergunta

Por que o painel Server/Channel da tela inicial ficava abaixo da posicao do
client nativo e qual composicao deve ser preservada durante entrada, retorno do
login e relogin?

## Fronteira de evidência

- Executavel: `references/client748/WYD.exe`, SHA-256 acima.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr` e
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Inicializador nativo: `FUN_004A8F14`, cena do estado `7`.
- Vtable: `0x005A4544`; o slot `+0x4C`, armazenado em `0x005A4590`, aponta
  para `FUN_004A8F14`.
- Source ativa: `TMSelectServerScene::InitializeScene` e
  `TMSelectServerScene::InitializeUI`.
- Recurso: `UI\SelServerScene2.txt`, materializado por
  `tmproject/client748/UI/SelServerScene2.bin`.
- Captura fornecida em 2026-09-02: o conjunto Server/Channel aparecia
  deslocado para baixo em relacao ao client original.

## Fluxo nativo 7.48

### Entrada observável

Ao entrar no estado de selecao de servidor, `FUN_004B3500` instancia a cena
`7` e chama indiretamente seu initializer pelo slot virtual `+0x4C`.
`FUN_004A8F14` seleciona `SelServerScene2` quando `g_UIVer == 2`, resolve a
arvore de controles e compoe o painel Server/Channel antes de exibi-lo.

O root nativo e o controle `4622` (`0x120E`). A traducao de recursos da source
o entrega como `P_SERVER_SEL` (`65537`); listas `4618/4619` tornam-se
`65542/65543`, e os titulos `5635/5636` tornam-se `65540/65541`.

### Callers

`CONFIRMED`: a vtable `0x005A4544` da cena `7` contem `FUN_004A8F14` no slot
`+0x4C`. O caller generico esta em `FUN_004B3500:0x004B370F`, por
`CALL [EDX+0x4C]`; retorno zero ativa destruicao parcial, diagnostico e
`WM_CLOSE`.

### Função principal

`FUN_004A8F14` busca o root `0x120E` e chama seu slot de posicionamento com:

```text
x = viewportWidth*0.5 - rootWidth*0.5
y = viewportHeight*0.5 - rootHeight*0.5
```

Nao existe uma segunda soma vertical de 75 pixels nesse receptor. O mesmo
initializer define `g_nChannelWidth = 140`, cria os tres trechos do cabecalho,
ajusta o titulo Server em `y += 5` e o titulo Channel em `x += 8`, `y += 5`.
Os tratamentos de logo para larguras 1024, 1280 e 1600 pertencem a receptores
separados e nao alteram a formula do root.

`SControl` ja materializa posicao e tamanho conforme as razoes do viewport.
Assim, `m_nWidth` e `m_nHeight` usados pela source sao dimensoes renderizadas;
aplicar outra escala ou offset apos `SetPos` duplicaria a transformacao.

### Callees

- carregamento do RC pelo slot virtual `+0x3C`;
- busca de controles pelo slot `+0x48`;
- posicionamento pelo slot `+0x70`;
- visibilidade pelo slot `+0x60`;
- anexacao dos paines dinamicos a arvore por `FUN_0054AC09`.

Os tres cabecalhos e os backgrounds de linhas/canais sao filhos do root. Uma
unica mudanca na posicao do root desloca desenho, textos, listas e hitboxes em
conjunto; nao ha justificativa para corrigir filhos individualmente.

### Altura dos humanos da demonstracao

`CONFIRMED`: o slot `+0x20` da vtable `0x005A4544` resolve para
`FUN_004AAEDC`. No `FrameMove` da cena, a funcao percorre ate `0x32` ponteiros
de humanos em `scene+0x26F0C` e, quando o ponteiro existe, grava
`0xC0800000` (`-4.0f`) em `human+0x228` antes da atualizacao visual. Portanto a
altura desejada dos humanos do backdrop da selecao de servidor e `-4.0f`; nao
e uma amostragem do terreno.

O TMProject748 e evidencia secundaria consistente: seu
`TMSelectServerScene::FrameMove` atribui `m_fWantHeight = -4.0f` aos mesmos
humanos e `TMHuman::GetMyHeight` devolve esse alvo diretamente para os tipos
ordinarios. A validacao integrada deste comportamento no client atual permanece
pendente; resultados de uma implementacao descontinuada nao a substituem.

O render skinned tambem possui um contrato de culling distinto do restante da
cena. No nativo 7.48, `FUN_004C3EEC` e sua continuacao decompilada em
`FUN_004C51FD` colocam `D3DRS_CULLMODE` (`0x16`) em `1` imediatamente antes do
draw e restauram `3` depois dele. O TMProject748 confirma a semantica em
`CMesh::RenderMesh`: usa `D3DCULL_NONE` no `DrawIndexedPrimitive` da malha
skinned e restaura `D3DCULL_CCW`. O contrato nativo distingue esse caminho
do estado de terreno e objetos estaticos.

### Textura-base do céu no estado Server/Login

`CONFIRMED`: `FUN_00547890` constroi o `TMSky` nativo e `FUN_00547E8A` executa
seu render. A transformacao observada usa a posicao X/Z da camera, altura
`-5.0f + 1.0f`, yaw de demonstracao `pi/2`, pitch `-90` graus e escala
`(0.5, 0.25, 0.5)`. Esta observacao nativa nao comprova validacao no client atual.

O delta visual que deixava o ceu como um borrao azul estava na selecao da
textura. A logica de weather em `FUN_005484D1` mostra que o estado `1` usado na
tela Server/Login troca a textura-base do common mesh do ceu para o indice
`68`. Nesse estado o segundo stage nao participa da composicao; portanto a
textura serializada dentro de `mesh\\sky001.msa` nao e a textura-base efetiva
do frame.

O asset oficial confirma o contrato: `Effect\\EffectTextureList.bin` possui
512 registros de `0x108` bytes e o slot `68` resolve para
`mesh\\sky02.wys`, alpha `N`. O TMProject748 e evidencia secundaria consistente:
`TMSelectServerScene` chama `SetWeatherState(1)` e `TMSky::SetWeatherState`
atribui `nState + 67` ao indice de textura-base para estados menores que 10.

Claims de loader e testes de uma implementacao descontinuada foram retirados.
O contrato de textura acima continua sendo evidencia nativa; verificar seu
consumo no fluxo vivo do TMProject antes de afirmar implementacao ou teste.

Procedencia desta unidade: binario/descompilacao Ghidra 7.48 `UTILIZADA` como
fonte primaria; assets oficiais 7.48 `UTILIZADA`; TMProject748 `UTILIZADA`
somente como confirmacao
secundaria; fontes legadas externas `NAO APLICAVEL` por exclusao da campanha.

### Saídas e erros

Falha de `FUN_00541065` faz o initializer registrar erro, mostrar
`MessageBoxA`, postar `WM_CLOSE` e retornar zero. O caller generico destrói a
cena parcial e repete o diagnostico. O caminho normal retorna um e entrega a
cena composta ao ObjectManager.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondicao | Funcao/call | Estado resultante | Side effects | Erro/saida |
| --- | --- | --- | --- | --- | --- |
| entrada na tela inicial | estado `7` solicitado | `FUN_004B3500 -> +0x4C -> FUN_004A8F14` | root centralizado e visivel | carrega Scene2 e compoe filhos | falha do RC fecha a janela |
| selecionar canal | grupo/canal validos | event handler da cena | root oculto; login visivel | foco vai ao ID | endpoint ainda e validado pelo servidor |
| voltar do login | cancelar/retornar | event handler da cena | root reaparece na mesma posicao | login perde foco | nao recria controles |
| Game Grade | demonstracao ativa | `FrameMoveGameGrade` | alterna root e logo | preserva coordenadas | sem novo ownership |
| logout/relogin | ObjectManager troca cenas | recriacao da cena `7` | nova arvore reaplica a formula | ponteiros antigos nao sobrevivem | segue falha padrao de init |
| teardown/shutdown | cena marcada para delecao | `FUN_004AE5C0 -> FUN_004A8E29 -> FUN_00494C00` | filhos e container liberados | detach por `FUN_0054AA45` | sem alocacao propria do layout |

### Vtables, vptrs e receptores

O vptr da cena `7` aponta para `0x005A4544`. O ObjectManager anexa a cena e sua
arvore por `FUN_0054AC09`. O slot `+0x4C` resolve o initializer; os receptores
de layout sao o root e os controles filhos obtidos do container.

### Ownership

O container e owner dos controles carregados e dos filhos adicionados ao root.
O helper de posicionamento nao aloca nem retém ponteiros fora da cena.

### Falha parcial

Se `FUN_00541065` falhar, `FUN_004A8F14` diagnostica, solicita `WM_CLOSE` e
retorna zero. `FUN_004B3500` destroi a cena parcialmente inicializada pelo
mesmo caminho de ownership; remover o offset nao acrescenta recursos parciais.

### Cleanup e teardown

`FUN_004B16C0` consome a marca de delecao e converge no deleting destructor
`FUN_004AE5C0`, cleanup especifico `FUN_004A8E29`, base `FUN_00494C00` e
detach `FUN_0054AA45`.

### Shutdown

O shutdown global usa a mesma coleta e teardown da cena. A geometria nao deixa
timer, captura de input, alocacao ou callback proprio a ser cancelado.

### Logout e relogin

Posicionamento nao cria estado persistente. Retorno do login usa a mesma
instancia; logout, relogin e shutdown descartam a arvore inteira e a proxima
cena executa novamente o initializer.

## Wire, ABI e recursos

Nao ha mudanca de wire, ABI, opcode, asset ou servidor. A adaptacao conserva o
recurso Scene2, os IDs traduzidos, callbacks, visibilidade, foco e ownership.

O arquivo `serverlist.bin` entrega cada endpoint em uma celula decodificada de
64 bytes. O loader legado processa a celula inteira e nao garante terminador
NUL. A selecao normal ainda passava essa celula diretamente a `%s`, enquanto
as consultas HTTP de populacao e a composicao do grupo agregado faziam a mesma
suposicao. O reconnect de migracao possuia uma checagem local equivalente, mas
os caminhos podiam divergir. A source adaptada usa `CopyServerEndpoint` na
selecao inicial, migracao, consultas de populacao e composicao do grupo
agregado, tanto na cena inicial quanto no painel em jogo. So publica ou consome
o endereco quando o texto termina dentro da propria celula; resposta HTTP
anterior tambem nao e reaproveitada quando a entrada atual e invalida. Este
guard e `MODERNIZACAO_COMPATIVEL`; nao altera o formato do asset nem o contrato
7.48.

## Mapeamento atual

Classificacao do layout: `PARIDADE_NATIVA`. Classificacao do guard de endpoint:
`MODERNIZACAO_COMPATIVEL`.

`TMSelectServerScene::InitializeUI` ja possuia a formula nativa de
centralizacao, mas somava `75.0f` a `m_nPosY` em seguida. A adaptacao remove
somente essa divergencia. Os filhos continuam ancorados ao root e os ajustes
nativos de titulo, lista, largura de canal e logo permanecem inalterados.

WYD-Go nao participa deste layout local e nao requer funcao correspondente.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source adaptada | Decisao |
| --- | --- | --- | --- | --- |
| root Server/Channel | centro exato em X/Y | centro em X/Y seguido de `y += 75` | centro exato em X/Y | remover apenas o offset extra |
| altura dos humanos demo | `FUN_004AAEDC` grava `-4.0f` em `human+0x228` | `m_fWantHeight = -4.0f` | validacao atual pendente | preservar alvo vertical da cena |
| culling dos humanos demo | draw skinned usa `D3DRS_CULLMODE=1` e restaura `3` | `CMesh::RenderMesh` usa `D3DCULL_NONE` e restaura `D3DCULL_CCW` | validacao atual pendente | preservar estado skinned sem alterar terreno/objetos |
| filhos e hitboxes | filhos do root | filhos do root | preservados | nao deslocar individualmente |
| titulos e largura | `+5`, `+8/+5`, largura 140 | ja equivalentes | preservados | sem alteracao |
| logos por resolucao | branches 1024/1280/1600 | ja equivalentes | preservados | sem alteracao |
| endpoint selecionado | celula fixa do `serverlist.bin` | `%s` podia atravessar uma celula sem NUL | copia limitada e rejeicao antes de publicar estado | preservar asset e impedir leitura cruzada |
| destino apos endpoint invalido | a ficha nao estabelece contrato nativo para o conteudo do buffer local apos rejeicao | `CopyServerEndpoint` rejeitava vazio/sem NUL mas mantinha um IP anterior no destino | rejeicao limpa o primeiro byte do destino; copia valida permanece identica | `MODERNIZACAO_COMPATIVEL`; evitar endereco obsoleto sem mudar endpoint valido ou wire |
| endpoint de status/agregado | mesma celula fixa de 64 bytes | HTTP e `%s` confiavam em NUL | copia limitada; resposta antiga e destino agregado invalido sao limpos | impedir leitura cruzada sem inventar canal |
| rótulo de canal cheio | limiar de 700 usuários já preservado | laço de padding nunca executava; `FULL` vinha logo após nomes curtos | helper limita leitura à capacidade, preenche até coluna 14 e grava `FULL` com NUL | `MODERNIZACAO_COMPATIVEL`; corrigir alinhamento local sem alterar o limiar |
| rótulo cheio no painel Field | fonte usa limites locais `>500`/`>600` | os dois laços de padding também tinham condição sempre falsa | ambos reutilizam `AppendFullChannelLabel` | `MODERNIZACAO_COMPATIVEL`; só formatação, sem mudar os limites ou a seleção do canal |
| seleção ao trocar de grupo | reset nativo do índice não aferido; endpoint segue a lista 7.48 | `SListBox::Empty` apaga linhas, mas mantém `m_nSelectedItem`; `Connect` poderia reutilizar o mesmo índice no novo grupo | a cena limpa o índice antes de adicionar canais do novo grupo | `MODERNIZACAO_COMPATIVEL`; exige novo clique, sem mudar wire nem mapeamento de endpoint |
| reabertura da lista de canais em Field | mutação local da seleção; sem alteração do comando de troca | `SListBox::Empty` preservava o índice antigo e o callback dereferenciava `GetItem` sem guarda | limpa o índice ao remontar e rejeita callback sem item | `MODERNIZACAO_COMPATIVEL`; evita seleção obsoleta e acesso nulo, sem mudar a intenção `srv` |
| varredura dos grupos | `serverlist.bin` reserva 10 grupos | selecao e troca em Field podiam ler o indice 10 ao procurar sentinela | descoberta limitada aos 10 grupos; tabela cheia termina em 9 e tabela vazia e rejeitada em Field | modernizacao compativel, sem wire novo |
| linhas de grupo selecionáveis | lista da cena insere slots válidos em ordem inversa | evento contava `j < m_nMaxGroup`, mas a montagem usava `i >= 0`, incluindo o último slot; com lacunas, índice de linha também podia resolver outro grupo | construção guarda os slots visíveis; clique e confirmação usam esse snapshot e os limites físicos | `MODERNIZACAO_COMPATIVEL`; preservar a ordem e o endpoint escolhido mesmo após mutação dos endpoints do agregado, sem alegar nova paridade nativa |
| fundos das linhas de grupo | a lista insere apenas grupos válidos, em linhas contíguas | o painel decorativo usava o slot físico `i` como posição vertical; com lacunas, separava-se do texto e da área clicável | posicionar o painel pela mesma linha visível `row` usada na inserção | `MODERNIZACAO_COMPATIVEL`; em tabela densa, as posições continuam as mesmas, sem mudar recurso ou wire |
| conclusão do fade dos grupos | a cena pode exibir até dez grupos e o fade parcial atualiza os dez painéis | ao completar o fade, apenas `m_pGroupPanel[0..1]` recebiam a cor final; outros ficavam com o alpha do último frame parcial | aplicar a cor final a todos os painéis existentes | `MODERNIZACAO_COMPATIVEL`; dois grupos e contrato de UI/servidor preservados |
| linhas de canal esparsas na seleção inicial | a lista 7.48 usa endpoints físicos de `serverlist.bin`; lacunas não foram aferidas no binário | a lista compactava canais configurados, mas `Connect` convertia a linha selecionada por `selectedChannel + 1` | cada linha inserida guarda seu canal físico; confirmação usa o snapshot e rejeita linha inválida | `MODERNIZACAO_COMPATIVEL` no grupo normal; preservar o endpoint exibido sem alterar wire ou o mapeamento especial do agregado |
| destruição após falha de recurso inicial | o fluxo nativo destrói a cena parcialmente inicializada quando o carregamento falha | `LoadRC` ou `sn.bin` podia falhar antes do `memset(m_pCheckHumanList)`; o destrutor percorria ponteiros indeterminados | os 50 ponteiros começam nulos na construção da cena | `MODERNIZACAO_COMPATIVEL`; garantir teardown seguro sem mudar o caminho de sucesso ou o recurso |
| controles obrigatórios de Scene2 | o initializer nativo resolve a árvore da cena; esta ficha não prova o tratamento de um recurso truncado | `LoadRC` podia aceitar uma árvore parcial e `InitializeUI`/login dereferenciavam controles ausentes | a cena exige root, listas, painel e botões de login, logos e campos de edição antes de montar a UI | `MODERNIZACAO_COMPATIVEL`; falhar na abertura e preservar o teardown já protegido, sem mudar recurso válido ou wire |
| tag de tipo truncada no RC | o formato válido contém tags de controle de quatro bytes; esta ficha não atribui ao nativo o tratamento de corrupção | `fread` de um item de quatro bytes tratava 1–3 bytes finais como EOF limpo; tipo desconhecido retornava sem fechar o arquivo | leitura distingue EOF entre tags de tag incompleta e fecha o arquivo nas rejeições | `MODERNIZACAO_COMPATIVEL`; falhar no recurso inválido sem alterar o formato válido nem o wire |
| nomes dos grupos | `sn.bin` e uma tabela binaria fixa de 143 bytes | `NewApp` tentava le-la como texto antes da cena e sobrescrevia nomes com `Meu Canal`/`STAFF` | a cena carrega a tabela binaria validada antes de construir os controles; parser textual removido | manter um unico leitor do asset 7.48 |
| servidor/wire | nao participa da geometria | inalterado | inalterado | nenhuma mudanca |

## Decisões

- Remover somente o deslocamento vertical extra de 75 pixels.
- Manter Scene2, traducoes de ID e composicao dinamica existente.
- Nao alterar logos, listas, callbacks, disponibilidade de canal ou servidor.
- Nao aplicar escala adicional a dimensoes ja materializadas.
- Rejeitar endpoint vazio ou sem NUL dentro dos 64 bytes antes de mudar indices,
  tela ou conexao; usar a mesma regra na selecao inicial, migracao, status e
  composicao do grupo agregado nas duas cenas. Limpar o destino quando a
  copia falhar, inclusive quando ele continha um endpoint anterior.
- Limitar a descoberta de grupos ao tamanho fisico de `g_pServerList` nas cenas
  de selecao e Field. A varredura e `MODERNIZACAO_COMPATIVEL`: nao altera o
  contrato 7.48; impede leitura fora da tabela quando os 10 grupos existem.
- Remover `NewApp::InitServerName`: seus unicos consumidores de estado sao a
  cena de selecao e, depois, Field; `TMSelectServerScene::InitializeScene`
  carrega `sn.bin` com `WYD748_LoadServerNameList` antes de montar a UI. O
  arquivo permanece binario e intocado. Esta limpeza e
  `MODERNIZACAO_COMPATIVEL`, sem mudanca de wire ou servidor.
- Corrigir a formatação local de `FULL` nos dois ramos da lista, mantendo a
  coluna 14 e o limiar `>700` existentes. O helper rejeita buffers pequenos ou
  sem terminador antes de escrever; não há mudança de recurso ou wire.
- Reutilizar o mesmo helper nos dois ramos do painel de canais em Field. Os
  limites `>500` e `>600` permanecem os da source; este lote não afirma que
  sejam os limites nativos e não altera o resultado da consulta HTTP.
- Invalidar a seleção de canal ao reconstruir a lista após clique em outro
  grupo. `SListBox::Empty` libera os itens, mas preserva o índice selecionado;
  um novo grupo com canal nessa posição poderia seguir para login sem escolha
  explícita. Mudança local da cena, sem alegação de paridade nativa.
- Aplicar a mesma invalidação ao reabrir a lista de canais em Field e rejeitar
  callback sem item antes de ler a população. O comando de troca `srv` e seu
  índice permanecem inalterados; a semântica de linhas esparsas desse comando
  ainda requer evidência própria antes de ajuste.
- Resolver o índice do clique e da confirmação pelos mesmos slots que criaram
  as linhas. Isso inclui o décimo grupo e ignora ordens vazias, inválidas ou
  sem status endpoint. O snapshot pertence à cena porque o clique no grupo
  agregado zera endpoints de status sem remover as linhas já criadas; recalcular
  os slots após esse clique trocaria sua identidade. Não alterar neste lote o
  mapeamento do grupo agregado.
- Usar a linha visível compactada também para o painel decorativo de cada
  grupo. O painel continua pertencendo ao slot físico, mas acompanha texto e
  hitbox quando há lacunas em `sn.bin`/`serverlist.bin`.
- Finalizar o fade de todos os painéis de grupo existentes, assim como sua
  fase parcial. Não alterar duração, cor final ou a lógica de fade dos logos.
- Guardar também o canal físico ao inserir cada linha da lista de canais.
  Uma configuração com lacunas compacta as linhas visíveis; o ordinal da linha
  não é o índice de `serverlist.bin`. Usar o snapshot apenas para a seleção
  normal; o ramo do grupo agregado mantém seu mapeamento próprio pendente de
  evidência nativa. A captura com a coluna Channel vazia antes do clique no
  grupo não demonstra falha dessa conversão.
- Inicializar `m_pCheckHumanList` na própria construção do objeto, antes de
  `InitializeScene`: a falha de `LoadRC` ou do loader de `sn.bin` retorna antes
  do `memset` anterior, e `ObjectManager` destrói a cena mesmo nessa falha.
  A mudança só define o estado seguro do teardown parcial; não altera demo,
  seleção, asset ou protocolo.
- Exigir os controles usados sem guarda na cena de seleção antes de
  `InitializeUI`. Um `SelServerScene2.bin` incompleto pode terminar a leitura
  sem que `LoadRC` denuncie todos os IDs ausentes; retornar falha evita acesso
  nulo e deixa o mesmo caminho de destruição parcial limpar a cena.
- Distinguir fim limpo do RC de uma tag de tipo incompleta. O parser compartilhado
  deve rejeitar 1–3 bytes finais e fechar o arquivo ao rejeitar tipo desconhecido;
  o caminho de sucesso e o contrato de recursos 7.48 não mudam.

## Lacunas

A correlacao estatica do layout e do lifecycle esta fechada. Falta validar no
client recompilado a aparencia e os hitboxes em resolucoes de referencia,
incluindo retorno do login e logout/relogin.

O grupo agregado continua sem contrato nativo suficiente para corrigir seu
mapeamento: a source consulta status no grupo `i`, mas copia endereco usando
`m_nDay[aggregateGroup-i]` e `aggregateGroup-i-1`, enquanto a confirmacao da
selecao resolve o grupo original por `g_nServerCountList`. Esse possivel
desencontro aparece tanto na selecao inicial quanto no painel Field; nao foi
alterado neste lote. Exige seguir o clique e a resolucao do endpoint no binario
7.48 antes de promover qualquer correcao. O corpus e o projeto Ghidra apontados
pela ficha nao estavam disponiveis no perfil local deste turno; nao inferir
paridade a partir da source 7.69.

No painel Field, o callback ainda transforma índice visual em canal por
`idwEvent + 1`. Se o asset tiver lacunas, isso pode divergir da linha exibida;
não alterar essa relação sem prova nativa e teste do contrato `srv` no servidor.

A seleção inicial de canais esparsos agora resolve a linha pelo snapshot, mas
esse caso ainda não foi exercitado no client real. O asset local não comprova
por si só a presença de lacunas no runtime do usuário.

## Validação

- Pesquisa: `TRACED`; initializer, caller virtual, vtable, receptores,
  geometria, falha parcial, ownership e teardown correlacionados.
- Implementacao: `IMPLEMENTED`; removido apenas o offset vertical ausente no
  nativo.
- Automacao do guard: testes cobrem endereco normal, vazio, sem NUL e o limite
  de 63 bytes; a versao anterior preservava o destino nas rejeicoes. O build oficial
  `Release|Win32` v145 executou 51.715 checks/asserts, compilou e instalou
  `tmproject/client748/project.exe` com SHA-256
  `0030265DE55056F7BA3887EB09E60CF60B9B3C0095DE8BB987C78A78ECC33C60`.
- Gate atual do destino em falha: testes cobrem limpeza de um endereco anterior
  para celula vazia e sem NUL; `Build-Client.ps1 -NoDeploy` passou 51.787 checks
  e recompilou os consumidores em Release|x86, candidato SHA-256
  `0595B7DD797EF394CC7DFD7944B48EDDC1C74B4C1BD7B0DD9D097325EA30934B`.
  Nao foi instalado nem exercitado no client real.
- Client real: a captura fornecida em 2026-09-22 mostra o root Server/Channel
  centralizado no viewport usado. A lista de canais vazia antes de escolher
  `Canal` ou `VPS` corresponde ao lifecycle atual: ela nasce oculta e e
  preenchida/exibida pelo clique no grupo. A captura nao exercita o novo
  candidato, esse clique, os hitboxes, o retorno ou o relogin; portanto o fluxo
  ainda nao e `CLIENT_TESTED`.
- Gate adicional da varredura: `Build-Client.ps1 -NoDeploy` passou 51.723
  checks, compilou o client Release|x86 e produziu `WYD.exe` SHA-256
  `C1B4379FEC7C2DB660F25204273A07215FD8BAF67723D2EB7517DDCE998BEDA7`.
  Testes cobrem tabela vazia, parcial e completamente ocupada. O candidato
  nao foi instalado no runtime; nao houve teste visual nem troca de canal real.
- Gate da remocao do parser textual: o asset local `sn.bin` tem 143 bytes e as
  11 ordens decodificadas estao em `0..10`. `Build-Client.ps1 -NoDeploy`
  passou 51.723 checks e compilou Release|x86, SHA-256
  `28F1764724CCF964C88852D3B8A1035B669591CB8D516C56E7E41007572F4980`.
  Os testes de arquitetura nao exercitam diretamente o loader binario; esta
  etapa e `STATICALLY VERIFIED / AUTOMATED TESTED` para compilacao e testes
  existentes, nao `CLIENT_TESTED`. O runtime instalado nao foi substituido.
- Gate do rótulo `FULL`: `Build-Client.ps1 -NoDeploy` passou 51.762 checks,
  incluindo padding curto, nome de 14 caracteres, truncamento, capacidade
  insuficiente e ausência de NUL; compilou `Release|x86` incremental. O
  candidato não foi instalado e a lista cheia não foi testada no client real.
- Gate da reutilização em Field: `Build-Client.ps1 -NoDeploy` passou os mesmos
  51.762 checks e recompilou `TMFieldScene.cpp` em `Release|x86`; o executável
  não foi instalado. O alinhamento no painel Field ainda não foi visto em jogo.
- Gate da seleção obsoleta: `Build-Client.ps1 -NoDeploy` passou 51.764 checks,
  incluindo a ordem `Empty` -> seleção `-1` -> inserção de canais, e recompilou
  `TMSelectServerScene.cpp` em `Release|x86`. Candidato SHA-256
  `42B00057EDBA6B5C885352958564FC1F258B1474C21A6B0B63ABB833FA53FD5F`;
  não instalado nem testado no client real. O teste de ordem é estático; o
  comportamento de clique e confirmação ainda exige validação manual.
- Gate do candidato instalado antes do ajuste de grupos: `Build-Client.ps1`
  passou 51.764 checks e instalou `project.exe` SHA-256
  `42B00057EDBA6B5C885352958564FC1F258B1474C21A6B0B63ABB833FA53FD5F`.
  O processo abriu a janela `WYDESTINY MMORPG`, mas a captura de tela falhou
  com `0x80004002`; nenhum clique foi executado ou validado.
- Gate dos slots visíveis: `Build-Client.ps1 -NoDeploy` passou 51.767 checks,
  incluindo tabela cheia de dez grupos, ordens esparsas e tabela vazia;
  recompilou `TMSelectServerScene.cpp` em Release|x86 e produziu `WYD.exe`
  SHA-256 `32C10B51AEEC463BB2B5A99A66410816ED0D914CF7E386E83F77497C464B7D9B`.
  Este candidato novo ainda não foi instalado nem exercitado na interface.
- Gate do snapshot de linhas: `Build-Client.ps1 -NoDeploy` passou 51.769
  checks, incluindo a guarda de que os eventos usam o snapshot construído
  antes da mutação dos endpoints do agregado e o teste de estabilidade após
  apagar um endpoint de status; recompilou a cena e seus consumidores em
  Release|x86. Candidato SHA-256
  `E3CA0135E84E4DA74B29A1528D247F4B5978CA151D05890E6D91C7941BB1418A`;
  ainda não instalado nem testado por clique real.
- Gate do painel Field: `Build-Client.ps1 -NoDeploy` passou 51.772 checks,
  incluindo ordem `Empty` -> seleção `-1` -> inserção e rejeição de item nulo;
  recompilou `TMFieldScene.cpp` em Release|x86. Candidato SHA-256
  `1E7AFFA9BA22D636529EBBAD6A7E3CDAA63B92A7CA659A199FD22B317FFE43AE`.
  O comando `srv` não foi alterado nem testado no client real.

- Gate da leitura HTTP de população: `MODERNIZACAO_COMPATIVEL`. O status é
  apenas uma dica de disponibilidade; `BASE_GetHttpRequest` agora reserva um
  byte para NUL, rejeita capacidade inválida e limpa o resultado quando
  `InternetReadFile` falha. Não muda endpoint, seleção ou protocolo do jogo.
  `Build-Client.ps1 -NoDeploy` passou 51.774 checks (incluindo contrato focado)
  e recompilou `Basedef.cpp` em Release|x86; candidato SHA-256
  `CAE948D565C85EA1F048800DE849BC2AC40E7CC02D89F14F24FD6908B7E2C819`.
  Não instalado nem `CLIENT_TESTED`; os warnings C4018 preexistentes em outras
  linhas de `Basedef.cpp` permanecem.
- Gate das linhas de canal esparsas: `Build-Client.ps1 -NoDeploy` passou 51.785
  checks, incluindo mapeamento `1,3,10`, rejeição de linha obsoleta e guarda
  estática da montagem/consumo do snapshot; compilou Release|x86 e produziu
  `WYD.exe` SHA-256
  `67B64372442E871601FE9A5006E79DC8097212C469EEB7CF8D7DABB4F9074237`.
  `STATICALLY VERIFIED / AUTOMATED TESTED`; candidato não instalado e nenhum
  clique/conexão real executado, portanto não `CLIENT_TESTED`.
- Gate da destruição após falha inicial: `Build-Client.ps1 -NoDeploy` passou
  51.787 checks, incluindo a inicialização dos ponteiros antes dos dois loaders
  que podem falhar; recompilou a cena em Release|x86, candidato SHA-256
  `22A0B984E9A50144B40827FA4BAAF41A6C326985AFA2485B22CBC9F534B3AE38`.
  Teste de ordem é estático; falha de recurso e destruição não foram injetadas
  em runtime. `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`.
- Gate dos controles obrigatórios: `Build-Client.ps1 -NoDeploy` passou 51.788
  checks, incluindo a ordem da pré-validação antes de `InitializeUI`; recompilou
  `TMSelectServerScene.cpp` em Release|x86. Candidato SHA-256
  `60E3831218A033646C1FE0367A481A7B0F1CB9FCF7124D7DDC532D947E769135`.
  A guarda foi verificada estaticamente, mas não houve injeção de recurso
  truncado nem teste no client real. Não instalado; não `CLIENT_TESTED`.
- Gate da tag RC: `Build-Client.ps1 -NoDeploy` passou 51.792 checks e recompilou
  `TMScene.cpp` em Release|x86; candidato SHA-256
  `E1646D6AFC0406F518477C9D4A1A4635790C7C09BB1A48CC10C9068976E36138`.
  O teste focado ampliado passou 51.793 checks, exercitando EOF limpo, tag
  completa, tag parcial e tag completa seguida de bytes finais parciais.
  A guarda de fechamento para tipo desconhecido foi verificada estaticamente;
  não houve injeção do RC completo na cena nem execução do client. Candidato
  não instalado; `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`.
- Gate do alinhamento dos grupos esparsos: `Build-Client.ps1 -NoDeploy` passou
  51.796 checks, incluindo a guarda estática da posição pelo índice visível,
  e recompilou `TMSelectServerScene.cpp` em Release|x86. Candidato SHA-256
  `E491C5C4C4011BED8105DE5CFD95FF17CDEAAC3C03C32B989CFBEBC5922F1B2D`.
  Não instalado nem testado por clique no client; `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`.
- Gate da conclusão do fade: após corrigir uma guarda estática sensível ao fim
  de linha, `Build-Client.ps1 -NoDeploy` passou 51.797 checks e recompilou
  `TMSelectServerScene.cpp` em Release|x86. Candidato SHA-256
  `8D139C4EF065F7AFDD65C6DEFC77D6ECC3887A6689BB92C1E414D9BF38D0C975`.
  A guarda ampliada passou 51.798 checks sem recompilar o produto.
  Os testes verificam a cobertura do ramo final, não o frame renderizado;
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`.
- Gate de instalação e tentativa visual deste candidato: `Build-Client.ps1`
  passou 51.798 checks, compilou Release|x86 e instalou `project.exe` com o
  mesmo SHA-256 acima. O processo abriu uma janela `WYDESTINY MMORPG`, mas
  a captura da janela falhou duas vezes com `SetIsBorderRequired failed:
  No such interface supported (0x80004002)`. Nenhum clique ou retorno do
  login foi executado; o processo depois não estava mais ativo. Não surgiu
  minidump novo nem evento `Application Error` para este executável no período,
  portanto esta tentativa não comprova crash do client. O fluxo continua sem
  validação `CLIENT_TESTED`.
- Nova tentativa visual com autorização do usuário (mesmo `project.exe`, sem
  rebuild): o processo abriu uma janela `WYDESTINY MMORPG` e permaneceu ativo.
  Após reconectar à janela correta, a captura voltou a falhar com
  `SetIsBorderRequired failed: No such interface supported (0x80004002)`.
  A acessibilidade expôs apenas a moldura/título, sem os controles desenhados
  pelo jogo. Nenhum clique em Canal/VPS nem conexão foi executado; o client
  foi deixado aberto para teste manual. Continua não `CLIENT_TESTED`.
- Gate da espera pelo status HTTP: `MODERNIZACAO_COMPATIVEL`. A leitura de
  população roda na thread da cena, mas WinINet não tinha timeout explícito;
  um endpoint indisponível podia atrasar a abertura dos canais. A sessão agora
  configura 1.500 ms para conexão e recebimento antes de abrir a URL, e
  falha fechando o handle se não puder aplicar os limites. O status continua
  apenas informativo; nenhum endpoint de jogo, packet ou índice de canal foi
  alterado. `Build-Client.ps1 -NoDeploy` passou 51.799 checks e compilou
  `Basedef.cpp` em Release|x86; candidato SHA-256
  `85302AAFA694F473ABA19E9F1E70D188784CB091D7CF3374EE3051E5AEBA68D6`.
  O teste do timeout verifica a ordem no source, não mede latência de rede.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`; o candidato
  não foi instalado sobre o client ainda aberto.
- Gate da leitura de `serverlist.bin`: `MODERNIZACAO_COMPATIVEL`. O asset local
  7.48 tem 7.040 bytes, exatamente o tamanho da tabela de 10 grupos x 11
  entradas x 64 bytes. O loader anterior ignorava leitura curta e decodificava
  bytes parciais como endpoints. Agora limpa a tabela antes da abertura, exige
  `sizeof g_pServerList` bytes e limpa novamente antes de rejeitar um arquivo
  truncado; arquivo válido, chave de decodificação, layout e wire não mudam.
  `Build-Client.ps1 -NoDeploy` passou 51.801 checks e recompilou `Basedef.cpp`
  em Release|x86; candidato SHA-256
  `1DD62DCBD9CBCB8184C9FB89E276FCD06CC64D3F4A4B61E6E8BCB41D3295ECE2`.
  A guarda foi verificada estaticamente, mas não houve injeção de asset
  truncado no executável nem teste de clique/conexão. `STATICALLY VERIFIED /
  AUTOMATED TESTED`, não `CLIENT_TESTED`; não instalado sobre o processo aberto.
- Gate da leitura exata executável: a verificação e limpeza de `serverlist.bin`
  foram isoladas em `ServerListAsset.h`, usada pelo loader real antes da mesma
  decodificação 7.48. Testes com arquivo ausente, completo e um byte curto
  confirmam rejeição e ausência de endpoints parciais; `Build-Client.ps1`
  passou 51.808 checks, compilou `Basedef.cpp` em Release|x86 e instalou
  `project.exe` SHA-256
  `13B282E638FB53F60FAB7696EC8E04E4DB0EBB8EB52024A388384ED86CAC94C3`.
  O emulador iniciou com PostgreSQL e listener 8281. O client abriu a janela,
  mas a captura falhou duas vezes com `SetIsBorderRequired failed:
  No such interface supported (0x80004002)`; nenhum clique foi realizado.
  O processo foi fechado para liberar a instalação do novo candidato.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, ainda não `CLIENT_TESTED`.

- Gate do parser de população: `MODERNIZACAO_COMPATIVEL`. As cinco leituras
  de status em `TMSelectServerScene` e `TMFieldScene` usavam um formato
  `sscanf_s` com `\\n` literal; uma resposta com LF/CRLF parava após o primeiro
  número. `ParseServerStatus` aceita LF, CRLF e o separador literal anterior,
  mantém entradas não lidas em `-1` e rejeita número fora do intervalo de
  `int`. O feed continua informativo: não muda endpoint, wire nem a decisão
  autoritativa do servidor. `Build-Client.ps1 -NoDeploy` passou 51.813 checks
  e compilou as duas cenas em Release|x86; candidato SHA-256
  `F1D96C0BF11E5554DEF2D04A212270A1599FD32B8EE4AACE61B723ECE05D41D6`.
  O build com deploy repetiu os 51.813 checks e instalou `project.exe`
  SHA-256 `C88DF1762673E22D269FBAF1ADB634B15AC31D6ACF3D64572D107A5C430CCC00`,
  idêntico ao artefato gerado. O client abriu, mas apareceu um `Windows Security
  Alert`; a janela de jogo mudou de identificador durante duas tentativas de
  captura. Nenhum alerta de segurança foi acionado nem houve clique no jogo.
  `STATICALLY VERIFIED / AUTOMATED TESTED`, não `CLIENT_TESTED`. A resposta
  HTTP real e a conexão pelos canais continuam pendentes.

Teste em jogo obrigatorio:

1. abrir a tela inicial em `800x600`, `1024x768` e `1280x960`;
2. confirmar que Server/Channel fica centralizado e que texto, overlay e
   hitboxes acompanham o frame;
3. selecionar canal, voltar do login e confirmar que a posicao nao muda;
4. entrar no mundo, executar logout/relogin e confirmar a nova instancia.
