#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Speed Up.
// Nao modifica velocidade: apenas representa localmente o evento ja produzido
// pela logica de jogo.
class TMSkillSpeedUp : public TMEffect
{
public:
    TMSkillSpeedUp(TMVector3 vecPosition, int nType);
    ~TMSkillSpeedUp();

    // Atualiza a animacao conforme o relogio do servidor.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do efeito.
    unsigned int m_dwLifeTime;  // Duracao total do efeito.
    float m_fAngle;             // Angulo corrente da animacao.
    int m_nType;                // Variante visual solicitada.
};
