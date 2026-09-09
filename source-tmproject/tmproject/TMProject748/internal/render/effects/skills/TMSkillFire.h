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
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    unsigned int m_dwLastTime;
    unsigned int m_dwColor;
    unsigned int m_dwColor2;
    float m_fAngle;
    int m_nType;

    // Sombra luminosa registrada no contêiner global de efeitos. A classe
    // conserva apenas a referencia para acompanhar a posicao.
    TMShade* m_pLightMap;
};
