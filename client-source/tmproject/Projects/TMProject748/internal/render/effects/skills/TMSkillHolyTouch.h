#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Holy Touch.
// Recebe posicao e variante por valor e nao assume ownership externo.
class TMSkillHolyTouch : public TMEffect
{
public:
    TMSkillHolyTouch(TMVector3 vecPosition, int nType);
    ~TMSkillHolyTouch();

    // Avanca a apresentacao ate o fim de seu lifetime.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do efeito.
    unsigned int m_dwLifeTime;  // Duracao total do efeito.
    float m_fAngle;             // Angulo corrente da animacao.
    int m_nType;                // Variante visual solicitada.
};
