#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "ServerListAsset.h"

int BASE_GetSpeed(STRUCT_SCORE* score)
{
    int Run;

    Run = score->AttackRun & 0xF;
    if (Run < 1)
        Run = 1;
    if (Run > 7)
        Run = 7;

    return Run;
}

int IsSkill(int nSkillIndex)
{
    if (nSkillIndex >= 5000 && nSkillIndex <= 5104)
        return 1;
    if (nSkillIndex >= 5400 && nSkillIndex <= 5447)
        return 1;

    return 0;
}

int GetSkillIndex(int nSkillIndex)
{
    if (nSkillIndex >= 5400)
        nSkillIndex -= 5200;
    else if (nSkillIndex >= 5000)
        nSkillIndex -= 5000;

    return nSkillIndex;
}


int IsValidSkill(int nSkillIndex)
{
    if (nSkillIndex >= 0 && nSkillIndex < 104)
    {
        if (nSkillIndex >= 96)
        {
            if (!((1 << (nSkillIndex - 72)) & g_pObjectManager->m_stMobData.LearnedSkill[0]))
                return 0;
        }
        else if (!((1 << nSkillIndex % 24) & g_pObjectManager->m_stMobData.LearnedSkill[0]))
            return 0;

        return 1;
    }
    else if (nSkillIndex == 205 && g_pObjectManager->m_stMobData.LearnedSkill[1] & 0x20)
        return 1;
    else if (nSkillIndex == 233 && g_pObjectManager->m_stMobData.LearnedSkill[1] & 0x200)
        return 1;
    else if (nSkillIndex == 238 && g_pObjectManager->m_stMobData.LearnedSkill[1] & 4)
        return 1;
    else if (nSkillIndex < 200 || nSkillIndex >= 247)
        return 0;
    else if (nSkillIndex == 205 || nSkillIndex == 233 || nSkillIndex == 238)
        return 0;
    else
        return ((1 << 4 * ((nSkillIndex - 200) / 4)) & g_pObjectManager->m_stMobData.LearnedSkill[1]) != 0;
}

int IsValidClassSkill(int nSkillIndex)
{
    if (nSkillIndex >= 0 && nSkillIndex < 96)
    {
        if (nSkillIndex / 24 != (unsigned __int8)g_pObjectManager->m_stMobData.Class)
            return 0;
        if (!((1 << nSkillIndex % 24) & g_pObjectManager->m_stMobData.LearnedSkill[0]))
            return 0;

        int kind = nSkillIndex % 24 / 8 + 1;
        if (kind <= 0 || kind > 3)
            return 0;
    }
    else if (nSkillIndex < 96 || nSkillIndex > 103 &&
        ((1 << (nSkillIndex - 72)) & g_pObjectManager->m_stMobData.LearnedSkill[0]))
    {
        return 1;
    }

    return 1;
}

int BASE_GetManaSpent(int SkillNumber, int SaveMana, int Special)
{
    return g_pSpell[SkillNumber].ManaSpent * (Special / 2 + 100) / 100 * (100 - SaveMana) / 100;
}

int BASE_GetSkillDamage(int dam, int ac, int combat)
{
    int tdam{};

    if (combat > 15)
        combat = 15;

    tdam = (rand() % (21 - combat) + combat + 90) * (dam - ac / 2) / 100;
    if (tdam < -50)
        tdam = 0;
    else if (tdam >= -50 && tdam < 0)
        tdam = (tdam + 50) / 10;
    else if (tdam >= 0 && tdam <= 45)
        tdam = 5 * tdam / 4 + 5;

    if (tdam <= 0)
        tdam = 1;

    return tdam;
}

// Skills, combat, and derived-attribute calculations.
int BASE_GetSkillDamage(int skillnum, STRUCT_MOB* mob, int weather, int weapondamage, int OriginalFace)
{
    int instanceindex = g_pSpell[skillnum].InstanceType;//ok

    int level = mob->CurrentScore.Level;//ok

    if (level < 0)//k
        level = 0;

    if (level >= 400)
        level = 400;

    int special = mob->CurrentScore.Mastery[skillnum % 24 / 8 + 1];//ok
    int base = g_pSpell[skillnum].InstanceValue;
    int affectbase = g_pSpell[skillnum].AffectValue;
    int skillclass = skillnum / 8 % 3;
    int dam = 0;

    if (instanceindex == 0)
    {
        switch (skillnum)
        {
        case 11:
            dam = affectbase + special / 10;
            break;

        case 13:
            dam = affectbase + 3 * special;
            break;
        case 41:
            dam = special / 25 + 2;
            break;
        case 43:
            dam = affectbase + special / 3 + 15;
            break;
        case 44:
            dam = 5 * (special / 3 + 15);
            break;
        case 45:
            dam = affectbase + special / 10;
            break;
        }
    }
    else if (instanceindex >= 1 && instanceindex <= 5)
    {
        int skind = skillnum / 8;
        int trans = 0;
        if (OriginalFace % 10 > 5)
            trans = 1;

        if (trans == 0)
        {
            if (skillnum == 97)
                dam = base + 15 * level;
            else if (!mob->Class && skind == 1)
                dam = 3 * weapondamage + 3 * mob->CurrentScore.Str + level / 2 + special + base;
            else if (!mob->Class && skind != 1)
                dam = weapondamage + mob->CurrentScore.Int / 4 + level / 2 + special + base + mob->CurrentScore.Int / 30;
            {
                switch (mob->Class)
                {
                case 1:
                    dam = mob->CurrentScore.Int / 30 + mob->CurrentScore.Int / 3 + level / 2 + special + base;
                    break;
                case 2:
                    dam = mob->CurrentScore.Int / 30 + mob->CurrentScore.Int / 3 + level / 2 + special + base;
                    break;
                case 3:
                    if (skillnum == 79)
                        dam = mob->CurrentScore.Attack;
                    else
                        dam = 3 * weapondamage + 3 * mob->CurrentScore.Str + level / 2 + special + base;
                    break;
                }
            }
        }
        else if (skillnum == 97)
            dam = base + 15 * level;
        else if (!mob->Class && skind == 1)
            dam = 3 * weapondamage + 3 * mob->CurrentScore.Str + level + special + base;
        else if (!mob->Class && skind != 1)
            dam = weapondamage + mob->CurrentScore.Int / 4 + level + special + base + mob->CurrentScore.Int /40;
        else
        {
            switch (mob->Class)
            {
            case 1:
                dam = mob->CurrentScore.Int / 30 + mob->CurrentScore.Int / 3 + level + base + 2 * special;
                break;
            case 2:
                dam = mob->CurrentScore.Int / 30 + mob->CurrentScore.Int / 3 + level + base + 2 * special;
                break;
            case 3:
                if (skillnum == 79)
                    dam = mob->CurrentScore.Attack;
                else
                    dam = 3 * weapondamage + 3 * mob->CurrentScore.Str + level / 2 + special + base;
                break;
            }
        }
        if (weather == 1)
        {
            if (instanceindex == 2)
                dam = 90 * dam / 100;
            if (instanceindex == 5)
                dam = 130 * dam / 100;
        }
        else if (weather == 2 && instanceindex == 3)
            dam = 120 * dam / 100;
        if (skillnum != 97)
        {
            if (skillnum == 79)
                return dam;
            if (!mob->Class && skind == 1 || mob->Class == 3)
                dam = 5 * dam / 4;
            else
                // The 7.48 canonical Score owns MagicAmp; the removed Magician
                // sidecar represented the same value and must not be read again.
                dam = 5 * (dam * (4 * (unsigned __int8)mob->CurrentScore.MagicAmp + 100) / 100) / 4;
        }
        if ((1 << (8 * skillclass)) & mob->LearnedSkill[0])
        {
            switch (mob->Class)
            {
            case 0:
                if (!skillclass)
                    dam = 115 * dam / 100;
                else if (skillclass == 1)
                    dam = 120 * dam / 100;
                else if (skillclass == 2)
                    dam = 115 * dam / 100;
                break;
            case 1:
                if (!skillclass)
                    dam = 110 * dam / 100;
                else if (skillclass == 1)
                    dam = 115 * dam / 100;
                else if (skillclass == 2)
                    dam = 115 * dam / 100;
                break;
            case 2:
                if (!skillclass)
                    dam = 110 * dam / 100;
                break;
            case 3:
                if (!skillclass)
                    dam = 110 * dam / 100;
                else if (skillclass == 1)
                    dam = 110 * dam / 100;
                else if (skillclass == 2)
                    dam = 120 * dam / 100;
                break;
            }
        }
    }
    else if (instanceindex == 6)
    {
        dam = base + 3 * special / 2;
        if (skillnum == 29 && mob->LearnedSkill[0] & 0x80)
            dam = 120 * dam / 100;
    }
    else if (instanceindex == 11)
        dam = g_pSpell[skillnum].InstanceValue;
    else
        // Skill instance 11 also consumes the canonical MagicAmp field.
        dam = 2 * (unsigned char)mob->CurrentScore.MagicAmp;

    return dam;
}

// Final state validation, critical hits, and low-level utilities.
int BASE_GetMobAbility(STRUCT_MOB* mob, char Type)
{
    int value = 0;
    if (Type == 27)
    {
        value = BASE_GetMaxAbility(mob, Type);
        if (value < 2 && mob->Class == 3)
        {
            if (mob->LearnedSkill[0] & 0x80000)
                value = 2;
        }

        return value;
    }

    int nUnique[MAX_EQUIPITEM]{};
    for (int i = 0; i < MAX_EQUIPITEM; ++i)
    {
        if (!mob->Equip[i].sIndex && i == 7)
            continue;

        if (i >= 1 && i <= 5)
            nUnique[i] = g_pItemList[mob->Equip[i].sIndex].nUnique;

        if ((Type == 2 && i == 6) || (Type == 60 && i == 7)) // changed
            continue;

        if (i == 7 && Type == 2)
        {
            int ldam = BASE_GetItemAbility(&mob->Equip[6], 73) + BASE_GetItemAbility(&mob->Equip[6], Type);
            int rdam = BASE_GetItemAbility(&mob->Equip[7], 73) + BASE_GetItemAbility(&mob->Equip[7], Type);

            int lidx = mob->Equip[6].sIndex;
            int ridx = mob->Equip[7].sIndex;

            int ltype = 0;
            if (lidx > 0 && lidx < 6500)
                ltype = g_pItemList[lidx].nUnique;

            int rtype = 0;
            if (ridx > 0 && ridx < 6500)
                rtype = g_pItemList[ridx].nUnique;

            if (!ltype || !rtype)
            {
                if (ldam <= rdam)
                    value += rdam;
                else
                    value += ldam;
            }
            else if (ltype == 47 && rtype == 45)
            {
                value += ldam;
            }
            else
            {
                int multi = 0;
                if (ltype == rtype)
                    multi = 50;
                else
                    multi = 30;
                if (!mob->Class && mob->LearnedSkill[0] & 0x200)
                    multi += 15;
                if (mob->Class == 3 && mob->LearnedSkill[0] & 0x400)
                    multi += 10;
                if (ldam <= rdam)
                    value += multi * ldam / 100 + rdam;
                else
                    value += multi * rdam / 100 + ldam;
            }
        }
        else
        {
            value += BASE_GetItemAbility(&mob->Equip[i], Type);
        }
    }

    if (value < 0)
        value = 0;

    return value;
}

int BASE_GetMaxAbility(STRUCT_MOB* mob, char Type)
{
    int value = 0;
    for (int i = 0; i < MAX_EQUIPITEM; ++i)
    {
        if (mob->Equip[i].sIndex)
        {
            int tvalue = BASE_GetItemAbility(&mob->Equip[i], Type);
            if (value < tvalue)
                value = tvalue;
        }
    }

    return value;
}

int BASE_GetDoubleCritical(STRUCT_MOB* mob, unsigned short* sProgress, unsigned short* cProgress, char* bDoubleCritical)
{
    *bDoubleCritical = 0;
    if (!cProgress)
        return 0;
    if ((int)*cProgress >= 1024)
        *cProgress %= 1024;
    if (sProgress && (int)*sProgress >= 1024)
        *sProgress %= 1024;

    int hitvalue[2] =
    {
        100 * (((int)(unsigned char)mob->CurrentScore.AttackRun >> 4) - 5),
        // Critical is part of the shared canonical Score in the 7.48 source
        // client; keeping a second byte in STRUCT_MOB caused ABI divergence.
        4 * (unsigned char)mob->CurrentScore.Critical
    };

    if (sProgress && cProgress && *cProgress != *sProgress)
    {
        if (*sProgress > (int)*cProgress)
            *cProgress = *sProgress;
        if (*sProgress < (int)*cProgress)
        {
            if (*sProgress + 5 < *cProgress)
            {
                *cProgress = *sProgress;
                return 0;
            }
            *sProgress = *cProgress;
        }
    }

    int value = g_pHitRate[*cProgress];
    for (int i = 0; i < 2; ++i)
    {
        int bit = 0;
        if (!i && value < hitvalue[0])
            bit = 1;
        if (i == 1 && 1000 - value < hitvalue[1])
            bit = 1;
        *bDoubleCritical |= bit << i;
    }

    if (sProgress)
        ++*sProgress;

    ++*cProgress;
    return 1;
}

/**
 * Normalizes the supplied ID (>=5400: -5200; >=5000: -5000) and reads Passive
 * from the global g_pSpell table. Returns 1 only when that field is 1.
 * The caller must ensure the table is loaded. Invalid normalized indexes
 * return false; this function does not modify the table.
 */
int IsPassiveSkill(int nSkillIndex)
{
    if (nSkillIndex >= 5400)
        nSkillIndex -= 5200;
    else if (nSkillIndex >= 5000)
        nSkillIndex -= 5000;

    if (nSkillIndex < 0 || nSkillIndex >= MAX_SPELL_LIST)
        return 0;

    return g_pSpell[nSkillIndex].Passive == 1;
}
