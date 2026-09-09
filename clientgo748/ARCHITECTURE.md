# Client Go 7.48 — arquitetura da fundação

## Objetivo

O `clientgo748` será um client novo em Go para o servidor WYD-Go. O servidor
continua autoritativo: o client envia intenções, recebe estado e renderiza a
apresentação. Regras de progressão, dano, inventário, skills, teleporte,
instâncias e recompensas não devem ser confiadas ao processo local.

## Camadas

```text
cmd/wydclient
       |
   lifecycle
       |
  protocol/session ---- domain/gameplay ---- assets
       |                     |                |
   platform/window       world/state       asset cache
       |
   graphics/renderer
```

### `cmd/wydclient`

Orquestra configuração, lifecycle e encerramento. Não contém regra de jogo,
parser binário ou chamada gráfica direta.

### `internal/protocol`

Cuida de framing, sessão, criptografia de transporte e packets. Cada opcode
deve ter contrato byte a byte e teste de aceitação/rejeição antes de ser usado
por gameplay. Os tipos não devem ser copiados automaticamente de versões
posteriores.

### `internal/domain` (próxima unidade)

Representará estado recebido do servidor, intenções do jogador e transições
locais de apresentação. Não será uma segunda autoridade de gameplay.

### `internal/assets`

Separa arquivos originais, conversão canônica, manifesto, pacote protegido e
cache runtime. O renderer só recebe estruturas validadas; não interpreta
offsets de arquivos históricos diretamente.

### `internal/graphics`

Expõe uma interface pequena de renderer. O backend inicial OpenGL/WGL em Go
puro já cria o contexto, acompanha o tamanho da área cliente, limpa e apresenta
o backbuffer. Texturas, device recovery e demais recursos só entram depois de
seus contratos próprios. A API não deve vazar handles ou ponteiros gráficos
para o restante do client.

### `internal/platform`

Encapsula janela, mensagens, input, relógio e caminhos do sistema operacional.
A primeira unidade Win32 já possui a janela e seu message loop; input e relógio
permanecem unidades futuras. O restante do client não depende diretamente de
Win32 ou de um toolkit.

## Lifecycle obrigatório

```text
Created -> Configured -> Connected -> Authenticated -> InWorld
    |          |             |             |             |
  Close      Fail          Close         Logout         Logout
    v          v             v             v             v
  Closed <-----+-------------+-------------+-------------+
```

Cada transição deve ter rollback, erro explícito e teardown idempotente. O
relogin deve liberar a sessão anterior, os recursos temporários e as referências
de UI antes de criar a próxima sessão.

## Regra de implementação

O TMProject só fornece um mapa estrutural. Não reutilizamos seu código,
ownership implícito ou comportamento sem validação. A fonte primária para uma
fronteira legada é o binário/descompilação WYD 7.48; o código atual, testes e
assets do projeto confirmam a integração. W2PP, Secrets e Micronics ficam fora
da cadeia de evidência.

## Gate antes de cada subsistema

1. contrato e procedência registrados;
2. API Go e ownership definidos;
3. parser/estado com testes de entrada válida e inválida;
4. lifecycle de sucesso, erro, logout e relogin coberto;
5. executar o menor gate proporcional: `Verify-Fast.ps1` para código ativo,
   `Verify-Contract.ps1` para protocolo/assets e `Verify-Mapping.ps1` para
   evidência/corpus/pesquisa, sempre com `git diff --check`;
6. validação visual ou client/server somente quando o subsistema realmente
   estiver conectado.

`go test ./...` continua incluído no `Verify-Fast.ps1`, mas não deve ser
repetido por cada etapa documental. `Build-ClientGo.ps1` só é necessário quando
o executável ou o smoke test Windows for afetado; após o gate rápido, use
`-SkipTests` para evitar duplicação.
