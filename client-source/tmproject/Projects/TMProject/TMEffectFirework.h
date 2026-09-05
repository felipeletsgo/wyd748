#pragma once

#include "TreeNode.h"
#include "Structures.h"

struct Particle
{
    // Estado cinemático de uma partícula individual. Os tempos usam a mesma
    // unidade adotada por FrameMove no efeito (milissegundos no client atual).
    TMVector3 vPosition;
    TMVector3 vLaunchVelocity;
    TMVector3 vInitialPosition;
    TMVector3 vInitialVelocity;
    float fLifetime;
    float fMaturity;
    unsigned short wType;
    float fSize;
};

class TMEffectFireWork : public TreeNode
{
public:
    // Cria um efeito na posição informada. nType seleciona a variante visual;
    // o objeto é administrado pelo lifecycle de TreeNode/ObjectManager.
    TMEffectFireWork(TMVector3 vecPositiont, int nType);
    ~TMEffectFireWork();
    
    int FrameMove(unsigned int dwServerTime) override;
    int Render() override;
    // Inicializa a partícula fornecida e devolve o mesmo ponteiro. result deve
    // ser válido e continua pertencendo ao chamador.
    Particle* SetParticle(Particle* result, unsigned short wType, float fLifeTime, TMVector3 vBasePosition, TMVector3 vBaseVelocity);
    // Carrega a forma customizada a partir de um buffer textual não nulo; o
    // método copia os dados necessários e não assume ownership do buffer.
    void SetCustomFireWork(const char* pBuffer);

public:
    // Buffer e temporização pertencem à própria instância do efeito.
    int m_nType;
    unsigned int m_dwColor;
    RDLVERTEX m_vertex[4];
    Particle m_Particle[100];
    unsigned int m_dwStartTime;
    int m_bBomb;
    TMVector3 m_vAddFireWork[100];
    int m_nColorType;
    char m_ArrCustomShape[100];
};
