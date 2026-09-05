#ifndef PCH_H
#define PCH_H

// Cabeçalho pré-compilado compartilhado pela source Win32 do client.
// Concentre aqui apenas dependências estáveis e amplamente usadas: cada include
// aumenta o acoplamento e o custo de recompilação de todas as unidades.
#include "framework.h"

#include <Windows.h>
#include <shellapi.h>
#include <cstdio>
#include <algorithm>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#define DIRECTINPUT_VERSION 0x0800

// Os headers legados do DirectX 9 geram avisos fora do controle do projeto.
// A supressão fica limitada aos próprios headers; os avisos da source continuam
// sujeitos à configuração normal do compilador.
#pragma warning(push, 0)
#include <d3d9.h>
#include <d3dx9.h>
#include <Dshow.h>
#pragma warning(pop)

#include <iostream>
#include <fileapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iphlpapi.h>
#include <chrono>
using namespace std::chrono_literals;

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Strmiids.lib")

// Tipos compartilhados com o servidor. Alterações de layout nesta fronteira
// exigem validação explícita de ABI/wire nos dois lados.
#include "SharedStructs.h"

#endif // PCH_H
