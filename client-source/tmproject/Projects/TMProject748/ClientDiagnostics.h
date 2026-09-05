#pragma once

class SControlContainer;

// Instala o diagnóstico opcional usado durante a adaptação do client 7.48. O
// handler grava um relatório compacto e um minidump sem alterar o fluxo normal
// do jogo nem exigir um depurador conectado.
void WYD748_InstallDiagnostics();

// Acrescenta uma linha com timestamp em client-debug.log. O log é separado do
// TMLog porque o handler de exceção pode executar enquanto TMLog já está sendo
// desmontado. format segue printf; os argumentos não são retidos.
void WYD748_DiagnosticsLog(const char* format, ...);

// Exporta a árvore de controles carregada para correlacionar IDs legados da
// FieldScene2 com os controles semânticos do código importado. container é uma
// referência não proprietária e deve ser válido; reason é apenas copiado ao log.
void WYD748_DumpControlTree(SControlContainer* container, const char* reason);
