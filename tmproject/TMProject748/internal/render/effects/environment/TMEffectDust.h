#pragma once

#include "TMEffect.h"
#include "Structures.h"

class TMMesh;

// Efeito ambiental temporario de poeira ou queda de pedras.
//
// O tipo seleciona entre billboards de poeira e quatro pedras que caem ate o
// terreno, onde podem produzir poeira, respingos e som. A classe apenas
// apresenta o impacto; colisao e regras de jogo nao sao decididas aqui.
class TMEffectDust : public TMEffect
{
public:
    TMEffectDust(TMVector3 vecPosition, float fRadius, int nDustType);
    ~TMEffectDust();

    int FrameMove(unsigned int dwServerTime) override;
    int Render() override;

public:
    int m_nDustType;
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;

    // Referencias compartilhadas obtidas de TMMeshManager. O destrutor nao
    // libera essas malhas; o cache global conserva o ownership.
    TMMesh* m_pMeshDropStone[4];
    TMVector3 m_vecStonePos[4];

    // Marca, por pedra, se o impacto no solo ou na agua ja foi emitido.
    // A grafia historica faz parte do layout e foi preservada.
    char m_cDroped[4];
};
