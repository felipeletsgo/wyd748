#include "pch.h"
// NewApp split by responsibility; application lifecycle stays in NewApp.cpp.
#include "DirShow.h"
#include "ObjectManager.h"
#include "../../application/ports/PacketDispatch.h"
#include "NewApp.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMCamera.h"
#include <WinInet.h>
#include "TMSkinMesh.h"
#include "resource.h"
#include "TMFieldScene.h"
#include "TMSelectCharScene.h"
#include "SControlContainer.h"
#include "WYD748Assets.h"
#include "ClientDiagnostics.h"

void NewApp::MixHelp()
{
	memset(g_pItemMixHelp, 0, sizeof(g_pItemMixHelp));
	char szItemHelpFile[128];
	sprintf(szItemHelpFile, MixHelp_Path);

	FILE* fp = nullptr;
	fopen_s(&fp, szItemHelpFile, "rt");

	if (fp == nullptr)
		return;

	int NumHelp = 10000;
	int ItemIndex = 0;
	int Icon = 0;
	int Color = 0;
	char Name[256]{};

	while (NumHelp < 11500)
	{
		char szTemp[256];
		for (int j = 0; j < 10 && fgets(szTemp, 256, fp); ++j)
		{
			if (j == 0)
			{
				sscanf(szTemp, "%s %d %d", Name, &ItemIndex, &Icon);
				continue;
			}

			char szCol[7];
			memset(szCol, 0, 7);
			strncpy(szCol, szTemp, 6u);
			sscanf(szCol, "%x", &Color);
			char* szRet = strstr(szTemp, "\n");
			if (szRet)
				*szRet = 0;

			if (szTemp[8] == '\t' || szTemp[8] == ' ')
				sprintf(g_pItemMixHelp[ItemIndex].Help[j - 1], "%s", &szTemp[9]);

			g_pItemMixHelp[ItemIndex].Color[j - 1] = Color;
			g_pItemMixHelp[ItemIndex].Icon = Icon;
			strcpy(g_pItemMixHelp[ItemIndex].Name, Name);
		}

		++NumHelp;
	}

	fclose(fp);
}
