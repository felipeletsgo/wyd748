#pragma once

#include "TreeNode.h"

class TMEffectBillBoard3;
class TMEffectMesh;

// Projétil visual que interpola entre dois pontos e mantém efeitos auxiliares.
// Não representa dano: o servidor continua autoritativo sobre o combate.
class TMArrow : public TreeNode
{
public:
    // nDestID apenas associa o visual ao destino; os ponteiros de efeitos são
    // criados/liberados pelo lifecycle da seta, conforme ReleaseEffect.
    TMArrow(TMVector3 vecStart, TMVector3 vecTarget, int nLevel, int nType, char cAvatar, int nColor, int nDestID);
    ~TMArrow();

    int Render() override;
    // Consultas sem transferência de ownership. Retornam o estado calculado
    // pela seta para renderização e descarte fora da câmera.
    virtual int IsVisible();
    int IsInView();
    int FrameMove(unsigned int dwServerTime) override;
    // Desassocia e encerra os efeitos filhos; pode ser chamado no teardown.
    void ReleaseEffect();

public:
    // m_pBeam e m_pEffectMesh podem ser nulos durante criação/teardown.
    unsigned int m_dwStartTime;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    TMVector3 m_vecCurrentPos;
    int m_nLevel;
    float m_fAngle;
    float m_fRotAngle;
    int m_bVisible;
    int m_nType;
    int m_nMeshIndex;
    unsigned int m_dwLifeTime;
    TMEffectBillBoard3* m_pBeam;
    TMEffectMesh* m_pEffectMesh;
    int m_nColor;
    unsigned int m_nDestID;
    char m_cAvatar;
    TMVector2 m_vecRotatePos1;
    TMVector2 m_vecRotatePos2;
    TMVector2 m_vecRotatePos3;
};
