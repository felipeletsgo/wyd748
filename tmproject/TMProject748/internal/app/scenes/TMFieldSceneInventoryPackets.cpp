#include "pch.h"
#include "TMFieldScene.h"
#include "TMEffectSkinMesh.h"
#include "TMGate.h"
#include "SGrid.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMItem.h"
#include "TMObjectContainer.h"
#include "TMUtil.h"
#include "WYD748Assets.h"
#include "FieldSceneInventorySupport.h"
#include "../../application/FieldInteractionPolicy.h"

	bool WYD748_IsUnsupportedCompatEquipSlot(bool compat, int slot)
	{
		// Keep the fixed Equip wire array intact; these positions simply have no
		// 7.48 visual/equip target in the active emulator UI.
		return compat && (slot == 9 || slot == 16 || slot == 17);
	}

int TMFieldScene::OnPacketUpdateCargoCoin(MSG_STANDARDPARM* pStd)
{
	g_pObjectManager->m_nCargoCoin = pStd->Parm;
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketCNFDropItem(MSG_CNFDropItem* pMsg)
{
	if (!pMsg || !g_pObjectManager)
		return 0;
	if (pMsg->SourType == 0 &&
		(pMsg->SourPos < 0 || pMsg->SourPos >= MAX_EQUIPITEM ||
			WYD748_IsUnsupportedCompatEquipSlot(m_bCompatFieldScene, pMsg->SourPos)))
		return 0;
	if (pMsg->SourType == 1 && !IsDropCarrySlot(pMsg->SourPos))
		return 0;
	if (pMsg->SourType == 2 && !IsDropCargoSlot(pMsg->SourPos))
		return 0;
	if (pMsg->SourType < 0 || pMsg->SourType > 2)
		return 0;

	SGridControlItem* pGridItem = nullptr;
	if (pMsg->SourType == 0)
	{
		SGridControl* pGridList[MAX_EQUIPITEM]{};
		pGridList[0] = nullptr;
		pGridList[1] = m_pGridHelm;
		pGridList[2] = m_pGridCoat;
		pGridList[3] = m_pGridPants;
		pGridList[4] = m_pGridGloves;
		pGridList[5] = m_pGridBoots;
		pGridList[6] = m_pGridLeft;
		pGridList[7] = m_pGridRight;
		pGridList[8] = m_pGridRing;
		pGridList[9] = m_pGridNecklace;
		pGridList[10] = m_pGridOrb;
		pGridList[11] = m_pGridCabuncle;
		pGridList[12] = m_pGridGuild;
		pGridList[13] = m_pGridEvent;
		pGridList[14] = m_pGridDRing;
		pGridList[15] = m_pGridMantua;
		pGridList[16] = m_pGridNewSlot1;
		pGridList[17] = m_pGridNewSlot2;
		if (pGridList[pMsg->SourPos])
			pGridItem = pGridList[pMsg->SourPos]->PickupItem(0, 0);

		memset(&g_pObjectManager->m_stMobData.Equip[pMsg->SourPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pMsg->SourType == 1)
	{
		// Resolve Carry through the single 7.48 9x7 mapping so a confirmed drop
		// removes the exact cell that originated the request.
		int cellX = 0;
		int cellY = 0;
		GetCarryCellForSlot(pMsg->SourPos, cellX, cellY);
		if (auto pCarryGrid = GetCarryGridForSlot(pMsg->SourPos))
			pGridItem = pCarryGrid->PickupAtItem(cellX, cellY);
		memset(&g_pObjectManager->m_stMobData.Carry[pMsg->SourPos], 0, sizeof(STRUCT_ITEM));
	}
	else if (pMsg->SourType == 2)
	{
		// Cargo uses a single nine-column grid in the native 7.48 executable.
		int cellX = 0;
		int cellY = 0;
		GetCargoCellForSlot(pMsg->SourPos, cellX, cellY);
		if (auto pCargoGrid = GetCargoGridForSlot(pMsg->SourPos))
			pGridItem = pCargoGrid->PickupAtItem(cellX, cellY);
		memset(&g_pObjectManager->m_stItemCargo[pMsg->SourPos], 0, sizeof(STRUCT_ITEM));
	}

	if (g_pCursor)
		g_pCursor->DetachItem();
	// Grid removal transfers ownership here, but its shared interaction aliases
	// do not own the item and must not survive the confirmed destruction.
	if (pGridItem)
	{
		if (SGridControl::m_pLastMouseOverItem == pGridItem)
		{
			SGridControl::m_pLastMouseOverItem = nullptr;
			SGridControl::m_sLastMouseOverIndex = -1;
		}
		if (SGridControl::m_pLastAttachedItem == pGridItem)
			SGridControl::m_pLastAttachedItem = nullptr;
		if (SGridControl::m_pSellItem == pGridItem)
			SGridControl::m_pSellItem = nullptr;
	}
	SAFE_DELETE(pGridItem);

	// Keep the authoritative slot update and ownership cleanup even if the
	// local renderer is not available during scene initialization or teardown.
	if (!m_pMyHuman)
		return 1;

	m_pMyHuman->m_sFamiliar = g_pObjectManager->m_stMobData.Equip[13].sIndex;
	GetSoundAndPlay(45, 0, 0);

	UpdateScoreUI(0);
	UpdateMyHuman();
	return 1;
}

int TMFieldScene::OnPacketCNFGetItem(MSG_CNFGetItem* pMsg)
{
	if (!pMsg || !g_pObjectManager)
		return 0;
	auto pGrid = m_pGridInv;
	auto pStructItem = new STRUCT_ITEM;
	if (!pStructItem)
		return 0;

	memcpy(pStructItem, &pMsg->Item, sizeof(pMsg->Item));
	if (BASE_GetItemAbility((STRUCT_ITEM*)pStructItem, 38) == 2)
	{
		int coin = (unsigned char)BASE_GetItemAbility(pStructItem, 36) << 8;
		int tempb = BASE_GetItemAbility(pStructItem, 37);

		g_pObjectManager->m_stMobData.Coin += coin + tempb;

		char szMoney[64]{};
		sprintf(szMoney, "%10d", g_pObjectManager->m_stMobData.Coin);
		if (m_pMoney1)
		{
			m_pMoney1->m_cComma = 2;
			m_pMoney1->SetText(szMoney, 0);
		}
		if (m_pMoney2)
		{
			m_pMoney2->m_cComma = 2;
			m_pMoney2->SetText(szMoney, 0);
		}

		sprintf(szMoney, "%10d", g_pObjectManager->m_stMobData.Coin);
		if (m_pMoney3)
		{
			m_pMoney3->m_cComma = 2;
			m_pMoney3->SetText(szMoney, 0);
		}
		delete pStructItem;
	}
	else
	{
		if (!IsPickupCarrySlot(pMsg->DestPos))
		{
			delete pStructItem;
			return 0;
		}
		// Insert pickup results through the native 9x7 Carry transform in 7.48;
		// using the 7.59 5-column transform made valid items invisible.
		int cellX = 0;
		int cellY = 0;
		GetCarryCellForSlot(pMsg->DestPos, cellX, cellY);
		pGrid = GetCarryGridForSlot(pMsg->DestPos);
		memcpy(&g_pObjectManager->m_stMobData.Carry[pMsg->DestPos], pStructItem, sizeof(STRUCT_ITEM));
		if (pGrid)
		{
			auto pControlItem = new SGridControlItem(pGrid, pStructItem, 0.0f, 0.0f);
			if (pControlItem)
			{
				if (!pGrid->AddItem(pControlItem, cellX, cellY))
					SAFE_DELETE(pControlItem);
			}
			else
				SAFE_DELETE(pStructItem);
		}
		else
			SAFE_DELETE(pStructItem);
	}

	GetSoundAndPlay(45, 0, 0);
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketUpdateItem(MSG_UpdateItem* pMsg)
{
	if (!pMsg || !g_pObjectManager)
		return 0;
	auto pItem = dynamic_cast<TMGate*>(g_pObjectManager->GetItemByID(pMsg->ItemID));
	if (pItem && BASE_GetItemAbility(&pItem->m_stItem, 34) > 0)
	{
		STRUCT_ITEM stItem{};
		stItem.sIndex = pItem->m_stItem.sIndex;

		int nMaskIndex = BASE_GetItemAbility(&stItem, 34);
		int nState = static_cast<int>(pItem->m_eState);
		if (nState > static_cast<int>(EGATE_STATE::EGATE_LOCKED))
			nState -= static_cast<int>(EGATE_STATE::EGATE_LOCKED);

		pItem->m_sAuth = 1;
		BASE_UpdateItem2(nMaskIndex, nState, pMsg->State, (int)pItem->m_vecPosition.x, (int)pItem->m_vecPosition.y,	(char*)m_HeightMapData,
			(int)(pItem->m_fAngle / D3DXToRadian(90)), pMsg->Height);

		pItem->SetState((EGATE_STATE)(pMsg->State + static_cast<int>(EGATE_STATE::EGATE_LOCKED)));
	}

	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketRemoveItem(MSG_STANDARDPARM* pStd)
{
	if (!pStd || !g_pObjectManager)
		return 0;
	m_pMouseOverItem = nullptr;
	g_pObjectManager->DeleteObject(pStd->Parm);
	return 1;
}

int TMFieldScene::OnPacketSwapItem(MSG_STANDARD* pStd)
{
	MSG_SwapItem* pSwapItem = reinterpret_cast<MSG_SwapItem*>(pStd);
	if (!pSwapItem ||
		!g_pObjectManager || !m_pMyHuman || !g_pCursor ||
		!IsSwapPlacePosition(static_cast<unsigned char>(pSwapItem->SourType),
			static_cast<unsigned char>(pSwapItem->SourPos)) ||
		!IsSwapPlacePosition(static_cast<unsigned char>(pSwapItem->DestType),
			static_cast<unsigned char>(pSwapItem->DestPos)))
		return 1;

	// The server confirms positions, not item payloads. Resolve the cached
	// slots independently of the optional visual controls.
	auto modelItem = [](unsigned char type, unsigned char position) -> STRUCT_ITEM*
	{
		switch (type)
		{
		case kSwapPlaceEquip: return &g_pObjectManager->m_stMobData.Equip[position];
		case kSwapPlaceCarry: return &g_pObjectManager->m_stMobData.Carry[position];
		case kSwapPlaceCargo: return &g_pObjectManager->m_stItemCargo[position];
		default: return nullptr;
		}
	};
	STRUCT_ITEM* sourceModel = modelItem(static_cast<unsigned char>(pSwapItem->SourType),
		static_cast<unsigned char>(pSwapItem->SourPos));
	STRUCT_ITEM* destinationModel = modelItem(static_cast<unsigned char>(pSwapItem->DestType),
		static_cast<unsigned char>(pSwapItem->DestPos));
	if (!sourceModel || !destinationModel)
		return 1;

	SGridControl* pSrcGrid = nullptr;
	SGridControl* pDestGrid = nullptr;
	SGridControlItem* pSrcItem = nullptr;
	SGridControlItem* pDestItem = nullptr;

	SGridControl* pGridSrc[MAX_EQUIPITEM]{};
	// The packet confirms logical state even when a visual does not fit the
	// current grid. On failure, ownership remains local until release.
	auto releaseRejectedVisual = [](SGridControlItem*& item)
	{
		if (SGridControl::m_pLastMouseOverItem == item)
		{
			SGridControl::m_pLastMouseOverItem = nullptr;
			SGridControl::m_sLastMouseOverIndex = -1;
		}
		if (SGridControl::m_pLastAttachedItem == item)
			SGridControl::m_pLastAttachedItem = nullptr;
		if (SGridControl::m_pSellItem == item)
			SGridControl::m_pSellItem = nullptr;
		if (g_pCursor && g_pCursor->m_pAttachedItem == item)
			g_pCursor->m_pAttachedItem = nullptr;
		SAFE_DELETE(item);
	};

	if (!pSwapItem->SourType)
	{
		pGridSrc[0] = m_pGridInv;
		pGridSrc[1] = m_pGridHelm;
		pGridSrc[2] = m_pGridCoat;
		pGridSrc[3] = m_pGridPants;
		pGridSrc[4] = m_pGridGloves;
		pGridSrc[5] = m_pGridBoots;
		pGridSrc[6] = m_pGridLeft;
		pGridSrc[7] = m_pGridRight;
		pGridSrc[8] = m_pGridRing;
		pGridSrc[9] = m_pGridNecklace;
		pGridSrc[10] = m_pGridOrb;
		pGridSrc[11] = m_pGridCabuncle;
		pGridSrc[12] = m_pGridGuild;
		pGridSrc[13] = m_pGridEvent;
		pGridSrc[14] = m_pGridDRing;
		pGridSrc[15] = m_pGridMantua;
		pGridSrc[16] = m_pGridNewSlot1;
		pGridSrc[17] = m_pGridNewSlot2;
		pSrcGrid = pGridSrc[pSwapItem->SourPos];
		if (pSrcGrid)
			pSrcItem = pSrcGrid->PickupItem(0, 0);
	}
	else if (pSwapItem->SourType == 1)
	{
		// Carry lookup must follow the active resource ABI: one 9x7 grid in 7.48,
		// paged 5x3 grids only in the newer TMProject UI.
		int cellX = 0;
		int cellY = 0;
		GetCarryCellForSlot(pSwapItem->SourPos, cellX, cellY);
		pSrcGrid = GetCarryGridForSlot(pSwapItem->SourPos);
		if (pSrcGrid)
			pSrcItem = pSrcGrid->PickupAtItem(cellX, cellY);
	}
	else if (pSwapItem->SourType == 2)
	{
		// Cargo follows the same centralized ABI selection as Carry.
		int cellX = 0;
		int cellY = 0;
		GetCargoCellForSlot(pSwapItem->SourPos, cellX, cellY);
		pSrcGrid = GetCargoGridForSlot(pSwapItem->SourPos);
		if (pSrcGrid)
			pSrcItem = pSrcGrid->PickupAtItem(cellX, cellY);
	}

	SGridControl* pGridDest[MAX_EQUIPITEM]{};
	if (!pSwapItem->DestType)
	{
		pGridDest[0] = m_pGridInv;
		pGridDest[1] = m_pGridHelm;
		pGridDest[2] = m_pGridCoat;
		pGridDest[3] = m_pGridPants;
		pGridDest[4] = m_pGridGloves;
		pGridDest[5] = m_pGridBoots;
		pGridDest[6] = m_pGridLeft;
		pGridDest[7] = m_pGridRight;
		pGridDest[8] = m_pGridRing;
		pGridDest[9] = m_pGridNecklace;
		pGridDest[10] = m_pGridOrb;
		pGridDest[11] = m_pGridCabuncle;
		pGridDest[12] = m_pGridGuild;
		pGridDest[13] = m_pGridEvent;
		pGridDest[14] = m_pGridDRing;
		pGridDest[15] = m_pGridMantua;
		pGridDest[16] = m_pGridNewSlot1;
		pGridDest[17] = m_pGridNewSlot2;
		pDestGrid = pGridDest[pSwapItem->DestPos];
		if (pDestGrid)
			pDestItem = pDestGrid->PickupItem(0, 0);
	}
	else if (pSwapItem->DestType == 1)
	{
		int cellX = 0;
		int cellY = 0;
		GetCarryCellForSlot(pSwapItem->DestPos, cellX, cellY);
		pDestGrid = GetCarryGridForSlot(pSwapItem->DestPos);
		if (pDestGrid)
			pDestItem = pDestGrid->PickupAtItem(cellX, cellY);
	}
	else if (pSwapItem->DestType == 2)
	{
		int cellX = 0;
		int cellY = 0;
		GetCargoCellForSlot(pSwapItem->DestPos, cellX, cellY);
		pDestGrid = GetCargoGridForSlot(pSwapItem->DestPos);
		if (pDestGrid)
			pDestItem = pDestGrid->PickupAtItem(cellX, cellY);
	}

	if (pDestItem)
	{
		if (!pSwapItem->SourType)
		{
			if (pDestItem->m_pItem->sIndex > 40)
			{
				const bool visualAdded = pSrcGrid && pSrcGrid->AddItem(pDestItem, 0, 0) == 1;
				if (!visualAdded)
					releaseRejectedVisual(pDestItem);
			}
			else
			{
				releaseRejectedVisual(pDestItem);
			}
		}
		else if (pSwapItem->SourType == 1)
		{
			if (pDestItem->m_pItem->sIndex > 40)
			{
				int cellX = 0;
				int cellY = 0;
				GetCarryCellForSlot(pSwapItem->SourPos, cellX, cellY);
				const bool visualAdded = pSrcGrid && pSrcGrid->AddItem(pDestItem, cellX, cellY) == 1;
				if (!visualAdded)
					releaseRejectedVisual(pDestItem);
			}
			else
			{
				releaseRejectedVisual(pDestItem);
			}
		}
		else if (pSwapItem->SourType == 2)
		{
			if (pDestItem->m_pItem->sIndex > 40)
			{
				int cellX = 0;
				int cellY = 0;
				GetCargoCellForSlot(pSwapItem->SourPos, cellX, cellY);
				const bool visualAdded = pSrcGrid && pSrcGrid->AddItem(pDestItem, cellX, cellY) == 1;
				if (!visualAdded)
					releaseRejectedVisual(pDestItem);
			}
			else
			{
				releaseRejectedVisual(pDestItem);
			}
		}
	}
	if (pSrcItem)
	{
		if (!pSwapItem->DestType)
		{
			if (pSrcItem->m_pItem->sIndex > 40)
			{
				const bool visualAdded = pDestGrid && pDestGrid->AddItem(pSrcItem, 0, 0) == 1;
				if (!visualAdded)
					releaseRejectedVisual(pSrcItem);
			}
			else
			{
				releaseRejectedVisual(pSrcItem);
			}
		}
		else if (pSwapItem->DestType == 1)
		{
			if (pSrcItem->m_pItem->sIndex > 40)
			{
				int cellX = 0;
				int cellY = 0;
				GetCarryCellForSlot(pSwapItem->DestPos, cellX, cellY);
				const bool visualAdded = pDestGrid && pDestGrid->AddItem(pSrcItem, cellX, cellY) == 1;
				if (!visualAdded)
					releaseRejectedVisual(pSrcItem);
			}
			else
			{
				releaseRejectedVisual(pSrcItem);
			}
		}
		else if (pSwapItem->DestType == 2)
		{
			if (pSrcItem->m_pItem->sIndex > 40)
			{
				int cellX = 0;
				int cellY = 0;
				GetCargoCellForSlot(pSwapItem->DestPos, cellX, cellY);
				const bool visualAdded = pDestGrid && pDestGrid->AddItem(pSrcItem, cellX, cellY) == 1;
				if (!visualAdded)
					releaseRejectedVisual(pSrcItem);
			}
			else
			{
				releaseRejectedVisual(pSrcItem);
			}
		}
	}
	ApplyConfirmedItemSwap(*sourceModel, *destinationModel);
	// A confirmed swap can outlive a missing source/destination control or an
	// already absent grid item. Recreate only a missing visible icon from the
	// committed cache; SetItemOnGrid copies the item and owns failed inserts.
	auto restoreMissingVisual = [this](SGridControl* grid, unsigned char type,
		unsigned char position, STRUCT_ITEM* model)
	{
		if (!grid || !model || model->sIndex <= 40)
			return;
		int cellX = 0;
		int cellY = 0;
		if (type == kSwapPlaceCarry)
			GetCarryCellForSlot(position, cellX, cellY);
		else if (type == kSwapPlaceCargo)
			GetCargoCellForSlot(position, cellX, cellY);
		if (!grid->GetAtItem(cellX, cellY))
			grid->SetItemOnGrid(model, cellX, cellY);
	};
	restoreMissingVisual(pSrcGrid, static_cast<unsigned char>(pSwapItem->SourType),
		static_cast<unsigned char>(pSwapItem->SourPos), sourceModel);
	restoreMissingVisual(pDestGrid, static_cast<unsigned char>(pSwapItem->DestType),
		static_cast<unsigned char>(pSwapItem->DestPos), destinationModel);

	auto pMobData = &g_pObjectManager->m_stMobData;
	// WYD 7.48 has no familiar in equipment slot 13: it is the costume slot.
	// Dispose any stale 7.59 familiar and let UpdateMyHuman rebuild the costume.
	if (m_pMyHuman->m_pFamiliar)
	{
		g_pObjectManager->DeleteObject(m_pMyHuman->m_pFamiliar);
		m_pMyHuman->m_pFamiliar = 0;
	}

	m_pMyHuman->m_sFamiliar = 0;
	m_pMyHuman->m_sCostume = pMobData->Equip[13].sIndex;
	if (!pMobData->Guild)
		g_pObjectManager->m_usWarGuild = -1;
	// Mount removal is slot 14 in the 7.48 ABI; slot 15 is the cape.
	if (pSwapItem->SourType == kSwapPlaceEquip && pSwapItem->SourPos == 14 &&
		(pSwapItem->DestType != kSwapPlaceEquip || pSwapItem->DestPos != 14))
		m_pMyHuman->m_sMountIndex = 0;

	auto pSoundManager = g_pSoundManager;
	if (pSoundManager)
	{
		auto pSoundData = pSoundManager->GetSoundData(31);
		if (pSoundData)
			pSoundData->Play();
	}

	g_pCursor->DetachItem();
	SGridControl::m_pLastAttachedItem = 0;
	UpdateScoreUI(0);
	UpdateMyHuman();
	return 1;
}

int TMFieldScene::OnPacketDeposit(MSG_STANDARD* pStd)
{
	auto pDeposit = reinterpret_cast<MSG_STANDARDPARM*>(pStd);

	g_pObjectManager->m_nCargoCoin += pDeposit->Parm;
	g_pObjectManager->m_stMobData.Coin -= pDeposit->Parm;
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketWithdraw(MSG_STANDARD* pStd)
{
	auto pWithdraw = reinterpret_cast<MSG_STANDARDPARM*>(pStd);

	g_pObjectManager->m_nCargoCoin -= pWithdraw->Parm;
	g_pObjectManager->m_stMobData.Coin += pWithdraw->Parm;
	UpdateScoreUI(0);
	return 1;
}

int TMFieldScene::OnPacketCapsuleInfo(MSG_CAPSULEINFO* pStd)
{
	bool bFind = false;
	for (int i = 0; i < 12; ++i)
	{
		if (g_pObjectManager->m_stCapsuleInfo[i].CIndex == pStd->CIndex)
		{
			g_pObjectManager->m_stCapsuleInfo[i] = *pStd;
			bFind = true;
			break;
		}
	}
	if (!bFind)
	{
		for (int i = 0; i < 12; ++i)
		{
			if (!g_pObjectManager->m_stCapsuleInfo[i].CIndex)
			{
				g_pObjectManager->m_stCapsuleInfo[i] = *pStd;
				bFind = true;
				break;
			}
		}
	}

	if (!bFind)
		g_pObjectManager->m_stCapsuleInfo[0] = *pStd;

	SGridControl::m_bNeedUpdate = 1;
	return 1;
}
