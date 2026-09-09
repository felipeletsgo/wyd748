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

Expõe uma interface pequena de renderer. O backend OpenGL/purego será adicionado
após o contrato de janela, assets e perda/restauração de recursos estar
fechado. A API não deve vazar ponteiros de uma biblioteca gráfica para o
restante do client.

### `internal/platform`

Encapsula janela, mensagens, input, relógio e caminhos do sistema operacional.
O restante do client não deve depender diretamente de Win32 ou de um toolkit.

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
5. `go test ./...` executado dentro de `clientgo748` e `git diff --check`
   aprovados;
6. validação visual ou client/server somente quando o subsistema realmente
   estiver conectado.
