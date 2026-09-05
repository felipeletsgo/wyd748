#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Cure.
// A cura permanece responsabilidade da logica de jogo/servidor; esta classe
// apenas controla a apresentacao local criada para uma posicao valida.
class TMSkillCure : public TMEffect
{
public:
	TMSkillCure(TMVector3 vecPosition, int nType);
	~TMSkillCure();

	// Avanca a animacao e sinaliza o fim conforme o contrato de TMEffect.
	int FrameMove(unsigned int dwServerTime) override;

public:
	unsigned int m_dwStartTime; // Inicio do lifecycle visual.
	unsigned int m_dwLifeTime;  // Duracao total do efeito.
	float m_fAngle;             // Angulo corrente da animacao.
	int m_nType;                // Variante visual solicitada.
};
