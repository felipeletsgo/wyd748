#pragma once

// Define a versao de API Win32 disponibilizada pelo SDK durante a compilacao.
// Este cabecalho pertence ao bootstrap da plataforma e nao contem estado de
// runtime. Para suportar uma versao anterior do Windows, o projeto deve definir
// _WIN32_WINNT explicitamente antes de incluir WinSDKVer.h/SDKDDKVer.h.
#include <SDKDDKVer.h>
