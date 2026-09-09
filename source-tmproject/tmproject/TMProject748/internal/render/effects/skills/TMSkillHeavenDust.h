#pragma once

#include "TMEffect.h"

// Efeito visual temporario de poeira celestial.
// O nome do arquivo historico permanece no singular para nao quebrar includes;
// a classe publica original continua sendo TMSkillHeavensDust.
class TMSkillHeavensDust : public TMEffect
{
public:
	TMSkillHeavensDust(TMVector3 vecPosition, int nType);
	~TMSkillHeavensDust();

	// Avanca a animacao e seu lifetime no relogio do servidor.
	int FrameMove(unsigned int dwServerTime) override;

public:
	unsigned int m_dwStartTime; // Inicio do lifecycle visual.
	unsigned int m_dwLifeTime;  // Duracao total do efeito.
	float m_fAngle;             // Angulo corrente da animacao.
	int m_nType;                // Variante visual solicitada.
};
