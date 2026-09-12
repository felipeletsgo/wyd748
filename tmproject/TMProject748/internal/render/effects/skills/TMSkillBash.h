#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Bash.
// Esta classe nao decide acerto, dano ou alvo; ela somente apresenta o evento
// que ja foi aceito pelo fluxo de jogo.
class TMSkillBash : public TMEffect
{
public:
	TMSkillBash(TMVector3 vecPosition, int nType);
	~TMSkillBash();

	// Avanca a animacao usando o tempo do servidor.
	int FrameMove(unsigned int dwServerTime) override;

public:
	unsigned int m_dwStartTime; // Inicio do lifecycle visual.
	unsigned int m_dwLifeTime;  // Duracao total do efeito.
	int m_nType;                // Variante visual solicitada.
	unsigned int m_dwLastTime;  // Ultimo instante processado.
};
