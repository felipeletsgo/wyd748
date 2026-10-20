#include "pch.h"
#include "LightingProfile.h"
#include "EventTranslator.h"
#include "RenderDevice.h"
#include "dsutil.h"
#include "DirShow.h"
#include "TimerManager.h"
#include "ObjectManager.h"
#include "CPSock.h"
#include "../../application/ports/PacketDispatch.h"
#include "../../application/MusicListLine.h"
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
#include "PerfTrace.h"
#include "ClientSettings.h"
#include "DeviceFrame.h"
#include "../../ui/ItemHelpText.h"
#include "../../ui/RenderControlList.h"
#include <fstream>

const char ClassName[4] = "WYD";

NewApp::NewApp()
{
	m_hWnd = 0;
	m_bActive = 0;
	m_dwScreenWidth = 800;
	m_dwScreenHeight = 600;
	m_pRenderDevice = 0;
	m_pSoundManager = 0;
	m_pBGMManager = 0;
	m_pEventTranslator = 0;
	m_pTimerManager = 0;
	m_pObjectManager = 0;
	m_pSocketManager = 0;
	g_pApp = this;
	ShellExecute(0, "open", ChangeUpdate_Path, 0, 0, 1);
	BASE_InitModuleDir();
	BASE_InitializeHitRate();
	BASE_ReadMessageBin();
	LOG_INITIALIZELOG(LogFile_Path);
	// Keep the crash filter active for the compatibility client so a malformed
	// legacy UI/control path leaves a reproducible dump instead of only a
	// generic Windows event entry.
	WYD748_InstallDiagnostics();
	sprintf(m_strWindowTitle, GameWindow_Title);
	m_Winstate = 1;
	china_bWrite = 0;
	china_Playtime = -1;
	m_binactive = 1;
#ifdef _DEBUG
	// CreateConsole(); // Debug console remains disabled.
#endif // DEBUG
}

NewApp::~NewApp()
{
	// Startup can fail before the render device is created; teardown must be
	// safe in that partial-initialization state as well as after normal play.
	if (m_bwFullScreen && m_pRenderDevice)
		m_pRenderDevice->RestoreWindowMode();

	LOG_FINALIZELOG();
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_pApp->MsgProc(hWnd, message, wParam, lParam);
}

HRESULT NewApp::Initialize(HINSTANCE hInstance, int nFull)
{
	struct stRes
	{
		unsigned int dwWidth;
		unsigned int dwHeight;
		unsigned int dwBit;
	} stResList[11];

	m_dwColorBit = 16;
	m_bwFullScreen = 0;
	g_hInstance = hInstance;
	m_dwScreenWidth = 800;
	m_dwScreenHeight = 600;
	m_nSound = 0;
	m_dwColorBit = 16;
	m_bwFullScreen = nFull;
	BASE_InitEffectString();
	stResList[0].dwWidth = 640;
	stResList[0].dwHeight = 480;
	stResList[0].dwBit = 32;
	stResList[1].dwWidth = 800;
	stResList[1].dwHeight = 600;
	stResList[1].dwBit = 32;
	stResList[2].dwWidth = 1024;
	stResList[2].dwHeight = 768;
	stResList[2].dwBit = 32;
	stResList[3].dwWidth = 1280;
	stResList[3].dwHeight = 720;
	stResList[3].dwBit = 32;
	stResList[4].dwWidth = 1280;
	stResList[4].dwHeight = 800;
	stResList[4].dwBit = 32;
	stResList[5].dwWidth = 1280;
	stResList[5].dwHeight = 960;
	stResList[5].dwBit = 32;
	stResList[6].dwWidth = 1280;
	stResList[6].dwHeight = 1024;
	stResList[6].dwBit = 32;
	stResList[7].dwWidth = 1440;
	stResList[7].dwHeight = 900;
	stResList[7].dwBit = 32;
	stResList[8].dwWidth = 1600;
	stResList[8].dwHeight = 900;
	stResList[8].dwBit = 32;
	stResList[9].dwWidth = 1600;
	stResList[9].dwHeight = 1024;
	stResList[9].dwBit = 32;
	stResList[10].dwWidth = 1600;
	stResList[10].dwHeight = 1080;
	stResList[10].dwBit = 32;

	int nResIndex = 2;
	int nBright = 50;
	int nCursor = 0;
	int nMipMap = 30;
	int nCameraRotate = 0;
	int nDXT = 0;
	int nCameraView = 1;
	// Keep the file handle function-scoped because initialization later reuses
	// it for itemhelp.dat after the configuration adapter has finished.
	FILE* fp = nullptr;

	SaveUpdatAndConfig Config{};
	const auto setDefaultConfig = [&Config]()
	{
		// These defaults remain the safe fallback for a missing or malformed
		// file; a valid 7.48 config.txt always takes precedence below.
		Config.Version = 7000;
		Config.Config[0] = 7;
		Config.Config[1] = 2;
		Config.Config[2] = 0;
		Config.Config[3] = 0;
		Config.Config[4] = -1;
		Config.Config[5] = 57;
		Config.Config[6] = 2;
		Config.Config[7] = 1;
		Config.Config[8] = 1;
		Config.Config[9] = 1;
		Config.Config[10] = 0;
		Config.Config[11] = 0;
		Config.Config[12] = 0;
		Config.Config[13] = 1;
	};
	setDefaultConfig();

	// The distributed 7.48 client owns a named text configuration. Keep support
	// for TMProject's Config.bin only as a fallback for upstream development.
	if (!WYD748_LoadClientConfig("config.txt", &Config.Version, Config.Config, _countof(Config.Config)))
	{
		fopen_s(&fp, ConfigFile_Path, "rb");
		if (fp)
		{
			if (fread(&Config, sizeof(Config), 1, fp) != 1)
				setDefaultConfig();
			fclose(fp);
		}
	}

	int nWindow = -1;
	nResIndex = Config.Config[0];

	TMSkinMesh::m_nSmooth = Config.Config[1];

	if (Config.Config[1] >= 3)
		TMSkinMesh::m_nSmooth = 2;

	g_bHideEffect = 0;
	g_bHideSkillBuffEffect = 0;
	g_bHideSkillBuffEffect2 = 0;

	m_nSound = Config.Config[2];
	m_nMusic = Config.Config[3];
	nBright = Config.Config[5];
	nCursor = Config.Config[6];

	g_nPlayDemo = Config.Config[7];
	nWindow = Config.Config[8];
	if (Config.Config[8] >= 0)
	{		 
		m_bwFullScreen = nWindow == 0;
	}

	// Native FUN_0055bc0a reads [CLASSIC] (and selects mode 1 for it or for
	// RES 1/6), but then writes mode 2 unconditionally after closing
	// config.txt (0x0055c136): the 7.48 client always runs the modern UI.
	// Honoring CLASSIC=1 here mixed classic atlases (skill shop) with modern
	// ones (learned skills, skill bar).
	g_UIVer = 2;

	nCameraRotate = Config.Config[10] > 0;
	nDXT = Config.Config[11] > 0;
	g_nKeyType = Config.Config[12];
	nCameraView = Config.Config[13];
	m_nCameraView = Config.Config[13];
	// Do not overwrite CAMERA after loading: the forced TMProject value made
	// the 7.48 UI render and hit-test with different coordinates.
	D3DDevice::m_bDxt = nDXT == 0;
	RenderDevice::m_bCameraRot = nCameraRotate;
	D3DDevice::m_nMipMap = nMipMap;

	if (nBright < 0)
		nBright = 0;
	if (nBright > 100)
		nBright = 100;
	RenderDevice::m_nBright = nBright;
	if (nCursor == 1)
		nCursor = 0;
	else
		nCursor = 2;

	SCursor::m_nCursorType = nCursor;
	if (nCursor == 2)
	{
		SCursor::m_hCursor1 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
		SCursor::m_hCursor2 = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR2));
		if (SCursor::m_hCursor1)
			SetCursor(SCursor::m_hCursor1);
	}

	// RES is a one-based 7.48 index. Clamp malformed external input before it
	// can index outside the fixed resolution table.
	if (nResIndex < 1 || nResIndex > static_cast<int>(_countof(stResList)))
		nResIndex = 2;
	m_dwScreenWidth = stResList[nResIndex - 1].dwWidth;
	m_dwScreenHeight = stResList[nResIndex - 1].dwHeight;
	m_dwColorBit = stResList[nResIndex - 1].dwBit;
	if (!CheckResolution(m_dwScreenWidth, m_dwScreenHeight, m_dwColorBit))
	{
		m_dwScreenWidth = stResList[1].dwWidth;
		m_dwScreenHeight = stResList[1].dwHeight;
		m_dwColorBit = stResList[1].dwBit;
	}

	// FUN_0042ced1 (UI mode 2) lays the UI out at 800x600 pixel sizes with a
	// 13-pixel font. ClientSettings UIScale enlarges that layout uniformly for
	// larger resolutions; the font follows so text keeps its proportions.
	const float uiScale = ResolveUIScale(g_ClientSettings.uiScale, m_dwScreenWidth, m_dwScreenHeight);
	RenderDevice::m_fUIScaleX = uiScale;
	RenderDevice::m_fUIScaleY = uiScale;
	RenderDevice::m_nFontSize = ScaledFontSize(uiScale);
	RenderDevice::m_nFontTextureSize = FontTextureSizeFor(RenderDevice::m_nFontSize);
	RenderDevice::m_nFontTextureSizeY = RenderDevice::m_nFontTextureSize / 8;

	if (m_hWnd == nullptr)
	{
		WNDCLASS wndClass;
		wndClass.style = 8;
		wndClass.lpfnWndProc = WndProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInstance;
		wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TMPROJECT));
		wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(4);
		wndClass.lpszMenuName = 0;
		wndClass.lpszClassName = ClassName;

		RegisterClass(&wndClass);

		DWORD dwCreationFlags = 0;

		if (m_bwFullScreen)
			dwCreationFlags = 0x80000000;
		else
			dwCreationFlags = 0x0CA0000; // check what flag is this

		m_dwWindowStyle = dwCreationFlags | 0x10000000;

		RECT rc;
		SetRect(&rc, 0, 0, m_dwScreenWidth, m_dwScreenHeight);
		AdjustWindowRect(&rc, m_dwWindowStyle, 0);

		m_hWnd = CreateWindowEx(
			0,
			ClassName,
			m_strWindowTitle,
			m_dwWindowStyle,
			0,
			0,
			rc.right - rc.left,
			rc.bottom - rc.top,
			0,
			0,
			hInstance,
			0);
	}

	hWndMain = m_hWnd;

	if (!BASE_InitializeBaseDef())
	{
		MessageBoxA(m_hWnd, "Initialize Data Failed", "Error", 0);
		LOG_WRITELOGSTRING("Initialize Data Failed\r\n");
		LOG_FINALIZELOG();
		return 0;
	}

	if (!BASE_InitializeAttribute() || !BASE_InitializeHeightMap())
	{
		MessageBoxA(m_hWnd, "The client terrain maps are missing or invalid.", "Error", MB_OK);
		LOG_WRITELOGSTRING("Client terrain map initialization failed\r\n");
		LOG_FINALIZELOG();
		return 0;
	}

	ReadItemName();
	if (!ReadUIString())
	{
		MessageBoxA(m_hWnd, "The client UI string table is missing or invalid.", "Error", MB_OK);
		LOG_WRITELOG("Client UI string initialization failed [%s]\r\n", UIString_Path);
		LOG_FINALIZELOG();
		return 0;
	}
	BASE_ReadItemPrice();

	memset(g_pItemHelp, 0, sizeof(g_pItemHelp));
	char szItemHelpFile[128];
	sprintf(szItemHelpFile, ItemHelp_Path);

	remove(ChangeUpdate_Path);

	MixHelp();

	std::ifstream itemHelp(szItemHelpFile);
	if (!itemHelp)
	{
		MessageBoxA(m_hWnd, "Can't open itemhelp.dat", "Error", 0);
		return 0;
	}

	item_help_748::Load(itemHelp, g_pItemHelp, _countof(g_pItemHelp));
	item_help_748::LocalizeRepliction(g_pItemHelp, _countof(g_pItemHelp));
	itemHelp.close();
	// Faithful English localization of the native 7.48 skill-help corpus.
	// Keep this separate from historical item records and their ANSI encoding.
	std::ifstream skillHelp("skillhelp.dat");
	if (!skillHelp || item_help_748::Load(skillHelp, g_pItemHelp, _countof(g_pItemHelp)) != 102)
	{
		MessageBoxA(m_hWnd, "Can't load the complete native skillhelp.dat", "Error", 0);
		return 0;
	}

	// The 7.48 client has no intro video (no .avi path in WYD.exe), so the
	// device is created right after the data files.
	return InitDevice();
 }

HRESULT NewApp::InitDevice()
{
	m_pRenderDevice = new RenderDevice(m_dwScreenWidth, m_dwScreenHeight, m_dwColorBit, m_bwFullScreen);

	if (m_bwFullScreen)
		m_pRenderDevice->SetWindowedFullScreen();

	if (!m_pRenderDevice->Initialize(m_hWnd))
	{
		MessageBoxA(m_hWnd, "Initialize Render Failed.", "Error", 0);
		LOG_WRITELOGSTRING("Initialize Render Failed\r\n");
		// todo: release the m_prenderdevice?
		return 0;
	}

	if (m_nSound > 0)
	{
		InitMusicList();

		m_pSoundManager = new CSoundManager();

		if (FAILED(m_pSoundManager->Initialize(m_hWnd, 2, 2, 0x5622, 0x10)))
		{
			LOG_WRITELOG("Initialize Sound Failed\r\n");
			SAFE_DELETE(m_pSoundManager);
		}

		int nVolume = 25 * m_nSound - 2500;
		if (nVolume > 0)
			nVolume = 0;
		if (nVolume < -2500)
			nVolume = -2500;

		if (m_pSoundManager)
		{
			m_pSoundManager->SetSoundVolume(nVolume);
			m_pSoundManager->LoadSoundData();
		}
	}

	if (m_nMusic > 0)
	{
		m_pBGMManager = new DS_SOUND_MANAGER(1, 30 * m_nMusic - 3000);
		g_pApp->m_pBGMManager->SetVolume(0, g_pApp->m_pBGMManager->m_lBGMVolume);
	}

	m_pEventTranslator = new EventTranslator();
	if (!m_pEventTranslator->Initialize(m_hWnd))
	{
		LOG_WRITELOG("Initialize Interface Failed");
		return 0;
	}

	if (!m_pEventTranslator->InitializeIME())
	{
		LOG_WRITELOG("Initialize IME Failed");
		return 0;
	}

	m_pTimerManager = new TimerManager();

	m_pObjectManager = new ObjectManager();
	if (m_pObjectManager != nullptr)
	{
		m_pObjectManager->InvalidateDeviceObjects();
		m_pObjectManager->RestoreDeviceObjects();
	}

	if (m_pTimerManager != nullptr)
		m_pTimerManager->StartTimer();

	m_pSocketManager = new CPSock();

	if (!m_pSocketManager->WSAInitialize())
	{
		LOG_WRITELOG("Initialize Socket Failed");
		return 0;
	}
	g_pSocketManager = m_pSocketManager;
	g_pTimerManager->SetServerTime(0);

	m_bActive = 1;

	int nMipMap;
	if (m_pRenderDevice->m_pd3dDevice->GetAvailableTextureMem() > 64)
	{
		nMipMap = D3DDevice::m_nMipMap;
	}
	else if (m_pRenderDevice->m_pd3dDevice->GetAvailableTextureMem() > 56 && D3DDevice::m_nMipMap > 20)
	{
		nMipMap = 20;
	}
	else if (m_pRenderDevice->m_pd3dDevice->GetAvailableTextureMem() > 48 && D3DDevice::m_nMipMap > 10)
	{
		nMipMap = 10;
	}
	else
	{
		nMipMap = 0;
	}

	D3DDevice::m_nMipMap = nMipMap;

	return 1;
}

void NewApp::InitMusicList()
{
	FILE* fp = nullptr;
	fopen_s(&fp, MusicConfig_Path, "rt");

	if (fp == nullptr)
		return;

	// A line without a path keeps the default entry from DirShow.cpp.
	char szLine[256];
	for (int i = 0; i < 15 && fgets(szLine, sizeof(szLine), fp); ++i)
	{
		char szPath[256];
		if (music_list::ParseLine(szLine, szPath, sizeof(szPath)))
			strcpy_s(DS_SOUND_MANAGER::m_szMusicPath[i], szPath);
	}

	fclose(fp);

	sprintf(DS_SOUND_MANAGER::m_szMusicPathOrigin[14], DSSound_1_Path);
	sprintf(DS_SOUND_MANAGER::m_szMusicPathOrigin[13], DSSound_2_Path);
}

HRESULT NewApp::Finalize()
{
	// The mesh manager destructor releases the same buffers. Freeing them here
	// without clearing the pointers made that second release a double free
	// (crash on exit from the field); the shared release clears them.
	MeshManager::ReleaseBoneAnimations();
	
	SAFE_DELETE(g_pObjectManager);
	SAFE_DELETE(m_pTimerManager);
	SAFE_DELETE(m_pRenderDevice);
	SAFE_DELETE(m_pSoundManager);
	SAFE_DELETE(m_pSocketManager);
	SAFE_DELETE(m_pBGMManager);
	SAFE_DELETE(m_pEventTranslator);

	g_pSocketManager = nullptr;
	DeleteObject(m_hBMBtnBG);

	return 1;
}

namespace {
double FrameClockMs()
{
	static LARGE_INTEGER frequency{};
	if (!frequency.QuadPart)
		QueryPerformanceFrequency(&frequency);
	LARGE_INTEGER counter{};
	QueryPerformanceCounter(&counter);
	return static_cast<double>(counter.QuadPart) * 1000.0 / static_cast<double>(frequency.QuadPart);
}
}

bool NewApp::IsIdleWindow() const
{
	// Unfocused and minimized windows share the light profile. The game is
	// played with several visible clients, so focus decides the profile.
	if (!m_hWnd || IsIconic(m_hWnd))
		return true;
	const HWND foreground = GetForegroundWindow();
	return !foreground || GetAncestor(foreground, GA_ROOT) != m_hWnd;
}

DWORD NewApp::Run()
{
	HACCEL hAccel = LoadAccelerators(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDC_TMPROJECT));

	// A 1 ms timer period keeps the frame caps accurate.
	const bool usesFrameCap = g_ClientSettings.idleFps > 0 || g_ClientSettings.focusedFps > 0;
	if (usesFrameCap)
		timeBeginPeriod(1);
	double lastFrameStartMs = -1.0;

	MSG msg = {};
	PeekMessage(&msg, 0, 0, 0, 0);
	while (msg.message != WM_QUIT)
	{
		int bGotMsg = 0;
		if (!m_bActive)
			bGotMsg = GetMessage(&msg, 0, 0, 0);
		else
			bGotMsg = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);

		if (bGotMsg < 0)
		{
			LOG_WRITELOG("GetMessage Failed\r\n");
			break;
		}

		if (bGotMsg > 0)
		{
			// Gameplay accelerators belong to KEYTYPE=1. Native edit windows
			// still need Ctrl+C/V/X in either mode.
			const bool useAccelerators = g_nKeyType == 1 || GetFocus() != m_hWnd;
			if (!useAccelerators || !hAccel || !m_hWnd || TranslateAccelerator(m_hWnd, hAccel, &msg) == 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (m_bActive == 1)
			{
				const bool idle = IsIdleWindow();
				const int frameCap = idle ? g_ClientSettings.idleFps : g_ClientSettings.focusedFps;
				const double frameStartMs = FrameClockMs();
				const double waitMs = FrameWaitMs(frameStartMs, lastFrameStartMs, frameCap);
				if (waitMs >= 1.0)
				{
					// Input, network and focus messages end the wait at once.
					MsgWaitForMultipleObjects(0, nullptr, FALSE, static_cast<DWORD>(waitMs), QS_ALLINPUT);
					continue;
				}
				lastFrameStartMs = frameStartMs;
				g_bIdleRenderProfile = idle;

				unsigned int dwServerTime = m_pTimerManager->GetServerTime();

				m_pEventTranslator->ReadInputEventData();
				m_pEventTranslator->CameraEventData();

				{
					perf_trace::ScopedPhase phase(perf_trace::Phase::FrameMove);
					m_pObjectManager->FrameMove(dwServerTime);
				}

				if (g_dwStartPlayTime != 0 && g_dwPausedTime == 0)
				{
					if (g_nDumpPacketSize > g_nCurrentPos)
					{
						unsigned int dwTermTime = 0;
						memcpy((char*)&dwTermTime, &g_pDumpPacket[g_nCurrentPos], 4);
						if (dwServerTime > g_dwStartPlayTime + dwTermTime)
						{
							g_nCurrentPos += 4;
							MSG_STANDARD* pStd = (MSG_STANDARD*)&g_pDumpPacket[g_nCurrentPos];
							m_pObjectManager->OnPacketEvent(
								*(unsigned short*)&g_pDumpPacket[g_nCurrentPos + 4],
								&g_pDumpPacket[g_nCurrentPos]);
							g_nCurrentPos += pStd->Size;
						}
					}
					else
					{
						g_dwStartPlayTime = 0;
						g_pCurrentScene->m_pMessagePanel->SetMessage("End Replay", 4000);
						g_pCurrentScene->m_pMessagePanel->SetVisible(1, 1);
						g_pCurrentScene->m_pMyHuman->m_cHide = 0;
					}
				}
				{
					perf_trace::ScopedPhase phase(perf_trace::Phase::Render);
					RenderScene();
				}
				if (m_pRenderDevice->m_pMeshManager == nullptr)
				{
					if (!m_pRenderDevice->InitMeshManager())
					{
						MessageBoxA(m_hWnd, "Game data crashed. Install game again.", "Error", MB_SYSTEMMODAL);
						PostMessageA(m_hWnd, WM_CLOSE, 0, 0);
					}

					m_pObjectManager->SetCurrentState(ObjectManager::TM_GAME_STATE::TM_SELECTSERVER_STATE);
				}
				if (m_pObjectManager->m_bCleanUp)
				{
					perf_trace::ScopedPhase phase(perf_trace::Phase::CleanUp);
					m_pObjectManager->CleanUp();
				}
				if (perf_trace::EndFrame())
					WritePerfSample();
				if (TMScene* layoutScene = m_pObjectManager->GetCurrentScene())
					WYD748_LayoutProbe(layoutScene->m_pControlContainer, m_dwScreenWidth,
						m_dwScreenHeight, RenderDevice::m_fUIScaleX);
			}
		}		
	}

	if (hAccel)
		DestroyAcceleratorTable(hAccel);
	if (usesFrameCap)
		timeEndPeriod(1);

	return msg.wParam;
}

namespace {
void CountPerfTree(TreeNode* root, std::uint32_t& live, std::uint32_t& deleted)
{
	TreeNode* node = root;
	while (node)
	{
		if (node->m_cDeleted)
			++deleted;
		else
			++live;
		if (node->m_pDown)
		{
			node = node->m_pDown;
			continue;
		}
		while (node && node != root && !node->m_pNextLink)
			node = node->m_pTop;
		if (!node || node == root)
			break;
		node = node->m_pNextLink;
	}
}

std::uint32_t CountPerfLive(TreeNode* root)
{
	std::uint32_t live = 0;
	std::uint32_t deleted = 0;
	if (root)
		CountPerfTree(root, live, deleted);
	return live;
}
}

void NewApp::WritePerfSample()
{
	perf_trace::Snapshot snapshot;
	if (TMScene* scene = g_pCurrentScene)
	{
		snapshot.sceneType = static_cast<int>(scene->m_eSceneType);
		CountPerfTree(scene, snapshot.nodes, snapshot.deletedNodes);
		snapshot.effectNodes = CountPerfLive(scene->m_pEffectContainer);
		snapshot.humanNodes = CountPerfLive(scene->m_pHumanContainer);
		snapshot.itemNodes = CountPerfLive(scene->m_pItemContainer);
		snapshot.groundObjectNodes = CountPerfLive(scene->m_pGroundObjectContainer);
	}
	// The runtime may move the device to another vtable; patch it as well.
	perf_trace::InstallDeviceHooks(m_pRenderDevice ? m_pRenderDevice->m_pd3dDevice : nullptr);
	perf_trace::FillProcess(snapshot, m_hWnd);
	perf_trace::WriteSample(snapshot);
}

HRESULT NewApp::RenderScene()
{
	// Discard this frame's non-owning UI references on every return path,
	// including device loss and frames where RenderControl is skipped.
	render_control_list::FrameCleanup clearControls([this] {
		TMScene* scene = m_pObjectManager ? m_pObjectManager->GetCurrentScene() : nullptr;
		if (scene && scene->m_pControlContainer)
			for (auto& list : scene->m_pControlContainer->m_pDrawControl)
				render_control_list::Clear(list);
	});
	if (!m_pRenderDevice || !m_pRenderDevice->m_pd3dDevice ||
		!m_pObjectManager || !m_pObjectManager->m_pCamera)
		return S_FALSE;
	device_frame::Scene<RenderDevice> frame(*m_pRenderDevice);

	m_pRenderDevice->SetViewPort(0, 0, m_dwScreenWidth, m_dwScreenHeight);

	if (!frame.Begin(1)) return S_FALSE;

	TMCamera* pCamera = m_pObjectManager->m_pCamera;
	TMVector3 vecLookAt = pCamera->GetCameraLookatPos();

	if (!m_pRenderDevice->SetViewVector(pCamera->m_cameraPos, vecLookAt) ||
		!m_pRenderDevice->SetRenderStateBlock(1)) return S_FALSE;
	m_pObjectManager->RenderObject();

	TMScene* pCurrentScene = m_pObjectManager->GetCurrentScene();
	if (pCurrentScene && pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELCHAR && g_UIVer != 2)
	{
		SControl* pCreateWindow = pCurrentScene->m_pControlContainer
			? pCurrentScene->m_pControlContainer->FindControl(1542u)
			: nullptr;
		constexpr float kPreviewWidth = 200.0f;
		constexpr float kPreviewHeight = 150.0f;
		constexpr float kPreviewBottomMargin = 5.0f;

		if (pCreateWindow && pCreateWindow->IsVisible() == 1 &&
			pCreateWindow->m_nPosY + kPreviewHeight * RenderDevice::m_fHeightRatio +
				kPreviewBottomMargin < static_cast<float>(m_dwScreenHeight))
		{
			if (!frame.End(0)) return S_FALSE;
			m_pRenderDevice->SetViewPort(
				static_cast<int>(pCreateWindow->m_nPosX) + 2,
				static_cast<int>(pCreateWindow->m_nPosY) + 2,
				static_cast<int>(kPreviewWidth * RenderDevice::m_fWidthRatio),
				static_cast<int>(kPreviewHeight * RenderDevice::m_fHeightRatio));
			m_pObjectManager->RenderTargetObject(
				static_cast<TMSelectCharScene*>(pCurrentScene)->m_fFocusHeight);
			if (!frame.Begin(0)) return S_FALSE;
		}
	}

	// The native 7.48 client renders controls in a fresh full-screen scene after
	// the optional character-preview viewport.
	if (!frame.End(0)) return S_FALSE;
	m_pRenderDevice->SetViewPort(0, 0, m_dwScreenWidth, m_dwScreenHeight);
	if (!frame.Begin(0)) return S_FALSE;
	if (!m_pRenderDevice->SetRenderStateBlock(3)) return S_FALSE;
	m_pObjectManager->RenderControl();
	if (!frame.End(1)) return S_FALSE;

	if (g_pTextureManager)
		g_pTextureManager->ReleaseNotUsingTexture();
	if (g_pMeshManager)
		g_pMeshManager->ReleaseNotUsingMesh();

	return 1;
}

void NewApp::SetObjectManager(ObjectManager* pManager)
{
	m_pObjectManager = pManager;
	g_pObjectManager = m_pObjectManager;
}

EventTranslator* NewApp::GetEventTranslator()
{
	return m_pEventTranslator;
}

HWND NewApp::GetSafeHwnd()
{
	return m_hWnd;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	const int monitorResult = WYD748_RunCrashMonitor();
	if (monitorResult >= 0) return monitorResult;
	// Install before any bootstrap work and install again after device/UI setup:
	// third-party initialization may replace the process-wide top-level filter.
	WYD748_InstallDiagnostics();
	// The native 7.48 single-window gate (FindWindow by title) is removed:
	// several windows may run at once, and the server enforces the limit of
	// authenticated clients per IP (max_authenticated_clients_per_ip).

	char* szArgument = GetCommandLine();
	if (CheckOS())
		sprintf(g_szOS, "98");
	else
		sprintf(g_szOS, "NT");

	g_ClientSettings = LoadClientSettings(ClientSettings_Path);
	g_LightingProfile = LoadLightingProfile(LightingProfile_Path);
	NewApp* newApp = new NewApp();

	int nFull = 1;
	char szDesc[256]{};
	GetKeyboardLayoutName(szDesc);
	
	// wWinMain receives UTF-16 text; comparing it as an ANSI pointer made the
	// optional windowed switch undefined and could leave the client in the
	// wrong display mode before the 7.48 config was applied.
	if (lpCmdLine != nullptr && wcscmp(lpCmdLine, L"/w") == 0)
		nFull = 0;

	// Retired launcher command-line gate; unused by the active client.
	//if (j___stricmp(lpCmdLine, "China No.1 It is TM Launcher"))
		//return 0;

	if (!ReadNameFiltraDataBase())
	{
		MessageBoxA(0, "Need System File", "WYD", 0);
		return 0;
	}
	else if (!ReadChatFiltraDataBase())
	{
		MessageBoxA(0, "Need System File", "WYD", 0);
		return 0;
	}
	else if (newApp->Initialize(hInstance, nFull))
	{
		WYD748_InstallDiagnostics();
		DisableSysKey();

		int nResult = newApp->Run();
		if (newApp)
			delete newApp;

		return nResult;
	}

	return 0;
}
