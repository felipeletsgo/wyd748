#include "pch.h"
// RenderDevice split by responsibility; device lifecycle stays in RenderDevice.cpp.
#include "RenderDevice.h"
#include "TMGlobal.h"
#include "TMFieldScene.h"
#include "TMCamera.h"

void RenderDevice::SetLight()
{
	for (int i = 0; i < 2; i++)
	{
		if(FAILED(m_pd3dDevice->SetLight(i, &m_light[i])))
			return;

		if (FAILED(m_pd3dDevice->LightEnable(i, 1)))
			return;
	}
}

void RenderDevice::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	if ((int)State >= 0 && (int)State < 256 && m_dwRenderStateList[State] != Value)
	{
		m_dwRenderStateList[State] = Value;
		m_pd3dDevice->SetRenderState(State, Value);
	}
}

void RenderDevice::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE type, DWORD Value)
{
	if (dwStage >= 8)
		return;

	if (type >= 0 && type < 29 && m_dwTextureStageStateList[dwStage][type] != Value)
	{
		m_dwTextureStageStateList[dwStage][type] = Value;
		m_pd3dDevice->SetTextureStageState(dwStage, type, Value);
	}
}

void RenderDevice::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	if (Stage >= 8)
		return;

	if (m_pTexture[Stage] != pTexture)
	{
		m_pTexture[Stage] = pTexture;
		m_pd3dDevice->SetTexture(Stage, pTexture);
	}
}

void RenderDevice::SetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE State, DWORD Value)
{
	if (dwStage >= 8)
		return;

	if (State >= 0 && State < 14 && m_dwSamplerStateList[dwStage][State] != Value)
	{
		m_dwSamplerStateList[dwStage][State] = Value;
		m_pd3dDevice->SetSamplerState(dwStage, State, Value);
	}
}

void RenderDevice::SetGamma()
{
	if (RenderDevice::m_nBright != 50)
	{
		static D3DGAMMARAMP gamma;
		memset(&gamma, 0, sizeof(gamma));

		m_pd3dDevice->GetGammaRamp(1, &gamma);

		for (int i = 0; i < 256; i++)
		{
			int nVal = static_cast<int>(((((float)RenderDevice::m_nBright * 0.02f) * (float)i) * 256.0f));
			if (nVal > 0xFFFF)
				nVal = 0xFFFF;

			gamma.red[i] = nVal;
			gamma.green[i] = nVal;
			gamma.blue[i] = nVal;
		}

		m_pd3dDevice->SetGammaRamp(0, 0, &gamma);

		if (m_d3dpp.Windowed)
		{
			HDC hdc = GetDC(g_pApp->m_hWnd);
			SetDeviceGammaRamp(hdc, &gamma);
			ReleaseDC(g_pApp->m_hWnd, hdc);
		}
	}
}

int RenderDevice::InitializeRenderingState()
{
	D3DXMatrixIdentity(&m_matWorld);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_matWorld);

	memset(m_dwRenderStateList, 0, sizeof(m_dwRenderStateList));

	for (int nState = 0; nState < 256; ++nState)
	{
		m_dwRenderStateList[nState] = -1;
	}

	for (int nStage = 0; nStage < 8; ++nStage)
	{
		for (int i = 0; i < 14; ++i)
		{
			m_dwSamplerStateList[nStage][i] = -1;
		}
	}

	for (int nStage = 0; nStage < 8; ++nStage)
	{
		for (int j = 0; j < 29; ++j)
			m_dwTextureStageStateList[nStage][j] = -1;
	}

	return 1;
}

int RenderDevice::SetRenderStateBlock(int nIndex)
{
	if (nIndex == 0)
	{
		SetRenderState(D3DRS_ZENABLE, 1u);
		SetRenderState(D3DRS_FOGENABLE, 0);
		SetRenderState(D3DRS_LIGHTING, 0);
		SetRenderState(D3DRS_SPECULARENABLE, 1u);
		SetRenderState(D3DRS_CULLMODE, 3u);
		SetRenderState(D3DRS_SHADEMODE, 2u);
		SetRenderState(D3DRS_AMBIENT, 0x0AAAAAA);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 7u);
		SetRenderState(D3DRS_COLORVERTEX, 1u);
		SetRenderState(D3DRS_DITHERENABLE, 1u);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
		SetTextureStageState(0, D3DTSS_COLOROP, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 1u);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 1u);
		SetSamplerState(0, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 1u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		m_pd3dDevice->SetFVF(322);
	}
	else if (nIndex == 1)
	{
		m_light[0].Diffuse = m_colorLight;
		m_light[0].Specular = m_colorLight;
		m_light[1].Diffuse = m_colorBackLight;
		m_light[1].Specular = m_colorBackLight;

		if (g_pCurrentScene != nullptr && g_pCurrentScene->GetSceneType() == ESCENE_TYPE::ESCENE_FIELD)
		{
			if (static_cast<TMFieldScene*>(g_pCurrentScene)->m_bShowBoss == 1)
			{
				m_light[0].Diffuse.a = 0.0;
				m_light[0].Diffuse.b = 0.0;
				m_light[0].Diffuse.g = 0.0;
				m_light[0].Diffuse.r = 0.0;
				m_light[0].Specular.a = 0.0;
				m_light[0].Specular.b = 0.0;
				m_light[0].Specular.g = 0.0;
				m_light[0].Specular.r = 0.0;
			}
			static_cast<TMFieldScene*>(g_pCurrentScene)->m_bShowBoss = 0;
		}

		m_light[1].Diffuse = m_colorBackLight;
		m_light[1].Specular = m_colorBackLight;
		m_light[0].Direction = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
		m_light[1].Direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

		for (int i = 0; i < 2; ++i)
		{
			if (FAILED(m_pd3dDevice->SetLight(i, &m_light[i])))
				return 0;

			if (FAILED(m_pd3dDevice->LightEnable(i, 1)))
				return 0;
		}

		D3DXVECTOR4 vLightDir;
		TMCamera* pCamera = g_pObjectManager->m_pCamera;

		if (RenderDevice::m_bDungeon != 0 && RenderDevice::m_bDungeon != 4)
		{
			vLightDir = D3DXVECTOR4(
				pCamera->m_vecCamDir.x - m_light[0].Direction.x,
				0.5f,
				(pCamera->m_vecCamDir.z + m_light[0].Direction.z) - 0.3f,
				0.0f);
		}
		else
		{
			vLightDir = D3DXVECTOR4(
				-m_light[0].Direction.x,
				0.5f,
				m_light[0].Direction.z,
				0.0f);
		}

		m_pd3dDevice->SetVertexShaderConstantF(1, (const float*)&vLightDir, 1);

		if (g_pDevice->m_iVGAID == 1)
		{
			if (g_pDevice->m_dwBitCount == 32)
				SetRenderState(D3DRS_ALPHAREF, 0xFF000000);
			else
				SetRenderState(D3DRS_ALPHAREF, 0xF000);
		}
		else if (g_pDevice->m_dwBitCount == 32)
		{
			SetRenderState(D3DRS_ALPHAREF, 0xDD);
		}
		else
		{
			SetRenderState(D3DRS_ALPHAREF, 0xD);
		}

		SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		SetRenderState(D3DRS_ALPHATESTENABLE, 1);
		SetRenderState(D3DRS_ALPHAFUNC, 7);
		SetRenderState(D3DRS_AMBIENT, 0x33FFFFFFu);
		SetRenderState(D3DRS_ZENABLE, 1);
		SetRenderState(D3DRS_ZFUNC, 4);
		SetRenderState(D3DRS_ZWRITEENABLE, 1);
		SetRenderState(D3DRS_LIGHTING, 1);
		SetRenderState(D3DRS_SPECULARENABLE, 0);
		SetRenderState(D3DRS_CULLMODE, 3);
		SetRenderState(D3DRS_SHADEMODE, 2);
		SetRenderState(D3DRS_COLORVERTEX, 1);
		SetRenderState(D3DRS_NORMALIZENORMALS, 1);
		SetRenderState(D3DRS_DITHERENABLE, 1);
		SetRenderState(D3DRS_SRCBLEND, 2);
		SetRenderState(D3DRS_DESTBLEND, 6);
		SetRenderState(D3DRS_FOGENABLE, m_bFog);
		SetRenderState(D3DRS_RANGEFOGENABLE, 0);

		m_dwActualClearColor = m_dwClearColor;
		float fstart = (float)(m_fFogStart + g_pApp->m_pObjectManager->m_pCamera->m_fSightLength) - 8.0f;
		float fend = (float)(m_fFogEnd + g_pApp->m_pObjectManager->m_pCamera->m_fSightLength) - 15.0f;

		SetRenderState(D3DRS_FOGCOLOR, m_dwClearColor);
		SetRenderState(D3DRS_FOGVERTEXMODE, 3);
		SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fstart));
		SetRenderState(D3DRS_FOGEND, *(DWORD*)(&fend));
		SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 1);
		SetRenderState(D3DRS_SPECULARMATERIALSOURCE, 2);
		SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 1);
		SetRenderState(D3DRS_SPECULARMATERIALSOURCE, 1);
		SetTextureStageState(0, D3DTSS_COLOROP, 4u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 0);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 1u);
		SetSamplerState(0, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 2u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
		SetTextureStageState(1u, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(1u, D3DTSS_COLORARG2, 1u);
		SetTextureStageState(1u, D3DTSS_ALPHAOP, 2u);
		SetTextureStageState(1u, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(1u, D3DTSS_ALPHAARG2, 1u);
		SetSamplerState(1u, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(1u, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(1u, D3DSAMP_MIPFILTER, 0);
		SetTextureStageState(1u, D3DTSS_TEXCOORDINDEX, 1u);
		SetTextureStageState(2u, D3DTSS_COLOROP, 1u);
	}
	else if (nIndex == 2)
	{
		SetRenderState(D3DRS_ZENABLE, 0);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
		SetRenderState(D3DRS_ALPHAREF, 8u);
		SetRenderState(D3DRS_ALPHAFUNC, 7u);
		SetRenderState(D3DRS_FILLMODE, 3u);
		SetRenderState(D3DRS_CULLMODE, 3u);
		SetRenderState(D3DRS_STENCILENABLE, 0);
		SetRenderState(D3DRS_CLIPPING, 1u);
		SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, 0);
		SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, 0);
		SetRenderState(D3DRS_VERTEXBLEND, 0);
		SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, 0);
		SetRenderState(D3DRS_FOGENABLE, 0);
		SetTextureStageState(0, D3DTSS_COLOROP, 4u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 0);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
		SetSamplerState(0, D3DSAMP_MINFILTER, 1u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 1u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 1u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
		SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
	}
	else if (nIndex == 3)
	{
		SetRenderState(D3DRS_ZENABLE, 0);
		SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
		SetRenderState(D3DRS_SRCBLEND, 5u);
		SetRenderState(D3DRS_DESTBLEND, 6u);
		SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
		SetRenderState(D3DRS_ALPHAREF, 8u);
		SetRenderState(D3DRS_ALPHAFUNC, 7u);
		SetRenderState(D3DRS_FILLMODE, 3u);
		SetRenderState(D3DRS_CULLMODE, 3u);
		SetRenderState(D3DRS_STENCILENABLE, 0);
		SetRenderState(D3DRS_CLIPPING, 1u);
		SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, 0);
		SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, 0);
		SetRenderState(D3DRS_VERTEXBLEND, 0);
		SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, 0);
		SetRenderState(D3DRS_FOGENABLE, 0);
		SetTextureStageState(0, D3DTSS_COLOROP, 4u);
		SetTextureStageState(0, D3DTSS_COLORARG1, 2u);
		SetTextureStageState(0, D3DTSS_COLORARG2, 0);
		SetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
		SetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
		SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
		SetSamplerState(0, D3DSAMP_MINFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MAGFILTER, 2u);
		SetSamplerState(0, D3DSAMP_MIPFILTER, 2u);
		SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
		SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		SetTextureStageState(1u, D3DTSS_COLOROP, 1u);
		SetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
	}

	return 1;
}

int RenderDevice::SetViewVector(TMVector3 ivCamera, TMVector3 ivLookat)
{
	D3DXVECTOR3 upVector(0.0f, 1.0f, 0.0f);
	m_vCamera = D3DXVECTOR3(ivCamera.x, ivCamera.y, ivCamera.z);
	m_vLookatPos = D3DXVECTOR3(ivLookat.x, ivLookat.y, ivLookat.z);

	D3DXMatrixLookAtLH(&m_matView, &m_vCamera, &m_vLookatPos, &upVector);

	return SUCCEEDED(m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView));
}

HRESULT RenderDevice::SetProjectionMatrix()
{
	D3DXMatrixPerspectiveFovLH(&m_matProj,
		RenderDevice::m_fFOVY * D3DXToRadian(180),
		(float)((float)(m_dwScreenWidth - m_nWidthShift) / (float)(m_dwScreenHeight - m_nHeightShift)) * 1.0f,
		g_ClipNear * 1.4f,
		g_ClipFar);

	HRESULT hr = m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
	D3DXMATRIX mat;
	D3DXMatrixTranspose(&mat, &m_matProj);

	m_pd3dDevice->SetVertexShaderConstantF(2, (const float*)&mat, 4);

	return SUCCEEDED(hr) ? 0 : hr;
}

int RenderDevice::SetMatrixForUI()
{
	D3DXMATRIX matUIProjection;
	// The 7.48 native UI path (FUN_00430c46) uses the viewport aspect ratio
	// verbatim; a horizontal correction factor creates radial drift across grids.
	D3DXMatrixPerspectiveFovLH(&matUIProjection, 0.1f, (float)m_d3dsdBackBuffer.Width / (float)m_d3dsdBackBuffer.Height, 10.0f, 100.0f);

	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matUIProjection);

	D3DXMATRIX matView;
	D3DXVECTOR3 upVector(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 vecCam(0.0f, 0.0f, 50.0f);
	D3DXVECTOR3 vecLookAt(0.0f, 0.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView, &vecCam, &vecLookAt, &upVector);

	m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	return 1;
}

void RenderDevice::GetPickRayVector(D3DXVECTOR3* pRickRayOrig, D3DXVECTOR3* pPickRayDir)
{
	D3DXVECTOR3 v;
	v.x = (((2.0f * g_pCursor->m_nPosX) / (float)(m_dwScreenWidth - m_nWidthShift)) - 1.0f) / m_matProj.m[0][0];
	v.y = -(((2.0f * g_pCursor->m_nPosY) / (float)(m_dwScreenHeight - m_nHeightShift)) - 1.0f) / m_matProj.m[1][1];
	v.z = 1.0f;

	D3DXMATRIX matViewInv;
	D3DXMatrixInverse(&matViewInv, 0, &m_matView);

	pPickRayDir->x = (float)((v.x * matViewInv.m[0][0]) + (v.y * matViewInv.m[1][0]) + (v.z * matViewInv.m[2][0]));
	pPickRayDir->y = (float)((v.x * matViewInv.m[0][1]) + (v.y * matViewInv.m[1][1]) + (v.z * matViewInv.m[2][1]));
	pPickRayDir->z = (float)((v.x * matViewInv.m[0][2]) + (v.y * matViewInv.m[1][2]) + (v.z * matViewInv.m[2][2]));

	pRickRayOrig->x = matViewInv.m[3][0];
	pRickRayOrig->y = matViewInv.m[3][1];
	pRickRayOrig->z = matViewInv.m[3][2];
}
