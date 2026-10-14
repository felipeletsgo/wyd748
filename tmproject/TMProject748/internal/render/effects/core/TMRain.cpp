#include "pch.h"
#include "ClientSettings.h"
#include "TMRain.h"
#include "WeatherPresentation.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "TMCamera.h"
#include "TMHuman.h"
#include "TMEffectBillBoard2.h"

TMRain::TMRain() :  
	TMEffect(),
	m_vecRainPosition{}
{
    m_dwOldServerTime = 0;
    m_bShow = m_bVisible = 0;
    m_vertex[0].position = { -0.008f, -0.2f, 0.0f };
    m_vertex[1].position = { 0.008f, -0.2f, 0.0f };
    m_vertex[2].position = { 0.008f, 0.2f, 0.0f };
    m_vertex[3].position = { -0.008f, 0.2f, 0.0f };
    m_vertex[0].tu = 0.002f;
    m_vertex[0].tv = 0.002f;
    m_vertex[1].tu = 0.98f;
    m_vertex[1].tv = 0.002f;
    m_vertex[2].tu = 0.98f;
    m_vertex[2].tv = 0.98f;
    m_vertex[3].tu = 0.002f;
    m_vertex[3].tv = 0.98f;
    m_vertex[0].diffuse = 0x33333333;
    m_vertex[1].diffuse = 0x33333333;
    m_vertex[2].diffuse = 0x33333333;
    m_vertex[3].diffuse = 0x33333333;

    m_fHalfSize = weather_area::HalfSize(g_ClientSettings.weatherArea);
    m_nDrops = weather_area::ParticleCount(weather_area::kNativeRainDrops, m_fHalfSize,
        weather_area::kMaxRainDrops);
    for (int i = 0; i < m_nDrops; ++i)
    {
        m_vecRainPosition[i].x = weather_area::RandomOffset(m_fHalfSize, rand());
        m_vecRainPosition[i].z = weather_area::RandomOffset(m_fHalfSize, rand());
        m_vecRainPosition[i].y = (float)(rand() % 10);
        m_fSpeed[i] = (float)(rand() % 3) * 0.079999998f;
        if (m_fSpeed[i] == 0.0)
            m_fSpeed[i] = 0.23999999f;
    }
}

TMRain::~TMRain()
{
    StopAmbientSound();
}

void TMRain::StopAmbientSound()
{
    if (g_pSoundManager)
    {
        auto sound = g_pSoundManager->GetSoundData(101);
        if (sound && sound->IsSoundPlaying())
            sound->Stop();
    }
}

void TMRain::SetActive(bool active)
{
    weather_presentation::SetActive(*this, active);
    if (!active)
        StopAmbientSound();
}

int TMRain::Render()
{
    // Stop audio before visual/idle early returns, including hidden weather.
    if (!m_bShow || !m_bVisible || IdleHidesAmbient() || g_bHideBackground == 1 || g_bHideEffect == 1)
    {
        StopAmbientSound();
        return 1;
    }

    GetSoundAndPlayIfNot(101, 0, 0);

    g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
    g_pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
    g_pDevice->SetRenderState(D3DRS_DESTBLEND, 2u);
    g_pDevice->SetRenderState(D3DRS_SRCBLEND, 5u);
    g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 8u);
    g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
    g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
    g_pDevice->SetRenderState(D3DRS_LIGHTING, 0);
    g_pDevice->SetRenderState(D3DRS_CULLMODE, 1u);

    TMVector2 vecCam{ 0.0f, 0.0f };
    auto* pObj = g_pObjectManager->m_pCamera->GetFocusedObject();
    if (pObj)
    {
        vecCam.x = pObj->m_vecPosition.x;
        vecCam.y = pObj->m_vecPosition.y;
    }
    else
    {
        vecCam.x = g_pObjectManager->m_pCamera->m_vecCamPos.x + 2.5f;
        vecCam.y = g_pObjectManager->m_pCamera->m_vecCamPos.y + 2.5f;
    }

    auto matView = g_pDevice->m_matView;
    TMVector3 vecPosXAxis{ g_pDevice->m_matView._11, g_pDevice->m_matView._21, g_pDevice->m_matView._31 };
    TMVector3 vecPosYAxis{ g_pDevice->m_matView._12, g_pDevice->m_matView._22, g_pDevice->m_matView._32 };
    TMVector3 vecNegXAxis{ -vecPosXAxis.x, -vecPosXAxis.y, -vecPosXAxis.z };
    TMVector3 vecNegYAxis{ -vecPosYAxis.x, -vecPosYAxis.y, -vecPosYAxis.z };

    float fSize = 0.008f;
    m_vertex[0].position.x = (float)(vecNegXAxis.x + vecPosYAxis.x) * fSize;
    m_vertex[0].position.z = (float)(vecNegXAxis.z + vecPosYAxis.z) * fSize;
    m_vertex[1].position.x = (float)(vecPosXAxis.x + vecPosYAxis.x) * fSize;
    m_vertex[1].position.z = (float)(vecPosXAxis.z + vecPosYAxis.z) * fSize;
    m_vertex[2].position.x = (float)(vecPosXAxis.x + vecNegYAxis.x) * fSize;
    m_vertex[2].position.z = (float)(vecPosXAxis.z + vecNegYAxis.z) * fSize;
    m_vertex[3].position.x = (float)(vecNegXAxis.x + vecNegYAxis.x) * fSize;
    m_vertex[3].position.z = (float)(vecNegXAxis.z + vecNegYAxis.z) * fSize;

    g_pDevice->SetTexture(0, g_pTextureManager->GetEffectTexture(9, 5000));
    // All drops outside houses go out in one triangle list in world space.
    int quads = 0;
    for (int i = 0; i < m_nDrops; ++i)
    {
        const float x = m_vecRainPosition[i].x + vecCam.x;
        const float z = m_vecRainPosition[i].z + vecCam.y;
        int bValue = g_pAttribute[weather_area::AttributeCell(z)][weather_area::AttributeCell(x)];
        if (!(bValue & 8))
            weather_area::AppendQuad(m_batch, quads++, m_vertex, x, m_vecRainPosition[i].y, z);
    }
    if (quads > 0)
    {
        D3DXMatrixIdentity(&m_matEffect);
        g_pDevice->m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_matEffect);
        g_pDevice->m_pd3dDevice->SetFVF(322);
        g_pDevice->m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, quads * 2, m_batch, 24u);
    }

    g_pDevice->SetRenderState(D3DRS_CULLMODE, 3u);
    g_pDevice->SetRenderState(D3DRS_LIGHTING, 1u);
    g_pDevice->SetRenderState(D3DRS_SRCBLEND, 2u);
    g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, 7u);
    g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1u);
    g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1u);
    g_pDevice->SetRenderState(D3DRS_DESTBLEND, 6u);
    return 1;
}

int TMRain::FrameMove(unsigned int dwServerTime)
{
    if (!m_bShow || !m_bVisible)
    {
        m_dwOldServerTime = 0;
        return 1;
    }

    float fElapsedTime = static_cast<float>(weather_presentation::Elapsed(dwServerTime, m_dwOldServerTime)) * 0.1f;
    
    TMVector2 vecCam{};
    auto pObj = g_pObjectManager->m_pCamera->GetFocusedObject();
    if (pObj)
    {
        vecCam.x = pObj->m_vecPosition.x;
        vecCam.y = pObj->m_vecPosition.y;
    }
    else
    {
        vecCam.x = g_pObjectManager->m_pCamera->m_vecCamPos.x + 2.5f;
        vecCam.y = g_pObjectManager->m_pCamera->m_vecCamPos.y + 2.5f;
    }

    float fCamHeight = 0.0f;

    if (pObj)
        fCamHeight = pObj->m_fHeight;

    for (int i = 0; i < m_nDrops; ++i)
    {
        if (m_vecRainPosition[i].y > fCamHeight)
        {
            m_vecRainPosition[i].y = m_vecRainPosition[i].y - (m_fSpeed[i] * fElapsedTime);
            continue;
        }

        if (!(rand() % 5) &&
            weather_area::InSplashArea(m_vecRainPosition[i].x, m_vecRainPosition[i].z))
        {
            int nX4 = weather_area::AttributeCell(m_vecRainPosition[i].x + vecCam.x);
            int nY4 = weather_area::AttributeCell(m_vecRainPosition[i].z + vecCam.y);
            int bValue = g_pAttribute[nY4][nX4];
            if (!(bValue & 8))
            {
                auto pEffect = new TMEffectBillBoard2(10, 700, 0.0099999998f, 0.0099999998f, 0.0099999998f, 0.00039999999f, 0);
                pEffect->m_efAlphaType = EEFFECT_ALPHATYPE::EF_BRIGHT;
                pEffect->m_vecPosition.x = m_vecRainPosition[i].x + vecCam.x;
                pEffect->m_vecPosition.z = m_vecRainPosition[i].z + vecCam.y;
                
                TMVector2 vec{ pEffect->m_vecPosition.x, pEffect->m_vecPosition.z };
                float fHeight = (float)g_pCurrentScene->GroundGetMask(vec);
                if (fHeight == 127.0f)
                    fHeight = 0.0f;

                TMVector2 vecWater{ pEffect->m_vecPosition.x, pEffect->m_vecPosition.z };
                if (g_pCurrentScene->GroundIsInWater(vecWater, -100.0f, &fHeight) == 1)
                    fHeight = fHeight + 0.25f;

                pEffect->m_vecPosition.y = (fHeight * 0.1f) + 0.25f;
                g_pCurrentScene->m_pEffectContainer->AddChild(pEffect);
            }
        }
        if (pObj)
            m_vecRainPosition[i].y = pObj->m_fHeight + 10.0f;
        m_vecRainPosition[i].x = weather_area::RandomOffset(m_fHalfSize, rand());
        m_vecRainPosition[i].z = weather_area::RandomOffset(m_fHalfSize, rand());
    }
    m_dwOldServerTime = dwServerTime;
    return 1;
}
