#pragma once

#include "Enums.h"

class TMFont2;

// Nó descritivo de uma primitiva da UI geométrica. A estrutura é mantida sem
// reorganização porque seu layout é consumido por código legado do client.
class GeomControl
{
public:
	// O construtor completo copia tipo, geometria, camada, textura e cor.
	GeomControl();
	GeomControl(RENDERCTRLTYPE ieRenderType, int inTextureSetIndex, float inPosX, float inPosY, float inWidth, float inHeight, int inLayer, unsigned int idwColor);

public:
	// pFont é uma referência opcional ao renderizador de fonte. m_pNextGeom
	// encadeia a lista e não implica, isoladamente, ownership do próximo nó.
	RENDERCTRLTYPE eRenderType;
	float nPosX;
	float nPosY;
	float nWidth;
	float nHeight;
	int nZ;
	float fAngle;
	float fScale;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;
	unsigned int dwColor;
	int bVisible;
	int n3DObjIndex;
	int nTextureSetIndex;
	int nTextureIndex;
	short sLegend;
	short sSanc;
	char strString[256];
	int nLayer;
	TMFont2* pFont;
	GeomControl* m_pNextGeom;
	float m_fWidth;
	float m_fHeight;
	unsigned int dwBGColor;
	int nMarkIndex;
	int nMarkLayout;
	int bClip;
};

struct stGeomList
{
	// Cabeça e cauda de uma lista intrusiva de GeomControl. O responsável por
	// alocar/liberar os nós é o container que mantém esta lista.
	GeomControl* pHeadGeom;
	GeomControl* pTailGeom;
};
