#include "pch.h"
#include "TMFont2.h"
#include "TMGlobal.h"
#include "FontTextureUpload.h"
#include <climits>

char* TMFont2::m_pBuffer{};
unsigned int TMFont2::m_nLength = 0;

TMFont2::TMFont2()
{
	m_dwShadeColor = 0xFF000000;
	m_dwColor = 0xFFFFFFFF;
	m_pTexture = nullptr;
	m_fSize = 1.0f;
	m_bMultiLine = 0;
	memset(m_szString, 0, sizeof(m_szString));
	memset(m_szStringArray, 0, sizeof(m_szStringArray));
	memset(m_szStringSize, 0, sizeof(m_szStringSize));

	m_nPosX = -1;
	m_nPosY = -1;
}

TMFont2::~TMFont2()
{
	SAFE_RELEASE(m_pTexture);
}

int TMFont2::SetText(const char* szString, unsigned int dwColor, int bCheckZero)
{
	if (!szString)
		return 0;
	const auto nStrLength = strnlen_s(szString, MAX_STRLENGTH);
	if (nStrLength >= MAX_STRLENGTH)
		return 0;
	m_nLineNumber = 0;

	if (RenderDevice::m_nBright > 58)
	{
		int nValue = RenderDevice::m_nBright - 40;
		int dwA = dwColor & 0xFF000000;

		int nR = WYDCOLOR_RED(dwColor) - nValue;
		int nG = WYDCOLOR_GREEN(dwColor) - nValue;
		int nB = WYDCOLOR_BLUE(dwColor) - nValue;

		if (nR < 0)
			nR = 0;
		if (nG < 0)
			nG = 0;
		if (nB < 0)
			nB = 0;

		dwColor = WYD_RGBA(nR, nG, nB, dwA);
	}

	m_dwColor = dwColor;

	// Render may retry with this instance's own buffer after a device failure.
	memmove(m_szString, szString, nStrLength + 1);
	m_szString[83] = 0;
	m_szString[82] = 0;

	char tempbuff[256]{};
	char* temp = tempbuff;

	strcpy(tempbuff, m_szString);
	int nLen = strlen(m_szString);

	if (nLen > 84)
	{
		if (IsClearString(temp, ')'))
		{
			strncpy(m_szStringArray[0], temp, 42);
			temp += '*';
		}
		else
		{
			strncpy(m_szStringArray[0], temp, 41);
			temp += ')';
		}
		if (IsClearString(temp, ')'))
		{
			strncpy(m_szStringArray[1], temp, 42);
			temp += '*';
			strncpy(m_szStringArray[2], temp, strlen(temp));
		}
		else
		{
			strncpy(m_szStringArray[1], temp, 41);
			temp += ')';
			strcpy(m_szStringArray[2], temp);
		}
		m_nLineNumber = 3;
	}
	else if (nLen > 42)
	{
		memset(m_szStringArray[0], 0, 44);
		memset(m_szStringArray[1], 0, 44);
		if (IsClearString(temp, ')'))
		{
			strncpy(m_szStringArray[0], temp, 42);
			temp += '*';
		}
		else
		{
			strncpy(m_szStringArray[0], temp, 41);
			temp += ')';
		}

		strcpy(m_szStringArray[1], temp);
		m_nLineNumber = 2;
	}
	else
	{
		memset(m_szStringArray[0], 0, 44);
		strncpy(m_szStringArray[0], temp, nLen);
		m_nLineNumber = 1;
	}

	const auto failTexture = [this]() {
		SAFE_RELEASE(m_pTexture);
		return FALSE;
	};
	if (!g_pDevice || !g_pDevice->m_pd3dDevice || !g_pDevice->m_hbmBitmap ||
		!g_pDevice->m_hDC || !g_pDevice->m_pBitmapBits || RenderDevice::m_nFontSize <= 0 ||
		RenderDevice::m_nFontSize >= INT_MAX / m_nLineNumber - 1)
		return failTexture();
	const int maskRows = m_nLineNumber * (RenderDevice::m_nFontSize + 1);
	if (!font_texture::ValidMaskDimensions(RenderDevice::m_nFontTextureSize,
		RenderDevice::m_nFontTextureSize, maskRows) ||
		maskRows > RenderDevice::m_nFontTextureSize / 8)
		return failTexture();
	{
		memset(m_szStringSize, 0, sizeof(m_szStringSize));
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = RenderDevice::m_nFontTextureSize;
		rect.bottom = maskRows;

		if (!FillRect(g_pDevice->m_hDC, &rect, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH))))
			return failTexture();
		for (int nLine = 0; nLine < m_nLineNumber; ++nLine)
		{
			char szTemp[45]{};
			memset(szTemp, ' ', sizeof(szTemp));

			// This buffer contains only spaces and has no terminator yet.
			if (!TextOut(g_pDevice->m_hDC, 0, nLine * (RenderDevice::m_nFontSize + 1), szTemp, sizeof(szTemp)))
				return failTexture();

			sprintf(szTemp, "%s ", m_szStringArray[nLine]);

			if (strlen(szTemp) && !TextOut(g_pDevice->m_hDC, 0,
				nLine * (RenderDevice::m_nFontSize + 1), szTemp, static_cast<int>(strlen(szTemp))))
				return failTexture();

			if (bCheckZero)
			{
				char szTemp2[45]{};

				bool bFind = false;
				int nLenTemp = strlen(szTemp);

				for (int nT = 0; nT < nLenTemp; ++nT)
				{
					if (szTemp[nT] == '0')
					{
						szTemp2[nT] = '/';
						bFind = true;
					}
					else
					{
						szTemp2[nT] = ' ';
					}
				}

				if (bFind)
				{
					if (!SetBkMode(g_pDevice->m_hDC, TRANSPARENT))
						return failTexture();
					const BOOL drawn = TextOut(g_pDevice->m_hDC, 0,
						nLine * (RenderDevice::m_nFontSize + 1), szTemp2, static_cast<int>(strlen(szTemp2)));
					const int restored = SetBkMode(g_pDevice->m_hDC, OPAQUE);
					if (!drawn || !restored)
						return failTexture();
				}
			}

			SIZE size{};
			if (!GetTextExtentPoint32(g_pDevice->m_hDC, m_szStringArray[nLine],
				static_cast<int>(strlen(m_szStringArray[nLine])), &size))
				return failTexture();

			m_szStringSize[nLine] = static_cast<short>(size.cx);
		}
	}

	if (m_pTexture == nullptr)
	{
		HRESULT created = E_FAIL;
		if (g_pDevice->m_bSavage == 1)
		{
			created = D3DXCreateTexture(g_pDevice->m_pd3dDevice,
				RenderDevice::m_nFontTextureSize,
				RenderDevice::m_nFontTextureSize / 8,
				1,
				0,
				D3DFORMAT::D3DFMT_A4R4G4B4,
				D3DPOOL::D3DPOOL_MANAGED,
				&m_pTexture);
		}
		else
		{
			if (TMFont2::m_pBuffer == nullptr)
			{
				std::unique_ptr<char[]> seed;
				unsigned int payloadLength = 0;
				if (!font_texture::LoadSeed(MiniMap_Path, seed, payloadLength))
					return failTexture();
				TMFont2::m_pBuffer = seed.release();
				TMFont2::m_nLength = payloadLength;
			}

			created = D3DXCreateTextureFromFileInMemoryEx(
				g_pDevice->m_pd3dDevice,
				TMFont2::m_pBuffer,
				TMFont2::m_nLength + 18,
				RenderDevice::m_nFontTextureSize,
				RenderDevice::m_nFontTextureSize / 8,
				1,
				0,
				D3DFORMAT::D3DFMT_A4R4G4B4,
				D3DPOOL::D3DPOOL_MANAGED,
				1,
				1,
				0xFF000000,
				NULL,
				NULL,
				&m_pTexture
				);
		}
		if (FAILED(created) || !m_pTexture)
			return failTexture();
	}

	if (!font_texture::UploadMask(m_pTexture, g_pDevice->m_pBitmapBits,
		RenderDevice::m_nFontTextureSize, RenderDevice::m_nFontTextureSize, maskRows))
		return failTexture();
	return TRUE;
}

char* TMFont2::GetText()
{
	return m_szString;
}

int TMFont2::Render(int nPosX, int nPosY, int nRenderType)
{
	if (m_nPosX != -1)
		nPosX = m_nPosX;
	if (m_nPosY != -1)
		nPosY = m_nPosY;

	nPosY = nPosY + 1;
	if (!g_pDevice || !g_pDevice->m_pd3dDevice ||
		(m_pTexture == nullptr && !SetText(m_szString, m_dwColor, 0)))
		return 0;

	int strLen = strlen(m_szString);

	if (strLen <= 0 || strLen >= MAX_STRRENDER)
		return 0;

	g_pDevice->SetRenderStateBlock(2);

	int nLength = 0;
	for (int nLine = 0; nLine < m_nLineNumber; ++nLine)
	{
		int nLocLen = m_szStringSize[nLine];
		if (m_bMultiLine == 1)
		{
			if (nRenderType != (int)RENDERCTRLTYPE::RENDER_TEXT)
			{
				g_pDevice->RenderRectC(
					0.0f,
					(float)nLine * (float)(RenderDevice::m_nFontSize + 1),
					(float)(nLocLen),
					(float)RenderDevice::m_nFontSize,
					(float)(nPosX + 1),
					((float)(nPosY + 1) + (float)(nLine * RenderDevice::m_nFontSize)),
					m_pTexture,
					m_dwShadeColor,
					1.0f,
					1.0f);
			}

			g_pDevice->RenderRectC(
				0.0f,
				(float)nLine * (float)(RenderDevice::m_nFontSize + 1),
				(float)nLocLen,
				(float)RenderDevice::m_nFontSize,
				(float)(nPosX),
				(float)((float)(nPosY) + (float)(nLine * RenderDevice::m_nFontSize)),
				m_pTexture,
				m_dwColor,
				1.0f,
				1.0f);
		}
		else
		{
			if (nRenderType != (int)RENDERCTRLTYPE::RENDER_TEXT)
			{
				g_pDevice->RenderRectC(
					0.0f,
					(float)nLine * (float)(RenderDevice::m_nFontSize + 1),
					(float)nLocLen,
					(float)RenderDevice::m_nFontSize,
					((float)nPosX + (float)nLength) + 1.0f,
					(float)(nPosY + 1),
					m_pTexture,
					m_dwShadeColor,
					m_fSize,
					m_fSize);
			}

			g_pDevice->RenderRectC(
				0.0f,
				(float)nLine * (float)(RenderDevice::m_nFontSize + 1),
				(float)nLocLen,
				(float)RenderDevice::m_nFontSize,
				(float)nPosX + (float)nLength,
				(float)nPosY,
				m_pTexture,
				m_dwColor,
				m_fSize,
				m_fSize);
		}

		nLength += nLocLen;
	}

	return TRUE;
}
