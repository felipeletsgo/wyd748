#include "pch.h"
// NewApp split by responsibility; application lifecycle stays in NewApp.cpp.
#include "EventTranslator.h"
#include "RenderDevice.h"
#include "DirShow.h"
#include "TMVideoWnd.h"
#include "JBlur.h"
#include "ObjectManager.h"
#include "CPSock.h"
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

void NewApp::OnCreate(HWND hWnd, DWORD wParam, int lParam)
{
	m_hBMBtnBG = LoadBitmap(g_hInstance, (LPCSTR)0xAB);
}

HRESULT NewApp::MsgProc(HWND hWnd, DWORD uMsg, DWORD wParam, int lParam)
{
	switch (uMsg)
	{
	case WM_SETCURSOR:
			return 0;
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	{
		if (SCursor::m_nCursorType == 0)
		{
			SetCursor(0);
		}
		else if (SCursor::m_nCursorType == 2 && g_pCursor)
		{
			if (g_pCursor->m_GCPanel.nTextureIndex == 0)
				SetCursor(SCursor::m_hCursor1);
			else if(g_pCursor->m_GCPanel.nTextureIndex == 1)
				SetCursor(SCursor::m_hCursor2);
		}

		if (m_pEventTranslator != nullptr)
			m_pEventTranslator->OnMouseEvent(uMsg, wParam, LOWORD(lParam), HIWORD(lParam));
	}
	break;
	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
		HDC hdcMem = CreateCompatibleDC(lpdis->hDC);

		HBITMAP hOldBitMap = nullptr;
		hOldBitMap = (HBITMAP)SelectObject(hdcMem, m_hBMBtnBG);

		StretchBlt(
			lpdis->hDC,
			lpdis->rcItem.left,
			lpdis->rcItem.top,
			lpdis->rcItem.right - lpdis->rcItem.left,
			lpdis->rcItem.bottom - lpdis->rcItem.top,
			hdcMem,
			0,
			0,
			86,
			21,
			0x0CC0020);

		SelectObject(hdcMem, hOldBitMap);
		DeleteDC(hdcMem);
	}
	break;
	case WM_INPUTLANGCHANGE:
	{
		OnInputLanguageChange(hWnd);
	}
	break;
	case WM_KEYDOWN:
	{
		const ExtractedFlow extractedFlow = OnKeyDownMessage(wParam);
		if (extractedFlow == ExtractedFlow::Break)
			break;
	}
	break;
	case WM_KEYUP:
	{
		if (m_pEventTranslator == nullptr)
			break;

		if (wParam == VK_SNAPSHOT)
			m_pRenderDevice->CaptureScreen();

		WORD sVal = GetKeyState(VK_CONTROL);
		m_pEventTranslator->m_bCtrl = (int)sVal >> 8 > 0;
		WORD sVal2 = GetKeyState(VK_SHIFT);
		m_pEventTranslator->m_bShift = (int)sVal2 >> 8 > 0;

		m_pEventTranslator->OnKeyUp(wParam);
	}
	break;
	case WM_CHAR:
	{
		if (m_pEventTranslator != nullptr && !m_pEventTranslator->m_bCtrl)
			m_pEventTranslator->OnChar(static_cast<char>(wParam), static_cast<int>(lParam));
	}
	break;
	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_MENU)
		{
			if (!g_nKeyType && m_pEventTranslator != nullptr)
				m_pEventTranslator->m_bAlt = 1;
		}
		else if (wParam == VK_F10 && m_pEventTranslator != nullptr)
		{
			m_pEventTranslator->OnKeyDown(VK_F10);
		}
	}
	break;
	case WM_SYSKEYUP:
	{
		if (wParam == VK_MENU && !g_nKeyType && m_pEventTranslator != nullptr)
		{
			m_pEventTranslator->m_bAlt = 0;
		}
		else if (wParam == VK_F10 && m_pEventTranslator != nullptr)
		{
			m_pEventTranslator->OnKeyUp(VK_F10);
		}
	}
	break;
	case WM_IME_ENDCOMPOSITION:
	{
		if (m_pEventTranslator == nullptr)
			break;

		m_pEventTranslator->SetVisibleCandidateList(lParam, 0);
		if (g_pCurrentScene != nullptr)
		{
			if (g_pCurrentScene->m_pTextCompose != nullptr)
				g_pCurrentScene->m_pTextCompose->SetVisible(0);
			if (g_pCurrentScene->m_pTextComposeB != nullptr)
				g_pCurrentScene->m_pTextComposeB->SetVisible(0);
		}
	}
	break;
	case WM_IME_COMPOSITION:
	{
		if (m_pEventTranslator == nullptr)
			break;

		m_pEventTranslator->OnIME(static_cast<char>(wParam), static_cast<int>(lParam));
	}
	break;
	case WM_COMMAND:
	{
		if (wParam == 999)
		{
			// SwitchWebBrowserState
			break;
		}

		int msg = wParam;
		HWND hWndFocus = GetFocus();

		if (hWndFocus != g_pApp->m_hWnd)
		{
			switch (msg)
			{
			case 40086:
				PostMessageA(hWndFocus, 0x301, 0, 0);
				return 1;
			case 40100:
				PostMessageA(hWndFocus, 0x302, 0, 0);
				return 1;
			case 40102:
				PostMessageA(hWndFocus, 0x300, 0, 0);
				return 1;
			}
		}

		if (g_pCurrentScene != nullptr)
			g_pCurrentScene->OnAccel(msg);
	}
	break;
	case WM_SYSCOMMAND:
	{
		switch (wParam)
		{
		case 0xF100u:
			SendMessageA(hWnd, 0x100, 0xA4, 0);
			return 0;
		case 0xF020u:
			g_pApp->m_Winstate = 0;
			break;
		case 0xF030u:
			g_pApp->m_Winstate = 1;
			break;
		}
	}
	break;
	case WM_IME_SETCONTEXT:
	{
		return DefWindowProc(hWnd, uMsg, lParam == -1, -2147483634);
	}
	break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	{
		if (m_pAviPlayer != nullptr && m_pAviPlayer->m_psCurrent == PLAYSTATE::Running)
		{
			m_pAviPlayer->CloseClip();
			SAFE_DELETE(m_pAviPlayer);
			InitDevice();
		}

		// TMProject previously forwarded only button-up messages. The 7.48 UI
		// requires the matching down event to arm SButton/SListBox before release.
		if (m_pEventTranslator != nullptr)
			m_pEventTranslator->OnMouseEvent(uMsg, wParam, LOWORD(lParam), HIWORD(lParam));
	}
	break;
	case WM_USER + 13:
	{
		if (m_pAviPlayer == nullptr)
			break;

		if (m_pAviPlayer->HandleGraphEvent() == 1)
		{
			SAFE_DELETE(m_pAviPlayer);
			InitDevice();
		}
		if (!m_bwFullScreen && m_pAviPlayer != nullptr)
			m_pAviPlayer->MoveVideoWindow();
	}
	break;
	case WM_IME_NOTIFY:
	{
		if (m_pEventTranslator == nullptr)
			break;

		switch (wParam)
		{
		case IMN_OPENCANDIDATE:
		case IMN_CHANGECANDIDATE:
			m_pEventTranslator->OnIME2();
			m_pEventTranslator->SetVisibleCandidateList(static_cast<int>(lParam), 1);
			break;
		case IMN_CLOSECANDIDATE:
			m_pEventTranslator->SetVisibleCandidateList(static_cast<int>(lParam), 0);
			break;
		default:
			break;
		}
	}
	break;
	case WM_USER + 100:
	{
		const ExtractedFlow extractedFlow = OnNetworkMessage(wParam, lParam);
		if (extractedFlow == ExtractedFlow::Break)
			break;
	}
	break;
	case WM_USER + 101:
	{
		if (m_pBGMManager != nullptr)
			m_pBGMManager->OnEvent();
	}
	break;
	case WM_CREATE:
	{
		OnCreate(hWnd, wParam, lParam);
	}
	break;
	case WM_DESTROY:
	case 4:
		break;
	case WM_MOVE:
	case WM_SIZE:
	{
		if (m_pAviPlayer != nullptr)
			m_pAviPlayer->MoveVideoWindow();
	}
	break;
	case WM_ACTIVATE:
	{
		if (g_pCurrentScene != nullptr && g_pCurrentScene->m_pTextIMEDesc != nullptr)
		{
			if (strcmp(g_pCurrentScene->m_pTextIMEDesc->GetText(), "Î"))
				SendMessageA(hWnd, 0x281, 0, -1073741809);
			else
				SendMessageA(hWnd, 0x281, 0, -1);
		}
		// WYD.exe 7.48 FUN_0055dab8 has only two activation states: active
		// restores BGM and inactive mutes it.  The imported fullscreen branches
		// were unreachable duplicates and skipped the native focus transition.
		if (wParam != 0)
		{
			if (m_pEventTranslator != nullptr)
				m_pEventTranslator->m_bAlt = 0;
			if (m_pBGMManager != nullptr)
				m_pBGMManager->SetVolume(0, m_pBGMManager->m_lBGMVolume);

			g_pApp->m_binactive = 1;
		}
		else
		{
			if (m_pEventTranslator)
				m_pEventTranslator->m_bAlt = 0;
			if (m_pBGMManager)
				m_pBGMManager->SetVolume(0, -10000);
			g_pApp->m_binactive = 0;
		}
	}
	break;
	case WM_CLOSE:
	{
		return OnCloseMessage(hWnd);
	}
	break;
	default:
		break;
	}

	if (m_pAviPlayer != nullptr)
	{
		if (m_pAviPlayer->m_pVW != nullptr)
			m_pAviPlayer->m_pVW->NotifyOwnerMessage((OAHWND)hWnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Extracted from NewApp::MsgProc; behavior is unchanged.
void NewApp::OnInputLanguageChange(HWND& hWnd)
{
	char szDesc[256]{};
	GetKeyboardLayoutName(szDesc);
	TMScene* pCurrentScene = g_pCurrentScene;

	if (!strncmp(szDesc, "0000", 4))
	{
		if (pCurrentScene != nullptr && pCurrentScene->m_pAlphaNative != nullptr)
			pCurrentScene->m_pAlphaNative->SetText((char*)"EN", 0);
		if (pCurrentScene != nullptr && pCurrentScene->m_pTextIMEDesc != nullptr)
			pCurrentScene->m_pTextIMEDesc->SetVisible(0);
	}
	else if (m_pEventTranslator != nullptr && pCurrentScene != nullptr &&
		pCurrentScene->m_pControlContainer != nullptr)
	{
		if (pCurrentScene->m_pControlContainer->m_pFocusControl != nullptr &&
			pCurrentScene->m_pControlContainer->m_pFocusControl->m_eCtrlType == CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT)
		{
			m_pEventTranslator->SetIMENative();
			int bNative = m_pEventTranslator->IsNative();

			if (bNative != 0)
			{
				if (pCurrentScene->m_pAlphaNative != nullptr)
					pCurrentScene->m_pAlphaNative->SetText((char*)"Ch", 0);

				if (pCurrentScene->m_pTextIMEDesc != nullptr)
				{
					HKL hkl = GetKeyboardLayout(0);
					char dst[256]{};
					ImmGetDescription(hkl, dst, 256);
					pCurrentScene->m_pTextIMEDesc->SetText(dst, 0);
					pCurrentScene->m_pTextIMEDesc->SetSize((float)(strlen(dst) * 8), 16.0f);
					pCurrentScene->m_pTextIMEDesc->SetVisible(1);
				}
			}
			else
			{
				if (pCurrentScene->m_pAlphaNative != nullptr)
					pCurrentScene->m_pAlphaNative->SetText((char*)"EN", 0);
				if (pCurrentScene->m_pTextIMEDesc != nullptr)
					pCurrentScene->m_pTextIMEDesc->SetVisible(0);
			}

			if (pCurrentScene->m_pTextIMEDesc != nullptr)
			{
				if (!strcmp(pCurrentScene->m_pTextIMEDesc->GetText(), "Î"))
					SendMessage(hWnd, 0x281, 0, -1073741809);
				else
					SendMessage(hWnd, 0x281u, 0, -1);
			}
		}
	}

}

// Extracted from NewApp::MsgProc; behavior is unchanged.
ExtractedFlow NewApp::OnKeyDownMessage(DWORD& wParam)
{
	if (m_pAviPlayer != nullptr && m_pAviPlayer->m_psCurrent == PLAYSTATE::Running)
	{
		m_pAviPlayer->CloseClip();
		SAFE_DELETE(m_pAviPlayer);
		InitDevice();
	}

	if (m_pEventTranslator == nullptr)
		return ExtractedFlow::Break;

	WORD sVal = GetKeyState(VK_CONTROL);
	m_pEventTranslator->m_bCtrl = (int)sVal >> 8 > 0;
	WORD sVal2 = GetKeyState(VK_SHIFT);
	m_pEventTranslator->m_bShift = (int)sVal2 >> 8 > 0;

	if (wParam != VK_CONTROL && g_pCurrentScene != nullptr && m_pEventTranslator->m_bCtrl != 0)
	{
		if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT)
		{
			g_pCurrentScene->OnAccel(40048);
			return ExtractedFlow::Break;
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD)
		{
			g_pCurrentScene->OnAccel(40047);
			return ExtractedFlow::Break;
		}
		else if (wParam == VK_OEM_4)
		{
			g_pCurrentScene->OnAccel(40027);
			return ExtractedFlow::Break;
		}
		else if (wParam == VK_OEM_6)
		{
			g_pCurrentScene->OnAccel(40028);
			return ExtractedFlow::Break;
		}
		else if (wParam == VK_OEM_7)
		{
			g_pCurrentScene->OnAccel(40029);
			return ExtractedFlow::Break;
		}
	}

	m_pEventTranslator->OnKeyDown(wParam);
	return ExtractedFlow::Next;
}

// Extracted from NewApp::MsgProc; behavior is unchanged.
ExtractedFlow NewApp::OnNetworkMessage(DWORD& wParam, int& lParam)
{
	if (m_pSocketManager == nullptr)
		return ExtractedFlow::Break;

	const auto result = m_pSocketManager->HandleNetworkEvent(wParam, lParam);
	if (result == CPSock::EventResult::Disconnected)
	{
		m_pObjectManager->OnPacketEvent(0, nullptr);
		return ExtractedFlow::Break;
	}
	if (result != CPSock::EventResult::ReadReady)
		return ExtractedFlow::Break;

	int ErrorCode = 0;
	int ErrorType = 0;
	while (1)
	{
		PacketView packet = m_pSocketManager->ReadPacketView(&ErrorCode, &ErrorType);

		if (ErrorCode != 0)
		{
			// A malformed stream cannot be retried at the same receive cursor.
			// Use the normal disconnect transition before any scene dispatch.
			m_pSocketManager->CloseSocket();
			m_pObjectManager->OnPacketEvent(0, nullptr);
			break;
		}
		if (!packet_dispatch::CanDispatch(packet, sizeof(MSG_STANDARD)))
			break;

		// The clock and dump only read the envelope. ObjectManager handles
		// mutable callback adaptation after these historical effects.
		const MSG_STANDARD* pStd = reinterpret_cast<const MSG_STANDARD*>(packet.data);

		unsigned int dwServerTime = m_pTimerManager->GetServerTime();
		g_dwServerTime = pStd->Tick;
		g_dwClientTime = dwServerTime / 1000;

		if (g_hPacketDump != nullptr)
		{
			unsigned int dwTermTime = dwServerTime - g_dwStartPacketTime;
			fwrite(&dwTermTime, 4, 1, g_hPacketDump);
			fwrite(pStd, packet.size, 1, g_hPacketDump);
		}

		if (dwServerTime > g_pLastFixTime + 10000)
		{
			if (g_pCurrentScene != nullptr && g_pCurrentScene->m_nAdjustTime != 0)
			{
				if (g_dwServerTime > dwServerTime + 500)
				{
					m_pTimerManager->SetServerTime(dwServerTime + 85);
				}
				else if (g_dwServerTime < dwServerTime - 500)
				{
					m_pTimerManager->SetServerTime(dwServerTime - 85);
				}
			}
			g_pLastFixTime = dwServerTime;
		}
		m_pObjectManager->OnPacketView(packet);
	}
	return ExtractedFlow::Next;
}

// Extracted from NewApp::MsgProc; behavior is unchanged.
HRESULT NewApp::OnCloseMessage(HWND& hWnd)
{
	if (g_pCurrentScene != nullptr && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
	{
		// A close message may arrive while login/field initialization is still
		// unwinding.  Skip the delayed server notification when its timer or
		// socket is not available, then let the normal teardown continue.
		if (m_pTimerManager && g_pSocketManager && g_pObjectManager && g_pCurrentScene->m_pMyHuman
			&& g_dwStartQuitGameTime == 0)
		{
			g_dwStartQuitGameTime = m_pTimerManager->GetServerTime();
			MSG_SysQuit stSysQuit{};
			// A partially initialized field scene may not have a local human yet;
			// never turn the normal close request into a null dereference.
			stSysQuit.Header.ID = g_pCurrentScene->m_pMyHuman
				? g_pCurrentScene->m_pMyHuman->m_dwID
				: g_pObjectManager->m_dwCharID;
			stSysQuit.Header.Type = MSG_SysQuit_Opcode;
			stSysQuit.Parm = 0;

			g_pSocketManager->SendPacket({
				stSysQuit.Header.Type,
				reinterpret_cast<char*>(&stSysQuit),
				sizeof(stSysQuit)
			});
			return 0;
		}

		if (m_pTimerManager && g_dwStartQuitGameTime != 0
			&& m_pTimerManager->GetServerTime() < g_dwStartQuitGameTime + 3000)
			return 0;
	}

	Finalize();
	if (g_hPacketDump != nullptr)
	{
		fclose(g_hPacketDump);
		g_hPacketDump = nullptr;
	}

	LOG_FINALIZELOG(); //Added this to finalize log file write?
	g_bEndGame = 1;
	EnableSysKey();
	DestroyWindow(hWnd);
	PostQuitMessage(0);
	g_dwStartQuitGameTime = 0;
	return 0;

}


bool NewApp::CheckResolution(DWORD x, DWORD y, DWORD bpp)
{
	int iModeNum = 0;
	DEVMODE devMode;
	for (int bResult = EnumDisplaySettings(0, 0, &devMode); bResult; bResult = EnumDisplaySettings(0, iModeNum, &devMode))
	{
		if (devMode.dmPelsWidth == x && devMode.dmPelsHeight == y && devMode.dmBitsPerPel == bpp)
			return true;

		++iModeNum;
	}

	return false;
}
