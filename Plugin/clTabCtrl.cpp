#include "ColoursAndFontsManager.h"
#include "Notebook.h"
#include "clTabCtrl.h"
#include "clTabRendererClassic.h"
#include "clTabRendererCurved.h"
#include "clTabRendererSquare.h"
#if CL_BUILD
#include "editor_config.h"
#include "event_notifier.h"
#include "globals.h"
#include "ieditor.h"
#endif
#include <wx/dcbuffer.h>
#include <wx/regex.h>
#include <wx/wupdlock.h>

#define GET_WINDOW_PTR(T) reinterpret_cast<wxWindow*>(T->GetPagePtr())

// -------------------------------------------------------------------------------
// clTabCtrl class.
// This is where things are actually getting done
// -------------------------------------------------------------------------------
clTabCtrl::clTabCtrl(wxWindow* notebook, size_t style)
    : clTabCtrlBase(notebook, style)
{
}

clTabCtrl::~clTabCtrl() {}

WindowStack* clTabCtrl::GetStack() { return reinterpret_cast<Notebook*>(GetParent())->m_windows; }

wxWindow* clTabCtrl::GetPage(size_t index) const
{
    clTabInfo::Ptr_t tab = GetTabInfo(index);
    if(tab) return GET_WINDOW_PTR(tab);
    return NULL;
}

int clTabCtrl::FindPage(wxWindow* page) const
{
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(m_tabs[i]->GetPagePtr() == page) {
            return i;
        }
    }
    return wxNOT_FOUND;
}

int clTabCtrl::GetPageByWin(wxWindow* win) const
{
    if(!win) return wxNOT_FOUND;
    for(size_t i = 0; i < m_tabs.size(); ++i) {
        if(m_tabs.at(i)->GetPagePtr() == win) return i;
    }
    return wxNOT_FOUND;
}

void clTabCtrl::GetAllPages(std::vector<wxWindow*>& pages)
{
    std::for_each(m_tabs.begin(), m_tabs.end(), [&](clTabInfo::Ptr_t tabInfo) {
        wxWindow* win = GET_WINDOW_PTR(tabInfo);
        if(win) {
            pages.push_back(win);
        }
    });
}

void clTabCtrl::AllPagesDeleted() { GetStack()->Clear(); }

void clTabCtrl::NotifyPageChanged(int oldIndex, int newIndex)
{
    // Fire an event
    clTabCtrlBase::NotifyPageChanged(oldIndex, newIndex);

    clTabInfo::Ptr_t activeTab = GetActiveTabInfo();
    if(activeTab) {
        wxWindow* win = GET_WINDOW_PTR(activeTab);
        if(win) {
            GetStack()->Select(win);
            win->CallAfter(&wxWindow::SetFocus);
        }
    }
}

void clTabCtrl::BeforePageChange(clTabInfo::Ptr_t tab)
{
    clTabCtrlBase::BeforePageChange(tab);
    GetStack()->Add(GET_WINDOW_PTR(tab), tab->IsActive());
}

void clTabCtrl::PageRemoved(clTabInfo::Ptr_t tab, bool deleteIt)
{
    wxWindow* w = GET_WINDOW_PTR(tab);
    GetStack()->Remove(w);
    if(deleteIt) {
        // Destory the page
        w->Destroy();
    } else {
        // Just hide it
        w->Hide();
    }
}
