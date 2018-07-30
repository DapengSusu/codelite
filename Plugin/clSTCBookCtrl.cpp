#include "clSTCBookCtrl.h"
#include "clSTCEventsHandler.h"
#include <wx/sizer.h>

wxDEFINE_EVENT(wxEVT_STC_BOOK_PAGE_CHANGING, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_STC_BOOK_PAGE_CHANGED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_STC_BOOK_PAGE_CLOSING, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_STC_BOOK_PAGE_CLOSED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_STC_BOOK_PAGE_CLOSE_BUTTON, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_STC_BOOK_TAB_DCLICKED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_STC_BOOK_TABAREA_DCLICKED, wxBookCtrlEvent);
wxDEFINE_EVENT(wxEVT_STC_BOOK_TAB_CONTEXT_MENU, wxBookCtrlEvent);

clSTCBookCtrl::clSTCBookCtrl(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
{
    SetSizer(new wxBoxSizer(wxVERTICAL));
    m_stc = new wxStyledTextCtrl(this, wxID_ANY);
    GetSizer()->Add(m_stc, 1, wxEXPAND);
    m_stc->SetClientData(nullptr);
}

clSTCBookCtrl::~clSTCBookCtrl() {}

void clSTCBookCtrl::AddPage(clSTCEventsHandler* handler, const wxString& label, bool selected, const wxBitmap& bmp)
{
    if(m_handlersMap.count(handler->GetFilename().GetFullPath()) == 0) {
        // a new file, add it
        m_handlersMap.insert({ handler->GetFilename().GetFullPath(), handler });
        m_handlersVec.push_back(handler);
    }

    if(selected) {
        SetSelection(handler);
    }
}

clSTCEventsHandler* clSTCBookCtrl::GetActiveHandler() const
{
    if(!m_stc->GetClientData()) {
        return nullptr;
    }

    clSTCEventsHandler* handler = reinterpret_cast<clSTCEventsHandler*>(m_stc->GetClientData());
    return handler;
}

void clSTCBookCtrl::ChangeSelection(clSTCEventsHandler* newHandler)
{
    // This method does the following:
    // 1. Attaches the handler's document pointer to wxSTC
    // 2. If there is an old handler attached to the wxSTC, Unbind wxSTC events from the old handler
    // 3. Bind wxSTC events to the new handler
    // 4. Attaches the new handler to the wxSTC
    newHandler->SelectIntoEditor();
}

void clSTCBookCtrl::SetSelection(size_t index) { SetSelection(GetHandler(index)); }

clSTCEventsHandler* clSTCBookCtrl::GetHandler(int index)
{
    if(index == wxNOT_FOUND) {
        return GetActiveHandler();
    }
    if(index >= m_handlersVec.size() || index < 0) {
        return nullptr;
    }
    return m_handlersVec[index];
}

void clSTCBookCtrl::SetSelection(clSTCEventsHandler* handler)
{
    if(!handler) {
        return;
    }

    int oldSelection = GetSelection();
    if(oldSelection != wxNOT_FOUND) {
        wxBookCtrlEvent changingEvent(wxEVT_STC_BOOK_PAGE_CHANGING);
        changingEvent.SetEventObject(this);
        changingEvent.SetSelection(oldSelection);
        changingEvent.SetOldSelection(wxNOT_FOUND);
        GetEventHandler()->ProcessEvent(changingEvent);
        if(!changingEvent.IsAllowed()) {
            return; // Vetoed by the user
        }
    }

    ChangeSelection(handler);

    {
        wxBookCtrlEvent changedEvent(wxEVT_STC_BOOK_PAGE_CHANGED);
        changedEvent.SetEventObject(this);
        changedEvent.SetSelection(GetSelection());
        changedEvent.SetOldSelection(oldSelection);
        GetEventHandler()->AddPendingEvent(changedEvent);
    }
}

int clSTCBookCtrl::GetSelection() const
{
    clSTCEventsHandler* handler = GetActiveHandler();
    if(!handler) {
        return wxNOT_FOUND;
    }
    for(size_t i = 0; i < m_handlersVec.size(); ++i) {
        if(m_handlersVec[i] == handler) {
            return (int)i;
        }
    }
    return wxNOT_FOUND;
}
