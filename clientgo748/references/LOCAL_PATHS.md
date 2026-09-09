# Tradução de caminhos históricos

Este pacote preserva fichas, handoffs, disassemblies e snapshots com os
caminhos que existiam quando a evidência foi produzida. Esses textos são
procedência histórica, não dependências operacionais. Para trabalhar no client
Go, use sempre a cópia equivalente abaixo, relativa à raiz de `clientgo748`.

| Caminho citado na evidência antiga | Cópia autocontida atual |
| --- | --- |
| `.agents/research/client748/flows/` | `references/research/flows/` |
| `.agents/research/client748/exports/` | `references/research/exports/` |
| `.agents/research/client748/inventory/` | `references/research/inventory/` |
| `.agents/handoffs/` | `references/handoffs/` |
| `.agents/skills/wyd-client748-*` | `references/skills/` e `tools/research/` |
| `.agents/skills/wyd-go-feature/references/` | `references/skills/` |
| `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled` | `references/ghidra/corpus/` |
| `%USERPROFILE%\Tools\GhidraProjects\WYD748Native_20260821.gpr` | `references/ghidra/project/WYD748Native_20260821.gpr` |
| `client748/wyd.exe nativo+patches/WYD.exe` | `references/ghidra/input/WYD.exe` |
| `client748/wyd.exe nativo+patches/WYDoriginal.exe` | `CLIENT OFICIAL 7.48/WYD.exe` |
| `client-source/tmproject/` | `references/tmproject/TMProject748/` (somente comparação) |
| source e dados do servidor na antiga raiz | `references/server-source/` e `references/server-data/` |
| documentação do servidor na antiga raiz | `references/server-docs/` |
| dados permitidos da antiga pasta `Tools` | `references/legacy-tools-data/` |
| `client748/project.exe` | sem equivalente ativo; o produto Go é `bin/wydclient.exe` |

Os arquivos de `references/project-rules/`, `references/handoffs/`,
`references/research/flows/`, `references/tmproject/` e `DECOMP/` podem citar os
caminhos da coluna esquerda sem tornar o pacote dependente deles. Não altere
essas citações em massa: elas registram a origem e o contexto temporal da
evidência.

As ferramentas ativas ficam em `tools/`, o módulo compilável começa em
`go.mod` e os assets ativos ficam em `assets/current/`. O gate
`tools/Test-SelfContained.ps1` confirma que esses pontos de entrada não usam
links ou caminhos operacionais externos.
