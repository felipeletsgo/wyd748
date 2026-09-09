# Matriz de procedência

| Fonte | Estado | Papel neste pacote |
| --- | --- | --- |
| `WYD.exe` nativo 7.48 + projeto Ghidra | `UTILIZADA` | autoridade para comportamento, UI, wire, ABI, lifecycle e assets materializados |
| `WYDoriginal.exe` stock | `UTILIZADA` | comparação histórica e confirmação de identidade |
| assets oficiais 7.48 | `UTILIZADA` | recursos, listas, IDs e loaders observáveis |
| source atual do WYD-Go | `UTILIZADA` | contratos server-authoritative, testes e integração |
| descompilação exportada | `UTILIZADA` | busca rápida; xrefs críticos devem ser confirmados no Ghidra |
| TMProject748 | `UTILIZADA` como comparação | organização e algoritmos candidatos; nenhum código é caminho ativo do client Go |
| guias e `DOCS` | `UTILIZADA` como contexto | regras de jogo e referências humanas, sem promover claim nativo |
| W2PP | `CONTRADITÓRIA/EXCLUÍDA` | fonte bugada; não valida nem orienta |
| Secrets | `CONTRADITÓRIA/EXCLUÍDA` | fonte bugada; não valida nem orienta |
| Micronics | `CONTRADITÓRIA/EXCLUÍDA` | source bugada; não valida nem orienta |

## Regras de promoção

Uma entrada só pode sair de `LOCATED` quando callers/callees, estado, efeitos,
erro e teardown estiverem resolvidos. Uma fronteira de wire/ABI/recurso exige
`CONTRACT`, com bytes, offsets, signedness, materialização e teste. O rótulo
`CLIENT_TESTED` exige execução real no `project.exe` e um cenário reproduzível.

O arquivo `catalog/functions.tsv` é a fonte operacional dos estados; este
documento explica a força de cada fonte e não substitui as fichas.
