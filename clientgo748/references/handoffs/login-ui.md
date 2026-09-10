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

Build Release 2026-09-10: `Build-ClientGo.ps1 -Configuration Release
-SkipTests` concluído; SHA-256 de `bin/wydclient.exe`:
`459E4AF8C879C98FD76BD2E85A80D5CF3392C28FA8A2377B8F804EE90BA1FA6D`.

Evidência adicional: `0049889a_FUN_0049889a.c` confirma que a ação nativa de
New Account abre a transição modal `0x1202`; `004ac985_FUN_004ac985.c` valida
esse estado e encaminha o diálogo. O client Go ainda não possui endpoint nem
contrato de criação de conta, então o botão permanece deliberadamente sem ação
até existir uma extensão coordenada documentada.

Próximo passo: rastrear os callbacks 4609/4611/4610 em
`references/ghidra/corpus/004ac985_FUN_004ac985.c` e no handler da cena;
comparar `TMSelectServerScene.cpp` para nomes/semântica secundários. Implementar
Close/New Account sem inventar endpoint. Depois corrigir captions Connect/Close
e padding de canais na seleção usando geometria comum e métricas da fonte.
Executar e comparar a tela recompilada; ainda faltam callbacks de Connect/New
Account e validação real de mouse/teclado no executável.

Incremento 2026-09-10: o callback 4611 (Close) agora encaminha para
`VisualOptions.RequestClose`, com fallback explícito em inglês quando o
aplicativo não fornece o callback. O callback 4610 (New Account) é reconhecido
pela geometria nativa, mas permanece protegido com a mensagem `Account creation
is unavailable.` porque não existe contrato de criação de conta no transporte
Go. Nenhum opcode foi inventado; a transição modal nativa `0x1202` continua
registrada apenas como evidência de compatibilidade. Testes focados de
`loginflow` e `ui` passaram.
O teste de layout também cobre o clique real nos controles Close/New Account e
confirma que o callback de encerramento é chamado uma vez.
