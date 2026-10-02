#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "SPanel.h"
#include "SText.h"
#include "SEditableText.h"
#include "SProgressBar.h"
#include "SScrollBar.h"

class SListBox;
class SListBoxItem : public SText
{
public:
    SListBoxItem(const char* istrText, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight,
        int ibBorder, unsigned int idwBorderColor, unsigned int dwType, unsigned int dwAlignType);
    ~SListBoxItem();
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag) override;
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
public:
    // WYD.exe 7.48 appends only this background-mode flag to SText. Keeping
    // later list-item state here changes the native widget contract and makes
    // server/channel selection depend on a texture that 7.48 never creates.
    int m_bBGColor;
};

class SListBoxBoardItem : public SListBoxItem
{
public:
    SListBoxBoardItem(char* szIndex, char* szVIndex, char* szTitle, char* szWriter, char* szCount, char* szDate, unsigned int dwColor, int bTitile);
    ~SListBoxBoardItem();
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag) override;
    void SetPos(float nPosX, float nPosY) override;

public:
    SText* m_pTitleText;
    SText* m_pWriterText;
    SText* m_pCountText;
    SText* m_pDateText;
    char m_szIndex[256];
};

class SListBoxPartyItem : public SListBoxItem
{
public:
    SListBoxPartyItem(char* iStrText, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight,
        unsigned int dwCharID, int nClass, int nLevel, int nHp, int nMaxHp);
    ~SListBoxPartyItem();

    void FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag) override;

public:
    unsigned int m_dwCharID;
    int m_nClass;
    int m_nLevel;
    int m_nState;
    SText* m_pLevelText;
    SPanel* m_pDirPanel;
    SProgressBar* m_pHpProgress;
};

class SListBoxServerItem : public SListBoxItem
{
public:
    SListBoxServerItem(int nTextureSet, char* iStrText, unsigned int idwFontColor, float inX, float inY, float inWidth, float inHeight,
        int nCount, char cCastle, char cGoldBug, int Num);
    ~SListBoxServerItem();
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivItemPos, int inParentLayer, int nFlag) override;

public:
    SProgressBar* m_pBusyProgress;
    SPanel* m_pCrownPanel;
    SPanel* m_pGoldBugPanel;
    SPanel* m_pAgePanel;
    int m_nCurrent;
    char m_cConnected;
    char m_cCastle;
    char m_cGoldBug;
};

class SListBox : public SPanel, public IEventListener
{
public:
    enum {
        TMC_SCROLL_BAR = 0x1,
        TMC_EDITBOX = 0x2,
    };

public:
    SListBox(int inTextureSetIndex, int inMaxCount, int inVisibleCount, float inX, float inY,
        float inWidth, float inHeight, unsigned int idwColor, RENDERCTRLTYPE eRenderType,
        int bSelectEnable, int bScrollBar, int bEditable);
    ~SListBox();

    int AddItem(SListBoxItem* ipNewItem);
    int DeleteItem(int inItemIndex);
    int DeleteItem(SListBoxItem* ipItem);
    SListBoxItem* GetItem(int inItemIndex);
    void Empty();
    void SetStartItemIndex(int nIndex);
    int GetSelectedIndex();
    void SetSelectedIndex(int nIndex);
    void SetSize(float nWidth, float nHeight) override;
    void SetPickSize(float nWidth, float nHeight);
    void SetEditable();
    int OnControlEvent(DWORD idwControlID, DWORD idwEvent) override;
    void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag) override;
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;

public:
    char m_cScrollBar;
    char m_cEditable;
    short m_sEditLine;
    int m_bRButton;
    int m_nMaxCount;
    int m_nVisibleCount;
    int m_nStartItemIndex;
    int m_nSelectedItem;
    int m_nHoverItem;
    float m_fPickWidth;
    float m_fPickHeight;
    SEditableText* m_pEditLine;
    SScrollBar* m_pScrollBar;
    int m_nNumItem;
    SListBoxItem* m_pItemList[1000];
};
