#pragma once
// Widget control; split from SControl.h/.cpp. Include "SControl.h" for the whole library.
#include "TreeNode.h"
#include "Structures.h"
#include "GeomObject.h"
#include "TMFont2.h"
#include "EventTranslator.h"

enum class CONTROL_TYPE : int
{
    CTRL_TYPE_NONE = -1,
    CTRL_TYPE_CURSOR = 0,
    CTRL_TYPE_PANEL = 1,
    CTRL_TYPE_BUTTON = 2,
    CTRL_TYPE_CHECKBOX = 3,
    CTRL_TYPE_RADIOBUTTON = 4,
    CTRL_TYPE_RADIOBUTTONSET = 5,
    CTRL_TYPE_LISTBOX = 6,
    CTRL_TYPE_LISTBOXITEM = 7,
    CTRL_TYPE_MESSAGEBOX = 8,
    CTRL_TYPE_MESSAGEPANEL = 9,
    CTRL_TYPE_PROGRESSBAR = 10,
    CTRL_TYPE_SCROLLBAR = 11,
    CTRL_TYPE_TEXT = 12,
    CTRL_TYPE_EDITABLETEXT = 13,
    CTRL_TYPE_DIALOG = 14,
    CTRL_TYPE_3DOBJ = 15,
    CTRL_TYPE_GRID = 16,
};

// CONTROL_TYPE is serialized as a 32-bit value in the RC stream.
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_NONE) == -1, "RC control NONE value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_CURSOR) == 0, "RC control CURSOR value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_PANEL) == 1, "RC control PANEL value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_BUTTON) == 2, "RC control BUTTON value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_CHECKBOX) == 3, "RC control CHECKBOX value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_RADIOBUTTON) == 4, "RC control RADIOBUTTON value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_RADIOBUTTONSET) == 5, "RC control RADIOBUTTONSET value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_LISTBOX) == 6, "RC control LISTBOX value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_LISTBOXITEM) == 7, "RC control LISTBOXITEM value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_MESSAGEBOX) == 8, "RC control MESSAGEBOX value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_MESSAGEPANEL) == 9, "RC control MESSAGEPANEL value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_PROGRESSBAR) == 10, "RC control PROGRESSBAR value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_SCROLLBAR) == 11, "RC control SCROLLBAR value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_TEXT) == 12, "RC control TEXT value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_EDITABLETEXT) == 13, "RC control EDITABLETEXT value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_DIALOG) == 14, "RC control DIALOG value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_3DOBJ) == 15, "RC control 3DOBJ value changed");
static_assert(static_cast<int>(CONTROL_TYPE::CTRL_TYPE_GRID) == 16, "RC control GRID value changed");

class IEventListener;
class SControl : public TreeNode
{
public:
    enum { TMC_NONE_ID };

    SControl(float inPosX, float inPosY, float inWidth, float inHeight);
    ~SControl();

    int OnPacketEvent(unsigned int dwCode, char* buf) override;
    int OnMouseEvent(unsigned int dwFlags, unsigned int wParam, int nX, int nY) override;
    int OnKeyDownEvent(unsigned int iKeyCode) override;
    int OnKeyUpEvent(unsigned int iKeyCode) override;
    int OnCharEvent(char iCharCode, int lParam) override;

    virtual int OnChangeIME();
    virtual int OnIMEEvent(char* ipComposeString);
    virtual int IsIMENative();

    virtual void SetControlID(unsigned int idwControlID);
    virtual unsigned int GetControlID();
    virtual unsigned int GetUniqueID();
    virtual void SetEventListener(IEventListener* ipEventListener);
    virtual void Update();
    virtual void FrameMove2(stGeomList* pDrawList, TMVector2 ivParentPos, int inParentLayer, int nFlag);

    virtual void SetAlwaysOnTop(int bAlwaysOnTop);
    virtual void SetVisible(int bVisible);
    virtual void SetEnable(int bEnable);
    virtual void SetFocused(int bFocused);

    int IsVisible();
    int IsFocused();
    int IsOver();
    TMVector2 GetPos();

    virtual int ChildCount();
    virtual void SetPos(float nPosX, float nPosY);
    virtual void SetSize(float nWidth, float nHeight);

    void SetRealPos(float nPosX, float nPosY);
    void SetRealSize(float nWidth, float nHeight);
    void SetAutoSize();
    void SetCenterSize();
    void SetStickLeft();
    void SetStickRight();
    void SetStickTop();
    void SetStickBottom();
    int PtInControl(int inPosX, int inPosY);

    virtual CONTROL_TYPE GetControlType();
    virtual void SetCenterPos(unsigned int dwControlID, float inPosX, float inPosY, float inWidth, float inHeight);

    void SetHeight(float nHeight);

public:
    static unsigned int m_dwStaticID;
    static int m_nGridCellSize;

public:
    int m_bAlwaysOnTop;
    int m_bVisible;
    int m_bEnable;
    int m_bFocused;
    int m_bOver;
    int m_bDeleteThisObject;
    int m_bSelectEnable;
    CONTROL_TYPE m_eCtrlType;
    unsigned int m_dwControlID;
    unsigned int m_dwUniqueID;
    float m_nPosX;
    float m_nPosY;
    float m_nWidth;
    float m_nHeight;
    IEventListener* m_pEventListener;
    int m_bModal;
    int m_cOver;
};

int PointInRect(int inPosX, int inPosY, float ifX, float ifY, float ifWidth, float ifHeight);
void RemoveRenderControlItem(stGeomList* pDrawList, GeomControl* pGeomControl, int nLayer);
int AddRenderControlItem(stGeomList* pDrawList, GeomControl* pGeomControl, int nLayer);
