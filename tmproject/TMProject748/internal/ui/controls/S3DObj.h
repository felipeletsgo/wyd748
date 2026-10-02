#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SControlBase.h"

class S3DObj : public SControl
{
public:
    S3DObj(int nObjIndex, float inX, float inY, float inWidth, float inHeight);
    ~S3DObj();
    virtual void SetObjIndex(int nObjIndex);
    virtual GeomControl* GetGeomControl();

    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;

public:
    GeomControl m_GCObj;
};
