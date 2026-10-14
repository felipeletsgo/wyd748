#pragma once

#include "TMItem.h"

enum class EGATE_STATE
{
    // Stable gate states and the timed transitions of its animation.
    EGATE_OPEN = 1,
    EGATE_CLOSED = 2,
    EGATE_LOCKED = 3,
    EGATE_OPENING = 4,
    EGATE_CLOSING = 5,
    EGATE_LOCKING = 6,
};

class TMEffectBillBoard2;

// World item specialised for gates built from left and right halves. Whether
// a gate may open still depends on the server state.
class TMGate : public TMItem
{
public:
    TMGate();
    ~TMGate();

    // Copies the base item and derives the indices/sizes used to assemble it.
    virtual void InitGate(STRUCT_ITEM stItem);
    int Render() override;
    int FrameMove(unsigned int dwServerTime) override;
    // Starts or completes the visual transition to the given state.
    virtual void SetState(EGATE_STATE eState);
    int IsMouseOver() override;

public:
    // The special effect belongs to this instance and may be null. Mesh
    // indices do not take ownership of globally loaded resources.
    TMEffectBillBoard2* m_pEffectSpecial;
    float m_fOpenAngle;
    EGATE_STATE m_eState;
    unsigned int m_dwLastSetTime;
    short m_sItemIndex;
    int m_nLeftIndex;
    int m_nRightIndex;
    float m_fWidth;
    short m_sAuth;
    short m_sType;
    float m_StdMinus;
    float m_StdPlus;
    float m_RotMinus;
    float m_RotPlus;
    // Collision height last received for the gate mask (MSG_CreateItem /
    // MSG_UpdateItem); -1 until known. Reapplied whenever the scene rebuilds
    // its height window, so a gate received before a warp keeps blocking.
    int m_nMaskHeight = -1;
};
