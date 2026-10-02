#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "ItemEffect.h"
#include "NativeItemVolatile.h"
#include "WYD748Assets.h"
#include "ServerListAsset.h"
#include <WinInet.h>

// Initialization, data loading, and resource normalization.
/** Converts logical width to pixels using the width of g_pDevice.
 * Requires a valid device; does not use height, allocate, or change state. */
float BASE_ScreenResize(float size)
{
	return (float)g_pDevice->m_dwScreenWidth * (size / WYD748_UI_BASE_WIDTH);
}

int BASE_GetHttpRequest(char* httpname, char* Request, int MaxBuffer)
{
    if (!httpname || !Request || MaxBuffer <= 0)
        return 0;

    Request[0] = '\0';
    auto hSession = InternetOpen("MS", 0, 0, 0, 0);
    if (!hSession)
        return 0;

    // Population is advisory and is fetched on the scene thread. Do not let
    // an unavailable status host stall channel selection for WinINet defaults.
    DWORD statusTimeoutMs = 1500;
    if (!InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT,
            &statusTimeoutMs, sizeof statusTimeoutMs) ||
        !InternetSetOption(hSession, INTERNET_OPTION_RECEIVE_TIMEOUT,
            &statusTimeoutMs, sizeof statusTimeoutMs))
    {
        InternetCloseHandle(hSession);
        return 0;
    }

    auto hHttpFile = InternetOpenUrl(hSession, httpname, 0, 0, 0x4000000u, 0);

    if (!hHttpFile)
    {
        GetLastError();
        InternetCloseHandle(hSession);
        return 0;
    }

    DWORD dwBytesRead = 0;
    const BOOL readSucceeded = InternetReadFile(hHttpFile, Request,
        static_cast<DWORD>(MaxBuffer - 1), &dwBytesRead);
    InternetCloseHandle(hHttpFile);
    Request[dwBytesRead] = 0;
    InternetCloseHandle(hSession);

    if (!readSucceeded)
    {
        Request[0] = '\0';
        return 0;
    }
    return 1;
}

int BASE_GetWeekNumber()
{
	time_t now;
	time(&now);

	unsigned int week = 86400;
	return (int)(now / week - 3);
}

void EnableSysKey()
{
}

bool CheckOS()
{
	return false;
}

void DisableSysKey()
{
}
