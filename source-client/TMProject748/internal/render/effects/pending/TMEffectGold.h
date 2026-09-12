#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Placeholder de um efeito dourado com trajetória e duração.
// A classe está compilada, mas seu comportamento visual ainda não foi adaptado.
class TMEffectGold : public TMEffect
{
public:
    // vecStart e vecLiveTime estão preservados na API, porém o stub ainda não
    // os materializa em estado interno.
    TMEffectGold(TMVector3 vecStart, float vecLiveTime);
    ~TMEffectGold();
    // Retorna inativo enquanto não houver contrato de atualização/renderização.
    int FrameMove(unsigned int dwServerTime) override;
    int Render() override;

public:
    // Estado reservado pela definição original da source. Não consumir estes
    // campos até o construtor passar a inicializá-los com evidência suficiente.
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecEndPos;
    float m_LiveTime;
    float m_Hight;
    int m_Meshidx;
};
