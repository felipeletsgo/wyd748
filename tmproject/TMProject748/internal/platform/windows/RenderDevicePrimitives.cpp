#include "pch.h"
// RenderDevice split by responsibility; device lifecycle stays in RenderDevice.cpp.
#include "TextureManager.h"
#include "MeshManager.h"
#include "TMFont2.h"
#include "RenderDevice.h"
#include "TMGlobal.h"
#include "TMMesh.h"

void RenderDevice::RenderRect(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, IDirect3DTexture9* pTexture, float fScaleX, float fScaleY)
{
	RECT srcRect{};
	srcRect.left = (int)iStartX;
	srcRect.top = (int)iStartY;
	srcRect.right = (int)(iStartX + iCX);
	srcRect.bottom = (int)(iStartY + iCY);

	D3DXVECTOR2 rotCenter((float)(iCX / 2.0) + iStartX, (float)(iCY / 2.0) + iStartY);
	D3DXVECTOR2 destPoint(iDestX, iDestY);
	D3DXVECTOR2 scaleVec(fScaleX, fScaleY);

	if (pTexture != nullptr && m_pSprite != nullptr)
	{
		m_pSprite->Begin();
		m_pSprite->Draw(pTexture, &srcRect, &scaleVec, &rotCenter, 0, &destPoint, 0xFFFFFFFF);
		m_pSprite->End();
	}
}

void RenderDevice::RenderRectC(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, IDirect3DTexture9* pTexture, DWORD dwColor, float fScaleX, float fScaleY)
{
	m_CtrlVertex[0].position.x = iDestX;
	m_CtrlVertex[0].position.y = iDestY;
	m_CtrlVertex[0].tu = iStartX / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[0].tv = iStartY / (float)RenderDevice::m_nFontTextureSizeY;
	m_CtrlVertex[1].position.x = (float)(iCX * fScaleX) + iDestX;
	m_CtrlVertex[1].position.y = iDestY;
	m_CtrlVertex[1].tu = (float)((float)(iStartX + iCX) + 0.5f) / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[1].tv = (float)(iStartY + 0.5f) / (float)RenderDevice::m_nFontTextureSizeY;
	m_CtrlVertex[2].position.x = (float)(iCX * fScaleX) + iDestX;
	m_CtrlVertex[2].position.y = (float)(iCY * fScaleY) + iDestY;
	m_CtrlVertex[2].tu = (float)((float)(iStartX + iCX) + 0.5f) / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[2].tv = (float)((float)(iStartY + iCY) + 0.5f) / (float)RenderDevice::m_nFontTextureSizeY;
	m_CtrlVertex[3].position.x = iDestX;
	m_CtrlVertex[3].position.y = (float)(iCY * fScaleY) + iDestY;
	m_CtrlVertex[3].tu = iStartX / (float)RenderDevice::m_nFontTextureSize;
	m_CtrlVertex[3].tv = (float)((float)(iStartY + iCY) + 0.5f) / (float)RenderDevice::m_nFontTextureSizeY;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

	SetTexture(0, pTexture);
	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2u,
		m_CtrlVertex,
		28);
}

void RenderDevice::RenderRectCoord(float iDestX, float iDestY, float iCX, float iCY, IDirect3DTexture9* pTexture, DWORD dwColor, float fU, float fV)
{
	m_CtrlVertex[0].position.x = iDestX;
	m_CtrlVertex[0].position.y = iDestY;
	m_CtrlVertex[0].tu = 0.0;
	m_CtrlVertex[0].tv = 0.0;
	m_CtrlVertex[1].position.x = iDestX + iCX;
	m_CtrlVertex[1].position.y = iDestY;
	m_CtrlVertex[1].tu = fU;
	m_CtrlVertex[1].tv = 0.0;
	m_CtrlVertex[2].position.x = iDestX + iCX;
	m_CtrlVertex[2].position.y = iDestY + iCY;
	m_CtrlVertex[2].tu = fU;
	m_CtrlVertex[2].tv = fV;
	m_CtrlVertex[3].position.x = iDestX;
	m_CtrlVertex[3].position.y = iDestY + iCY;
	m_CtrlVertex[3].tu = 0.0;
	m_CtrlVertex[3].tv = fV;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

	SetTexture(0, pTexture);
	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2u,
		m_CtrlVertex,
		28);
}

void RenderDevice::RenderRectNoTex(float iX, float iY, float iCX, float iCY, DWORD dwColor, int bTrans)
{
	m_CtrlVertex[0].position.x = iX;
	m_CtrlVertex[0].position.y = iY;
	m_CtrlVertex[1].position.x = iX + iCX;
	m_CtrlVertex[1].position.y = iY;
	m_CtrlVertex[2].position.x = iX + iCX;
	m_CtrlVertex[2].position.y = iY + iCY;
	m_CtrlVertex[3].position.x = iX;
	m_CtrlVertex[3].position.y = iY + iCY;

	for (int j = 0; j < 4; ++j)
	{
		m_CtrlVertex[j].diffuse = dwColor;
	}

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
	}

	SetTexture(0, nullptr);
	if (bTrans == 1)
	{
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHAFUNC, 8u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		SetRenderState(D3DRS_ZWRITEENABLE, 0);
	}
	else
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
	}

	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		28);

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000u);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDDu);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDu);
	}
}

void RenderDevice::RenderRectTex(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 512.0f;
	float fTexHeight = 512.0f;

	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}
	if (fAngle <= 0.1 && fAngle >= -0.1)
	{
		m_CtrlVertex[0].position.x = iDestX;
		m_CtrlVertex[0].position.y = iDestY;
		m_CtrlVertex[1].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertex[1].position.y = iDestY;
		m_CtrlVertex[2].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertex[2].position.y = (float)(DestCY * fScale) + iDestY;
		m_CtrlVertex[3].position.x = iDestX;
		m_CtrlVertex[3].position.y = (float)(DestCY * fScale) + iDestY;
	}
	else
	{
		float fRX = (float)(DestCX * fScale) / 2.0f;
		float fRY = (float)(DestCY * fScale) / 2.0f;
		float fCenterX = (float)((float)((float)(DestCX * fScale) + iDestX) + iDestX) / 2.0f;
		float fCenterY = (float)((float)((float)(DestCY * fScale) + iDestY) + iDestY) / 2.0f;
		float fSin = sinf(fAngle);
		float fCos = cosf(fAngle);
		m_CtrlVertex[0].position.x = (float)((float)(-fRX * fCos)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertex[0].position.y = (float)((float)(-fRX * fSin)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertex[1].position.x = (float)((float)(fCos * fRX)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertex[1].position.y = (float)((float)(fSin * fRX)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertex[2].position.x = (float)((float)(fCos * fRX) - (float)(fSin * fRY)) + fCenterX;
		m_CtrlVertex[2].position.y = (float)((float)(fSin * fRX) + (float)(fCos * fRY)) + fCenterY;
		m_CtrlVertex[3].position.x = (float)((float)(-fRX * fCos)
			- (float)(fSin * fRY))
			+ fCenterX;
		m_CtrlVertex[3].position.y = (float)((float)(-fRX * fSin)
			+ (float)(fCos * fRY))
			+ fCenterY;
	}

	float nEndX = iStartX + iCX;
	float nEndY = iStartY + iCY;
	float fEndX = 0.0f;
	float fEndY = 0.0f;

	if (m_iVGAID == 1)
	{
		fEndX = (float)(signed int)(float)((float)(nEndX / fTexWidth) * 10000.0f) * 0.000099999997f;
		fEndY = (float)(signed int)(float)((float)(nEndY / fTexHeight) * 10000.0f) * 0.000099999997f;
	}
	else
	{
		fEndX = nEndX / fTexWidth;
		fEndY = nEndY / fTexHeight;
	}

	m_CtrlVertex[0].tu = (float)(iStartX / fTexWidth) + (float)(0.5f / fTexWidth);
	m_CtrlVertex[0].tv = (float)(iStartY / fTexHeight) + (float)(0.5f / fTexHeight);
	m_CtrlVertex[1].tu = fEndX - (float)(0.5f / fTexWidth);
	m_CtrlVertex[1].tv = (float)(iStartY / fTexHeight) + (float)(0.5f / fTexHeight);
	m_CtrlVertex[2].tu = fEndX - (float)(0.5f / fTexWidth);
	m_CtrlVertex[2].tv = fEndY - (float)(0.5f / fTexHeight);
	m_CtrlVertex[3].tu = (float)(iStartX / fTexWidth) + (float)(0.5f / fTexWidth);
	m_CtrlVertex[3].tv = fEndY - (float)(0.5f / fTexHeight);

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
	}

	SetTexture(0, pTexture);
	if (bTrans == 1)
	{
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHAFUNC, 8u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		SetRenderState(D3DRS_ZWRITEENABLE, 0);
	}
	else
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
	}

	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		28);
}

void RenderDevice::RenderRectTex2C(float iStartX, float iStartY, float iCX, float iCY, float iStartX2, float iStartY2, float iCX2, float iCY2, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, IDirect3DTexture9* pTexture2, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}
	if (fAngle <= 0.1 && fAngle >= -0.1)
	{
		m_CtrlVertexC2[0].position.x = iDestX;
		m_CtrlVertexC2[0].position.y = iDestY;
		m_CtrlVertexC2[1].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertexC2[1].position.y = iDestY;
		m_CtrlVertexC2[2].position.x = (float)(DestCX * fScale) + iDestX;
		m_CtrlVertexC2[2].position.y = (float)(DestCY * fScale) + iDestY;
		m_CtrlVertexC2[3].position.x = iDestX;
		m_CtrlVertexC2[3].position.y = (float)(DestCY * fScale) + iDestY;
	}
	else
	{
		float fRX = (float)(DestCX * fScale) / 2.0f;
		float fRY = (float)(DestCY * fScale) / 2.0f;
		float fCenterX = (float)((float)((float)(DestCX * fScale) + iDestX) + iDestX) / 2.0f;
		float fCenterY = (float)((float)((float)(DestCY * fScale) + iDestY) + iDestY) / 2.0f;
		float fSin = sinf(fAngle);
		float fCos = cosf(fAngle);
		m_CtrlVertexC2[0].position.x = (float)((float)(-fRX * fCos)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertexC2[0].position.y = (float)((float)(-fRX * fSin)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertexC2[1].position.x = (float)((float)(fCos * fRX)
			- (float)(-fRY * fSin))
			+ fCenterX;
		m_CtrlVertexC2[1].position.y = (float)((float)(fSin * fRX)
			+ (float)(-fRY * fCos))
			+ fCenterY;
		m_CtrlVertex[2].position.x = (float)((float)(fCos * fRX) - (float)(fSin * fRY)) + fCenterX;
		m_CtrlVertex[2].position.y = (float)((float)(fSin * fRX) + (float)(fCos * fRY)) + fCenterY;
		m_CtrlVertexC2[3].position.x = (float)((float)(-fRX * fCos)
			- (float)(fSin * fRY))
			+ fCenterX;
		m_CtrlVertexC2[3].position.y = (float)((float)(-fRX * fSin)
			+ (float)(fCos * fRY))
			+ fCenterY;
	}

	float fEndX = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f) * 0.000099999997f;
	float fEndY = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_CtrlVertexC2[0].tu1 = iStartX / fTexWidth;
	m_CtrlVertexC2[0].tv1 = iStartY / fTexHeight;
	m_CtrlVertexC2[1].tu1 = fEndX;
	m_CtrlVertexC2[1].tv1 = iStartY / fTexHeight;
	m_CtrlVertexC2[2].tu1 = fEndX;
	m_CtrlVertexC2[2].tv1 = fEndY;
	m_CtrlVertexC2[3].tu1 = iStartX / fTexWidth;
	m_CtrlVertexC2[3].tv1 = fEndY;

	if (pTexture2)
	{
		D3DSURFACE_DESC desc;
		pTexture2->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	float fEndX2 = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f) * 0.000099999997f;
	float fEndY2 = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_CtrlVertexC2[0].tu2 = iStartX2 / fTexWidth;
	m_CtrlVertexC2[0].tv2 = iStartY2 / fTexHeight;
	m_CtrlVertexC2[1].tu2 = fEndX2;
	m_CtrlVertexC2[1].tv2 = iStartY2 / fTexHeight;
	m_CtrlVertexC2[2].tu2 = fEndX2;
	m_CtrlVertexC2[2].tv2 = fEndY2;
	m_CtrlVertexC2[3].tu2 = iStartX2 / fTexWidth;
	m_CtrlVertexC2[3].tv2 = fEndY2;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertexC2[j].diffuse = dwColor;

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
	}

	SetTexture(0, (IDirect3DBaseTexture9*)pTexture);
	SetTexture(1u, (IDirect3DBaseTexture9*)pTexture2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	SetRenderState(D3DRS_SRCBLEND, 5u);
	SetRenderState(D3DRS_DESTBLEND, 6u);
	SetRenderState(D3DRS_ALPHAFUNC, 8u);
	SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	SetRenderState(D3DRS_ZWRITEENABLE, 0);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 4u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);

	m_pd3dDevice->SetFVF(580);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		36);

	SetRenderStateBlock(2);
	SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
	SetTexture(1u, 0);
}

void RenderDevice::RenderRectTex2(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, IDirect3DTexture9* pTexture2, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture2->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	m_CtrlVertex2[0].position.x = iDestX;
	m_CtrlVertex2[0].position.y = iDestY;
	m_CtrlVertex2[1].position.x = (float)(DestCX * fScale) + iDestX;
	m_CtrlVertex2[1].position.y = iDestY;
	m_CtrlVertex2[2].position.x = (float)(DestCX * fScale) + iDestX;
	m_CtrlVertex2[2].position.y = (float)(DestCY * fScale) + iDestY;
	m_CtrlVertex2[3].position.x = iDestX;
	m_CtrlVertex2[3].position.y = (float)(DestCY * fScale) + iDestY;

	float fEndX = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f)	* 0.000099999997f;
	float fEndY = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_CtrlVertex2[0].tu2 = iStartX / fTexWidth;
	m_CtrlVertex2[0].tv2 = iStartY / fTexHeight;
	m_CtrlVertex2[1].tu2 = fEndX;
	m_CtrlVertex2[1].tv2 = iStartY / fTexHeight;
	m_CtrlVertex2[2].tu2 = fEndX;
	m_CtrlVertex2[2].tv2 = fEndY;
	m_CtrlVertex2[3].tu2 = iStartX / fTexWidth;
	m_CtrlVertex2[3].tv2 = fEndY;

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex2[j].diffuse = dwColor;

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
	}

	SetTexture(0, (IDirect3DBaseTexture9*)pTexture);
	SetTexture(1u, (IDirect3DBaseTexture9*)pTexture2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	SetRenderState(D3DRS_SRCBLEND, 5u);
	SetRenderState(D3DRS_DESTBLEND, 6u);
	SetRenderState(D3DRS_ALPHAFUNC, 8u);
	SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	SetRenderState(D3DRS_ZWRITEENABLE, 0);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 4u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);

	m_pd3dDevice->SetFVF(580);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		36);

	SetRenderStateBlock(2);
	SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
	SetTexture(1u, 0);
}

void RenderDevice::RenderRectTex2M(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, IDirect3DTexture9* pTexture2, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture2->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	m_MiniMapVertex2[0].position.x = iDestX;
	m_MiniMapVertex2[0].position.y = iDestY;
	m_MiniMapVertex2[1].position.x = (float)(DestCX * fScale) + iDestX;
	m_MiniMapVertex2[1].position.y = iDestY;
	m_MiniMapVertex2[2].position.x = (float)(DestCX * fScale) + iDestX;
	m_MiniMapVertex2[2].position.y = (float)(DestCY * fScale) + iDestY;
	m_MiniMapVertex2[3].position.x = iDestX;
	m_MiniMapVertex2[3].position.y = (float)(DestCY * fScale) + iDestY;

	float fEndX = ((((iStartX + iCX) + 0.5f) / fTexWidth) * 10000.0f) * 0.000099999997f;
	float fEndY = ((((iStartY + iCY) + 0.5f) / fTexHeight) * 10000.0f) * 0.000099999997f;

	m_MiniMapVertex2[0].tu2 = iStartX / fTexWidth;
	m_MiniMapVertex2[0].tv2 = iStartY / fTexHeight;
	m_MiniMapVertex2[1].tu2 = fEndX;
	m_MiniMapVertex2[1].tv2 = iStartY / fTexHeight;
	m_MiniMapVertex2[2].tu2 = fEndX;
	m_MiniMapVertex2[2].tv2 = fEndY;
	m_MiniMapVertex2[3].tu2 = iStartX / fTexWidth;
	m_MiniMapVertex2[3].tv2 = fEndY;

	for (int j = 0; j < 4; ++j)
		m_MiniMapVertex2[j].diffuse = dwColor;

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
	}

	SetTexture(0, (IDirect3DBaseTexture9*)pTexture);
	SetTexture(1u, (IDirect3DBaseTexture9*)pTexture2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	SetRenderState(D3DRS_SRCBLEND, 5u);
	SetRenderState(D3DRS_DESTBLEND, 6u);
	SetRenderState(D3DRS_ALPHAFUNC, 8u);
	SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	SetRenderState(D3DRS_ZWRITEENABLE, 0);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
	SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_ALPHAOP, 4u);
	SetTextureStageState(1u, D3DTSS_COLOROP, 4u);
	SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);

	m_pd3dDevice->SetFVF(580);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		36);

	SetRenderStateBlock(2);
	SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
	SetTexture(1u, 0);
}

void RenderDevice::RenderRectTexDamage(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float DestCX, float DestCY, IDirect3DTexture9* pTexture, DWORD dwColor, int bTrans, float fAngle, float fScale)
{
	float fTexWidth = 256.0f;
	float fTexHeight = 256.0f;
	if (pTexture)
	{
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);
		fTexWidth = (float)desc.Width;
		fTexHeight = (float)desc.Height;
	}

	m_CtrlVertex[0].position.x = iDestX - (float)(DestCX * (float)(fScale * 0.5));
	m_CtrlVertex[0].position.y = iDestY - (float)(DestCY * (float)(fScale * 0.5));
	m_CtrlVertex[1].position.x = (float)(DestCX * (float)(fScale * 0.5)) + iDestX;
	m_CtrlVertex[1].position.y = iDestY - (float)(DestCY * (float)(fScale * 0.5));
	m_CtrlVertex[2].position.x = (float)(DestCX * (float)(fScale * 0.5)) + iDestX;
	m_CtrlVertex[2].position.y = (float)(DestCY * (float)(fScale * 0.5)) + iDestY;
	m_CtrlVertex[3].position.x = iDestX - (float)(DestCX * (float)(fScale * 0.5));
	m_CtrlVertex[3].position.y = (float)(DestCY * (float)(fScale * 0.5)) + iDestY;

	if (fAngle <= 0.1 && fAngle >= -0.1)
	{
		m_CtrlVertex[0].tu = iStartX / fTexWidth;
		m_CtrlVertex[0].tv = iStartY / fTexHeight;
		m_CtrlVertex[1].tu = (float)(iStartX + iCX) / fTexWidth;
		m_CtrlVertex[1].tv = iStartY / fTexHeight;
		m_CtrlVertex[2].tu = (float)(iStartX + iCX) / fTexWidth;
		m_CtrlVertex[2].tv = (float)(iStartY + iCY) / fTexHeight;
		m_CtrlVertex[3].tu = iStartX / fTexWidth;
		m_CtrlVertex[3].tv = (float)(iStartY + iCY) / fTexHeight;
	}
	else
	{
		m_CtrlVertex[3].tu = iStartX / fTexWidth;
		m_CtrlVertex[3].tv = (float)((float)(iCY / 2.0) + iStartY) / fTexHeight;
		m_CtrlVertex[0].tu = (float)((float)(iCX / 2.0) + iStartX) / fTexWidth;
		m_CtrlVertex[0].tv = iStartY / fTexHeight;
		m_CtrlVertex[1].tu = (float)(iStartX + iCX) / fTexWidth;
		m_CtrlVertex[1].tv = (float)((float)(iCY / 2.0) + iStartY) / fTexHeight;
		m_CtrlVertex[2].tu = (float)((float)(iCX / 2.0) + iStartX) / fTexWidth;
		m_CtrlVertex[2].tv = (float)(iStartY + iCY) / fTexHeight;
	}

	for (int j = 0; j < 4; ++j)
		m_CtrlVertex[j].diffuse = dwColor;

	if (g_pDevice->m_iVGAID == 1)
	{
		if (g_pDevice->m_dwBitCount == 32)
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
	}
	else if (g_pDevice->m_dwBitCount == 32)
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
	}
	else
	{
		g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
	}

	SetTexture(0, pTexture);
	if (bTrans == 1)
	{
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHAFUNC, 8u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 0);
		SetRenderState(D3DRS_ZWRITEENABLE, 0);
	}
	else
	{
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
	}

	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLEFAN,
		2u,
		m_CtrlVertex,
		28);

	if (m_dwBitCount == 32)
		SetRenderState(D3DRS_ALPHAREF, 0xAA);
	else
		SetRenderState(D3DRS_ALPHAREF, 0xFF);
}

void RenderDevice::RenderRectProgress2(float iX, float iY, float iCX, float iCY, float fProgress, DWORD dwColor)
{
	if (fProgress > 0.0)
	{
		RenderProgressFill(iX, iY, iCX, iCY, fProgress, dwColor);
	}
}

// Extracted from RenderDevice::RenderRectProgress2; behavior is unchanged.
void RenderDevice::RenderProgressFill(float& iX, float& iY, float& iCX, float& iCY, float& fProgress, DWORD& dwColor)
{
	if (m_iVGAID == 1)
	{
		if (m_dwBitCount == 32)
			SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
		else
			SetRenderState(D3DRS_ALPHAREF, 0xF000u);
	}
	else if (m_dwBitCount == 32)
	{
		SetRenderState(D3DRS_ALPHAREF, 0xDDu);
	}
	else
	{
		SetRenderState(D3DRS_ALPHAREF, 0xDu);
	}

	SetTexture(0, 0);
	SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
	SetRenderState(D3DRS_SRCBLEND, 5u);
	SetRenderState(D3DRS_DESTBLEND, 6u);
	SetRenderState(D3DRS_ALPHAFUNC, 8u);
	SetRenderState(D3DRS_ALPHATESTENABLE, 0);
	SetRenderState(D3DRS_ZWRITEENABLE, 0);

	m_pd3dDevice->SetFVF(324);
	float fSX = (float)(iCX / 2.0f) + iX;
	float fSY = (float)(iCY / 2.0f) + iY;
	float fWidth = iCX / 2.0f;
	float fHeight = iCY / 2.0f;
	for (int i = 0; i < 10; ++i)
		m_CtrlProgressVertex[i].diffuse = dwColor;

	if (fProgress <= 0.125f)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = (float)(fWidth * (float)(fProgress / 0.125f)) + fSX;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			1u,
			m_CtrlProgressVertex,
			28u);
	}
	else if (fProgress <= 0.25f)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = fSX + fWidth;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_CtrlProgressVertex[3].position.x = fSX + fWidth;
		m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight)
			+ (float)(fHeight * (float)((float)(fProgress - 0.125f) / 0.125f));
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			2u,
			m_CtrlProgressVertex,
			28u);
	}
	else if (fProgress <= 0.375f)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = fSX + fWidth;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_CtrlProgressVertex[3].position.x = fSX + fWidth;
		m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
		m_CtrlProgressVertex[4].position.x = fSX + fWidth;
		m_CtrlProgressVertex[4].position.y = (float)(fHeight * (float)((float)(fProgress - 0.25f) / 0.125f)) + fSY;
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			3u,
			m_CtrlProgressVertex,
			28u);
	}
	else if (fProgress <= 0.5f)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = fSX + fWidth;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_CtrlProgressVertex[3].position.x = fSX + fWidth;
		m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
		m_CtrlProgressVertex[4].position.x = fSX + fWidth;
		m_CtrlProgressVertex[4].position.y = fSY + fHeight;
		m_CtrlProgressVertex[5].position.x = (float)(fSX + fWidth)
			- (float)(fWidth * (float)((float)(fProgress - 0.375f) / 0.125f));
		m_CtrlProgressVertex[5].position.y = fSY + fHeight;
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			4u,
			m_CtrlProgressVertex,
			28u);
	}
	else if (fProgress <= 0.625f)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = fSX + fWidth;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_CtrlProgressVertex[3].position.x = fSX + fWidth;
		m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
		m_CtrlProgressVertex[4].position.x = fSX + fWidth;
		m_CtrlProgressVertex[4].position.y = fSY + fHeight;
		m_CtrlProgressVertex[5].position.x = fSX;
		m_CtrlProgressVertex[5].position.y = fSY + fHeight;
		m_CtrlProgressVertex[6].position.x = fSX - (float)(fWidth * (float)((float)(fProgress - 0.5f) / 0.125f));
		m_CtrlProgressVertex[6].position.y = fSY + fHeight;
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			5u,
			m_CtrlProgressVertex,
			28u);
	}
	else if (fProgress <= 0.75f)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = fSX + fWidth;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_CtrlProgressVertex[3].position.x = fSX + fWidth;
		m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
		m_CtrlProgressVertex[4].position.x = fSX + fWidth;
		m_CtrlProgressVertex[4].position.y = fSY + fHeight;
		m_CtrlProgressVertex[5].position.x = fSX;
		m_CtrlProgressVertex[5].position.y = fSY + fHeight;
		m_CtrlProgressVertex[6].position.x = fSX - fWidth;
		m_CtrlProgressVertex[6].position.y = fSY + fHeight;
		m_CtrlProgressVertex[7].position.x = fSX - fWidth;
		m_CtrlProgressVertex[7].position.y = (float)(fSY + fHeight)
			- (float)(fHeight * (float)((float)(fProgress - 0.625f) / 0.125f));
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			6u,
			m_CtrlProgressVertex,
			28u);
	}
	else if (fProgress <= 0.875f)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = fSX + fWidth;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_CtrlProgressVertex[3].position.x = fSX + fWidth;
		m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
		m_CtrlProgressVertex[4].position.x = fSX + fWidth;
		m_CtrlProgressVertex[4].position.y = fSY + fHeight;
		m_CtrlProgressVertex[5].position.x = fSX;
		m_CtrlProgressVertex[5].position.y = fSY + fHeight;
		m_CtrlProgressVertex[6].position.x = fSX - fWidth;
		m_CtrlProgressVertex[6].position.y = fSY + fHeight;
		m_CtrlProgressVertex[7].position.x = fSX - fWidth;
		m_CtrlProgressVertex[7].position.y = fSY;
		m_CtrlProgressVertex[8].position.x = fSX - fWidth;
		m_CtrlProgressVertex[8].position.y = fSY - (float)(fHeight * (float)((float)(fProgress - 0.75f) / 0.125f));
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			7u,
			m_CtrlProgressVertex,
			28u);
	}
	else if (fProgress <= 1.0)
	{
		m_CtrlProgressVertex[0].position.x = fSX;
		m_CtrlProgressVertex[0].position.y = fSY;
		m_CtrlProgressVertex[1].position.x = fSX;
		m_CtrlProgressVertex[1].position.y = fSY - fHeight;
		m_CtrlProgressVertex[2].position.x = fSX + fWidth;
		m_CtrlProgressVertex[2].position.y = fSY - fHeight;
		m_CtrlProgressVertex[3].position.x = fSX + fWidth;
		m_CtrlProgressVertex[3].position.y = (float)(fSY - fHeight) + fHeight;
		m_CtrlProgressVertex[4].position.x = fSX + fWidth;
		m_CtrlProgressVertex[4].position.y = fSY + fHeight;
		m_CtrlProgressVertex[5].position.x = fSX;
		m_CtrlProgressVertex[5].position.y = fSY + fHeight;
		m_CtrlProgressVertex[6].position.x = fSX - fWidth;
		m_CtrlProgressVertex[6].position.y = fSY + fHeight;
		m_CtrlProgressVertex[7].position.x = fSX - fWidth;
		m_CtrlProgressVertex[7].position.y = fSY;
		m_CtrlProgressVertex[8].position.x = fSX - fWidth;
		m_CtrlProgressVertex[8].position.y = fSY - fHeight;
		m_CtrlProgressVertex[9].position.x = (float)(fSX - fWidth)
			+ (float)(fWidth * (float)((float)(fProgress - 0.875f) / 0.125f));
		m_CtrlProgressVertex[9].position.y = fSY - fHeight;
		m_pd3dDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLEFAN,
			8u,
			m_CtrlProgressVertex,
			28u);
	}

}


void RenderDevice::RenderRectRot(float iStartX, float iStartY, float iCX, float iCY, float iDestX, float iDestY, float nCenX, float nCenY, float fAngle, IDirect3DTexture9* pTexture, float fScaleX, float fScaleY)
{
	RECT srcRect{};
	srcRect.left = (int)iStartX;
	srcRect.top = (int)iStartY;
	srcRect.right = (int)(iStartX + iCX);
	srcRect.bottom = (int)(iStartY + iCY);

	D3DXVECTOR2 rotCenter(nCenX, nCenY);
	D3DXVECTOR2 destPoint(iDestX, iDestY);
	D3DXVECTOR2 scaleVec(fScaleX, fScaleY);

	if (pTexture != nullptr && m_pSprite != nullptr)
	{
		m_pSprite->Begin();
		m_pSprite->Draw(pTexture, &srcRect, &scaleVec, &rotCenter, -fAngle, &destPoint, 0xFFFFFFFF);
		m_pSprite->End();
	}
}

void RenderDevice::RenderGeomRectImage(GeomControl* ipControl)
{
	if (ipControl->nTextureSetIndex >= 0)
	{
		const ExtractedFlow extractedFlow = RenderTexturedGeomRect(ipControl);
		if (extractedFlow == ExtractedFlow::Return)
			return;
	}
	else if (ipControl->nTextureSetIndex < -2)
	{
		ControlTextureSet* pUISet = g_pTextureManager->GetUITextureSet(-ipControl->nTextureSetIndex);
		if (pUISet != nullptr)
		{
			if (pUISet->pTextureCoord == nullptr)
				return;

			LPDIRECT3DTEXTURE9 pTexture = g_pTextureManager->GetUITexture(pUISet->pTextureCoord[ipControl->nTextureIndex].nTextureIndex,
				2000);

			if (pTexture != nullptr)
			{
				RenderRectTex(
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
					(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
					ipControl->nWidth,
					ipControl->nHeight,
					pTexture,
					ipControl->dwColor,
					1,
					ipControl->fAngle,
					1.0);
			}

			return;
		}
	}
	else
	{
		RenderGuildMarkImage(ipControl);
	}
}

// Extracted from RenderDevice::RenderGeomRectImage; behavior is unchanged.
ExtractedFlow RenderDevice::RenderTexturedGeomRect(GeomControl*& ipControl)
{
	ControlTextureSet* pUISet = g_pTextureManager->GetUITextureSet(ipControl->nTextureSetIndex);

	if (pUISet == nullptr)
		return ExtractedFlow::Return;

	if (pUISet->nCount < ipControl->nTextureIndex)
		return ExtractedFlow::Return;

	if (pUISet->pTextureCoord == nullptr)
		return ExtractedFlow::Return;

	LPDIRECT3DTEXTURE9 pTexture = g_pTextureManager->GetUITexture(pUISet->pTextureCoord[ipControl->nTextureIndex].nTextureIndex, 2000);
	if (ipControl->eRenderType == RENDERCTRLTYPE::RENDER_IMAGE)
	{
		RenderRect(
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + (float)ipControl->nPosX,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + (float)ipControl->nPosY,
			pTexture,
			1.0f,
			1.0f);
	}
	else if (ipControl->eRenderType == RENDERCTRLTYPE::RENDER_IMAGE_STRETCH)
	{
		float fScaleX = ipControl->nWidth / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth;
		float fScaleY = ipControl->nHeight / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight;

		if (ipControl->bClip == 1)
		{
			const float fTextureWidth = (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth;
			const float fTextureHeight = (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight;
			float fLeft = ipControl->fLeft;
			float fTop = ipControl->fTop;
			float fRight = ipControl->fRight;
			float fBottom = ipControl->fBottom;

			if (fLeft < 0.0f)
				fLeft = 0.0f;
			if (fTop < 0.0f)
				fTop = 0.0f;
			if (fRight > fTextureWidth)
				fRight = fTextureWidth;
			if (fBottom > fTextureHeight)
				fBottom = fTextureHeight;

			const float fClipWidth = fRight - fLeft;
			const float fClipHeight = fBottom - fTop;
			if (fClipWidth <= 0.0f || fClipHeight <= 0.0f)
				return ExtractedFlow::Return;

			fScaleX = ipControl->nWidth / fClipWidth;
			fScaleY = ipControl->nHeight / fClipHeight;
			RenderRectRot(
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX + fLeft,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY + fTop,
				fClipWidth,
				fClipHeight,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
				(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
				ipControl->nWidth / 2.0f,
				ipControl->nHeight / 2.0f,
				ipControl->fAngle,
				pTexture,
				fScaleX,
				fScaleY);
			return ExtractedFlow::Return;
		}

		SetRenderStateBlock(0);

		if (ipControl->sSanc > 0)
		{
			float fLayoutScaleX = BASE_ScreenResize(2.0f) + ipControl->nWidth;
			float fLayoutScaleY = BASE_ScreenResize(2.0f) + ipControl->nHeight;
			float fLayoutPosX = BASE_ScreenResize(1.0f);
			float fLayoutPosY = BASE_ScreenResize(1.0f);
			float lev = (float)((float)ipControl->sSanc - 1.0f) * 35.0f;
			LPDIRECT3DTEXTURE9 pEfTexture = g_pTextureManager->GetUITexture(338, 20000);
			if (pEfTexture != nullptr)
			{
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 800)
				{
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.37f,
						1.37f);
				}
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 720)
				{
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.37f,
						1.24f);
				}
				if (g_pApp->m_dwScreenWidth == 1024 && m_dwScreenHeight == 768)
				{//
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.28f,
						1.33f);
				}
				if (g_pApp->m_dwScreenWidth == 800 && m_dwScreenHeight == 600)
				{//
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.01f,
						1.045f);
				}
				if (g_pApp->m_dwScreenWidth == 640 && m_dwScreenHeight == 480)
				{//
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						0.82f,
						0.85f);
				}
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 960)
				{//
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.38f,
						1.66f);
				}
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 1024)
				{//
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.38f,
						1.77f);
				}
				if (g_pApp->m_dwScreenWidth == 1600 && m_dwScreenHeight == 900)
				{//
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.71f,
						1.54f);
				}
				if (g_pApp->m_dwScreenWidth == 1600 && m_dwScreenHeight == 1024)
				{//
					RenderRectRot(
						lev,
						0.0f,
						35.0f,
						35.0f,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.72f,
						1.77f);
				}
			}
		}
		if (ipControl->sLegend > 0)
		{
			float fLayoutScaleX = BASE_ScreenResize(2.0f) + ipControl->nWidth;
			float fLayoutScaleY = BASE_ScreenResize(2.0f) + ipControl->nHeight;
			float fLayoutPosX = BASE_ScreenResize(1.0f);
			float fLayoutPosY = BASE_ScreenResize(1.0f);
			float fStartX = 0.0f;
			float fStartY = 0.0f;

			LPDIRECT3DTEXTURE9 pEfTexture = g_pTextureManager->GetUITexture(338, 2000);
			switch (ipControl->sLegend)
			{
			case 5:
			case 9:
				fStartX = 315.0f;
				break;
			case 6:
			case 10:
				fStartX = 350.0f;
				break;
			case 7:
			case 11:
				fStartX = 385.0f;
				break;
			case 8:
			case 12:
				fStartX = 420.0f;
				break;
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
			case 38:
			case 39:
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:
			case 47:
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:
			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
			case 60:
			case 61:
			case 62:
			case 63:
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
			case 69:
			case 70:
			case 71:
			case 72:
			case 73:
			case 74:
			case 75:
			case 76:
			case 77:
			case 78:
			case 79:
			case 80:
			case 81:
			case 82:
			case 83:
			case 84:
			case 85:
			case 86:
			case 87:
			case 88:
			case 89:
			case 90:
			case 91:
			case 92:
			case 93:
			case 94:
			case 95:
			case 96:
			case 97:
			case 98:
			case 99:
			case 100:
			case 101:
			case 102:
			case 103:
			case 104:
			case 105:
			case 106:
			case 107:
			case 108:
			case 109:
			case 110:
			case 111:
			case 112:
			case 113:
			case 114:
			case 115:
				pEfTexture = nullptr;
				break;
			case 116:
				fStartX = 100.0;
				fStartY = 35.0f;
				break;
			case 117:
				fStartX = 135.0f;
				fStartY = 35.0f;
				break;
			case 118:
				fStartX = 170.0f;
				fStartY = 35.0f;
				break;
			case 119:
				fStartX = 205.0f;
				fStartY = 35.0f;
				break;
			case 120:
				fStartX = 240.0f;
				fStartY = 35.0f;
				break;
			case 121:
				fStartX = 275.0f;
				fStartY = 35.0f;
				break;
			case 122:
				fStartX = 310.0f;
				fStartY = 35.0f;
				break;
			case 123:
				fStartX = 345.0f;
				fStartY = 35.0f;
				break;
			case 124:
				fStartX = 380.0f;
				fStartY = 35.0f;
				break;
			case 125:
				fStartX = 415.0f;
				fStartY = 35.0f;
				break;
			}

			if (pEfTexture != nullptr && g_pApp->m_dwScreenWidth == 1600)
			{
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 800)
				{
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.37f,
						1.37f);
				}
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 720)
				{
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.37f,
						1.24f);
				}
				if (g_pApp->m_dwScreenWidth == 1024 && m_dwScreenHeight == 768)
				{//
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.11f,
						1.33f);
				}
				if (g_pApp->m_dwScreenWidth == 800 && m_dwScreenHeight == 600)
				{//
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						0.88f,
						1.030f);
				}
				if (g_pApp->m_dwScreenWidth == 640 && m_dwScreenHeight == 480)
				{//
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						0.71f,
						0.85f);
				}
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 960)
				{//
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.38f,
						1.66f);
				}
				if (g_pApp->m_dwScreenWidth == 1280 && m_dwScreenHeight == 1024)
				{//
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.38f,
						1.77f);
				}
				if (g_pApp->m_dwScreenWidth == 1600 && m_dwScreenHeight == 900)
				{//
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.68f,
						1.51f);
				}
				if (g_pApp->m_dwScreenWidth == 1600 && m_dwScreenHeight == 1024)
				{//
					RenderDevice::RenderRectRot(
						fStartX,
						fStartY,
						35.0,
						35.0,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX + -1,
						(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY + -1,
						ipControl->nWidth / 2.0f,
						ipControl->nHeight / 2.0f,
						ipControl->fAngle,
						pEfTexture,
						1.70f,
						1.74f);
				}
			}
		}
		RenderDevice::RenderRectRot(
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartX,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nStartY,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
			ipControl->nWidth / 2.0f,
			ipControl->nHeight / 2.0f,
			ipControl->fAngle,
			pTexture,
			fScaleX,
			fScaleY);
	}
	else if (ipControl->eRenderType == RENDERCTRLTYPE::RENDER_IMAGE_TILE)
	{
		RenderRectCoord(
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestX + ipControl->nPosX,
			(float)pUISet->pTextureCoord[ipControl->nTextureIndex].nDestY + ipControl->nPosY,
			ipControl->nWidth,
			ipControl->nHeight,
			pTexture,
			ipControl->dwColor,
			ipControl->nWidth / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nWidth,
			ipControl->nHeight / (float)pUISet->pTextureCoord[ipControl->nTextureIndex].nHeight);
	}
	return ExtractedFlow::Next;
}


// Extracted from RenderDevice::RenderGeomRectImage; behavior is unchanged.
void RenderDevice::RenderGuildMarkImage(GeomControl*& ipControl)
{
	int nMarkIndex = ipControl->nMarkIndex;
	if (nMarkIndex >= 0 && nMarkIndex <= 64)
	{
		if (g_pTextureManager->m_stGuildMark[nMarkIndex].pTexture)
		{
			float iX = ipControl->nPosX - 2.0f;
			float iY = ipControl->nPosY - 2.0f;
			float iCX = 2.0f + 18.0f;
			float iCY = 2.0f + 14.0f;
			float fWidthRatio = RenderDevice::m_fWidthRatio;
			float fHeightRatio = RenderDevice::m_fHeightRatio;
			if (RenderDevice::m_fWidthRatio != 1.0f)
			{
				iCX = RenderDevice::m_fWidthRatio * 18.5f;
				iCY = RenderDevice::m_fHeightRatio * 14.5f;
			}
			if (RenderDevice::m_fHeightRatio == 0.8f)
			{
				iX = iX - 0.0f;
				iY = iY + 1.0f;
			}
			else if (RenderDevice::m_fHeightRatio == 1.0f)
			{
				iX = iX + 0.0f;
				iY = iY + 0.0f;
			}
			else if (RenderDevice::m_fWidthRatio == 1.6f)
			{
				iX = iX - 1.0f;
				iY = iY + 0.0f;
			}
			else if (RenderDevice::m_fWidthRatio == 2.0f)
			{
				iX = iX - 1.0f;
				iY = iY + 0.0f;
			}
			if (ipControl->nMarkLayout == 1)
			{
				RenderRectNoTex(
					iX - (float)((float)(16.0f * RenderDevice::m_fWidthRatio) - 16.0f),
					iY - (float)((float)(12.0f * RenderDevice::m_fHeightRatio) - 12.0f),
					iCX,
					iCY,
					0xFFFFD700,
					1);
			}
			else if (ipControl->nMarkLayout == 2)
			{
				RenderRectNoTex(
					iX - (float)((float)(16.0f * RenderDevice::m_fWidthRatio) - 16.0f),
					iY - (float)((float)(12.0f * RenderDevice::m_fHeightRatio) - 12.0f),
					iCX,
					iCY,
					0xFFC0C0C0,
					1);
			}

			RenderRect(
				0.0,
				0.0,
				16.0,
				12.0,
				ipControl->nPosX - (float)((float)(16.0f * fWidthRatio) - 16.0f),
				ipControl->nPosY - (float)((float)(12.0f * fHeightRatio) - 12.0f),
				g_pTextureManager->m_stGuildMark[nMarkIndex].pTexture,
				fWidthRatio,
				fHeightRatio);
			g_pTextureManager->m_stGuildMark[nMarkIndex].dwLastRenderTime = timeGetTime();
		}
	}
	else
	{
		int bTrans = 0;
		if (ipControl->nTextureSetIndex == -2)
			bTrans = 1;
		RenderRectNoTex(
			ipControl->nPosX,
			ipControl->nPosY,
			ipControl->nWidth,
			ipControl->nHeight,
			ipControl->dwColor,
			bTrans);
	}

}


void RenderDevice::RenderGeomControl(GeomControl* ipControl)
{
	if (ipControl != nullptr && ipControl->bVisible == 1)
	{
		if (g_pDevice->m_iVGAID == 1)
		{
			if (g_pDevice->m_dwBitCount == 32)
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
			else
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
		}
		else if (g_pDevice->m_dwBitCount == 32)
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
		}
		else
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
		}

		SetRenderStateBlock(0);

		switch (ipControl->eRenderType)
		{
		case RENDERCTRLTYPE::RENDER_TEXT:
		case RENDERCTRLTYPE::RENDER_SHADOW:
			if (ipControl->pFont)
			{
				int nUp = 0;
				if (m_dwScreenWidth == 640)
					nUp = -1;
				ipControl->pFont->Render(
					(int)ipControl->nPosX,
					(int)ipControl->nPosY + nUp,
					(int)ipControl->eRenderType);
			}
			break;
		case RENDERCTRLTYPE::RENDER_IMAGE:
		case RENDERCTRLTYPE::RENDER_IMAGE_TILE:
		case RENDERCTRLTYPE::RENDER_IMAGE_STRETCH:
		case RENDERCTRLTYPE::RENDER_TEXT_FOCUS:
			RenderGeomRectImage(ipControl);
			if (ipControl->strString[0])
			{
				int nLength = strlen(ipControl->strString);
				if (nLength > 0 && nLength < 64 && ipControl->pFont)
				{
					int nUp = 0;
					if (m_dwScreenWidth == 640)
						nUp = -1;
					if (ipControl->nTextureSetIndex == 526)
					{
						ipControl->pFont->Render(
							(int)(float)((float)(ipControl->nWidth - (float)(10 * nLength)) + ipControl->nPosX),
							nUp + (int)(float)((float)(ipControl->nHeight - 18.0) + ipControl->nPosY),
							(int)ipControl->eRenderType);
					}
					else
					{
						ipControl->pFont->Render(
							(int)(float)((float)((float)(ipControl->nWidth - (float)(6 * nLength)) / 2.0f) + ipControl->nPosX),
							nUp + (int)(float)((float)((float)(ipControl->nHeight - 12.0f) / 2.0f) + ipControl->nPosY),
							(int)ipControl->eRenderType);
					}
				}
			}
			break;
		case RENDERCTRLTYPE::RENDER_3DOBJ:
			if (ipControl->n3DObjIndex < 737 || ipControl->n3DObjIndex > 739)
			{
				SetRenderStateBlock(1);
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 8);
				SetRenderState(D3DRS_FOGENABLE, 0);
				SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
				SetRenderState(D3DRS_ALPHATESTENABLE, 0);

				TMMesh* pMesh = g_pMeshManager->GetCommonMesh(ipControl->n3DObjIndex, 0, 3_min);
				// Port the exact WYD 7.48 control renderer from Ghidra
				// FUN_0042E8A1: classic UI keeps scale 1, UI mode 2 compensates
				// only by the width ratio. The imported 7.59 resize and 0.9 factors
				// enlarged legacy 3D item meshes and moved them outside their cells.
				float fScale = g_UIVer == 2 ? 1.0f / RenderDevice::m_fWidthRatio : 1.0f;
				if (ipControl->dwBGColor == -1)
					fScale = fScale * 0.80000001f;

				if (ipControl->n3DObjIndex >= 937 && ipControl->n3DObjIndex <= 946
					|| ipControl->n3DObjIndex >= 300 && ipControl->n3DObjIndex <= 303)
				{
					fScale = 0.5f;
				}

				if (pMesh)
				{
					pMesh->RenderForUI((int)ipControl->nPosX, (int)ipControl->nPosY, ipControl->fAngle, ipControl->fScale * fScale,
						ipControl->dwColor, ipControl->nTextureIndex, ipControl->nTextureSetIndex, ipControl->sLegend);
				}
				if (ipControl->dwBGColor)
				{
					int nTexIndex = 123;
					if (ipControl->dwBGColor == 0xFFFFFFAA)
						nTexIndex = 56;
					if (ipControl->dwBGColor == 0xFFFFFFFF)
						nTexIndex = 273;
					if (ipControl->dwBGColor == 0xFF8800BB)
						nTexIndex = 122;
					if (ipControl->dwBGColor == 0xFF444488)
						nTexIndex = 122;
					if (ipControl->dwBGColor == 0xFFFF0000)
						nTexIndex = 56;

					RenderGeomControlBG(ipControl, ipControl->dwBGColor, nTexIndex);
				}

				SetRenderState(D3DRS_FOGENABLE, m_bFog);
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 4);
			}
			else
			{
				unsigned int dwColor[3] = { 0xFFFF4400, 0xFF3366FF, 0xFF00FF00 };
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 8u);
				g_pDevice->SetRenderState(D3DRS_LIGHTING, 0);
				g_pDevice->SetRenderState(D3DRS_DESTBLEND, 2u);
				g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
				g_pDevice->SetRenderState(D3DRS_SRCBLEND, 5u);
				g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
				SetRenderState(D3DRS_FOGENABLE, 0);

				float fScale = 1.0f / RenderDevice::m_fWidthRatio;
				TMMesh* pMesh = g_pMeshManager->GetCommonMesh(ipControl->n3DObjIndex, 1, 3_min);

				if (pMesh)
				{
					D3DVERTEXBUFFER_DESC vDesc;
					if (SUCCEEDED(pMesh->m_pVB->GetDesc(&vDesc)))
					{
						RDLVERTEX* pVertex;
						pMesh->m_pVB->Lock(0, 0, (void**)&pVertex, 0);

						int nCount = vDesc.Size / 24;
						for (int i = 0; i < nCount; ++i)
							pVertex[i].diffuse = ipControl->n3DObjIndex - 2964;

						pMesh->m_pVB->Unlock();

						pMesh->m_nTextureIndex[0] = 204;

						pMesh->RenderForUI((int)ipControl->nPosX, (int)ipControl->nPosY, ipControl->fAngle, ipControl->fScale * fScale,
							ipControl->dwColor, ipControl->nTextureIndex, ipControl->nTextureSetIndex, ipControl->sLegend);
					}
				}

				SetRenderState(D3DRS_FOGENABLE, m_bFog);
				g_pDevice->SetRenderState(D3DRS_ZFUNC, 4);
			}
			break;
		}

		if (g_pDevice->m_iVGAID == 1)
		{
			if (g_pDevice->m_dwBitCount == 32)
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
			else
				g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xF000);
		}
		else if (g_pDevice->m_dwBitCount == 32)
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xDD);
		}
		else
		{
			g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0xD);
		}
		return;
	}
}

void RenderDevice::RenderGeomControlBG(GeomControl* ipControl, DWORD dwColor, int nTextureIndex)
{
	if (m_dwStartTime == 0)
		m_dwStartTime = g_pTimerManager->GetServerTime();

	float fCurrRatio = (float)((g_pTimerManager->GetServerTime() - m_dwStartTime) % 2000);
	fCurrRatio = (((fCurrRatio * 0.0005f) - 0.5f) * 2.0f) * D3DXToRadian(180);
	fCurrRatio = (fCurrRatio * 0.34f) + 0.64f;

	unsigned int dwCurrColor = WYD_RGBA((unsigned int)((float)WYDCOLOR_RED(dwColor) * fCurrRatio), (unsigned int)((float)WYDCOLOR_GREEN(dwColor) * fCurrRatio),
		(unsigned int)((float)WYDCOLOR_BLUE(dwColor) * fCurrRatio), (unsigned int)((float)WYDCOLOR_ALPHA(dwColor) * fCurrRatio));

	RDTLVERTEX v[4]{};
	for (int i = 0; i < 4; ++i)
	{
		v[i].rhw = 1.0f;
		v[i].diffuse = dwCurrColor;
	}

	float fX = ipControl->nPosX;
	float fY = ipControl->nPosY;
	float fHalfWidth = (float)(ipControl->m_fWidth * 0.5f) * ipControl->fScale;
	float fHalfHeight = (float)(ipControl->m_fHeight * 0.5f) * ipControl->fScale;

	if (dwColor == -1)
	{
		fHalfWidth = fHalfWidth * 2.0f;
		fHalfHeight = fHalfHeight * 2.0f;
	}

	v[0].position.x = fX - fHalfWidth;
	v[0].position.y = fY - fHalfHeight;
	v[0].position.z = 0.1f;
	v[0].tu = 0.0f;
	v[0].tv = 0.0f;
	v[1].position.x = fX + fHalfWidth;
	v[1].position.y = fY - fHalfHeight;
	v[1].position.z = 0.1f;
	v[1].tu = 0.1f;
	v[1].tv = 0.0f;
	v[2].position.x = fX - fHalfWidth;
	v[2].position.y = fY + fHalfHeight;
	v[2].position.z = 0.1f;
	v[2].tu = 0.0f;
	v[2].tv = 1.0f;
	v[3].position.x = fX + fHalfWidth;
	v[3].position.y = fY + fHalfHeight;
	v[3].position.z = 0.1f;
	v[3].tu = 1.0f;
	v[3].tv = 1.0f;

	SetTexture(0, g_pTextureManager->GetEffectTexture(nTextureIndex, 5000));
	SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	SetRenderState(D3DRS_SRCBLEND, 5);
	SetRenderState(D3DRS_DESTBLEND, 2);
	SetTextureStageState(0, D3DTSS_ALPHAOP, 2);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	m_pd3dDevice->SetFVF(324);
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, 28);
	SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
}
