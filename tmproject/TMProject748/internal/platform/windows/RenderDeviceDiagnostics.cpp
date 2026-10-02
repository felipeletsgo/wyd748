#include "pch.h"
// RenderDevice split by responsibility; device lifecycle stays in RenderDevice.cpp.
#include "RenderDevice.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMSky.h"
#include "TMFieldScene.h"
#include "TMCamera.h"
#include "TMMesh.h"

void RenderDevice::LogRenderState()
{
	FILE* fp = nullptr;
	fopen_s(&fp, RenderStateLog_Path, "wt");

	if (fp == nullptr)
		return;

	for (int i = 0; i < 256; i++)
	{
		fprintf(fp, "Index:%d Val=%d\n", i, m_dwRenderStateList[i]);
	}

	fclose(fp);
}

void RenderDevice::LogSamplerState()
{
	FILE* fp = nullptr;
	fopen_s(&fp, SamplerStateLog_Path, "wt");

	if (fp == nullptr)
		return;

	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 14; ++j)
		{
			fprintf(fp, "Stage:%d State:%d  Val=%d\n", i, j, m_dwSamplerStateList[i][j]);
		}
	}

	fclose(fp);
}

void RenderDevice::LogTextureStageState()
{
	FILE* fp = nullptr;
	fopen_s(&fp, TextureStateLog_Path, "wt");

	if (fp == nullptr)
		return;

	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 29; ++j)
		{
			fprintf(fp, "Stage:%d State:%d  Val=%d\n", i, j, m_dwTextureStageStateList[i][j]);
		}
	}

	fclose(fp);
}
