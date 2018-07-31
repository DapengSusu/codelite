#include "clSTCBookCtrl.h"
#include "clSTCEventsHandler.h"
#include "clSTCTabCtrl.h"
#include "clTabRenderer.h"
#include <wx/sizer.h>


clSTCBookCtrl::clSTCBookCtrl(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
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
    handler->SetFilename(filename);
    m_tabCtrl->AddPage(clTabInfo::Ptr_t(new clTabInfo(m_tabCtrl, m_tabCtrl->GetStyle(), (void*)handler, label, bmp)));
    if(selected) {
        SetSelection(m_tabCtrl->GetPageCount() - 1);
    }
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
    if(m_tabCtrl->SetSelection(index)) {
        ChangeSelection(handler);
    }
}

clSTCEventsHandler* clSTCBookCtrl::GetHandler(int index) const
{
    if(index == wxNOT_FOUND) {
        index = m_tabCtrl->GetSelection();
    }
    clTabInfo::Ptr_t tab = m_tabCtrl->GetTabInfo(index);
    if(!tab) {
        return nullptr;
    }
    return tab->GetPtrAs<clSTCEventsHandler>();
}
clSTCEventsHandler* clSTCBookCtrl::GetActiveHandler() const { return GetHandler(wxNOT_FOUND); }

int clSTCBookCtrl::GetSelection() const { return m_tabCtrl->GetSelection(); }
