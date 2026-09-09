#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Thunder Bolt.
// Dano, acerto e alvo nao sao decididos por esta classe de renderizacao.
class TMSkillThunderBolt : public TMEffect
{
public:
    TMSkillThunderBolt(TMVector3 vecPosition, int nType);
    ~TMSkillThunderBolt();

    // Atualiza a animacao conforme o tempo do servidor.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do efeito.
    unsigned int m_dwLifeTime;  // Duracao total do efeito.
    float m_fAngle;             // Angulo corrente da animacao.
    int m_nType;                // Variante visual solicitada.
};
