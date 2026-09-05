# Lote 02 — interfaces leves e fluxos curtos

## Escopo e decisão

Este lote documenta mais 25 dos 100 arquivos leves selecionados para adaptação
incremental. A alteração é `MODERNIZACAO_COMPATIVEL`: comentários em português
foram acrescentados sem mudar assinaturas, layout de classes, ABI, wire,
recursos, ordem de efeitos ou comportamento de runtime.

Os arquivos pertencem à source atual derivada do TMProject. A presença de um
tipo nesta source não comprova que ele existia no executável nativo 7.48. Por
isso, a documentação descreve o comportamento vivo observável no código atual
e separa explicitamente os stubs posteriores ainda sem contrato 7.48.

## Unidades documentadas

| Grupo | Arquivos | Estado atual |
| --- | --- | --- |
| Cena posterior inerte | `TMDemoScene.h/.cpp` | Interface preservada; controles, assets, câmera, timeline e teardown ainda não rastreados |
| Efeitos e skills | `TMEffectFirework.h`, `TMArrow.h`, `TMEffectBillBoard2.h`, `TMEffectBillBoard3.h`, `TMEffectMesh.h`, `TMEffectMeshRotate.h`, `TMEffectSpark.h`, `TMEffectSpark2.h`, `TMSkillDoubleSwing.h`, `TMSkillMeteorStorm.h`, `TMSkillLusterFurnish.cpp`, `TMShade.h` | Contratos internos explicados; sem alegação nova de paridade nativa |
| Mundo e geometria | `TMItem.h`, `TMGate.h`, `GeomObject.h`, `CMesh.h` | Responsabilidades, referências e limites de ownership documentados; layout preservado |
| Texto e tempo | `TMFont2.h`, `TMFont3.h`, `TimerManager.h` | Buffers, retornos e bases de tempo descritos sem alterar limites legados |
| Diagnóstico local | `ClientDiagnostics.h` | Comentários traduzidos e parâmetros/ownership explicitados |
| Macro ativa | `MacroFactory.cpp`, `MacroMsg.cpp` | Criação sob demanda, ownership e roteamento por IDs/área documentados |
| Render posterior inerte | `JBlur.cpp` | Pipeline continua sem draw calls nem alocação de recursos |

## Decisões de adaptação

- `TMDemoScene` e `JBlur` continuam como stubs deliberados. Implementá-los por
  semelhança com uma versão posterior inventaria recursos e lifecycle ainda não
  confirmados para o client ativo.
- Os efeitos e entidades que já possuem implementação permanecem ativos como
  source compatível. Este lote não promove sua origem para `PARIDADE_NATIVA`.
- Ponteiros de owner, target, fonte, surface, fonte de texto e recursos globais
  foram documentados como referências não proprietárias apenas onde a API os
  recebe/consulta. Ponteiros auxiliares com criação parcial foram marcados como
  potencialmente nulos.
- Offsets comentados sem ficha de ABI em `CMesh.h` foram removidos dos
  comentários: os campos e sua ordem não mudaram, mas números não comprovados
  não devem aparentar contrato 7.48.
- Nenhuma refatoração estrutural foi necessária. Preservar o fluxo foi mais
  seguro que renomear membros, converter buffers ou reorganizar classes neste
  lote documental.

## Validação exigida

Como não há mudança funcional, o gate deste lote é validação estática e build
Release da source. O resultado não deve ser classificado como `CLIENT_TESTED`
sem exercício manual do client.
