#include "clSTCEventsHandler.h"
#include "clSTCTabCtrl.h"

clSTCTabCtrl::clSTCTabCtrl(wxWindow* notebook, size_t style)
    : clTabCtrlBase(notebook, style)
{
}

clSTCTabCtrl::~clSTCTabCtrl() {}

void clSTCTabCtrl::AllPagesDeleted()
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        clSTCEventsHandler* handler = reinterpret_cast<clSTCEventsHandler*>(m_tabs[i]->GetPagePtr());
        if(handler) {
            wxDELETE(handler);
            m_tabs[i]->SetPagePtr(nullptr);
        }
    }
}

void clSTCTabCtrl::NotifyPageChanged(int oldIndex, int newIndex)
{
    clTabCtrlBase::NotifyPageChanged(oldIndex, newIndex);
    clTabInfo::Ptr_t tabInfo = GetTabInfo(newIndex);
    if(tabInfo) {
        clSTCEventsHandler* handler = tabInfo->GetPtrAs<clSTCEventsHandler>();
        if(handler) {
            handler->SelectIntoEditor();
        }
    }
}

void clSTCTabCtrl::PageRemoved(clTabInfo::Ptr_t tab, bool deleteIte)
{
    clTabCtrlBase::PageRemoved(tab, deleteIte);
    // We always delete the window
    clSTCEventsHandler* handler = tab->GetPtrAs<clSTCEventsHandler>();
    wxDELETE(handler);
    tab->SetPagePtr(nullptr);
}
