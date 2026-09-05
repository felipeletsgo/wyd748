#pragma once

// Dependências básicas da plataforma Win32 usadas pelo client source-built.
// WIN32_LEAN_AND_MEAN reduz colisões e tempo de compilação ao excluir partes
// raramente usadas de windows.h.
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

// API Win32.
#include <windows.h>

// Biblioteca de runtime C necessária ao código legado.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
