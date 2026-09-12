#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Poison.
// Cor, quantidade, audio e variante configuram somente a apresentacao; a classe
// nao aplica dano periodico nem altera o estado autoritativo do personagem.
class TMSkillPoison : public TMEffect
{
public:
    TMSkillPoison(TMVector3 vecPosition, unsigned int dwColor, int nCount, int bSound, int nType);
    ~TMSkillPoison();

    // Atualiza particulas e lifetime pelo tempo do servidor.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do efeito.
    unsigned int m_dwLifeTime;  // Duracao total do efeito.
    float m_fAngle;             // Angulo corrente da animacao.
    int m_nType;                // Variante visual solicitada.
};
