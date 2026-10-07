#pragma once

#include "TMGlobal.h"

// Opens/recreates the log file and initializes the global handle g_hLogFile.
// szLogFile must point to a zero-terminated string. Returns 1 when the file
// opens and 0 on failure.
char LOG_INITIALIZELOG(const char* szLogFile);

// Logs a formatted message with the local date and time.
void LOG_WRITELOG(const char* lpszFormat, ...);



// Logs only the internal error index.
void LOG_WRITEERROR(unsigned int dwErrorIndex);

// Writes formatted text without a timestamp; requires an initialized log.
void LOG_WRITELOGSTRING(const char* lpszFormat, ...);

// Flushes pending data and closes the global log file handle.
void LOG_FINALIZELOG();
