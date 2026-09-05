#pragma once

#include "TMEffect.h"
#include "Structures.h"

class TMShade;
class TMObject;
class TMEffectBillBoard3;

// Conjunto de até cinco faíscas que liga uma origem a um alvo/posição. A classe
// anima efeitos locais e não transmite nem confirma ações de combate.
class TMEffectSpark : public TMEffect
{
public:
    // pTarget é observado, sem transferência de ownership. Cores, largura,
    // quantidade e progresso inicial são copiados para a instância.
    TMEffectSpark(TMVector3 vecStart, TMObject* pTarget, TMVector3 vecEnd, unsigned int dwColor, unsigned int dwColor2, unsigned int dwLifeTime, float fWidth, int nSparkCount, float fProgress);
    ~TMEffectSpark();
    
    int FrameMove(unsigned int dwServerTime) override;
    // Atualiza a duração e retorna o status definido pela implementação.
    int SetLifeTime(unsigned int dwLifeTime);

public:
    // Entradas dos arrays são opcionais; a quantidade válida é limitada por
    // m_nSparkCount e pela capacidade fixa de cinco elementos.
    float m_fWidth;
    float m_fRange;
    TMEffectBillBoard3* m_pSpark[5];
    TMShade* m_pShade[5];
    unsigned int m_dwColor;
    unsigned int m_dwColor2;
    unsigned int m_dwCreateTime;
    unsigned int m_dwLifeTime;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecEndPos;
    int m_nSparkCount;
};
