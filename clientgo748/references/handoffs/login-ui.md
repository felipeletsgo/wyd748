# Login UI — continuação

Escopo: finalizar servidor → login → personagens → mundo com assets originais.
Não declarar concluído pelo build ou pela presença do processo.

Incremento 2026-09-10: captions de UIString ligadas aos IDs do recurso real;
geometria de texto/input compartilhada antes de Render e após resize; validação
dos controles consumidos e cópia própria. Testes focados de assets/loginflow
passaram. A ficha `flows/login/login-screen-materialization.md` mantém LOCATED
para o lifecycle; o incremento não prova paridade visual.

Incremento adicional 2026-09-10: Escape no seletor agora usa o mesmo callback de
fechamento do botão nativo; títulos e linhas de canais foram alinhados à métrica
documentada do recurso (offset vertical de 5 px, coluna de canais deslocada 8 px
e largura de 140 px). Teste de fechamento por Escape e Verify-Fast passaram.

Validação de runtime 2026-09-10: `bin/wydclient.exe` iniciou pelo menos até a
janela/processo permanecer ativo por 3 segundos e foi encerrado limpidamente.
Não houve captura visual automatizada nesta sessão; portanto o fluxo continua
sem estado `CLIENT_TESTED`.

Próximo passo: rastrear os callbacks 4609/4611/4610 em
`references/ghidra/corpus/004ac985_FUN_004ac985.c` e no handler da cena;
comparar `TMSelectServerScene.cpp` para nomes/semântica secundários. Implementar
Close/New Account sem inventar endpoint. Depois corrigir captions Connect/Close
e padding de canais na seleção usando geometria comum e métricas da fonte.
Executar e comparar a tela recompilada; ainda faltam callbacks de Connect/New
Account e validação real de mouse/teclado no executável.
