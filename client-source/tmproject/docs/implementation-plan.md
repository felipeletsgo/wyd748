# Plano de implementação arquitetural

## Princípios de execução

- Uma unidade pequena por vez; nunca migrar um subsistema inteiro sem build intermediário.
- Classificação padrão: `MODERNIZACAO_COMPATIVEL`.
- Preservar ABI, packing, opcodes, nomes públicos e ordem de teardown.
- Cada etapa termina com evidência: diff focado, build Debug Win32 e teste específico.
- Fachadas temporárias são permitidas, mas devem ter proprietário e critério de remoção.

## Fase 0 — Baseline e contratos

**Entregáveis:** inventário arquitetural, mapa de includes, testes de `sizeof`/`offsetof` das mensagens críticas e registro do executável produzido.

**Gate:** build limpo; nenhum layout alterado; catálogo de contratos revisado.

## Fase 1 — Portas mínimas

Criar interfaces pequenas para `ITransport`, `IClock`, `IResourceProvider`, `IRenderSink` e `IAudioSink`. Implementar adaptadores que chamem o código atual sem mudar o comportamento.

**Gate:** cenas compilam usando os adaptadores; smoke test de bootstrap, login e encerramento.

## Fase 2 — Wire e aplicação

Mover validação de tamanho/opcode e tradução de mensagens para `wire`. Criar casos de uso para login, mudança de cena, movimento e logout. `TMFieldScene` continua como facade e delega gradualmente.

**Gate:** cada opcode migrado tem teste de entrada válida, tamanho inválido e fallback; nenhuma mensagem é enviada duas vezes.

## Fase 3 — TMFieldScene

Separar por responsabilidades, nesta ordem: dispatch de pacotes, estado de interação (trade/party/quest), mix/inventário e posicionamento de painéis. Manter a classe atual como coordenadora de lifecycle.

**Gate:** abrir/fechar cena, reconectar, trade e mix preservam estado; build e teste manual de dois clientes quando aplicável.

## Fase 4 — Domínio de entidades

Extrair primeiro funções puras de `TMHuman` e entidades: cálculo, validação, seleção e transformação de estado. Depois introduzir comandos/resultados de domínio. O adaptador visual permanece fora do domínio.

**Gate:** testes determinísticos das regras; nenhum include de Win32/DirectX em `domain`.

## Fase 5 — UI e render

Dividir `SGrid` em modelo de dados e controle visual. Criar portas de render para efeitos e mundo; mover detalhes DirectX para adapters/render. Preservar IDs, callbacks e fechamento.

**Gate:** inventário, equipamento, trade e painéis existentes funcionam; validação visual manual somente após build estável.

## Fase 6 — Redução de `core`

Mover tipos para seus donos (`domain`, `wire`, `platform`) e reduzir `Basedef` a facade/adaptadores. Remover includes transitivos somente quando todos os consumidores incluírem o contrato correto.

**Gate:** busca de dependências proibidas, build completo e comparação de layouts/opcodes.

## Fase 7 — Endurecimento e limpeza

Adicionar documentação de ownership/lifecycle, eliminar fachadas sem consumidores, ativar warnings relevantes e atualizar o mapa arquitetural. Não remover código legado apenas por ausência no nativo 7.48.

**Definição de concluído:** todos os domínios têm dono claro, dependências seguem as regras, contratos críticos têm testes e o fluxo bootstrap→login→cena→mundo→logout foi validado.

## Ordem operacional por ciclo

1. Selecionar um único contrato/fluxo.
2. Mapear callers/callees e dependências reais.
3. Adicionar teste ou assertion de preservação.
4. Extrair/adaptar sem alterar semântica.
5. Atualizar `.vcxproj`, filtros e documentação.
6. Compilar e executar a validação focada.
7. Registrar resultado e só então iniciar o próximo ciclo.
