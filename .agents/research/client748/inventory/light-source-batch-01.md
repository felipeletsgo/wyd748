# Lote 01 — unidades leves da source

## Escopo e decisão

Este lote inicia a documentação incremental dos 100 arquivos leves listados
para adaptação. A mudança é `MODERNIZACAO_COMPATIVEL`: somente comentários e
documentação foram alterados, sem modificar comportamento, ABI, wire, recursos
ou lifecycle.

A procedência nativa destas unidades não foi alegada. Os arquivos pertencem à
source do TMProject recebida pelo projeto; onde o equivalente 7.48 não foi
rastreado, o texto registra explicitamente a lacuna em vez de atribuir uma
origem histórica.

## Unidades documentadas

| Grupo | Arquivos | Estado atual |
| --- | --- | --- |
| Infraestrutura Win32 | `pch.h`, `pch.cpp`, `framework.h`, `targetver.h` | Ativos no build; comentários explicam dependências e fronteiras |
| Casca do projeto | `TMProject.h`, `TMProject.cpp` | Compilada; não contém bootstrap ou lógica de jogo |
| Cabeçalhos reservados | `AES.h`, `Base64.h`, `BlockKeyBoard.h`, `MD5Checksum.h`, `desbase64.h`, `StackWalker.h`, `Quest.h`, `md5.h`, `NewClientB.h` | Sem declarações e sem includes ativos encontrados |
| Stubs visuais | `TMFlail.h/.cpp`, `TMEffectGold.h/.cpp`, `TMSkillSnow.h/.cpp` | Compilados, mas deliberadamente inertes; recursos e lifecycle não confirmados |

## Limites da adaptação

- Não foram inventados algoritmos criptográficos, sistema de quest nem stack
  walker para preencher placeholders.
- `TMFlail`, `TMEffectGold` e `TMSkillSnow` não foram promovidos a efeitos
  funcionais. A ativação exige rastrear consumidores, recursos, entrada,
  atualização, renderização e teardown.
- Nomes de campos, assinaturas e layouts foram preservados para não introduzir
  alteração silenciosa de ABI.

Os demais arquivos do inventário serão tratados em lotes pequenos, começando
pelas declarações passivas e avançando para efeitos/entidades somente quando o
fluxo e os recursos necessários estiverem decididos.
