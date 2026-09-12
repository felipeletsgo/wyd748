#pragma once

#include "TMEffect.h"
#include "Structures.h"

class TMObject;
class TMEffectBillBoard3;

// Variante de faísca identificada por ID, animada ao longo de um trecho de
// efeito. É exclusivamente visual e não controla a resolução da ação.
class TMEffectSpark2 : public TMEffect
{
public:
    // pTarget é referência não proprietária. Os demais argumentos são copiados
    // e delimitam início, extensão, cores e largura da animação.
    TMEffectSpark2(TMObject* pTarget, unsigned int dwID, float fEffectStart, float fEffectLength, unsigned int dwColor, unsigned int dwColor2, float fWidth);
    ~TMEffectSpark2();
    
    int FrameMove(unsigned int dwServerTime) override;

public:
    // Cada entrada pode ser nula durante criação ou teardown; o array pertence
    // à instância e comporta no máximo cinco billboards auxiliares.
    float m_fWidth;
    float m_fRange;
    TMEffectBillBoard3* m_pSpark[5];
    unsigned int m_dwSparkID;
    float m_fEffectLength;
    unsigned int m_dwColor;
    unsigned int m_dwColor2;
    unsigned int m_dwCreateTime;
    unsigned int m_dwLifeTime;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecEndPos;
};
