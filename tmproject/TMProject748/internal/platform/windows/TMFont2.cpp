#include "pch.h"
#include "ResourceTextGdi.h"
#include "TMFont2.h"
#include "TMGlobal.h"
#include "FontTextureUpload.h"
#include "SingleLineTextLayout.h"
#include <climits>
#include <limits>

char* TMFont2::m_pBuffer{};
unsigned int TMFont2::m_nLength = 0;

TMFont2::TMFont2()
{
	m_dwShadeColor = 0xFF000000;
	m_dwColor = 0xFFFFFFFF;
	m_pTexture = nullptr;
	m_fSize = 1.0f;
	m_bMultiLine = 0;
	m_nLineNumber = 0;
	m_fMaximumWidth = 0.0f;
	m_fRasterScale = m_fSize;
	m_bRasterMultiLine = m_bMultiLine;
	m_bCheckZero = 0;
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
	m_bCheckZero = bCheckZero;
	return Rasterize();
}

int TMFont2::Rasterize()
{
	m_fRasterScale = m_fSize;
	m_bRasterMultiLine = m_bMultiLine;
	if (!g_pDevice || !g_pDevice->m_hDC) {
		SAFE_RELEASE(m_pTexture);
		return FALSE;
	}
	const auto measure = [](const std::string& text) {
		float width = 0.0f;
		// Match the texture chunk boundaries, including their GDI metrics.
		for (size_t offset = 0; offset < text.size(); offset += 42) {
			SIZE size{};
			const int count = static_cast<int>((std::min)(size_t{42}, text.size() - offset));
			if (!resource_text::Measure(g_pDevice->m_hDC, text.data() + offset, count, &size))
				return std::numeric_limits<float>::infinity();
			width += size.cx;
		}
		return width;
	};
	const auto display = m_bMultiLine == 1 ? std::string(m_szString) :
		font_layout::Ellipsize(m_szString, m_fSize, m_fMaximumWidth, measure);
	const int nLen = static_cast<int>(display.size());

	// Windows-1252 has one byte per glyph, including accented letters. The
	// old DBCS boundary test could move an accent into the wrong chunk.
	memset(m_szStringArray, 0, sizeof(m_szStringArray));
	m_nLineNumber = nLen > 42 ? 2 : 1;
	for (int line = 0; line < m_nLineNumber; ++line)
	{
		const int offset = line * 42;
		const int count = (nLen - offset) > 42 ? 42 : nLen - offset;
		memcpy(m_szStringArray[line], display.data() + offset, count);
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
			if (!resource_text::Draw(g_pDevice->m_hDC, 0, nLine * (RenderDevice::m_nFontSize + 1), szTemp, sizeof(szTemp)))
				return failTexture();

			sprintf(szTemp, "%s ", m_szStringArray[nLine]);

			if (strlen(szTemp) && !resource_text::Draw(g_pDevice->m_hDC, 0,
				nLine * (RenderDevice::m_nFontSize + 1), szTemp, static_cast<int>(strlen(szTemp))))
				return failTexture();

			if (m_bCheckZero)
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
					const BOOL drawn = resource_text::Draw(g_pDevice->m_hDC, 0,
						nLine * (RenderDevice::m_nFontSize + 1), szTemp2, static_cast<int>(strlen(szTemp2)));
					const int restored = SetBkMode(g_pDevice->m_hDC, OPAQUE);
					if (!drawn || !restored)
						return failTexture();
				}
			}

			SIZE size{};
			if (!resource_text::Measure(g_pDevice->m_hDC, m_szStringArray[nLine],
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

float TMFont2::GetSingleLineWidth() const
{
	const float width = font_layout::Measure(m_szStringSize, m_nLineNumber);
	return width > 0.0f ? width * m_fSize + 1.0f : 0.0f;
}

void TMFont2::SetMaximumWidth(float width)
{
	if (!std::isfinite(width) || width < 0.0f)
		width = 0.0f;
	if (m_fMaximumWidth == width && m_fRasterScale == m_fSize && m_bRasterMultiLine == m_bMultiLine)
		return;
	m_fMaximumWidth = width;
	Rasterize();
}

int TMFont2::Render(int nPosX, int nPosY, int nRenderType)
{
	if (m_nPosX != -1)
		nPosX = m_nPosX;
	if (m_nPosY != -1)
		nPosY = m_nPosY;

	nPosY = nPosY + 1;
	if (!g_pDevice || !g_pDevice->m_pd3dDevice ||
		((m_pTexture == nullptr || m_fRasterScale != m_fSize || m_bRasterMultiLine != m_bMultiLine) && !Rasterize()))
		return 0;

	int strLen = strlen(m_szString);

	if (strLen <= 0 || strLen >= MAX_STRRENDER)
		return 0;

	g_pDevice->SetRenderStateBlock(2);
	if (m_bMultiLine != 1 && GetSingleLineWidth() <= 0.0f)
		return 0;
	// Keep the unbounded legacy chunk positioning for non-UI font users.
	const float advanceScale = m_fMaximumWidth > 0.0f ? m_fSize : 1.0f;
	const float singleLineY = static_cast<float>(nPosY);

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
					((float)nPosX + nLength * advanceScale) + 1.0f,
					singleLineY + 1.0f,
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
				(float)nPosX + nLength * advanceScale,
				singleLineY,
				m_pTexture,
				m_dwColor,
				m_fSize,
				m_fSize);
		}

		nLength += nLocLen;
	}

	return TRUE;
}
