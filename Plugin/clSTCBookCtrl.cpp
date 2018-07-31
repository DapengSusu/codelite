#include "clSTCBookCtrl.h"
#include "clSTCEventsHandler.h"
#include "clSTCTabCtrl.h"
#include "clTabRenderer.h"
#include <wx/sizer.h>

clSTCBookCtrl::clSTCBookCtrl(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
    , m_defaultPage(nullptr)
{
    m_tabCtrl = new clSTCTabCtrl(this, kNotebook_Default | kNotebook_AllowDnD);

    SetSizer(new wxBoxSizer(wxVERTICAL));

    m_stc = new wxStyledTextCtrl(this, wxID_ANY);
    GetSizer()->Add(m_tabCtrl, 0, wxEXPAND);
    GetSizer()->Add(m_stc, 1, wxEXPAND);
    m_stc->SetClientData(nullptr);
}

clSTCBookCtrl::~clSTCBookCtrl() { m_tabCtrl->DeleteAllPages(); }

void clSTCBookCtrl::AddPage(clSTCEventsHandler* handler, const wxFileName& filename, const wxString& label,
                            bool selected, const wxBitmap& bmp)
{
    if(!m_stc->IsShown()) { m_stc->Show(); }
    handler->SetFilename(filename);
    m_tabCtrl->AddPage(clTabInfo::Ptr_t(new clTabInfo(m_tabCtrl, m_tabCtrl->GetStyle(), (void*)handler, label, bmp)));
    if(selected) { SetSelection(m_tabCtrl->GetPageCount() - 1); }
}

void clSTCBookCtrl::ChangeSelection(clSTCEventsHandler* newHandler)
{
    // This method does the following:
    // 1. Attaches the handler's document pointer to wxSTC
    // 2. If there is an old handler attached to the wxSTC, Unbind wxSTC events from the old handler
    // 3. Bind wxSTC events to the new handler
    // 4. Attaches the new handler to the wxSTC
    newHandler->SelectIntoEditor();
    m_stc->CallAfter(&wxStyledTextCtrl::SetFocus);
}

void clSTCBookCtrl::SetSelection(size_t index)
{
    clSTCEventsHandler* handler = GetHandler(index);
    if(!handler) return;
    if(m_tabCtrl->SetSelection(index)) { ChangeSelection(handler); }
}

clSTCEventsHandler* clSTCBookCtrl::GetHandler(int index) const
{
    if(index == wxNOT_FOUND) { index = m_tabCtrl->GetSelection(); }
    clTabInfo::Ptr_t tab = m_tabCtrl->GetTabInfo(index);
    if(!tab) { return nullptr; }
    return tab->GetPtrAs<clSTCEventsHandler>();
}
clSTCEventsHandler* clSTCBookCtrl::GetActiveHandler() const { return GetHandler(wxNOT_FOUND); }

int clSTCBookCtrl::GetSelection() const { return m_tabCtrl->GetSelection(); }

void clSTCBookCtrl::ChangeSelection(size_t index)
{
    clSTCEventsHandler* handler = GetHandler(index);
    if(!handler) return;
    if(m_tabCtrl->ChangeSelection(index)) { ChangeSelection(handler); }
}

void clSTCBookCtrl::SetPageText(int index, const wxString& label) { m_tabCtrl->SetPageText(index, label); }

wxString clSTCBookCtrl::GetPageText(int index) const { return m_tabCtrl->GetPageText(index); }

void clSTCBookCtrl::DeleteAllPages()
{
    m_tabCtrl->DeleteAllPages();
    m_stc->ClearAll();
    m_stc->Hide();
}

void clSTCBookCtrl::SetDefaultPage(wxWindow* page)
{
    wxWindowUpdateLocker locker(this);
    if(m_defaultPage == page) { return; }

    bool needToShow = false;

    // 1. If the current default page is displayed, detach it from the view
    if(m_defaultPage && GetSizer()->GetItem(m_defaultPage)) {
        // The current item is visible
        needToShow = true;
        GetSizer()->Detach(m_defaultPage);
    }

    // 2. If we have a default view -> destory it
    if(m_defaultPage) {
        m_defaultPage->Destroy();
        m_defaultPage = nullptr;
    }

    // 3. Replace the default page
    m_defaultPage = page;

    if(needToShow) {
        DoShowPage(m_defaultPage, true, 1);
    } else if(m_defaultPage) {
        m_defaultPage->Hide();
    }
}

void clSTCBookCtrl::DisplayDefaultPage(bool show)
{
    if(!m_defaultPage) return;
    if(show) {
        DoShowPage(m_defaultPage, true, 1);
        DoShowPage(m_tabCtrl, false, wxNOT_FOUND);
        DoShowPage(m_stc, false, wxNOT_FOUND);
    } else {
        DoShowPage(m_defaultPage, false, wxNOT_FOUND);
        DoShowPage(m_tabCtrl, true, 0);
        DoShowPage(m_stc, true, 1);
    }
}

void clSTCBookCtrl::DoShowPage(wxWindow* win, bool show, int proportion)
{
    if(!win) { return; }
    if(show) {
        if(!GetSizer()->GetItem(win)) {
            // Attach it to the sizer
            GetSizer()->Add(win, proportion, wxEXPAND);
        }
        if(!win->IsShown()) { win->Show(); }
    } else {
        // Hide it
        if(GetSizer()->GetItem(win)) { GetSizer()->Detach(win); }
        win->Hide();
    }
}

void clSTCBookCtrl::SetStyle(size_t style) { m_tabCtrl->SetStyle(style); }
