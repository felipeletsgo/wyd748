#pragma once

constexpr int MAX_STRLENGTH = 120;
constexpr int MAX_STRRENDER = 256;

// Legacy text renderer that keeps its own copy of the string and the
// rasterized texture. The fixed limits are part of the current contract.
class TMFont2
{
public:
    TMFont2();
    virtual ~TMFont2();
    
    // Copies szString into the internal buffer, applies the color and optionally
    // handles the zero character. The caller keeps ownership of the string.
    // Returns zero on invalid input or resource failure. Resource failure releases
    // the texture; Render may retry but never draws a failed upload.
    int SetText(const char* szString, unsigned int dwColor, int bCheckZero);
    // Returns a mutable pointer to the internal buffer; valid only while this
    // instance exists and receives no other text.
    char* GetText();
    // Renders at the given position and mode; transfers no resources.
    int Render(int nPosX, int nPosY, int nRenderType);
    // Optional screen-space bound for single-line UI captions. Zero disables
    // ellipsis; multiline and non-UI callers retain their existing rendering.
    // Never resizes rasterized glyphs. GetText keeps the complete source.
    void SetMaximumWidth(float width);
    float GetSingleLineWidth() const;

private:
    float m_fMaximumWidth;
    float m_fRasterScale;
    int m_bRasterMultiLine;
    int m_bCheckZero;
    int Rasterize();

public:
    // m_pTexture belongs to the instance and may be null before SetText or
    // after a creation failure. m_pBuffer is scratch shared between fonts.
    unsigned int m_dwShadeColor;
    unsigned int m_dwColor;
    int m_bMultiLine;
    float m_fSize;
    int m_nLineNumber;
    char m_szString[MAX_STRRENDER];
    char m_szStringArray[3][44];
    short m_szStringSize[3];
    IDirect3DTexture9* m_pTexture;
	static char* m_pBuffer;
	static unsigned int m_nLength;
    int m_nPosX;
    int m_nPosY;
    int m_UsedFont;
    int m_FontSize;
};
