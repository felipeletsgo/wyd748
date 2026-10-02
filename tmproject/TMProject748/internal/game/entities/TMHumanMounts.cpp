#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "TMScene.h"
#include "TMEffectMeshRotate.h"
#include "TMEffectSkinMesh.h"
#include "TMGround.h"
#include "TMFieldScene.h"

void TMHuman::UpdateMount()
{
    TMFieldScene* pScene = (TMFieldScene*)g_pCurrentScene;
    if (pScene->m_bAirMove == 1 && g_pCurrentScene->m_pMyHuman == this)
    {
        m_cMount = 1;
        m_nMountSkinMeshType = 40;
        SetAnimation(ECHAR_MOTION::ECMOTION_SEATING, 1);

        memset(&m_stMountLook, 0, sizeof(m_stMountLook));
        if (m_pMantua)
        {
            m_pMantua->SetVecMantua(1, 40);
            m_pMantua->SetAnimation(3);
        }
    }

    SAFE_DELETE(m_pMount);

    if (m_cMount > 0)
    {
        if (m_pMount == nullptr)
        {
            m_pMount = new TMSkinMesh(&m_stMountLook,
                &m_stMountSanc,
                m_nMountSkinMeshType,
                0,
                0,
                1,
                0,
                1);

            if (m_pMount)
            {
                m_pMount->m_pOwner = this;
                if (m_pMount->m_nBoneAniIndex == 50)
                    m_pMount->m_dwFPS = 7;
                else
                    m_pMount->m_dwFPS = 40;
                if (m_nClass == 40)
                {
                    m_pMount->m_vScale.x = m_fMountScale;
                    m_pMount->m_vScale.y = m_fMountScale;
                    m_pMount->m_vScale.z = m_fMountScale;
                }
                else
                {
                    m_pMount->m_vScale.x = m_fScale * m_fMountScale;
                    m_pMount->m_vScale.y = m_fScale * m_fMountScale;
                    m_pMount->m_vScale.z = m_fScale * m_fMountScale;
                }

                m_pMount->m_bBaseMat = 0;
                if (m_nMountSkinMeshType == 20 && m_stMountLook.Mesh0 == 7)
                {
                    m_pSkinMesh->SetVecMantua(4, m_nMountSkinMeshType);
                }
                else if (m_nMountSkinMeshType == 20)
                {
                    m_pSkinMesh->SetVecMantua(3, m_nMountSkinMeshType);
                }
                else
                {
                    m_pSkinMesh->SetVecMantua(2, m_nMountSkinMeshType);
                }
            }
        }

        if (m_pMount)
            m_pMount->RestoreDeviceObjects();

        return;
    }

    if (g_pCurrentScene && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD && g_pCurrentScene->m_pMyHuman == this)
    {
        pScene->SetPosPKRun();
        SetSpeed(pScene->m_bMountDead);
    }
}

float TMHuman::GetMyHeight()
{
    if (g_pCurrentScene == nullptr)
        return 0.0f;

    if (m_nClass == 45 && !m_stLookInfo.LeftMesh)
    {
        m_fWantHeight = 0.0;
        return 0.0f;
    }
    else if (m_nClass == 50 && g_pCurrentScene->m_pGround)
    {
        m_fWantHeight = (float)g_pCurrentScene->GetMask2(m_vecPosition) * 0.1f;
        return m_fWantHeight;
    }

    else if (m_nSkinMeshType == 40
        || m_nSkinMeshType == 24
        || m_nSkinMeshType == 20
        || m_nSkinMeshType == 39
        || m_nSkinMeshType == 8
        || m_cMount && (m_nMountSkinMeshType == 40
            || m_nMountSkinMeshType == 20
            || m_nMountSkinMeshType == 39
            || m_nMountSkinMeshType == 45
            || m_nMountSkinMeshType == 46
            || m_nMountSkinMeshType == 47))
    {
        if (fabsf(m_fHeight - m_fWantHeight) <= 0.1f)
            return m_fWantHeight;
        else
            return (float)(m_fWantHeight
                - (float)((float)(m_fWantHeight - m_fHeight) * 0.89999998f));
    }

    else if (m_nSkinMeshType == 5 && m_stLookInfo.FaceMesh != 1)
    {
        if (fabsf(m_fHeight - m_fWantHeight) <= 0.1f)
            return m_fWantHeight;
        else
            return (float)(m_fWantHeight
                - (float)((float)(m_fWantHeight - m_fHeight) * 0.89999998f));
    }

    return m_fWantHeight;
}

bool TMHuman::SetImportedMountCostume(unsigned int itemIndex)
{
    struct ImportedMountVisual
    {
        unsigned short item;
        short type;
        float scale;
        short mesh0;
        short mesh1;
        short mesh2;
        short skin0;
        short skin1;
        short skin2;
        unsigned char sanc0;
        unsigned char sanc1;
        unsigned char sanc2;
    };

    // The original KR patch selected these visuals from the complete item ID.
    // Keep that table in source so the rebuilt 7.48 client no longer depends on
    // executable hooks or on the lossy Equip2 costume byte. A negative type is
    // an authentic KR entry whose required mesh is absent from the supplied data.
    static const ImportedMountVisual kImportedMounts[] =
    {
        {4190, 29, 1.25f, 5, 5, 0, 0, 0, 0, 13, 13, 13},
        {4191, 31, 1.00f, 8, 8, 0, 1, 1, 0, 0, 0, 0},
        {4192, 31, 1.00f, 8, 8, 0, 0, 0, 0, 12, 12, 12},
        {4193, 48, 0.80f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {4194, 49, 0.70f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {4195, 49, 0.70f, 0, 0, 0, 1, 0, 0, 0, 0, 0},
        {4196, 31, 0.90f, 14, 14, 0, 0, 0, 0, 13, 13, 13},
        {4197, 31, 0.90f, 14, 14, 0, 1, 1, 0, 13, 13, 13},
        {4198, 50, 1.00f, 0, 0, 0, 0, 0, 0, 13, 13, 13},
        {4199, 49, 0.70f, 1, 1, 0, 1, 1, 0, 0, 0, 0},
        {4200, 51, 1.00f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {4201, 59, 1.00f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {4202, 31, 1.00f, 11, 11, 0, 0, 0, 0, 12, 12, 12},
        {4203, 39, 1.00f, 1, 1, 0, 0, 0, 0, 12, 12, 12},
        {4204, 39, 1.00f, 2, 2, 0, 0, 0, 0, 12, 12, 12},
        {4205, 31, 1.00f, 12, 12, 0, 0, 0, 0, 12, 12, 12},
        {4206, 31, 1.00f, 13, 13, 0, 0, 0, 0, 13, 13, 13},
        {4207, 29, 1.00f, 6, 6, 0, 0, 0, 0, 13, 13, 13},
        {4208, 40, 1.00f, 46, 46, 0, 0, 0, 0, 8, 8, 8},
        {4209, 40, 1.00f, 46, 46, 0, 1, 1, 0, 13, 13, 13},
        {4210, 48, 0.80f, 1, 1, 0, 0, 0, 0, 13, 13, 13},
        {4211, -1, 1.00f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {4212, 49, 0.70f, 3, 3, 0, 0, 0, 0, 0, 0, 0},
        {4213, 20, 0.50f, 48, 48, 0, 0, 0, 0, 13, 13, 13},
        {4214, 40, 1.00f, 46, 46, 46, 3, 3, 0, 6, 6, 6},
        {4215, 51, 1.00f, 0, 0, 0, 1, 1, 1, 6, 6, 6},
        {4216, 49, 0.70f, 4, 4, 0, 0, 0, 0, 12, 12, 12},
        {4217, 39, 1.00f, 3, 3, 0, 0, 0, 0, 12, 12, 12},
        {4218, 50, 1.00f, 1, 1, 0, 0, 0, 0, 13, 13, 13},
        {4219, 31, 1.00f, 18, 18, 0, 0, 0, 0, 10, 10, 10},
        {4220, 31, 1.00f, 19, 19, 0, 0, 0, 0, 13, 13, 13},
        {4221, 49, 0.70f, 0, 0, 0, 5, 0, 0, 0, 0, 0},
        {4222, 31, 1.00f, 20, 20, 0, 0, 0, 0, 13, 13, 13},
        {4223, 59, 1.00f, 1, 1, 1, 0, 0, 0, 7, 7, 7},
        {4224, 49, 0.70f, 0, 0, 0, 6, 0, 0, 0, 0, 0},
        {4225, 31, 0.90f, 21, 21, 0, 0, 0, 0, 10, 10, 10},
        {4226, 40, 0.90f, 48, 48, 48, 0, 0, 0, 9, 9, 9},
        {4227, 31, 1.00f, 17, 17, 0, 0, 0, 0, 15, 15, 15},
        {4228, 49, 0.80f, 7, 7, 0, 0, 0, 0, 13, 13, 13},
        {4229, 49, 0.80f, 8, 8, 0, 0, 0, 0, 13, 13, 13},
        {4230, 29, 1.25f, 10, 10, 0, 0, 0, 0, 13, 13, 13},
        {4231, 49, 0.60f, 9, 9, 0, 0, 0, 0, 12, 12, 12},
        {4232, 49, 0.70f, 0, 0, 0, 10, 0, 0, 0, 0, 0},
        {4233, 49, 0.70f, 0, 0, 0, 10, 0, 0, 0, 0, 0},
        {4234, 49, 0.70f, 0, 0, 0, 10, 0, 0, 0, 0, 0},
        {4235, -1, 1.00f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {4241, 49, 0.70f, 17, 17, 0, 0, 0, 0, 12, 12, 12},
        {6000, 30, 1.00f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6001, 31, 1.00f, 0, 0, 4, 1, 1, 0, 0, 0, 0},
        {6002, 31, 1.00f, 0, 0, 5, 1, 1, 0, 0, 0, 0},
        {6003, -1, 1.00f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6004, 25, 1.00f, 3, 3, 0, 0, 0, 0, 12, 12, 12},
        {6005, 31, 1.00f, 50, 50, 0, 0, 0, 0, 0, 0, 0},
        {6006, 49, 0.60f, 9, 9, 0, 0, 0, 0, 12, 12, 12},
        {6007, 49, 0.70f, 0, 0, 0, 10, 0, 0, 0, 0, 0},
        {6008, 38, 1.00f, 4, 4, 0, 0, 0, 0, 8, 8, 8},
        {6009, 29, 1.00f, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6010, 38, 1.00f, 1, 1, 0, 0, 0, 0, 8, 8, 8},
        {6011, 38, 1.00f, 2, 2, 0, 0, 0, 0, 8, 8, 8},
        {6012, 49, 0.80f, 7, 7, 0, 0, 0, 0, 13, 13, 13},
        {6013, 49, 0.80f, 8, 8, 0, 0, 0, 0, 13, 13, 13},
        {6014, 29, 1.25f, 10, 10, 0, 0, 0, 0, 13, 13, 13}
    };

    const ImportedMountVisual* visual = nullptr;
    for (const ImportedMountVisual& candidate : kImportedMounts)
    {
        if (candidate.item == itemIndex)
        {
            visual = &candidate;
            break;
        }
    }

    if (!visual)
        return false;

    memset(&m_stMountLook, 0, sizeof(m_stMountLook));
    memset(&m_stMountSanc, 0, sizeof(m_stMountSanc));
    memset(&m_stOldMountSanc, 0, sizeof(m_stOldMountSanc));
    m_cLastMount = m_cMount;

    // Do not fabricate replacement visuals for unavailable authentic assets or
    // for merchant bodies, which the native costume selector also excludes.
    if (visual->type < 0 || (m_stScore.Merchant & 2))
    {
        m_cMount = 0;
        return true;
    }

    m_cMount = 1;
    m_sMountIndex = -1;
    m_nMountSkinMeshType = visual->type;
    m_fMountScale = visual->scale;
    m_stMountLook.Mesh0 = visual->mesh0;
    m_stMountLook.Mesh1 = visual->mesh1;
    m_stMountLook.Mesh2 = visual->mesh2;
    m_stMountLook.Skin0 = visual->skin0;
    m_stMountLook.Skin1 = visual->skin1;
    m_stMountLook.Skin2 = visual->skin2;
    m_stMountSanc.Sanc0 = visual->sanc0;
    m_stMountSanc.Sanc1 = visual->sanc1;
    m_stMountSanc.Sanc2 = visual->sanc2;
    m_stOldMountSanc = m_stMountSanc;
    return true;
}

void  TMHuman::SetMountCostume(unsigned int  index)
{
    int nSanc = 0, nSkin = 0;
    int curIndex = index;

    if (index >= 11 && index <= 200 && (!(m_stScore.Merchant & 2)))
    {
        // This must update the member consumed by UpdateMount; the former local
        // variable shadowed it and discarded every costume-specific KR scale.
        m_fMountScale = 1.0f;
        m_stMountLook.Mesh2 = 0;
        m_stMountLook.Mesh1 = 0;
        m_stMountLook.Mesh0 = 0;
        m_stMountLook.Skin2 = 0;
        m_stMountLook.Skin1 = 0;
        m_stMountLook.Skin0 = 0;
        memset(&m_stMountSanc.Sanc0, 0, sizeof m_stMountSanc);
        memset(&m_stOldMountSanc.Sanc0, 0, sizeof m_stOldMountSanc);

        switch (curIndex)
        {
        case 11:
            m_nMountSkinMeshType = 29;
            m_stMountLook.Mesh0 = 5;
            m_stMountLook.Mesh1 = 5;
            m_fMountScale = 1.25f;
            break;
        case 12:
        case 13:
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh0 = 8;
            m_stMountLook.Mesh1 = 8;
            if (index == 12)
            {
                m_stMountLook.Skin0 = 1;
                m_stMountLook.Skin1 = 1;
            }
            else
            {
                m_stMountSanc.Sanc2 = 12;
                m_stMountSanc.Sanc1 = 12;
                m_stMountSanc.Sanc0 = 12;
                m_stOldMountSanc.Sanc2 = 12;
                m_stOldMountSanc.Sanc1 = 12;
                m_stOldMountSanc.Sanc0 = 12;
            }

            m_fMountScale = 1.0f;
            break;
        case 14:
            m_nMountSkinMeshType = 48;
            m_fMountScale = 0.80f;
            break;
        case 15:
            m_stMountLook.Skin0 = 0;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69f;
            break;
        case 16:
            m_stMountLook.Skin0 = 1;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69f;
            break;
        case 17:
        case 18:
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh0 = 14;
            m_stMountLook.Mesh1 = 14;
            if (index == 18)
            {
                m_stMountLook.Skin0 = 1;
                m_stMountLook.Skin1 = 1;
            }

            m_fMountScale = 0.89f;
            m_stMountSanc.Sanc2 = 13;
            m_stMountSanc.Sanc1 = 13;
            m_stMountSanc.Sanc0 = 13;
            m_stOldMountSanc.Sanc2 = 13;
            m_stOldMountSanc.Sanc1 = 13;
            m_stOldMountSanc.Sanc0 = 13;
            break;
        case 19:
            m_nMountSkinMeshType = 50;
            m_stMountSanc.Sanc2 = 13;
            m_stMountSanc.Sanc1 = 13;
            m_stMountSanc.Sanc0 = 13;
            m_stOldMountSanc.Sanc2 = 13;
            m_stOldMountSanc.Sanc1 = 13;
            m_stOldMountSanc.Sanc0 = 13;
            break;
        case 20:
            m_stMountLook.Skin0 = 1;
            m_stMountLook.Skin1 = 1;
            m_stMountLook.Mesh0 = 1;
            m_stMountLook.Mesh1 = 1;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69f;
            break;
        case 21:
            m_nMountSkinMeshType = 51;
            m_stOldMountSanc.Sanc2 = 13;
            m_stOldMountSanc.Sanc1 = 13;
            m_stOldMountSanc.Sanc0 = 13;
            m_fMountScale = 1.0f;
            break;
        case 22:
            m_nMountSkinMeshType = 59;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_fMountScale = 1.0f;
            break;
        case 23:
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh0 = 11;
            m_stMountLook.Mesh1 = 11;
            m_stMountSanc.Sanc2 = 11;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;

            break;
        case 24:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 1;
            m_nMountSkinMeshType = 39;
            m_stMountLook.Mesh1 = 1;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 25:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 2;
            m_nMountSkinMeshType = 39;
            m_stMountLook.Mesh1 = 2;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 26:
            m_fMountScale = 1.0f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountLook.Mesh0 = 12;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh1 = 12;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 27:
            m_fMountScale = 1.0f;
            m_stMountSanc.Sanc2 = 13;
            m_stMountLook.Mesh0 = 13;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh1 = 13;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 28:
            m_fMountScale = 1.25f;
            m_stMountLook.Mesh0 = 6;
            m_stMountLook.Mesh1 = 6;
            m_nMountSkinMeshType = 29;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 29:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 46;
            m_nMountSkinMeshType = 40;
            m_stMountLook.Mesh1 = 46;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 30:
            m_stMountLook.Mesh1 = 46;
            m_stMountLook.Mesh0 = 46;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 40;
            m_stMountLook.Skin1 = 1;
            m_stMountLook.Skin0 = 1;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 31:
            m_fMountScale = 0.80000001f;
            m_nMountSkinMeshType = 48;
            m_stMountLook.Mesh0 = 1;
            m_stMountLook.Mesh1 = 1;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;

           /* m_stMountSanc.Sanc2 = 3341;
            m_stOldMountSanc.Sanc1 = 3341;*/
            break;
        case 32:
            m_stMountLook.Mesh0 = 8;
            m_stMountLook.Skin0 = 8;
            m_stMountLook.Mesh1 = 8;
            m_stMountLook.Skin1 = 8;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 33:
            m_nMountSkinMeshType = 49;
            m_stMountLook.Mesh0 = 3;
            m_stMountLook.Mesh1 = 3;
            m_fMountScale =0.8f;
            break;
        case 34:
            m_fMountScale = 0.5f;
            m_stMountLook.Mesh0 = 48;
            m_stMountSanc.Sanc1 = 13;
            m_nMountSkinMeshType = 20;
            m_stMountLook.Mesh1 = 48;
         /*   m_stMountSanc.Sanc2 = 3341;
            m_stOldMountSanc.Sanc1 = 3084;*/
            m_stMountSanc.Sanc2 = 12;
            break;
        case 35:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh1 = 46;
            m_stMountLook.Mesh0 = 46;
            m_stMountLook.Mesh2 = 46;
            m_stMountLook.Skin0 = 3;
            m_nMountSkinMeshType = 40;
            m_stMountLook.Skin1 = 3;
            /*m_stMountSanc.Sanc2 = 1542;
            m_stOldMountSanc.Sanc1 = 1542;*/
            m_stMountSanc.Sanc2 = 6;
            m_stMountSanc.Sanc1 = 6;
            break;
        case 36:
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 51;
            m_stMountLook.Skin0 = 1;
            m_stMountLook.Skin2 = 1;
            break;
        case 37:
            m_fMountScale = 0.69999999f;
            m_stMountLook.Mesh0 = 4;
            m_stMountLook.Mesh1 = 4;
            m_nMountSkinMeshType = 49;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 38:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 3;
            m_nMountSkinMeshType = 39;
            m_stMountLook.Mesh1 = 3;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 39:
            m_stMountLook.Mesh0 = 1;
            m_nMountSkinMeshType = 50;
            m_stMountLook.Mesh1 = 1;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 40:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 18;
            m_stMountLook.Mesh1 = 18;
            m_nMountSkinMeshType = 31;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 41:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 19;
            m_stMountLook.Mesh1 = 19;
            m_nMountSkinMeshType = 31;
            m_stMountSanc.Sanc2 = 255;
            m_stMountSanc.Sanc1 = 255;
            m_stMountSanc.Sanc0 = 255;
            break;
        case 42:
            m_fMountScale = 0.7f;
            m_nMountSkinMeshType = 49;
            m_stMountLook.Skin0 = 5;
            break;
        case 43:
            m_stMountLook.Mesh0 = 20;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh1 = 20;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 44:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 1;
            m_stMountLook.Mesh2 = 1;
            m_nMountSkinMeshType = 59;
            m_stMountLook.Mesh1 = 1;
            m_stMountLook.Mesh3 = 1;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;;
            break;
        case 45:
            m_nMountSkinMeshType = 49;
            m_stMountLook.Skin0 = 6;
            m_fMountScale = 0.7f;
            break;
        case 46:
            m_fMountScale = 0.50f;
            m_stMountLook.Mesh0 = 21;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh1 = 21;
            nSanc = 10;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;

            m_fMountScale = 0.89999998f;
            m_stMountSanc.Sanc2 = nSanc;
            m_stMountSanc.Sanc1 = nSanc;
            break;
        case 47:
            m_fMountScale = 0.89999998f;
            m_stMountLook.Mesh1 = 48;
            m_stMountLook.Mesh0 = 48;
            m_stMountLook.Mesh2 = 48;
            m_stMountLook.Skin0 = 0;
            m_nMountSkinMeshType = 40;
            m_stMountLook.Skin1 = 0;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 48:
            m_nMountSkinMeshType = 31;
            m_stMountLook.Mesh0 = 17;
            m_stMountLook.Mesh1 = 17;
            m_stMountLook.Skin1 = 0;
            m_fMountScale = BASE_GetMountScale(31, 17);
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 49:
            m_fMountScale = 0.80000001f;
            m_stMountLook.Skin0 = 0;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Mesh0 = 7;
            m_stMountLook.Mesh1 = 7;
            m_nMountSkinMeshType = 49;
            m_stMountSanc.Sanc2 = 255;
            m_stMountSanc.Sanc1 = 255;
            m_stMountSanc.Sanc0 = 255;
            break;
        case 50:
            m_stMountLook.Skin0 = 0;
            m_fMountScale = 0.80000001f;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Mesh1 = 8;
            m_stMountLook.Mesh0 = 8;
            m_nMountSkinMeshType = 49;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 51:
            m_fMountScale = 1.35f;
            m_stMountLook.Skin0 = 0;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Mesh0 = 10;
            m_stMountLook.Mesh1 = 10;
            m_nMountSkinMeshType = 29;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 52:
            m_fMountScale = 0.60000002f;
            m_stMountLook.Skin0 = 0;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Mesh0 = 9;
            m_stMountLook.Mesh1 = 9;
            m_nMountSkinMeshType = 49;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 53:
            m_stMountLook.Mesh0 = 2;
            m_nMountSkinMeshType = 50;
            m_stMountLook.Mesh1 = 2;
            m_stMountLook.Skin0 = 0;
            m_stMountLook.Skin1 = 0;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 54:
           break;
        case 55:
            break;
        case 56:
            m_fMountScale = 0.60000002f;
            m_stMountLook.Mesh0 = 11;
            m_stMountLook.Mesh1 = 11;
            m_nMountSkinMeshType = 49;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 57:
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 12;
            m_stMountLook.Mesh1 = 12;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.6f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 58:
            m_stMountLook.Mesh1 = 16;
            m_stMountLook.Skin0 = 14;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69999999f;
            break;
        case 59:
            m_stMountLook.Mesh1 = 16;
            m_stMountLook.Skin0 = 13;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69999999f;
            break;
        case 60:
            m_stMountLook.Mesh1 = 16;
            m_stMountLook.Skin0 = 15;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69999999f;
            break;
        case 61:
            m_stMountLook.Mesh1 = 16;
            m_stMountLook.Skin0 = 16;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69999999f;
            break;
        case 62://4241
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 17;
            m_stMountLook.Mesh1 = 17;
            m_nMountSkinMeshType = 49;
            m_fMountScale = 0.69999999f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 63://4242
            break;
        case 64://4243
            break;
        case 65://4244
            break;
        case 66://4245
            break;
        case 67://4246
            break;
        case 68://4247
            break;
        case 69://4248
            break;
        case 70://4249
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 42;
            m_stMountLook.Mesh1 = 42;
            m_nMountSkinMeshType = 20;
            m_fMountScale = 1.3f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 71://4250
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 42;
            m_stMountLook.Mesh1 = 42;
            m_nMountSkinMeshType = 38;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_fMountScale = 1.10f;
            break;
        case 72://4251
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 43;
            m_stMountLook.Mesh1 = 43;
            m_nMountSkinMeshType = 38;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_fMountScale = 1.10f;
            break;
        case 73://4252
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 43;
            m_stMountLook.Mesh1 = 43;
            m_nMountSkinMeshType = 20;
            m_fMountScale = 1.60f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_fMountScale = 1.10f;
            break;
        case 74://4253
            break;
        case 75://4254
            break;
        case 76://4255
            break;
        case 77://4256
            break;
        case 78://4257
            break;
        case 79://4258
            break;
        case 80://4259
            break;
        case 81://4260
            break;
        case 82://4261
            break;
        case 83://4262
            break;
        case 84://4263
            break;
        case 85://4264
            break;
        case 86://4265
            break;
        case 87://4266
            break;
        case 88://4267
            break;
        case 89://4268
            break;
        case 90://4269
            break;
        case 91://4270
            m_stMountLook.Mesh1 = 40;
            m_stMountLook.Mesh0 = 40;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 40;
            m_stMountSanc.Sanc0 = 3;
            m_stMountSanc.Sanc1 = 3;
            m_stMountSanc.Sanc2 = 12;
            break;
        case 92://4271
            m_stMountLook.Mesh1 = 41;
            m_stMountLook.Mesh0 = 41;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 40;
            m_stMountSanc.Sanc0 = 3;
            m_stMountSanc.Sanc1 = 3;
            m_stMountSanc.Sanc2 = 12;
            break;
        case 93://4272
            m_stMountLook.Mesh1 = 42;
            m_stMountLook.Mesh0 = 42;
            m_fMountScale = 1.0;
            m_nMountSkinMeshType = 40;
            m_stMountSanc.Sanc0 = 3;
            m_stMountSanc.Sanc1 = 3;
            m_stMountSanc.Sanc2 = 12;
            break;
        case 94://4273
            m_stMountLook.Mesh1 = 46;
            m_stMountLook.Mesh0 = 46;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Skin0 = 0;
            break;
        case 95://4274
            m_stMountLook.Mesh1 = 46;
            m_stMountLook.Mesh0 = 46;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 1;
            m_stMountLook.Skin0 = 1;
            break;
        case 96://4275
            m_stMountLook.Mesh1 = 40;
            m_stMountLook.Mesh0 = 40;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Skin0 = 0;
            break;
        case 97://4276
            m_stMountLook.Mesh1 = 40;
            m_stMountLook.Mesh0 = 40;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 1;
            m_stMountLook.Skin0 = 1;
            break;
        case 98://4277
            m_fMountScale = 1.5;
            m_stMountLook.Mesh0 = 40;
            m_stMountLook.Mesh1 = 40;
            m_nMountSkinMeshType = 29;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 99://4278
            m_fMountScale = 1.5f;
            m_stMountLook.Mesh0 = 41;
            m_stMountLook.Mesh1 = 41;
            m_nMountSkinMeshType = 29;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 100://4279
            m_fMountScale = 1.10f;
            m_stMountLook.Mesh0 = 40;
            m_stMountLook.Mesh1 = 40;
            m_nMountSkinMeshType = 30;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Skin1 = 0;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 101://4280
            m_fMountScale = 1.10f;
            m_stMountLook.Mesh0 = 41;
            m_stMountLook.Mesh1 = 41;
            m_nMountSkinMeshType = 30;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 102://4281
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 44;
            m_stMountLook.Mesh1 = 44;
            m_nMountSkinMeshType = 38;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_fMountScale = 1.10f;
            break;
        case 103://4282
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 41;
            m_stMountLook.Mesh1 = 41;
            m_nMountSkinMeshType = 38;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_fMountScale = 1.10f;
            break;
        case 104://4283
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 1;
            m_nMountSkinMeshType = 39;
            m_stMountLook.Mesh1 = 1;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 105:
            m_fMountScale = 1.0f;
            m_stMountLook.Mesh0 = 2;
            m_nMountSkinMeshType = 39;
            m_stMountLook.Mesh1 = 2;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            m_stOldMountSanc.Sanc2 = 12;
            m_stOldMountSanc.Sanc1 = 12;
            m_stOldMountSanc.Sanc0 = 12;
            break;
        case 106://4285
            m_stMountLook.Mesh1 = 41;
            m_stMountLook.Mesh0 = 41;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 1;
            m_stMountLook.Skin0 = 1;
            break;
        case 107://4286
            m_stMountLook.Mesh1 = 42;
            m_stMountLook.Mesh0 = 42;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 1;
            m_stMountLook.Skin0 = 1;
            break;
        case 108://4287
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 40;
            m_stMountLook.Mesh1 = 40;
            m_nMountSkinMeshType = 20;
            m_fMountScale = 1.3f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 109://4288
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 41;
            m_stMountLook.Mesh1 = 41;
            m_nMountSkinMeshType = 20;
            m_fMountScale = 1.3f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 110://4289
            nSanc = 10;
            m_stMountLook.Skin0 = nSkin;
            m_stMountLook.Mesh0 = 40;
            m_stMountLook.Mesh1 = 40;
            m_nMountSkinMeshType = 39;
            m_fMountScale = 1.20f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 111://4290
            m_stMountLook.Mesh0 = 41;
            m_stMountLook.Mesh1 = 41;
            m_nMountSkinMeshType = 39;
            m_fMountScale = 1.20f;
            m_stMountSanc.Sanc2 = 12;
            m_stMountSanc.Sanc1 = 12;
            m_stMountSanc.Sanc0 = 12;
            break;
        case 112://4291
            m_fMountScale = 1.2f;
            m_stMountLook.Mesh0 = 42;
            m_stMountSanc.Sanc1 = 13;
            m_nMountSkinMeshType = 29;
            m_stMountLook.Mesh1 = 42;
            m_stMountSanc.Sanc0 = 13;
            break;
        case 113://4283
            m_fMountScale = 0.8f;
            m_stMountLook.Mesh0 = 47;
            m_stMountSanc.Sanc1 = 13;
            m_nMountSkinMeshType = 20;
            m_stMountLook.Mesh1 = 47;
            m_stMountSanc.Sanc0 = 13;
            break;
        case 114://4284
            m_stMountLook.Mesh1 = 43;
            m_stMountLook.Mesh0 = 43;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 40;
            m_stMountSanc.Sanc0 = 13;
            m_stMountSanc.Sanc1 = 13;
            m_stMountSanc.Sanc2 = 12;
            break;
        case 115://4285
            m_stMountLook.Mesh1 = 44;
            m_stMountLook.Mesh0 = 44;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 40;
            m_stMountSanc.Sanc0 = 13;
            m_stMountSanc.Sanc1 = 13;
            m_stMountSanc.Sanc2 = 12;
            break;
        case 116://4286
            m_stMountLook.Mesh1 = 45;
            m_stMountLook.Mesh0 = 45;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 40;
            m_stMountSanc.Sanc0 = 13;
            m_stMountSanc.Sanc1 = 13;
            m_stMountSanc.Sanc2 = 12;
            break;
        case 117://4287
            m_stMountLook.Mesh1 = 48;
            m_stMountLook.Mesh0 = 48;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Skin0 = 0;
            break;
        case 118://4288
            m_stMountLook.Mesh1 = 49;
            m_stMountLook.Mesh0 = 49;
            m_fMountScale = 1.0f;
            m_nMountSkinMeshType = 31;
            m_stMountLook.Skin1 = 0;
            m_stMountLook.Skin0 = 0;
            break;
        case 119://4289
            break;
        case 120://4290
            break;
        default:
            return;
        }
    }
}
