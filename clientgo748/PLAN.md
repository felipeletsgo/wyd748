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
composição compatível. Mensagens visíveis permanecem em inglês. Sem override,
`CONNECT` usa o endpoint padrão `127.0.0.1:8281`; `WYD_SERVER_ADDRESS` permite
substituí-lo antes de iniciar o processo.

Ainda falta a execução contra o servidor real e a validação manual do
executável Windows.

Aceite restante: um personagem entra no mundo, recebe estado inicial e
consegue sair e entrar novamente sem crash ou dados antigos no client.

### 5. Mundo mínimo observável

Adicionar dispatcher de pacotes, relógio/tick, mapa, entidades e movimento
mínimos necessários para observar o personagem. O servidor continua sendo a
fonte de verdade; o client apenas interpola/apresenta o estado recebido.

Aceite: spawn, atualização, movimento, desconexão e limpeza de entidades com
testes de sequência e uma execução manual no client Go.

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
2. Um MSH validado e exportado para geometria de diagnóstico.
3. Um personagem estático e depois animado usando MSH/BON/ANI/WYT.
4. Um trecho de terreno TRN com câmera e primitivas de depuração.

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
relogin real, antes de iniciar o mundo mínimo. A fonte/atlas nativo de texto e
os assets de personagem permanecem uma etapa visual posterior; seleção,
loading e mundo ainda usam composição provisória até cada binding de asset ser
confirmado.
