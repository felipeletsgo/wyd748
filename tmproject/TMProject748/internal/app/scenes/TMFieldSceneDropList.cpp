#include "pch.h"
#include "TMFieldScene.h"
#include "SControl.h"
#include "SControlContainer.h"
#include "TMGlobal.h"
#include "TMUtil.h"
#include "TMHuman.h"
#include "TMItem.h"
#include "../../application/SceneSafetyPolicy.h"
#include <string>
#include "../../application/DropSearchText.h"
#include "../../application/DropSearchLayout.h"

namespace {
constexpr unsigned int ControlBase = drop_search_layout::PanelID;
class DropButton final : public SButton {
public:
    using SButton::SButton;
    void SetBackground(unsigned int color) {
        m_dwColor=m_GCPanel.dwColor=color;
        Update();
    }
};
char* Text(int row, const char* fallback) { return const_cast<char*>(ui_text::RowOr(g_UIString,row,fallback)); }
SText* Label(SPanel* parent, const char* text, float x,float y,float width,unsigned int color=0xFFFFFFFF) {
    auto result=new SText(-1,text,color,x,y,width,16,0,0,0,SText::TEXT_ALIGN_NOMARGINE);
    result->m_bSelectEnable=0;
    parent->AddChild(result);return result;
}
void SetButtonEnabled(SControlContainer* container,unsigned int id,bool enabled) {
    if(auto button=static_cast<SButton*>(container->FindControl(id))) {
        button->SetEnable(enabled?1:0);
        for(auto node=button->m_pDown;node;node=node->m_pNextLink)
            if(auto label=dynamic_cast<SText*>(node)) label->SetTextColor(enabled?0xFFFFFFFFu:0xFF777777u);
        button->Update();
    }
}

}

bool TMFieldScene::IsDropSearchVisible() const {
    return m_pDropListPanel && m_pDropListPanel->IsVisible() == 1;
}

void TMFieldScene::ToggleDropSearch() {
    if(!m_pControlContainer || !m_pMyHuman) return;
    if(m_pDropListPanel && m_pDropListPanel->IsVisible()) {CloseDropSearch();return;}
    for(auto modal:m_pControlContainer->m_pModalControl)
        if(modal && modal!=m_pDropListPanel && modal->m_bVisible && modal->m_bModal) return;
    float topHudBottom=0.0f, bottomHudTop=static_cast<float>(g_pDevice->m_dwScreenHeight);
    for(unsigned int id:{5723u,289u})
        if(auto hud=m_pControlContainer->FindControl(id); hud && hud->IsVisible())
            topHudBottom=(std::max)(topHudBottom,hud->m_nPosY+hud->m_nHeight);
    for(unsigned int id:{5716u,5745u,12624u})
        if(auto hud=m_pControlContainer->FindControl(id); hud && hud->IsVisible())
            bottomHudTop=(std::min)(bottomHudTop,hud->m_nPosY);
    const auto placement=drop_search_layout::Place(static_cast<float>(g_pDevice->m_dwScreenWidth),
        static_cast<float>(g_pDevice->m_dwScreenHeight),RenderDevice::m_fUIScaleY,topHudBottom,bottomHudTop);
    if(!placement.fits) {
        if(m_pMessagePanel) {
            m_pMessagePanel->SetMessage(Text(290,"Reduce UI scale in Settings to open the drop list."),4000);
            m_pMessagePanel->SetVisible(1,1);
        }
        return;
    }
    if(!m_pDropListPanel) {
        // FieldScene2 control 12848 uses the existing quest-window frame.
        auto skin=dynamic_cast<SPanel*>(m_pControlContainer->FindControl(12848));
        if(!skin) return;
        auto panel=new SPanel(skin->m_GCPanel.nTextureSetIndex,0,0,
            drop_search_layout::Width,drop_search_layout::Height,0xFFFFFFFF,RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
        panel->SetControlID(ControlBase);panel->m_bModal=1;panel->m_bAlwaysOnTop=1;
        m_pControlContainer->m_pControlRoot->AddChild(panel);
        m_pDropListPanel=panel;
        auto title=Label(panel,Text(267,"Drop list (D)"),16,4,588,0xFFFFFFAA);
        title->m_dwAlignType=SText::TEXT_ALIGN_CENTER;
        auto button=[&](unsigned int id,const char* caption,float x,float y,float width) {
            auto control=new DropButton(-2,x,y,width,20,0x503A3935,1,nullptr);
            control->SetControlID(ControlBase+id);control->SetEventListener(m_pControlContainer);panel->AddChild(control);
            auto label=Label(control,caption,0,2,width);
            label->m_dwAlignType=SText::TEXT_ALIGN_CENTER;
            return label;
        };
        m_dropModeLabels[0]=button(1,Text(268,"Monsters"),16,31,90);
        m_dropModeLabels[1]=button(2,Text(269,"Items"),110,31,70);
        m_dropSearchEdit=new SEditableText(-2,"",63,0,0xFFFFFFFF,190,31,280,20,1,0xFF333333,0,0);
        m_dropSearchEdit->SetControlID(ControlBase+3);m_dropSearchEdit->SetEventListener(m_pControlContainer);panel->AddChild(m_dropSearchEdit);
        button(4,Text(270,"Search"),482,31,120);
        m_dropSearchHint=Label(panel,Text(285,"Search monsters by name."),16,56,384,0xFFCCCCCC);
        m_dropSearchBonus=Label(panel,Text(288,"Drop bonus: loading"),410,56,192,0xFFFFFFAA);
        const float x[4]={16,234,414,506};const float width[4]={210,172,84,96};
        const int captions[4]={271,272,273,274};const char* fallback[4]={"Item","Monster","Base chance","With bonus"};
        for(int col=0;col<4;++col) Label(panel,Text(captions[col],fallback[col]),x[col],76,width[col],0xFFFFFFAA);
        for(int row=0;row<10;++row) {
            auto stripe=new SPanel(-2,14,96+16.0f*row,590,16,row%2?0x503A3935:0x50202020,RENDERCTRLTYPE::RENDER_IMAGE_STRETCH);
            stripe->m_bSelectEnable=0;
            panel->AddChild(stripe);
            for(int col=0;col<4;++col) m_dropSearchRows[row][col]=Label(panel,"",x[col],96+16.0f*row,width[col]);
        }
        button(5,Text(275,"Previous"),16,262,100);
        button(6,Text(276,"Next"),124,262,100);
        SetButtonEnabled(m_pControlContainer,ControlBase+5,false);
        SetButtonEnabled(m_pControlContainer,ControlBase+6,false);
        const auto close=drop_search_layout::CloseButton;
        auto closeLabel=button(7,Text(277,"Close"),close.x,close.y,close.width);
        closeLabel->SetPos(0.0f,(close.height-16.0f)*RenderDevice::m_fUIScaleY*0.5f);
        auto closeControl=static_cast<DropButton*>(m_pControlContainer->FindControl(ControlBase+7));
        closeControl->SetSize(close.width*RenderDevice::m_fUIScaleX,close.height*RenderDevice::m_fUIScaleY);
        closeControl->SetBackground(0);
        m_dropSearchStatus=Label(panel,"",234,264,368,0xFFFFFFAA);
        m_dropSearchMessage=Label(panel,"",16,176,588,0xFFFFFFAA);
        m_dropSearchMessage->m_dwAlignType=SText::TEXT_ALIGN_CENTER;
        m_dropSearchMessage->SetVisible(0);
        Label(panel,Text(282,"Chance per eligible kill; combines independent item rolls."),16,284,588,0xFFCCCCCC);
        Label(panel,Text(283,"Your bonus affects normal rates; fixed rewards do not change."),16,300,588,0xFFCCCCCC);
        Label(panel,Text(284,"* Event/boss rules apply; EXP-only areas have no drops."),16,316,588,0xFFCCCCCC);
    }
    m_pDropListPanel->SetPos(placement.panel.x,placement.panel.y);
    m_dropPreviousModal=m_pControlContainer->m_pModalControl[0];
    m_pControlContainer->m_pModalControl[0]=m_pDropListPanel;
    m_pDropListPanel->SetVisible(1);
    // UI geometry is queued before entity Render; hide existing labels on opening too.
    for(auto node=m_pHumanContainer?m_pHumanContainer->m_pDown:nullptr;node;node=node->m_pNextLink)
        if(auto human=dynamic_cast<TMHuman*>(node)) human->HideLabel();
    for(auto node=m_pItemContainer?m_pItemContainer->m_pDown:nullptr;node;node=node->m_pNextLink)
        if(auto item=dynamic_cast<TMItem*>(node)) item->LabelPosition();
    m_pControlContainer->SetFocusedControl(m_dropSearchEdit);
    SearchDrops(0);
}

void TMFieldScene::SetDropSearchFeedback(int row,const char* fallback) {
    if(m_dropSearchStatus) m_dropSearchStatus->SetText(const_cast<char*>(""),0);
    if(m_dropSearchMessage) {
        m_dropSearchMessage->SetText(Text(row,fallback),0);
        m_dropSearchMessage->SetVisible(1);
    }
}

void TMFieldScene::CloseDropSearch() {
    if(!m_pDropListPanel || !m_pControlContainer) return;
    m_pDropListPanel->SetVisible(0);m_dropPending.Reset();m_dropRequestTime=0;m_dropDeferred=false;
    if(m_pControlContainer->m_pModalControl[0]==m_pDropListPanel)
        m_pControlContainer->m_pModalControl[0]=m_dropPreviousModal;
    m_dropPreviousModal=nullptr;
    if(m_pControlContainer->m_pFocusControl==m_dropSearchEdit) {
        // SetFocusedControl(nullptr) refuses release in the alternate keyboard mode.
        m_dropSearchEdit->SetFocused(0);
        m_pControlContainer->m_pFocusControl=nullptr;
    }
}

void TMFieldScene::SearchDrops(unsigned int offset) {
    if(!m_pDropListPanel || !m_pDropListPanel->IsVisible() || !m_dropSearchEdit || !m_pMyHuman) return;
    for(unsigned int mode=0;mode<2;++mode) {
        if(auto button=static_cast<DropButton*>(m_pControlContainer->FindControl(ControlBase+1+mode))) {
            button->SetSelected(m_dropMode==mode);
            button->SetBackground(m_dropMode==mode?0x90605A35u:0x503A3935u);
            if(m_dropModeLabels[mode]) m_dropModeLabels[mode]->SetTextColor(m_dropMode==mode?0xFFFFFFAAu:0xFFCCCCCCu);
            button->Update();
        }
    }
    if(m_dropSearchHint)
        m_dropSearchHint->SetText(Text(m_dropMode==0?285:286,m_dropMode==0?"Search monsters by name.":"Search items by name or ID."),0);
    SetDropSearchFeedback(278,"Loading...");
    if(m_dropSearchBonus) m_dropSearchBonus->SetText(Text(288,"Drop bonus: loading"),0);
    m_dropTotal=0;
    for(auto& row:m_dropSearchRows) for(auto cell:row) cell->SetText(const_cast<char*>(""),0);
    SetButtonEnabled(m_pControlContainer,ControlBase+5,false);
    SetButtonEnabled(m_pControlContainer,ControlBase+6,false);
    const auto now=timeGetTime();
    if(m_dropRequestTime && now-m_dropRequestTime<300) {
        m_dropPending.Reset();m_dropDeferred=true;m_dropDeferredOffset=offset;return;
    }
    m_dropDeferred=false;
    drop_list::Query query{};
    query.Header.Size=sizeof(query);query.Header.Type=drop_list::QueryOpcode;query.Header.ID=m_pMyHuman->m_dwID;
    query.Version=1;query.Mode=m_dropMode;query.ID=m_dropPending.Start(query.Mode);query.Offset=offset;
    strncpy_s(query.Text,m_dropSearchEdit->GetText(),_TRUNCATE);
    if(m_dropMode==1 && query.Text[0]) {
        const auto key=drop_search::Key(query.Text);
        for(unsigned int id=1;id<6500;++id) {
            if(drop_search::Key(g_pItemList[id].Name).find(key)!=std::string::npos || std::to_string(id)==key)
                query.Items[id/8]|=1u<<(id%8);
        }
    }
    m_dropRequestTime=now;m_dropOffset=offset;
    SendOneMessage(reinterpret_cast<char*>(&query),sizeof(query));
}

int TMFieldScene::DropSearchControl(unsigned int id) {
    if(!m_pDropListPanel || !m_pDropListPanel->IsVisible()) return 1;
    switch(id-ControlBase) {
    case 1:m_dropMode=0;SearchDrops(0);break;
    case 2:m_dropMode=1;SearchDrops(0);break;
    case 3:case 4:SearchDrops(0);break;
    case 5:SearchDrops(m_dropOffset>=10?m_dropOffset-10:0);break;
    case 6:if(m_dropOffset+10<m_dropTotal) SearchDrops(m_dropOffset+10);break;
    case 7:CloseDropSearch();break;
    }
    return 1;
}

void TMFieldScene::UpdateDropSearch() {
    if(m_dropDeferred && (!m_dropRequestTime || timeGetTime()-m_dropRequestTime>=300))
        SearchDrops(m_dropDeferredOffset);
    if(m_dropPending.IsPending() && m_dropRequestTime && timeGetTime()-m_dropRequestTime>7000) {
        m_dropPending.Reset();m_dropRequestTime=0;
        if(m_dropSearchStatus && m_pDropListPanel && m_pDropListPanel->IsVisible()) {
            SetDropSearchFeedback(281,"Server did not answer. Search again.");
            if(m_dropSearchBonus) m_dropSearchBonus->SetText(Text(289,"Drop bonus unavailable."),0);
        }
    }
}

int TMFieldScene::OnDropSearchResult(const void* bytes,unsigned int size) {
    drop_list::Result result{};
    if(!drop_list::Parse(bytes,size,result) || !m_pDropListPanel || !m_pDropListPanel->IsVisible() ||
        result.Mode!=m_dropMode || !m_dropPending.Consume(result)) return 0;
    m_dropPending.Reset();m_dropOffset=result.Offset;m_dropTotal=result.Total;
    if(result.Status) {
        SetDropSearchFeedback(280,"Search again in a moment.");
        if(m_dropSearchBonus) m_dropSearchBonus->SetText(Text(289,"Drop bonus unavailable."),0);
        return 1;
    }
    if(m_dropSearchBonus) {
        char bonus[64]{};
        sprintf_s(bonus,"%s %+d%%",Text(287,"Drop bonus:"),result.Bonus);
        m_dropSearchBonus->SetText(bonus,0);
    }
    for(unsigned int i=0;i<result.Count;++i) {
        const auto& row=result.Rows[i];char name[64]{};
        const char* item=g_pItemList[row.Item].Name;
        if(!item[0]) sprintf_s(name,"#%u",row.Item);
        else {strncpy_s(name,item,_TRUNCATE);for(char& c:name) if(c=='_') c=' ';}
        m_dropSearchRows[i][0]->SetText(name,0);
        char monster[64]{};strncpy_s(monster,row.Monster,_TRUNCATE);
        if(row.Flags&14u) strcat_s(monster," *");
        m_dropSearchRows[i][1]->SetText(monster,0);
        char chance[32]{};sprintf_s(chance,"%.5g%%",100.0*row.Base/drop_list::ChanceScale);
        m_dropSearchRows[i][2]->SetText(chance,0);
        sprintf_s(chance,"%.5g%%",100.0*row.Current/drop_list::ChanceScale);
        m_dropSearchRows[i][3]->SetText(chance,0);
    }
    char status[128]{};
    if(!result.Count) SetDropSearchFeedback(279,"No results. Check the search term and selected tab.");
    else {
        if(m_dropSearchMessage) m_dropSearchMessage->SetVisible(0);
        sprintf_s(status,"%u-%u / %u",result.Offset+1,result.Offset+result.Count,result.Total);
        m_dropSearchStatus->SetText(status,0);
    }
    SetButtonEnabled(m_pControlContainer,ControlBase+5,result.Count>0 && result.Offset>0);
    SetButtonEnabled(m_pControlContainer,ControlBase+6,result.Count>0 && result.Offset+result.Count<result.Total);
    return 1;
}
