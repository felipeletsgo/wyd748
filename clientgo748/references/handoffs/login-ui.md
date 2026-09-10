# Login UI — continuação

Escopo: finalizar servidor → login → personagens → mundo com assets originais.
Não declarar concluído pelo build ou pela presença do processo.

Incremento 2026-09-10: captions de UIString ligadas aos IDs do recurso real;
geometria de texto/input compartilhada antes de Render e após resize; validação
dos controles consumidos e cópia própria. Testes focados de assets/loginflow
passaram. A ficha `flows/login/login-screen-materialization.md` mantém LOCATED
para o lifecycle; o incremento não prova paridade visual.

Próximo passo: rastrear os callbacks 4609/4611/4610 em
`references/ghidra/corpus/004ac985_FUN_004ac985.c` e no handler da cena;
comparar `TMSelectServerScene.cpp` para nomes/semântica secundários. Implementar
Close/New Account sem inventar endpoint. Depois corrigir captions Connect/Close
e padding de canais na seleção usando geometria comum e métricas da fonte.
Executar e comparar a tela recompilada; ainda faltam callbacks e validação real.
