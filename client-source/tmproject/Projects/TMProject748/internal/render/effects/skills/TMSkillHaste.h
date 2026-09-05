#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Haste.
// Nao aplica velocidade nem valida a habilidade; apenas anima a posicao e a
// variante fornecidas pelo fluxo de efeitos da cena.
class TMSkillHaste : public TMEffect
{
public:
	TMSkillHaste(TMVector3 vecPosition, int nType);
	~TMSkillHaste();

	// Avanca a animacao e encerra o efeito segundo seu lifetime.
	int FrameMove(unsigned int dwServerTime) override;

public:
	unsigned int m_dwStartTime; // Inicio do lifecycle visual.
	unsigned int m_dwLifeTime;  // Duracao total do efeito.
	float m_fAngle;             // Angulo corrente da animacao.
	int m_nType;                // Variante visual solicitada.
};
