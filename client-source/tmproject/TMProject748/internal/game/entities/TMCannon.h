#pragma once

#include "TMItem.h"
#include "Structures.h"

// Entidade visual de canhao posicionada no mundo como um TMItem.
//
// A classe anima o recuo e apresenta clarão, fumaça, luz e som quando o estado
// de disparo e ativado externamente. Ela nao cria projeteis autoritativos nem
// calcula dano.
class TMCannon : public TMItem
{
public:
    TMCannon();
    ~TMCannon();

    int FrameMove(unsigned int dwServerTime) override;
    void SetAngle(float fYaw, float fPitch, float fRoll) override;
    void SetPosition(float fX, float fY, float fZ) override;

public:
    TMVector2 m_vecBasePosition;
    unsigned int m_dwLastFireTime;
    float m_fSinF;
    float m_fCosF;
    float m_fCannonHeight;
    float m_fCannonLen;

    // Flags de apresentacao alimentadas pelo estado da cena. m_cFire dispara
    // uma vez; m_cAutoFire mantem o ciclo automatico legado.
    char m_cAutoFire;
    char m_cFire;
};
