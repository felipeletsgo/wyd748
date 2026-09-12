# WYD 7.48 — client e servidor

Dois projetos integrados: o client C++ `tmproject/`, baseado na source 7.69 e
adaptado ao 7.48, e o servidor Go autoritativo `wydgo748/`. A meta é um client
e um servidor completos e interoperáveis; a adaptação e a validação em jogo
ainda têm trabalho pendente.

| Local | Conteúdo |
| --- | --- |
| `tmproject/TMProject748/` | Source C++ e testes do client |
| `tmproject/client748/` | Runtime e assets 7.48 |
| `wydgo748/` | Módulo Go, servidor, dados e testes |
| `DOCS/` | Documentação de arquitetura, operação e contratos |
| `references/client748/` | Evidência histórica; não executar os patchers |
| `.agents/` | Skills, pesquisa e continuidade |
| `tools/repository/` | Manutenção e validação da organização |

## Entradas

- [Documentação](DOCS/README.md) e [inventário completo](DOCS/documentation-map.md).
- [Regras únicas](AGENTS.md).
- [Operação do servidor](DOCS/server/operations.md).
- [Build e integração](DOCS/build-and-integration.md).
- [Implementado](DOCS/IMPLEMENTED.md) e [pendências](DOCS/ROADMAP.md).

O servidor valida intenções e mantém a verdade do jogo. Arquitetura posterior
pode ser aproveitada quando compatível com os assets e contratos 7.48, ou por
extensão explícita e testada nos dois projetos. Evidência nativa e testes
devem sustentar cada afirmação de paridade.
