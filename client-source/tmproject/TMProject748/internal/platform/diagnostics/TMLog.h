#pragma once

#include "TMGlobal.h"

// Abre/recria o arquivo de log e inicializa o handle global g_hLogFile.
// szLogFile deve apontar para uma string terminada em zero. Retorna 1 em caso
// de abertura bem-sucedida e 0 em caso de falha.
char LOG_INITIALIZELOG(const char* szLogFile);

// Registra uma mensagem formatada com data e hora locais.
void LOG_WRITELOG(const char* lpszFormat, ...);

// Registra um codigo de erro em formato hexadecimal.
void LOG_WRITELOG(unsigned int dwError);

// Registra o indice interno e o codigo devolvido pelo sistema operacional.
void LOG_WRITESYSERROR(unsigned int dwErrorIndex, unsigned int dwErrorCode);

// Registra apenas o indice interno de erro.
void LOG_WRITEERROR(unsigned int dwErrorIndex);

// Escreve texto formatado sem adicionar timestamp; exige log inicializado.
void LOG_WRITELOGSTRING(const char* lpszFormat, ...);

// Confirma dados pendentes e encerra o handle global do arquivo de log.
void LOG_FINALIZELOG();
