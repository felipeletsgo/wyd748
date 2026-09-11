# Plano de início do client WYD 7.48 em Go

Este plano inicia o client novo em Go sem reutilizar código executável do
TMProject. Para o fluxo de login, seleção de personagem e entrada no mundo, a
compatibilidade do TMProject com o 7.48 foi autorizada como referência
secundária de composição e comportamento visual. Contratos legados e
fronteiras de ABI continuam confirmados pelo binário nativo 7.48 e pela
descompilação estudada antes da implementação.

## Estado de partida

Já concluído:

- módulo Go independente em `clientgo748/`;
- janela Win32 e contexto OpenGL/WGL sem CGO;
- parser seguro de WYT e textura RGBA8 própria;
- formato canônico `WYDASSET` e conversor offline;
- pacote protegido com manifesto, zlib, AES-256-GCM e assinatura Ed25519;
- cache runtime protegido ligado ao renderer, com ferramenta offline de
  empacotamento e configuração sem chave privada no client;
- bootstrap com upload de textura e teardown na mesma thread;
- testes automatizados e smoke test de janela, resize, fechamento e Alt+F4.

Também foi adotada uma camada operacional de assets, separada do catálogo de
funções nativas. O census determinístico encontrou 7.370 arquivos em
`assets/current` (incluindo as duas tabelas JSON), agrupados por família em
`references/assets/asset-map.tsv`. Relações ainda não comprovadas permanecem
`UNKNOWN` em `asset-dependencies.tsv`; a única relação confirmada é a
materialização de `UI/logo1.wyt` documentada na ficha de textura.

O transporte/framing já foi implementado e integrado ao lifecycle da aplicação,
mas ainda não foi exercitado contra o servidor real. Os layouts, parsers, a
máquina de estados, a fila de eventos do socket, o dispatcher e o controller de
login estão implementados e cobertos por testes automatizados. As cenas do
fluxo real, mundo e UI de gameplay continuam pendentes.

## Ordem de implementação

O trabalho passa a ser organizado em fatias verticais e três trilhas que podem
avançar em paralelo: protocolo/login, pipeline de assets e renderer. A
prioridade de produto é fechar uma cadeia visual pequena com dados reais, sem
bloquear a contraparte de comunicação já existente no emulador.

### 1. Assets protegidos no runtime — concluída

Criar uma API pequena `AssetSource`/`AssetCache` que receba bytes de pacote,
valide assinatura e conteúdo através de `OpenProtectedPackage`, entregue uma
cópia de `assets.Texture` e tenha ownership explícito. O renderer nunca deve
conhecer WYT, `WYDASSET` ou o formato do pacote.

Entregas:

- ferramenta offline para materializar o pacote a partir dos assets oficiais;
- provedor de chave pública e de chave de conteúdo injetáveis (sem chave
  privada no client e sem segredo hardcoded);
- logo protegido carregado no bootstrap;
- rejeição antes do upload para pacote inválido, asset ausente, hash errado,
  nonce repetido ou limite excedido;
- testes de cache, cópia/ownership, falha e limpeza.

Aceite: o executável Go renderiza o logo a partir do pacote protegido e fecha
sem vazamento; o fluxo inválido não cria textura OpenGL. Os testes, o smoke
test protegido e a conferência visual passaram.

### 2. Lifecycle e cenas mínimas — concluída

Separar `Application`, `SceneManager`, input/eventos e recursos por cena. Fechar
explicitamente textura, cache, renderer e janela; preparar recriação de contexto
e logout/relogin sem estado global pendurado.

Implementado nesta unidade:

- `internal/scene.Manager` com `Start`, transição agendada, falha parcial,
  `Exit`/`Close` ordenados e fechamento idempotente;
- `internal/input.Event` independente de Win32;
- tradução de resize, foco, teclado, mouse e fechamento na janela Win32;
- integração do manager ao loop da `Application`, mantendo o renderer como
  owner do contexto;
- ficha de contrato em
  `references/research/flows/lifecycle/go-scene-manager.md`.

Aceite: bootstrap → cena inicial → fechamento em testes determinísticos,
incluindo falhas parciais. O bootstrap integrado foi executado nos ambientes
default e protegido; logout/relogin real permanece dependente do transporte.

### 3. Transporte e sessão

Antes de portar qualquer opcode, rastrear no Ghidra a entrada, framing,
criptografia, sequência, timeout, erros e teardown do nativo 7.48 e comparar
com os contratos server-authoritative atuais. Implementar transporte tipado,
cancelável e testável; manter mensagens visíveis em inglês.

Implementado nesta unidade:

- `internal/protocol.ClientSession` com handshake, framing, criptografia,
  checksum, fragmentação e escrita parcial;
- recepção assíncrona em uma fila limitada, sem callbacks ou mutação de estado
  na goroutine do socket;
- limites e falhas do parser alinhados ao contrato nativo 7.48;
- drenagem limitada pela `Application` antes do update da cena, na thread
  principal, e fechamento da sessão antes das cenas e do renderer;
- testes de bytes, stream, ownership e ordem de teardown.

Estado: `CONTRACT` nativo e `AUTOMATED TESTED` na implementação Go. A conexão
contra o servidor real e o fluxo de login permanecem no próximo gate.

Aceite restante: conexão local, handshake, autenticação e encerramento
reproduzidos com testes de bytes e integração com o servidor, sem inventar
layout ou opcode.

### 4. Login, seleção de personagem e entrada no mundo

Implementar uma fatia vertical estreita: tela de login → lista de personagens
→ seleção → carregamento da cena do mundo. Cada transição terá ficha própria
com callers/callees, estados, erros, ownership e rollback.

Implementado nesta unidade:

- layouts tipados e parsers dos packets `0x20D`, `0x10A`, `0x213`, `0x114`,
  `0x215` e `0x116`;
- `internal/login.SessionState` com validação de ordem, rollback,
  desconexão, logout e relogin sem reutilizar snapshot;
- `internal/login.Dispatcher` para `0x10A`, `0x114` e `0x116`, com rejeição de
  packets na direção errada e composição futura com os domínios de mundo/UI;
- `internal/login.Controller` para os envios `0x20D`, `0x213` e `0x215`, com
  `Tick` e `KeyWord` injetáveis, envio único e rollback quando o transporte não
  aceita o packet;
- geração padrão de `KeyWord` por `crypto/rand`, preservando o campo de um byte
  do wire nativo sem reproduzir a dependência histórica de `_rand()`;
- limpeza da senha fornecida e dos packets temporários em sucesso, rejeição e
  falha de envio;
- callbacks de conexão/desconexão no lifecycle da aplicação, publicados
  somente depois do handshake e limpos depois do fechamento da sessão;
- testes byte a byte de tamanhos, offsets, campos reservados, truncamento,
  duplicação, respostas fora de ordem, falhas de envio, desconexão e relogin.

Estado: `CONTRACT` nativo e `AUTOMATED TESTED` no pacote Go. A entrega dos
packets pela sessão à thread principal, o dispatcher e o controller de envios
estão implementados. As factories visuais agora ligam a sequência
`ServerSelection -> Login -> CharacterSelect -> Loading -> World`. A tela de
servidor seleciona uma entrada com mouse/setas e só abre a sessão ao confirmar
em `CONNECT`/Enter; o endpoint é aplicado antes do socket e nunca pode ser
alterado por uma sessão viva. A tela de login usa `UI/loginbox2.wyt`, e a
seleção usa `UI/ServerList2.wyt`, ambos assets oficiais 7.48 escolhidos como
skin moderna; o TMProject foi usado somente como comparação autorizada para a
composição compatível. Mensagens visíveis permanecem em inglês. A seleção é
sempre a primeira cena e só abre o transporte depois de `CONNECT`/Enter. Sem
override, `CONNECT` usa o endpoint padrão `127.0.0.1:8281`;
`WYD_SERVER_ADDRESS` permite substituí-lo antes de iniciar o processo. Para
uma lista real de servidores, `WYD_SERVER_LIST` aceita entradas no formato
`Group|host:port;Another Group|host:port` ou
`Group|Channel|host:port`, validadas antes da janela e limitadas a 32 itens.
O formato legado de dois campos usa `Channel 1` e o painel direito nunca fica
sem o canal selecionado. A cópia ativa `assets/current` contém somente os assets modernos;
`loginbox.wyt`, `loginicon.wyt`, `LoginScene.bin` e `SelServerScene.bin`
permanecem apenas no corpus oficial read-only para referência nativa.

O servidor WYD-Go foi iniciado em `127.0.0.1:8281` com PostgreSQL local e
carregou o catálogo autoritativo, incluindo 36 teleportes, 104 skills e 355
volatiles. O executável Go também iniciou e permaneceu responsivo nesse
ambiente. A sessão disponível não possui automação de janelas nativas, portanto
o clique em `CONNECT`, o login, a seleção de personagem e o relogin ainda não
foram observados; essa lacuna não é promovida a `CLIENT_TESTED`.

Aceite restante: um personagem entra no mundo, recebe estado inicial e
consegue sair e entrar novamente sem crash ou dados antigos no client.

### 5. Mundo mínimo observável

Adicionar dispatcher de pacotes, relógio/tick, mapa, entidades e movimento
mínimos necessários para observar o personagem. O servidor continua sendo a
fonte de verdade; o client apenas interpola/apresenta o estado recebido.

Implementado nesta etapa: a cena `world` agora materializa uma superfície
diagnóstica a partir do snapshot `0x114`, com posição autoritativa, grade de
referência, marcador do personagem e HUD em inglês. A cena consulta o
viewport real e não cria coordenadas ou entidades locais. Isso fecha a primeira
observação visual do personagem sem fingir que o pipeline de terreno está
pronto.

Aceite: spawn, atualização, movimento, desconexão e limpeza de entidades com
testes de sequência e uma execução manual no client Go.

Implementado nesta unidade: dispatcher isolado para `0x364`/`0x363`,
`0x336`, `0x366`/`0x367`/`0x368` e `0x165`. Os envelopes coordenados são
validados antes da mutação; entidades, score, ação, título de trade e remoção
ficam em estado próprio e são limpos no disconnect/relogin. A integração com o
`loginflow.Coordinator` mantém o login separado do estado de gameplay.

Implementado nesta unidade: `worldScene` recebe uma fonte de snapshots do
coordenador e desenha entidades autoritativas relativas ao personagem, com
ordenação determinística, nomes diagnósticos em inglês e exclusão do próprio
personagem. A cena ordena uma cópia e não retém nem altera o estado do
coordenador.

Implementado nesta unidade: clique esquerdo na superfície do mundo produz uma
intenção `0x366` de caminhada. A origem, `ClientID` e fase vêm do snapshot da
sessão; destino, velocidade e efeito são codificados no contrato de 52 bytes,
e o pacote é enviado sem alterar posição local. O servidor continua
autoritativo para rota, colisão e posição resultante.

Ainda pendente: movimento contínuo real, terreno/mesh, renderização final de
personagens e execução manual contra o servidor.

### 5a. Pipeline de assets orientado ao produto — em andamento

Depois de WYT, fechar incrementalmente uma cadeia representativa de
personagem, sem converter todo o pacote de uma vez:

```text
seletor de personagem -> MSH -> material/textura -> WYT
                     -> BON -> ANI -> pose
```

Cada formato deve passar por parser com limites, modelo canônico, ferramenta de
inspeção/exportação diagnóstica e golden test antes de ser ligado ao renderer.
As dependências devem ser promovidas somente quando houver evidência nativa,
de recurso ou de runtime; extensões desconhecidas continuam explicitamente
pendentes. O renderer recebe `Texture`, `Mesh`, `Skeleton`, `Animation` e
`Terrain`, nunca caminhos ou bytes de formatos proprietários.

Milestones visuais:

1. WYT decodificado e exportável para imagem de diagnóstico (concluído para o
   logo).
2. Um MSH validado e exportado para geometria de diagnóstico (`AUTOMATED_TESTED`;
   renderer WGL já aceita XYZ + índices, ainda sem câmera/material).
3. Um personagem estático e depois animado usando MSH/BON/ANI/WYT.
4. Um trecho de terreno TRN com câmera e primitivas de depuração.

Progresso desta unidade: `internal/assets.ParseTerrain` valida o framing
observado em `Env/Character.trn` (nome, dois bytes de cabeçalho e 4.096
registros de 12 bytes, formando uma superfície 64x64), com limites de
memória, ownership independente e testes de entradas inválidas. A
descompilação nativa confirma o primeiro byte como altura assinada; o parser
expõe esse campo e preserva os outros 11 bytes em bruto. Textura, demais
atributos e colisão ainda não foram promovidos. O parser ainda não alimenta
colisão, movimento ou renderização de gameplay.

O contrato e a matriz de fontes estão em
`references/research/flows/render-assets/terrain-trn-loader.md`.

O loader MSH nativo foi rastreado em `FUN_004c097c`: oito DWORDs, matrizes de
64 bytes por entrada de paleta, IDs de 4 bytes, vértices de `stride*count` e
índices `uint16`. O client Go agora extrai somente o prefixo XYZ comprovado,
valida índices antes do draw e oferece `MeshRenderer` opcional no WGL. Esta
etapa ainda não define câmera, material, UV, skinning, BON/ANI nem aparência de
personagem e, portanto, não é `CLIENT_TESTED`.

BON/ANI agora possui parser e loader próprios em Go. Nesta família de assets,
a tradução/interpretação do formato pode seguir diretamente o parser do
TMProject748, cuja compatibilidade com 7.48 já foi validada no projeto; não é
necessário repetir engenharia reversa byte a byte do parser. A fronteira nativa
continua sendo usada para lifecycle, integração e contratos externos. O loader
preserva o BON bruto, lê `BoneAni4.txt`/`ValidIndex.bin`, acumula as matrizes ANI
e tolera ANI ausente como o TMProject. O catálogo oficial inteiro passa nos
testes automatizados. Pose, lookup de motion, quaternion e skinning continuam
pendentes e não são `CLIENT_TESTED`.

O `worldScene` agora aceita o TRN validado e projeta a altura confirmada em
uma superfície diagnóstica opcional. Essa projeção é somente visual: não cria
colisão, não altera a origem autoritativa do personagem e não replaneja o
movimento local. A ausência do arquivo mantém o diagnóstico desativado sem
impedir o bootstrap das cenas de login.

O empacotamento protegido continua separado desta trilha e só deve receber
formatos estabilizados; durante o desenvolvimento, o `assetc` e o cache local
podem trabalhar com `assets/current`.

### 6. UI e gameplay por fatias

Depois do caminho crítico, implementar inventário/equipamento, NPC/trade,
combate/skills, teleporte e instâncias em unidades separadas. Para cada uma:

1. rastrear o fluxo nativo 7.48 quando houver fronteira legada;
2. definir contrato client/server e mensagens em inglês;
3. implementar estado/UI com testes de erro e relogin;
4. validar no executável, não apenas no build.

Não iniciar todas as telas ao mesmo tempo. A primeira tela de gameplay deve ser
uma vertical completa, com abertura, interação, atualização, fechamento e
relogin.

### 7. Endurecimento e distribuição

Adicionar logs sem segredos, tratamento de crash, limites de memória, rotação e
revogação de `KeyID`, verificação de integridade dos assets, empacotamento e
atualização segura. Medir tempo de carregamento, uso de memória e estabilidade
em duas instâncias.

## Gate proporcional de cada unidade

Antes de editar: `git status`, ficha de pesquisa e matriz de fontes. Para
fronteiras nativas, o mínimo é `TRACED` para comportamento e `CONTRACT` para
wire/ABI/asset. A ficha deve marcar binário nativo, Ghidra, assets oficiais,
WYD-Go, testes e referências secundárias como `UTILIZADA`, `NÃO APLICÁVEL` ou
`CONTRADITÓRIA`.

Depois de editar, executar o menor gate que cubra a alteração:

```powershell
pwsh -NoProfile -File .\Verify-Fast.ps1       # código ativo
pwsh -NoProfile -File .\Verify-Contract.ps1   # protocolo/assets
pwsh -NoProfile -File .\Verify-Mapping.ps1    # evidência/pesquisa
```

O nível superior inclui os anteriores. Mudança de comportamento observável
continua sujeita à ficha e ao gate Ghidra; a separação reduz apenas trabalho
redundante, não reduz evidência. Build e smoke test Windows são executados
quando a unidade toca bootstrap, plataforma, renderer ou fluxo visual.

Só usar `CLIENT_TESTED` após executar o fluxo real no executável Go. Cada
unidade aprovada deve ser commitada diretamente em `main`, publicada em
`origin/main` e registrada em `MAPPING.md` e no manifesto.

## Próxima ação concreta

As cenas lógicas `Login` → `CharacterSelect` → `Loading` → `World` e o
`loginflow.Coordinator` foram adicionados. O coordenador dirige as transições
exclusivamente pelo estado atualizado na thread principal; `Application` chama
o sincronizador depois dos eventos de sessão/input e antes do update da cena.

Próximo gate: executar o ciclo completo contra o WYD-Go, incluindo logout e
relogin real, em uma sessão com controle de janela nativa. A fonte/atlas nativo
de texto e os assets de personagem permanecem uma etapa visual posterior;
seleção, loading e mundo ainda usam composição provisória até cada binding de
asset ser confirmado.
