#pragma once

#include "TMEffect.h"
#include "Structures.h"

// Apresentacao visual temporaria de mudanca de nivel.
// A classe nao altera nivel ou atributos; ela cria efeitos filhos no container
// da cena e controla apenas o lifetime de sua instancia principal.
class TMEffectLevelUp : public TMEffect
{
public:
    TMEffectLevelUp(TMVector3 vecPosition, int nType);
    ~TMEffectLevelUp();
    
    // Atualiza visibilidade e solicita remocao ao fim do lifetime.
    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime; // Inicio do efeito.
    unsigned int m_dwLifeTime;  // Duracao da instancia principal.
    float m_fAngle;             // Campo visual preservado no layout historico.
    int m_nType;                // Variante visual de level up.
};
