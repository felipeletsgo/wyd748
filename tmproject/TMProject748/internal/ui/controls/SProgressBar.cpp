#include "pch.h"
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SGrid.h"
#include "SControl.h"
#include "TMGlobal.h"
#include "SControlContainer.h"

SProgressBar::SProgressBar(int inTextureSetIndex, int inCurrent, int inMax, float inX, float inY, float inWidth, float inHeight, unsigned int idwProgressColor, unsigned int idwColor, unsigned int dwStyle)
	: SPanel(inTextureSetIndex, inX, inY, inWidth, inHeight, idwColor, RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
{
	m_nCurrent = inCurrent;
	m_nMax = inMax;
	m_InitHeight = 0.0f;
	m_InitStartY = 0.0f;
	m_InitWidth = 0.0f;
	m_InitStartX = 0.0f;
	m_GCProgress = GeomControl(RENDERCTRLTYPE::RENDER_IMAGE_STRETCH, inTextureSetIndex, inX, inY, 0.0f, inHeight, 0, idwProgressColor);
	m_GCPanel.eRenderType = RENDERCTRLTYPE::RENDER_IMAGE_STRETCH;
	m_GCPanel.nTextureIndex = 1;
	m_dwStyle = dwStyle;
	m_eCtrlType = CONTROL_TYPE::CTRL_TYPE_PROGRESSBAR;

	if (RenderDevice::m_nBright > 58)
	{
		int nR = WYDCOLOR_RED(idwProgressColor) - (RenderDevice::m_nBright - 40);
		int nG = WYDCOLOR_GREEN(idwProgressColor) - (RenderDevice::m_nBright - 40);
		int nB = WYDCOLOR_BLUE(idwProgressColor) - (RenderDevice::m_nBright - 40);

		if (nR < 0)
			nR = 0;
		if (nG < 0)
			nG = 0;
		if (nB < 0)
			nB = 0;

		m_GCProgress.dwColor = nB | (nG << 8) | idwProgressColor & 0xFF000000 | (nR << 16);
	}

	int nTextureSetIndex = m_GCProgress.nTextureSetIndex;
	if (nTextureSetIndex < -2)
		nTextureSetIndex = -m_GCProgress.nTextureSetIndex;

	auto pUISet = g_pTextureManager->GetUITextureSet(nTextureSetIndex);
	if (pUISet != nullptr)
	{
		m_InitHeight = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nHeight;
		m_InitStartY = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nStartY;
		m_InitWidth = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nWidth;
		m_InitStartX = (float)pUISet->pTextureCoord[m_GCProgress.nTextureIndex].nStartX;
	}

	Update();
}

SProgressBar::~SProgressBar()
{
	ResetBar();

	SControlContainer* pControlContainer = g_pCurrentScene->m_pControlContainer;

	if (pControlContainer != nullptr && m_GCProgress.nLayer >= 0)
		RemoveRenderControlItem(pControlContainer->m_pDrawControl, &m_GCProgress, m_GCProgress.nLayer);
}

void SProgressBar::SetCurrentProgress(int inCurrent)
{
	if (m_nCurrent != inCurrent)
	{
		m_nCurrent = inCurrent;
		if (inCurrent < 0)
			m_nCurrent = 0;

		Update();
	}
}

void SProgressBar::SetMaxProgress(int inMax)
{
	if (m_nMax != inMax)
	{
		m_nMax = inMax;
		Update();
	}
}

int SProgressBar::GetCurrentProgress()
{
	return m_nCurrent;
}

int SProgressBar::GetMaxProgress()
{
	return m_nMax;
}

void SProgressBar::Update()
{
	if (m_nCurrent > m_nMax)
		m_nCurrent = m_nMax;
	if (m_nMax <= 0)
		m_nMax = 1;

	if (m_dwStyle == 1)
		m_nProgressWidth = (float)((float)m_nCurrent * m_nWidth) / (float)m_nMax;
	else if (m_dwStyle == 2)
		m_nProgressWidth = (float)((float)m_nCurrent * m_nWidth) / (float)m_nMax;
	else
		m_nProgressHeight = (float)((float)m_nCurrent * m_nHeight) / (float)m_nMax;
}

void SProgressBar::FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag)
{
	SPanel::FrameMove2(pDrawList, ivParentPos, inParentLayer, nFlag);

	if (m_dwStyle == 1)
	{
		m_GCProgress.bClip = 0;
		m_GCProgress.nPosX = (float)(ivParentPos.x + m_nPosX) + 2.0f;
		m_GCProgress.nPosY = (float)(ivParentPos.y + m_nPosY) + 2.0f;
		m_GCProgress.nWidth = m_nProgressWidth;
		m_GCProgress.nHeight = m_nHeight - 4.0f;
		m_GCProgress.nLayer = inParentLayer;
	}
	else if (m_dwStyle == 2)
	{
		float fProgress = m_nWidth > 0.0f ? m_nProgressWidth / m_nWidth : 0.0f;
		if (fProgress < 0.0f)
			fProgress = 0.0f;
		else if (fProgress > 1.0f)
			fProgress = 1.0f;

		m_GCProgress.bClip = m_InitWidth > 0.0f && m_InitHeight > 0.0f;
		m_GCProgress.fLeft = 0.0f;
		m_GCProgress.fTop = 0.0f;
		m_GCProgress.fRight = m_InitWidth * fProgress;
		m_GCProgress.fBottom = m_InitHeight;

		m_GCProgress.nPosX = (float)(ivParentPos.x + m_nPosX) + 2.0f;
		m_GCProgress.nPosY = (float)(ivParentPos.y + m_nPosY) + 2.0f;
		m_GCProgress.nWidth = m_nProgressWidth;
		m_GCProgress.nHeight = m_nHeight - 4.0f;
		m_GCProgress.nLayer = inParentLayer;
	}
	else
	{
		float fProgress = m_nHeight > 0.0f ? m_nProgressHeight / m_nHeight : 0.0f;
		if (fProgress < 0.0f)
			fProgress = 0.0f;
		else if (fProgress > 1.0f)
			fProgress = 1.0f;

		m_GCProgress.bClip = m_InitWidth > 0.0f && m_InitHeight > 0.0f;
		m_GCProgress.fLeft = 0.0f;
		m_GCProgress.fTop = m_InitHeight * (1.0f - fProgress);
		m_GCProgress.fRight = m_InitWidth;
		m_GCProgress.fBottom = m_InitHeight;

		m_GCProgress.nPosX = ivParentPos.x + m_nPosX;
		m_GCProgress.nPosY = ((ivParentPos.y + m_nPosY) + m_nHeight) - m_nProgressHeight;
		m_GCProgress.nWidth = m_nWidth;
		m_GCProgress.nHeight = m_nProgressHeight;
		m_GCProgress.nLayer = inParentLayer;
	}

	AddRenderControlItem(pDrawList, &m_GCProgress, inParentLayer);
}

void SProgressBar::ResetBar()
{
	int nTextureSetIndex = 0;
	if (m_GCProgress.nTextureSetIndex < -2)
		nTextureSetIndex = -m_GCProgress.nTextureSetIndex;
}
