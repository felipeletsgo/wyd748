#pragma once

// Fachada minima da plataforma Win32 usada pelo client source-built.
//
// Este arquivo delimita dependencias do sistema operacional; nao deve receber
// estado do jogo, protocolo ou bootstrap. targetver.h escolhe a versao minima
// do SDK e WIN32_LEAN_AND_MEAN reduz a superficie importada por windows.h.
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

// API nativa do Windows.
#include <windows.h>

// Partes do runtime C ainda exigidas pelo codigo legado.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
