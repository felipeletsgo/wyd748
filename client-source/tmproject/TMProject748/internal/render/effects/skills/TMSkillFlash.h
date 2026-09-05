#pragma once

#include "TMEffect.h"

// Efeito visual temporario associado a Flash.
// A classe apresenta a habilidade na posicao recebida; regras de alvo e efeito
// permanecem fora do renderer.
class TMSkillFlash : public TMEffect
{
public:
    TMSkillFlash(TMVector3 vecPosition, int nType);
    ~TMSkillFlash();

    // Avanca a animacao e encerra seu lifecycle visual pelo tempo do servidor.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do efeito.
    unsigned int m_dwLifeTime;  // Duracao total do efeito.
    float m_fAngle;             // Angulo corrente da animacao.
    int m_nType;                // Variante visual solicitada.
};
