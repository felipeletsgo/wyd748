#include "pch.h"
// BASE_* functions split by domain; declarations stay in Basedef.h, global tables and loaders in Basedef.cpp.
#include "Basedef.h"
#include "TMGlobal.h"
#include "ServerListAsset.h"
#include "ObjectManager.h"
#include "TMHuman.h"
#include "../application/AttackSpeedPolicy.h"
#include "../application/PremiumMountMagic.h"
#include "../application/SkillDamagePreview.h"
#include "../application/SkillValidityPolicy.h"

static_assert(skill_validity::kSpellTableSize == MAX_SPELL_LIST,
    "Skill validity bound must match g_pSpell");

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
    const STRUCT_MOB& mob = g_pObjectManager->m_stMobData;
    return skill_validity::IsValidSkill(nSkillIndex, mob.LearnedSkill[0], mob.LearnedSkill[1]) ? 1 : 0;
}

int IsValidClassSkill(int nSkillIndex)
{
    const STRUCT_MOB& mob = g_pObjectManager->m_stMobData;
    return skill_validity::IsValidClassSkill(nSkillIndex,
        static_cast<unsigned char>(mob.Class), mob.LearnedSkill[0]) ? 1 : 0;
}

int BASE_GetManaSpent(int SkillNumber, int SaveMana, int Special)
{
    if (SkillNumber < 0 || SkillNumber >= 248) return 0;
    SaveMana = (std::max)(0, (std::min)(99, SaveMana));
    return skill_damage_preview::ManaCost(g_pSpell[SkillNumber].ManaSpent, Special, static_cast<unsigned int>(SaveMana));
}

// Skills, combat, and derived-attribute calculations.
int BASE_GetSkillDamage(int skillnum, STRUCT_MOB* mob, int weather, int weapondamage, int OriginalFace)
{
    if (!mob || skillnum < 0 || skillnum >= MAX_SPELL_LIST) return 0;
    const int formula = skill_damage_preview::FormulaSkill(mob->Class, skillnum);
    const skill_damage_preview::Caster caster{
        mob->Class, (std::max)(0, (std::min)(400, static_cast<int>(mob->CurrentScore.Level))),
        static_cast<int>(mob->CurrentScore.Mastery[formula % 24 / 8 + 1]),
        static_cast<int>(mob->CurrentScore.Str), static_cast<int>(mob->CurrentScore.Int),
        static_cast<int>(mob->CurrentScore.Attack), weapondamage};
    const int mountIndex = mob->Equip[14].sIndex;
    int premium = 0;
    if (mountIndex > 0 && mountIndex < MAX_ITEMLIST) {
        const int horsePercent = mountIndex >= 3980 && mountIndex <= 3989
            ? g_pMountBonus2[mountIndex - 3980][1] : 0;
        premium = premium_mount_magic::Percent(mountIndex, g_pItemList[mountIndex], horsePercent);
    }
    return skill_damage_preview::Damage(skillnum, g_pSpell[skillnum].InstanceType,
        g_pSpell[skillnum].InstanceValue, g_pSpell[skillnum].AffectValue, weather, OriginalFace,
        caster, mob->CurrentScore.MagicAmp, static_cast<int>(mob->BaseScore.MagicAttack),
        mob->LearnedSkill[0], premium);
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

    // A09-10: a never-read nUnique store indexed g_pItemList with unchecked
    // equipment indexes from 0x114/0x336 snapshots; it was removed.
    for (int i = 0; i < MAX_EQUIPITEM; ++i)
    {
        if (!mob->Equip[i].sIndex && i == 7)
            continue;

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

int BASE_GetAttackSpeedPercent(STRUCT_MOB* mob, const TMHuman* human)
{
    return attack_speed::Percent(mob->CurrentScore.Dex, mob->CurrentScore.Mastery[2],
        mob->LearnedSkill[0], BASE_GetMobAbility(mob, 26),
        human ? human->m_nClass : 0, human ? human->m_stLookInfo.FaceMesh : 0,
        human ? human->m_cSpeedUp : 0, human ? human->m_cSpeedDown : 0,
        human && human->m_cFreeze == 1);
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

    // Only local intentions use this prediction; observed hits use server flags.
    const auto human = g_pObjectManager && mob == &g_pObjectManager->m_stMobData
        ? g_pObjectManager->GetHumanByID(g_pObjectManager->m_dwCharID) : nullptr;
    const int percent = human ? BASE_GetAttackSpeedPercent(mob, human)
        : 100 + static_cast<int>((mob->CurrentScore.AttackRun >> 4) & 15) * 10;
    int hitvalue[2] =
    {
        attack_speed::DoubleChancePerMille(percent, mob->CurrentScore.AttackRun),
        // Critical is part of the shared canonical Score and is expressed in
        // tenths of a percent (1000 = 100%), matching the server roll.
        (int)(mob->CurrentScore.Critical > 1000 ? 1000 : mob->CurrentScore.Critical)
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
