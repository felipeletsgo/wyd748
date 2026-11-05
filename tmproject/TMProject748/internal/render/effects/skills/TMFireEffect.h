#pragma once

#include "TreeNode.h"
#include "Structures.h"

class TMObject;

// Rastro visual de fogo emitido de uma origem em direcao a um alvo.
//
// O alvo e consultado somente na construcao para calcular direcao, distancia e
// duracao. A classe depois cria billboards ao longo dessa trajetoria; acerto e
// dano continuam sob responsabilidade do fluxo autoritativo de jogo.
class TMFireEffect : public TreeNode
{
public:
    // pTarget e uma referencia de entrada e nao fica armazenada.
    TMFireEffect(TMVector3 vecStart, TMObject* pTarget, int nTextureIndex);
    ~TMFireEffect();
    
    int FrameMove(unsigned int dwServerTime) override;

public:
    // Defaults keep a null-target instance defined: it expires after 1 ms.
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    TMVector3 m_vecDistance;
    unsigned int m_dwCreateTime = 0;
    unsigned int m_dwLifeTime = 1;
    int m_nEntity = 0;
    int m_nTextureIndex = 0;
    float m_fLen = 0.0f;
};
