# TMProject 7.48 — mapa arquitetural

## Escopo

Este documento registra o inventário da árvore `internal/` após a migração
arquitetural. O mapeamento é estrutural: ele não transforma nomes de arquivos
em prova de paridade nativa nem altera contratos de wire, ABI ou lifecycle.

Inventário atual: 244 arquivos de código (`111 .cpp` e `133 .h`),
aproximadamente 118.342 linhas.

| Domínio | Arquivos | Responsabilidade |
|---|---:|---|
| `app` | 11 | bootstrap, cenas e coordenação do fluxo principal |
| `core` | 28 | tipos compartilhados, tabelas, recursos e compatibilidade |
| `game` | 25 | entidades, combate e estado de jogo |
| `platform` | 34 | Win32, entrada, mídia e integração do sistema |
| `render` | 132 | DirectX, mundo, efeitos, malhas e recursos visuais |
| `ui` | 9 | controles, grids e HUD |
| `wire` | 5 | mensagens e transporte/protocolo |

## Fluxo vivo

```text
platform/bootstrap -> app/scenes -> wire/transport
                                  -> game/entities
                                  -> ui
                                  -> render/world + render/effects
```

`core` fornece as estruturas e serviços transversais. `Basedef.h` continua
sendo uma fachada de compatibilidade para estruturas legadas; constantes de
layout e declarações globais já foram extraídas para `UiLayout.h` e
`BasedefGlobals.h` sem mudar a ordem ou o layout das estruturas.

## Arquivos extensos e decisão de fragmentação

Os maiores arquivos são `app/scenes/TMFieldScene.cpp` (~947 KB),
`game/entities/TMHuman.cpp` (~694 KB) e `ui/SGrid.cpp` (~184 KB). Eles
orquestram estado privado, callbacks, mensagens e teardown. A fragmentação
automática por função não é segura: exigiria expor estado privado e poderia
alterar ordem de inicialização ou dispatch. Por isso, a próxima divisão deve
ser feita por contrato independente, com testes de build entre cada extração.

## Estado da documentação

Comentários de contrato foram adicionados aos pontos de compatibilidade e aos
loaders de `Basedef`. `TMFieldScene.cpp` documenta a razão de manter os
handlers de compatibilidade juntos. Funções ainda sem comentário individual
devem ser documentadas durante a próxima extração, sempre explicando entrada,
efeito observável, ownership e cleanup quando aplicável.

## Regra para próximas alterações

Classificar cada extração como `MODERNIZACAO_COMPATIVEL`, preservar nomes
públicos e incluir os novos arquivos no `.vcxproj`/`.filters`. Validar com
MSBuild Debug Win32 e `git diff --check`; não declarar `CLIENT_TESTED` sem
execução manual do fluxo correspondente.
