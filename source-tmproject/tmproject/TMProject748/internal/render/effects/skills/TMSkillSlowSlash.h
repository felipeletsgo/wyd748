#pragma once

#include "TMEffect.h"

class TMObject;

// Trajetoria visual temporaria da habilidade Slow Slash.
//
// O efeito interpola a origem ate o alvo e emite billboards periodicamente.
// O owner recebido e apenas acompanhado durante a primeira metade da vida;
// dano, alvo e aplicacao do debuff pertencem ao fluxo de jogo.
class TMSkillSlowSlash : public TMEffect
{
public:
    // pOwner e uma referencia nao proprietaria e pode ser nula.
    TMSkillSlowSlash(TMVector3 vecStart, TMVector3 vecTarget, int nType, TMObject* pOwner);
    ~TMSkillSlowSlash();

    int FrameMove(unsigned int dwServerTime) override;

public:
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    unsigned int m_dwLastTime;
    int m_nType;
    int m_nTextureIndex;
};
