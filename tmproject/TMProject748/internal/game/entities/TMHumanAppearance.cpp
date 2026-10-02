#include "pch.h"
#include "TMHuman.h"
#include "TMGlobal.h"
#include "HumanCostumeRefinement.h"
#include "SControl.h"
#include "TMScene.h"
#include "TMEffectSkinMesh.h"
#include "TMEffectSWSwing.h"
#include "ObjectManager.h"
#include "TMMesh.h"
#include "TMFieldScene.h"
#include "TMLog.h"

void TMHuman::SetRace(short sIndex)
{
    if (m_dwDelayDel != 0)
        return;

    STRUCT_ITEM item{};
    item.sIndex = sIndex;
    m_nClass = BASE_GetItemAbility(&item, 18);
    m_nSkinMeshType = BASE_DefineSkinMeshType(m_nClass);

    if(sIndex == 25)
        m_fScale = (((float)m_stScore.Mastery[3] * 0.003f) + 1.0f) * m_fScale;
}

void TMHuman::SetWeaponType(int nWeaponType)
{
    if (m_dwDelayDel != 0)
        return;

    m_nWeaponTypeIndex = nWeaponType;
}

void TMHuman::CheckWeapon(short sIndexL, short sIndexR)
{
    if (m_dwDelayDel != 0)
        return;

    m_cHasShield = 0;

    STRUCT_ITEM itemL{};
    STRUCT_ITEM itemR{};

    m_sLeftIndex = sIndexL;
    itemL.sIndex = sIndexL;
    m_sRightIndex = sIndexR;
    itemR.sIndex = sIndexR;
    m_nWeaponTypeL = BASE_GetItemAbility(&itemL, 21);
    m_nWeaponTypeR = BASE_GetItemAbility(&itemR, 21);
    unsigned int nWeaponPosL = BASE_GetItemAbility(&itemL, 17);
	unsigned int nWeaponPosR = BASE_GetItemAbility(&itemR, 17);

    if (itemR.sIndex < 0 || itemL.sIndex < 0 || itemR.sIndex >= 6500 || itemL.sIndex >= 6500)
    {
        LOG_WRITELOG("Check Weapon : L = %d R = %d", itemL.sIndex, itemR.sIndex);
        return;
    }
    if (g_pItemList[itemR.sIndex].nIndexMesh >= 0 && g_pItemList[itemL.sIndex].nIndexMesh >= 0)
    {
        TMMesh* pMesh1 = g_pMeshManager->GetCommonMesh(g_pItemList[itemR.sIndex].nIndexMesh, 0, 3_min);
        TMMesh* pMesh2 = g_pMeshManager->GetCommonMesh(g_pItemList[itemL.sIndex].nIndexMesh, 0, 3_min);

        if (!pMesh1 || !pMesh2)
        {
            LOG_WRITELOG("NULL Mesh Check Weapon : L = %d R = %d", itemL.sIndex, itemR.sIndex);
            return;
        }

        m_fSowrdLength[0] = pMesh1->m_fMaxZ;
        m_fSowrdLength[1] = pMesh2->m_fMaxZ;
        m_bSwordShadow[0] = 0;
        m_bSwordShadow[1] = 0;

        if (m_pSkinMesh)
        {
            if (m_nClass == 26 || m_nClass == 33 || m_nClass == 40)
            {
                SetWeaponType(0);
                if (m_sRightIndex)
                    m_bSwordShadow[0] = 1;
                if (m_sLeftIndex)
                    m_bSwordShadow[1] = 1;
                if (m_pSkinMesh->m_pSwingEffect[0])
                {
                    m_fSowrdLength[0] = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[0]->m_fWeaponLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[0]->m_fEffectLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect = 16 * g_pItemList[itemR.sIndex].nGrade;
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect += m_stSancInfo.Sanc6;
                }
                if (m_pSkinMesh->m_pSwingEffect[1])
                {
                    m_fSowrdLength[1] = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[1]->m_fWeaponLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[1]->m_fEffectLength = 0.029f;
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect = 16 * g_pItemList[itemL.sIndex].nGrade;
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect += m_stSancInfo.Sanc7;
                }
                return;
            }

            if (m_pSkinMesh->m_pSwingEffect[0])
            {
                m_pSkinMesh->m_pSwingEffect[0]->m_fWeaponLength = m_fSowrdLength[0] - 0.1f;
                m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect = 16 * g_pItemList[itemR.sIndex].nGrade;
                m_pSkinMesh->m_pSwingEffect[0]->m_cMixEffect += m_stSancInfo.Sanc6;
            }
            if (m_pSkinMesh->m_pSwingEffect[1])
            {
                m_pSkinMesh->m_pSwingEffect[1]->m_fWeaponLength = m_fSowrdLength[1] - 0.1f;
                m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect = 16 * g_pItemList[itemL.sIndex].nGrade;
                m_pSkinMesh->m_pSwingEffect[1]->m_cMixEffect += m_stSancInfo.Sanc7;
            }
            if (m_cWeapon == 1)
            {
                if (m_pSkinMesh->m_pSwingEffect[0] && m_sRightIndex)
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 1;
                if (m_pSkinMesh->m_pSwingEffect[1] && m_sLeftIndex)
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 1;
            }
            else
            {
                if (m_pSkinMesh->m_pSwingEffect[0])
                    m_pSkinMesh->m_pSwingEffect[0]->m_cMagicWeapon = 0;
                if (m_pSkinMesh->m_pSwingEffect[1])
                    m_pSkinMesh->m_pSwingEffect[1]->m_cMagicWeapon = 0;
            }

            if (m_nClass == 36)
            {
                if (m_nWeaponTypeL == 1 || m_nWeaponTypeL == 61 || m_nWeaponTypeL == 31)
                {
                    SetWeaponType(11);
                }
                else if (m_nWeaponTypeL == 11)
                {
                    SetWeaponType(15);
                }
                else if (m_nWeaponTypeL == 13)
                {
                    SetWeaponType(12);
                }
                else if (m_nWeaponTypeL == 21 || m_nWeaponTypeL == 22 || m_nWeaponTypeL == 23)
                {
                    SetWeaponType(13);
                }
                else if (m_nWeaponTypeL == 3 || m_nWeaponTypeL == 63)
                {
                    SetWeaponType(14);
                }
                m_bSwordShadow[1] = 1;
                return;
            }
            if (m_nClass == 37)
            {
                SetWeaponType(11);
                m_bSwordShadow[1] = 1;
                return;
            }
            if (m_nClass == 60)
            {
                if (m_nWeaponTypeL == 41 && !nWeaponPosR)
                {
                    m_pSkinMesh->m_cRotate[1] = 1;
                    SetWeaponType(0);
                    m_bSwordShadow[0] = 1;
                }
                else if (m_nWeaponTypeL == 11 && nWeaponPosR == 128)
                {
                    SetWeaponType(1);
                    m_bSwordShadow[0] = 1;
                }
                else if (m_nWeaponTypeL == 11 && nWeaponPosR == 192)
                {
                    SetWeaponType(4);
                    m_bSwordShadow[0] = 1;
                    m_bSwordShadow[1] = 1;
                }
                else
                {
                    m_bSwordShadow[1] = 1;
                    m_bSwordShadow[0] = 1;
                    SetWeaponType(0);
                }
                return;
            }
            if (m_nClass == 61)
            {
                SetWeaponType(1);
                m_bSwordShadow[1] = 1;
                return;
            }
            if (m_nClass == 62)
            {
                if (m_nWeaponTypeL == 1
                    || m_nWeaponTypeL == 11
                    || m_nWeaponTypeL == 61
                    || m_nWeaponTypeL == 2
                    || m_nWeaponTypeL == 12
                    || m_nWeaponTypeL == 62
                    || m_nWeaponTypeL == 31)
                {
                    SetWeaponType(1);
                    m_bSwordShadow[1] = 1;
                    m_bSwordShadow[0] = 1;
                }
                else
                {
                    SetWeaponType(0);
                }
                return;
            }

            if (!m_nSkinMeshType)
            {
                if (!m_cMount)
                {
                    if (!m_nWeaponTypeL && nWeaponPosR == 128)
                    {
                        SetWeaponType(2);
                    }
                    else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                    {
                        SetWeaponType(4);
                        m_bSwordShadow[0] = 1;
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 1
                        || m_nWeaponTypeL == 11
                        || m_nWeaponTypeL == 61
                        || m_nWeaponTypeL == 31)
                    {
                        if (!m_nWeaponTypeR)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 128)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 196)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[0] = 1;
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 2
                        || m_nWeaponTypeL == 12
                        || m_nWeaponTypeL == 62)
                    {
                        if (nWeaponPosR == 128)
                        {
                            SetWeaponType(3);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (!m_nWeaponTypeR)
                        {
                            SetWeaponType(5);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 196)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[0] = 1;
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 3 || m_nWeaponTypeL == 63)
                    {
                        SetWeaponType(6);
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 13)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(7);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 21
                        || m_nWeaponTypeL == 22
                        || m_nWeaponTypeL == 23)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(8);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(10);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 104)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(9);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 101)
                    {
                        SetWeaponType(3);
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 32 || m_nWeaponTypeL == 33)
                    {
                        SetWeaponType(5);
                        m_bSwordShadow[1] = 1;
                    }
                    else
                    {
                        if (m_nWeaponTypeL == 41)
                        {
                            m_bSwordShadow[1] = 1;
                            m_bSwordShadow[0] = 1;
                        }
                        SetWeaponType(0);
                    }
                }
                else if (!m_nWeaponTypeL && nWeaponPosR == 128)
                {
                    SetWeaponType(3);
                }
                else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                {
                    SetWeaponType(2);
                    m_bSwordShadow[0] = 1;
                    m_bSwordShadow[1] = 1;
                }
                else if (m_nWeaponTypeL == 1
                    || m_nWeaponTypeL == 11
                    || m_nWeaponTypeL == 61
                    || m_nWeaponTypeL == 2
                    || m_nWeaponTypeL == 12
                    || m_nWeaponTypeL == 62
                    || m_nWeaponTypeL == 31)
                {
                    SetWeaponType(1);
                    if (!m_nWeaponTypeR)
                    {
                        m_bSwordShadow[1] = 1;
                    }
                    else if (nWeaponPosR == 128)
                    {
                        m_bSwordShadow[1] = 1;
                    }
                }
                else if (m_nWeaponTypeL == 21
                    || m_nWeaponTypeL == 22
                    || m_nWeaponTypeL == 23
                    || m_nWeaponTypeL == 13
                    || m_nWeaponTypeL == 3
                    || m_nWeaponTypeL == 63
                    || m_nWeaponTypeL == 32
                    || m_nWeaponTypeL == 33)
                {
                    if (nWeaponPosL == 64)
                    {
                        SetWeaponType(4);
                        m_bSwordShadow[1] = 1;
                    }
                }
                else if (m_nWeaponTypeL == 101)
                {
                    SetWeaponType(5);
                    m_bSwordShadow[1] = 1;
                }
                else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                {
                    if (nWeaponPosL == 64)
                    {
                        SetWeaponType(1);
                        m_bSwordShadow[1] = 1;
                    }
                }
                else
                {
                    if (m_nWeaponTypeL == 41)
                    {
                        m_bSwordShadow[1] = 1;
                        m_bSwordShadow[0] = 1;
                    }
                    SetWeaponType(0);
                }
                if (m_nWeaponTypeL == 41)
                {
                    m_bSwordShadow[1] = 1;
                    m_bSwordShadow[0] = 1;
                }
            }
            else
            {
                switch (m_nSkinMeshType)
                {
                case 1:
                    if (!m_cMount)
                    {
                        if (!m_nWeaponTypeL && nWeaponPosR == 128)
                        {
                            SetWeaponType(2);
                        }
                        else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[0] = 1;
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 3 || m_nWeaponTypeL == 63)
                        {
                            SetWeaponType(10);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 1
                            || m_nWeaponTypeL == 11
                            || m_nWeaponTypeL == 61
                            || m_nWeaponTypeL == 2
                            || m_nWeaponTypeL == 12
                            || m_nWeaponTypeL == 62
                            || m_nWeaponTypeL == 31)
                        {
                            if (!m_nWeaponTypeR)
                            {
                                SetWeaponType(1);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (nWeaponPosR == 128)
                            {
                                SetWeaponType(3);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (nWeaponPosR == 196)
                            {
                                SetWeaponType(4);
                                m_bSwordShadow[0] = 1;
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else if (m_nWeaponTypeL == 21
                            || m_nWeaponTypeL == 22
                            || m_nWeaponTypeL == 23)
                        {
                            SetWeaponType(5);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                        {
                            if (nWeaponPosL == 64)
                            {
                                SetWeaponType(3);
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else if (m_nWeaponTypeL == 31)
                        {
                            SetWeaponType(3);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 13)
                        {
                            SetWeaponType(7);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 32 || m_nWeaponTypeL == 33)
                        {
                            if (nWeaponPosL == 64)
                            {
                                SetWeaponType(9);
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else if (m_nWeaponTypeL == 101)
                        {
                            if (nWeaponPosL == 64)
                            {
                                SetWeaponType(6);
                                m_bSwordShadow[1] = 1;
                            }
                        }
                        else
                        {
                            if (m_nWeaponTypeL == 41)
                            {
                                m_bSwordShadow[1] = 1;
                                m_bSwordShadow[0] = 1;
                            }
                            SetWeaponType(0);
                        }
                    }
                    else if (!m_nWeaponTypeL && nWeaponPosR == 128)
                    {
                        SetWeaponType(3);
                    }
                    else if (nWeaponPosL == 192 && nWeaponPosR == 192)
                    {
                        SetWeaponType(2);
                        m_bSwordShadow[0] = 1;
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 1
                        || m_nWeaponTypeL == 11
                        || m_nWeaponTypeL == 61
                        || m_nWeaponTypeL == 2
                        || m_nWeaponTypeL == 12
                        || m_nWeaponTypeL == 62
                        || m_nWeaponTypeL == 31)
                    {
                        SetWeaponType(1);
                        if (!m_nWeaponTypeR)
                        {
                            m_bSwordShadow[1] = 1;
                        }
                        else if (nWeaponPosR == 128)
                        {
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 21
                        || m_nWeaponTypeL == 22
                        || m_nWeaponTypeL == 23
                        || m_nWeaponTypeL == 13
                        || m_nWeaponTypeL == 3
                        || m_nWeaponTypeL == 63
                        || m_nWeaponTypeL == 32
                        || m_nWeaponTypeL == 33)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(4);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else if (m_nWeaponTypeL == 101)
                    {
                        SetWeaponType(5);
                        m_bSwordShadow[1] = 1;
                    }
                    else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                    {
                        if (nWeaponPosL == 64)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                    }
                    else
                    {
                        if (m_nWeaponTypeL == 41)
                        {
                            m_bSwordShadow[1] = 1;
                            m_bSwordShadow[0] = 1;
                        }
                        SetWeaponType(0);
                    }
                    break;
                    case 2:
                        if (!m_cMount)
                        {
                            if (m_nWeaponTypeL == 101)
                            {
                                SetWeaponType(1);
                                m_bSwordShadow[1] = 0;
                            }
                            else if (m_nWeaponTypeL == 12)
                            {
                                SetWeaponType(2);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (nWeaponPosR == 128 || m_nWeaponTypeL == 1 || m_nWeaponTypeL == 11)
                            {
                                SetWeaponType(5);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (m_nWeaponTypeL == 11
                                || m_nWeaponTypeL == 12
                                || m_nWeaponTypeL == 13
                                || m_nWeaponTypeL == 21)
                            {
                                SetWeaponType(3);
                                m_bSwordShadow[1] = 1;
                            }
                            else if (m_nWeaponTypeL == 31
                                || m_nWeaponTypeL == 32
                                || m_nWeaponTypeL == 33)
                            {
                                SetWeaponType(4);
                                m_bSwordShadow[1] = 1;
                            }
                            else
                            {
                                SetWeaponType(0);
                                if (nWeaponPosL)
                                    m_bSwordShadow[1] = 1;
                                else
                                    m_bSwordShadow[1] = 0;
                            }
                        }
                        else if (m_nWeaponTypeL == 101)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 0;
                        }
                        else if (m_nWeaponTypeL == 1
                            || m_nWeaponTypeL == 11
                            || m_nWeaponTypeL == 12
                            || m_nWeaponTypeL == 13
                            || m_nWeaponTypeL == 31
                            || m_nWeaponTypeL == 32
                            || m_nWeaponTypeL == 33)
                        {
                            SetWeaponType(5);
                            m_bSwordShadow[1] = 1;
                        }
                        else
                        {
                            SetWeaponType(0);
                            if (nWeaponPosL)
                                m_bSwordShadow[1] = 1;
                            else
                                m_bSwordShadow[1] = 0;
                        }
                        break;
                    case 3:
                        if (m_cMount)
                        {
                            SetWeaponType(0);
                        }
                        else if (m_stLookInfo.FaceMesh)
                        {
                            if (m_stLookInfo.FaceMesh == 1)
                                SetWeaponType(1);
                        }
                        else
                        {
                            SetWeaponType(0);
                        }
                        break;
                    case 4:
                        if (m_nWeaponTypeL == 1
                            || m_nWeaponTypeL == 11
                            || m_nWeaponTypeL == 61)
                        {
                            SetWeaponType(1);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 21
                            || m_nWeaponTypeL == 22
                            || m_nWeaponTypeL == 23
                            || m_nWeaponTypeL == 13)
                        {
                            SetWeaponType(2);
                            m_bSwordShadow[1] = 1;
                        }
                        else if (m_nWeaponTypeL == 102 || m_nWeaponTypeL == 103)
                        {
                            SetWeaponType(3);
                            m_bSwordShadow[1] = 0;
                        }
                        else
                        {
                            SetWeaponType(0);
                            if (nWeaponPosL)
                                m_bSwordShadow[1] = 1;
                            else
                                m_bSwordShadow[1] = 0;
                        }
                        break;
                }
            }
            if (m_nWeaponTypeL == 41 && m_pSkinMesh)
            {
                m_pSkinMesh->m_cRotate[1] = 1;
            }
            else if (m_pSkinMesh)
            {
                m_pSkinMesh->m_cRotate[1] = 0;
            }
            if (m_nSkinMeshType == 11)
            {
                m_pSkinMesh->m_cRotate[1] = 1;
            }
            else if (m_nSkinMeshType == 10)
            {
                m_pSkinMesh->m_cRotate[0] = 1;
                SetWeaponType(0);
                if (m_sRightIndex)
                    m_bSwordShadow[0] = 1;
                if (m_sLeftIndex)
                    m_bSwordShadow[1] = 1;
            }
            if ((int)m_eMotion < 14)
                SetAnimation(ECHAR_MOTION::ECMOTION_STAND02, 1);
        }
    }
}

void TMHuman::SetPacketMOBItem(STRUCT_MOB* pMobData)
{
    if (!m_dwDelayDel)
    {
        m_sHeadIndex = pMobData->Equip[0].sIndex;
        m_sHelmIndex = pMobData->Equip[1].sIndex;
        m_citizen = static_cast<unsigned char>(pMobData->Equip[0].stEffect[2].cValue);
        m_cLegend = static_cast<char>(g_pItemList[pMobData->Equip[0].sIndex].nGrade);
        m_stLookInfo.FaceMesh = g_pItemList[pMobData->Equip[0].sIndex].nIndexMesh;
        m_stLookInfo.FaceSkin = g_pItemList[pMobData->Equip[0].sIndex].nIndexTexture;

        if (pMobData->Equip[1].sIndex >= 3500 && (pMobData->Equip[1].sIndex <= 3502 || pMobData->Equip[1].sIndex == 3507))
        {
            m_stLookInfo.HelmMesh = 0;
            m_stLookInfo.HelmSkin = 0;
        }
        else
        {
            m_stLookInfo.HelmMesh = g_pItemList[pMobData->Equip[1].sIndex].nIndexMesh;
            m_stLookInfo.HelmSkin = g_pItemList[pMobData->Equip[1].sIndex].nIndexTexture;
        }

        m_stLookInfo.CoatMesh = g_pItemList[pMobData->Equip[2].sIndex].nIndexMesh;
        m_stLookInfo.CoatSkin = g_pItemList[pMobData->Equip[2].sIndex].nIndexTexture;
        m_stLookInfo.PantsMesh = g_pItemList[pMobData->Equip[3].sIndex].nIndexMesh;
        m_stLookInfo.PantsSkin = g_pItemList[pMobData->Equip[3].sIndex].nIndexTexture;
        m_stLookInfo.GlovesMesh = g_pItemList[pMobData->Equip[4].sIndex].nIndexMesh;
        m_stLookInfo.GlovesSkin = g_pItemList[pMobData->Equip[4].sIndex].nIndexTexture;
        m_stLookInfo.BootsMesh = g_pItemList[pMobData->Equip[5].sIndex].nIndexMesh;
        m_stLookInfo.BootsSkin = g_pItemList[pMobData->Equip[5].sIndex].nIndexTexture;
        m_stLookInfo.LeftMesh = g_pItemList[pMobData->Equip[6].sIndex].nIndexMesh;
        m_stLookInfo.LeftSkin = g_pItemList[pMobData->Equip[6].sIndex].nIndexTexture;
        m_stLookInfo.RightMesh = g_pItemList[pMobData->Equip[7].sIndex].nIndexMesh;
        m_stLookInfo.RightSkin = g_pItemList[pMobData->Equip[7].sIndex].nIndexTexture;

        if (pMobData->Equip[15].sIndex > 0)
        {
            m_sMantuaIndex = pMobData->Equip[15].sIndex;
            m_wMantuaSkin = g_pItemList[pMobData->Equip[15].sIndex].nIndexTexture;
            SetMantua(m_wMantuaSkin);
            m_ucMantuaLegend = static_cast<char>(g_pItemList[m_sMantuaIndex].nGrade);
            m_ucMantuaSanc = m_nTotalKill / 100;
            if ((unsigned char)m_ucMantuaSanc > 9)
                m_ucMantuaSanc = 9;
        }
        else
            m_cMantua = 0;

		// The rebuilt client speaks the 7.48 equipment ABI: costume is slot 13,
		// mount is 14 and cape is 15.  The imported 7.59 familiar/costume mapping
		// must not reinterpret the 7.48 costume as a pet.
		m_sFamiliar = 0;
		m_sFamCount = 0;
		m_sCostume = pMobData->Equip[13].sIndex;
        m_stSancInfo.Sanc0 = BASE_GetItemSanc(pMobData->Equip);
        m_stSancInfo.Sanc1 = BASE_GetItemSanc(&pMobData->Equip[1]);
        m_stSancInfo.Sanc2 = BASE_GetItemSanc(&pMobData->Equip[2]);
        m_stSancInfo.Sanc3 = BASE_GetItemSanc(&pMobData->Equip[3]);
        m_stSancInfo.Sanc4 = BASE_GetItemSanc(&pMobData->Equip[4]);
        m_stSancInfo.Sanc5 = BASE_GetItemSanc(&pMobData->Equip[5]);
        m_stSancInfo.Sanc7 = BASE_GetItemSanc(&pMobData->Equip[6]);
        m_stSancInfo.Sanc6 = BASE_GetItemSanc(&pMobData->Equip[7]);
        m_stColorInfo.Sanc0 = BASE_GetItemColorEffect(pMobData->Equip);
        m_stColorInfo.Sanc1 = BASE_GetItemColorEffect(&pMobData->Equip[1]);
        m_stColorInfo.Sanc2 = BASE_GetItemColorEffect(&pMobData->Equip[2]);
        m_stColorInfo.Sanc3 = BASE_GetItemColorEffect(&pMobData->Equip[3]);
        m_stColorInfo.Sanc4 = BASE_GetItemColorEffect(&pMobData->Equip[4]);
        m_stColorInfo.Sanc5 = BASE_GetItemColorEffect(&pMobData->Equip[5]);
        m_stColorInfo.Sanc7 = BASE_GetItemColorEffect(&pMobData->Equip[6]);
        m_stColorInfo.Sanc6 = BASE_GetItemColorEffect(&pMobData->Equip[7]);
		SetGuildBattleLifeCount();
		m_cDamageRate = 1;
		// Familiar-only damage metadata from TMProject 7.59 cannot be read from
		// slot 13 because that byte range carries a 7.48 costume item.
		if (pMobData->Equip[11].sIndex == 786)
			m_cDamageRate += 16 * BASE_GetItemSanc(&pMobData->Equip[11]);

        bool bSvadilfari = false;
        int tempIndex = pMobData->Equip[14].sIndex;
        // STRUCT_ITEM preserves the complete mount item ID. Consume imported KR
        // IDs before the legacy range mapper can reinterpret their low 12 bits.
        const bool hasImportedMount = SetImportedMountCostume(tempIndex);

        if (pMobData->Equip[14].sIndex == 2383 || pMobData->Equip[14].sIndex == 2382)
            pMobData->Equip[14].sIndex = 2387;
        else if (pMobData->Equip[14].sIndex == 2387)
            bSvadilfari = true;

        if (hasImportedMount)
        {
            // The table has already populated m_stMountLook/m_stMountSanc; the
            // common InitObject path below will materialize the TMSkinMesh.
        }
        else if (tempIndex >= 2360 && tempIndex < 2390
            || tempIndex >= 3980 && tempIndex < 3999
            || tempIndex >= 2960 && tempIndex < 3000)
        {
            int nMountHP = BASE_GetItemAbility(&pMobData->Equip[14], 80);
            if (nMountHP > 0
                || pMobData->Equip[14].sIndex >= 2960 && pMobData->Equip[14].sIndex < 2999
                || pMobData->Equip[14].sIndex >= 3980 && pMobData->Equip[14].sIndex < 3999)
            {
                m_cLastMount = m_cMount;
                m_cMount = 1;
                int sIndex = pMobData->Equip[14].sIndex - 2045;
                int _nEquipIdx = pMobData->Equip[14].sIndex;

                if (_nEquipIdx == 2389)
                    sIndex = 346;
                else if (_nEquipIdx == 2378)
                    sIndex = 333;
                else if (_nEquipIdx >= 2387 && _nEquipIdx <= 2388)
                    sIndex = 336;
                else if (_nEquipIdx >= 3980 && _nEquipIdx <= 3982)
                    sIndex = _nEquipIdx - 3638;
                else if (_nEquipIdx >= 3983 && _nEquipIdx <= 3985)
                    sIndex = _nEquipIdx - 3641;
                else if (_nEquipIdx >= 3986 && _nEquipIdx <= 3988)
                    sIndex = _nEquipIdx - 3644;
                else if(_nEquipIdx == 3989)
                    sIndex = 345;
                else if (_nEquipIdx == 3990)
                    sIndex = 334;
                else if (_nEquipIdx == 3991)
                    sIndex = 335;
                else if (_nEquipIdx == 3992)
                    sIndex = 318;
                else  if (_nEquipIdx == 3993 || _nEquipIdx == 3994)
                    sIndex = 200;
                else if (_nEquipIdx == 3995)
                    sIndex = 352;
                else if (_nEquipIdx == 3996)
                    sIndex = 389;
                else if (_nEquipIdx == 3997)
                    sIndex = 293;
                else if (_nEquipIdx >= 2960 && _nEquipIdx <= 2961)
                    sIndex = _nEquipIdx - 2616;

                STRUCT_ITEM item{};

                item.sIndex = sIndex;
                int nMountSanc = BASE_GetItemAbility(&pMobData->Equip[14], 81) / 10;
                int nClass = BASE_GetItemAbility(&item, 18);
                m_nMountSkinMeshType = BASE_DefineSkinMeshType(nClass);
                m_stMountLook.Mesh0 = g_pItemList[sIndex].nIndexMesh;
                m_stMountLook.Mesh1 = m_stMountLook.Mesh0;
                m_stMountLook.Skin0 = g_pItemList[sIndex].nIndexTexture;
                m_stMountLook.Skin1 = m_stMountLook.Skin0;
                m_sMountIndex = sIndex - 315;
                if (_nEquipIdx == 3993)
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = 12;
                    m_stMountSanc.Sanc1 = 12;
                }
                else if (_nEquipIdx == 3994)
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = 12;
                    m_stMountSanc.Sanc1 = 12;
                    m_stMountLook.Skin0 = 8;
                    m_stMountLook.Skin1 = 8;
                }
                else if (_nEquipIdx == 3995)
                {
                    m_nMountSkinMeshType = 59;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc1 = 0;
                    m_stMountSanc.Sanc0 = 0;

                    m_fMountScale = 1.0;
                }
                else if (_nEquipIdx == 3996)
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;

                }
                else if (_nEquipIdx == 3997)
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = 12;
                    m_stMountSanc.Sanc1 = 12;

                }
                else if (sIndex >= 321 && sIndex <= 325)
                {
                    m_stMountLook.Mesh2 = sIndex - 320;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                }
                else if (sIndex >= 326 && sIndex <= 330)
                {
                    m_stMountLook.Mesh2 = sIndex - 325;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                }
                else if (sIndex == 334 || sIndex == 335)
                {
                    m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }
                else if (sIndex >= 336 && sIndex <= 338)
                {
                    if (bSvadilfari)
                    {
                        m_stMountLook.Mesh0 = 10;
                        m_stMountLook.Mesh1 = 10;
                        m_stMountLook.Mesh2 = 11;
                        m_stMountSanc.Sanc2 = nMountSanc;
                        m_stMountSanc.Sanc0 = 0;
                        m_stMountSanc.Sanc1 = 0;
                    }
                    else
                    {
                        m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                        m_stMountSanc.Sanc2 = nMountSanc;
                        m_stMountSanc.Sanc0 = nMountSanc;
                        m_stMountSanc.Sanc1 = nMountSanc;
                    }
                }
                else if (sIndex >= 339 && sIndex <= 341)
                {
                    m_stMountLook.Mesh2 = sIndex + m_stMountLook.Mesh0 - 339;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }
                else if (sIndex >= 342 && sIndex <= 345)
                {
                    switch (sIndex)
                    {
                    case 344:
                        m_stMountLook.Mesh2 = 10;
                        break;
                    case 345:
                        m_stMountLook.Mesh2 = 11;
                        break;
                    case 343:
                        m_stMountLook.Mesh2 = 11;
                        break;
                    default:
                        m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                        break;
                    }
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    if (sIndex == 345)
                        m_stMountSanc.Sanc2 = 7;
                }
                else
                {
                    m_stMountLook.Mesh2 = 0;
                    m_stMountSanc.Sanc2 = 0;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }

                if (_nEquipIdx >= 2387 && _nEquipIdx <= 2388 && !bSvadilfari)
                    m_stMountLook.Mesh2 = _nEquipIdx - 2379;

                m_fMountScale = BASE_GetMountScale(m_nMountSkinMeshType, m_stMountLook.Mesh0);
                if (sIndex == 333)
                    m_fMountScale = 1.1f;

                if (!m_cLastMount && m_nMountSkinMeshType == 31)
                {
                    auto pSoundManager = g_pSoundManager;

                    if (pSoundManager)
                    {
                        auto pSoundData = pSoundManager->GetSoundData(276);
                        if (pSoundData && !pSoundData->IsSoundPlaying())
                            pSoundData->Play();
                    }
                    m_cLastMount = m_cMount;
                }

                int nMountMaxHPIndex = sIndex - 315;
                if (sIndex - 315 < 0)
                    nMountMaxHPIndex = 0;
                switch (_nEquipIdx)
                {
                case 2378:
                    nMountMaxHPIndex = 18;
                    break;
                case 2379:
                    nMountMaxHPIndex = 19;
                    break;
                case 2380:
                    nMountMaxHPIndex = 21;
                    break;
                case 2381:
                    nMountMaxHPIndex = 20;
                    break;
                case 2382:
                case 2383:
                case 2384:
                case 2385:
                case 2386:
                    break;
                case 2387:
                    nMountMaxHPIndex = 20;
                    break;
                case 2388:
                    nMountMaxHPIndex = 21;
                    break;
                case 2389:
                    nMountMaxHPIndex = 22;
                    break;
                }
                // The 7.48 compatibility field does not create the optional
                // mount progress control; keep materializing the character
                // and mount mesh even when that HUD bar is unavailable.
                if (m_pMountHPBar)
                {
                    m_pMountHPBar->SetMaxProgress(g_nMountHPTable[nMountMaxHPIndex]);
                    m_pMountHPBar->SetCurrentProgress(nMountHP);
                }

                TMFieldScene* pFSCene = (TMFieldScene*)g_pCurrentScene;
                // The compact 7.48 field scene has no mount HUD controls; the
                // visual mount must still initialize when those optional bars
                // are absent, so each control is checked before it is updated.
                if (pFSCene && pFSCene->m_pMHPBar && pFSCene->m_pMaxMHPText && pFSCene->m_pCurrentMHPText)
                {
                    pFSCene->m_pMHPBar->SetMaxProgress(g_nMountHPTable[nMountMaxHPIndex]);
                    pFSCene->m_pMHPBar->SetCurrentProgress(nMountHP);

                    char szMHP[32]{};
                    sprintf(szMHP, "%d", g_nMountHPTable[nMountMaxHPIndex]);
                    pFSCene->m_pMaxMHPText->SetText(szMHP, 0);

                    sprintf(szMHP, "%d", nMountHP);
                    pFSCene->m_pCurrentMHPText->SetText(szMHP, 0);
                }

                if (_nEquipIdx && (tempIndex < 3980 || tempIndex >= 3999))
                    SetMountCostume((unsigned char)pMobData->Equip[14].stEffect[2].cValue);
            }
            else
                m_cMount = 0;
        }
        else
        {
            m_cMount = 0;
            TMFieldScene* pFScene = (TMFieldScene*)g_pCurrentScene;
            // The legacy 7.48 HUD omits these mount text controls, so a normal
            // unmounted entity must not dereference them during materialization.
            if (pFScene && !pFScene->m_bAirMove && pFScene->m_pMaxMHPText && pFScene->m_pCurrentMHPText)
            {
                char buffer[4];
                sprintf(buffer, "%d", 0);

                pFScene->m_pMaxMHPText->SetText(buffer, 0);
                sprintf(buffer, "%d", 0);
                pFScene->m_pCurrentMHPText->SetText(buffer, 0);
            }

            // Clearing a mount is valid during bootstrap when the legacy
            // resource intentionally provides no mount status panel.
            if(pFScene && pFScene->m_pMHPBar)
                pFScene->m_pMHPBar->SetCurrentProgress(0);
        }

        pMobData->Equip[14].sIndex = tempIndex;
        m_stSancInfo.Legend0 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[0].sIndex].nGrade);
        m_stSancInfo.Legend1 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[1].sIndex].nGrade);
        m_stSancInfo.Legend2 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[2].sIndex].nGrade);
        m_stSancInfo.Legend3 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[3].sIndex].nGrade);
        m_stSancInfo.Legend4 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[4].sIndex].nGrade);
        m_stSancInfo.Legend5 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[5].sIndex].nGrade);
        m_stSancInfo.Legend7 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[6].sIndex].nGrade);
        m_stSancInfo.Legend6 = static_cast<unsigned char>(g_pItemList[pMobData->Equip[7].sIndex].nGrade);

        if ((unsigned char)m_stSancInfo.Legend0 <= 4
            && (unsigned char)m_stSancInfo.Sanc0 > 9)
        {
            m_stSancInfo.Legend0 = BASE_GetItemTenColor(pMobData->Equip) + 4;
        }
        else if (m_stSancInfo.Legend0 == 4 && (unsigned char)m_stSancInfo.Sanc0 > 9)
        {
            m_stSancInfo.Legend0 = BASE_GetItemTenColor(pMobData->Equip);
        }
        if ((unsigned char)m_stSancInfo.Legend1 <= 4
            && (unsigned char)m_stSancInfo.Sanc1 > 9)
        {
            m_stSancInfo.Legend1 = BASE_GetItemTenColor(&pMobData->Equip[1]) + 4;
        }
        else if (m_stSancInfo.Legend1 == 4 && (unsigned char)m_stSancInfo.Sanc1 > 9)
        {
            m_stSancInfo.Legend1 = BASE_GetItemTenColor(&pMobData->Equip[1]);
        }

        if ((unsigned char)m_stSancInfo.Legend2 <= 4
            && (unsigned char)m_stSancInfo.Sanc2 > 9)
        {
            m_stSancInfo.Legend2 = BASE_GetItemTenColor(&pMobData->Equip[2]) + 4;
        }
        else if (m_stSancInfo.Legend2 == 4 && (unsigned char)m_stSancInfo.Sanc2 > 9)
        {
            m_stSancInfo.Legend2 = BASE_GetItemTenColor(&pMobData->Equip[2]);
        }
        if ((unsigned char)m_stSancInfo.Legend3 <= 4
            && (unsigned char)m_stSancInfo.Sanc3 > 9)
        {
            m_stSancInfo.Legend3 = BASE_GetItemTenColor(&pMobData->Equip[3]) + 4;
        }
        else if (m_stSancInfo.Legend3 == 4 && (unsigned char)m_stSancInfo.Sanc3 > 9)
        {
            m_stSancInfo.Legend3 = BASE_GetItemTenColor(&pMobData->Equip[3]);
        }
        if ((unsigned char)m_stSancInfo.Legend4 <= 4
            && (unsigned char)m_stSancInfo.Sanc4 > 9)
        {
            m_stSancInfo.Legend4 = BASE_GetItemTenColor(&pMobData->Equip[4]) + 4;
        }
        else if (m_stSancInfo.Legend4 == 4 && (unsigned char)m_stSancInfo.Sanc4 > 9)
        {
            m_stSancInfo.Legend4 = BASE_GetItemTenColor(&pMobData->Equip[4]);
        }
        if ((unsigned char)m_stSancInfo.Legend5 <= 4
            && (unsigned char)m_stSancInfo.Sanc5 > 9)
        {
            m_stSancInfo.Legend5 = BASE_GetItemTenColor(&pMobData->Equip[5]) + 4;
        }
        else if (m_stSancInfo.Legend5 == 4 && (unsigned char)m_stSancInfo.Sanc5 > 9)
        {
            m_stSancInfo.Legend5 = BASE_GetItemTenColor(&pMobData->Equip[5]);
        }
        if ((unsigned char)m_stSancInfo.Legend7 <= 4
            && (unsigned char)m_stSancInfo.Sanc7 > 9)
        {
            m_stSancInfo.Legend7 = BASE_GetItemTenColor(&pMobData->Equip[6]) + 4;
        }
        else if (m_stSancInfo.Legend7 == 4 && (unsigned char)m_stSancInfo.Sanc7 > 9)
        {
            m_stSancInfo.Legend7 = BASE_GetItemTenColor(&pMobData->Equip[6]);
        }
        if ((unsigned char)m_stSancInfo.Legend6 <= 4
            && (unsigned char)m_stSancInfo.Sanc6 > 9)
        {
            m_stSancInfo.Legend6 = BASE_GetItemTenColor(&pMobData->Equip[7]) + 4;
        }
        else if (m_stSancInfo.Legend6 == 4 && (unsigned char)m_stSancInfo.Sanc6 > 9)
        {
            m_stSancInfo.Legend6 = BASE_GetItemTenColor(&pMobData->Equip[7]);
        }

        TMFieldScene* pFScene = nullptr;
        if (g_pCurrentScene && g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_FIELD)
            pFScene = (TMFieldScene*)g_pCurrentScene;

        if (g_pCurrentScene && g_pCurrentScene->m_pMyHuman == this && pFScene)
        {
            int nValue = BASE_GetStaticItemAbility(&pMobData->Equip[14], 80);
            if (pMobData->Equip[14].sIndex < 3980 || pMobData->Equip[14].sIndex >= 3999)
                pFScene->m_bMountDead = nValue <= 0;
            else
                pFScene->m_bMountDead = 0;
        }

        m_stOldSancInfo = m_stSancInfo;
        m_stOldColorInfo = m_stColorInfo;
        m_stOldMountSanc = m_stMountSanc;
    }
}

void TMHuman::SetPacketEquipItem(unsigned short* sEquip)
{
    if (!m_dwDelayDel)
    {
        m_sHeadIndex = sEquip[0] & 0xFFF;
        m_sHelmIndex = sEquip[1] & 0xFFF;
        m_cLegend = static_cast<char>(g_pItemList[sEquip[0] & 0xFFF].nGrade);
        m_stLookInfo.FaceMesh = g_pItemList[sEquip[0] & 0xFFF].nIndexMesh;
        m_stLookInfo.FaceSkin = g_pItemList[sEquip[0] & 0xFFF].nIndexTexture;
        if ((sEquip[1] & 0xFFF) < 3500 || (sEquip[1] & 0xFFF) > 3502 && (sEquip[1] & 0xFFF) != 3507)
        {
            m_stLookInfo.HelmMesh = g_pItemList[sEquip[1] & 0xFFF].nIndexMesh;
            m_stLookInfo.HelmSkin = g_pItemList[sEquip[1] & 0xFFF].nIndexTexture;
        }
        m_stLookInfo.CoatMesh = g_pItemList[sEquip[2] & 0xFFF].nIndexMesh;
        m_stLookInfo.CoatSkin = g_pItemList[sEquip[2] & 0xFFF].nIndexTexture;
        m_stLookInfo.PantsMesh = g_pItemList[sEquip[3] & 0xFFF].nIndexMesh;
        m_stLookInfo.PantsSkin = g_pItemList[sEquip[3] & 0xFFF].nIndexTexture;
        m_stLookInfo.GlovesMesh = g_pItemList[sEquip[4] & 0xFFF].nIndexMesh;
        m_stLookInfo.GlovesSkin = g_pItemList[sEquip[4] & 0xFFF].nIndexTexture;
        m_stLookInfo.BootsMesh = g_pItemList[sEquip[5] & 0xFFF].nIndexMesh;
        m_stLookInfo.BootsSkin = g_pItemList[sEquip[5] & 0xFFF].nIndexTexture;
        m_stLookInfo.LeftMesh = g_pItemList[sEquip[6] & 0xFFF].nIndexMesh;
        m_stLookInfo.LeftSkin = g_pItemList[sEquip[6] & 0xFFF].nIndexTexture;
        m_stLookInfo.RightMesh = g_pItemList[sEquip[7] & 0xFFF].nIndexMesh;
        m_stLookInfo.RightSkin = g_pItemList[sEquip[7] & 0xFFF].nIndexTexture;
        if ((sEquip[15] & 0xFFF) > 0)
        {
            m_sMantuaIndex = sEquip[15] & 0xFFF;
            m_wMantuaSkin = g_pItemList[sEquip[15] & 0xFFF].nIndexTexture;
            SetMantua(m_wMantuaSkin);
            m_ucMantuaLegend = static_cast<char>(g_pItemList[sEquip[15] & 0xFFF].nGrade);
            m_ucMantuaSanc = m_nTotalKill / 10;
            if ((unsigned char)m_ucMantuaSanc > 12)
                m_ucMantuaSanc = 12;
        }
        else
        {
            m_cMantua = 0;
        }

		// The compact 7.48 CreateMob projection uses slot 13 for costume, 14 for
		// mount and 15 for cape.  Do not reinterpret a costume as the 7.59 familiar.
		// Unlike the refined armor slots, the 7.48 costume value is a full item ID;
		// preserving all 16 bits is required by imported costumes above index 4095.
		const int nCostumeIndex = sEquip[13];
		m_sCostume = nCostumeIndex;
		m_sFamiliar = 0;
        m_stSancInfo.Sanc0 = (int)sEquip[0] >> 12;
        m_stSancInfo.Sanc1 = (int)sEquip[1] >> 12;
        m_stSancInfo.Sanc2 = (int)sEquip[2] >> 12;
        m_stSancInfo.Sanc3 = (int)sEquip[3] >> 12;
        m_stSancInfo.Sanc4 = (int)sEquip[4] >> 12;
        m_stSancInfo.Sanc5 = (int)sEquip[5] >> 12;
        m_stSancInfo.Sanc7 = (int)sEquip[6] >> 12;
        m_stSancInfo.Sanc6 = (int)sEquip[7] >> 12;

        if (g_pCurrentScene->m_pMyHuman == this)
            g_pObjectManager->m_stMobData.Equip[0].stEffect[0].cValue = m_stSancInfo.Sanc0;

		m_sFamCount = 0;

		SetGuildBattleLifeCount();
		m_cDamageRate = 1;
		// Familiar damage-rate metadata is deliberately absent from the 7.48
		// equipment ABI; slot 11 keeps its independent native effect below.
        if ((sEquip[11] & 0xFFF) == 786)
            m_cDamageRate += 16 * ((int)sEquip[11] >> 12);

        // Imported KR item IDs intentionally exceed the native 12-bit table.
        // Resolve them before applying the mask used only by legacy 7.48 mounts.
        const unsigned int fullMountIndex = sEquip[14];
        const bool hasImportedMount = SetImportedMountCostume(fullMountIndex);
        int nMountIndex = fullMountIndex & 0xFFF;
        bool bSvadilfari = false;
        if (nMountIndex == 2383 || nMountIndex == 2382)
        {
            nMountIndex = 2387;
        }
        else if (nMountIndex == 2387)
        {
            bSvadilfari = 1;
        }

        if (hasImportedMount)
        {
            // SetImportedMountCostume owns the complete visual contract.
        }
        else if (nMountIndex >= 2360 && nMountIndex < 2390
            || nMountIndex >= 3980 && nMountIndex < 3999
            || nMountIndex >= 2960 && nMountIndex < 3000)
        {
            m_cMount = 1;
            int sIndex = nMountIndex - 2045;
            if (nMountIndex == 2389)
                sIndex = 346;
            else if (nMountIndex == 2378)
                sIndex = 333;
            else if (nMountIndex >= 2387 && nMountIndex <= 2388)
                sIndex = 336;
            else if (nMountIndex >= 3980 && nMountIndex <= 3982)
                sIndex = nMountIndex - 3638;
            else if (nMountIndex >= 3983 && nMountIndex <= 3985)
                sIndex = nMountIndex - 3641;
            else if (nMountIndex >= 3986 && nMountIndex <= 3988)
                sIndex = nMountIndex - 3644;
            else if (nMountIndex == 3989)
                sIndex = 345;
            else if (nMountIndex == 3990)
                sIndex = 334;
            else if (nMountIndex == 3991)
                sIndex = 335;
            else if (nMountIndex == 3992)
                sIndex = 318;
            else  if (nMountIndex == 3993 || nMountIndex == 3994)
                sIndex = 200;
            else if (nMountIndex == 3995)
                sIndex = 352;
            else if (nMountIndex == 3996)
                sIndex = 389;
            else if (nMountIndex == 3997)
                sIndex = 293;
            else if (nMountIndex >= 2960 && nMountIndex <= 2961)
                sIndex = nMountIndex - 2616;

            STRUCT_ITEM item{};

            item.sIndex = sIndex;
            m_sMountIndex = sIndex - 315;

            int nMountSanc = (int)sEquip[14] >> 12;
            int nClass = BASE_GetItemAbility(&item, 18);

            m_nMountSkinMeshType = BASE_DefineSkinMeshType(nClass);
            m_stMountLook.Mesh0 = g_pItemList[sIndex].nIndexMesh;
            m_stMountLook.Mesh1 = m_stMountLook.Mesh0;
            m_stMountLook.Skin0 = g_pItemList[sIndex].nIndexTexture;
            m_stMountLook.Skin1 = m_stMountLook.Skin0;
            m_sMountIndex = sIndex - 315;
            if (nMountIndex == 3993)
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = 12;
                m_stMountSanc.Sanc1 = 12;
            }
            else if (nMountIndex == 3994)
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = 12;
                m_stMountSanc.Sanc1 = 12;
                m_stMountLook.Skin0 = 8;
                m_stMountLook.Skin1 = 8;
            }
            else if (nMountIndex == 3995)
            {
                m_nMountSkinMeshType = 59;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc1 = 0;
                m_stMountSanc.Sanc0 = 0;

                m_fMountScale = 1.0;
            }
            else if (nMountIndex == 3996)
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = 0;
                m_stMountSanc.Sanc1 = 0;

            }
            else if (nMountIndex == 3997)
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = 12;
                m_stMountSanc.Sanc1 = 12;

            }
            else if (sIndex >= 321 && sIndex <= 325)
            {
                m_stMountLook.Mesh2 = sIndex - 320;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = 0;
                m_stMountSanc.Sanc1 = 0;
            }
            else if (sIndex >= 326 && sIndex <= 330)
            {
                m_stMountLook.Mesh2 = sIndex - 325;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = 0;
                m_stMountSanc.Sanc1 = 0;
            }
            else if (sIndex == 334 || sIndex == 335)
            {
                m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = nMountSanc;
                m_stMountSanc.Sanc1 = nMountSanc;
            }
            else if (sIndex >= 336 && sIndex <= 338)
            {
                if (bSvadilfari)
                {
                    m_stMountLook.Mesh0 = 10;
                    m_stMountLook.Mesh1 = 10;
                    m_stMountLook.Mesh2 = 11;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = 0;
                    m_stMountSanc.Sanc1 = 0;
                }
                else
                {
                    m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                    m_stMountSanc.Sanc2 = nMountSanc;
                    m_stMountSanc.Sanc0 = nMountSanc;
                    m_stMountSanc.Sanc1 = nMountSanc;
                }
            }
            else if (sIndex >= 339 && sIndex <= 341)
            {
                m_stMountLook.Mesh2 = sIndex + m_stMountLook.Mesh0 - 339;
                m_stMountSanc.Sanc2 = nMountSanc;
                m_stMountSanc.Sanc0 = nMountSanc;
                m_stMountSanc.Sanc1 = nMountSanc;
            }
            else if (sIndex >= 342 && sIndex <= 345)
            {
                switch (sIndex)
                {
                case 344:
                    m_stMountLook.Mesh2 = 10;
                    break;
                case 345:
                    m_stMountLook.Mesh2 = 11;
                    break;
                case 343:
                    m_stMountLook.Mesh2 = 11;
                    break;
                default:
                    m_stMountLook.Mesh2 = m_stMountLook.Mesh0;
                    break;
                }
                m_stMountSanc.Sanc0 = 0;
                m_stMountSanc.Sanc1 = 0;
                m_stMountSanc.Sanc2 = 0;
                if (sIndex == 345)
                    m_stMountSanc.Sanc2 = 7;
            }
            else
            {
                m_stMountLook.Mesh2 = 0;
                m_stMountSanc.Sanc2 = 0;
                m_stMountSanc.Sanc0 = nMountSanc;
                m_stMountSanc.Sanc1 = nMountSanc;
            }

            if (nMountIndex >= 2387 && nMountIndex <= 2388 && !bSvadilfari)
                m_stMountLook.Mesh2 = nMountIndex - 2379;

            m_fMountScale = BASE_GetMountScale(m_nMountSkinMeshType, m_stMountLook.Mesh0);
            if (sIndex == 333)
                m_fMountScale = 1.0f;
        }
        else
            m_cMount = 0;

        m_stSancInfo.Legend0 = static_cast<unsigned char>(g_pItemList[sEquip[0] & 0xFFF].nGrade);
        m_stSancInfo.Legend1 = static_cast<unsigned char>(g_pItemList[sEquip[1] & 0xFFF].nGrade);
        m_stSancInfo.Legend2 = static_cast<unsigned char>(g_pItemList[sEquip[2] & 0xFFF].nGrade);
        m_stSancInfo.Legend3 = static_cast<unsigned char>(g_pItemList[sEquip[3] & 0xFFF].nGrade);
        m_stSancInfo.Legend4 = static_cast<unsigned char>(g_pItemList[sEquip[4] & 0xFFF].nGrade);
        m_stSancInfo.Legend5 = static_cast<unsigned char>(g_pItemList[sEquip[5] & 0xFFF].nGrade);
        m_stSancInfo.Legend7 = static_cast<unsigned char>(g_pItemList[sEquip[6] & 0xFFF].nGrade);
        m_stSancInfo.Legend6 = static_cast<unsigned char>(g_pItemList[sEquip[7] & 0xFFF].nGrade);

        m_stOldSancInfo = m_stSancInfo;
        m_stOldColorInfo = m_stColorInfo;
        m_stOldMountSanc = m_stMountSanc;
    }
}

void TMHuman::SetColorItem(char* sEquip2)
{
    char sEquipType = 0;
    char sEquipTypea = 0;
    char sEquipTypeb = 0;
    char sEquipTypec = 0;
    char sEquipTyped = 0;
    char sEquipTypee = 0;
    char sEquipTypef = 0;
    char sEquipTypeg = 0;
    if (!m_dwDelayDel)
    {
        if (m_stSancInfo.Sanc0 > 9)
        {
            m_stColorInfo.Sanc0 = *sEquip2 & 0xF;
            if (m_stColorInfo.Sanc0)
                m_stColorInfo.Sanc0 += 115;
        }
        else
        {
            m_stColorInfo.Sanc0 = *sEquip2;
        }
        if (m_stSancInfo.Sanc1 > 9)
        {
            m_stColorInfo.Sanc1 = sEquip2[1] & 0xF;
            if (m_stColorInfo.Sanc1)
                m_stColorInfo.Sanc1 += 115;
        }
        else
        {
            m_stColorInfo.Sanc1 = sEquip2[1];
        }
        if (m_stSancInfo.Sanc2 > 9)
        {
            m_stColorInfo.Sanc2 = sEquip2[2] & 0xF;
            if (m_stColorInfo.Sanc2)
                m_stColorInfo.Sanc2 += 115;
        }
        else
        {
            m_stColorInfo.Sanc2 = sEquip2[2];
        }
        if (m_stSancInfo.Sanc3 > 9)
        {
            m_stColorInfo.Sanc3 = sEquip2[3] & 0xF;
            if (m_stColorInfo.Sanc3)
                m_stColorInfo.Sanc3 += 115;
        }
        else
        {
            m_stColorInfo.Sanc3 = sEquip2[3];
        }
        if (m_stSancInfo.Sanc4 > 9)
        {
            m_stColorInfo.Sanc4 = sEquip2[4] & 0xF;
            if (m_stColorInfo.Sanc4)
                m_stColorInfo.Sanc4 += 115;
        }
        else
        {
            m_stColorInfo.Sanc4 = sEquip2[4];
        }
        if (m_stSancInfo.Sanc5 > 9)
        {
            m_stColorInfo.Sanc5 = sEquip2[5] & 0xF;
            if (m_stColorInfo.Sanc5)
                m_stColorInfo.Sanc5 += 115;
        }
        else
        {
            m_stColorInfo.Sanc5 = sEquip2[5];
        }
        if (m_stSancInfo.Sanc7 > 9)
        {
            m_stColorInfo.Sanc7 = sEquip2[6] & 0xF;
            if (m_stColorInfo.Sanc7)
                m_stColorInfo.Sanc7 += 115;
        }
        else
        {
            m_stColorInfo.Sanc7 = sEquip2[6];
        }
        if (m_stSancInfo.Sanc6 > 9)
        {
            m_stColorInfo.Sanc6 = sEquip2[7] & 0xF;
            if (m_stColorInfo.Sanc6)
                m_stColorInfo.Sanc6 += 115;
        }
        else
        {
            m_stColorInfo.Sanc6 = sEquip2[7];
        }
        sEquipType = *sEquip2 >> 4;
        if (!sEquipType || m_stSancInfo.Legend0 > 4 || m_stSancInfo.Sanc0 <= 9)
        {
            if (sEquipType && m_stSancInfo.Legend0 == 4 && m_stSancInfo.Sanc0 > 9)
                m_stSancInfo.Legend0 = sEquipType + 4;
        }
        else
        {
            m_stSancInfo.Legend0 = sEquipType + 8;
        }
        sEquipTypea = sEquip2[1] >> 4;
        if (!sEquipTypea || m_stSancInfo.Legend1 > 4 || m_stSancInfo.Sanc1 <= 9)
        {
            if (sEquipTypea && m_stSancInfo.Legend1 == 4 && m_stSancInfo.Sanc1 > 9)
                m_stSancInfo.Legend1 = sEquipTypea + 4;
        }
        else
        {
            m_stSancInfo.Legend1 = sEquipTypea + 8;
        }
        sEquipTypeb = sEquip2[2] >> 4;
        if (!sEquipTypeb || m_stSancInfo.Legend2 > 4 || m_stSancInfo.Sanc2 <= 9)
        {
            if (sEquipTypeb && m_stSancInfo.Legend2 == 4 && m_stSancInfo.Sanc2 > 9)
                m_stSancInfo.Legend2 = sEquipTypeb + 4;
        }
        else
        {
            m_stSancInfo.Legend2 = sEquipTypeb + 8;
        }
        sEquipTypec = sEquip2[3] >> 4;
        if (!sEquipTypec || m_stSancInfo.Legend3 > 4 || m_stSancInfo.Sanc3 <= 9)
        {
            if (sEquipTypec && m_stSancInfo.Legend3 == 4 && m_stSancInfo.Sanc3 > 9)
                m_stSancInfo.Legend3 = sEquipTypec + 4;
        }
        else
        {
            m_stSancInfo.Legend3 = sEquipTypec + 8;
        }
        sEquipTyped = sEquip2[4] >> 4;
        if (!sEquipTyped || m_stSancInfo.Legend4 > 4 || m_stSancInfo.Sanc4 <= 9)
        {
            if (sEquipTyped && m_stSancInfo.Legend4 == 4 && m_stSancInfo.Sanc4 > 9)
                m_stSancInfo.Legend4 = sEquipTyped + 4;
        }
        else
        {
            m_stSancInfo.Legend4 = sEquipTyped + 8;
        }
        sEquipTypee = sEquip2[5] >> 4;
        if (!sEquipTypee || m_stSancInfo.Legend5 > 4 || m_stSancInfo.Sanc5 <= 9)
        {
            if (sEquipTypee && m_stSancInfo.Legend5 == 4 && m_stSancInfo.Sanc5 > 9)
                m_stSancInfo.Legend5 = sEquipTypee + 4;
        }
        else
        {
            m_stSancInfo.Legend5 = sEquipTypee + 8;
        }
        sEquipTypef = sEquip2[6] >> 4;
        if (m_stSancInfo.Legend7 > 4 || m_stSancInfo.Sanc7 <= 9)
        {
            if (sEquipTypef && m_stSancInfo.Legend7 == 4 && m_stSancInfo.Sanc7 > 9)
                m_stSancInfo.Legend7 = sEquipTypef + 4;
        }
        else
        {
            m_stSancInfo.Legend7 = sEquipTypef + 8;
        }
        sEquipTypeg = sEquip2[7] >> 4;
        int result = m_stSancInfo.Legend6;
        if (result > 4 || m_stSancInfo.Sanc6 <= 9)
        {
            if (sEquipTypeg)
            {
                if (m_stSancInfo.Legend6 == 4)
                {
                    result = m_stSancInfo.Sanc6;
                    if (result > 9)
                    {
                        result = sEquipTypeg;
                        m_stSancInfo.Legend6 += sEquipTypeg + 4;
                    }
                }
            }
        }
        else
        {
            result = sEquipTypeg + 8;
            m_stSancInfo.Legend6 = result;
        }
    }
}

void TMHuman::GetLegType()
{
    if (!m_dwDelayDel)
    {
        int nSkinMeshType = m_nSkinMeshType;
        if (m_cMount == 1)
            nSkinMeshType = m_nMountSkinMeshType;

        if (nSkinMeshType)
        {
            switch (nSkinMeshType)
            {
            case 1:
                m_nLegType = 1;
                break;
            case 11:
                m_nLegType = 4;
                break;
            case 20:
                m_nLegType = 0;
                break;
            case 21:
                m_nLegType = 2;
                break;
            case 22:
                m_nLegType = 1;
                break;
            case 23:
                m_nLegType = 0;
                break;
            case 24:
                m_nLegType = 0;
                break;
            case 2:
                m_nLegType = 1;
                break;
            case 25:
                m_nLegType = 2;
                break;
            case 26:
                m_nLegType = 3;
                break;
            case 27:
                m_nLegType = 3;
                break;
            case 3:
                m_nLegType = 1;
                break;
            case 28:
                m_nLegType = 2;
                break;
            case 29:
                m_nLegType = 2;
                break;
            case 6:
                m_nLegType = 1;
                break;
            case 4:
                m_nLegType = 2;
                break;
            case 32:
                m_nLegType = 0;
                break;
            case 7:
                m_nLegType = 2;
                break;
            case 8:
                m_nLegType = 0;
                break;
            case 69:
                m_nLegType = 0;
                break;
            case 30:
                m_nLegType = 2;
                break;
            case 31:
                m_nLegType = 2;
                break;
            case 36:
                m_nLegType = 3;
                break;
            case 35:
                m_nLegType = 5;
                break;
            case 34:
                m_nLegType = 3;
                break;
            case 38:
                m_nLegType = 2;
                break;
            case 39:
                m_nLegType = 2;
                break;
            case 40:
                m_nLegType = 0;
                break;
            case 12:
                m_nLegType = 0;
                break;
            case 43:
                m_nLegType = 0;
                break;
            case 42:
                m_nLegType = 4;
                break;
            case 10:
                m_nLegType = 1;
                break;
            case 44:
                m_nLegType = 0;
                break;
            case 5:
                m_nLegType = m_stMountLook.Mesh0 == 1;
                break;
            }
        }
        else
        {
            m_nLegType = 1;
        }
    }
}

int TMHuman::GetBloodColor()
{
    int nBlood = 0;
    if (m_nClass <= 8)
        nBlood = 0;

    if (m_nClass == 25 && m_stLookInfo.FaceMesh == 3 && m_stLookInfo.FaceSkin == 8 || m_nClass == 25 && m_stLookInfo.FaceMesh == 12)
        nBlood = 89;

    if (m_nClass == 28 && m_stLookInfo.FaceMesh == 2)
        nBlood = 89;
    if (m_nClass == 16 && m_stLookInfo.FaceMesh == 6)
        nBlood = 89;

    switch (m_nSkinMeshType)
    {
    case 26:
        nBlood = 89;
        break;
    case 35:
        nBlood = 89;
        break;
    case 36:
        nBlood = 89;
        break;
    case 11:
        nBlood = 89;
        break;
    }

    if (m_nClass == 44 || m_nClass == 45)
        nBlood = 56;

    return nBlood;
}

void TMHuman::SetCharHeight(float fCon)
{
    float fRatio = 0.0f;
    if (m_dwID >= 0 && m_dwID < 1000 || g_pCurrentScene->m_eSceneType == ESCENE_TYPE::ESCENE_SELCHAR)
    {
        fRatio = 4000.0f;
        if (fCon > 500.0f && m_stScore.Level < 500)
            fCon = 500.0f;
    }
    else
    {
        fRatio = 2000.0;
    }

    m_fScale = ((fCon / fRatio) + 1.0f) * 0.89999998f;
}

void TMHuman::SetAvatar(char cAvatar)
{
    m_cAvatar = cAvatar;
}

void TMHuman::SetMantua(int nTexture)
{
    if (!nTexture
        || nTexture == 2
        || nTexture == 8
        || nTexture == 9
        || nTexture == 11
        || nTexture == 24
        || nTexture == 27
        || nTexture == 31
        || nTexture == 34)
    {
        m_cMantua = 1;
    }
    else if (nTexture == 1
        || nTexture == 3
        || nTexture == 12
        || nTexture == 13
        || nTexture == 14
        || nTexture == 25
        || nTexture == 28
        || nTexture == 30
        || nTexture == 32
        || nTexture == 35)
    {
        m_cMantua = 2;
    }
    else if (nTexture == 6
        || nTexture == 7
        || nTexture == 15
        || nTexture == 16
        || nTexture == 17
        || nTexture == 19
        || nTexture == 26
        || nTexture == 29
        || nTexture == 33
        || nTexture == 36)
    {
        m_cMantua = 3;
    }
    else
    {
        m_cMantua = 4;
    }
}

int TMHuman::SetCitizenMantle(int BaseSkin)
{
    if (m_citizen < 0 || m_citizen > 10)
        m_citizen = 0;
    if (!m_citizen)
        return BaseSkin;

    switch (BaseSkin)
    {
    case 34:
    case 35:
    case 36:
        return BaseSkin;
    case 19:
        return m_citizen + 39;
    case 3:
        return m_citizen + 49;
    case 2:
        return m_citizen + 59;
    case 6:
        return m_citizen + 69;
    case 1:
        return m_citizen + 79;
    case 0:
        return m_citizen + 89;
    case 7:
        return m_citizen + 99;
    case 25:
        return m_citizen + 109;
    case 24:
        return m_citizen + 119;
    case 26:
        return m_citizen + 129;
    case 28:
        return m_citizen + 139;
    case 27:
        return m_citizen + 149;
    case 29:
        return m_citizen + 159;
    case 32:
        return m_citizen + 169;
    case 31:
        return m_citizen + 179;
    case 33:
        BaseSkin = m_citizen + 189;
        break;
    }

    return BaseSkin;
}

int TMHuman::UnSetCitizenMantle(int BaseSkin)
{
    int mantle = BaseSkin / 10;
    if (BaseSkin / 10 == 4)
        return 19;

    switch (mantle)
    {
    case 5:
        return 3;
    case 6:
        return 2;
    case 7:
        return 6;
    case 8:
        return 1;
    case 9:
        return 0;
    case 10:
        return 7;
    case 11:
        return 25;
    case 12:
        return 24;
    case 13:
        return 26;
    case 14:
        return 28;
    case 15:
        return 27;
    case 16:
        return 29;
    case 17:
        return 32;
    case 18:
        return 31;
    case 19:
        BaseSkin = 33;
        break;
    }
    return BaseSkin;
}

int TMHuman::SetHumanCostume()
{
    int nCos = 0;
    m_nSkinMeshType = 0;
    memset(&m_stColorInfo, 0, 6); // Reset costume colors.
    memset(&m_stColorInfo.Legend0, 0, 6);

        if (m_sCostume <= 6301)
        {
            if (m_sCostume != 6301)
            {
                switch (m_sCostume)
                {

                case 4150:
                    nCos = 16;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4151:
                    nCos = -1;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4152:
                    nCos = 1;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4153:
                    nCos = 2;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4154:
                    nCos = 3;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4155:
                    nCos = 4;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4156:
                    nCos = 5;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4157:
                    nCos = 7;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4158:
                    nCos = 6;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4159:
                    nCos = 8;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4160:
                    nCos = 9;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4161:
                    nCos = 10;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4162:
                    nCos = 11;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4163:
                    nCos = 13;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4164:
                    nCos = 12;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4165:
                    nCos = 14;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4166:
                    nCos = 15;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4167:
                    nCos = 17;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4168:
                    nCos = 18;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4169:
                    nCos = 21;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4170:
                    nCos = 22;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4171:
                    nCos = 19;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4172:
                    nCos = 20;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4173:
                    nCos = 23;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4174:
                    nCos = 24;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4175:
                    nCos = 25;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4176:
                    nCos = 26;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4177:
                    nCos = 27;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4178:
                    nCos = 28;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4179:
                    nCos = 29;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4180:
                    nCos = 30;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4181:
                    nCos = 31;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4182:
                    nCos = 32;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4183:
                    nCos = 33;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4189:
                    nCos = 34;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4312:
                    nCos = 35;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4313:
                    nCos = 36;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4315:
                    nCos = 37;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4316:
                    nCos = 38;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4317:
                    nCos = 39;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4318:
                    nCos = 148;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4319:
                    nCos = 40;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4320:
                    nCos = 41;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4321:
                    nCos = 42;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4322:
                    nCos = 43;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4323:
                    nCos = 44;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4324:
                    nCos = 45;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4325:
                    nCos = 46;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4326:
                    nCos = 47;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4327:
                    nCos = 48;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4328:
                    nCos = 49;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4329:
                    nCos = 50;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4339:
                    nCos = 51;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4341:
                    nCos = 52;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4352:
                    nCos = 54;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4350:
                    nCos = 55;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4351:
                    nCos = 56;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4353:
                    nCos = 57;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4354:
                    nCos = 58;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4355:
                    nCos = 59;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4356:
                    nCos = 60;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4357:
                    nCos = 61;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4358:
                    nCos = 62;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4359:
                    nCos = 63;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4360:
                    nCos = 64;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4361:
                    nCos = 65;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4362:
                    nCos = 66;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4363:
                    nCos = 67;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4364:
                    nCos = 68;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4365:
                    nCos = 68;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4366:
                    nCos = 69;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4367:
                    nCos = 70;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4368:
                    nCos = 71;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4369:
                    nCos = 76;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4370:
                    nCos = 77;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4371:
                    nCos = 78;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4372:
                    nCos = 79;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4373:
                    nCos = 80;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4374:
                    nCos = 81;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4375:
                    nCos = 88;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4376:
                    nCos = 89;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4377:
                    nCos = 90;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4378:
                    nCos = 00;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4379:
                    nCos = 91;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4380:
                    nCos = 92;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4381:
                    nCos = 93;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4382:
                    nCos = 00;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4383:
                    nCos = 94;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4384:
                    nCos = 95;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4385:
                    nCos = 96;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4386:
                    nCos = 97;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4387:
                    nCos = 98;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4388:
                    nCos = 150;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4389:
                    nCos = 149;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4390:
                    nCos = 101;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4391:
                    nCos = 102;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4392:
                    nCos = 103;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4393:
                    nCos = 104;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4394:
                    nCos = 105;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4395:
                    nCos = 106;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4396:
                    nCos = 107;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4397:
                    nCos = 151;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4398:
                    nCos = 152;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4399:
                    nCos = 153;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4400:
                    nCos = 111;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4401:
                    nCos = 112;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4402:
                    nCos = 154;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4403:
                    nCos = 155;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4404:
                    nCos = 115;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4405:
                    nCos = 116;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4410:
                    nCos = 117;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4411:
                    nCos = 118;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4412:
                    nCos = 119;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4415:
                    nCos = 120;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4416:
                    nCos = 121;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4417:
                    nCos = 122;
                    m_nSkinMeshType = 1;
                    break;
                case 4418:
                    nCos = 123;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4309:
                    nCos = 124;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4330:
                    nCos = 123;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4331:
                    nCos = 125;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4332:
                    nCos = 156;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4333:
                    nCos = 157;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4334:
                    nCos = 126;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4335:
                    nCos = 127;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4336:
                    nCos = 158;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4337:
                    nCos = 159;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4310:
                    nCos = 128;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4311:
                    nCos = 129;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4301:
                    nCos = 160;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4300:
                    nCos = 130;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;
                    break;
                case 4345:
                    nCos = 139;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4346:
                    nCos = 140;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4347:
                    nCos = 141;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4348:
                    nCos = 142;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4305:
                    nCos = 143;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4304:
                    nCos = 131;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4307:
                    nCos = 132;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4314:
                    nCos = 133;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4338:
                    nCos = 134;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4340:
                    nCos = 135;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4342:
                    nCos = 136;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4343:
                    nCos = 137;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4344:
                    nCos = 138;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                case 4302:
                    nCos = 144;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4303:
                    nCos = 145;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4306:
                    nCos = 146;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4308:
                    nCos = 147;
                    m_nSkinMeshType = 1;
                    m_nClass = 4;

                    break;
                case 4349:
                    nCos = 107;
                    m_nSkinMeshType = 0;
                    m_nClass = 4;

                    break;
                    }
            }
            else
            {
                nCos = 16;
                m_nSkinMeshType = 0;
            }
        }
        else if (m_sCostume == 6400)
        {
            nCos = 34;
            m_nSkinMeshType = 1;
            m_nClass = 4;
        }



        m_stLookInfo.FaceMesh = 0;
        m_stLookInfo.FaceSkin = 0;
        m_stLookInfo.HelmMesh = 0;
        m_stLookInfo.CoatMesh = 0;
        m_stLookInfo.PantsMesh = 0;
        m_stLookInfo.GlovesMesh = 0;
        m_stLookInfo.BootsMesh = 0;
        m_stLookInfo.CoatSkin = 0;
        m_stLookInfo.CoatSkin = 0;
        m_stLookInfo.PantsSkin = 0;
        m_stLookInfo.GlovesSkin = 0;
        m_stLookInfo.BootsSkin = 0;

        if (human_costume::UsesFixedBodyRefinement(m_sCostume))
        {
            memset(&m_stSancInfo.Sanc0, 9, 6u);
            memset(&m_stSancInfo.Legend0, 0, 6u);
        }
        else
        {
            memset(&m_stSancInfo.Sanc0, 0, 6u);
            memset(&m_stSancInfo.Legend0, 0, 6u);
        }

    return nCos;
}
