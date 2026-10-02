#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMFieldScene.h"

SMessagePanel::SMessagePanel(const char* istrMessage, float inX, float inY, float inWidth, float inHeight, unsigned int dwTime)
	: SPanel(-45, inX, inY, inWidth, inHeight, 0xAAFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	// FUN_00403924 constructs the stock texture first and then applies the 7.48
	// MessagePanel2 composition.  This project deliberately keeps that translucent
	// notification contract even with the classic HUD because login errors, notices
	// and the delayed-exit countdown all share this panel.
	SetTextureSetIndex(-178);
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_MESSAGEPANEL;
	m_dwControlID = 5638;
	SetCenterPos(m_dwControlID, inX, inY, inWidth, inHeight);
	m_pPanelL = nullptr;
	m_pPanelR = nullptr;

	m_pText = new SText(-2, istrMessage, 0xFFFFFFFF, 30.0f, 1.0f, (float)(inWidth - 2.0f) - 50.0f, inHeight - 2.0f, 1, 0, 1, 1);
	m_pText2 = new SText(-2, "", 0xFFFFFFFF, 50.0f, 24.0f, (float)(inWidth - 2.0) - 50.0f, 14.0f, 1, 0, 1, 1);
	m_pPanelL = new SPanel(-259, -11.0f, 0.0f, 11.0f, inHeight, 0xAAFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
	m_pPanelR = new SPanel(-260, inWidth, 0.0f, 11.0f, inHeight, 0xAAFFFFFF, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);

	if (m_pPanelL != nullptr)
		AddChild(m_pPanelL);
	if (m_pPanelR != nullptr)
		AddChild(m_pPanelR);
	if (m_pText != nullptr)
		AddChild(m_pText);

	m_dwLifeTime = dwTime;
	m_dwOldServerTime = g_pTimerManager->GetServerTime();
}

SMessagePanel::~SMessagePanel()
{
	SAFE_DELETE(m_pText2);
}

void SMessagePanel::SetMessage(const char* istrMessage, unsigned int dwTime)
{
	m_dwLifeTime = dwTime;
	m_dwOldServerTime = g_pTimerManager->GetServerTime();
	m_pText->SetText((char*)istrMessage, 0);
}

void SMessagePanel::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	unsigned int dwServerTime = g_pTimerManager->GetServerTime();
	if (m_dwLifeTime && dwServerTime - m_dwOldServerTime > m_dwLifeTime)
		SetVisible(0, 1);

	SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);
}

void SMessagePanel::SetVisible(int bVisible, int bSound)
{
	SPanel::SetVisible(bVisible);

	auto soundManager = g_pSoundManager;
	if (bVisible == 1 && bSound == 1 && soundManager != nullptr)
	{
		auto soundData = soundManager->GetSoundData(33);
		if (soundData)
			soundData->Play();
	}
}
