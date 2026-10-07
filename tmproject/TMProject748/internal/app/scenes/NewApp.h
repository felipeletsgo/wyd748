#pragma once
#include "ExtractedFlow.h"

class EventTranslator;
class RenderDevice;
class CSoundManager;
class DS_SOUND_MANAGER;
class TimerManager;
class ObjectManager;
class CPSock;

struct SaveUpdatAndConfig
{
    short Version;
    short Config[14];
};

class NewApp
{
public:
    NewApp();
    virtual ~NewApp();

    virtual HRESULT Initialize(HINSTANCE hInstance, int nFull);
    HRESULT InitDevice();
    void InitMusicList();
    virtual HRESULT Finalize();
    virtual DWORD Run();
    virtual void OnCreate(HWND hWnd, DWORD wParam, int lParam);
    virtual HRESULT RenderScene();
    // Writes one opt-in perf_trace sample (WYD_PERF_LOG).
    void WritePerfSample();
    // True when the window is unfocused or minimized (light profile).
    bool IsIdleWindow() const;
    virtual void SetObjectManager(ObjectManager* pManager);
    virtual EventTranslator* GetEventTranslator();

    HWND GetSafeHwnd();
    void MixHelp();
    HRESULT MsgProc(HWND hWnd, DWORD uMsg, DWORD wParam, int lParam);
    bool CheckResolution(DWORD x, DWORD y, DWORD bpp);


public:
    HWND m_hWnd;
    int m_bActive;
    unsigned int m_dwWindowStyle;
    unsigned int m_dwScreenWidth;
    unsigned int m_dwScreenHeight;
    unsigned int m_dwColorBit;
    int m_bwFullScreen;
    char m_strWindowTitle[64];
    char m_szServerIP[128];
    char m_szLoginServerIP[128];
    RenderDevice* m_pRenderDevice;
    CSoundManager* m_pSoundManager;
    DS_SOUND_MANAGER* m_pBGMManager;
    EventTranslator* m_pEventTranslator;
    TimerManager* m_pTimerManager;
    ObjectManager* m_pObjectManager;
    CPSock* m_pSocketManager;
    int m_bTex;
    int m_nSound;
    int m_nMusic;
    int m_nCameraView;
    int m_Winstate;
    bool m_binactive;
    HBITMAP m_hBMBtnBG;
    bool china_bWrite;
    int china_Playtime;

private:
	// Helpers extracted from NewApp::MsgProc.
	void OnInputLanguageChange(HWND& hWnd);
	ExtractedFlow OnKeyDownMessage(DWORD& wParam);
	ExtractedFlow OnNetworkMessage(DWORD& wParam, int& lParam);
	HRESULT OnCloseMessage(HWND& hWnd);
};
