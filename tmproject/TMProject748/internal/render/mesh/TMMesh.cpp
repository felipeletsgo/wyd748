#include "pch.h"
#include "AssetFileOpen.h"
#include "../../platform/diagnostics/PerfTrace.h"
#include "TreeNode.h"
#include "TMMesh.h"
#include "TMGlobal.h"
#include "TMObject.h"
#include "TMLog.h"
#include "DyePixelShader.h"
#include "RefinementGlowTexture.h"
#include "NativeMountGlow.h"
#include "MsaLoad.h"
#include <cfloat>
#include <memory>

TMMesh::TMMesh()
{
	m_pVB = 0;
	m_pIB = 0;
	m_bDummy = 0;
	m_bFirst = 1;
	m_sizeVertex = 0;
	m_dwFVF = 0;
	m_dwAttCount = 0;
	m_nMeshType = 0;
	m_dwLastUsedTime = 0;
	// SetTexture reads the retention time; meshes built outside GetCommonMesh
	// (for example TMSea) never receive one.
	m_dwShowTime = 0;

	m_fScaleV = 1.0f;
	m_fScaleH = 1.0f;
	m_fRadius = 1.0f;

	// Native WYD 7.48 FUN_004baf10 initializes every maximum with FLT_MIN
	// (0x00800000) and every minimum with FLT_MAX (0x7F7FFFFF).  The recovered
	// source had lost the decimal exponents, which biased m_vecCenter and moved
	// inventory/equipment meshes away from the centre of their controls.
	m_fMaxX = FLT_MIN;
	m_fMinX = FLT_MAX;
	m_fMaxY = FLT_MIN;
	m_fMinY = FLT_MAX;
	m_fMaxZ = FLT_MIN;
	m_fMinZ = FLT_MAX;

	m_bEffect = 0;

	memset(m_AttRange, 0, sizeof(m_AttRange));
	memset(m_nTextureIndex, 0, sizeof(m_nTextureIndex));
}

TMMesh::~TMMesh()
{
	if (m_pVB != nullptr)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}

	if (m_pIB != nullptr)
	{
		m_pIB->Release();
		m_pIB = nullptr;
	}
}

int TMMesh::Render(char cMulti, int nTexOffset)
{
	if (!m_pVB || !m_pIB)
		return 1;

	if (g_pDevice->m_pd3dDevice->SetStreamSource(0, m_pVB, 0, m_sizeVertex) < 0)
		return 1;

	if (g_pDevice->m_pd3dDevice->SetIndices(m_pIB) < 0)
		return 1;

	if (g_pDevice->m_pd3dDevice->SetFVF(m_dwFVF) < 0)
		return 1;

	g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FOGVERTEXMODE, 3);

	if (!cMulti || m_dwFVF != 530)
		return RenderDraw(nTexOffset);
	if (!UvAnimatable())
		return RenderDraw(nTexOffset);

	DWORD dwTime = g_pTimerManager->GetServerTime();
	float fProgress;
	if (cMulti == 100)
		fProgress = (float)(dwTime % 8000) / 8000.0f;
	else
		fProgress = (float)(dwTime % 4000) / 4000.0f;

	RDVERTEX2* pVertex = nullptr;

	if (FAILED(m_pVB->Lock(0, 0, (void**)&pVertex, 0)))
		return 1;

	for (size_t k = 0; k < m_dwAttCount; ++k)
	{
		int nStartIndex = m_AttRange[k].VertexStart;
		int nCount = m_AttRange[k].VertexCount;

		switch (cMulti)
		{
		case 1:
			for (int j = m_AttRange[k].VertexStart; j < nCount + nStartIndex; ++j)
			{
				pVertex[j].tu2 = pVertex[j].tu1 + fProgress;
				pVertex[j].tv2 = pVertex[j].tv1 + fProgress;
			}
			break;
		case 2:
			for (int l = m_AttRange[k].VertexStart; l < nCount + nStartIndex; ++l)
				pVertex[l].tu2 = pVertex[l].tu1 + fProgress;
			break;
		case 3:
			for (int m = m_AttRange[k].VertexStart; m < nCount + nStartIndex; ++m)
				pVertex[m].tv2 = pVertex[m].tv1 + fProgress;
			break;
		case 4:
			for (int n = m_AttRange[k].VertexStart; n < nCount + nStartIndex; ++n)
				pVertex[n].tu2 = pVertex[n].tu1 - fProgress;
			break;
		case 5:
			for (int ii = m_AttRange[k].VertexStart; ii < nCount + nStartIndex; ++ii)
				pVertex[ii].tv2 = pVertex[ii].tv1 - fProgress;
			break;
		case 6:
			if (m_bFirst == 1)
			{
				for (int jj = m_AttRange[k].VertexStart; jj < nCount + nStartIndex; ++jj)
					pVertex[jj].tv2 = pVertex[jj].tv1;
				m_bFirst = 0;
			}
			else
			{
				for (int kk = m_AttRange[k].VertexStart; kk < nCount + nStartIndex; ++kk)
					pVertex[kk].tv1 = pVertex[kk].tv2 - fProgress;
			}
			break;
		case 7:
			for (int ll = m_AttRange[k].VertexStart; ll < nCount + nStartIndex; ++ll)
				pVertex[ll].tu1 = pVertex[ll].tu2 - (float)(fProgress * 3.0);
			break;
		case 100:
			for (int mm = m_AttRange[k].VertexStart; mm < nCount + nStartIndex; ++mm)
				pVertex[mm].tu1 = pVertex[mm].tu2 - (float)(fProgress * 2.0);
			break;
		case 101:
			if (m_bFirst == 1)
			{
				for (int nn = m_AttRange[k].VertexStart; nn < nCount + nStartIndex; ++nn)
					pVertex[nn].tv2 = pVertex[nn].tv1;
				m_bFirst = 0;
			}
			else
			{
				for (int i1 = m_AttRange[k].VertexStart; i1 < nCount + nStartIndex; ++i1)
				{
					pVertex[i1].tv1 = pVertex[i1].tv2 - fProgress;
					pVertex[i1].tu1 = pVertex[i1].tu2 + fProgress;
				}
			}
			break;
		}
	}

	if (FAILED(m_pVB->Unlock()))
		return 1;

	return RenderDraw(nTexOffset);
}

int TMMesh::Render(float fX, float fY, float fZ, float fAngle, float fAngle2, float fAngle3, char cMulti, int nTexOffset)
{
	D3DXMATRIX mat{};
	D3DXMATRIX matPosition{};
	D3DXMATRIX matScale{};

	SetTexture(nTexOffset + m_nTextureIndex[0]);

	D3DXMatrixScaling(&matScale, m_fScaleH, m_fScaleV, m_fScaleH);
	D3DXMatrixTranslation(&matPosition, fX, fY, fZ);
	D3DXMatrixRotationYawPitchRoll(&mat, fAngle, fAngle2 + -D3DXToRadian(90), fAngle3);
	D3DXMatrixMultiply(&mat, &g_pDevice->m_matWorld, &mat);
	D3DXMatrixMultiply(&mat, &mat, &matScale);
	D3DXMatrixMultiply(&mat, &mat, &matPosition);

	// In the original ER shows 256 value but this macro do the job according to arg
	g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &mat);

	Render(cMulti, nTexOffset);
	return 1;
}

int TMMesh::Render(float fX, float fY, float fZ, D3DXQUATERNION quat, char cMulti, int nTexOffset)
{
	D3DXMATRIX mat{};
	D3DXMATRIX matPosition{};
	D3DXMATRIX matScale{};
	D3DXMATRIX matQuat{};

	SetTexture(nTexOffset + m_nTextureIndex[0]);

	D3DXMatrixScaling(&matScale, m_fScaleH, m_fScaleV, m_fScaleH);
	D3DXMatrixTranslation(&matPosition, fX, fY, fZ);
	D3DXMatrixRotationQuaternion(&matQuat, &quat);
	D3DXMatrixMultiply(&mat, &matQuat, &g_pDevice->m_matWorld);
	D3DXMatrixMultiply(&mat, &mat, &matScale);
	D3DXMatrixMultiply(&mat, &mat, &matPosition);

	// In the original ER shows 256 value but this macro do the job according to arg
	g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &mat);

	Render(cMulti, nTexOffset);
	return 1;
}

void TMMesh::SetTexture(int nIndex)
{
	IDirect3DBaseTexture9* texture = nullptr;
	if (m_bEffect == 1)
		texture = g_pTextureManager->GetEffectTexture(nIndex, m_dwShowTime <= 5000u ? 5000u : m_dwShowTime);
	else
		texture = g_pTextureManager->GetModelTexture(nIndex, m_dwShowTime <= 10000 ? 10000u : m_dwShowTime);

	g_pDevice->SetTexture(0, texture);
}

HRESULT TMMesh::RenderDraw(int nTexOffset)
{
	for (size_t i = 0; i < m_dwAttCount; ++i)
	{
		SetTexture(nTexOffset + m_nTextureIndex[i]);

		if (FAILED(g_pDevice->m_pd3dDevice->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,
			0,
			m_AttRange[i].VertexStart,
			m_AttRange[i].VertexCount,
			3 * m_AttRange[i].FaceStart,
			m_AttRange[i].FaceCount)))
			return 0;
	}

	return 1;
}

int TMMesh::RenderPick(float fX, float fY, float fZ, float fAngle, float fAngle2, float fAngle3, float cMulti, int nTexOffset)
{
	SetTexture(nTexOffset + m_nTextureIndex[0]);

	D3DMATERIAL9 bmaterials{};
	g_pDevice->m_pd3dDevice->GetMaterial(&bmaterials);

	D3DCOLORVALUE color{};
	color.r = 1.0f;
	color.g = 1.0f;
	color.b = 1.0f;

	D3DMATERIAL9 materials{};
	materials.Emissive.r = 1.0f;
	materials.Emissive.g = 1.0f;
	materials.Emissive.b = 1.0f;
	materials.Diffuse.r = 1.0f;
	materials.Diffuse.g = 1.0f;
	materials.Diffuse.b = 1.0f;
	materials.Diffuse.a = color.a;
	materials.Specular.r = 1.0f;
	materials.Specular.g = 1.0f;
	materials.Specular.b = 1.0f;
	materials.Specular.a = color.a;
	materials.Power = 0.0f;

	g_pDevice->m_pd3dDevice->SetMaterial(&materials);

	D3DXMATRIX matPosition{};
	D3DXMATRIX matScale{};
	D3DXMATRIX matQuat{};
	D3DXMATRIX mat{};

	D3DXMatrixScaling(&matScale, m_fScaleH, m_fScaleV, m_fScaleH);
	D3DXMatrixTranslation(&matPosition, fX, fY, fZ);
	D3DXMatrixRotationYawPitchRoll(&mat, fAngle, fAngle2 + -1.5707964f, fAngle3);
	D3DXMatrixMultiply(&mat, &g_pDevice->m_matWorld, &mat);
	D3DXMatrixMultiply(&mat, &mat, &matScale);
	D3DXMatrixMultiply(&mat, &mat, &matPosition);

	g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &mat);

	Render((char)cMulti, nTexOffset);

	g_pDevice->m_pd3dDevice->SetMaterial(&bmaterials);
	return 1;
}	


int TMMesh::RenderForUI(int nX, int nY, float fAngle, float fScale, DWORD dwColor, int nMultiTex, int nTexOffset, short sLegend)
{
	// WYD 7.48 Ghidra FUN_004BBCB5 uses 6.68 for the regular viewport and
	// 6.26 only below the 1.26 aspect threshold. These camera constants keep
	// legacy 3D inventory meshes centered in the native FieldScene2 cells.
	float fWidthRatio = 6.68f;
	if (((float)g_pDevice->m_viewport.Width / (float)g_pDevice->m_viewport.Height) < 1.26f)
		fWidthRatio = 6.26f;

	float fX = (float)nX / (float)((float)g_pDevice->m_viewport.Width / fWidthRatio);
	float fY = (float)nY / (float)((float)g_pDevice->m_viewport.Height / 4.96f) - 2.48f;

	fX = fX - (float)(fWidthRatio / 2.0f);
	float fYaw = atan2f(fX, 50.0f);
	float fPitch = atan2f(fY, 50.0f);

	D3DXMATRIX matScale;
	D3DXMATRIX matPosition;
	D3DXMATRIX mat;

	D3DXMatrixScaling(&matScale, 0.51999998f * fScale, 0.51999998f * fScale, 0.51999998f * fScale);
	D3DXMatrixTranslation(&matPosition,
		-fX - ((m_vecCenter.x * 0.51999998f) * fScale),
		-fY - ((m_vecCenter.z * 0.51999998f) * fScale),
		-(m_vecCenter.y * 0.51999998f) * fScale);

	D3DXMatrixRotationYawPitchRoll(&mat, fYaw + fAngle, fPitch + -1.5707964f, 0.0);
	D3DXMatrixMultiply(&mat, &g_pDevice->m_matWorld, &mat);
	D3DXMatrixMultiply(&mat, &mat, &matScale);
	D3DXMatrixMultiply(&mat, &mat, &matPosition);

	g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &mat);

	const int nFirstTexture = FirstModelTexture();
	char cAlpha = nFirstTexture >= 0 ? g_pTextureManager->m_stModelTextureList[nFirstTexture].cAlpha : 'N';

	D3DCOLORVALUE color{};
	color.r = (float)((dwColor >> 16) & 0xFF) / 256.0f;
	color.g = (float)((unsigned short)dwColor >> 8) / 256.0f;
	color.b = (float)((unsigned char)dwColor) / 256.0f;

	D3DMATERIAL9 materials{};
	materials.Emissive.r = (color.r * 0.30000001f) + 0.1f;
	materials.Emissive.g = (color.r * 0.30000001f) + 0.1f;
	materials.Emissive.b = (color.r * 0.30000001f) + 0.1f;
	materials.Diffuse.r = color.r;
	materials.Diffuse.g = color.g;
	materials.Diffuse.b = (float)((unsigned char)dwColor) / 256.0f;
	materials.Diffuse.a = color.a;
	materials.Specular.r = color.r;
	materials.Specular.g = color.g;
	materials.Specular.b = (float)((unsigned char)dwColor) / 256.0f;
	materials.Specular.a = color.a;
	materials.Power = 0;

	g_pDevice->m_pd3dDevice->SetMaterial(&materials);

	if (nMultiTex <= 0 && (sLegend < 4 || sLegend > 8))
	{
		// FUN_004BBCB5 draws an unrefined icon with Render(0, offset) and the
		// caller's alpha states unchanged.
		Render(0, nTexOffset);
		return 1;
	}

	g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_TEXCOORDINDEX, 1);
	int dyePaletteIndex = -1;

	if (sLegend == 4)
	{
		g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nMultiTex + 179, 5000));
		g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400) ? 4 : 5);
		g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
	}
	else if (sLegend > 4 && sLegend <= 8)
	{
		int texSum = 153;
		if (sLegend == 6)
			texSum = 166;
		if (sLegend == 7)
			texSum = 246;
		if (sLegend == 8)
			texSum = 260;

		g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nMultiTex + texSum, 5000));
		g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
		g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400) ? 7 : 11);
	}
	else if (sLegend >= 9 && sLegend <= 12)
	{
		const int texSum = refinement_glow::TextureBase(sLegend);

		g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nMultiTex + texSum, 5000));

		if (cAlpha != 67 || g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
		{
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, 2);
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, 1);
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAARG1, 2);
		}
		else
		{
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG1, 1);
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, 2);
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 18);
		}

		if (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400)
		{
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 7);
		}
		else
		{
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, nMultiTex < 7 ? 4 : 5);
			if (cAlpha != 67)
				g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 11);
		}
	}
	else if (sLegend >= 116 && sLegend <= 125)
	{
		int texSum = 153;
		if (sLegend == 116)
			texSum = 275;
		if (sLegend == 117)
			texSum = 288;
		if (sLegend == 118)
			texSum = 301;
		if (sLegend == 119)
			texSum = 314;
		if (sLegend == 120)
			texSum = 327;
		if (sLegend == 121)
			texSum = 340;
		if (sLegend == 122)
			texSum = 353;
		if (sLegend == 123)
			texSum = 366;
		if (sLegend == 124)
			texSum = 425;
		if (sLegend == 125)
			texSum = 392;
		dyePaletteIndex = texSum;

		g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(nMultiTex + texSum - 1, 5000));

		if (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400 || g_pDevice->m_bTNT)
		{
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 7);
		}
		else
		{
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, 1);
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 24);
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 6);
		}
	}
	else
	{
		const bool legacyHardware = g_pDevice->m_bVoodoo || g_pDevice->m_bIntel
			|| g_pDevice->m_bG400 || g_pDevice->m_bTNT;
		g_pDevice->SetTexture(1, g_pTextureManager->GetEffectTexture(
			native_mount_glow::InventoryTextureIndex(nMultiTex, legacyHardware), 5000));

		if (g_pDevice->m_bVoodoo || g_pDevice->m_bIntel || g_pDevice->m_bG400 || g_pDevice->m_bTNT)
		{
			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
			g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 7);
		}
		else
		{
			if (cAlpha == 67)
			{
				g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG1, 1);
				g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLORARG2, 2);
				g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 18);
			}
			else
			{
				g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, 2);
				g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAOP, 1);
				g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_ALPHAARG1, 2);
			}

			g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, nMultiTex < 7 ? 4 : 5);
			if (cAlpha != 67)
				g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 11);

			g_pDevice->SetRenderState(D3DRENDERSTATETYPE::D3DRS_FOGVERTEXMODE, 0);
		}
	}

	bool renderedWithDyeShader = false;
	if (dyePaletteIndex >= 0 && nMultiTex > 0 && m_dwFVF == 530)
	{
		// The inventory icon should use the equipped armor's matte dye and
		// narrow grade streak instead of the darker fixed-function DOTPRODUCT3.
		dye_pixel_shader::Binding dye(g_pDevice->m_pd3dDevice,
			g_pDevice->m_dyePixelShader, sLegend, nMultiTex, true);
		if (dye.Active())
		{
			const int maskIndex = sLegend == 124 ? 314 : dyePaletteIndex;
			dye_pixel_shader::TextureBinding textures(g_pDevice->m_pd3dDevice,
				g_pTextureManager->GetEffectTexture(maskIndex, 5000),
				g_pTextureManager->GetEffectTexture(maskIndex + nMultiTex - 1, 5000),
				g_pTextureManager->GetEffectTexture(dyePaletteIndex, 5000));
			if (textures.Active())
			{
				Render(1, nTexOffset);
				renderedWithDyeShader = true;
			}
		}
	}
	if (!renderedWithDyeShader)
		Render(1, nTexOffset);

	g_pDevice->SetTexture(1, nullptr);
	g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_TEXCOORDINDEX, 1);
	g_pDevice->SetTextureStageState(0, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 4);
	g_pDevice->SetTextureStageState(1, D3DTEXTURESTAGESTATETYPE::D3DTSS_COLOROP, 1);
	return 1;
}

bool TMMesh::UvAnimatable()
{
	if (!msa_load::UvAnimatable(m_pVB != nullptr, m_dwFVF, m_sizeVertex, m_dwAttCount))
		return false;
	D3DVERTEXBUFFER_DESC desc{};
	if (FAILED(m_pVB->GetDesc(&desc)))
		return false;
	// Index limits are not needed here: only vertex ranges are rewritten.
	return msa_load::RangesFit(reinterpret_cast<const msa_load::Range*>(m_AttRange), m_dwAttCount,
		desc.Size / m_sizeVertex, 0xFFFFFFFFu);
}

int TMMesh::FirstModelTexture() const
{
	return m_nTextureIndex[0] >= 0 && m_nTextureIndex[0] < MAX_MODEL_TEXTURE ? m_nTextureIndex[0] : -1;
}

void TMMesh::ResetToEmpty()
{
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);
	m_dwFVF = 0;
	m_sizeVertex = 0;
	m_dwAttCount = 0;
	memset(m_AttRange, 0, sizeof(m_AttRange));
	memset(m_nTextureIndex, 0, sizeof(m_nTextureIndex));
}

namespace {
// Resolves one texture stem through the effect or model table.
int FindMeshTexture(bool effect, const char* stem, char (&path)[64])
{
	sprintf_s(path, effect ? EffectTexture_FileFormat_Path : MeshTexture_FileFormat_Path, stem);
	return effect ? g_pTextureManager->GetEffectTextureIndex(path)
		: g_pTextureManager->GetModelTextureIndex(path);
}
}

int TMMesh::LoadMsa(const char* szFileName)
{
	static_assert(sizeof(D3DXATTRIBUTERANGE) == sizeof(msa_load::Range), "attribute range layout");
	static_assert(sizeof(RDVERTEX2) == msa_load::UvAnimatedStride, "UV animation vertex layout");
	static_assert(sizeof(m_AttRange) / sizeof(m_AttRange[0]) == msa_load::MaxAttributes, "attribute capacity");

	if (!szFileName || !szFileName[0])
		return 0;

	FILE* handle = asset_file::OpenFile("msa", szFileName);
	if (!handle)
	{
		LOG_WRITELOG("ReadingError : %s\n", szFileName);
		return 0;
	}
	std::unique_ptr<FILE, decltype(&fclose)> input(handle, &fclose);
	const auto fail = [&](const char* reason) {
		LOG_WRITELOG("Invalid mesh %s: %s\r\n", szFileName, reason);
		ResetToEmpty();
		return 0;
	};

	if (_fseeki64(handle, 0, SEEK_END) != 0)
		return fail("size");
	const __int64 fileBytes = _ftelli64(handle);
	if (fileBytes < 0 || _fseeki64(handle, 0, SEEK_SET) != 0)
		return fail("size");
	const auto remaining = [&]() -> std::uint64_t {
		const __int64 position = _ftelli64(handle);
		return position < 0 || position > fileBytes ? 0 : static_cast<std::uint64_t>(fileBytes - position);
	};

	std::uint32_t header[3]{};
	if (fread(header, sizeof(header), 1, handle) != 1)
		return fail("header");
	if (!msa_load::ValidHeader(header[0], header[1], header[2]))
		return fail("header limits");
	m_dwFVF = header[0];
	m_sizeVertex = header[1];
	m_dwAttCount = header[2];
	if (fread(m_AttRange, sizeof(D3DXATTRIBUTERANGE), m_dwAttCount, handle) != m_dwAttCount)
		return fail("attribute ranges");

	// MeshList names are "<folder>\\<file>.msa"; the folder selects the table.
	const size_t len = strlen(szFileName);
	char szPathName[24] = { 0, };
	size_t nSeparator = 0;
	for (; nSeparator < len && szFileName[nSeparator] != '\\'; ++nSeparator)
	{
		if (nSeparator + 1 >= sizeof(szPathName))
			return fail("folder name");
		szPathName[nSeparator] = szFileName[nSeparator];
	}
	if (nSeparator + 2 > len)
		return fail("file name");

	char szTmpFileName[24] = { 0, };
	if (strncpy_s(szTmpFileName, &szFileName[nSeparator + 2], _TRUNCATE) != 0)
		return fail("file name length");
	const bool bEffectFolder = !_stricmp(szPathName, "effect");

	char szTextureName[24] = { 0, };
	char szTemp[24] = { 0, };
	char szTex[64] = { 0, };

	for (size_t i = 0; i < m_dwAttCount; ++i)
	{
		memset(szTemp, 0, sizeof(szTemp));
		if (fread(szTemp, msa_load::TextureFieldBytes, 1, handle) != 1)
			return fail("texture name");

		if (szTmpFileName[0] != 'a' || szTmpFileName[1] != 'a')
		{
			// Keep the text after the last backslash, then its stem.
			const char* name = szTemp;
			for (int nIndex = static_cast<int>(strlen(szTemp)) - 1; nIndex > 0; --nIndex)
			{
				if (szTemp[nIndex] == '\\')
				{
					name = &szTemp[nIndex + 1];
					break;
				}
			}
			if (!msa_load::CopyStem(szTextureName, sizeof(szTextureName), name, sizeof(szTemp)))
				return fail("texture name length");
		}
		else if (!msa_load::CopyStem(szTextureName, sizeof(szTextureName), szTmpFileName, sizeof(szTmpFileName)))
			return fail("texture name length");

		m_nTextureIndex[i] = FindMeshTexture(bEffectFolder, szTextureName, szTex);
		if (m_nTextureIndex[i] == -1)
		{
			// Fall back to the mesh file stem.
			if (!msa_load::CopyStem(szTextureName, sizeof(szTextureName), szTmpFileName, sizeof(szTmpFileName)))
				return fail("texture name length");
			m_nTextureIndex[i] = FindMeshTexture(bEffectFolder, szTextureName, szTex);
			if (m_nTextureIndex[i] == -1)
			{
				LOG_WRITELOG("%s , %s \r\n", szFileName, szTex);
				ResetToEmpty();
				return 0;
			}
		}
	}

	std::uint32_t indexBytes = 0;
	if (fread(&indexBytes, sizeof(indexBytes), 1, handle) != 1 ||
		!msa_load::ValidIndexBytes(indexBytes, remaining()))
		return fail("index buffer size");
	if (FAILED(g_pDevice->m_pd3dDevice->CreateIndexBuffer(
		indexBytes,
		8,
		D3DFORMAT::D3DFMT_INDEX16,
		D3DPOOL::D3DPOOL_MANAGED,
		&m_pIB,
		nullptr)) || !m_pIB)
		return fail("index buffer");

	void* pIndex = nullptr;
	if (FAILED(m_pIB->Lock(0, 0, &pIndex, 0)) || !pIndex)
		return fail("index lock");
	const bool indicesRead = fread(pIndex, indexBytes, 1, handle) == 1;
	if (FAILED(m_pIB->Unlock()) || !indicesRead)
		return fail("index data");

	std::uint32_t vertexBytes = 0;
	msa_load::VertexPlan plan;
	if (fread(&vertexBytes, sizeof(vertexBytes), 1, handle) != 1 ||
		!msa_load::PlanVertices(m_dwFVF, m_sizeVertex, vertexBytes, remaining(), plan))
		return fail("vertex buffer size");
	if (!msa_load::RangesFit(reinterpret_cast<const msa_load::Range*>(m_AttRange), m_dwAttCount,
		plan.vertices, indexBytes / sizeof(std::uint16_t)))
		return fail("attribute range");

	const int nFloatCount = static_cast<int>(m_sizeVertex >> 2);
	const int nNumVertex = static_cast<int>(plan.vertices);
	const unsigned int storedStride = m_sizeVertex;
	m_dwFVF = msa_load::LoadedFvf(m_dwFVF);
	if (FAILED(g_pDevice->m_pd3dDevice->CreateVertexBuffer(
		plan.bufferBytes,
		8,
		m_dwFVF,
		D3DPOOL::D3DPOOL_MANAGED,
		&m_pVB,
		nullptr)) || !m_pVB)
		return fail("vertex buffer");

	float* pVertex = nullptr;
	if (FAILED(m_pVB->Lock(0, 0, (void**)&pVertex, 0)) || !pVertex)
		return fail("vertex lock");

	bool verticesRead = true;
	if (!plan.addUv)
	{
		verticesRead = fread(pVertex, plan.readBytes, 1u, handle) == 1;
	}
	else
	{
		char* pVertexB = (char*)pVertex;
		for (int i = 0; i < nNumVertex && verticesRead; ++i)
			verticesRead = fread(&pVertexB[i * (storedStride + msa_load::AddedUvBytes)], storedStride, 1u, handle) == 1;
		m_sizeVertex = plan.loadedStride;
	}
	if (!verticesRead)
	{
		m_pVB->Unlock();
		return fail("vertex data");
	}

	int nAddedFloatNum = 0;
	if (m_dwFVF != msa_load::ColorFvf)
		nAddedFloatNum = 2;

	// The native bounds scan seeds from the second vertex.
	m_fMinX = pVertex[nAddedFloatNum + nFloatCount];
	m_fMaxX = m_fMinX;
	m_fMinY = *(&pVertex[nAddedFloatNum + 1] + nFloatCount);
	m_fMaxY = m_fMinY;
	m_fMinZ = *(&pVertex[nAddedFloatNum + 2] + nFloatCount);
	m_fMaxZ = m_fMinZ;

	for (int i = 0; i < nNumVertex - 1; ++i)
	{
		if (m_dwFVF == msa_load::UvAnimatedFvf)
		{
			pVertex[i * (nAddedFloatNum + nFloatCount) + 8] = pVertex[i * (nAddedFloatNum + nFloatCount) + 6];
			pVertex[i * (nAddedFloatNum + nFloatCount) + 9] = pVertex[i * (nAddedFloatNum + nFloatCount) + 7];
		}

		if (pVertex[i * (nAddedFloatNum + nFloatCount)] > m_fMaxX)
			m_fMaxX = pVertex[i * (nAddedFloatNum + nFloatCount)];
		if (m_fMinX > pVertex[i * (nAddedFloatNum + nFloatCount)])
			m_fMinX = pVertex[i * (nAddedFloatNum + nFloatCount)];
		if (pVertex[i * (nAddedFloatNum + nFloatCount) + 1] > m_fMaxY)
			m_fMaxY = pVertex[i * (nAddedFloatNum + nFloatCount) + 1];
		if (m_fMinY > pVertex[i * (nAddedFloatNum + nFloatCount) + 1])
			m_fMinY = pVertex[i * (nAddedFloatNum + nFloatCount) + 1];
		if (pVertex[i * (nAddedFloatNum + nFloatCount) + 2] > m_fMaxZ)
			m_fMaxZ = pVertex[i * (nAddedFloatNum + nFloatCount) + 2];
		if (m_fMinZ > pVertex[i * (nAddedFloatNum + nFloatCount) + 2])
			m_fMinZ = pVertex[i * (nAddedFloatNum + nFloatCount) + 2];
	}

	if (fabsf(m_fMaxX) > m_fRadius)
		m_fRadius = fabsf(m_fMaxX);
	if (fabsf(m_fMinX) > m_fRadius)
		m_fRadius = fabsf(m_fMinX);

	if (fabsf(m_fMaxY) > m_fRadius)
		m_fRadius = fabsf(m_fMaxY);
	if (fabsf(m_fMinY) > m_fRadius)
		m_fRadius = fabsf(m_fMinY);

	if (fabsf(m_fMaxZ) > m_fRadius)
		m_fRadius = fabsf(m_fMaxZ);
	if (fabsf(m_fMinZ) > m_fRadius)
		m_fRadius = fabsf(m_fMinZ);

	m_vecCenter = TMVector3((float)(m_fMaxX + m_fMinX) * 0.5f,
		(float)(m_fMaxY + m_fMinY) * 0.5f,
		(float)(m_fMaxZ + m_fMinZ) * 0.5f);

	if (FAILED(m_pVB->Unlock()))
		return fail("vertex unlock");

	return 1;
}
