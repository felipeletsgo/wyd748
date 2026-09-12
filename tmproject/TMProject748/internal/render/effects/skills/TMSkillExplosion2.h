#pragma once

#include "TMEffect.h"

// Controlador visual de uma explosao que se expande em oito direcoes.
//
// A classe consulta o terreno e a camera para posicionar emissores de fogo,
// transferindo cada efeito criado ao contêiner da cena. Dano e regras de area
// nao sao calculados aqui.
class TMSkillExplosion2 : public TMEffect
{
public:
    TMSkillExplosion2(TMVector3 vecPosition, int nType, float fRange, unsigned int dwTerm, unsigned int dwColor);
    ~TMSkillExplosion2();

    int FrameMove(unsigned int dwServerTime) override;

public:
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    unsigned int m_dwLastTime;
    unsigned int m_dwTerm;
    unsigned int m_dwColor;
    int m_nType;
    float m_fRange;

    // Tabela compartilhada das direcoes radiais. Os tamanhos historicos sao
    // preservados porque integram o layout estatico da implementacao atual.
    static float m_fDirX[8];
    static float m_fDirY[11];
};
