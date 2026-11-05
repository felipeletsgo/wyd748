#pragma once
#include "../../../application/EffectEmissionClock.h"

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
    effect_emission::Clock m_emissionClock;
public:
    // pOwner e observado durante o efeito e nao pertence a esta classe.
    TMSkillSpChange(TMVector3 vecPosition, int nType, TMObject* pOwner);
    ~TMSkillSpChange();

    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime = 0;
    unsigned int m_dwLifeTime = 0;
    float m_fAngle = 0.0f;
    int m_nType = 0;
    float m_fScale = 0.0f;

    // Objects registered in the scene effect container. The destructor only
    // requests removal of the light map; particles follow their own
    // lifecycle. Types other than 0 and 1 create no light map, so the pointer
    // must default to null before the destructor reads it.
    TMShade* m_pLightMap = nullptr;
    TMEffectParticle* m_pParticle = nullptr;
};
