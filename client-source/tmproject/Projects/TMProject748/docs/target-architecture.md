# Arquitetura-alvo do TMProject 7.48

## Decisão

O projeto deve evoluir para uma arquitetura em camadas, com dependências apontando para dentro. A compatibilidade legada fica nas bordas; regras de jogo e contratos não devem depender de DirectX, Win32 ou controles visuais.

```text
platform -> adapters -> application -> domain
presentation (ui/scenes) -> application
wire <-> adapters        -> application
render <- presentation/adapters
```

`core` não deve continuar como depósito geral. Ele será reduzido em etapas: tipos de domínio vão para `domain`, contratos de mensagem para `wire`, e serviços de sistema para `platform` ou `adapters`.

## Camadas

### `domain`

Estado e regras puras: entidades, inventário, combate, fórmulas, posições e value objects. Não inclui `HWND`, DirectX, singletons globais, logging de UI ou envio direto de pacote. Funções devem receber dados explicitamente e devolver resultado/erro.

### `application`

Casos de uso e orquestração: login, mudança de cena, movimento, trade, combate, quests e ciclo do campo. Define portas para transporte, relógio, recursos, áudio e renderização. `TMFieldScene` será migrado para coordenador de casos de uso, não para dono das regras.

### `wire`

Mensagens, serialização, validação de tamanho/opcode e tradução para comandos da aplicação. Layouts legados permanecem estáveis e recebem `static_assert`/testes de tamanho antes de qualquer alteração.

### `presentation`

`ui` e cenas exibem estado e traduzem input em comandos. Controles não devem alterar entidades diretamente; callbacks chamam casos de uso e atualizam a visão. A ordem de lifecycle permanece explícita.

### `adapters`

Implementações concretas das portas: transporte atual, tabelas/arquivos, recursos, DirectX e APIs antigas. Esta é a área apropriada para encapsular globals e funções de `Basedef` durante a transição.

### `platform`

Win32, janela, entrada, mídia, temporização e bootstrap. Não contém regra de jogo nem decisão de protocolo.

## Regras de dependência

1. `domain` não inclui headers de `platform`, `render`, `ui` ou `wire`.
2. `application` depende somente de `domain` e interfaces próprias.
3. `wire`, `render` e `platform` implementam/adaptam interfaces; não são importados pelo domínio.
4. Globals existentes são acessados por um adaptador único, inicialmente compatível com `BasedefGlobals.h`.
5. Cada extração deve preservar ABI, packing, opcode, ownership e teardown.

## Sequência de migração

1. Congelar o inventário e adicionar testes de layout/opcode.
2. Extrair portas de transporte, relógio, recursos e renderização.
3. Separar mensagens de `TMFieldScene` em handlers de aplicação, mantendo um facade temporário.
4. Migrar entidades de `TMHuman` para `domain`, começando por funções puras.
5. Dividir `SGrid` em modelo de dados e controle visual.
6. Encapsular globals de `Basedef` em adaptadores e reduzir a fachada.
7. Remover dependências proibidas por camada, uma compilação por vez.

Cada etapa deve ser pequena, compilável e reversível. Build valida compilação; não permite declarar migração ou fluxo concluído sem testes correspondentes.
