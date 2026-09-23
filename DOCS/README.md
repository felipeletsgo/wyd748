# Documentação do WYD-Go

Esta é a entrada única da documentação durável do repositório. O código está
separado em duas sources: [`../tmproject/`](../tmproject/) é o client e
[`../wydgo748/`](../wydgo748/) é o servidor autoritativo.

## Mapa

O inventário por assunto e por localização está em
[`documentation-map.md`](documentation-map.md). Ele distingue documentação
ativa do produto, evidência de pesquisa e registros de continuidade.

## Por onde começar

| Objetivo | Documento de entrada |
| --- | --- |
| Preparar o ambiente Windows | [Ambiente Windows](windows-development-environment.md) |
| Ligar o servidor pela primeira vez | [README da raiz](../README.md#início-rápido-no-windows) |
| Operar banco, contas, painel, rede e backup | [Operação do servidor](server/operations.md) |
| Compilar e validar servidor e client | [Build e integração](build-and-integration.md) |
| Entender o contrato de atributos | [Score](SCORE.md) |
| Consultar o estado do servidor | [Operação do servidor](server/operations.md) e código/testes em `wydgo748/` |
| Consultar pendências da adaptação 7.48 | [Handoff de paridade](../.agents/handoffs/client748-parity.md) e fichas em `.agents/research/client748/` |
| Adaptar o client 7.48 | [Port do client](client/port-748.md) |
| Consultar protocolos conhecidos | [Catálogo de opcodes](wire-opcode-catalog.md) |
| Manter a organização documental | [Regras do repositório](../AGENTS.md) e [inventário](documentation-map.md) |

O calendário, recompensas, comandos e pendências das guerras estão em
[Guerras de guilda](guild-wars.md). O plano do painel e da plataforma está em
[WYD Web Platform](WYD-WEB-PLATAFORM.md); as instruções executáveis do painel
integrado permanecem em [Operação do servidor](server/operations.md), para que
o plano de produto não vire um segundo tutorial concorrente.

As diferenças funcionais e arquiteturais encontradas na auditoria estática do
W2PP contra o servidor atual estão em
[Auditoria W2PP x WYD-Go](w2pp-go-gap-analysis.md); o W2PP é tratado ali como
referência comparativa, não como autoridade de paridade 7.48.

Snapshots antigos de status e planejamento foram retirados da documentação
ativa. Código, testes e fichas de evidência prevalecem sobre registros datados;
build automatizado não equivale a validação no client.

## Organização

- Documentação de arquitetura, status, protocolo, client e servidor fica em
  `DOCS/`.
- Evidência técnica que alimenta o trabalho do agente fica em
  `.agents/research/`; handoffs ficam em `.agents/handoffs/`.
- Skills operacionais ficam em `.agents/skills/`.
- `tmproject/` e `wydgo748/` são reservados para source, assets e dados
  necessários à execução/build. Não colocar documentação ou temporários ali.
- `testdata/protocol/` contém fixtures canônicos compartilhados entre client e
  server; não duplicar o mesmo frame dentro das duas sources.
- `tools/client-assets/Audit-ClientAssets.ps1` compara manifests e literais
  estáticos do TMProject com os assets materializados em `tmproject/client748/`.
  Dependências marcadas explicitamente como indisponíveis nos manifests são
  dívida conhecida e permanecem diagnósticas; `-FailOnMissing` falha somente
  para referências obrigatórias ausentes, faltas da source ainda não
  classificadas ou divergências de casing.

## Princípios do produto

O client alvo é o WYD 7.48. A source TMProject disponível é posterior (7.69) e
é adaptada somente quando o contrato 7.48 ou uma extensão coordenada foi
decidida. O servidor Go valida intenções, mantém o estado autoritativo e
conversa com o client por um contrato comum de packets e estado.

As decisões de paridade devem registrar evidência nativa 7.48/Ghidra. O mapa
não transforma uma referência em prova: o estado real de cada investigação
fica na ficha ou relatório correspondente.
