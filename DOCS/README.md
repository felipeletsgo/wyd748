# Documentação do WYD-Go

Esta é a entrada única da documentação durável do repositório. O código está
separado em duas sources: [`../tmproject/`](../tmproject/) é o client e
[`../wydgo748/`](../wydgo748/) é o servidor autoritativo.

## Mapa

O inventário por assunto e por localização está em
[`documentation-map.md`](documentation-map.md). Ele distingue documentação
do produto, evidência de pesquisa e registros históricos de continuidade.

## Organização

- Documentação de arquitetura, status, protocolo, client e servidor fica em
  `DOCS/`.
- Evidência técnica que alimenta o trabalho do agente fica em
  `.agents/research/`; handoffs ficam em `.agents/handoffs/`.
- Skills operacionais ficam em `.agents/skills/`.
- `tmproject/` e `wydgo748/` são reservados para source, assets e dados
  necessários à execução/build. Não colocar documentação ou temporários ali.

## Princípios do produto

O client alvo é o WYD 7.48. A source TMProject disponível é posterior (7.69) e
é adaptada somente quando o contrato 7.48 ou uma extensão coordenada foi
decidida. O servidor Go valida intenções, mantém o estado autoritativo e
conversa com o client por um contrato comum de packets e estado.

As decisões de paridade devem registrar evidência nativa 7.48/Ghidra. O mapa
não transforma uma referência em prova: o estado real de cada investigação
fica na ficha ou relatório correspondente.
