#pragma once
#include "../../../application/EffectEmissionClock.h"

#include "TMEffect.h"
#include "Structures.h"
#include "EffectTiming.h"

class TMEffectBillBoard;
class TMSkinMesh;

struct stSwingScale
{
    int nType;
    float Scale;
};

static stSwingScale sSwingScale[22] =
{
  { 1,  1.0f },
  { 2,  1.0f },
  { 3,  1.0f },
  { 11,  1.0f },
  { 12,  1.0f },
  { 13,  1.0f },
  { 21,  1.0f },
  { 22,  1.0f },
  { 23,  1.0f },
  { 31,  1.0f },
  { 32,  1.0f },
  { 33,  1.3f },
  { 41,  1.0f },
  { 51,  1.0f },
  { 61,  1.0f },
  { 62,  1.0f },
  { 63,  1.0f },
  { 101,  1.0f },
  { 102,  1.0f },
  { 103,  1.0f },
  { 104,  1.0f },
  { 105,  1.0f }
};

class TMEffectSWSwing : public TMEffect
{
    effect_emission::Clock m_emissionClock;
public:
    TMEffectSWSwing();
    ~TMEffectSWSwing();
    
    int Render() override;
    int FrameMove(unsigned int dwServerTime) override;
    virtual void SetColor(unsigned int dwColor);

public:
    // Every scalar and pointer member has a default initializer: Render can
    // run before the first timed FrameMove (an attack sets m_cSForce first).
    unsigned int m_dwStartTime = 0;
    TMSkinMesh* m_pParentSkin = nullptr;
    TMEffectBillBoard* m_pEnchant = nullptr;
    RDLVERTEX m_vertex[32];
    D3DXMATRIX m_matTemp[48];
    D3DXMATRIX m_matCombine[48];
    unsigned int m_dwIndices[48]{};
    unsigned int m_dwNumIndex = 0;
    D3DXMATRIX m_matRot[48];
    D3DXMATRIX m_matEffectCombine;
    D3DXMATRIX m_matEffectMat;
    int m_nNumTicks = 15;
    int m_nWeaponType = 0;
    int m_nCurrIndex = 0;
    int m_bDraw = 0;
    int m_nPrevIndex = 0;
    int m_nOldIndex = 0;
    float m_fEffectLength = 0.0f;
    float m_fWeaponLength = 0.0f;
    char m_cFireEffect = 0;
    char m_cMagicWeapon = 0;
    int m_bEnchant = 0;
    char m_cAssert = 0;
    char m_cGoldPiece = 0;
    char m_cArmorClass = 0;
    char m_cMixEffect = 0;
    char m_cSForce = 0;
    int m_nHandEffect = 0;
    unsigned int m_dwSwingID = 0;
    unsigned int m_dwSWTextureIndex = 221;
    int m_bHide = 0;
    unsigned int m_dwR = 255;
    unsigned int m_dwG = 255;
    unsigned int m_dwB = 255;
    // One clock per weapon emitter family (see swing_emitter::Clock).
    swing_emitter::Clock m_emitters;
    unsigned int m_dwOldMixTime = 0;
};