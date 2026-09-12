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
dump, log ou temporário dentro das sources. Manter somente as duas raízes
ativas descritas acima. Evidência binária fica em `references/client748/`.

## Procedimento seguro

1. Usar a entrada única e os gates do `AGENTS.md`; não repetir a inspeção ao
   entrar nesta skill. Delimitar arquivos e consumidores da alteração.
2. Antes de mover ou remover, buscar referências aos alvos com `rg`. Inventário
   global só para reorganização global, não para editar um parágrafo.
3. Mover documentação para uma única categoria em `DOCS/` e corrigir links.
   Manter evidência técnica em `.agents/`, sem duplicar relatórios.
4. Remover somente duplicados, regras obsoletas e artefatos gerados com
   caminhos exatos confirmados. Não apagar source, assets ou evidência nativa
   por conveniência.
5. No fechamento do lote, executar `tools/repository/Test-RepositoryLayout.ps1`.
   Usar `-UpdateMap` somente se caminhos, entradas ou categorias do inventário
   mudarem; essa chamada já valida o resultado, sem uma segunda execução.
   Completar com `git diff --check` e os validadores dos recursos alterados.

## Skills

Cada skill ativa tem um único `SKILL.md` curto e específico. Não copiar regras
globais para todas as skills. Referências e scripts devem ter um propósito
claro, caminhos atuais e testes reproduzíveis.

Ao criar/revisar skills, usar `skill-creator` oferecida pela sessão. Validar
frontmatter e revisar cenários de acionamento, reaproveitamento de evidência e
bloqueio; um YAML válido não demonstra que o workflow evita loops. Não acionar
skills de gameplay apenas porque a documentação menciona client ou servidor.
