# Diagnóstico local de conexão e personagens

Modo: MODERNIZACAO_COMPATIVEL. Procedência: implementação local Go.
Source atual e testes WYD-Go: UTILIZADA para os pontos de instrumentação.
Binário 7.48, Ghidra/descompilação, assets, TMProject e guias: NÃO APLICÁVEL
ao delta de observabilidade; nenhum opcode, offset, loader ou contrato foi
adaptado. Não há novo claim de paridade nativa neste lote.

Cada execução grava `logs/<data-hora-pid>/client.log` ao lado do executável.
O diretório `dumps/` dessa execução contém apenas cabeçalhos lógicos decifrados
de 12 bytes. Não são frames completos reproduzíveis: o corpo é deliberadamente
omitido, pois autenticação e lista de personagens contêm senha/SecretCode.
O log conserva direção, opcode, tamanho original, ID e nome do dump. TX indica
tentativa de envio; `send_error` identifica falha posterior de escrita.

Eventos `server_selected`, `connect_begin/end`, `auth_begin/end`,
`packet_dispatch` e `scene_current` localizam a parada no login. Eventos
`character_list_build`, `character_model_built`, `character_build_end` e
`character_draw_ok` distinguem lista recebida, montagem e primeiro draw sem erro.
Draw sem erro não comprova visibilidade na tela. `client_exit` registra erros
propagados de inicialização, cena ou renderização; não é um minidump de crash.

Para reproduzir: iniciar o executável atualizado, selecionar servidor e canal,
clicar Connect, preencher o login e tentar entrar. Examinar a pasta de logs
mais recente. Falha ao criar logs não impede execução. O diretório por execução
evita sobrescrever dumps anteriores. Remoção de logs antigos é manual.

Validação do fluxo real e correção da causa da conexão/personagens permanecem
pendentes da reprodução com esse executável.
