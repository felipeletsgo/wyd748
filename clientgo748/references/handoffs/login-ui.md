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

Build/runtime 2026-09-10: `Build-ClientGo.ps1 -Configuration Release
-SkipTests` concluiu; o executável permaneceu ativo por 3 segundos no smoke
test e foi encerrado pelo harness. SHA-256 atual de `bin/wydclient.exe`:
`011532D3BB725F7ED0E5AF75A0147D3213F168A39B1B1C845DD3EAD98CE320EA`.
Isso confirma inicialização, mas não promove o fluxo para `CLIENT_TESTED`, pois
não houve interação visual automatizada com servidor/login.

Incremento 2026-09-10: `HandleSessionEvent(SessionDisconnected)` agora limpa a
seleção do servidor e o snapshot do mundo também para eventos de rede, com teste
de desconexão repetida, retorno à seleção e reconexão. `Verify-Fast.ps1` passou e
o commit `5cb44801` foi publicado em `origin/main`. A captura visual permanece
pendente: o helper Windows retornou `SetIsBorderRequired (0x80004002)` antes de
capturar a janela.

Incremento 2026-09-10: a cena de carregamento agora exibe `LOADING WORLD` e
`Please wait...` acima da barra de progresso, mantendo a geometria existente e
sem introduzir protocolo ou assets fictícios. `go test ./internal/loginflow
-count=1` e `git diff --check` passaram. A alteração ainda não foi validada
visualmente no executável devido à limitação do helper de captura.

Validação adicional 2026-09-10: `go test ./... -count=1` passou em todos os
pacotes; `Build-ClientGo.ps1 -Configuration Release -SkipTests` também concluiu
e gerou `bin/wydclient.exe`. O HUD da cena World foi publicado no commit
`a29c2fcd`; a execução continua sem classificação `CLIENT_TESTED` por falta de
interação visual automatizada funcional.

Incremento 2026-09-10: o `loginflow.Coordinator` agora descarta o cache de
entidades somente depois de uma confirmação válida `0x116` de logout. Uma
confirmação com `ClientID` divergente preserva o mundo atual; após a confirmação
válida, a fase retorna a `CharacterSelect` e um novo `0x213 -> 0x114` inicia com
cache vazio. O teste focado de logout/relogin e `Verify-Fast.ps1` passaram. O
build Release subsequente gerou `bin/wydclient.exe` com SHA-256
`B620E267B2395256D7EBA4BA7EE398C9E65C0DA0ECCB6B8D63368884E1B4DE22`.
Isso promove esta fronteira para `AUTOMATED TESTED`; a validação manual do ciclo
completo contra o WYD-Go ainda é necessária para `CLIENT_TESTED`.
