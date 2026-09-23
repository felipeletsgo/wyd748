---
id: airmove-contract
title: Transporte aéreo 0xAD9 no client WYD 7.48
subsystem: transport
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-22
---

# Transporte aéreo 0xAD9 no client WYD 7.48

## Pergunta

Qual intenção o client envia ao iniciar/encerrar AirMove e quem decide o destino?

## Fronteira de evidência

- Executável canônico somente leitura: `references/client748/WYD.exe`, SHA-256
  acima. Análise nativa/Ghidra registrada nos artefatos
  `b0f942ba0403483066c168b3dbea8854e121d258` (frame),
  `c155d2500f64052fb2d65bb1dc8786bee40302a7` (start) e
  `ddab79441b3475059f820a0115a7f0ddf6eb6793` (end).
- Callgraph nativo: `FUN_0047EF1D` (frame), `FUN_0047F7DA` (start) e
  `FUN_0047FA1D` (end). TMProject 7.69 foi referência secundária.
- Source compilável: `TMFieldScene.cpp`, `TMFieldScene.h`, `TMHuman.cpp`,
  `Basedef.h`, `AirMoveContract.h`; servidor `airmove.go` e testes.

## Fluxo nativo 7.48

### Entrada observável

A interação com NPC visível `ID >= 1000`, face/head `63`, nibble baixo de
Merchant `7`, no chunk `(16,16)` abre a UI de AirMove. O clique de uma das
cinco rotas chama o start; a animação de subida, caminho e descida termina no
end. O pacote não carrega o ID do NPC nem coordenadas escolhidas pelo client.

### Callers

O fluxo de seleção da UI chama `FUN_0047F7DA`. O frame da cena
`FUN_0047EF1D` avança o estado e chama `FUN_0047FA1D` ao terminar; o fluxo de
ação de `TMHuman` também pode finalizar o movimento ao receber ação do server.

### Função principal

Start emite C->S `0xAD9`, 20 bytes, `Header.ID` do personagem local, `Parm1`
rota `0..4`, `Parm2=1`. O frame executa estados de subida e descida de 2800 ms
cada, além da travessia visual dos waypoints. End emite o mesmo envelope com
`Parm1` da rota e `Parm2=2`.

### Callees

O transporte de pacotes recebe a estrutura temporária; a cena controla mount,
altura, partícula e posição visual. O servidor decide se aceita a intenção e
qual posição autoritativa publicar.

### Saídas e erros

Índice fora de `0..4` não deve acessar a tabela de rotas. Fim duplicado não
deve repetir o envio. Pacotes forjados, início sem NPC/contexto e fim
antecipado não concedem destino.

## Estado e lifecycle

### Matriz de transições

| Evento | Precondição | Estado resultante | Wire | Falha |
| --- | --- | --- | --- | --- |
| abrir UI | NPC visível compatível no chunk 16 | seleção visível | nenhum | sem NPC, sem UI |
| start | rota `0..4`, humano local | voo ativo, estado 2 | `0xAD9/20`, modo 1 | rota inválida, sem acesso à tabela |
| frame | voo ativo | subida, waypoints, descida | nenhum | sem destino arbitrário |
| end | voo ativo | voo inativo | `0xAD9/20`, modo 2 | repetição sem novo envio |
| morte/teleporte externo | voo pendente no servidor | pendência cancelada | nenhum destino AirMove | fim tardio ignorado |

### Vtables, vptrs e receptores

Não há mudança de vtable: as chamadas existentes da `TMFieldScene` e de
`TMHuman` recebem o mesmo estado de cena. O wire não adiciona receptor S->C.

### Ownership

Os estados de animação e rota pertencem à FieldScene; o packet está na pilha.
O servidor guarda apenas rota, origem e instante por personagem em sessão.

### Falha parcial

Sem `m_pMyHuman` ou rota válida, start retorna antes de construir efeitos ou
packet. No servidor, falha de teleporte não publica destino e encerra o gate.
Na UI, a inicialização compacta 7.48 vincula também o título `12549` e as
colunas `12550..12552` do painel `12544`, observados no dump real de
`FieldScene2.bin` em `tmproject/client748/client-debug.log` (primeiro bloco
`compat-field-before-hide`). A source importada usava `12551/52/53/60`.
Sem lista, texto principal ou nomes de rota carregados, a abertura
falha com painel oculto e flag limpa. O fechamento não depende da lista.
O titulo e aplicado diretamente ao controle; os cinco nomes de rota sao
copiados com limite de 18 caracteres, sem interpretar texto do recurso como
formato `printf`. Isso preserva o campo visual existente e evita overflow.
Na travessia, `AirMove_Main` produz um delta por frame. `TMHuman::SetPosition`
ja grava `m_vecPosition`; somar novamente o delta acumulado em cada
`TMHuman::Update` deslocava o personagem mais de uma vez e o end o somava
outra vez. O delta pendente agora e consumido e zerado no update ou, se o
voo terminar antes do proximo update, no end. Os waypoints e o wire nao mudam.
No retorno da montaria temporaria, `UpdateMount` havia substituido o tipo por
`40` e zerado `m_stMountLook`. O end salvava somente o tipo anterior e o
escrevia em `m_nSkinMeshType` (corpo), deixando a montaria de voo ativa.
Agora preserva tambem o look e restaura ambos os campos da montaria antes de
reconstrui-la. Correcao interna `MODERNIZACAO_COMPATIVEL`; nao afirma que o
binario nativo usa estes mesmos campos ou a mesma sequencia de implementacao.
Na passagem entre waypoints, a source incrementava o indice antes de testar
o proximo elemento de `g_pAirMoveRoute[5][10]`. Uma rota que ocupasse a decima
posicao leria fora do array no fim. A consulta agora verifica o limite e a
sentinela `(0,0)` antes de incrementar; as cinco rotas e seus pontos nao
mudaram. E `MODERNIZACAO_COMPATIVEL`, sem nova afirmacao sobre a implementacao
nativa ou mudanca no wire/servidor.

### Cleanup e teardown

O fim restaura mount, animação e altura. O estado autoritativo é limpo no fim,
no teleporte externo e no reset de runtime de personagem.

### Shutdown

`SysQuit` continua permitido enquanto o voo está ativo; não há buffer retido
ou operação assíncrona proprietária da cena.

### Logout e relogin

Logout é permitido pelo gate e o reset de sessão remove rota e origem.
Relogin começa sem voo pendente; um `Parm2=2` antigo não concede destino.

## Wire, ABI e recursos

`MSG_STANDARDPARM2` tem 20 bytes: header 12, `Parm1` DWORD em `+12`, `Parm2`
DWORD em `+16`; opcode `0xAD9`; modos `1/2`; rotas `0..4`. A source contém
`g_pAirMoveRoute[5][10]`. Somente os pontos finais são autoritativos:

| Rota | Destino `(X,Y)` |
| --- | --- |
| 0 | `2112,2051` |
| 1 | `2372,2099` |
| 2 | `2220,1714` |
| 3 | `2365,2279` |
| 4 | `1826,1771` |

## Mapeamento atual

O client conserva o percurso visual e agora usa `AirMoveContract.h` para
opcode, tamanho, offsets, modos e limite de rota. O servidor aceita só o
envelope exato, exige NPC visível compatível e origem no chunk 16, guarda
início/rota, bloqueia outras intenções de gameplay durante o voo e teleporta
somente após fim correspondente e duração mínima. O servidor segue autoritativo.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Delta |
| --- | --- | --- | --- |
| C->S `0xAD9` | 20 B, dois modos | client emitia; servidor não reconhecia | contrato coordenado |
| cinco rotas | tabela nativa | client tinha tabela, bounds frouxos | guarda `0..4` |
| destino | terminal da rota | sem handler Go | destino autoritativo |
| replay/falsificação | não define confiança do servidor | sem gate | rejeição e cleanup |
| UI AirMove incompleta | painel de transporte usa título `12549` e colunas `12550..12552` no dump 7.48 | bootstrap compacto não vinculava textos; importado procura `12553/12560` ausentes | vincular IDs presentes e verificar dependências antes de exibir |

## Decisões

- `PARIDADE_NATIVA` no envelope, seleção e cinco rotas do client.
- `EXTENSAO_COORDENADA` no validador/estado autoritativo do servidor; o
  tempo mínimo de 5400 ms é regra de segurança com 200 ms de tolerância em
  relação aos dois estados de 2800 ms, não uma duração nativa exata.
- `MODERNIZACAO_COMPATIVEL` para o gate de recurso parcial e a copia limitada
  dos textos da UI: não altera packet, rotas ou resultado quando os controles
  e textos existem.
- `MODERNIZACAO_COMPATIVEL` para consumir o delta do voo uma so vez por frame,
  preservando os waypoints e os modos de start/end comprovados no 7.48.
- `MODERNIZACAO_COMPATIVEL` para impedir leitura apos a decima posicao da
  rota visual; os dados de percurso e o destino autoritativo nao mudam.
- Não remover estados de animação da source por parecerem inativos: não há
  demonstração de incompatibilidade com o 7.48.

## Lacunas

- Os datasets atuais não contêm NPC face 63/Merchant 7 correspondente;
  preservar essa ausência, sem inventar spawn, nome, preço ou rota.
- A UI, o NPC e a viagem não foram exercitados no client construído. Falta
  teste real para promover a `CLIENT_TESTED` e confirmar falhas visuais.
- O build e os testes de wire não exercitam abertura/fechamento da UI nem
  comprovam a presença de cada texto no recurso carregado; verificar no
  client com NPC de transporte real antes de declarar o fluxo operacional.

## Validação

- Testes Go focados cobrem cinco destinos, falsificação, tamanho, replay,
  morte e cleanup. `PacketViewTests` fixa constantes/limites C++ e os asserts
  em `Basedef.h` fixam o layout real da estrutura.
- `go test -count=1 ./...` e `go vet ./...` passaram no lote do contrato;
  não foram repetidos para este ajuste só de UI. O build integrado após a
  guarda de recurso e a vinculação dos IDs reais passaram 51.693 checks e
  instalaram `project.exe` SHA-256
  `A29A600F5DF1B54ADFC352D8E02584185856C33F670D11B84F558444C7A4890A`.
- Validador de fichas, layout/links e `git diff --check` passaram. O gate da
  UI é build estático; a abertura real com NPC continua sem teste.
- A copia limitada dos cinco nomes e a remocao de formato controlado pelo
  recurso passaram novamente no build Release integrado e nos 51.693 checks.
  O candidato instalado tem SHA-256
  `10791835D8098F3F6D08D7951568C4EE5535BC5375F3375E2ABEAAAF2736B1FA`.
- O teste C++ de regressao aplica um delta e verifica que o frame seguinte
  nao o reaplica. Build Release integrado passou com 51.695 checks e instalou
  `project.exe` SHA-256
  `2140019A43B8B46D3739B6EA12375549CEFDFB2ACF98C0A55EF3141469A402C6`.
  O teste isolado nao substitui a viagem real no client.
- `STATICALLY VERIFIED` e `AUTOMATED TESTED`; não `CLIENT_TESTED`.
