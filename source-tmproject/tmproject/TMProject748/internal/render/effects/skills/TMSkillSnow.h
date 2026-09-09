#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Placeholder do efeito de neve associado a uma posição-alvo.
// Os buffers existem na definição, mas não há simulação/renderização ativa.
class TMSkillSnow : public TMEffect
{
public:
    // vecTarget está preservado na API; o stub atual ainda não inicializa estado.
    TMSkillSnow(TMVector3 vecTarget);
    ~TMSkillSnow();

    // Métodos inertes até a confirmação de recursos, timing e lifecycle.
    int Render() override;
    int IsVisible() override;
    int FrameMove(unsigned int dwServerTime) override;

public:
    // Estado reservado para até 1.000 partículas. Não é válido para leitura
    // enquanto o construtor permanecer sem inicialização.
    unsigned int m_dwOldServerTime;
    float m_fSpeed[1000];
    TMVector3 m_vecSnowPos[1000];
    RDLVERTEX m_vertex[4];
    float m_fScale;
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    TMVector3 m_vecTarget;
};
