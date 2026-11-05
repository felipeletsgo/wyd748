#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "ItemEffect.h"
#include "NativeItemVolatile.h"
#include "WYD748Assets.h"
#include "ServerListAsset.h"
#include "../platform/windows/ChannelStatusCache.h"
#include "../platform/diagnostics/PerfTrace.h"

// Initialization, data loading, and resource normalization.
/** Converts logical width to pixels using the width of g_pDevice.
 * Requires a valid device; does not use height, allocate, or change state. */
float BASE_ScreenResize(float size)
{
	// 7.59 helper kept for 2D UI geometry; UI mode 2 draws it in pixels.
	return size * RenderDevice::m_fUIScaleX;
}

int BASE_GetHttpRequest(char* httpname, char* Request, int MaxBuffer)
{
    return channel_status::StatusCache().Read(httpname, Request, MaxBuffer);
}

unsigned long long BASE_PollHttpRequests()
{
    return channel_status::StatusCache().Poll([](const channel_status::Completion& result) {
        perf_trace::RecordIO("channel-status", result.url.c_str(), result.success, result.milliseconds);
    });
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
