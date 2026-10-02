#include "pch.h"
// TMScene split by responsibility; the scene lifecycle and dispatch stay in TMScene.cpp.
#include "SControlContainer.h"
#include "SGrid.h"
#include "UIBinary.h"
#include "TMGround.h"
#include "TMSky.h"
#include "TMSun.h"
#include "TMLight.h"
#include "TMCamera.h"
#include "TMObjectContainer.h"
#include "TMItem.h"
#include "TMGlobal.h"
#include "TMLog.h"
#include "TMScene.h"
#include "TMFieldScene.h"
#include "TMEffectFirework.h"
#include "TMSnow.h"
#include "TMRain.h"
#include "WYD748Assets.h"

namespace
{
	bool CopyUIString(int stringIndex, char* output, size_t outputCapacity,
		int controlID, const char* resourceName)
	{
		if (!output || outputCapacity == 0 || stringIndex < 0 || stringIndex >= MAX_STRING)
		{
			LOG_WRITELOG("Invalid UI string index [%d] for control [%d] in [%s]\r\n",
				stringIndex, controlID, resourceName ? resourceName : "<unknown>");
			return false;
		}

		strncpy_s(output, outputCapacity, g_UIString[stringIndex], _TRUNCATE);
		return true;
	}
}

int TMScene::LoadRC(const char* szFileName)
{
	if (!m_pControlContainer)
		m_pControlContainer = new SControlContainer(this);

	char szBinFileName[128]{};

	sprintf_s(szBinFileName, "%s", szFileName);

	int nLength = strlen(szBinFileName);

	if (strchr(szBinFileName, '_'))
		sprintf_s(&szBinFileName[nLength - 7], sizeof(szBinFileName) - nLength - 7, ".bin");
	else
		sprintf_s(&szBinFileName[nLength - 3], sizeof(szBinFileName) - nLength - 3, "bin");

	return ReadRCBin(szBinFileName);
}

int TMScene::ParseRC(FILE* fp, FILE* fpBinary, char* szControlType)
{
	return 0;
}

int TMScene::ReadRCBin(char* szBinFileName)
{
	FILE* fpBinary = nullptr;

	fopen_s(&fpBinary, szBinFileName, "rb");

	if (!fpBinary)
		return 0;

	// Some older 7.48-compatible RC files store button/text captions inline.
	// Detect that ABI once per file while the primary Scene2 UI keeps the compact
	// indexed-string parser used by the live 7.48 selection screen.
	const bool legacyInlineCaptions = WYD748_IsLegacyRCFile(szBinFileName);

	int rawControlType = 0;
	while (true)
	{
		const auto readResult = ReadRCControlType(fpBinary, rawControlType);
		if (readResult == RCControlTypeReadResult::End)
			break;
		if (readResult != RCControlTypeReadResult::Record)
		{
			LOG_WRITELOG("Incomplete RC control type in [%s]\r\n", szBinFileName);
			fclose(fpBinary);
			return 0;
		}
		const auto nControlType = static_cast<CONTROL_TYPE>(rawControlType);
		switch (nControlType)
		{
		case CONTROL_TYPE::CTRL_TYPE_PANEL:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCPanel(fpBinary, nControlType, szBinFileName, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_GRID:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCGrid(fpBinary, nControlType, szBinFileName, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_3DOBJ:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRC3DObj(fpBinary, nControlType, szBinFileName, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_BUTTON:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCButton(fpBinary, nControlType, szBinFileName, legacyInlineCaptions, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_TEXT:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCText(fpBinary, nControlType, szBinFileName, legacyInlineCaptions, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCEditableText(fpBinary, nControlType, szBinFileName, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_PROGRESSBAR:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCProgressBar(fpBinary, nControlType, szBinFileName, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_CHECKBOX:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCCheckBox(fpBinary, nControlType, szBinFileName, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		case CONTROL_TYPE::CTRL_TYPE_LISTBOX:
		{
			int extractedResult{};
			const ExtractedFlow extractedFlow = ReadRCListBox(fpBinary, nControlType, szBinFileName, extractedResult);
			if (extractedFlow == ExtractedFlow::Return)
				return extractedResult;
		}
		break;
		default:
			LOG_WRITELOG("Not Support Control Type [%d]\r\n", nControlType);
			fclose(fpBinary);
			return 0;
		}
	}

	fclose(fpBinary);
	return 1;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCPanel(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinPanel binPanelData;

	if (!fread(&binPanelData, sizeof(binPanelData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
		fclose(fpBinary);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	auto pPanel = new SPanel(
		binPanelData.nTextureSetIndex,
		(float)binPanelData.nStartX,
		(float)binPanelData.nStartY,
		(float)binPanelData.nWidth,
		(float)binPanelData.nHeight,
		binPanelData.nColor,
		static_cast<RENDERCTRLTYPE>(binPanelData.nFillType));

	if (!pPanel)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binPanelData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pPanel->SetControlID(binPanelData.nID);
	pPanel->SetCenterPos(binPanelData.nID,
		(float)binPanelData.nStartX,
		(float)binPanelData.nStartY,
		(float)binPanelData.nWidth,
		(float)binPanelData.nHeight);

	if (binPanelData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binPanelData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pPanel));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pPanel));
	}

	pPanel->m_bPickable = binPanelData.nPickable;
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCGrid(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinGrid binGridData;

	if (!fread(&binGridData, sizeof(binGridData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
		fclose(fpBinary);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	auto pGrid = new SGridControl(
		binGridData.nTextureSetIndex,
		binGridData.nRowCount,
		binGridData.nColumnCount,
		(float)binGridData.nStartX,
		(float)binGridData.nStartY,
		(float)binGridData.nWidth,
		(float)binGridData.nHeight,
		static_cast<TMEITEMTYPE>(binGridData.nType));

	if (!pGrid)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binGridData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pGrid->SetControlID(binGridData.nID);

	pGrid->SetCenterPos(binGridData.nID,
		(float)binGridData.nStartX,
		(float)binGridData.nStartY,
		(float)binGridData.nWidth,
		(float)binGridData.nHeight);

	if (m_pControlContainer)
		pGrid->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pGrid->SetEventListener(nullptr);

	if (binGridData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binGridData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pGrid));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pGrid));
	}
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRC3DObj(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	Bin3DObj bin3DObjData;

	if (!fread(&bin3DObjData, sizeof(bin3DObjData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
		fclose(fpBinary);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	auto p3DObj = new S3DObj(bin3DObjData.n3DObjIndex,
		(float)bin3DObjData.nStartX,
		(float)bin3DObjData.nStartY,
		(float)bin3DObjData.nWidth,
		(float)bin3DObjData.nHeight);

	if (!p3DObj)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", bin3DObjData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	p3DObj->SetControlID(bin3DObjData.nID);
	p3DObj->SetCenterPos(bin3DObjData.nID,
		(float)bin3DObjData.nStartX,
		(float)bin3DObjData.nStartY,
		(float)bin3DObjData.nWidth,
		(float)bin3DObjData.nHeight);

	if (bin3DObjData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(bin3DObjData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(p3DObj));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(p3DObj));
	}
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCButton(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, const bool& legacyInlineCaptions, int& extractedResult)
{
	BinButton binButtonData{};
	char strbuf[128]{};
	if (legacyInlineCaptions)
	{
		// Map the disk-only 7.48 record into the common modern fields;
		// the caption is already part of the record and needs no index.
		LegacyBinButton legacy{};
		if (!fread(&legacy, sizeof(legacy), 1, fpBinary))
		{
			LOG_WRITELOG("Can't Read Legacy Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
			fclose(fpBinary);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		binButtonData.nID = legacy.nID;
		binButtonData.nParentID = legacy.nParentID;
		binButtonData.nTextureSetIndex = legacy.nTextureSetIndex;
		binButtonData.nStartX = legacy.nStartX;
		binButtonData.nStartY = legacy.nStartY;
		binButtonData.nWidth = legacy.nWidth;
		binButtonData.nHeight = legacy.nHeight;
		binButtonData.nColor = legacy.nColor;
		binButtonData.nSound = legacy.nSound;
		strcpy_s(strbuf, legacy.szString);
	}
	else
	{
		if (!fread(&binButtonData, sizeof(binButtonData), 1, fpBinary))
		{
			LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
			fclose(fpBinary);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		if (!CopyUIString(binButtonData.nStringIndex, strbuf, sizeof(strbuf),
			binButtonData.nID, szBinFileName))
		{
			fclose(fpBinary);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
	}

	auto pButton = new SButton(
		binButtonData.nTextureSetIndex,
		(float)binButtonData.nStartX,
		(float)binButtonData.nStartY,
		(float)binButtonData.nWidth,
		(float)binButtonData.nHeight,
		binButtonData.nColor,
		binButtonData.nSound,
		strbuf);

	if (!pButton)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binButtonData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pButton->SetControlID(binButtonData.nID);

	pButton->SetCenterPos(binButtonData.nID,
		(float)binButtonData.nStartX,
		(float)binButtonData.nStartY,
		(float)binButtonData.nWidth,
		(float)binButtonData.nHeight);

	if (binButtonData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binButtonData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pButton));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pButton));
	}

	if (m_pControlContainer)
		pButton->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pButton->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCText(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, const bool& legacyInlineCaptions, int& extractedResult)
{
	BinText binTextData{};
	char strbuf[128]{};
	if (legacyInlineCaptions)
	{
		// Text records use the same inline-caption contract as legacy
		// buttons; copy fields explicitly so layouts never alias.
		LegacyBinText legacy{};
		if (!fread(&legacy, sizeof(legacy), 1, fpBinary))
		{
			LOG_WRITELOG("Can't Read Legacy Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
			fclose(fpBinary);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		binTextData.nID = legacy.nID;
		binTextData.nParentID = legacy.nParentID;
		binTextData.nTextureSetIndex = legacy.nTextureSetIndex;
		binTextData.nStartX = legacy.nStartX;
		binTextData.nStartY = legacy.nStartY;
		binTextData.nWidth = legacy.nWidth;
		binTextData.nHeight = legacy.nHeight;
		binTextData.nFontColor = legacy.nFontColor;
		binTextData.nBorder = legacy.nBorder;
		binTextData.nBorderColor = legacy.nBorderColor;
		binTextData.nTextType = legacy.nTextType;
		binTextData.nAlignType = legacy.nAlignType;
		strcpy_s(strbuf, legacy.szString);
	}
	else
	{
		if (!fread(&binTextData, sizeof(binTextData), 1, fpBinary))
		{
			LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
			fclose(fpBinary);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		if (!CopyUIString(binTextData.nStringIndex, strbuf, sizeof(strbuf),
			binTextData.nID, szBinFileName))
		{
			fclose(fpBinary);
			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
	}

	auto pText = new SText(
		binTextData.nTextureSetIndex,
		strbuf,
		binTextData.nFontColor,
		(float)binTextData.nStartX,
		(float)binTextData.nStartY,
		(float)binTextData.nWidth,
		(float)binTextData.nHeight,
		binTextData.nBorder,
		binTextData.nBorderColor,
		binTextData.nTextType,
		binTextData.nAlignType);

	if (!pText)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binTextData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pText->SetControlID(binTextData.nID);

	pText->SetCenterPos(binTextData.nID,
		(float)binTextData.nStartX,
		(float)binTextData.nStartY,
		(float)binTextData.nWidth,
		(float)binTextData.nHeight);

	if (binTextData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binTextData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pText));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pText));
	}
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCEditableText(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinEdit binEditData;

	if (!fread(&binEditData, sizeof(binEditData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
		fclose(fpBinary);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	auto pEdit = new SEditableText(
		binEditData.nTextureSetIndex,
		binEditData.szString,
		binEditData.nMaxStringLength,
		binEditData.nPassword,
		binEditData.nFontColor,
		(float)binEditData.nStartX,
		(float)binEditData.nStartY,
		(float)binEditData.nWidth,
		(float)binEditData.nHeight,
		binEditData.nBorder,
		binEditData.nBorderColor,
		binEditData.nTextType,
		binEditData.nAlignType);

	if (!pEdit)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binEditData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pEdit->SetControlID(binEditData.nID);

	pEdit->SetCenterPos(binEditData.nID,
		(float)binEditData.nStartX,
		(float)binEditData.nStartY,
		(float)binEditData.nWidth,
		(float)binEditData.nHeight);

	if (binEditData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binEditData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pEdit));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pEdit));
	}

	if (m_pControlContainer)
		pEdit->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pEdit->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCProgressBar(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinProgress binProgressData;

	if (!fread(&binProgressData, sizeof(binProgressData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
		fclose(fpBinary);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	auto pProgress = new SProgressBar(
		binProgressData.nTextureSetIndex,
		binProgressData.nCurrent,
		binProgressData.nMaxValue,
		(float)binProgressData.nStartX,
		(float)binProgressData.nStartY,
		(float)binProgressData.nWidth,
		(float)binProgressData.nHeight,
		binProgressData.nProgressColor,
		binProgressData.nColor,
		binProgressData.nStyle);

	if (!pProgress)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binProgressData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pProgress->SetControlID(binProgressData.nID);

	pProgress->SetCenterPos(binProgressData.nID,
		(float)binProgressData.nStartX,
		(float)binProgressData.nStartY,
		(float)binProgressData.nWidth,
		(float)binProgressData.nHeight);

	if (binProgressData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binProgressData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pProgress));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pProgress));
	}

	if (m_pControlContainer)
		pProgress->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pProgress->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCCheckBox(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinCheckBox binCheckBoxData;

	if (!fread(&binCheckBoxData, sizeof(binCheckBoxData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
		fclose(fpBinary);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	auto pCheckBox = new SCheckBox(
		binCheckBoxData.nTextureSetIndex,
		(float)binCheckBoxData.nStartX,
		(float)binCheckBoxData.nStartY,
		(float)binCheckBoxData.nWidth,
		(float)binCheckBoxData.nHeight,
		binCheckBoxData.nColor);

	if (!pCheckBox)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binCheckBoxData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pCheckBox->SetControlID(binCheckBoxData.nID);

	pCheckBox->SetCenterPos(binCheckBoxData.nID,
		(float)binCheckBoxData.nStartX,
		(float)binCheckBoxData.nStartY,
		(float)binCheckBoxData.nWidth,
		(float)binCheckBoxData.nHeight);

	if (binCheckBoxData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binCheckBoxData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pCheckBox));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pCheckBox));
	}

	if (m_pControlContainer)
		pCheckBox->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pCheckBox->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// Extracted from TMScene::ReadRCBin; behavior is unchanged.
ExtractedFlow TMScene::ReadRCListBox(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinListBox binListBoxData;

	if (!fread(&binListBoxData, sizeof(binListBoxData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);
		fclose(fpBinary);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	auto pListBox = new SListBox(
		binListBoxData.nTextureSetIndex,
		binListBoxData.nMaxCount,
		binListBoxData.nVisibleCount,
		(float)binListBoxData.nStartX,
		(float)binListBoxData.nStartY,
		(float)binListBoxData.nWidth,
		(float)binListBoxData.nHeight,
		binListBoxData.nColor,
		static_cast<RENDERCTRLTYPE>(binListBoxData.nFillType),
		binListBoxData.nSelect,
		binListBoxData.nScroll,
		0);

	if (!pListBox)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binListBoxData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	pListBox->SetControlID(binListBoxData.nID);

	pListBox->SetCenterPos(binListBoxData.nID,
		(float)binListBoxData.nStartX,
		(float)binListBoxData.nStartY,
		(float)binListBoxData.nWidth,
		(float)binListBoxData.nHeight);

	if (binListBoxData.nParentID)
	{
		auto pParent = m_pControlContainer->FindControl(binListBoxData.nParentID);

		if (pParent)
			pParent->AddChild(static_cast<TreeNode*>(pListBox));
	}
	else
	{
		m_pControlContainer->AddItem(static_cast<SControl*>(pListBox));
	}

	if (m_pControlContainer)
		pListBox->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pListBox->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}


int TMScene::FindID(char* szID)
{
	int nID = 0;
	int bFindID = 0;

	if (!strcmp(szID, "NONE"))
		return 0;

	for (int i = 0; i < 2560; ++i)
	{
		if (!strcmp(szID, g_pObjectManager->m_ResourceList[i].szString))
		{
			nID = g_pObjectManager->m_ResourceList[i].nNumber;
			bFindID = 1;
			break;
		}
	}

	if (!bFindID)
		LOG_WRITELOG("Cannot Match Resource ID [%s] in [%s].\r\n", szID, "UI\\TMResource.h");

	return nID;
}

int TMScene::LoadMsgText(SListBox* pListBox, const char* szFileName)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	if (!pListBox)
		return 0;

	char szText[256]{};
	char szTemp[256]{};

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[7]{};

		strncpy(szCol, szTemp, 6);

		DWORD dwCol{};

		sscanf(szCol, "%x", &dwCol);

		char* szRet = strstr(szTemp, "\n");

		if (szRet)
			*szRet = 0;

		if (szTemp[6] == 32)
		{
			sprintf_s(szText, "%s", &szTemp[6]);

			pListBox->AddItem(new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				pListBox->m_nWidth,
				16.0f,
				0,
				0x77777777u,
				1u,
				0));
		}
	}

	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

int TMScene::LoadMsgText2(SListBox* pListBox, const char* szFileName, int nStartLine, int nEndLine)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	if (!pListBox)
		return 0;

	char szText[256]{};
	char szTemp[256]{};

	int nCount = 0;

	pListBox->Empty();

	for (int i = 0; i < 560 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[7]{};

		strncpy(szCol, szTemp, 6);

		DWORD dwCol{};

		sscanf(szCol, "%x", &dwCol);

		char* szRet = strstr(szTemp, "\n");

		if (szRet)
			*szRet = 0;

		if (szTemp[6] == 32 && i >= nStartLine && i <= nEndLine)
		{
			sprintf_s(szText, "%s", &szTemp[6]);

			pListBox->AddItem(new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				pListBox->m_nWidth,
				16.0f,
				0,
				0x77777777u,
				1u,
				0));

			if (++nCount > 100)
				break;
		}
	}

	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

int TMScene::LoadMsgText3(SListBox* pListBox, const char* szFileName, int nLv, int ntrans)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	if (!pListBox)
		return 0;

	char szText[256]{};
	char szTemp[256]{};

	int nLvLimit = 300;

	pListBox->Empty();

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[11]{};

		strncpy(szCol, szTemp, 10);

		DWORD dwCol{};

		sscanf_s(szCol, "%d %x", &nLvLimit, &dwCol);

		if (nLvLimit > nLv)
			dwCol = 0xFF777777;

		if (nLvLimit > 350 && ntrans < 6)
			dwCol = 0xFF777777;

		if (g_pCurrentScene)
		{
			if (m_pMyHuman->Is2stClass() == 2)
				dwCol = 0xFF777777;

			else if (nLvLimit > nLv)
				dwCol = 0xFF777777;
		}

		char* szRet = strstr(szTemp, "\n");

		if (szRet)
			*szRet = 0;

		if (szTemp[10] == 32)
		{
			sprintf(szText, "%s", &szTemp[10]);

			pListBox->AddItem(new SListBoxItem(
				szText,
				dwCol | 0xFF000000,
				0.0f,
				0.0f,
				pListBox->m_nWidth,
				16.0f,
				0,
				0x77777777u,
				1u,
				0));
		}
	}

	if (pListBox->m_pScrollBar)
		pListBox->m_pScrollBar->SetCurrentPos(0);

	fclose(fp);
	return 1;
}

unsigned int TMScene::LoadMsgText4(char* pStr, int dwStrSize, const char* szFileName, int nLv, int ntrans)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	int nLvLimit = 300;
	DWORD dwCol = 0;

	char szTemp[256]{};

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[11]{};

		strncpy(szCol, szTemp, 10);

		sscanf_s(szCol, "%d %x", &nLvLimit, &dwCol);

		if (nLvLimit == nLv)
		{
			if (dwCol == 0xFFAAAA && ntrans >= 6)
			{
				dwCol = 0;
			}
			else if (nLvLimit <= 350 || ntrans >= 6)
			{
				char* szRet = strstr(szTemp, "\n");

				if (szRet)
					*szRet = 0;

				if (szTemp[10] == 32)
					sprintf_s(pStr, dwStrSize, "%s", &szTemp[10]);
			}
			else
			{
				dwCol = 0;
			}

			break;
		}
	}

	fclose(fp);

	return dwCol | 0xFF000000;
}

int TMScene::LoadMsgLevel(char* LevelQuest, const char* szFileName, char cType)
{
	FILE* fp{};

	fopen_s(&fp, szFileName, "rt");

	if (!fp)
		return 0;

	char szTemp[256]{};

	for (int i = 0; i < 100 && fgets(szTemp, 256, fp); ++i)
	{
		char szCol[11]{};

		strncpy(szCol, szTemp, 10);

		int nLvLimit{};
		DWORD dwCol{};

		sscanf_s(szCol, "%d %x", &nLvLimit, &dwCol);

		LevelQuest[nLvLimit - 1] = cType;
	}
	fclose(fp);
	return 1;
}
