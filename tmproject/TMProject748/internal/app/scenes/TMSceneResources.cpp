#include "pch.h"
// TMScene split by responsibility; the scene lifecycle and dispatch stay in TMScene.cpp.
#include "SControlContainer.h"
#include "SGrid.h"
#include "UIBinary.h"
#include "RCControlTransaction.h"
#include <memory>
#include <new>
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
#include <share.h>

namespace
{
	bool AttachRCControl(SControlContainer* container, SControl* control, int parentID)
	{
		SControl* parent = parentID ? container->FindControl(parentID) : container->m_pControlRoot;
		if (!parent)
		{
			LOG_WRITELOG("Missing RC parent [%d] for control [%u]\r\n",
				parentID, control->GetControlID());
			return false;
		}
		parent->AddChild(control);
		return true;
	}

	bool CopyUIString(int stringIndex, char* output, size_t outputCapacity,
		int controlID, const char* resourceName)
	{
		if (!CopyRCIndexedCaption(g_UIString, stringIndex, output, outputCapacity))
		{
			LOG_WRITELOG("Invalid or missing UI caption [%d] for control [%d] in [%s]\r\n",
				stringIndex, controlID, resourceName ? resourceName : "<unknown>");
			return false;
		}

		return true;
	}
}

int TMScene::LoadRC(const char* szFileName) try
{
	char szBinFileName[128]{};
	if (!ResolveRCBinaryPath(szFileName, szBinFileName, sizeof(szBinFileName)))
	{
		LOG_WRITELOG("Invalid RC resource path\r\n");
		return 0;
	}
	if (!m_pControlContainer)
		m_pControlContainer = new SControlContainer(this);

	return ReadRCBin(szBinFileName);
}
catch (const std::bad_alloc&)
{
	LOG_WRITELOG("Cannot allocate RC control container\r\n");
	return 0;
}

int TMScene::ParseRC(FILE* fp, FILE* fpBinary, char* szControlType)
{
	return 0;
}

int TMScene::ReadRCBin(char* szBinFileName) try
{
	if (!szBinFileName || !m_pControlContainer)
		return 0;
	FILE* fpBinary = nullptr;

	// Keep validation and construction on stable bytes until this handle closes.
	fpBinary = _fsopen(szBinFileName, "rb", _SH_DENYWR);

	if (!fpBinary)
		return 0;

	const std::unique_ptr<FILE, decltype(&std::fclose)> fileOwner(fpBinary, &std::fclose);

	// Some older 7.48-compatible RC files store button/text captions inline.
	// Detect that ABI once per file while the primary Scene2 UI keeps the compact
	// indexed-string parser used by the live 7.48 selection screen.
	const bool legacyInlineCaptions = WYD748_IsLegacyRCFile(szBinFileName);
	RCValidationError validationError{};
	if (!ValidateRCStream(fpBinary, legacyInlineCaptions, static_cast<int>(_countof(g_UIString)),
		sizeof(SEditableText::m_strText),
		[this](unsigned int id) { return m_pControlContainer->FindControl(id) != nullptr; },
		WYD748_TranslateControlID, validationError, sizeof(GeomControl)))
	{
		LOG_WRITELOG("Invalid RC resource [%s] at byte [%ld], control [%d]: %s\r\n",
			szBinFileName, validationError.offset, validationError.controlID, validationError.reason);

		return 0;
	}

	RCControlTransaction<TreeNode> transaction(m_pControlContainer->m_pControlRoot);
	int rawControlType = 0;
	while (true)
	{
		const auto readResult = ReadRCControlType(fpBinary, rawControlType);
		if (readResult == RCControlTypeReadResult::End)
			break;
		if (readResult != RCControlTypeReadResult::Record)
		{
			LOG_WRITELOG("Incomplete RC control type in [%s]\r\n", szBinFileName);

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

			return 0;
		}
	}

	transaction.Commit();
	return 1;
}
catch (const std::bad_alloc&)
{
	LOG_WRITELOG("RC construction ran out of memory in [%s]\r\n", szBinFileName);
	return 0;
}

// The local owner releases the control only after successful tree attachment.
ExtractedFlow TMScene::ReadRCPanel(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinPanel binPanelData;

	if (!fread(&binPanelData, sizeof(binPanelData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	std::unique_ptr<SPanel> controlOwner(new SPanel(
		binPanelData.nTextureSetIndex,
		(float)binPanelData.nStartX,
		(float)binPanelData.nStartY,
		(float)binPanelData.nWidth,
		(float)binPanelData.nHeight,
		binPanelData.nColor,
		static_cast<RENDERCTRLTYPE>(binPanelData.nFillType)));
	auto pPanel = controlOwner.get();

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

	if (!AttachRCControl(m_pControlContainer, pPanel, binPanelData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();

	pPanel->m_bPickable = binPanelData.nPickable;
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
ExtractedFlow TMScene::ReadRCGrid(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinGrid binGridData;

	if (!fread(&binGridData, sizeof(binGridData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	std::unique_ptr<SGridControl> controlOwner(new SGridControl(
		binGridData.nTextureSetIndex,
		binGridData.nRowCount,
		binGridData.nColumnCount,
		(float)binGridData.nStartX,
		(float)binGridData.nStartY,
		(float)binGridData.nWidth,
		(float)binGridData.nHeight,
		static_cast<TMEITEMTYPE>(binGridData.nType)));
	auto pGrid = controlOwner.get();

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

	if (!AttachRCControl(m_pControlContainer, pGrid, binGridData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
ExtractedFlow TMScene::ReadRC3DObj(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	Bin3DObj bin3DObjData;

	if (!fread(&bin3DObjData, sizeof(bin3DObjData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	std::unique_ptr<S3DObj> controlOwner(new S3DObj(bin3DObjData.n3DObjIndex,
		(float)bin3DObjData.nStartX,
		(float)bin3DObjData.nStartY,
		(float)bin3DObjData.nWidth,
		(float)bin3DObjData.nHeight));
	auto p3DObj = controlOwner.get();

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

	if (!AttachRCControl(m_pControlContainer, p3DObj, bin3DObjData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
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
		WYD748_LocalizeCaption(szBinFileName, static_cast<int>(nControlType),
			legacy.nID, legacy.nParentID, legacy.nTextureSetIndex, strbuf, sizeof(strbuf));
	}
	else
	{
		if (!fread(&binButtonData, sizeof(binButtonData), 1, fpBinary))
		{
			LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		if (!CopyUIString(binButtonData.nStringIndex, strbuf, sizeof(strbuf),
			binButtonData.nID, szBinFileName))
		{

			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
	}

	std::unique_ptr<SButton> controlOwner(new SButton(
		binButtonData.nTextureSetIndex,
		(float)binButtonData.nStartX,
		(float)binButtonData.nStartY,
		(float)binButtonData.nWidth,
		(float)binButtonData.nHeight,
		binButtonData.nColor,
		binButtonData.nSound,
		strbuf));
	auto pButton = controlOwner.get();

	if (!pButton)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binButtonData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	if (!pButton->IsCaptionReady())
	{
		LOG_WRITELOG("Cannot rasterize RC button caption [%d] in [%s]\r\n", binButtonData.nID, szBinFileName);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}
	pButton->SetControlID(binButtonData.nID);

	pButton->SetCenterPos(binButtonData.nID,
		(float)binButtonData.nStartX,
		(float)binButtonData.nStartY,
		(float)binButtonData.nWidth,
		(float)binButtonData.nHeight);

	if (!AttachRCControl(m_pControlContainer, pButton, binButtonData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();

	if (m_pControlContainer)
		pButton->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pButton->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
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
		WYD748_LocalizeCaption(szBinFileName, static_cast<int>(nControlType),
			legacy.nID, legacy.nParentID, legacy.nTextureSetIndex, strbuf, sizeof(strbuf));
	}
	else
	{
		if (!fread(&binTextData, sizeof(binTextData), 1, fpBinary))
		{
			LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
		if (!CopyUIString(binTextData.nStringIndex, strbuf, sizeof(strbuf),
			binTextData.nID, szBinFileName))
		{

			{ extractedResult = 0; return ExtractedFlow::Return; }
		}
	}

	std::unique_ptr<SText> controlOwner(new SText(
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
		binTextData.nAlignType));
	auto pText = controlOwner.get();

	if (!pText)
	{
		LOG_WRITELOG("Can't Create [%d] in Scene [%d]\r\n", binTextData.nID, GetSceneType());
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	if (!pText->IsCaptionReady())
	{
		LOG_WRITELOG("Cannot rasterize RC text caption [%d] in [%s]\r\n", binTextData.nID, szBinFileName);
		{ extractedResult = 0; return ExtractedFlow::Return; }
	}
	pText->SetControlID(binTextData.nID);

	pText->SetCenterPos(binTextData.nID,
		(float)binTextData.nStartX,
		(float)binTextData.nStartY,
		(float)binTextData.nWidth,
		(float)binTextData.nHeight);

	if (!AttachRCControl(m_pControlContainer, pText, binTextData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
ExtractedFlow TMScene::ReadRCEditableText(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinEdit binEditData;

	if (!fread(&binEditData, sizeof(binEditData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	std::unique_ptr<SEditableText> controlOwner(new SEditableText(
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
		binEditData.nAlignType));
	auto pEdit = controlOwner.get();

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

	if (!AttachRCControl(m_pControlContainer, pEdit, binEditData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();

	if (m_pControlContainer)
		pEdit->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pEdit->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
ExtractedFlow TMScene::ReadRCProgressBar(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinProgress binProgressData;

	if (!fread(&binProgressData, sizeof(binProgressData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	std::unique_ptr<SProgressBar> controlOwner(new SProgressBar(
		binProgressData.nTextureSetIndex,
		binProgressData.nCurrent,
		binProgressData.nMaxValue,
		(float)binProgressData.nStartX,
		(float)binProgressData.nStartY,
		(float)binProgressData.nWidth,
		(float)binProgressData.nHeight,
		binProgressData.nProgressColor,
		binProgressData.nColor,
		binProgressData.nStyle));
	auto pProgress = controlOwner.get();

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

	if (!AttachRCControl(m_pControlContainer, pProgress, binProgressData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();

	if (m_pControlContainer)
		pProgress->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pProgress->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
ExtractedFlow TMScene::ReadRCCheckBox(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinCheckBox binCheckBoxData;

	if (!fread(&binCheckBoxData, sizeof(binCheckBoxData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	std::unique_ptr<SCheckBox> controlOwner(new SCheckBox(
		binCheckBoxData.nTextureSetIndex,
		(float)binCheckBoxData.nStartX,
		(float)binCheckBoxData.nStartY,
		(float)binCheckBoxData.nWidth,
		(float)binCheckBoxData.nHeight,
		binCheckBoxData.nColor));
	auto pCheckBox = controlOwner.get();

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

	if (!AttachRCControl(m_pControlContainer, pCheckBox, binCheckBoxData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();

	if (m_pControlContainer)
		pCheckBox->SetEventListener(static_cast<IEventListener*>(m_pControlContainer));
	else
		pCheckBox->SetEventListener(nullptr);
	return ExtractedFlow::Next;
}

// The local owner releases the control only after successful tree attachment.
ExtractedFlow TMScene::ReadRCListBox(FILE*& fpBinary, const CONTROL_TYPE& nControlType, char*& szBinFileName, int& extractedResult)
{
	BinListBox binListBoxData;

	if (!fread(&binListBoxData, sizeof(binListBoxData), 1, fpBinary))
	{
		LOG_WRITELOG("Can't Read Resource Data[%d] in [%s]\r\n", nControlType, szBinFileName);

		{ extractedResult = 0; return ExtractedFlow::Return; }
	}

	std::unique_ptr<SListBox> controlOwner(new SListBox(
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
		0));
	auto pListBox = controlOwner.get();

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

	if (!AttachRCControl(m_pControlContainer, pListBox, binListBoxData.nParentID))
		{ extractedResult = 0; return ExtractedFlow::Return; }
	controlOwner.release();

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

	const auto textPath = WYD748_TextPath(szFileName);
	fopen_s(&fp, textPath.c_str(), "rt");

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

	const auto textPath = WYD748_TextPath(szFileName);
	fopen_s(&fp, textPath.c_str(), "rt");

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

	const auto textPath = WYD748_TextPath(szFileName);
	fopen_s(&fp, textPath.c_str(), "rt");

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

	const auto textPath = WYD748_TextPath(szFileName);
	fopen_s(&fp, textPath.c_str(), "rt");

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

	const auto textPath = WYD748_TextPath(szFileName);
	fopen_s(&fp, textPath.c_str(), "rt");

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
