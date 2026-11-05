#pragma once

#include "TMGlobal.h"

// Opens/recreates the log file and publishes its descriptor in g_hLogFile
// (-1 while closed). szLogFile must point to a zero-terminated string.
// Returns 1 when the file opens and 0 on failure.
char LOG_INITIALIZELOG(const char* szLogFile);

// Logs a formatted message with the local date and time.
void LOG_WRITELOG(const char* lpszFormat, ...);



// Logs only the internal error index.
void LOG_WRITEERROR(unsigned int dwErrorIndex);

// Writes formatted text without a timestamp. Text past 1,023 bytes is
// truncated; calls while the log is closed are ignored.
void LOG_WRITELOGSTRING(const char* lpszFormat, ...);

// Flushes and closes an open log once; later calls are ignored.
void LOG_FINALIZELOG();
