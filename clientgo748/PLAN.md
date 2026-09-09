# Plano de início do client WYD 7.48 em Go

Este plano inicia o client novo sem reutilizar código executável do TMProject.
O TMProject pode indicar nomes e organização, mas contratos legados devem ser
confirmados no binário nativo 7.48 e na descompilação estudada antes da
implementação.

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

Ainda não concluído: conclusão da validação do lifecycle de cenas, transporte,
login, cenas do mundo e UI de gameplay.

## Ordem de implementação

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

### 2. Lifecycle e cenas mínimas — implementação concluída, smoke pendente

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

Aceite: bootstrap → cena inicial → fechamento e bootstrap → logout/relogin em
testes determinísticos, incluindo falhas parciais.

### 3. Transporte e sessão

Antes de portar qualquer opcode, rastrear no Ghidra a entrada, framing,
criptografia, sequência, timeout, erros e teardown do nativo 7.48 e comparar
com os contratos server-authoritative atuais. Implementar transporte tipado,
cancelável e testável; manter mensagens visíveis em inglês.

Aceite: conexão local, handshake, autenticação e encerramento reproduzidos com
testes de bytes e integração com o servidor, sem inventar layout ou opcode.

### 4. Login, seleção de personagem e entrada no mundo

Implementar uma fatia vertical estreita: tela de login → lista de personagens
→ seleção → carregamento da cena do mundo. Cada transição terá ficha própria
com callers/callees, estados, erros, ownership e rollback.

Aceite: um personagem entra no mundo, recebe estado inicial e consegue sair e
entrar novamente sem crash ou dados antigos no client.

### 5. Mundo mínimo observável

Adicionar dispatcher de pacotes, relógio/tick, mapa, entidades e movimento
mínimos necessários para observar o personagem. O servidor continua sendo a
fonte de verdade; o client apenas interpola/apresenta o estado recebido.

Aceite: spawn, atualização, movimento, desconexão e limpeza de entidades com
testes de sequência e uma execução manual no client Go.

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

## Gate obrigatório de cada unidade

Antes de editar: `git status`, ficha de pesquisa e matriz de fontes. Para
fronteiras nativas, o mínimo é `TRACED` para comportamento e `CONTRACT` para
wire/ABI/asset. A ficha deve marcar binário nativo, Ghidra, assets oficiais,
WYD-Go, testes e referências secundárias como `UTILIZADA`, `NÃO APLICÁVEL` ou
`CONTRADITÓRIA`.

Depois de editar:

```powershell
go test ./...
pwsh -NoProfile -File .\Verify-Mapping.ps1
pwsh -NoProfile -File .\Build-ClientGo.ps1 -Configuration Debug
pwsh -NoProfile -File .\tools\Test-ClientBootstrap.ps1
git diff --check
python .agents/skills/wyd-client748-research/scripts/validate_research.py --repo .
```

Só usar `CLIENT_TESTED` após executar o fluxo real no executável Go. Cada
unidade aprovada deve ser commitada diretamente em `main`, publicada em
`origin/main` e registrada em `MAPPING.md` e no manifesto.

## Próxima ação concreta

Executar o smoke test do executável Go com a cena inicial e fechar a unidade 2
em `main`. Depois iniciar a pesquisa de transporte/login; nenhum opcode deve ser
adicionado antes da ficha `CONTRACT` correspondente.
