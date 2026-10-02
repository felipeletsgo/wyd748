#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "ItemEffect.h"
#include "NativeItemVolatile.h"
#include "WYD748Assets.h"
#include "ServerListAsset.h"
#include <WinInet.h>

int BASE_DefineSkinMeshType(int nClass)
{
    switch (nClass)
    {
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 0;
    case 8:
        return 1;
    case 16:
        return 20;
    case 17:
        return 21;
    case 18:
        return 22;
    case 19:
        return 23;
    case 20:
        return 24;
    case 21:
        return 2;
    case 22:
        return 25;
    case 23:
        return 26;
    case 24:
        return 27;
    case 25:
        return 2;
    case 26:
        return 3;
    case 27:
        return 28;
    case 28:
        return 29;
    case 29:
        return 6;
    case 30:
        return 4;
    case 31:
        return 32;
    case 32:
        return 7;
    case 33:
        return 8;
    case 34:
        return 0;
    case 35:
        return 29;
    case 36:
        return 0;
    case 37:
        return 1;
    case 38:
        return 1;
    case 39:
        return 0;
    case 40:
        return 0;
    case 41:
        return 69;
    case 42:
        return 30;
    case 43:
        return 31;
    case 44:
        return 33;
    case 45:
        return 23;
    case 46:
        return 11;
    case 47:
        return 35;
    case 48:
        return 34;
    case 49:
        return 36;
    case 50:
        return 37;
    case 51:
        return 38;
    case 52:
        return 39;
    case 53:
        return 40;
    case 54:
        return 9;
    case 55:
        return 10;
    case 56:
        return 41;
    case 57:
        return 12;
    case 58:
        return 42;
    case 59:
        return 43;
    case 60:
        return 0;
    case 61:
        return 1;
    case 62:
        return 5;
    case 63:
        return 0;
    case 64:
        return 44;
    case 66:
        return 45;
    case 67:
        return 46;
    case 68:
        return 47;
    case 69:
        return 48;
    case 70:
        return 53;
    case 71:
        return 54;
    case 72:
        return 55;
    case 73:
        return 56;
    case 74:
        return 57;
    }

    LOG_WRITELOG("Invalide Item Class %d\n", nClass);
    return 0;
}

float BASE_GetMountScale(int nSkinMeshType, int nMeshIndex)
{
    float fSize;

    fSize = 1.0f;
    if (nSkinMeshType == 28)
        fSize = 1.45f;
    else if (nSkinMeshType == 25 && nMeshIndex == 1)
        fSize = 1.4f;
    else if (nSkinMeshType == 20 && nMeshIndex == 7)
        fSize = 0.6f;
    else if (nSkinMeshType == 20 && !nMeshIndex)
        fSize = 1.3f;
    else if (nSkinMeshType == 29 && nMeshIndex == 4)
        fSize = 1.3f;

    return fSize;
}

unsigned int BASE_GetItemTenColor(STRUCT_ITEM* pItem)
{
    if (BASE_GetItemSanc(pItem) <= 9)
        return 0;

    unsigned char sanc{};

    if (pItem->stEffect[0].cEffect == EF_SANC || (pItem->stEffect[0].cEffect >= 115 && pItem->stEffect[0].cEffect <= 126))
        sanc = pItem->stEffect[0].cValue;

    else if (pItem->stEffect[1].cEffect == EF_SANC || (pItem->stEffect[1].cEffect >= 115 && pItem->stEffect[1].cEffect <= 126))
        sanc = pItem->stEffect[1].cValue;

    else if (pItem->stEffect[2].cEffect == EF_SANC || (pItem->stEffect[2].cEffect >= 115 && pItem->stEffect[2].cEffect <= 126))
        sanc = pItem->stEffect[2].cValue;

    return (sanc - 230) % 4 + 5;
}

int BASE_GetItemColorEffect(STRUCT_ITEM* item)
{
    int effect{};

    if (item->stEffect[0].cEffect != EF_SANC && item->stEffect[1].cEffect != EF_SANC && item->stEffect[2].cEffect != EF_SANC)
    {
        if (item->stEffect[0].cEffect >= 115 && item->stEffect[0].cEffect <= 126)
            effect = item->stEffect[0].cEffect;

        else if (item->stEffect[1].cEffect >= 115 && item->stEffect[1].cEffect <= 126)
            effect = item->stEffect[1].cEffect;

        else if (item->stEffect[2].cEffect >= 115 && item->stEffect[2].cEffect <= 126)
            effect = item->stEffect[2].cEffect;
    }
    else if (item->stEffect[0].cEffect == EF_SANC)
        effect = item->stEffect[0].cEffect;

    else if (item->stEffect[1].cEffect == EF_SANC)
        effect = item->stEffect[1].cEffect;

    else
        effect = item->stEffect[2].cEffect;

    return effect;
}

unsigned int BASE_GetItemColor(STRUCT_ITEM* item)
{
    if (!item)
        return 0xFF99EE99;

    unsigned int dwColor = 0xFFAAAAFF;
    int nMaxValue = 0;
    unsigned int dwMaxParm = 0;

    for (int i = 0; i < 49; ++i)
    {
        auto parm = dwEFParam[i];
        if (parm == 45)
            parm = 69;
        if (parm == 46)
            parm = 70;

        int nPos = BASE_GetItemAbility(item, 17);
        int nValue2 = 0;
        int nValue3 = 0;
        unsigned int dwTempColor = 0;
        if (parm == 42 || parm == 53 || nPos == 32 && parm == 2)
        {
            nValue2 = BASE_GetItemAbility(item, parm);
            nValue3 = BASE_GetItemAbilityNosanc(item, parm);
            dwTempColor = BASE_GetOptionColor(nPos, parm, nValue3);
        }
        else
        {
            nValue2 = BASE_GetBonusItemAbility(item, parm);
            nValue3 = BASE_GetBonusItemAbilityNosanc(item, parm);
            dwTempColor = BASE_GetOptionColor(nPos, parm, nValue3);
        }

        if (dwMaxParm && (unsigned __int8)dwMaxParm == parm)
        {
            nMaxValue += nValue3;
            dwColor = BASE_GetOptionColor(nPos, parm, nValue2);
        }
        else if (nValue3)
        {
            if (BASE_GetColorCount(dwColor) < BASE_GetColorCount(dwTempColor))
            {
                nMaxValue = nValue3;
                dwMaxParm = parm;
                dwColor = dwTempColor;
            }
        }
    }

    return dwColor;
}

int BASE_GetColorCount(unsigned int dwColor)
{
    unsigned int nCount = 0;
    if (dwColor == 0xFFAAAAFF)
        nCount = 1;
    if (dwColor == 0xFF99EE99)
        nCount = 2;
    if (dwColor == 0xFFFFFFAA)
        nCount = 3;
    if (dwColor == 0xFFFFAA00)
        nCount = 4;

    return nCount;
}

unsigned int BASE_GetOptionColor(int nPos, unsigned int dwParam, int nValue)
{
    if (nPos >= 64)
    {
        if (nPos != 64 && nPos != 128 && nPos != 192)
            return 0xFF99EE99;
        else if (dwParam == 2 || dwParam == 73 || dwParam == 67)
        {
            if (nValue < 45)
                return 0xFF99EE99;
            else if (nValue < 45 || nValue > 54)
                return 0xFFFFAA00;
            else
                return 0xFFFFFFAA;
        }
        else if (dwParam == 60 || dwParam == 68)
        {
            if (nValue < 20)
                return 0xFF99EE99;
            else if (nValue < 20 || nValue > 24)
                return 0xFFFFAA00;
            else
                return 0xFFFFFFAA;
        }
        else if (dwParam == 26)
        {
            if (nValue < 21)
                return 0xFF99EE99;
            else if (nValue < 21 || nValue > 24)
                return 0xFFFFAA00;
            else
                return 0xFFFFFFAA;
        }
        else if (dwParam == 74)
        {
            if (nValue < 21)
                return 0xFF99EE99;
            else if (nValue < 21 || nValue > 24)
                return 0xFFFFAA00;
            else
                return 0xFFFFFFAA;
        }
        else
            return 0xFF99EE99;
    }
    else if (dwParam == 60 || dwParam == 68)
    {
        if (nPos == 2)
        {
            if (nValue < 12)
                return 0xFF99EE99;
            else if (nValue < 12 || nValue > 14)
                return 0xFFFFAA00;
            else
                return 0xFFFFFFAA;
        }
        else if (nValue >= 6)
        {
            if (nValue == 6)
                return 0xFFFFFFAA;
            else
                return 0xFFFFAA00;
        }
        else
            return 0xFF99EE99;
    }
    else if (dwParam == 42 || dwParam == 71)
    {
        if (nValue < 50)
            return 0xFF99EE99;
        else if (nValue < 50 || nValue >= 60)
            return 0xFFFFAA00;
        else
            return 0xFFFFFFAA;
    }
    else if (dwParam == 26)
    {
        if (nValue < 12)
            return 0xFF99EE99;
        else if (nValue == 12)
            return 0xFFFFFFAA;
        else
            return 0xFFFFAA00;
    }
    else if (dwParam == 74)
    {
        if (nValue < 12)
            return 0xFF99EE99;
        else if (nValue == 12)
            return 0xFFFFFFAA;
        else
            return 0xFFFFAA00;
    }
    else if (dwParam == 3 || dwParam == 53 || dwParam == 72)
    {
        if (nPos == 16)
        {
            if (nValue < 30)
                return 0xFF99EE99;
            else if (nValue == 30)
                return 0xFFFFFFAA;
            else
                return 0xFFFFAA00;
        }
        else if (nValue < 15)
            return 0xFF99EE99;
        else if (nValue == 15)
            return 0xFFFFFFAA;
        else
            return 0xFFFFAA00;
    }
    else if (dwParam == 2 || dwParam == 73 || dwParam == 67)
    {
        if (nPos == 32)
        {
            if (nValue < 24)
                return 0xFF99EE99;
            else if (nValue < 24 || nValue > 30)
                return 0xFFFFAA00;
            else
                return 0xFFFFFFAA;
        }
        else if (nValue < 18)
            return 0xFF99EE99;
        else if (nValue == 18)
            return 0xFFFFFFAA;
        else
            return 0xFFFFAA00;
    }
    else if (dwParam == 4 || dwParam == 45 || dwParam == 69)
    {
        if (nValue < 40)
            return 0xFF99EE99;
        else if (nValue == 40)
            return 0xFFFFFFAA;
        else
            return 0xFFFFAA00;
    }

    return 0xFF99EE99;
}

int BASE_GetMeshIndex(short sIndex)
{
    STRUCT_ITEM item{};
    item.sIndex = sIndex;
    int nPos = BASE_GetItemAbility(&item, 17);
    int nClassType = BASE_GetItemAbility(&item, 18);
    int nClassIndex = 0;
    if (g_pItemList[sIndex].nIndexMesh >= 40 && g_pItemList[sIndex].nIndexMesh < 50
        && (nPos & 4 || nPos & 8 || nPos & 0x10 || nPos & 0x20))
    {
        if (g_pItemList[sIndex].nIndexMesh == 40)
            return 0;

        switch (nClassType)
        {
        case 1:
            nClassIndex = 0;
            break;
        case 4:
            nClassIndex = 1;
            break;
        case 2:
            nClassIndex = 2;
            break;
        case 8:
            nClassIndex = 3;
            break;
        }

        if (nPos & 4)
            nPos = 0;
        else if (nPos & 8)
            nPos = 1;
        else if (nPos & 0x10)
            nPos = 2;
        else if (nPos & 0x20)
            nPos = 3;

        return nPos + 4 * (g_pItemList[sIndex].nIndexMesh + nClassIndex - 41) + 1401;
    }

    switch (nClassType)
    {
    case 1:
        nClassIndex = 0;
        break;
    case 2:
        nClassIndex = 200;
        break;
    case 4:
        nClassIndex = 20;
        break;
    case 8:
        nClassIndex = 220;
        break;
    }

    if (nClassType <= 8)
    {
        if (nPos & 2)
            return g_pItemList[sIndex].nIndexMesh + nClassIndex + 1001;
        if (nPos & 4)
            return g_pItemList[sIndex].nIndexMesh + nClassIndex + 1041;
        if (nPos & 8)
            return g_pItemList[sIndex].nIndexMesh + nClassIndex + 1081;
        if (nPos & 0x10)
            return g_pItemList[sIndex].nIndexMesh + nClassIndex + 1121;
        if (nPos & 0x20)
            return g_pItemList[sIndex].nIndexMesh + nClassIndex + 1161;
    }

    return g_pItemList[sIndex].nIndexMesh;
}
