#pragma once

#include "TMEffect.h"

class TMObject;

// Efeito visual temporario associado a Heal.
// pOwner referencia o objeto que origina/recebe a apresentacao; a classe nao
// declara transferencia de ownership e o ponteiro pode ser apenas observado.
class TMSkillHeal : public TMEffect
{
public:
    TMSkillHeal(TMVector3 vecPosition, int nType, TMObject* pOwner);
    ~TMSkillHeal();

    // Avanca a animacao e seu lifetime pelo tempo do servidor.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do efeito.
    unsigned int m_dwLifeTime;  // Duracao total do efeito.
    unsigned int m_dwLastTime;  // Ultimo instante processado.
    int m_nType;                // Variante visual solicitada.
};
