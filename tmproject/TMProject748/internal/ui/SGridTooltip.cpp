#include "pch.h"
// SGridControl split by responsibility; see SGrid.cpp for the grid model.
#include "SGrid.h"
#include "GridInsertion.h"
#include "../application/NativeVolatileRoutes.h"
#include "TMGlobal.h"
#include "SControlContainer.h"
#include "TMMesh.h"
#include "TMFieldScene.h"
#include "TMUtil.h"
#include "ItemEffect.h"
#include "ClientDiagnostics.h"
#include "SellConfirmationText.h"
#include "NativeSaleQuote.h"
#include "SGridSupport.h"


void SGridControl::UpdateDescPanelHeight()
{
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);
	// Native 7.48 FUN_00435b13 initializes every tooltip control before grids
	// receive mouse input.  Compatibility bootstrap is intentionally defensive:
	// a missing or custom FieldScene2 tooltip must disable resizing, never turn a
	// harmless mouse move into a null dereference inside SGridControl::OnMouseEvent.
	if (!pFScene || !pFScene->m_pDescPanel)
		return;
	auto lastParamText = pFScene->m_pParamText[pFScene->GetLascDescParamId()];
	if (!lastParamText)
		return;
	float descPanelHeight = lastParamText->m_nPosY + lastParamText->m_Font.m_FontSize + 25.0f;

	pFScene->m_pDescPanel->SetHeight(descPanelHeight);
}

int SGridControl::MouseOver(int nCellX, int nCellY, int bPtInRect)
{
	const auto eCursorStyle = g_pCursor->GetStyle();
	const bool bSkillBeltCrossHair =
		m_eGridType == TMEGRIDTYPE::GRID_SKILLB && eCursorStyle == ECursorStyle::TMC_CURSOR_CROSS_HAIR;
	if (eCursorStyle != ECursorStyle::TMC_CURSOR_HAND && !bSkillBeltCrossHair)
	{
		return MouseOverWithoutHandCursor(eCursorStyle, nCellX, nCellY);
	}

	m_dwEnableColor = 0;
	m_vecPickupedPos.x = 0;
	m_vecPickupedPos.y = 0;
	m_vecPickupedSize.x = 0;
	m_vecPickupedSize.y = 0;
	if (!bPtInRect && g_pCurrentScene->m_pDescPanel)
	{
		m_dwEnableColor = 0;
		return 0;
	}
	auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

	auto pItem = GetItem(nCellX, nCellY);
	auto pDescPanel = pFScene->m_pDescPanel;
	if (!pDescPanel || !pFScene->m_pParamText[11])
		return 2;
	pDescPanel->SetAlwaysOnTop(0);
	pDescPanel->SetVisible(0);

	// FUN_00435b13 creates exactly twelve native description rows (0..11).
	// Keep price/status text on the last real FieldScene2 control instead of
	// addressing the two tooltip rows introduced by TMProject 7.59.
	auto pParamText = pFScene->m_pParamText[11];
	pParamText->SetTextColor(0xFFFFFFFF);

	if (!pItem || !pDescPanel)
		return 2;


	float nPosX = 0.0f;
	float nPosY = 0.0f;

	auto vecCursorPos = g_pCursor->GetPos();
	auto vecDescPanelPos = pDescPanel->GetPos();

	if ((float)g_pDevice->m_dwScreenWidth <= ((float)(pDescPanel->m_nWidth / 2.0f) + vecCursorPos.x))
		nPosX = (float)g_pDevice->m_dwScreenWidth - pDescPanel->m_nWidth;
	else
		nPosX = vecCursorPos.x - (float)(pDescPanel->m_nWidth / 2.0f);

	if ((float)((g_pDevice->m_dwScreenHeight >> 1) - 30) <= vecCursorPos.y)
		nPosY = (float)(vecCursorPos.y - pDescPanel->m_nHeight) - (float)(10.0f * RenderDevice::m_fHeightRatio);
	else
		nPosY = (float)(30.0f * RenderDevice::m_fHeightRatio) + vecCursorPos.y;

	pDescPanel->SetRealPos(nPosX, nPosY);
	pDescPanel->SetVisible(1);

	unsigned int dwServerTime = g_pTimerManager->GetServerTime();

	if (SGridControl::m_pLastMouseOverItem == pItem && SGridControl::m_sLastMouseOverIndex == pItem->m_pItem->sIndex &&
		!SGridControl::m_bNeedUpdate)
	{
		if (pItem->m_pItem->sIndex != 3324 && pItem->m_pItem->sIndex != 3325 && pItem->m_pItem->sIndex != 3326)
			return 1;
		if (pFScene->m_dwNightmareTime > dwServerTime - 1000)
			return 1;
	}
	else if (pItem->m_pItem->sIndex >= 10000 && pItem->m_pItem->sIndex < 11500)
	{
		auto pDescNameText = pFScene->m_pDescNameText;
		pDescNameText->SetText(g_pItemMixHelp[pItem->m_pItem->sIndex].Name, 0);
		pDescNameText->SetTextColor(0x0FFAAAAFF);

		pFScene->m_pParamText[0]->SetText((char*)"                                 ", 0);
		pFScene->m_pParamText[1]->SetText((char*)"                                 ", 0);

		if (g_pItemMixHelp[pItem->m_pItem->sIndex].Color[0])
		{
			for (int i = 0; i < 9; ++i)
			{
				pFScene->m_pParamText[i + 2]->SetText((char*)"                                 ", 0);

				if (g_pItemMixHelp[pItem->m_pItem->sIndex].Help[i][0] != '\0')
				{
					pFScene->m_pParamText[i + 2]->SetTextColor(g_pItemMixHelp[pItem->m_pItem->sIndex].Color[i]);
					pFScene->m_pParamText[i + 2]->SetText(g_pItemMixHelp[pItem->m_pItem->sIndex].Help[i], 0);
				}
			}
		}

		SGridControl::m_pLastMouseOverItem = pItem;
		SGridControl::m_sLastMouseOverIndex = pItem->m_pItem->sIndex;
		return 1;
	}
	else if (pItem->m_pItem->sIndex >= 11500 && pItem->m_pItem->sIndex < 11600)
	{
		auto pDescNameText = pFScene->m_pDescNameText;
		int itemId = pItem->m_pItem->sIndex - 11500;

		pDescNameText->SetText(pFScene->m_MissionClass.m_stMissionHelp[itemId].Name, 0);

		if (itemId >= 50)
			pDescNameText->SetText(g_pMessageStringTable[434], 0);

		pDescNameText->SetTextColor(0x0FFAAAAFF);
		pFScene->m_pParamText[0]->SetText((char*)"                                 ", 0);
		pFScene->m_pParamText[1]->SetText((char*)"                                 ", 0);

		if (pFScene->m_MissionClass.m_stMissionHelp[itemId].Color[0])
		{
			for (int k = 0; k < 9; ++k)
			{
				if (strcmp(pFScene->m_MissionClass.m_stMissionHelp[itemId].Help[k], ""))
				{
					pFScene->m_pParamText[k]->SetTextColor(pFScene->m_MissionClass.m_stMissionHelp[itemId].Color[k]);
					pFScene->m_pParamText[k]->SetText(pFScene->m_MissionClass.m_stMissionHelp[itemId].Help[k], 0);
				}
				else
					pFScene->m_pParamText[k]->SetText((char*)"                            ", 0);
			}
		}

		SGridControl::m_pLastMouseOverItem = pItem;
		SGridControl::m_sLastMouseOverIndex = pItem->m_pItem->sIndex;
		return 1;
	}
	else if (pItem->m_pItem->sIndex == 3443)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = DescribeItem3443(pItem, pParamText, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}
	else if (pItem->m_pItem->sIndex == 3444)
	{
		int extractedResult{};
		const ExtractedFlow extractedFlow = DescribeItem3444(pFScene, pItem, pParamText, extractedResult);
		if (extractedFlow == ExtractedFlow::Return)
			return extractedResult;
	}

	SGridControl::m_pLastMouseOverItem = pItem;
	SGridControl::m_sLastMouseOverIndex = pItem->m_pItem->sIndex;

	int nSanc = BASE_GetItemSanc(pItem->m_pItem);
	int nGuildId = BASE_GetItemAbility(pItem->m_pItem, 57) | (BASE_GetItemAbility(pItem->m_pItem, 56) << 8);

	auto pDescNameText = pFScene->m_pDescNameText;

	if (pDescNameText && nGuildId)
	{
		char szText[128]{};
		char src[128]{};

		strcat(szText, g_pItemList[pItem->m_pItem->sIndex].Name);
		if (nSanc > 0 && BASE_GetItemAbility(pItem->m_pItem, 17) > 0)
		{
			sprintf(src, " +%d", nSanc);
			strcat(szText, src);
		}

		pDescNameText->SetText(szText, 0);
		pDescNameText->SetTextColor(BASE_GetItemColor(pItem->m_pItem));
	}
	else if (pDescNameText)
	{
		DescribeItemName(pItem, pDescNameText, nSanc);
	}

	int nAddHP = BASE_GetItemAbility(pItem->m_pItem, 4);
	int nAddMP = BASE_GetItemAbility(pItem->m_pItem, 5);
	unsigned int nItemPos = BASE_GetItemAbility(pItem->m_pItem, 17);
	int nWeaponType = BASE_GetItemAbility(pItem->m_pItem, 21);
	int nClassType = BASE_GetItemAbility(pItem->m_pItem, 18);
	int nLineId = 0;
	if (nClassType & 1)
	{
		SGridControl::m_szParamString[23] = g_pMessageStringTable[106];
		SGridControl::m_szParamString[24] = g_pMessageStringTable[107];
		SGridControl::m_szParamString[25] = g_pMessageStringTable[108];
		SGridControl::m_szParamString[26] = g_pMessageStringTable[109];
	}
	else if (nClassType & 2)
	{
		SGridControl::m_szParamString[23] = g_pMessageStringTable[106];
		SGridControl::m_szParamString[24] = g_pMessageStringTable[110];
		SGridControl::m_szParamString[25] = g_pMessageStringTable[111];
		SGridControl::m_szParamString[26] = g_pMessageStringTable[112];
	}
	else if (nClassType & 4)
	{
		SGridControl::m_szParamString[23] = g_pMessageStringTable[106];
		SGridControl::m_szParamString[24] = g_pMessageStringTable[113];
		SGridControl::m_szParamString[25] = g_pMessageStringTable[114];
		SGridControl::m_szParamString[26] = g_pMessageStringTable[115];
	}
	else if (nClassType & 8)
	{
		SGridControl::m_szParamString[23] = g_pMessageStringTable[106];
		SGridControl::m_szParamString[24] = g_pMessageStringTable[133];
		SGridControl::m_szParamString[25] = g_pMessageStringTable[134];
		SGridControl::m_szParamString[26] = g_pMessageStringTable[135];
	}
	if (nClassType == 255)
	{
		SGridControl::m_szParamString[23] = g_pMessageStringTable[95];
		SGridControl::m_szParamString[24] = g_pMessageStringTable[96];
		SGridControl::m_szParamString[25] = g_pMessageStringTable[97];
		SGridControl::m_szParamString[26] = g_pMessageStringTable[98];
	}

	auto pMobData = &g_pObjectManager->m_stMobData;

	unsigned int dwColor = 0;
	if (BASE_CanEquip(pItem->m_pItem, &pMobData->CurrentScore, -1, pMobData->Equip[0].sIndex, pMobData->Equip,
		g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex, pMobData->HasSoulSkill()))
	{
		dwColor = 0x0FFFFFFF;
	}
	else
		dwColor = 0xFFFF0000;

	// FieldScene2.bin exposes only the twelve rows recovered from FUN_00435b13.
	for (int l = 0; l < NUM_ITEM_DESC_PARAMS; ++l)
		pFScene->m_pParamText[l]->SetText((char*)"", 0);

	if (pItem->m_pItem->sIndex >= 10000)
		return 0;

	if (g_pItemHelp[pItem->m_pItem->sIndex].Color[0])
	{
		int nId = 0;
		for (int m = 0; m < 9; ++m)
		{
			if (strcmp(g_pItemHelp[pItem->m_pItem->sIndex].Help[m], ""))
			{
				pFScene->m_pParamText[nId]->SetTextColor(g_pItemHelp[pItem->m_pItem->sIndex].Color[m]);
				pFScene->m_pParamText[nId]->SetText(g_pItemHelp[pItem->m_pItem->sIndex].Help[m], 0);

			}
			++nId;
		}
	}

	char szDesc[128]{};
	if (IsSkill(pItem->m_pItem->sIndex) == 1)
	{
		DescribeSkillItem(pItem, pFScene, dwColor, nLineId, szDesc);
	}
	else if (pItem->m_pItem->sIndex == 3324 || pItem->m_pItem->sIndex == 3325 || pItem->m_pItem->sIndex == 3326 ||
		pItem->m_pItem->sIndex == 3390 || pItem->m_pItem->sIndex == 3391 || pItem->m_pItem->sIndex == 3392 ||
		pItem->m_pItem->sIndex == 3328 || pItem->m_pItem->sIndex == 3329)
	{
		if (pFScene->m_dwLastNightmareTime || dwServerTime > pFScene->m_dwLastNightmareTime + 60000)
		{
			MSG_MessageWhisper Msg{};
			Msg.Header.ID = g_pObjectManager->m_dwCharID;
			Msg.Header.Type = MSG_MessageWhisper_Opcode;
			sprintf(Msg.MobName, "nig");
			SendOneMessage((char*)& Msg, sizeof(Msg));

			if (!pFScene->m_dwLastNightmareTime)
			{
				pFScene->m_dwLastNightmareTime = dwServerTime;
				pFScene->m_NightmareTime.wHour = 0;
				pFScene->m_NightmareTime.wMonth = 0;
				pFScene->m_NightmareTime.wSecond = 0;
			}

			unsigned int nowNightTime = ((dwServerTime - pFScene->m_dwLastNightmareTime) / 1000);
			unsigned int nextNightTime = pFScene->m_NightmareTime.wSecond
				+ 60 * pFScene->m_NightmareTime.wMonth
				+ 3600 * pFScene->m_NightmareTime.wHour;

			unsigned int nightTime = nowNightTime + nextNightTime;
			unsigned int min = 20;

			unsigned int leftTime = 0;
			if (pItem->m_pItem->sIndex == 3324 || pItem->m_pItem->sIndex == 3390)
			{
				leftTime = 1200 * (nightTime / 1200) + 1200;
			}
			else if (pItem->m_pItem->sIndex == 3325 || pItem->m_pItem->sIndex == 3391)
			{
				leftTime = 1200 * (nightTime / 1200) + 1500;
			}
			else if (pItem->m_pItem->sIndex == 3326 || pItem->m_pItem->sIndex == 3392)
			{
				leftTime = 1200 * (nightTime / 1200) + 1800;
			}
			else if (pItem->m_pItem->sIndex == 3328 || pItem->m_pItem->sIndex == 3329)
			{
				leftTime = 1800 * (nightTime / 1800) + 1800;
				min = 30;
			}

			if (min - 4 <= ((leftTime - nightTime) / 60 % min))
			{
				sprintf(szDesc, "0 : 0");
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFAA);
				++nLineId;

				sprintf(szDesc, g_pMessageStringTable[280]);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
				++nLineId;
			}
			else
			{
				sprintf(szDesc, "%02d : %02d", (leftTime - nightTime) / 60 % min, (leftTime - nightTime) % 60);
				pFScene->m_dwNightmareTime = dwServerTime;
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFAAFFAA);
				++nLineId;

				sprintf(szDesc, g_pMessageStringTable[281]);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFAAFFAA);
			}
		}
		++nLineId;
	}
	else if (pItem->m_pItem->sIndex >= 3000 && pItem->m_pItem->sIndex <= 3015
		|| pItem->m_pItem->sIndex >= 3050 && pItem->m_pItem->sIndex <= 3099)
	{
		auto itemEffect = pItem->m_pItem->stEffect;

		unsigned char date = 0;
		unsigned char year = 0;
		unsigned char month = 0;
		for (int i = 0; i < 3; ++i)
		{
			switch (itemEffect[i].cEffect)
			{
			case EF_DATE:
				date = (unsigned char)itemEffect[i].cValue;
				break;
			case EF_YEAR:
				year = (unsigned char)itemEffect[i].cValue;
				break;
			case EF_MONTH:
				month = (unsigned char)itemEffect[i].cValue;
				break;
			}
		}

		char formattedDate[128]{};
		char formattedYear[128]{};
		char formattedMonth[128]{};

		if (date)
			sprintf(formattedDate, g_pMessageStringTable[291], date);
		else
			sprintf(formattedDate, "");
		if (year)
			sprintf(formattedYear, g_pMessageStringTable[297], year + 2000);
		else
			sprintf(formattedYear, "");
		if (month)
			sprintf(formattedMonth, g_pMessageStringTable[296], month);
		else
			sprintf(formattedMonth, "");

		char timeTypeStr[128]{};
		sprintf(timeTypeStr, g_pMessageStringTable[298], 0);
		sprintf(szDesc, "%s %s %s %s 0%s", g_pMessageStringTable[299], formattedYear, formattedMonth, formattedDate, timeTypeStr);

		pFScene->m_dwNightmareTime = dwServerTime;
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFAA);
		++nLineId;

		for (int i = 1; i < 49; ++i)
		{
			int add = BASE_GetStaticItemAbility(pItem->m_pItem, dwEFParam[i]);
			if (dwEFParam[i] == 2 && BASE_GetItemAbility(pItem->m_pItem, 17) != 32)
			{
				sprintf(szDesc, "%s : %d", SGridControl::m_szParamString[i], add);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
				++nLineId;
			}
		}
	}
	else if (pItem->m_pItem->sIndex >= 3900 && pItem->m_pItem->sIndex < 3980)
	{
		DescribeTimedItem(pItem, pFScene, nLineId, szDesc);
	}
	else if (pItem->m_pItem->sIndex >= 4106 && pItem->m_pItem->sIndex < 4110)
	{
		// Yeah!
	}
	else if (pItem->m_pItem->sIndex == 4147)
	{
		return DescribeItem4147(nLineId, pFScene, pItem, szDesc);
	}
	else
	{
		DescribeGeneralItem(pItem, pFScene, dwColor, dwServerTime, nItemPos, nLineId, nWeaponType, pMobData, szDesc);
	}

	int hGuild = BASE_GetItemAbility(pItem->m_pItem, 56);
	if (hGuild > 0)
	{
		sprintf(szDesc, g_pMessageStringTable[439], hGuild / 10, hGuild % 10);
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFF00);
		++nLineId;
	}
	hGuild = BASE_GetItemAbility(pItem->m_pItem, 57);
	if (hGuild > 0)
	{
		sprintf(szDesc, g_pMessageStringTable[440], hGuild / 10, hGuild % 10);
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFF00);
		++nLineId;
	}

	int nGrade = g_pItemList[pItem->m_pItem->sIndex].nGrade;
	if (pItem->m_pItem
		&& (nGrade >= 5 && nGrade <= 10 || nGrade >= 20 && nGrade <= 22 || nGrade >= 24 && nGrade <= 26 || nSanc >= 10))
	{
		if (nSanc >= 10)
		{
			int sancValue = BASE_GetSancEffValue(*pItem->m_pItem);

			int sancCalc = (sancValue - 230) % 4;
			int mult = 1;
			int sanc = BASE_GetItemSanc(pItem->m_pItem) - 9;
			if (sanc < 0)
				sanc = 0;
			if (nGrade >= 5 && nGrade <= 8)
				mult = 2;

			if (!sancCalc)
				sprintf(szDesc, g_pMessageStringTable[195], 8 * mult);
			if (sancCalc == 1)
				sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], mult * 40 * sanc);
			if (sancCalc == 2)
				sprintf(szDesc, g_pMessageStringTable[197], 2 * mult);
			if (sancCalc == 3)
				sprintf(szDesc, "%s : %d", g_pMessageStringTable[198], mult * 40 * sanc);
		}
		else
		{
			if (nGrade == 5)
				sprintf(szDesc, g_pMessageStringTable[195], 8);
			if (nGrade == 6)
				sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], 40);
			if (nGrade == 7)
				sprintf(szDesc, g_pMessageStringTable[197], 2);
			if (nGrade == 8)
				sprintf(szDesc, "%s : %d", g_pMessageStringTable[198], 40);
		}

		int sancCalc = 10;

		if (nGrade >= 20)
			sancCalc = 15;
		if (nGrade >= 23)
			sancCalc = 30;
		if (nGrade >= 26)
			sancCalc = 40;

		int otherSancCalc = nSanc * sancCalc / 10;
		if (nSanc >= 9)
			otherSancCalc = sancCalc;

		switch (nGrade)
		{
		case 9:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], otherSancCalc + sancCalc);
			break;
		case 10:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[198], otherSancCalc + sancCalc);
			break;
		case 20:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], otherSancCalc + sancCalc);
			break;
		case 21:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[198], otherSancCalc + sancCalc);
			break;
		case 22:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], otherSancCalc + sancCalc);
			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
			++nLineId;
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[198], otherSancCalc + sancCalc);
			break;
		case 24:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], otherSancCalc + sancCalc);
			break;
		case 25:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], otherSancCalc + sancCalc);
			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
			++nLineId;
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[198], otherSancCalc + sancCalc);
			break;
		case 26:
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[196], otherSancCalc + sancCalc);
			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
			++nLineId;
			sprintf(szDesc, "%s : %d", g_pMessageStringTable[198], otherSancCalc + sancCalc);
			break;
		}

		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
		++nLineId;
	}

	if (!pParamText)
		return 1;

	if (m_eGridType != TMEGRIDTYPE::GRID_SHOP)
	{
		return DescribeOutsideShop(pItem, pParamText);
	}



	const int itemIndex = pItem->m_pItem->sIndex;
	const int nItemPrice = WYD748_IsValidItemIndex(static_cast<short>(itemIndex))
		? g_pItemList[itemIndex].nPrice : 0;
	char szText[128]{};

	// FUN_00418828 in WYD.exe 7.48 has exactly three shop-price modes: the
	// fixed 1% tax group, MP, and Gold plus the current kingdom tax.  Removing
	// later donate/coupon/repurchase branches keeps the displayed amount equal
	// to the value used by native BuyItem and by the 7.48 server protocol.
	if (itemIndex == 4010 || itemIndex == 4011 ||
		(itemIndex >= 4026 && itemIndex <= 4029))
	{
		sprintf(szText, g_pMessageStringTable[57], nItemPrice + nItemPrice / 100);
		sprintf(szText, "%s (%s:%d%%)", szText, g_pMessageStringTable[146], 1);
	}
	else if (pFScene->m_nIsMP == 1)
	{
		sprintf(szText, g_pMessageStringTable[385], nItemPrice);
	}
	else
	{
		const int taxPrice = static_cast<int>(static_cast<float>(nItemPrice)
			* (static_cast<float>(g_pObjectManager->m_nTax) / 100.0f));
		sprintf(szText, g_pMessageStringTable[57], nItemPrice + taxPrice);
		sprintf(szText, "%s (%s:%d%%)", szText, g_pMessageStringTable[146],
			g_pObjectManager->m_nTax);
	}

	pParamText->SetText(szText, 0);
	return 1;
}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
int SGridControl::MouseOverWithoutHandCursor(const ECursorStyle& eCursorStyle, int& nCellX, int& nCellY)
{
	// A stale enable color must not remain visible after the cursor releases
	// its item or changes mode outside the native pickup flow.
	m_dwEnableColor = 0;
	if (eCursorStyle == ECursorStyle::TMC_CURSOR_PICKUP && g_pCursor->m_pAttachedItem)
	{
		auto pDescPanel = g_pCurrentScene->m_pDescPanel;
		if (pDescPanel)
			pDescPanel->SetVisible(0);

		m_vecPickupedPos.x = nCellX;
		m_vecPickupedPos.y = nCellY;
		m_vecPickupedSize.x = g_pCursor->m_pAttachedItem->m_nCellWidth;
		m_vecPickupedSize.y = g_pCursor->m_pAttachedItem->m_nCellHeight;

		if (m_vecPickupedSize.x < 0 || m_vecPickupedSize.x > 16)
			return 2;
		if (m_vecPickupedSize.y < 0 || m_vecPickupedSize.y > 16)
			return 2;

		auto pFScene = static_cast<TMFieldScene*>(g_pCurrentScene);

		// Only the native 7.48 Carry and Cargo controls can clip a dragged item
		// against their visible bounds; later-client page controls do not exist.
		if (pFScene->m_pGridInv == this || pFScene->m_pCargoGrid == this)
		{
			IVector2 vecGrid;
			vecGrid.x = m_vecPickupedSize.x + nCellX;
			vecGrid.y = m_vecPickupedSize.y + nCellY;

			if (vecGrid.x > m_nColumnGridCount)
				m_vecPickupedSize.x -= vecGrid.x - m_nColumnGridCount;
			if (vecGrid.y > m_nRowGridCount)
				m_vecPickupedSize.y -= vecGrid.y - m_nRowGridCount;
		}

		if (CanChangeItem(g_pCursor->m_pAttachedItem, nCellX, nCellY, 1))
			m_dwEnableColor = 0x330000FF;
		else if (m_eGridType == TMEGRIDTYPE::GRID_SHOP)
			m_dwEnableColor = 0x330000FF;
		else if (m_eGridType == TMEGRIDTYPE::GRID_SKILLB)
			m_dwEnableColor = 0x33FF0000;
		else
		{
			auto pItem = GetItem(nCellX, nCellY);
			int nSrcVolatile = BASE_GetItemAbility(g_pCursor->m_pAttachedItem->m_pItem, 38);

			int nSanc = 0;
			int nGrade = 0;
			int nUnique = 0;

			if (pItem)
			{
				nSanc = BASE_GetItemSanc(pItem->m_pItem);
				nGrade = g_pItemList[pItem->m_pItem->sIndex].nGrade;
				nUnique = g_pItemList[pItem->m_pItem->sIndex].nUnique;
			}

			int nDstVolatile = -1;
			if (pItem)
				nDstVolatile = BASE_GetItemAbility(pItem->m_pItem, 38);

			int nCheckType = CheckType(m_eItemType, m_eGridType);
			int nCheckPos = CheckPos(m_eItemType);

			if ((nSrcVolatile >= 4 && nSrcVolatile <= 6 || nSrcVolatile >= 90 && nSrcVolatile < 95 ||
				nSrcVolatile == 9 || nSrcVolatile == 15 || nSrcVolatile == 16 || nSrcVolatile == 179) &&
				!nDstVolatile && !nCheckType && pItem)
			{
				m_dwEnableColor = 0x3300FF00;
			}
			else if (nSrcVolatile >= 180 && nSrcVolatile <= 183 && !nDstVolatile && !nCheckType && pItem)
			{
				m_dwEnableColor = 0x33FF0000;

				if (nSanc > 9 && (m_eItemType == TMEITEMTYPE::ITEMTYPE_HELM || m_eItemType == TMEITEMTYPE::ITEMTYPE_COAT ||
					m_eItemType == TMEITEMTYPE::ITEMTYPE_PANTS || m_eItemType == TMEITEMTYPE::ITEMTYPE_GLOVES || m_eItemType == TMEITEMTYPE::ITEMTYPE_BOOTS ||
					nUnique == 51) ||
					nGrade >= 5 && nGrade <= 8 && (m_eItemType == TMEITEMTYPE::ITEMTYPE_LEFT || m_eItemType == TMEITEMTYPE::ITEMTYPE_RIGHT &&
						nUnique == 51))
				{
					m_dwEnableColor = 0x3300FF00;
				}
			}
			else if (nSrcVolatile == 186 && !nDstVolatile && !nCheckType && pItem)
			{
				m_dwEnableColor = 0x33FF0000;
				if (m_eItemType == TMEITEMTYPE::ITEMTYPE_HELM || m_eItemType == TMEITEMTYPE::ITEMTYPE_COAT ||
					m_eItemType == TMEITEMTYPE::ITEMTYPE_PANTS || m_eItemType == TMEITEMTYPE::ITEMTYPE_GLOVES ||
					m_eItemType == TMEITEMTYPE::ITEMTYPE_BOOTS || m_eItemType == TMEITEMTYPE::ITEMTYPE_RIGHT &&
					BASE_GetItemAbility(pItem->m_pItem, 21) <= 0)
				{
					m_dwEnableColor = 0x3300FF00;
				}
			}
			else if (nSrcVolatile >= 235 && nSrcVolatile <= 238 && !nDstVolatile && !nCheckType && pItem)
			{
				m_dwEnableColor = 0x33FF0000;
				if (nSanc >= 11 &&
					(m_eItemType == TMEITEMTYPE::ITEMTYPE_COAT || m_eItemType == TMEITEMTYPE::ITEMTYPE_PANTS ||
						m_eItemType == TMEITEMTYPE::ITEMTYPE_GLOVES || m_eItemType == TMEITEMTYPE::ITEMTYPE_BOOTS || nUnique == 51)
					|| nGrade >= 5 && nGrade <= 8 &&
					(m_eItemType == TMEITEMTYPE::ITEMTYPE_LEFT || m_eItemType == TMEITEMTYPE::ITEMTYPE_RIGHT && nUnique == 51))
				{
					m_dwEnableColor = 0x3300FF00;
				}
			}
			else if (nSrcVolatile == 241 && pItem)
			{
				m_dwEnableColor = 0x33FF0000;
				if (nDstVolatile == 16)
					m_dwEnableColor = 0x3300FF00;
			}
			else if (nSrcVolatile >= 239 && nSrcVolatile <= 240 && pItem)
			{
				m_dwEnableColor = 0x33FF0000;
				if (m_eItemType == TMEITEMTYPE::ITEMTYPE_HELM || m_eItemType == TMEITEMTYPE::ITEMTYPE_COAT ||
					m_eItemType == TMEITEMTYPE::ITEMTYPE_PANTS || m_eItemType == TMEITEMTYPE::ITEMTYPE_GLOVES ||
					m_eItemType == TMEITEMTYPE::ITEMTYPE_BOOTS || m_eItemType == TMEITEMTYPE::ITEMTYPE_RIGHT &&
					BASE_GetItemAbility(pItem->m_pItem, 21) <= 0)
				{
					m_dwEnableColor = 0x3300FF00;
				}
			}
			else if (nSrcVolatile == 190 && pItem && !nCheckType)
			{
				m_dwEnableColor = 0x33FF0000;

				unsigned int nItemPos = BASE_GetItemAbility(pItem->m_pItem, 17);
				m_vecPickupedPos.x = pItem->m_nCellIndexX;
				m_vecPickupedPos.y = pItem->m_nCellIndexY;
				m_vecPickupedSize.x = pItem->m_nCellWidth;
				m_vecPickupedSize.y = pItem->m_nCellHeight;

				if (nItemPos == 2 || nItemPos == 4 || nItemPos == 8 || nItemPos == 16 || nItemPos == 32)
				{
					auto pSrcItemId = (g_pCursor->m_pAttachedItem->m_pItem->sIndex - 4016) % 5 + 1;
					int nRefLevel = BASE_GetItemAbility(pItem->m_pItem, 87);
					if (pSrcItemId == nRefLevel && nSanc < 10)
						m_dwEnableColor = 0x330000FF;
				}
			}
			else
			{
				m_dwEnableColor = 0x33FF0000;
			}
		}

		if (nCellX < 0 || nCellX >= m_nColumnGridCount || nCellY < 0 || nCellY >= m_nRowGridCount)
			m_dwEnableColor = 0;
	}
	return 2;

}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
ExtractedFlow SGridControl::DescribeItem3443(SGridControlItem*& pItem, SText*& pParamText, int& extractedResult)
{
	if (pItem->m_pItem->stEffect[0].cEffect == 59)
	{
		unsigned char nEFV1 = (unsigned char)pItem->m_pItem->stEffect[0].cValue;
		unsigned char nEFV2 = (unsigned char)pItem->m_pItem->stEffect[1].cValue;

		int nCapsuleIndex = nEFV2 + (nEFV1 << 8);
		bool bFindCapsule = false;

		for (int nIndex = 0; nIndex < 12; nIndex++)
		{
			if (g_pObjectManager->m_stCapsuleInfo[nIndex].CIndex != nCapsuleIndex)
				continue;

			UpdateCapsuleInfo(nIndex);
			bFindCapsule = true;
			break;
		}

		if (!bFindCapsule)
		{
			MSG_STANDARDPARM dst{};
			dst.Header.Type = MSG_RequestCapsuleInfo_Opcode;
			dst.Header.ID = g_pCurrentScene->m_pMyHuman->m_dwID;
			dst.Parm = nCapsuleIndex;

			SendOneMessage((char*)& dst, sizeof(dst));
		}

		SGridControl::m_pLastMouseOverItem = pItem;
		SGridControl::m_sLastMouseOverIndex = pItem->m_pItem->sIndex;

		if (pParamText)
		{
			char Price[128]{};
			if (AutoSellShowPrice(Price))
				pParamText->SetText(Price, 0);
		}
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	return ExtractedFlow::Next;
}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
ExtractedFlow SGridControl::DescribeItem3444(TMFieldScene*& pFScene, SGridControlItem*& pItem, SText*& pParamText, int& extractedResult)
{
	unsigned char nEFV1 = (unsigned char)pItem->m_pItem->stEffect[0].cValue;
	unsigned char nEFV2 = (unsigned char)pItem->m_pItem->stEffect[1].cValue;

	int nItemId = nEFV2 + (nEFV1 << 8);
	if (nItemId)
	{
		auto pDescNameText = pFScene->m_pDescNameText;

		char Buffer[128]{};
		sprintf(Buffer, "%s", g_pItemList[3444].Name);

		pDescNameText->SetText(Buffer, 0);
		pDescNameText->SetTextColor(0x0FFFFFFAA);

		int nId = 0;
		pFScene->m_pParamText[nId]->SetText(g_pItemList[nItemId].Name, 0);
		pFScene->m_pParamText[nId]->SetTextColor(0x0FFFFBBFF);
		++nId;

		while (nId < NUM_ITEM_DESC_PARAMS)
		{
			pFScene->m_pParamText[nId]->SetText((char*)"", 0);
			pFScene->m_pParamText[nId]->SetTextColor(0x0FFFFBBFF);
			++nId;
		}

		// The native 7.48 tooltip reserves its final row for the item price.
		nId = NUM_ITEM_DESC_PARAMS - 1;
		int nPrice = 0;
		if (nItemId > 0 && nItemId < 6500)
			nPrice = g_pItemList[nItemId].nPrice;

		auto vecPos = pFScene->m_pMyHuman->m_vecPosition;

		char szStrPrice[128]{};
		if (nItemId == 4010 || nItemId == 4011 || nItemId >= 4026 && nItemId <= 4029)
		{
			sprintf(szStrPrice, g_pMessageStringTable[57], nPrice + nPrice / 100);
			sprintf(szStrPrice, "%s (%s:%d%%)", szStrPrice, g_pMessageStringTable[146], 1);
		}
		else
		{
			float fTax = (float)g_pObjectManager->m_nTax / 100.0f;
			float fFinalPrice = (float)nPrice * fTax;

			if (pFScene->m_nIsMP == 2)
			{
				sprintf(szStrPrice, g_pMessageStringTable[487], nPrice);
				sprintf(szStrPrice, "%s", szStrPrice);
			}
			else if (pFScene->m_nIsMP == 1)
			{
				sprintf(szStrPrice, g_pMessageStringTable[385], nPrice);
				sprintf(szStrPrice, "%s", szStrPrice);
			}
			else
			{
				sprintf(szStrPrice, g_pMessageStringTable[57], nPrice + (int)fFinalPrice);
				sprintf(szStrPrice, "%s (%s:%d%%)", szStrPrice, g_pMessageStringTable[146], g_pObjectManager->m_nTax);
			}
		}

		pFScene->m_pParamText[nId]->SetText(szStrPrice, 0);
		pFScene->m_pParamText[nId]->SetTextColor(0xFFFFFFFF);
		if (pParamText)
		{
			char Price[128]{};
			if (AutoSellShowPrice(Price))
				pParamText->SetText(Price, 0);
		}
		{ extractedResult = 1; return ExtractedFlow::Return; }
	}
	return ExtractedFlow::Next;
}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
int SGridControl::DescribeItem4147(int& nLineId, TMFieldScene*& pFScene, SGridControlItem*& pItem, char (&szDesc)[128])
{
	int match = 0;
	int scoreA = 0;
	int scoreB = 0;
	for (int i = 0; i < 3; ++i)
	{
		switch (pItem->m_pItem->stEffect[i].cEffect)
		{
		case 64:
			match = pItem->m_pItem->stEffect[i].cValue;
			break;
		case 65:
			scoreA = pItem->m_pItem->stEffect[i].cValue;
			break;
		case 66:
			scoreB = pItem->m_pItem->stEffect[i].cValue;
			break;
		}
	}

	if (match > 0 && nLineId < NUM_ITEM_DESC_PARAMS)
	{
		sprintf_s(szDesc, "Match: %d", match);
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId++]->SetTextColor(0xFFFFFFAA);
	}
	if (match > 0 && nLineId < NUM_ITEM_DESC_PARAMS)
	{
		sprintf_s(szDesc, "Team A score: %d", scoreA);
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId++]->SetTextColor(0xFFFFFFFF);
	}
	if (match > 0 && nLineId < NUM_ITEM_DESC_PARAMS)
	{
		sprintf_s(szDesc, "Team B score: %d", scoreB);
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId++]->SetTextColor(0xFFFFFFFF);
	}
	return 1;

}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
int SGridControl::DescribeOutsideShop(SGridControlItem*& pItem, SText*& pParamText)
{
	if (m_eGridType == TMEGRIDTYPE::GRID_SELL)
	{
		const int itemIndex = pItem->m_pItem->sIndex;
		int catalogPrice = 0;
		int volatileAbility = 0;
		if (native_sale_quote::IsValidCatalogIndex(itemIndex))
		{
			catalogPrice = g_pItemList[itemIndex].nPrice;
			volatileAbility = native_sale_quote::GetVolatileAbility(itemIndex,
				g_pItemList[itemIndex].stEffect, pItem->m_pItem->stEffect);
		}
		const int nPrice = native_sale_quote::Calculate(itemIndex, catalogPrice, volatileAbility);
		char szText[128]{};
		sprintf(szText, g_pMessageStringTable[58], nPrice);
		pParamText->SetText(szText, 0);
		if (native_sale_quote::HasUnavailablePrice(itemIndex, catalogPrice))
		{
			pParamText->SetText(g_pMessageStringTable[340], 0);
		}
	}
	else if (m_eGridType == TMEGRIDTYPE::GRID_SKILLM)
	{
		int nSkillPoint = g_pSpell[g_pItemList[pItem->m_pItem->sIndex].nIndexTexture].SkillPoint;
		char szText[128]{};

		// Skill-book affordability must compare against the uint32 counter
		// received from WYD-Go, not STRUCT_MOB's signed-short projection.
		if ((unsigned int)nSkillPoint > g_pObjectManager->m_stMobData.CurrentScore.SkillPts)
			pParamText->SetTextColor(0xFFFF0000);

		sprintf(szText, g_pMessageStringTable[59], nSkillPoint);
		pParamText->SetText(szText, 0);
	}



	else if (m_eGridType == TMEGRIDTYPE::GRID_TRADEMY2 || m_eGridType == TMEGRIDTYPE::GRID_TRADEOP)
	{
		char szText[128]{};
		if (AutoSellShowPrice(szText))
			pParamText->SetText(szText, 0);
	}
	else if (pItem->m_pItem->sIndex >= 666 && pItem->m_pItem->sIndex < 672)
	{
		int nSkillPoint = g_pSpell[pItem->m_pItem->sIndex - 570].SkillPoint;
		char szText[128]{};
		sprintf(szText, g_pMessageStringTable[59], nSkillPoint);
		pParamText->SetText(szText, 0);
	}
	else
	{
		pParamText->SetText((char*)"", 0);
	}
	return 1;

}


// Extracted from SGridControl::MouseOver; behavior is unchanged.
void SGridControl::DescribeItemName(SGridControlItem*& pItem, SText*& pDescNameText, int& nSanc)
{
	char szText[128]{};
	unsigned int nItemPos = BASE_GetItemAbility(pItem->m_pItem, 17);

	if (pItem->m_pItem->sIndex == 411 ||
		pItem->m_pItem->sIndex >= 400 && pItem->m_pItem->sIndex <= 409 ||
		pItem->m_pItem->sIndex >= 428 && pItem->m_pItem->sIndex <= 435 ||
		pItem->m_pItem->sIndex >= 680 && pItem->m_pItem->sIndex <= 691)
	{
		sprintf(szText, "%s", g_pItemList[pItem->m_pItem->sIndex].Name);
		pDescNameText->SetText(szText, 0);
		pDescNameText->SetTextColor(0xFFAAAAFF);
	}
	else if (pItem->m_pItem->sIndex == 412 || pItem->m_pItem->sIndex == 413 || pItem->m_pItem->sIndex == 4141 ||
		pItem->m_pItem->sIndex == 419 || pItem->m_pItem->sIndex == 420 ||
		nSanc > 0 && nItemPos == 0)
	{
		sprintf(szText, "%s", g_pItemList[pItem->m_pItem->sIndex].Name);
		pDescNameText->SetText(szText, 0);
		pDescNameText->SetTextColor(0xFFFFFFAA);
	}
	else if (!BASE_CanRefine(pItem->m_pItem) && nItemPos && nItemPos != (int)TMEITEMTYPE::ITEMTYPE_MOUNT)
	{
		sprintf(szText, g_pMessageStringTable[48], g_pItemList[pItem->m_pItem->sIndex].Name);

		int nRefLevel = BASE_GetItemAbility(pItem->m_pItem, 87) + 64;
		if (nRefLevel >= 65)
			sprintf(szText, "%s [%c]", szText, nRefLevel);

		pDescNameText->SetText(szText, 0);
		pDescNameText->SetTextColor(BASE_GetItemColor(pItem->m_pItem));
	}
	else if (nSanc > 0 && nItemPos > 0)
	{
		int nSancSuccess = BASE_GetItemSancSuccess(pItem->m_pItem) * g_pSuccessRate[nSanc + 1];
		int nRefLevel = BASE_GetItemAbility(pItem->m_pItem, 87) + 64;

		if (nRefLevel >= 65)
		{
			if (nSancSuccess <= 0)
				sprintf(szText, "%s +%d [%c]", g_pItemList[pItem->m_pItem->sIndex].Name, nSanc, nRefLevel);
			else
				sprintf(szText, "%s + %d (+%d%%) [%c]", g_pItemList[pItem->m_pItem->sIndex].Name, nSanc, nSancSuccess, nRefLevel);

			if (pItem->m_pItem->sIndex >= 2390 && pItem->m_pItem->sIndex <= 2419)
				sprintf(szText, "%s +%d (%d)", g_pItemList[pItem->m_pItem->sIndex].Name, nSanc, pItem->m_pItem->stEffect[1].cValue);
		}
		else
		{
			if (nSancSuccess <= 0)
				sprintf(szText, "%s +%d", g_pItemList[pItem->m_pItem->sIndex].Name, nSanc);
			else
				sprintf(szText, "%s +%d (+%d%%)", g_pItemList[pItem->m_pItem->sIndex].Name, nSanc, nSancSuccess);

			if (pItem->m_pItem->sIndex >= 2390 && pItem->m_pItem->sIndex <= 2419)
				sprintf(szText, "%s +%d (%d)", g_pItemList[pItem->m_pItem->sIndex].Name, nSanc, pItem->m_pItem->stEffect[1].cValue);
		}

		pDescNameText->SetText(szText, 0);
		pDescNameText->SetTextColor(BASE_GetItemColor(pItem->m_pItem));
	}
	else if ((!pItem->m_pItem->stEffect[0].cEffect || pItem->m_pItem->stEffect[0].cEffect == 59) &&
		(!pItem->m_pItem->stEffect[1].cEffect || pItem->m_pItem->stEffect[1].cEffect == 59) &&
		(!pItem->m_pItem->stEffect[2].cEffect || pItem->m_pItem->stEffect[2].cEffect == 59))
	{
		int nRefLevel = BASE_GetItemAbility(pItem->m_pItem, 87) + 64;
		if (nRefLevel >= 65)
			sprintf(szText, "%s [%c]", g_pItemList[pItem->m_pItem->sIndex].Name, nRefLevel);
		else
			sprintf(szText, "%s", g_pItemList[pItem->m_pItem->sIndex].Name);

		pDescNameText->SetText(szText, 0);
		pDescNameText->SetTextColor(BASE_GetItemColor(pItem->m_pItem));

		if (pItem->m_pItem->sIndex == 753 || pItem->m_pItem->sIndex == 769 || pItem->m_pItem->sIndex == 1726)
			pDescNameText->SetTextColor(0xFFFFFFAA);
	}
	else
	{
		int nRefLevel = BASE_GetItemAbility(pItem->m_pItem, 87) + 64;
		if (nRefLevel >= 65)
			sprintf(szText, "%s [%c]", g_pItemList[pItem->m_pItem->sIndex].Name, nRefLevel);
		else if (pItem->m_pItem->sIndex >= 2360 && pItem->m_pItem->sIndex <= 2389 && (pItem->m_pItem->stEffect[2].cValue >= 10))
			sprintf(szText, g_pMessageStringTable[483], g_pItemList[pItem->m_pItem->sIndex].Name, g_pItemList[pItem->m_pItem->stEffect[2].cValue + 4179].Name);
		else
			sprintf(szText, "%s", g_pItemList[pItem->m_pItem->sIndex].Name);

		pDescNameText->SetText(szText, 0);
		pDescNameText->SetTextColor(BASE_GetItemColor(pItem->m_pItem));
	}

}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
void SGridControl::DescribeSkillItem(SGridControlItem*& pItem, TMFieldScene*& pFScene, unsigned int& dwColor, int& nLineId, char (&szDesc)[128])
{
	dwColor = 0xFFFFFFFF;
	int SkillNumber = GetSkillIndex(pItem->m_pItem->sIndex);

	sprintf(szDesc, g_pMessageStringTable[49], g_pSpell[SkillNumber].Range);
	pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
	pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
	++nLineId;

	auto mob = &g_pObjectManager->m_stMobData;
	if (SkillNumber == 22 || SkillNumber == 31)
	{
		sprintf(szDesc, g_pMessageStringTable[231]);
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
		++nLineId;
	}
	else
	{
		int manaSpent = 0;
		if (SkillNumber < 96)
		{
			int Special = g_pObjectManager->m_stMobData.CurrentScore.Mastery[(SkillNumber - 24 * mob->Class) / 8 + 1];
			manaSpent = BASE_GetManaSpent(SkillNumber, g_pObjectManager->m_stMobData.CurrentScore.SaveMana, Special);
		}

		char szText[128]{};
		sprintf(szText, g_pMessageStringTable[50], g_pSpell[SkillNumber].ManaSpent);
		if (manaSpent)
			sprintf(szDesc, "%s ( %d )", szText, manaSpent);
		else
			sprintf(szDesc, "%s", szText);

		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
		++nLineId;
	}

	auto weather = g_nWeather;
	if (g_pCurrentScene->m_pMyHuman)
	{
		auto vecPos = g_pCurrentScene->m_pMyHuman->m_vecPosition;
		if ((int)vecPos.x >> 7 > 26 && (int)vecPos.x >> 7 < 31 &&
			(int)vecPos.y >> 7 > 20 && (int)vecPos.y >> 7 < 25)
		{
			weather = 2;
		}
	}

	int faceId = g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex;
	int nSkillDamage = BASE_GetSkillDamage(SkillNumber, mob, weather, pFScene->GetWeaponDamage(), faceId);

	char szText[128]{};
	sprintf(szText, g_pMessageStringTable[51], g_pSpell[SkillNumber].InstanceValue);

	if (nSkillDamage)
		sprintf(szDesc, "%s ( %d )", szText, nSkillDamage);
	else
		sprintf(szDesc, "%s", szText);

	pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
	pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
	++nLineId;

	int attribute = 0;
	if (g_pSpell[SkillNumber].InstanceAttribute <= g_pSpell[SkillNumber].TickAttribute)
		attribute = g_pSpell[SkillNumber].TickAttribute;
	else
		attribute = g_pSpell[SkillNumber].InstanceAttribute;

	if (SkillNumber == 92)
		attribute = 5;

	static const char* attributes[6] = {
		g_pMessageStringTable[116],
		g_pMessageStringTable[117],
		g_pMessageStringTable[118],
		g_pMessageStringTable[119],
		g_pMessageStringTable[120],
		g_pMessageStringTable[481]
	};

	sprintf(szDesc, g_pMessageStringTable[52], attributes[attribute]);

	pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
	pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
	++nLineId;

	static const char* requirements[12] = {
				g_pMessageStringTable[107],
				g_pMessageStringTable[108],
				g_pMessageStringTable[109],
				g_pMessageStringTable[110],
				g_pMessageStringTable[111],
				g_pMessageStringTable[112],
				g_pMessageStringTable[113],
				g_pMessageStringTable[114],
				g_pMessageStringTable[115],
				g_pMessageStringTable[133],
				g_pMessageStringTable[134],
				g_pMessageStringTable[135]
	};

	for (int l = 3; l < 6; ++l)
	{
		int reqScore = BASE_GetItemAbility(pItem->m_pItem, dwEFParam[l]);
		if (reqScore > 0)
		{
			if (reqScore <= *((unsigned short*)&g_pObjectManager->m_stMobData.CurrentScore.Dex + l))
				dwColor = 0xFFFFFFFF;
			else
				dwColor = 0xFFFF0000;

			int itemId = pItem->m_pItem->sIndex;
			if (itemId >= 5400)
				itemId = (itemId - 5400) / 4;
			else if (itemId >= 5000)
			{
				itemId = (itemId - 5000) / 8;
				itemId /= 2;
			}

			sprintf(szDesc, g_pMessageStringTable[53], requirements[itemId], reqScore);

			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
			++nLineId;
		}
	}
	for (int l = 0; l < 3; ++l)
	{
		int req = BASE_GetItemAbility(pItem->m_pItem, dwEFParam[l]);
		if (req < 0 || nLineId >= 11)
			continue;

		if (l == 0)
		{
			dwColor = 0xFFFFFFFF;
			if (req == 255)
				continue;

			static const char* classRequirements[4] = {
				g_pMessageStringTable[121],
				g_pMessageStringTable[122],
				g_pMessageStringTable[123],
				g_pMessageStringTable[124]
			};

			auto mob = &g_pObjectManager->m_stMobData;
			sprintf(szDesc, "%s : ", SGridControl::m_szParamString[l]);

			for (int n = 0; n < 4; ++n)
			{
				if ((req & (1 << n)) == (1 << n))
				{
					if (mob->Class != n)
						dwColor = 0xFFFF0000;

					strcat(szDesc, classRequirements[n]);
				}
			}

			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
			++nLineId;
		}
		else if (l == 1 && m_eGridType == TMEGRIDTYPE::GRID_SKILLM)
		{
			if (g_pObjectManager->m_stMobData.CurrentScore.Level >= req)
				dwColor = 0xFFFFFFFF;
			else
				dwColor = 0xFFFF0000;

			sprintf(szDesc, g_pMessageStringTable[54], ++req);
			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
			++nLineId;
		}
		else if (l == 2 && m_eGridType == TMEGRIDTYPE::GRID_SKILLM && req)
		{
			dwColor = 0xFFFFFFFF;
			int itemId = (pItem->m_pItem->sIndex - 5000) / 8;
			sprintf(szDesc, g_pMessageStringTable[55], requirements[itemId], req);

			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
			++nLineId;
		}
	}
	if (IsPassiveSkill(pItem->m_pItem->sIndex) == 1)
	{
		sprintf(szDesc, g_pMessageStringTable[139]);
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(0xFFAAFFAA);
		++nLineId;
	}

}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
void SGridControl::DescribeTimedItem(SGridControlItem*& pItem, TMFieldScene*& pFScene, int& nLineId, char (&szDesc)[128])
{
	auto itemEffect = pItem->m_pItem->stEffect;

	unsigned char date = 0;
	unsigned char hour = 0;
	unsigned char min = 0;
	for (int i = 0; i < 3; ++i)
	{
		switch (itemEffect[i].cEffect)
		{
		case EF_DATE:
			date = (unsigned char)itemEffect[i].cValue;
			break;
		case EF_HOUR:
			hour = (unsigned char)itemEffect[i].cValue;
			break;
		case EF_MIN:
			min = (unsigned char)itemEffect[i].cValue;
			break;
		}
	}

	char formattedDate[128]{};
	char formattedHour[128]{};
	char formattedMin[128]{};

	if (date)
		sprintf(formattedDate, g_pMessageStringTable[291], date);
	else
		sprintf(formattedDate, "");
	if (hour)
		sprintf(formattedHour, g_pMessageStringTable[292], hour);
	else
		sprintf(formattedHour, "");
	if (min)
		sprintf(formattedMin, g_pMessageStringTable[293], min);
	else
		sprintf(formattedMin, "");


	char timeTypeStr[128]{};
	sprintf(timeTypeStr, g_pMessageStringTable[298], 0);
	sprintf(szDesc, "%s %s %s %s", formattedDate, formattedHour, formattedMin ,timeTypeStr);

	pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
	pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFAA);
	++nLineId;

}

// Extracted from SGridControl::MouseOver; behavior is unchanged.
void SGridControl::DescribeGeneralItem(SGridControlItem*& pItem, TMFieldScene*& pFScene, unsigned int& dwColor, unsigned int& dwServerTime, unsigned int& nItemPos, int& nLineId, int& nWeaponType, STRUCT_MOB*& pMobData, char (&szDesc)[128])
{
	if (pItem->m_pItem->sIndex >= 3980 && pItem->m_pItem->sIndex <= 3999 &&
		pItem->m_pItem->sIndex != 3993 && pItem->m_pItem->sIndex != 3994)
	{
		auto itemEffect = pItem->m_pItem->stEffect;

		unsigned char date = 0;
		unsigned char year = 0;
		unsigned char month = 0;
		for (int i = 0; i < 3; ++i)
		{
			switch (itemEffect[i].cEffect)
			{
			case EF_DATE:
				date = (unsigned char)itemEffect[i].cValue;
				break;
			case EF_YEAR:
				year = (unsigned char)itemEffect[i].cValue;
				break;
			case EF_MONTH:
				month = (unsigned char)itemEffect[i].cValue;
				break;
			}
		}

		char formattedDate[128]{};
		char formattedYear[128]{};
		char formattedMonth[128]{};

		if (date)
			sprintf(formattedDate, g_pMessageStringTable[291], date);
		else
			sprintf(formattedDate, "");
		if (year)
			sprintf(formattedYear, g_pMessageStringTable[297], year + 2000);
		else
			sprintf(formattedYear, "");
		if (month)
			sprintf(formattedMonth, g_pMessageStringTable[296], month);
		else
			sprintf(formattedMonth, "");

		char timeTypeStr[128]{};
		sprintf(timeTypeStr, g_pMessageStringTable[298], 0);
		sprintf(szDesc, "%s %s %s %s 0%s", g_pMessageStringTable[299], formattedYear, formattedMonth, formattedDate, timeTypeStr);

		pFScene->m_dwNightmareTime = dwServerTime;
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFAA);
		++nLineId;
	}
	else if (pItem->m_pItem->sIndex >= 4150 && pItem->m_pItem->sIndex <= 4189
		|| pItem->m_pItem->sIndex >= 4300 && pItem->m_pItem->sIndex <= 4420)
	{
		auto itemEffect = pItem->m_pItem->stEffect;

		unsigned char date = 0;
		unsigned char year = 0;
		unsigned char month = 0;
		for (int i = 0; i < 3; ++i)
		{
			switch (itemEffect[i].cEffect)
			{
			case EF_DATE:
				date = (unsigned char)itemEffect[i].cValue;
				break;
			case EF_YEAR:
				year = (unsigned char)itemEffect[i].cValue;
				break;
			case EF_MONTH:
				month = (unsigned char)itemEffect[i].cValue;
				break;
			}
		}

		char formattedDate[128]{};
		char formattedYear[128]{};
		char formattedMonth[128]{};

		if (date)
			sprintf(formattedDate, g_pMessageStringTable[291], date);
		else
			sprintf(formattedDate, "");
		if (year)
			sprintf(formattedYear, g_pMessageStringTable[297], year + 2000);
		else
			sprintf(formattedYear, "");
		if (month)
			sprintf(formattedMonth, g_pMessageStringTable[296], month);
		else
			sprintf(formattedMonth, "");

		char timeTypeStr[128]{};
		sprintf(timeTypeStr, g_pMessageStringTable[298], 0);
		sprintf(szDesc, "%s %s %s %s 0%s", g_pMessageStringTable[299], formattedYear, formattedMonth, formattedDate, timeTypeStr);

		pFScene->m_dwNightmareTime = dwServerTime;
		pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
		pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFAA);
		++nLineId;
	}

	STRUCT_REQ req;
	BASE_CanEquip_RecvRes(
		&req,
		pItem->m_pItem,
		&pMobData->CurrentScore,
		-1,
		pMobData->Equip[0].sIndex,
		pMobData->Equip,
		g_pObjectManager->m_stSelCharData.Equip[g_pObjectManager->m_cCharacterSlot][0].sIndex);

	for (int l = 0; l < 49; ++l)
	{
		int add = BASE_GetStaticItemAbility(pItem->m_pItem, dwEFParam[l]);
		if (dwEFParam[l] == 80 && add <= 0 && pItem->m_pItem->sIndex >= 2330 && pItem->m_pItem->sIndex <= 2389)
		{
			sprintf(szDesc, "%s", g_pMessageStringTable[168]);
			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
			++nLineId;
		}
		else if (add && nLineId < NUM_ITEM_DESC_PARAMS)
		{
			if (l == 0)
			{
				int cktrans = 0;
				if (BASE_GetItemAbility(pItem->m_pItem, 112) == 1 && g_pItemList[pItem->m_pItem->sIndex].nUnique > 40)
					cktrans = 1;

				if (add != 255 && !cktrans && (pItem->m_pItem->sIndex < 4190 || pItem->m_pItem->sIndex > 4200))
				{
					static const char* reqs[4] = {
						 g_pMessageStringTable[121],
						 g_pMessageStringTable[122],
						 g_pMessageStringTable[123],
						 g_pMessageStringTable[124]
					};

					sprintf(szDesc, "%s : ", SGridControl::m_szParamString[l]);

					for (int mm = 0; mm < 4; ++mm)
					{
						if ((add & (1 << mm)) == 1 << mm)
							strcat(szDesc, reqs[mm]);
					}

					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(req.Class ? 0xFFFFFFFF : 0xFFFF0000);
					++nLineId;
				}
			}
			else if (l == 1)
			{
				if (nItemPos != 192 || l < 1 || l > 5 || nWeaponType % 10 <= 1)
				{
					sprintf(szDesc, "%s : %d", SGridControl::m_szParamString[l], add);
				}
				else
				{
					int someWeaponAdd = 100;
					if (!(nWeaponType / 10))
					{
						someWeaponAdd = 130;
					}
					else if ((nWeaponType / 10) == 6)
					{
						someWeaponAdd = 150;
					}

					char szText[128]{};
					sprintf(szDesc, "%s : %d ", SGridControl::m_szParamString[l], add);
					sprintf(szText, g_pMessageStringTable[56], someWeaponAdd * add / 100 + 1);
					strcat(szDesc, szText);
				}

				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(req.Class ? 0xFFFFFFFF : 0xFFFF0000);
				++nLineId;
			}
			else if (dwEFParam[l] == 2 && BASE_GetItemAbility(pItem->m_pItem, 17) != 32)
			{
				sprintf(szDesc, "%s : %d", SGridControl::m_szParamString[l], add);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
				++nLineId;
			}
			else if (dwEFParam[l] == 3)
			{
				sprintf(szDesc, "%s : %d", SGridControl::m_szParamString[l], add);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
				++nLineId;
			}
			else if (dwEFParam[l] == 60)
			{
				sprintf(szDesc, "%s : %d%%", SGridControl::m_szParamString[l], add);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
				++nLineId;
			}
			else if ((dwEFParam[l] != 2 || BASE_GetItemAbility(pItem->m_pItem, 17) != 32)
				&& dwEFParam[l] != 42
				&& dwEFParam[l] != 53
				&& dwEFParam[l] != 67
				&& dwEFParam[l] != 68)
			{
				if (dwEFParam[l] == 26 || dwEFParam[l] == 45 || dwEFParam[l] == 46)
				{
					sprintf(szDesc, "%s : %d%%", SGridControl::m_szParamString[l], add);
					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
					++nLineId;
				}
				else if (dwEFParam[l] >= 64 && dwEFParam[l] <= 66)
				{
					sprintf(szDesc, "%s", SGridControl::m_szParamString[l]);
					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFF00);
					++nLineId;
				}
				else if (dwEFParam[l] == 81)
				{
					if (pItem->m_pItem->sIndex >= 2360 && pItem->m_pItem->sIndex < 2390
						|| pItem->m_pItem->sIndex >= 2960 && pItem->m_pItem->sIndex < 3000)
					{
						sprintf(szDesc, "%s : %u", g_pMessageStringTable[167], add);
					}
					else
					{
						int growth = 100;
						if (pItem->m_pItem->sIndex == 2330)
							growth = 25;
						if (pItem->m_pItem->sIndex == 2331)
							growth = 35;
						if (pItem->m_pItem->sIndex == 2332)
							growth = 45;
						if (pItem->m_pItem->sIndex == 2333)
							growth = 55;
						if (pItem->m_pItem->sIndex == 2334)
							growth = 65;
						if (pItem->m_pItem->sIndex == 2335)
							growth = 75;

						sprintf(szDesc, "%s : %u/%d", SGridControl::m_szParamString[l], add, growth);
					}

					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
					++nLineId;
				}
				else if (dwEFParam[l] == 83)
				{
					int nMountSanc = BASE_GetItemAbility(pItem->m_pItem, 81);
					int growth = 100;
					if (pItem->m_pItem->sIndex == 2330)
						growth = 25;
					if (pItem->m_pItem->sIndex == 2331)
						growth = 35;
					if (pItem->m_pItem->sIndex == 2332)
						growth = 45;
					if (pItem->m_pItem->sIndex == 2333)
						growth = 55;
					if (pItem->m_pItem->sIndex == 2334)
						growth = 65;
					if (pItem->m_pItem->sIndex == 2335)
						growth = 75;

					sprintf(szDesc, "%s : %d/%d", SGridControl::m_szParamString[l], add, nMountSanc + growth);
					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
					++nLineId;
				}
				else if (dwEFParam[l] == 80)
				{
					if (add >= 0)
						sprintf(szDesc, "%s : %d", SGridControl::m_szParamString[l], add);
					else
						sprintf(szDesc, "%s", g_pMessageStringTable[168]);
					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
					++nLineId;
				}
				else if (dwEFParam[l] == 40)
				{
					sprintf(szDesc, "%s : %d.%d%%", SGridControl::m_szParamString[l], add / 10, add % 10);
					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(dwColor);
					++nLineId;
				}
				else if (dwEFParam[l] == 84 && pItem->m_pItem->sIndex >= 2300 && pItem->m_pItem->sIndex < 2330 && add <= 0)
				{
					sprintf(szDesc, "%s", g_pMessageStringTable[170]);
					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
					pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
					++nLineId;
				}
				else if (add)
				{
					if (nItemPos != 192 || l < 1 || l > 5 || nWeaponType % 10 <= 1)
					{
						sprintf(szDesc, "%s : %d", SGridControl::m_szParamString[l], add);
					}
					else
					{
						int someWeaponAdd = 100;
						if (!(nWeaponType / 10))
						{
							someWeaponAdd = 130;
						}
						else if ((nWeaponType / 10) == 6)
						{
							someWeaponAdd = 150;
						}

						char szText[128]{};
						sprintf(szDesc, "%s : %d ", SGridControl::m_szParamString[l], add);
						sprintf(szText, g_pMessageStringTable[56], someWeaponAdd * add / 100 + 1);
						strcat(szDesc, szText);
					}

					pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);

					switch (l)
					{
					case 2:
						pFScene->m_pParamText[nLineId]->SetTextColor(req.Str ? 0xFFFFFFFF : 0xFFFF0000);
						break;
					case 3:
						pFScene->m_pParamText[nLineId]->SetTextColor(req.Int ? 0xFFFFFFFF : 0xFFFF0000);
						break;
					case 4:
						pFScene->m_pParamText[nLineId]->SetTextColor(req.Dex ? 0xFFFFFFFF : 0xFFFF0000);
						break;
					case 5:
						pFScene->m_pParamText[nLineId]->SetTextColor(req.Con ? 0xFFFFFFFF : 0xFFFF0000);
						break;
					default:
						pFScene->m_pParamText[nLineId]->SetTextColor(0xFFFFFFFF);
						break;
					}

					++nLineId;
				}
			}
		}
	}

	for (int l = 0; l < 49; ++l)
	{
		int nEf = dwEFParam[l];
		int nValue = 0;
		int nValueAbility = 0;

		if (nEf == 45)
			nEf = 69;
		if (nEf == 46)
			nEf = 70;
		unsigned int nPos = BASE_GetItemAbility(pItem->m_pItem, 17);

		if (nEf == 42 || nEf == 53 || nPos == 32 && nEf == 2)
		{
			nValueAbility = BASE_GetItemAbility(pItem->m_pItem, nEf);
			nValue = BASE_GetItemAbilityNosanc(pItem->m_pItem, nEf);
		}
		else
		{
			nValueAbility = BASE_GetBonusItemAbility(pItem->m_pItem, nEf);
			nValue = BASE_GetBonusItemAbilityNosanc(pItem->m_pItem, nEf);
		}
		if (nLineId < NUM_ITEM_DESC_PARAMS && nValueAbility)
		{
			if (dwEFParam[l] == 42)
			{
				if (nValue == nValueAbility)
					sprintf(szDesc, "%s : %d.%d%%", SGridControl::m_szParamString[l], nValueAbility / 10, nValueAbility % 10);
				else
					sprintf(szDesc, "%s : %d.%d%% (%d.%d%%)", SGridControl::m_szParamString[l],
						nValue / 10,
						nValue % 10,
						nValueAbility / 10,
						nValueAbility % 10);
			}
			else if (dwEFParam[l] == 26 || dwEFParam[l] == 60 || dwEFParam[l] == 45 || dwEFParam[l] == 46 || dwEFParam[l] == 68)
			{
				if (nValue == nValueAbility)
					sprintf(szDesc, "%s : %d%%", SGridControl::m_szParamString[l], nValueAbility);
				else
					sprintf(szDesc, "%s : %d%% (%d%%)", SGridControl::m_szParamString[l], nValue, nValueAbility);
			}
			else if ((dwEFParam[l] != 73 || nPos != 32) && (dwEFParam[l] != 67 || nPos == 64 || nPos == 192) && dwEFParam[l] != 63)
			{
				if (nValue == nValueAbility)
					sprintf(szDesc, "%s : %d", SGridControl::m_szParamString[l], nValueAbility);
				else
					sprintf(szDesc, "%s : %d (%d)", SGridControl::m_szParamString[l], nValue, nValueAbility);
			}
			else
				continue;

			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(BASE_GetOptionColor(nPos, dwEFParam[l], nValue));
			++nLineId;
		}
	}

	// Optional lines stop at the final FieldScene2 row; the 7.59 tooltip's
	// extra controls do not exist in the 7.48 resource.
	if (nLineId < NUM_ITEM_DESC_PARAMS && BASE_GetItemSanc(pItem->m_pItem) >= 9)
	{
		unsigned int nPos = BASE_GetItemAbility(pItem->m_pItem, 17);
		if (nPos == 4 || nPos == 8 || nPos == 128)
		{
			sprintf(szDesc, "%s : 25", g_pMessageStringTable[80]);
			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
			++nLineId;
		}
		else if (nPos == 16)
		{
			sprintf(szDesc, "%s : 1", g_pMessageStringTable[151]);
			pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
			pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
			++nLineId;
		}
		else if (nPos == 64 || nPos == 192)
		{
			int nUnique = g_pItemList[pItem->m_pItem->sIndex].nUnique;
			if (nUnique == 47 || nUnique == 44)
			{
				sprintf(szDesc, "%s : 8%%", g_pMessageStringTable[104]);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
				++nLineId;
			}
			else
			{
				sprintf(szDesc, "%s : 40", g_pMessageStringTable[79]);
				pFScene->m_pParamText[nLineId]->SetText(szDesc, 0);
				pFScene->m_pParamText[nLineId]->SetTextColor(0xFF88AAFF);
				++nLineId;
			}
		}
	}

}



void SGridControl::UpdateCapsuleInfo(int nIndex)
{
	if (SGridControl::m_bNeedUpdate && nIndex >= 0 && nIndex <= 11)
	{
		auto pScene = static_cast<TMFieldScene*>(g_pCurrentScene);
		auto pNameText = pScene->m_pDescNameText;

		int nClass = g_pObjectManager->m_stCapsuleInfo[nIndex].Class;
		int nVisualClass = -1;
		if (nClass % 10 > 5)
		{
			nClass = nClass % 10 - 6;
			nVisualClass = g_pObjectManager->m_stCapsuleInfo[nIndex].Class / 10;
		}
		else
			nClass = nClass / 10;

		char szStr[256]{};
		sprintf(szStr, "%s", g_pItemList[3443].Name);
		pNameText->SetText(szStr, 0);
		pNameText->SetTextColor(0xFFFFFFAA);

		if (nVisualClass >= 0)
			sprintf(szStr, "%s : %s[%s]", g_pMessageStringTable[73], g_pMessageStringTable[nClass + 121],
				g_pMessageStringTable[nVisualClass + 121]);
		else
			sprintf(szStr, "%s : %s", g_pMessageStringTable[73], g_pMessageStringTable[nClass + 121]);

		pScene->m_pParamText[0]->SetText(szStr, 0);
		pScene->m_pParamText[0]->SetTextColor(0xFFAAFFFF);

		sprintf(szStr, "%s : %d", g_pMessageStringTable[167], g_pObjectManager->m_stCapsuleInfo[nIndex].Level + 1);
		pScene->m_pParamText[1]->SetText(szStr, 0);
		pScene->m_pParamText[1]->SetTextColor(0xFFFFFFFF);

		sprintf(szStr, "%s : %d", g_pMessageStringTable[100], g_pObjectManager->m_stCapsuleInfo[nIndex].sStr);
		pScene->m_pParamText[2]->SetText(szStr, 0);
		pScene->m_pParamText[2]->SetTextColor(0xFFFFFFFF);

		sprintf(szStr, "%s : %d", g_pMessageStringTable[101], g_pObjectManager->m_stCapsuleInfo[nIndex].sInt);
		pScene->m_pParamText[3]->SetText(szStr, 0);
		pScene->m_pParamText[3]->SetTextColor(0xFFFFFFFF);

		sprintf(szStr, "%s : %d", g_pMessageStringTable[102], g_pObjectManager->m_stCapsuleInfo[nIndex].sDex);
		pScene->m_pParamText[4]->SetText(szStr, 0);
		pScene->m_pParamText[4]->SetTextColor(0xFFFFFFFF);

		sprintf(szStr, "%s : %d", g_pMessageStringTable[103], g_pObjectManager->m_stCapsuleInfo[nIndex].sCon);
		pScene->m_pParamText[5]->SetText(szStr, 0);
		pScene->m_pParamText[5]->SetTextColor(0xFFFFFFFF);


		int nSkill0 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[0];
		int nSkill1 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[1];
		int nSkill2 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[2];

		memset(szStr, 0, 4u);
		if (nSkill0 > 0 && nSkill0 < 110)
			strcat(szStr, g_pItemList[nSkill0 + 5000].Name);

		strcat(szStr, " ");
		if (nSkill1 > 0 && nSkill1 < 110)
			strcat(szStr, g_pItemList[nSkill1 + 5000].Name);

		strcat(szStr, " ");
		if (nSkill2 > 0 && nSkill2 < 110)
			strcat(szStr, g_pItemList[nSkill2 + 5000].Name);

		pScene->m_pParamText[6]->SetText(szStr, 0);
		pScene->m_pParamText[6]->SetTextColor(0xFFFFBBFF);

		nSkill0 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[3];
		nSkill1 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[4];
		nSkill2 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[5];

		memset(szStr, 0, 4u);
		if (nSkill0 > 0 && nSkill0 < 110)
			strcat(szStr, g_pItemList[nSkill0 + 5000].Name);

		strcat(szStr, " ");
		if (nSkill1 > 0 && nSkill1 < 110)
			strcat(szStr, g_pItemList[nSkill1 + 5000].Name);

		strcat(szStr, " ");
		if (nSkill2 > 0 && nSkill2 < 110)
			strcat(szStr, g_pItemList[nSkill2 + 5000].Name);

		pScene->m_pParamText[7]->SetText(szStr, 0);
		pScene->m_pParamText[7]->SetTextColor(0xFFFFBBFF);

		nSkill0 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[6];
		nSkill1 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[7];
		nSkill2 = g_pObjectManager->m_stCapsuleInfo[nIndex].skill[8];

		memset(szStr, 0, 4u);
		if (nSkill0 > 0 && nSkill0 < 110)
			strcat(szStr, g_pItemList[nSkill0 + 5000].Name);

		strcat(szStr, " ");
		if (nSkill1 > 0 && nSkill1 < 110)
			strcat(szStr, g_pItemList[nSkill1 + 5000].Name);

		strcat(szStr, " ");
		if (nSkill2 > 0 && nSkill2 < 110)
			strcat(szStr, g_pItemList[nSkill2 + 5000].Name);

		pScene->m_pParamText[8]->SetText(szStr, 0);
		pScene->m_pParamText[8]->SetTextColor(0xFFFFBBFF);

		int nQuest = 0;
		if (g_pObjectManager->m_stCapsuleInfo[nIndex].Quest & 1)
			nQuest = 1;
		if (g_pObjectManager->m_stCapsuleInfo[nIndex].Quest & 0x11)
			nQuest = 2;
		if (g_pObjectManager->m_stCapsuleInfo[nIndex].Quest & 0x111)
			nQuest = 3;
		if (g_pObjectManager->m_stCapsuleInfo[nIndex].Quest & 0x1111)
			nQuest = 4;

		sprintf(szStr, "%s : %d", g_pMessageStringTable[358], nQuest);
		pScene->m_pParamText[9]->SetText(szStr, 0);
		pScene->m_pParamText[9]->SetTextColor(0xFFFFFFFF);
	}
}
