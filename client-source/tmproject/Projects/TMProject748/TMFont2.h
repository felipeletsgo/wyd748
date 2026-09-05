#pragma once

constexpr int MAX_STRLENGTH = 120;
constexpr int MAX_STRRENDER = 256;

// Renderizador de texto legado que mantém sua própria cópia da string e a
// textura rasterizada. Os limites fixos fazem parte do contrato atual.
class TMFont2
{
public:
    TMFont2();
    virtual ~TMFont2();
    
    // Copia szString para o buffer interno, aplica a cor e opcionalmente trata
    // o caractere zero. O chamador conserva ownership da string.
    int SetText(const char* szString, unsigned int dwColor, int bCheckZero);
    // Retorna ponteiro mutável para o buffer interno; válido somente enquanto
    // esta instância existir e não receber outro texto.
    char* GetText();
    // Renderiza na posição e modo informados; não transfere recursos.
    int Render(int nPosX, int nPosY, int nRenderType);
    // Analisa os bytes da string para respeitar caracteres multibyte legados.
    int StrByteCheck(char* szString);

public:
    // m_pTexture é recurso da instância e pode ser nulo antes de SetText ou
    // após falha de criação. m_pBuffer é scratch compartilhado entre fontes.
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
