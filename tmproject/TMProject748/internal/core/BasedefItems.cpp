#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "ItemEffect.h"
#include "NativeItemVolatile.h"
#include "ServerListAsset.h"

// Item, equipment, and effect rules.
int BASE_GetItemSanc(STRUCT_ITEM* item)
{
    if (item->sIndex >= 2330 && item->sIndex < 2390)
        return 0;

    if (item->sIndex >= 3200 && item->sIndex < 3300)
        return 0;

    if (item->sIndex >= 3980 && item->sIndex < 4000)
        return 0;

    int sanc{};

    if (item->stEffect[0].cEffect != EF_SANC && item->stEffect[1].cEffect != EF_SANC && item->stEffect[2].cEffect != EF_SANC)
    {
        if (item->stEffect[0].cEffect >= 115 && item->stEffect[0].cEffect <= 126)
            sanc = item->stEffect[0].cValue;

        else if (item->stEffect[1].cEffect >= 115 && item->stEffect[1].cEffect <= 126)
            sanc = item->stEffect[1].cValue;

        else if (item->stEffect[2].cEffect >= 115 && item->stEffect[2].cEffect <= 126)
            sanc = item->stEffect[2].cValue;
    }
    else if (item->stEffect[0].cEffect == EF_SANC)
        sanc = item->stEffect[0].cValue;

    else if (item->stEffect[1].cEffect == EF_SANC)
        sanc = item->stEffect[1].cValue;

    else
        sanc = item->stEffect[2].cValue;

    if (item->sIndex != 786 && item->sIndex != 1936 && item->sIndex != 1937)
    {
        if (sanc < 230)
            sanc %= 10;
        else
            sanc -= 220;

        if (sanc >= 10 && sanc <= 35)
            sanc = (sanc - 10) / 4 + 10;
    }

    return sanc;
}

int BASE_GetItemAbility(STRUCT_ITEM* item, char Type)
{
    int value = 0;
    int idx = item->sIndex;

    if (idx <= 0 || idx >= MAX_ITEMLIST)
        return 0;

    if (Type == EF_VOLATILE)
        return native_item_volatile::GetAbility(idx, g_pItemList[idx].stEffect, item->stEffect);

    int nUnique = g_pItemList[idx].nUnique;
    int nPos = g_pItemList[idx].nPos;

    if ((Type == EF_DAMAGEADD || Type == EF_MAGICADD) && (nUnique < 41 || nUnique > 50))
        return 0;

    if (Type == EF_CRITICAL && (item->stEffect[1].cEffect == EF_CRITICAL2 || item->stEffect[2].cEffect == EF_CRITICAL2))
        Type = EF_CRITICAL2;

    if (Type == EF_DAMAGE && nPos == 32 && (item->stEffect[1].cEffect == EF_DAMAGE2 || item->stEffect[2].cEffect == EF_DAMAGE2))
        Type = EF_DAMAGE2;

    if (Type == EF_MPADD && (item->stEffect[1].cEffect == EF_MPADD2 || item->stEffect[2].cEffect == EF_MPADD2))
        Type = EF_MPADD2;

    if (Type == EF_HPADD && (item->stEffect[1].cEffect == EF_HPADD2 || item->stEffect[2].cEffect == EF_HPADD2))
        Type = EF_HPADD2;

    if (Type == EF_ACADD && (item->stEffect[1].cEffect == EF_ACADD2 || item->stEffect[2].cEffect == EF_ACADD2))
        Type = EF_ACADD2;

    if (Type == EF_LEVEL)
        value = g_pItemList[idx].nReqLvl;

    if (Type == EF_REQ_STR)
        value += g_pItemList[idx].nReqStr;

    if (Type == EF_REQ_INT)
        value += g_pItemList[idx].nReqInt;

    if (Type == EF_REQ_DEX)
        value += g_pItemList[idx].nReqDex;

    if (Type == EF_REQ_CON)
        value += g_pItemList[idx].nReqCon;

    if (Type == EF_POS)
        value += g_pItemList[idx].nPos;

    if (Type != EF_INCUBATE)
    {
        for (int i = 0; i < 12; ++i)
        {
            if (g_pItemList[idx].stEffect[i].sEffect == Type ||
                g_pItemList[idx].stEffect[i].sEffect == EF_HPADD && Type == EF_HPADD2)
            {
                int tvalue = g_pItemList[idx].stEffect[i].sValue;

                if (Type == EF_ATTSPEED && tvalue == 1)
                    tvalue = 10;

                value += tvalue;
            }
        }
    }

    if (item->sIndex >= 2330 && item->sIndex < 2390)
    {
        switch (Type)
        {
        case EF_MOUNTHP:
            return item->stEffect[0].sValue;
        case EF_MOUNTSANC:
            return item->stEffect[1].cEffect;
        case EF_MOUNTLIFE:
            return item->stEffect[1].cValue;
        case EF_MOUNTFEED:
            return item->stEffect[2].cEffect;
        case EF_MOUNTKILL:
            return item->stEffect[2].cValue;
        }

        if (item->sIndex < 2362 || item->sIndex >= 2390 || item->stEffect[0].sValue <= 0)
            return value;

        int lv = item->stEffect[1].cEffect;
        int cd = item->sIndex - 2360;

        switch (Type)
        {
        case EF_DAMAGE:
            value = g_pMountBonus[cd][0] * (lv + 20) / 100;
            break;
        case EF_MAGIC:
            value = g_pMountBonus[cd][1] * (lv + 15) / 100;
            break;
        case EF_PARRY:
            value = g_pMountBonus[cd][2];
            break;
        case EF_RESISTALL:
            value = g_pMountBonus[cd][3];
            break;
        default:
            break;
        }
    }
    else if (item->sIndex >= 3980 && item->sIndex < 4000)
    {
        int cd = item->sIndex - 3980;

        switch (Type)
        {
        case EF_DAMAGE:
            value = g_pMountBonus2[cd][0];
            break;
        case EF_MAGIC:
            value = g_pMountBonus2[cd][1];
            break;
        case EF_PARRY:
            value = g_pMountBonus2[cd][2];
            break;
        case EF_RESISTALL:
            value = g_pMountBonus2[cd][3];
            break;
        default:
            break;
        }
    }
    else
    {
        for (int j = 0; j < 3; ++j)
        {
            if (item->stEffect[j].cEffect == Type)
            {
                int tvalue = item->stEffect[j].cValue;

                if (Type == EF_ATTSPEED && tvalue == 1)
                    tvalue = 10;

                value += tvalue;
            }
        }

        int sanc = BASE_GetItemSanc(item);

        if (item->sIndex <= 40)
            sanc = 0;

        if (sanc >= 9 && nPos & 0xF00)
            sanc++;

        if (sanc
            && Type != EF_GRID
            && Type != EF_CLASS
            && Type != EF_POS
            && Type != EF_WTYPE
            && Type != EF_RANGE
            && Type != EF_LEVEL
            && Type != EF_REQ_STR
            && Type != EF_REQ_INT
            && Type != EF_REQ_DEX
            && Type != EF_REQ_CON
            && Type != EF_VOLATILE
            && Type != EF_INCUBATE
            && Type != EF_INCUDELAY
            && Type != EF_PREVBONUS
            && Type != EF_TRANS
            && Type != EF_REFLEVEL
            && Type != EF_GAMEROOM
            && Type != EF_REGENMP
            && Type != EF_REGENHP
            && Type != EF_FAME)
        {
            if (sanc > 10)
            {
                int UpSanc = sanc - 10;

                switch (UpSanc)
                {
                case 1:
                    UpSanc = 220;
                    break;
                case 2:
                    UpSanc = 250;
                    break;
                case 3:
                    UpSanc = 280;
                    break;
                case 4:
                    UpSanc = 320;
                    break;
                case 5:
                    UpSanc = 370;
                    break;
                case 6:
                    UpSanc = 400;
                    break;
                }

                value = UpSanc * 10 * value / 100 / 10;
            }
            else
            {
                value = value * (sanc + 10) / 10;
            }
        }

        if (Type == EF_RUNSPEED)
        {
            if (value >= 3)
                value = 2;

            if (value > 0 && sanc >= 9)
                value++;
        }

        if (Type == EF_HWORDGUILD || Type == EF_LWORDGUILD)
            value = value;

        if (Type == EF_REGENMP || Type == EF_REGENHP)
            value *= sanc;

        if (Type == EF_GRID && (value < 0 || value > 7))
            value = 0;
    }

	return value;
}

int BASE_GetSubGuild(int item)
{
    int ret = 0;
    if (item >= 3 && item <= 8)
        ret = item % 3 + 1;

    return ret;
}

int BASE_GetStaticItemAbility(STRUCT_ITEM* item, char Type)
{
    int value = 0;
    int idx = item->sIndex;

    if (idx <= 0 || idx >= MAX_ITEMLIST)
        return value;

    if (idx >= 3200 && idx <= 3300)
        return 0;

    int nPos = g_pItemList[idx].nPos;

    if (Type == EF_LEVEL)
        value += g_pItemList[idx].nReqLvl;

    if (Type == EF_REQ_STR)
        value += g_pItemList[idx].nReqStr;

    if (Type == EF_REQ_INT)
        value += g_pItemList[idx].nReqInt;

    if (Type == EF_REQ_DEX)
        value += g_pItemList[idx].nReqDex;

    if (Type == EF_REQ_CON)
        value += g_pItemList[idx].nReqCon;

    if (Type == EF_POS)
        value += g_pItemList[idx].nPos;

    if (Type != EF_INCUBATE)
    {
        for (int i = 0; i < 12; i++)
        {
            if (g_pItemList[idx].stEffect[i].sEffect != Type)
                continue;

            int tvalue = g_pItemList[idx].stEffect[i].sValue;

            if (Type == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
        }
    }

    if (idx >= 2330 && idx < 2390)
    {
        if (Type == EF_MOUNTHP)
            return item->stEffect[0].sValue;

        else if (Type == EF_MOUNTSANC)
            return item->stEffect[1].cEffect;

        else if (Type == EF_MOUNTLIFE)
            return item->stEffect[1].cValue;

        else if (Type == EF_MOUNTFEED)
            return item->stEffect[2].cEffect;

        else if (Type == EF_MOUNTKILL)
            return item->stEffect[2].cValue;

        if (idx < 2362 || idx >= 2390 || item->stEffect[0].sValue <= 0)
            return value;

        int lv = item->stEffect[1].cEffect;
        int cd = item->sIndex - 2360;

        if (Type == EF_DAMAGE)
            return g_pMountBonus[cd][0] * (lv + 20) / 100;

        else if (Type == EF_MAGIC)
            return g_pMountBonus[cd][1] * (lv + 15) / 100;

        else if (Type == EF_PARRY)
            return g_pMountBonus[cd][2];

        else if (Type == EF_RESISTALL)
            return g_pMountBonus[cd][3];
        else
            return value;
    }

    if (idx >= 3980 && idx <= 3994)
    {

        if (Type == EF_DAMAGE)
            return g_pMountBonus2[idx - 3980][0];

        else if (Type == EF_MAGIC)
            return g_pMountBonus2[idx - 3980][1];

        else if (Type == EF_PARRY)
            return g_pMountBonus2[idx - 3980][2];

        else if (Type == EF_RESISTALL)
            return g_pMountBonus2[idx - 3980][3];
        else
            return value;
    }

    int sanc = BASE_GetItemSanc(item);

    if (sanc >= 9 && nPos & 0xF00)
        sanc++;

    if (sanc
        && Type != EF_GRID
        && Type != EF_CLASS
        && Type != EF_POS
        && Type != EF_WTYPE
        && Type != EF_RANGE
        && Type != EF_LEVEL
        && Type != EF_REQ_STR
        && Type != EF_REQ_INT
        && Type != EF_REQ_DEX
        && Type != EF_REQ_CON
        && Type != EF_VOLATILE
        && Type != EF_INCUBATE
        && Type != EF_INCUDELAY
        && Type != EF_PREVBONUS
        && Type != EF_REGENMP
        && Type != EF_REGENHP)
    {
        if (sanc > 10)
        {
            int UpSanc = sanc - 10;
            switch (UpSanc)
            {
            case 1:
                UpSanc = 220;
                break;
            case 2:
                UpSanc = 250;
                break;
            case 3:
                UpSanc = 280;
                break;
            case 4:
                UpSanc = 320;
                break;
            case 5:
                UpSanc = 370;
                break;
            case 6:
                UpSanc = 400;
                break;
            }
            value = UpSanc * 10 * value / 100 / 10;
        }
        else
        {
            value = value * (sanc + 10) / 10;
        }
    }

    if (Type == EF_RUNSPEED)
    {
        if (value >= 3)
            value = 2;

        if (value > 0 && sanc >= 9)
            value++;
    }

    if ((Type == EF_REGENMP || Type == EF_REGENHP) && sanc > 0)
        value *= sanc;

    return value;
}

int BASE_GetItemAmount(STRUCT_ITEM* item)
{
    return BASE_GetEffectValue(item, EF_AMOUNT);
}

int BASE_CanCarry(STRUCT_ITEM* Carry, int pos)
{
    if (pos < 30)
        return 1;
    if (pos / 15 == 2 && Carry[60].sIndex != 3467)
        return 0;
    if (pos / 15 != 3 || Carry[61].sIndex == 3467)
        return 1;

    return 0;
}

int BASE_CanTrade(STRUCT_ITEM* Dest, STRUCT_ITEM* Carry, char* MyTrade, STRUCT_ITEM* OpponentTrade)
{
    STRUCT_ITEM OpponentTemp[MAX_TRADE]{};

    memcpy(Dest, Carry, sizeof(STRUCT_ITEM) * MAX_CARRY);

    for (int i = 0; i < MAX_TRADE; ++i)
    {
        int pos = (unsigned char)MyTrade[i];
        if (pos != -1)
            BASE_ClearItem(&Dest[pos]);
    }

    BASE_SortTradeItem(OpponentTemp, EF_GRID);

    for (int i = 0; i < MAX_TRADE; i++)
    {
        if (!OpponentTemp[i].sIndex)
            continue;

        int j = 0;
        for (j = 0; j < MAX_VISIBLE_CARRY; j++)
        {
            if (!Dest[j].sIndex && BASE_CanCarry(Dest, j))
            {
                Dest[j] = OpponentTemp[i];
                break;
            }
        }
        if (j == MAX_VISIBLE_CARRY)
            return 0;
    }

    return 1;
}

void BASE_ClearItem(STRUCT_ITEM* item)
{
    memset(item, 0, sizeof(item));
}

void BASE_SortTradeItem(STRUCT_ITEM* Item, int Type)
{
    int Buffer[15]{};
    for (int i = 0; i < MAX_TRADE; ++i)
    {
        if (Item[i].sIndex)
            Buffer[i] = BASE_GetItemAbility(&Item[i], Type);
        else
            Buffer[i] = -1;
    }

    STRUCT_ITEM ItemTemp[MAX_TRADE]{};

    for (int i = 0; i < MAX_TRADE; ++i)
    {
        int MaxBufferIndex = 0;
        int MaxBuffer = -1;
        for (int j = 0; j < MAX_TRADE; ++j)
        {
            if (Buffer[j] > MaxBuffer)
            {
                MaxBufferIndex = j;
                MaxBuffer = Buffer[j];
            }
        }

        if (MaxBuffer == -1)
            break;

        Buffer[MaxBufferIndex] = -1;

        ItemTemp[i] = Item[MaxBufferIndex];
    }

    memcpy(Item, ItemTemp, sizeof(ItemTemp));
}

int BASE_CanCargo(STRUCT_ITEM* item, STRUCT_ITEM* cargo, int DestX, int DestY)
{
    if (!item || !item->sIndex || !cargo)
        return 0;

    // The source-built 7.48 client and the authoritative Go server store every item
    // in exactly one cargo entry.  Checking the destination entry directly
    // avoids the old malformed 2x4 mask indexing and keeps drag feedback equal
    // to the packet slot that the server validates.
    if (DestX < 0 || DestX >= 9 || DestY < 0 || DestY >= 14)
        return 0;

    const int destination = DestX + 9 * DestY;
    if (destination < 0 || destination >= MAX_CARGO)
        return 0;

    return cargo[destination].sIndex == 0;
}

int BASE_CanEquip(STRUCT_ITEM* item, STRUCT_SCORE* score, int Pos, int Class, STRUCT_ITEM* pBaseEquip, int OriginalFace, bool hasSoulLimitSkill)
{
    int idx = item->sIndex;
    if (idx <= 0 || idx >= 6500)
        return FALSE;

    int nUnique = g_pItemList[idx].nUnique;
    if (Pos == 15)
        return FALSE;

    if (Pos != -1)
    {
        int tpos = BASE_GetItemAbility(item, EF_POS);
        int pos = (tpos >> Pos) & 1;

        if (pos == 0)
            return FALSE;

        if (Pos == 6 || Pos == 7)
        {
            int OtherPos = (Pos == 6) ? 7 : 6;
            int	OtherIdx = pBaseEquip[OtherPos].sIndex;

            if (OtherIdx > 0 && OtherIdx < MAX_ITEMLIST)
            {
                int nUnique2 = g_pItemList[OtherIdx].nUnique;
                int otherpos = BASE_GetItemAbility(&pBaseEquip[OtherPos], EF_POS);

                if (tpos == 64 || otherpos == 64)
                {
                    if (nUnique == 46)
                    {
                        if (otherpos != 128)
                            return FALSE;
                    }
                    else if (nUnique2 == 46)
                    {
                        if (tpos != 128)
                            return FALSE;
                    }
                    else
                        return FALSE;
                }
            }
        }
    }

    if (Class >= 22 && Class <= 25 || Class == 32)
        Class = OriginalFace;

    int trans = Class % 10;

    if (Pos == 1 && trans > 5
        && item->sIndex != 747
        && item->sIndex != 3500
        && item->sIndex != 3507
        && item->sIndex != 3501
        && item->sIndex != 3502
        && item->sIndex != 3303
        && item->sIndex != 3507)
    {
        return FALSE;
    }

    int transitem = BASE_GetItemAbility(item, EF_TRANS);
    switch (transitem)
    {
    case 1:
        if (trans < 6)
            return FALSE;
        break;
    case 2:
        if (trans >= 6)
            return FALSE;
        break;
    case 3:
        if (trans < 6)
            return FALSE;
        if (!hasSoulLimitSkill)
            return FALSE;
        break;
    }

    if (!((BASE_GetItemAbility(item, EF_CLASS) >> Class / 10) & 1))
    {
        if (trans <= 5)
            return FALSE;

        int pos = BASE_GetItemAbility(item, EF_POS);
        if (pos != 64 && pos != 128 && pos != 192)
            return FALSE;
    }
    if (nUnique % 10 >= 9 && nUnique < 40 && trans < 6)
        return FALSE;

    int lvl = BASE_GetItemAbility(item, EF_LEVEL);
    int str = BASE_GetItemAbility(item, EF_REQ_STR);
    int spt = BASE_GetItemAbility(item, EF_REQ_INT);
    int agi = BASE_GetItemAbility(item, EF_REQ_DEX);
    int con = BASE_GetItemAbility(item, EF_REQ_CON);

    int wtype = BASE_GetItemAbility(item, EF_WTYPE);

    int weapontype = wtype % 10;
    int modweapon = wtype % 10;

    int divweapon = wtype % 10 / 10;
    if (Pos == 7 && weapontype)
    {
        int rate = 100;
        if (divweapon || modweapon <= 1)
        {
            if (divweapon == 6 && modweapon > 1)
                rate = 150;
        }
        else
        {
            rate = 130;
        }
        lvl = rate * lvl / 100;
        str = rate * str / 100;
        spt = rate * spt / 100;
        agi = rate * agi / 100;
        con = rate * con / 100;
    }

    if (trans < 5)
    {
        if (lvl > score->Level)
            return FALSE;
        if (str > score->Str)
            return FALSE;
        if (spt > score->Int)
            return FALSE;
        if (agi > score->Dex)
            return FALSE;
        if (con > score->Con)
            return FALSE;
    }

    return TRUE;
}

int BASE_CanEquip_RecvRes(STRUCT_REQ* req, STRUCT_ITEM* item, STRUCT_SCORE* score, int Pos, int Class, STRUCT_ITEM* pBaseEquip, int OriginalFace)
{
    if (!req)
        return 0;
    req->Class = 0;
    req->Level = 0;
    req->Str = 0;
    req->Int = 0;
    req->Dex = 0;
    req->Con = 0;
    int idx = item->sIndex;

    if (idx <= 0 || idx >= 6500)
        return 0;
    int nUnique = g_pItemList[idx].nUnique;
    if (Pos == 15)
        return 0;
    int grade = g_pItemList[idx].nGrade;

    if (Pos != -1)
    {
        int nPos = BASE_GetItemAbility(item, 17);
        if (!((nPos >> Pos) & 1))
            return 0;

        if (Pos == 6 || Pos == 7)
        {
            int OtherPos = Pos == 6 ? 7 : 6;
            int OtherIdx = pBaseEquip[OtherPos].sIndex;

            if (OtherIdx > 0 && OtherIdx < 6500)
            {
                int nUnique2 = g_pItemList[OtherIdx].nUnique;
                int otherpos = BASE_GetItemAbility(&pBaseEquip[OtherPos], 17);
                if (nPos == 64 || otherpos == 64)
                {
                    if (nUnique == 46)
                    {
                        if (otherpos != 128)
                            return 0;
                    }
                    else
                    {
                        if (nUnique2 != 46)
                            return 0;
                        if (nPos != 128)
                            return 0;
                    }
                }
            }
        }
    }

    if (Class >= 22 && Class <= 25 || Class == 32)
        Class = OriginalFace;
    int mount = 0;
    if (idx >= 2300 && idx < 2390)
        mount = (idx - 2300) % 30;
    int  trans = Class % 10;
    if (Pos == 1 && item->sIndex != 747 && trans > 5)
        return 0;
    if ((mount == 19 || mount == 20) && trans <= 5)
        return 0;
    req->Class = 1;
    int transitem = BASE_GetItemAbility(item, 112);

    if (transitem == 1)
    {
        if (trans < 6)
            req->Class = 0;
    }
    else if (transitem == 2 && trans >= 6)
    {
        req->Class = 0;
    }

    int cls = (BASE_GetItemAbility(item, 18) >> Class / 10) & 1;
    int tpos = BASE_GetItemAbility(item, 17);
    if (!cls)
    {
        if (trans > 5)
        {
            if (tpos != 64 && tpos != 128 && tpos != 192)
                req->Class = 0;
        }
        else
        {
            req->Class = 0;
        }
    }
    if (Class <= 31 && Class % 10 != 1 && tpos == 2)
        req->Class = 0;
    if (nUnique % 10 >= 9 && nUnique < 40 && trans < 6)
        req->Class = 0;

    int lvl = BASE_GetItemAbility(item, 1);
    int str = BASE_GetItemAbility(item, 22);
    int spt = BASE_GetItemAbility(item, 23);
    int agi = BASE_GetItemAbility(item, 24);
    int con = BASE_GetItemAbility(item, 25);
    int wtype = BASE_GetItemAbility(item, 21);
    int modweapon = wtype % 10;
    int divweapon = wtype % 10 / 10;
    if (Pos == 7 && modweapon)
    {
        int rate = 100;
        if (divweapon || modweapon <= 1)
        {
            if (divweapon == 6 && modweapon > 1)
                rate = 150;
        }
        else
        {
            rate = 130;
        }
        lvl = rate * lvl / 100;
        str = rate * str / 100;
        spt = rate * spt / 100;
        agi = rate * agi / 100;
        con = rate * con / 100;
    }

    if (score->Level >= lvl)
        req->Level = 1;
    if (score->Str >= str)
        req->Str = 1;
    if (score->Int >= spt)
        req->Int = 1;
    if (score->Dex >= agi)
        req->Dex = 1;
    if (score->Con >= con)
        req->Con = 1;

    return 1;
}

int BASE_GetBonusItemAbilityNosanc(STRUCT_ITEM* item, char Type)
{
    if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
        return 0;

    if (item->sIndex >= 3200 && item->sIndex <= 3300)
        return 0;

    if (item->sIndex >= 2330 && item->sIndex < 2390)
        return 0;

    if (item->sIndex >= 3980 && item->sIndex < 4000)
        return 0;

    int value = 0;

    for (int i = 0; i < 3; i++)
    {
        if (item->stEffect[i].cEffect != Type)
            continue;

        int tvalue = item->stEffect[i].cValue;

        if (Type == EF_ATTSPEED && tvalue == 1)
            tvalue = 10;

        value += tvalue;
    }

    return value;
}

int BASE_GetBonusItemAbility(STRUCT_ITEM* item, char Type)
{
    if (item->sIndex <= 0 || item->sIndex >= MAX_ITEMLIST)
        return 0;

    if (item->sIndex >= 3200 && item->sIndex <= 3300)
        return 0;

    if (item->sIndex >= 2330 && item->sIndex < 2390)
        return 0;

    if (item->sIndex >= 3980 && item->sIndex < 4000)
        return 0;

    int value = 0;

    int nPos = g_pItemList[item->sIndex].nPos;

    for (int i = 0; i < 3; ++i)
    {
        if (item->stEffect[i].cEffect == Type)
        {
            int tvalue = item->stEffect[i].cValue;

            if (Type == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
        }
    }

    int sanc = BASE_GetItemSanc(item);

    if (sanc >= 9 && nPos & 0xF00)
        sanc++;

    if (sanc
        && Type != EF_GRID
        && Type != EF_CLASS
        && Type != EF_POS
        && Type != EF_WTYPE
        && Type != EF_RANGE
        && Type != EF_LEVEL
        && Type != EF_REQ_STR
        && Type != EF_REQ_INT
        && Type != EF_REQ_DEX
        && Type != EF_REQ_CON
        && Type != EF_VOLATILE
        && Type != EF_INCUBATE
        && Type != EF_INCUDELAY
        && Type != EF_PREVBONUS)
    {
        if (sanc > 10)
        {
            int UpSanc = sanc - 10;
            switch (UpSanc)
            {
            case 1:
                UpSanc = 220;
                break;
            case 2:
                UpSanc = 250;
                break;
            case 3:
                UpSanc = 280;
                break;
            case 4:
                UpSanc = 320;
                break;
            case 5:
                UpSanc = 370;
                break;
            case 6:
                UpSanc = 400;
                break;
            }
            value = UpSanc * 10 * value / 100 / 10;
        }
        else
        {
            value = value * (sanc + 10) / 10;
        }
    }
    return value;
}

int BASE_GetItemAbilityNosanc(STRUCT_ITEM* item, char type)
{
    int itemId = item->sIndex;
    int value = 0;

    if (itemId < 0 || itemId >= MAX_ITEMLIST)
        return value;

    int nUnique = g_pItemList[itemId].nUnique;
    int nPos = g_pItemList[itemId].nPos;

    if (type == EF_DAMAGEADD || type == EF_MAGICADD)
    {
        if (nUnique < 41 || nUnique > 50)
            return value;
    }

    if (type == EF_CRITICAL)
    {
        if (item->stEffect[1].cEffect == EF_CRITICAL2 || item->stEffect[2].cEffect == EF_CRITICAL2)
            type = EF_CRITICAL2;
    }

    if (type == EF_DAMAGE && nPos == 32)
    {
        if (item->stEffect[1].cEffect == EF_DAMAGE2 || item->stEffect[2].cEffect == EF_DAMAGE2)
            type = EF_DAMAGE2;
    }

    if (type == EF_MPADD)
    {
        if (item->stEffect[1].cEffect == EF_MPADD2 || item->stEffect[2].cEffect == EF_MPADD2)
            type = EF_MPADD2;
    }

    if (type == EF_HPADD)
    {
        if (item->stEffect[1].cEffect == EF_HPADD2 || item->stEffect[2].cEffect == EF_HPADD2)
            type = EF_HPADD2;
    }

    if (type == EF_ACADD)
    {
        if (item->stEffect[1].cEffect == EF_ACADD2 || item->stEffect[2].cEffect == EF_ACADD2)
            type = EF_ACADD2;
    }

    if (type == EF_LEVEL)
        value += g_pItemList[itemId].nReqLvl;

    if (type == EF_REQ_STR)
        value += g_pItemList[itemId].nReqStr;

    if (type == EF_REQ_INT)
        value += g_pItemList[itemId].nReqInt;

    if (type == EF_REQ_DEX)
        value += g_pItemList[itemId].nReqDex;

    if (type == EF_REQ_CON)
        value += g_pItemList[itemId].nReqCon;

    if (type == EF_POS)
        value += g_pItemList[itemId].nPos;

    if (type != EF_INCUBATE)
    {
        for (int i = 0; i < 12; i++)
        {
            if (g_pItemList[itemId].stEffect[i].sEffect != type)
                continue;

            if (g_pItemList[itemId].stEffect[i].sEffect == EF_HPADD || g_pItemList[itemId].stEffect[i].sEffect == EF_HPADD2)
                continue;

            int tvalue = g_pItemList[itemId].stEffect[i].sValue;

            if (itemId == EF_ATTSPEED && tvalue == 1)
                tvalue = 10;

            value += tvalue;
        }
    }

    if (itemId >= 2330 && itemId < 2390)
    {
        if (type == EF_MOUNTHP)
            return item->stEffect[0].sValue;

        else if (type == EF_MOUNTSANC)
            return item->stEffect[1].cEffect;

        else if (type == EF_MOUNTLIFE)
            return item->stEffect[1].cValue;

        else if (type == EF_MOUNTFEED)
            return item->stEffect[2].cEffect;

        else if (type == EF_MOUNTKILL)
            return item->stEffect[2].cValue;

        if (itemId < 2362 || itemId >= 2390 || item->stEffect[0].sValue <= 0)
            return value;

        int lv = item->stEffect[1].cEffect;
        int cd = item->sIndex - 2360;

        if (type == EF_DAMAGE)
            return g_pMountBonus[cd][0] * (lv + 20) / 100;

        else if (type == EF_MAGIC)
            return g_pMountBonus[cd][1] * (lv + 15) / 100;

        else if (type == EF_PARRY)
            return g_pMountBonus[cd][2];

        else if (type == EF_RESISTALL)
            return g_pMountBonus[cd][3];
        else
            return value;
    }

    if (itemId >= 3980 && itemId <= 3994)
    {

        if (type == EF_DAMAGE)
            return g_pMountBonus2[itemId - 3980][0];

        else if (type == EF_MAGIC)
            return g_pMountBonus2[itemId - 3980][1];

        else if (type == EF_PARRY)
            return g_pMountBonus2[itemId - 3980][2];

        else if (type == EF_RESISTALL)
            return g_pMountBonus2[itemId - 3980][3];
        else
            return value;
    }

    for (int i = 0; i < 3; i++)
    {
        if (item->stEffect[i].cEffect == type)
        {
            int total = item->stEffect[i].cValue;
            if (type == EF_ATTSPEED && total == 1)
                total = 10;

            value += total;
        }
    }

    return value;
}

void BASE_SetItemAmount(STRUCT_ITEM* item, int amount)
{
    if (item->sIndex == 419 || item->sIndex == 420 || item->sIndex == 412 || item->sIndex == 413 ||
        item->sIndex >= 2390 && item->sIndex <= 2418)
    {
        BASE_RemoveEffect(item, EF_UNIQUE);
    }

    BASE_ChangeOrAddEffectValue(item, EF_AMOUNT, amount);
}

/**
 * Checks whether an effect slot is empty or contains replaceable refinement/color.
 * item is borrowed, required, and read-only. Returns false when all slots
 * contain other effects; does not validate category, cost, or server rules.
 */
bool BASE_CanRefine(STRUCT_ITEM* item)
{
	for (auto i : item->stEffect)
	{
		if (i.cEffect == 0 || i.cEffect == EF_SANC || (i.cEffect >= EF_COLOR0 && i.cEffect <= EF_COLOR9))
			return true;
	}

	return false;
}

/**
 * Identifies EF_SANC or a color in [EF_STARTCOL, EF_MAXCOL).
 * effect is a borrowed read-only reference; cValue is not validated.
 */
bool BASE_HasSancAdd(const STRUCT_BONUSEFFECT& effect)
{
    return effect.cEffect == EF_SANC || (effect.cEffect >= EF_STARTCOL && effect.cEffect < EF_MAXCOL);
}

/**
 * Returns cValue from the first effect recognized by BASE_HasSancAdd.
 * item is read-only. Zero also means no such effect; later effects are
 * neither summed nor used as fallbacks.
 */
int BASE_GetSancEffValue(const STRUCT_ITEM& item)
{
    for (auto& effect : item.stEffect)
    {
        if (BASE_HasSancAdd(effect))
            return effect.cValue;
    }

    return 0;
}

/**
 * Decodes refinement as integer division by 10 up to a maximum of 210.
 * item is borrowed and required. Returns 0 for larger values, missing effects,
 * or IDs 2330..2389, 3200..3299, and 3980..3999 without changing the item.
 * Despite its name, this function neither rolls success nor returns a
 * validated probability.
 */
int BASE_GetItemSancSuccess(STRUCT_ITEM* item)
{
    if (item->sIndex >= 2330 && item->sIndex < 2390)
        return 0;
    if (item->sIndex >= 3200 && item->sIndex < 3300)
        return 0;
    if (item->sIndex >= 3980 && item->sIndex < 4000)
        return 0;

    auto sanc = BASE_GetSancEffValue(*item);

    if (sanc <= 210)
        return sanc / 10;

    return 0;
}

/**
 * Finds effect and returns cValue from its first occurrence, or 0 if absent.
 * item must be non-null and remains unchanged. Zero does not distinguish an
 * absent effect from a present zero-valued one; repeated codes are not summed.
 */
int BASE_GetEffectValue(STRUCT_ITEM* item, int effect)
{
    for (auto i : item->stEffect)
    {
        if (i.cEffect == effect)
            return i.cValue;
    }

    return 0;
}

/**
 * Updates the first occurrence of effect or uses the first empty slot.
 * item is borrowed, required, and changed in place. With no free slot this
 * does nothing and reports no failure. effect/value are cast to bytes without
 * range checks. The change is neither persisted nor published.
 */
void BASE_ChangeOrAddEffectValue(STRUCT_ITEM* item, int effect, int value)
{
    for (auto& i : item->stEffect)
    {
        if (i.cEffect == effect)
        {
            i.cValue = static_cast<unsigned char>(value);
            return;
        }
    }

    // Search every slot first: an earlier empty slot must not duplicate an
    // effect already present in a later slot.
    for (auto& i : item->stEffect)
    {
        if (i.cEffect == 0)
        {
            i.cEffect = static_cast<unsigned char>(effect);
            i.cValue = static_cast<unsigned char>(value);
            return;
        }
    }
}

/**
 * Clears the code and value of every occurrence of effect without compacting.
 * item is borrowed, required, and changed in place. Absence is a no-op;
 * this function neither persists the item nor sends an update to the server.
 */
void BASE_RemoveEffect(STRUCT_ITEM* item, int effect)
{
    for (auto& i : item->stEffect)
    {
        if (i.cEffect == effect)
        {
            i.cEffect = 0;
            i.cValue = 0;
        }
    }
}
