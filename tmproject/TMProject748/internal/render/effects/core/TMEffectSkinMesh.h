#pragma once
#include "../../../application/EffectEmissionClock.h"

#include "TMEffect.h"
#include "Structures.h"
#include "TMSkinMesh.h"
#include "TMObject.h"

class TMEffectSkinMesh : public TMObject
{
    effect_emission::Clock m_emissionClock;
public:
    TMEffectSkinMesh(int nSkinMeshType, TMVector3 vecStart, TMVector3 vecTarget, int nLevel, TMObject* pOwner);
    ~TMEffectSkinMesh();
    
    virtual int InitObject(int bExpand);
    int Render() override;
    int FrameMove(unsigned int dwServerTime) override;
    void RestoreDeviceObjects() override;
    void InvalidateDeviceObjects() override;
    void InitPosition(float fX, float fY, float fZ) override;

private:
    // Natural expiry spawns the follow-up effect. Destruction alone (scene
    // teardown, owner removal) must not spawn effects with a stale owner.
    void SpawnFollowUp();

public:
    LOOK_INFO m_stLookInfo;
    SANC_INFO m_stSancInfo;
    unsigned int m_dwStartTime;
    unsigned int m_dwLifeTime;
    EEFFECT_ALPHATYPE m_efAlphaType;
    int m_nMotionType;
    float m_fStartAngle;
    D3DCOLORVALUE m_StartColor;
    D3DCOLORVALUE m_EndColor;
    D3DCOLORVALUE m_color;
    int m_nFade;
    TMVector3 m_vecStartPos;
    TMVector3 m_vecTargetPos;
    int m_nLevel;
    float m_fAngle;
    unsigned int m_dwLastSpeedUp;
    LOOK_INFO m_stLookInfo2;
    TMSkinMesh* m_pSkinMesh2;
    int m_nSkinMeshType2;
    float m_fScale2;
    int m_bMirror;
};