#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"

class SCheckBox : public SPanel
{
public:
    enum {
        TMC_IMAGE_UNCHECKED = 0x0,
        TMC_IMAGE_CHECKED = 0x1,
    };
    enum {
        TMC_CHECKBOX_CHANGED = 0x0,
    };

public:
    SCheckBox(unsigned int inTextureSetIndex, float inX, float inY, float inWidth, float inHeight, unsigned int dwColor);
    ~SCheckBox();

    void SetValue(int ibValue);
    int GetValue();
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
    void Update() override;

protected:
    int m_bValue;
    int m_bOver;
    unsigned int m_dwSelectedColor;
    unsigned int m_dwUnSelectedColor;
};
