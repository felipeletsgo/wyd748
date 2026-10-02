#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"

class SReelPanel : public SPanel
{
public:
    SReelPanel(unsigned int inTextureSetIndex, float inX, float inY, float inSizeX, float inSizeY, float inPitch);
    ~SReelPanel();

    void SetRoll(bool bRoll, int StopPos1, int StopPos2, int StopPos3, unsigned int StopTime);
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    void SetVisible(int bVisible) override;
    void SetResult(char cResult);
    void UpDateJackpot();

public:
    SPanel* m_pGamBleReel[3][22];
    SPanel* m_pGamBleBox[2][9];
    SPanel* m_pJACKPOTCOUNT[10];
    SPanel* m_pJACKPOTBG;
    bool m_bGamBleBox[9];
    bool m_bRoling;
    int m_RollPos[3];
    int m_StopPos[3];
    unsigned int m_dwOldServerTime;
    unsigned int m_dwStopTime;
    int m_bResult;
    unsigned int m_dwJackpot;
    unsigned int m_dwJackPotView;
    unsigned int m_dwAutoTime;
    unsigned int m_dwBoxAniTime;
    int m_nPresent;
    TMVector2 m_vSize;
    int m_nPitch;
    unsigned int m_dwBatCoin;
};
