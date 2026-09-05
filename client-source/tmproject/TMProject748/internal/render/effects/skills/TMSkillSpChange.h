#pragma once

#include "TMEffect.h"

class TMObject;
class TMShade;
class TMEffectParticle;

// Apresentacao visual de uma mudanca de SP ja decidida pelo fluxo de jogo.
//
// A classe combina skin meshes, uma sombra luminosa e particulas. Ela nao
// modifica atributos do personagem nem confirma o resultado da habilidade.
class TMSkillSpChange : public TMEffect
{
public:
    // pOwner e observado durante o efeito e nao pertence a esta classe.
    TMSkillSpChange(TMVector3 vecPosition, int nType, TMObject* pOwner);
    ~TMSkillSpChange();

    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    float m_fAngle;
    int m_nType;
    float m_fScale;

    // Objetos registrados no contêiner global de efeitos. O destrutor apenas
    // solicita a remocao do light map; as particulas seguem seu lifecycle.
    TMShade* m_pLightMap;
    TMEffectParticle* m_pParticle;
};
