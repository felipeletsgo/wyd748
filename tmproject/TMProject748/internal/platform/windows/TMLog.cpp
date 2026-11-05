#include "pch.h"
#include "TMLog.h"
#include "LegacyLogFile.h"

namespace
{
	// Legacy flags 33049: write-only, append, random access, create, binary.
	constexpr int kLogOpenFlags = _O_WRONLY | _O_APPEND | _O_RANDOM | _O_CREAT | _O_BINARY;
	static_assert(kLogOpenFlags == 33049, "the log keeps its established open flags");

	struct CrtLogIo
	{
		static int Open(const char* path) { return _open(path, kLogOpenFlags, _S_IWRITE); }
		static int Write(int descriptor, const char* data, unsigned int size) { return _write(descriptor, data, size); }
		static int Commit(int descriptor) { return _commit(descriptor); }
		static int Close(int descriptor) { return _close(descriptor); }
	};

	// Open state is separate from the descriptor value: before initialization,
	// after a failed open and after finalization, no descriptor is used.
	legacy_log::File<CrtLogIo> g_logFile;

	void PublishDescriptor()
	{
		g_hLogFile = g_logFile.Descriptor();
	}
}

char LOG_INITIALIZELOG(const char* szLogFile)
{
	if (!szLogFile || !szLogFile[0])
		return 0;

	// The open flags append without truncation, so remove the previous log.
	// The name resolves against the current directory, as _open does.
	DeleteFileA(szLogFile);
	Sleep(100);

	const bool opened = g_logFile.Open(szLogFile);
	PublishDescriptor();
	if (!opened)
		return 0;

	LOG_WRITELOGSTRING("------------------------------------------\r\n");
	LOG_WRITELOGSTRING(">>> Logging system turn on.\r\n");
	LOG_WRITELOGSTRING(">> mark \r\n");
	return 1;
}

void LOG_WRITELOG(const char* lpszFormat, ...)
{
	if (!g_logFile.IsOpen() || !lpszFormat)
		return;

	SYSTEMTIME sysTime;
	char szBuffer[1024];
	va_list va;

	va_start(va, lpszFormat);
	vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, va);
	va_end(va);
	GetLocalTime(&sysTime);

	LOG_WRITELOGSTRING("%02d/%02d %02d:%02d:%02d - %s",
		sysTime.wMonth,
		sysTime.wDay,
		sysTime.wHour,
		sysTime.wMinute,
		sysTime.wSecond,
		szBuffer);
}

void LOG_WRITEERROR(unsigned int dwErrorIndex)
{
	LOG_WRITELOG("Error : 0x%x\r\n", dwErrorIndex);
}

void LOG_WRITELOGSTRING(const char* lpszFormat, ...)
{
	if (!g_logFile.IsOpen() || !lpszFormat)
		return;

	char szBuffer[1024];
	va_list va;
	va_start(va, lpszFormat);
	const int nBuf = vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, va);
	va_end(va);

	// vsnprintf returns the untruncated length; write only the buffered part.
	g_logFile.Write(szBuffer, nBuf, sizeof(szBuffer));
}

void LOG_FINALIZELOG()
{
	g_logFile.Close();
	PublishDescriptor();
}
