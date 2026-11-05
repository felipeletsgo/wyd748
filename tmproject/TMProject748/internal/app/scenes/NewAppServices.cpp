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
#include "../../application/MixHelpRecord.h"
#include <string>

void NewApp::MixHelp()
{
	memset(g_pItemMixHelp, 0, sizeof(g_pItemMixHelp));
	// The resolved path is used directly; a fixed copy could fail fast.
	const std::string helpFile = WYD748_TextPath(MixHelp_Path);

	FILE* fp = nullptr;
	fopen_s(&fp, helpFile.c_str(), "rt");

	if (fp == nullptr)
		return;

	// Indices, names and rows come from the file and are bounded by the parser.
	mix_help::Load([fp](char* line, int capacity) { return fgets(line, capacity, fp) != nullptr; },
		g_pItemMixHelp, static_cast<int>(_countof(g_pItemMixHelp)));

	fclose(fp);
}
