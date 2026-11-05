#pragma once

#include "TMEffect.h"

class TMShade;

// Emissor visual de fogo usado por habilidades e eventos da cena.
//
// O tipo controla textura, cadencia e duracao dos billboards. O efeito pode
// acompanhar temporariamente um owner, mas nao aplica dano nem altera estado
// do alvo.
class TMSkillFire : public TMEffect
{
public:
    // pOwner e uma referencia nao proprietaria e pode ser nula.
    TMSkillFire(TMVector3 vecPosition, int nType, TMObject* pOwner, unsigned int dwColor, unsigned int dwColor2);
    ~TMSkillFire();

    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime = 0;
    unsigned int m_dwLifeTime = 0;
    unsigned int m_dwLastTime = 0;
    unsigned int m_dwColor = 0;
    unsigned int m_dwColor2 = 0;
    float m_fAngle = 0.0f;
    int m_nType = 0;

    // Light map registered in the scene effect container. The class keeps
    // only a reference to move it. Types 7 and 8 create none, so the pointer
    // must default to null.
    TMShade* m_pLightMap = nullptr;
};
