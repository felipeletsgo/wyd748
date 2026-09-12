---
name: repo-architecture
description: Organizar a arquitetura, documentação, regras e skills do repositório WYD-Go sem misturar arquivos nas sources.
---

# Arquitetura do repositório

Use esta skill quando a tarefa for estrutural, documental ou de manutenção das
skills.

## Layout canônico

- `tmproject/`: somente client C++ 7.48 adaptado, runtime e assets necessários;
- `wydgo748/`: somente servidor Go autoritativo e seus dados/testes;
- `DOCS/`: documentação durável, com índice em `README.md` e inventário em
  `documentation-map.md`;
- `.agents/skills/`: skills ativas;
- `.agents/research/` e `.agents/handoffs/`: evidência e continuidade.

Não criar `AGENTS.md`, `CLAUDE.md`, README de projeto, script de pesquisa,
dump, log ou temporário dentro das sources. Não reintroduzir os nomes
`source-client`, `source-server` ou `clientgo748` como raízes ativas.

## Procedimento seguro

1. Conferir status, `HEAD` e a árvore atual; tratar o worktree como fonte de
   verdade e preservar alterações alheias.
2. Inventariar referências e links antes de mover ou remover arquivos.
3. Mover documentação para uma única categoria em `DOCS/` e corrigir links.
   Manter evidência técnica em `.agents/`, sem duplicar relatórios.
4. Remover somente duplicados, regras obsoletas e artefatos gerados com
   caminhos exatos confirmados. Não apagar source, assets ou evidência nativa
   por conveniência.
5. Atualizar o mapa central e validar layout, links relevantes, `git diff
   --check` e testes afetados.

## Skills

Cada skill ativa tem um único `SKILL.md` curto e específico. Não copiar regras
globais para todas as skills. Referências e scripts devem ter um propósito
claro, caminhos atuais e testes reproduzíveis.
