#pragma once

#include "TreeNode.h"
#include "Structures.h"

// Define como o efeito combina sua cor com o framebuffer.
enum class EEFFECT_ALPHATYPE
{
	EF_DEFAULT = 0x0,
	EF_BRIGHT = 0x1,
	EF_NONEBRIGHT = 0x2,
	EF_ALPHA = 0x3,
};

// Base comum para objetos puramente visuais inseridos na arvore da cena.
//
// A classe conserva estado de transformacao, composicao e visibilidade. As
// subclasses implementam animacao e desenho; regras autoritativas de jogo nao
// devem depender deste tipo de apresentacao.
class TMEffect : public TreeNode
{
public:
	TMEffect();
	~TMEffect();

	int Render() override;
	int FrameMove(unsigned int dwServerTime) override;
	virtual int IsVisible();

public:
	int m_bShow;
	int m_bVisible;
	D3DXMATRIX m_matEffect;
	EEFFECT_ALPHATYPE m_efAlphaType;
	float m_fRadius;
	TMVector3 m_vecPosition;
};
